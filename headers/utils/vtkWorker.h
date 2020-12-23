//
// Created by GuVictory on 23.12.2020.
//

#ifndef INC_3D_CURRENTLINEVISUALIZATION_VTKWORKER_H
#define INC_3D_CURRENTLINEVISUALIZATION_VTKWORKER_H

#include <vtkSmartPointer.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>
#include <vtkDataArray.h>
#include <vtkStructuredPoints.h>
#include <vtkPointData.h>
#include "utils/vector3.h"
#include "utils/trilinearInterpolation.h"

/*
	Загружает файл с заданным именем файла и возвращает ссылку на сгенерированный объект vtkStructuredPoints
	@param имя файла
	@return VtkSmartPointer на объект vtkStructuredPoints
*/
vtkSmartPointer<vtkStructuredPoints> loadStructuredPointsFile(char* filename)
{
    // Определяем reader и загружаем файл
    vtkSmartPointer<vtkStructuredPointsReader> reader = vtkSmartPointer<vtkStructuredPointsReader>::New();
    reader->SetFileName(filename);
    reader->Update();

    // Возвращаем созданный объект
    return reader->GetOutput();
}

/*
	Вычисляет интерполированный 3D-вектор для физической точки (x, y, z), используя данные в векторном поле
	@param vectorField указатель на объект vtkStructuredPoints с информацией векторного поля
	@param T Физическая точка в векторном поле
	@return Указатель Vector3 с интерполированным значением. На самом деле это 2D-вектор с z = 0
*/
Vector3* getVoxel3DVector(vtkSmartPointer<vtkStructuredPoints> vectorField, Vector3* T)
{
    // Получить размеры, расстояние и начало координат векторного поля
    int* dimensions = vectorField->GetDimensions();
    double* spacing = vectorField->GetSpacing();
    double* origin = vectorField->GetOrigin();

    // Получить размеры структуры данных
    double width = (double)dimensions[0];
    double height = (double)dimensions[1];
    double depth = (double)dimensions[2];

    // Получите эквивалентную точку с точки зрения структуры данных
    //! NOTE: Это точка между (0,0,0) и (ширина, высота, глубина)
    Vector3* P = new Vector3((T->x - origin[0]) / spacing[0], (T->y - origin[1]) / spacing[1], (T->z - origin[2]) / spacing[2]);

    // Вычислите значения (u, v, w) вокселя
    //! NOTE: Это не значения (u, v, w) для трилинейной интерполяции,
    //! а для определения того, в каком вокселе находится текущая точка
    double u = ((P->x * (width - 1.0))) / width;
    double v = ((P->y * (height - 1.0))) / height;
    double w = ((P->z * (depth - 1.0))) / depth;

    // Получите координаты восьмых ближайших точек на сетке
    double u_min = floor(u);
    double v_min = floor(v);
    double w_min = floor(w);
    double u_max = ceil(u);
    double v_max = ceil(v);
    double w_max = ceil(w);

    // Получить массив точек из векторного поля
    //! NOTE: Поскольку существует одна последовательность точек, то можно смело спрашивать только о первой
    vtkSmartPointer<vtkDataArray> array = vectorField->GetPointData()->GetArray(0);

    //! Получите восемь точек для трилинейной интерполяции
    //! NOTE: Имейте в виду, что оси выровнены по-разному для данных в массиве и для точек для трилинейной интерполяции.
    //! Для точек данных u-горизонталь, v-вертикаль, w-глубина.
    //! Для трилинейной интерполяции u-горизонталь, v-глубина, w-вертикаль.
    //! Итак, при запросе точек в массиве следует следовать первому подходу.
    //! Однако для порядка точек, которые будут отправлены в трилинейную интерполяционную функцию,
    //! они следуют второму подходу.

    // Определим P1
    int i = u_min + (v_min * width) + (w_min * width * height);
    double* data = array->GetTuple(i);
    Vector3* P1 = new Vector3(data[0], data[1], data[2]);

    // Определим P2
    i = u_max + (v_min * width) + (w_min * width * height);
    data = array->GetTuple(i);
    Vector3* P2 = new Vector3(data[0], data[1], data[2]);

    // Определим P3
    i = u_max + (v_max * width) + (w_min * width * height);
    data = array->GetTuple(i);
    Vector3* P3 = new Vector3(data[0], data[1], data[2]);

    // Определим P4
    i = u_min + (v_max * width) + (w_min * width * height);
    data = array->GetTuple(i);
    Vector3* P4 = new Vector3(data[0], data[1], data[2]);

    // Определим P5
    i = u_min + (v_min * width) + (w_max * width * height);
    data = array->GetTuple(i);
    Vector3* P5 = new Vector3(data[0], data[1], data[2]);

    // Определим P6
    i = u_max + (v_min * width) + (w_max * width * height);
    data = array->GetTuple(i);
    Vector3* P6 = new Vector3(data[0], data[1], data[2]);

    // Определим P7
    i = u_max + (v_max * width) + (w_max * width * height);
    data = array->GetTuple(i);
    Vector3* P7 = new Vector3(data[0], data[1], data[2]);

    // Определим P8
    i = u_min + (v_max * width) + (w_max * width * height);
    data = array->GetTuple(i);
    Vector3* P8 = new Vector3(data[0], data[1], data[2]);

    // Получим (u, v, w) для трилинейной интерполяции
    //! NOTE: Значения между [0, 1]
    u = u - u_min;
    v = v - v_min;
    w = w - w_min;

    // Вычислим интерполированный вектор с помощью трилинейной интерполяции
    Vector3* result = trilinearInterpolation(P1, P2, P3, P4, P5, P6, P7, P8, u, v, w);

    // Удалим указатели
    delete P;
    delete P1;
    delete P2;
    delete P3;
    delete P4;
    delete P5;
    delete P6;
    delete P7;
    delete P8;

    // Возвращает результат трилинейной интерполяции
    return result;
}

#endif //INC_3D_CURRENTLINEVISUALIZATION_VTKWORKER_H
