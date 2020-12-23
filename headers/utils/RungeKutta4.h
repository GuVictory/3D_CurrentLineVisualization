//
// Created by GuVictory on 23.12.2020.
//

#ifndef INC_3D_CURRENTLINEVISUALIZATION_RUNGEKUTTA4_H
#define INC_3D_CURRENTLINEVISUALIZATION_RUNGEKUTTA4_H

#include "services/vector3.h"
#include "utils/vtkWorker.h"

/*
	Метод Рунге-Кутта четвертого порядка (RK4) для численного интегрирования.
    Учитывая физическую точку, он возвращает следующую физическую
    точку в линии тока, следующую за информацией векторного поля
	@param vectorField Указатель на объект vtkStructuredPoints с информацией векторного поля
	@param T физическая точка
	@param h Значение шага на обтекаемой линии.
             Отрицательные значения пересекают обтекаемую линию в отрицательном сечении
	@return Указатель Vector3 с координатами следующей точки в линии тока
*/
Vector3* RungeKutta4(vtkSmartPointer<vtkStructuredPoints> vectorField, Vector3* T, double h)
{
    // Получить размеры, расстояние и начало координат векторного поля
    int* dimensions = vectorField->GetDimensions();
    double* spacing = vectorField->GetSpacing();
    double* origin = vectorField->GetOrigin();

    // Вычислить физическую границу векторного поля
    double width = (dimensions[0] * spacing[0]) + origin[0];
    double height = (dimensions[1] * spacing[1]) + origin[1];
    double depth = (dimensions[2] * spacing[2]) + origin[2];

    // Получим K1
    Vector3* K1 = getVoxel3DVector(vectorField, T);

    // Получим K2
    Vector3* K2 = NULL;
    double xn = T->x + ((h / 2.0) * K1->x);
    double yn = T->y + ((h / 2.0) * K1->y);
    double zn = T->z + ((h / 2.0) * K1->z);
    if (xn >= origin[0] && xn < width && yn >= origin[1] && yn < height && zn >= origin[2] && zn < depth)
    {
        Vector3* Xn = new Vector3(xn, yn, zn);
        K2 = getVoxel3DVector(vectorField, Xn);
        delete Xn;
    }
    else
    {
        // Удалим указатель и вернем NULL
        delete K1;
        return NULL;
    }

    // Получим K3
    Vector3* K3 = NULL;
    xn = T->x + ((h / 2.0) * K2->x);
    yn = T->y + ((h / 2.0) * K2->y);
    zn = T->z + ((h / 2.0) * K2->z);
    if (xn >= origin[0] && xn < width && yn >= origin[1] && yn < height && zn >= origin[2] && zn < depth)
    {
        Vector3* Xn = new Vector3(xn, yn, zn);
        K3 = getVoxel3DVector(vectorField, Xn);
        delete Xn;
    }
    else
    {
        // Удалим указатели и вернем NULL
        delete K1;
        delete K2;
        return NULL;
    }

    // Получим K4
    Vector3* K4 = NULL;
    xn = T->x + (h * K3->x);
    yn = T->y + (h * K3->y);
    zn = T->z + (h * K3->z);
    if (xn >= origin[0] && xn < width && yn >= origin[1] && yn < height && zn >= origin[2] && zn < depth)
    {
        Vector3* Xn = new Vector3(xn, yn, zn);
        K4 = getVoxel3DVector(vectorField, Xn);
        delete Xn;
    }
    else
    {
        // Удалим указатели и вернем NULL
        delete K1;
        delete K2;
        delete K3;
        return NULL;
    }

    // Вычислить следующие координаты
    Vector3* term1 = K1->clone();
    Vector3* term2 = K2->clone()->multiply(2.0);
    Vector3* term3 = K3->clone()->multiply(2.0);
    Vector3* term4 = K4->clone();
    Vector3* Xnext = term1->clone()->add(term2)->add(term3)->add(term4)->multiply(h / 6.0)->add(T);

    // Удалить указатели
    delete K1;
    delete K2;
    delete K3;
    delete K4;
    delete term1;
    delete term2;
    delete term3;
    delete term4;

    // Вернем результат
    return Xnext;
}

#endif //INC_3D_CURRENTLINEVISUALIZATION_RUNGEKUTTA4_H
