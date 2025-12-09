#pragma once

#include <cmath>
#include <vector>

#include "raylib.h"

using std::vector;

class Wall;

// Класс точек между зеркальнымы стенами
class Point {
private:
    Vector2 coord;        // Ее координаты
    vector<Wall *> walls; // Связанные с ней стены

public:
    Point(const Vector2 &coord);

    void setCoord(const Vector2 &coord);

    const Vector2 &getCoord();
    float getX();
    float getY();

    void updateWalls();

    void addWall(Wall *wall);

    void draw();
};

// Абстрактный класс зеркальной стены
class Wall {
protected:
    Point *start; // Начальная точка
    Point *end;   // Конечная точка

public:
    const Color static color; // Цвет
    const float static thick; // Толщина

    Wall(Point *start, Point *end);

    virtual void updateParams() {}

    virtual void draw() {}
};

// Прямая стена
class WallLine: public Wall {
public:
    WallLine(Point *start, Point *end): Wall(start, end) {}

    void draw();
};

// Дуга
class WallRound: public Wall {
    float startAngle; // Угол начала относительно горизонтальной прямой
    float endAngle;   // Угол конца относительно горизонтальной прямой

    float radius;   // Радиус дуги
    Vector2 center; // Центр дуги
    bool isBig;
    bool orient;

    void updateParams();
    void updateAngles();

public:
    WallRound(Point *start, Point *end);

    void toggleOrient();

    void draw();
};

// Комната, представляющая собой многоугольник
class Room {
private:
    vector<Point> points; // Вершины многоугольника
    vector<Wall> walls;   // Стены

public:
    class PointsAreTooClose {};

    void addPoint(const Vector2 &coord);

    void addWallLine(Point &start, Point &end);
    void addWallRound(Point &start, Point &end);

    void movePoint(Point &p, float x, float y);

    void draw();
};
