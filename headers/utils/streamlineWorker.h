//
// Created by GuVictory on 23.12.2020.
//

#ifndef INC_3D_CURRENTLINEVISUALIZATION_STREAMLINEWORKER_H
#define INC_3D_CURRENTLINEVISUALIZATION_STREAMLINEWORKER_H

#include "utils/RungeKutta4.h"
#include "services/streamline.h"


/*
	Возвращает сечение (положительное или отрицательное) обтекаемой линии, начинающееся в данной точке
	@param vectorField Указатель на объект vtkStructuredPoints с информацией векторного поля
	@param T Исходная точка секции обтекания. Это физическая точка
	@param h Значение шага на обтекаемой линии. Отрицательные значения пересекают обтекаемую линию в отрицательном сечении
	@return Обтекаемый объект с информацией о линии тока
*/
Streamline* getStreamlineSection(vtkSmartPointer<vtkStructuredPoints> vectorField, Vector3* T, double h)
{
    // Получить размеры, расстояния и начало координат векторного поля
    int* dimensions = vectorField->GetDimensions();
    double* spacing = vectorField->GetSpacing();
    double* origin = vectorField->GetOrigin();

    // Получить физические граничные значения векторного поля
    double width = (dimensions[0] * spacing[0]) + origin[0];
    double height = (dimensions[1] * spacing[1]) + origin[1];
    double depth = (dimensions[2] * spacing[2]) + origin[2];

    // Инициализируйте объект, в котором будет храниться раздел streamline
    Streamline* streamlineSection = new Streamline();

    // Определите максимальную длину обтекаемого участка
    int n = 1000;
    int length = 0;

    // Хранить координаты начальной точки
    int x0 = T->x;
    int y0 = T->y;
    int z0 = T->z;

    // Пересекайте обтекаемую линию до тех пор, пока она не достигнет границы или своей максимальной длины
    while (length < n)
    {
        // Если текущая точка лежит внутри физической границы векторного поля, то продолжаем;
        // в противном случае остановим генерацию сечения обтекания
        if (x0 >= origin[0] && x0 <= width && y0 >= origin[1] && y0 <= height && z0 >= origin[2] && z0 <= depth)
        {
            // Текущая точка является действительной. Сгенерируем для него объект Vector3
            Vector3* current = new Vector3(x0, y0, z0);

            // Скорость в этой точке - это интерполяция текущей точки относительно восьми соседних точек в структуре данных.
            // Вычислим такой интерполированный вектор,
            // получим его величину и удалим указатель на вектор (он больше не нужен)
            Vector3* speedVector = getVoxel3DVector(vectorField, current);
            double speed = speedVector->magnitude();
            delete speedVector;

            // Вставим текущую точку и ее скорость в секцию обтекания
            streamlineSection->push_back(current, speed);
            length += 1;

            // Получить следующую точку в векторном поле
            Vector3* next = RungeKutta4(vectorField, current, h);

            // Если задано следующее местоположение, то процесс выполняется; в противном случае остановите цикл
            if (next != NULL)
            {
                if (x0 == next->x && y0 == next->y && z0 == next->z)
                {
                    // std::cout << "+++++++++++++++++++++++++++  ОДИНАКОВЫЕ ТОЧКИ  +++++++++++++++++++++++++++" << std::endl;

                    // Удалим указатель и остановите цикл
                    delete next;
                    break;
                }
                else
                {
                    // Сохраним координаты следующей точки в потоке
                    x0 = next->x;
                    y0 = next->y;
                    z0 = next->z;

                    // Удалим указатель
                    delete next;
                }
            }
            else
            {
                //std::cout << "+++++++++++++++++++++++++++ NULL УКАЗАТЕЛЬ +++++++++++++++++++++++++++" << std::endl;

                // Удалим указатель и остановите цикл
                delete next;
                break;
            }
        }
        else
        {
            //std::cout << "+++++++++++++++++++++++++++ ВЫШЛИ ЗА ПРЕДЕЛЫ +++++++++++++++++++++++++++" << std::endl;

            break;
        }
    }

    // Вернем полученный резульатат
    return streamlineSection;
}

