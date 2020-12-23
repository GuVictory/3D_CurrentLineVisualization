#ifndef VECTOR3_H
#define VECTOR3_H

#include <string>

class Vector3
{
public:

    // Компоненты вектора
    double x;
    double y;
    double z;

    /*
    Конструктор класса
    */
    Vector3() : x(0), y(0), z(0) { }

    /*
    Конструктор класса
    */
    Vector3(double x, double y, double z) : x(x), y(y), z(z) { }

    /*
    Конструктор класса
    */
    Vector3(Vector3* V) : x(V->x), y(V->y), z(V->z) { }

    /*
    Возвращает величину вектора
    */
    double magnitude();

    /*
    нормализует вектор
    */
    Vector3* normalize();

    /*
    Умножает вектор на скаляр
    */
    Vector3* multiply(double s);

    /*
    Создает копию вектора
    */
    Vector3* clone();

    /*
    Добавляет значение заданного вектора
    */
    Vector3* add(Vector3* B);

    /*
    Вычитает значения заданного вектора
    */
    Vector3* sub(Vector3* B);

    /*
    Вычисляет скалярное произведение с вектором B
    */
    double dot(Vector3* B);

    /*
    Вычисляет векторное произведение с вектором B
    */
    Vector3* cross(Vector3* B);

    /*
    Вычисляет квадратное евклидово расстояние до другого вектора
    */
    double squareDistance(Vector3* B);

    /*
    Вычисляет евклидово расстояние до другого вектора
    */
    double distance(Vector3* B);

    /*
    Возвращает интерполированную точку с помощью параметра t и точки B
    */
    Vector3* interpolate(double t, Vector3* B);

    /*
    Сравнивает вектора на равенство
    */
    bool equal(Vector3* B);

    /*
    Присваивает вектору значения (x, y, z)
    */
    Vector3* set(double x, double y, double z);

    /*
    Присваивает вектору значения (x, y, z) вектора V
    */
    Vector3* set(Vector3* V);

    /*
    */
    std::string toString();

};

#endif // VECTOR3_H
