//
// Created by GuVictory on 23.12.2020.
//

#ifndef INC_3D_CURRENTLINEVISUALIZATION_STREAMSURFACEWORKER_H
#define INC_3D_CURRENTLINEVISUALIZATION_STREAMSURFACEWORKER_H

#include <vtkTriangle.h>
#include <vtkCellArray.h>
#include "services/streamline.h"

/*
	Создаем поверхность потока на основе информации о линиях тока. Каждый треугольник
    лента генерируется с использованием жадного подхода, описанного в [Hultquist 1992]
    необходимо для работы с криволинейной сеткой
	@param streamlines Указатель на вектор с линиями тока
	@return Указатель на объект vtkCellArray с информацией о треугольниках
*/
vtkSmartPointer<vtkCellArray> buildStreamSurface(std::vector<Streamline*>* streamlines)
{
    // Инициализируйте объект, в котором будут храниться треугольники поверхности потока
    vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();

    // Получим количество сохраненных линий тока
    int n = streamlines->size();

    // Получим указатель на первую линию тока
    //! NOTE: Это указатель на левую линию тока
    Streamline* L = streamlines->at(0);

    // Пройдем по линиям тока, начиная с индекса 1
    for (int i = 1; i < n; i += 1)
    {
        // Построим треугольную ленту между текущей линией тока и предыдущей

        // Получим указатель на вторую линию тока
        //! NOTE: Это указатель на правую линию тока
        Streamline* R = streamlines->at(i);

        // Получим размер каждой линии тока
        int nLeft = L->size();
        int nRight = R->size();

        // Получим первую точку для каждой линии тока
        //! NOTE: Предполагаем, что каждая линия тока имеет хотя бы одну точку
        Vector3* L0 = L->pointAt(0);
        Vector3* R0 = R->pointAt(0);

        // Определите значения индексов в соответствующих линиях тока для точек L0, L1, R0 и R1
        int iL0 = 0;
        int iL1 = 1;
        int iR0 = 0;
        int iR1 = 1;

        // Получим вторую точку для каждой линии тока
        //! NOTE: Если ее не существуюет, то указатель на NULL
        Vector3* L1 = (nLeft == 1) ? nullptr : L->pointAt(1);
        Vector3* R1 = (nRight == 1) ? nullptr : R->pointAt(1);

        // Генерируйте ленты до тех пор, пока одна из вторых точек не станет нулевой
        // NOTE: Это означает, что достигнут конец одной линии тока;
        // следовательно, L0 или R0 имеют указатель на последнюю точку линии тока
        while (L1 != nullptr && R1 != nullptr)
        {
            // Вычислим расстояние диагоналей (L0-R1 и L1-R0)
            // NOTE: Квадратное расстояние быстрее.
            // Вычисление квадратного корня на самом деле не является необходимым для определения того,
            // какое расстояние является самым длинным
            double dL0R1 = L0->squareDistance(R1);
            double dL1R0 = L1->squareDistance(R0);

            // Если первое расстояние меньше или равно, чем второе расстояние, затем определяют треугольник L0R 1R0
            if (dL0R1 <= dL1R0)
            {
                // Определение треугольника L0 R1 R0
                vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
                triangle->GetPointIds()->SetId(0, L->indexInPoints->at(iL0));
                triangle->GetPointIds()->SetId(1, R->indexInPoints->at(iR1));
                triangle->GetPointIds()->SetId(2, R->indexInPoints->at(iR0));

                // Добавить треугольник к объекту треугольников
                triangles->InsertNextCell(triangle);

                // Обновите указатель и индексы для правильной оптимизации
                R0 = R1;
                R1 = (iR1 + 1 < nRight) ? R->pointAt(iR1 + 1) : nullptr;
                iR0 += 1;
                iR1 += 1;
            }
            else
            {
                // Определите треугольник R0 L1 L0
                vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
                triangle->GetPointIds()->SetId(0, R->indexInPoints->at(iR0));
                triangle->GetPointIds()->SetId(1, L->indexInPoints->at(iL1));
                triangle->GetPointIds()->SetId(2, L->indexInPoints->at(iL0));

                // Добавить треугольник к объекту треугольников
                triangles->InsertNextCell(triangle);

                // Обновление указателя и индексов для левой линии тока
                L0 = L1;
                L1 = (iL1 + 1 < nLeft) ? L->pointAt(iL1 + 1) : nullptr;
                iL0 += 1;
                iL1 += 1;
            }
        }

        // Если и L1, и R1 равны нулю, то ничего не нужно делать; в противном случае должен быть сгенерирован треугольник
        //! NOTE: На самом деле это произошло бы, если бы обе линии тока имели только одну точку, очень странный случай
        if (!(L1 == nullptr && R1 == nullptr))
        {
            // Закончим ленту для оставшихся точек

            // Если L1 равно нулю, это означает, что L0 находится в последней точке левой линии тока.
            // Затем соединим все оставшиеся точки в правой линии с этой точкой
            if (L1 == nullptr)
            {
                while (R1 != nullptr)
                {
                    // Определение треугольника L0 R1 R0
                    vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
                    triangle->GetPointIds()->SetId(0, L->indexInPoints->at(iL0));
                    triangle->GetPointIds()->SetId(1, R->indexInPoints->at(iR1));
                    triangle->GetPointIds()->SetId(2, R->indexInPoints->at(iR0));

                    triangles->InsertNextCell(triangle);

                    R0 = R1;
                    R1 = (iR1 + 1 < nRight) ? R->pointAt(iR1 + 1) : nullptr;
                    iR0 += 1;
                    iR1 += 1;
                }
            }
            else
            {
                // Это означает, что R1 равен нулю,а R0 находится в последней точке правой обтекаемой линии.
                // Затем соедините все оставшиеся точки в левой обтекаемой линии с этой точкой

                // Пройдем через левую линию и закончим ленту треугольным веером
                while (L1 != nullptr)
                {
                    // Определение треугольника R0 L1 L0
                    vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
                    triangle->GetPointIds()->SetId(0, R->indexInPoints->at(iR0));
                    triangle->GetPointIds()->SetId(1, L->indexInPoints->at(iL1));
                    triangle->GetPointIds()->SetId(2, L->indexInPoints->at(iL0));

                    triangles->InsertNextCell(triangle);

                    L0 = L1;
                    L1 = (iL1 + 1 < nLeft) ? L->pointAt(iL1 + 1) : nullptr;
                    iL0 += 1;
                    iL1 += 1;
                }
            }
        }

        // Установим текущую правую линию тока в качестве следующей левой линии тока
        L = R;
    }

    return triangles;
}

#endif //INC_3D_CURRENTLINEVISUALIZATION_STREAMSURFACEWORKER_H