/*
	Возвращает указанную линию тока с началом координат в данной точке
	@param vectorField Указатель на объект vtkStructuredPoints с информацией векторного поля
	@param T Базовая точка
	@param h Длина шага
	@return streamline объект
*/
Streamline* getStreamline(vtkSmartPointer<vtkStructuredPoints> vectorField, Vector3* T, double h)
{
    // Получите положительные и отрицательные разделы линии тока
    Streamline* positiveSection = getStreamlineSection(vectorField, T, h);
    Streamline* negativeSection = getStreamlineSection(vectorField, T, -h);

    // Определим streamline оъект
    Streamline* streamline = new Streamline();

    // Получим размер отрицательного сечения линии
    int n = negativeSection->size();

    // Поместим элементы отрицательного участка обтекания в объект основной линии
    for (int i = n - 1; i >= 0; i -= 1)
    {
        // Скопируем текущий элемент в основной объект
        //! NOTE: Здесь точечный вектор клонируется вместо того, чтобы просто хранить свой указатель.
        //! Причина в том, что отрицательный участок объекта впоследствии разрушается.
        streamline->push_back(negativeSection->pointAt(i)->clone(), negativeSection->speedAt(i));
    }

    // Получим размер положительного сечения линии
    n = positiveSection->size();

    // Поместим элементы положительного участка обтекания в объект основной линии
    //! NOTE: Первый элемент не вставляется, так как он уже пришел из отрицательной секции
    for (int i = 1; i < n; i += 1)
    {
        // Скопируем текущий элемент в основной объект
        //! NOTE: Здесь точечный вектор клонируется вместо того, чтобы просто хранить свой указатель.
        //! Причина в том, что объект положительного сечения позже разрушается.
        streamline->push_back(positiveSection->pointAt(i)->clone(), positiveSection->speedAt(i));
    }

    // Удалим указатели
    delete positiveSection;
    delete negativeSection;

    // Вернем линию тока
    return streamline;
}


/*
	Возвращает линии тока, сгенерированные вдоль главной диагонали векторного поля
	@param vectorField Указатель на объект vtkStructuredPoints с информацией векторного поля
	@param n Количество узлов (точек) на главной диагонали, где будут генерироваться линии тока
	@param h Длина шага
	@return Вектор с информацией о сгенерированных линиях тока
*/
std::vector<Streamline*>* getStreamlines(vtkSmartPointer<vtkStructuredPoints> vectorField, Vector3* P, Vector3* Q, int n, double h)
{
    // Определим направление диагонального сегмента (от начала до конца)
    Vector3* D = Q->clone()->sub(P);

    // Инициализируем вектор, в котором будут храниться линии тока
    std::vector<Streamline*>* streamlines = new std::vector<Streamline*>();

    // Инициализируем вспомогательные векторные указатели для генерации линий тока
    Vector3* tD = new Vector3();
    Vector3* T = new Vector3();

    // Вычислим шаг для начальных значений вдоль диагональной линии
    double step = 1.0 / (double)n;

    // Инициализировать параметр диагонали
    double t = 0.0;

    // Сгенерируйте линию тока для каждой из точек диагонали
    for (int i = 0; i < n; i += 1)
    {
        // Вычислим текущую точку
        //! NOTE: Это физическая точка, а не точка структуры данных!!
        tD->set(D)->multiply(t);
        T->set(P)->add(tD);

        // Добавим полученную линию тока в вектор для хранения линий тока
        streamlines->push_back(getStreamline(vectorField, T, h));

        // Значение t для следующей точки по диагонали
        t += step;
    }

    // Удалиим указатели
    delete P;
    delete Q;
    delete D;
    delete tD;
    delete T;

    // Вернем полученные линии тока
    return streamlines;
}


/*
	Возвращает общее количество точек во всех линиях тока
	@param streamlines Указатель на вектор, в котором хранятся линии тока
	@return Количество точек во всех линиях тока
*/
int getNumPoints(std::vector<Streamline*>* streamlines)
{
    // Получить количество сохраненных линий тока
    int n = streamlines->size();

    // Инициализировать счетчик точек
    int count = 0;

    // Пройдем через линии тока и накапливайте их размер
    for (int i = 0; i < n; i += 1)
    {
        count += streamlines->at(i)->size();
    }

    // Вернем итоговое значение счетчика
    return count;
}

#endif //INC_3D_CURRENTLINEVISUALIZATION_STREAMLINEWORKER_H
