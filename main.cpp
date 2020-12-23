#define vtkRenderingCore_AUTOINIT 3(vtkInteractionStyle, vtkRenderingFreeType, vtkRenderingOpenGL2)
#define vtkRenderingVolume_AUTOINIT 1(vtkRenderingVolumeOpenGL2)

#include <vtkDataArray.h>
#include <vtkPoints.h>
#include <vtkDoubleArray.h>
#include <vtkLine.h>
#include <vtkPolyData.h>
#include <vtkColorTransferFunction.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <iostream>
#include "utils/vtkWorker.h"
#include "utils/streamlineWorker.h"
#include "utils/streamSurfaceWorker.h"

int main(int argc, char** argv)
{
    // Зададим длину шага
    double h = 0.15;
    std::cout << "Длина шага: ";
    std::cin >> h;

    // Установим количество узлов вдоль главной диагонали для нахождения линий тока
    int nSeeds = 100;
    std::cout << "Количество узлов: ";
    std::cin >> nSeeds;

    // Укажите, должна ли быть сгенерирована поверхность на основе линий тока
    bool generateSurface = true;
    std::cout << "Отрисовать поверхность на основе линий тока? (0=Нет, 1=Да): ";
    std::cin >> generateSurface;

    // Загрузим информацию о сетке
    vtkSmartPointer<vtkStructuredPoints> vectorField = loadStructuredPointsFile("delta.vtk");

    // Получим размеры, расстояние и начало координат физического векторного поля
    int* dimensions = vectorField->GetDimensions();
    double* spacing = vectorField->GetSpacing();
    double* origin = vectorField->GetOrigin();

    // Найдем физическую начальную и конечную точки векторного поля
    // NOTE: Эти две точки соответствуют диагонали, которая проходит через векторное поле
    // от исходного угла до противоположного угла
    Vector3* P = new Vector3(origin[0], origin[1], origin[2]);
    Vector3* Q = new Vector3((dimensions[0] * spacing[0]) + origin[0], (dimensions[1] * spacing[1]) + origin[1], (dimensions[2] * spacing[2]) + origin[2]);

    std::cout << std::endl;
    std::cout << "Найдена исходная точка: " << P->toString() << std::endl;
    std::cout << "Противоположная точка найдена: " << Q->toString() << std::endl;

    // Укажим, будет ли диагональ использоваться для генерации линий тока
    bool useDiagonal = true;

    std::cout << std::endl;
    std::cout << "Использовать диагональ для генерации линиий тока? (0=Нет, 1=Да): ";
    std::cin >> useDiagonal;

    // Если диагональ не будет использоваться, то просим начальную и конечную точки
    if (!useDiagonal)
    {
        double px;
        double py;
        double pz;

        double qx;
        double qy;
        double qz;

        std::cout << "Координаты начальной точки (например: 20 31 42): ";
        std::cin >> px >> py >> pz;

        std::cout << "Координаты конечной точки (например: 56 78 90): ";
        std::cin >> qx >> qy >> qz;

        // Обновим стартовую и конечную точку
        P->set(px, py, pz);
        Q->set(qx, qy, qz);
    }

    // Инициализируем объект, в котором будут храниться точки линий тока
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

    // Инициализируйте объект, в котором будут храниться линии тока
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

    // Установите индикатор для обработки нового потока
    //! NOTES: Это необходимо для создания линий тока. Если он установлен в true, это означает,
    //! что только первые точки были вставлены в визуальную линию;
    //! в противном случае у нас есть по крайней мере две точки, и поэтому мы можем определить сегмент линии
    bool newStreamline = true;

    // Получим линии тока
    std::vector<Streamline*>* streamlines = getStreamlines(vectorField, P, Q, nSeeds, h);

    // Инициализируйте переменные, в которых будут храниться максимальная и минимальная скорости
    double maxSpeed = 0;
    double minSpeed = 0;

    // Укажим следующее значение скорости, которое будет обрабатываться первым
    //! NOTE: Это делается для получения значений максимальной и минимальной скорости в векторном поле
    bool firstSpeed = true;

    // Инициализировать указатель точек
    //! NOTE: Это необходимо для определения линий
    int pointIndex = 0;

    // Инициализируем массив, в котором будет храниться цвет точек.
    // Затем установим его размер с общим количеством точек во всех линиях тока плюс восемь (для граничных линий)
    vtkSmartPointer<vtkDoubleArray> pointColors = vtkSmartPointer<vtkDoubleArray>::New();
    pointColors->SetNumberOfValues(getNumPoints(streamlines) + 8);

    // Пройдемся по каждой из линий тока и сгенерируем их
    int nStreamlines = streamlines->size();
    for (int i = 0; i < nStreamlines; i += 1)
    {
        Streamline* streamline = streamlines->at(i);

        int nPoints = streamline->size();

        newStreamline = true;

        // Добавим точки в визуальную линию тока, генерируя линии в процессе
        for (int j = 0; j < nPoints; j += 1)
        {
            // Получим указатель на текущую точку в потоке и сохраним его в объекте points
            Vector3* P = streamline->pointAt(j);
            points->InsertNextPoint(P->x, P->y, P->z);

            // Храним его скорость для окрашивания
            pointColors->SetValue(pointIndex, streamline->speedAt(j));

            // Сохраним индекс точки в объекте
            // NOTE: Это необходимо для построения поверхности
            streamline->indexInPoints->push_back(pointIndex);

            pointIndex += 1;

            // Если это первая сохраненная скорость, то установим ее как для max, так и для min;
            // в противном случае обновим переменные соответственно
            if (firstSpeed)
            {
                maxSpeed = streamline->speedAt(j);
                minSpeed = streamline->speedAt(j);
                firstSpeed = false;
            }
            else
            {
                if (streamline->speedAt(j) > maxSpeed)
                {
                    maxSpeed = streamline->speedAt(j);
                }
                else if (streamline->speedAt(j) < minSpeed)
                {
                    minSpeed = streamline->speedAt(j);
                }
            }

            // Если это не новая линия тока, то добавим линию, соответствующую двум последним точкам
            if (!newStreamline)
            {
                // Инициализируем текущую строку, определим ее и сохраните в объекте lines
                vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
                line->GetPointIds()->SetId(0, pointIndex - 2);
                line->GetPointIds()->SetId(1, pointIndex - 1);
                lines->InsertNextCell(line);
            }

            // Укажим, что это больше не новая линия обтекания, так как она уже имеет точку в объекте points
            newStreamline = false;
        }
    }

    // Добавим граничные точки и создадим граничные линии
    points->InsertNextPoint(origin[0], origin[1], origin[2]);
    pointColors->SetValue(pointIndex, maxSpeed);
    pointIndex += 1;

    points->InsertNextPoint((dimensions[0] * spacing[0]) + origin[0], origin[1], origin[2]);
    pointColors->SetValue(pointIndex, maxSpeed);
    pointIndex += 1;

    points->InsertNextPoint((dimensions[0] * spacing[0]) + origin[0], origin[1], (dimensions[2] * spacing[2]) + origin[2]);
    pointColors->SetValue(pointIndex, maxSpeed);
    pointIndex += 1;

    points->InsertNextPoint(origin[0], origin[1], (dimensions[2] * spacing[2]) + origin[2]);
    pointColors->SetValue(pointIndex, maxSpeed);
    pointIndex += 1;

    points->InsertNextPoint(origin[0], (dimensions[1] * spacing[1]) + origin[1], origin[2]);
    pointColors->SetValue(pointIndex, maxSpeed);
    pointIndex += 1;

    points->InsertNextPoint((dimensions[0] * spacing[0]) + origin[0], (dimensions[1] * spacing[1]) + origin[1], origin[2]);
    pointColors->SetValue(pointIndex, maxSpeed);
    pointIndex += 1;

    points->InsertNextPoint((dimensions[0] * spacing[0]) + origin[0], (dimensions[1] * spacing[1]) + origin[1], (dimensions[2] * spacing[2]) + origin[2]);
    pointColors->SetValue(pointIndex, maxSpeed);
    pointIndex += 1;

    points->InsertNextPoint(origin[0], (dimensions[1] * spacing[1]) + origin[1], (dimensions[2] * spacing[2]) + origin[2]);
    pointColors->SetValue(pointIndex, maxSpeed);
    pointIndex += 1;

    vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, pointIndex - 8);
    line->GetPointIds()->SetId(1, pointIndex - 7);
    lines->InsertNextCell(line);

    line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, pointIndex - 7);
    line->GetPointIds()->SetId(1, pointIndex - 6);
    lines->InsertNextCell(line);

    line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, pointIndex - 6);
    line->GetPointIds()->SetId(1, pointIndex - 5);
    lines->InsertNextCell(line);

    line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, pointIndex - 5);
    line->GetPointIds()->SetId(1, pointIndex - 8);
    lines->InsertNextCell(line);

    line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, pointIndex - 4);
    line->GetPointIds()->SetId(1, pointIndex - 3);
    lines->InsertNextCell(line);

    line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, pointIndex - 3);
    line->GetPointIds()->SetId(1, pointIndex - 2);
    lines->InsertNextCell(line);

    line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, pointIndex - 2);
    line->GetPointIds()->SetId(1, pointIndex - 1);
    lines->InsertNextCell(line);

    line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, pointIndex - 1);
    line->GetPointIds()->SetId(1, pointIndex - 4);
    lines->InsertNextCell(line);

    line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, pointIndex - 8);
    line->GetPointIds()->SetId(1, pointIndex - 4);
    lines->InsertNextCell(line);

    line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, pointIndex - 5);
    line->GetPointIds()->SetId(1, pointIndex - 1);
    lines->InsertNextCell(line);

    line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, pointIndex - 7);
    line->GetPointIds()->SetId(1, pointIndex - 3);
    lines->InsertNextCell(line);

    line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, pointIndex - 6);
    line->GetPointIds()->SetId(1, pointIndex - 2);
    lines->InsertNextCell(line);

    // Если поверхность потока должна быть отображена, то сгенерируем ее;
    // в противном случае установм для этого объекта значение NULL
    vtkSmartPointer<vtkCellArray> triangles = generateSurface ? buildStreamSurface(streamlines) : nullptr;

    // Инициализируем объект, в который будет добавлена вся геометрия. Затем вставьте точки и линии
    vtkSmartPointer<vtkPolyData> linesPolydata = vtkSmartPointer<vtkPolyData>::New();
    linesPolydata->SetPoints(points);
    linesPolydata->SetLines(lines);
    linesPolydata->GetPointData()->SetScalars(pointColors);

    // Если была построена поверхность, то вставим треугольники
    if (generateSurface)
    {
        linesPolydata->SetPolys(triangles);
    }

    // Определим цветовую функцию для визуализации скорости на векторном поле
    vtkSmartPointer<vtkColorTransferFunction> colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    colorFunction->AddRGBPoint(maxSpeed, 1.0, 1.0, 1.0);	// White for max speed
    colorFunction->AddRGBPoint(minSpeed, 0.0, 0.0, 1.0); 	// Blue for low speed

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(linesPolydata);
    mapper->SetLookupTable(colorFunction);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(actor);
    renderer->SetBackground(0.1, 0.1, 0.1);

    vtkSmartPointer<vtkRenderWindow> window = vtkSmartPointer<vtkRenderWindow>::New();
    window->AddRenderer(renderer);
    window->SetSize(800, 800);
    window->Render();

    vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(window);
    interactor->Initialize();
    interactor->Start();

    return EXIT_SUCCESS;
}
