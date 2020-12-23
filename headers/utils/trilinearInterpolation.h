//
// Created by GuVictory on 23.12.2020.
//

#ifndef INC_3D_CURRENTLINEVISUALIZATION_TRILINEARINTERPOLATION_H
#define INC_3D_CURRENTLINEVISUALIZATION_TRILINEARINTERPOLATION_H

#include "utils/vector3.h"

/*
	Вычисляет новую точку с помощью трилинейной интерполяции
	@param P1
	@param P2
	@param P3
	@param P4
	@param P5
	@param P6
	@param P7
	@param P8
	@param u
	@param v
	@param w
	@return Указатель Vector3 на объект с интерполированными значениями
*/
Vector3* trilinearInterpolation(Vector3* P1, Vector3* P2, Vector3* P3, Vector3* P4, Vector3* P5, Vector3* P6, Vector3* P7, Vector3* P8, double u, double v, double w)
{
    // Вычисляем необходимые условия для интерполяции
    Vector3* term1 = P1->clone();
    Vector3* term2 = P2->clone()->sub(P1)->multiply(u);
    Vector3* term3 = P4->clone()->sub(P1)->multiply(v);
    Vector3* term4 = P5->clone()->sub(P1)->multiply(w);
    Vector3* term5 = P1->clone()->sub(P2)->add(P3)->sub(P4)->multiply(u * v);
    Vector3* term6 = P1->clone()->sub(P2)->add(P6)->sub(P5)->multiply(u * w);
    Vector3* term7 = P1->clone()->sub(P4)->add(P8)->sub(P5)->multiply(v * w);
    Vector3* term8 = P1->clone()->sub(P2)->add(P3)->sub(P4)->add(P5)->sub(P6)->add(P7)->sub(P8)->multiply(u * v * w);

    // Вычисляем интерполированное значение
    Vector3* result = term1->clone()->add(term2)->add(term3)->add(term4)->add(term5)->add(term6)->add(term7)->add(term8);

    // Удаляем указатели
    delete term1;
    delete term2;
    delete term3;
    delete term4;
    delete term5;
    delete term6;
    delete term7;
    delete term8;

    // Возвращает интерполированное значение
    return result;
}

#endif //INC_3D_CURRENTLINEVISUALIZATION_TRILINEARINTERPOLATION_H
