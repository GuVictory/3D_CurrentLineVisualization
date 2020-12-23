#ifndef STREAMLINE_H
#define STREAMLINE_H


#include <vector>
#include "utils/vector3.h"

/*
    Класс, представляющий линию тока в 3D пространстве

    PS: Этот класс предназначен для хранения данных. Здесь не реализована существенная логика.
*/
class Streamline
{
    public:

        // Точки линии тока
        std::vector<Vector3*>* points;

        // Значение скорости линии тока в каждой точке
        std::vector<double>* speeds;

        // Расположение каждой точки обтекаемой линии в объекте points
        //! NOTE: Это необходимо для визуализации поверхности потока
        std::vector<int>* indexInPoints;

        /*
            Конструктор класса
        */
        Streamline();

        /*
            Деструктор класса
        */
        ~Streamline();

        /*
            Возвращает размер линии тока (количесвто точек)
        */
        size_t size();

        /*
            Возвращает указатель на вектор в заданной позиции
        */
        Vector3* pointAt(int i);

        /*
            Возвращает скорость вектора в заданном положении
        */
        double speedAt(int i);

        /*
            Вставляет заданную точку и значение скорости в векторы линий тока
        */
        void push_back(Vector3* P, double s);
};

#endif // STREAMLINE_H
