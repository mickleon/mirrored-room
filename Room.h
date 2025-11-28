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
    Point(float x, float y) { coord = Vector2{x, y}; }

    void setCoord(const Vector2 &coord) {
        Point::coord = coord;
        updateWalls();
    }

    const Vector2 &getCoord() { return coord; }

    const float getX() { return coord.x; }

    const float getY() { return coord.y; }

    void updateWalls();

    void addWall(Wall *wall) { walls.push_back(wall); }

    void draw() {}
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

void Point::updateWalls() {
    for (auto *wall : walls) {
        wall->updateParams();
    }
}

Wall::Wall(Point *start, Point *end): start(start), end(end) {
    start->addWall(this);
    end->addWall(this);
    updateParams();
}

// Прямая стена
class WallLine: public Wall {
public:
    WallLine(Point *start, Point *end): Wall(start, end) {}

    void draw() {
        DrawLineEx(start->getCoord(), end->getCoord(), thick, BROWN);
    }
};

// Дуга
class WallRound: public Wall {
    float startAngle; // Угол начала относительно горизонтальной прямой
    float endAngle;   // Угол конца относительно горизонтальной прямой

    float radius;   // Радиус дуги
    Vector2 center; // Центр дуги

public:
    WallRound(Point *start, Point *end): Wall(start, end) {}

    void updateParams() {
        Vector2 m = Vector2{
            (start->getX() + end->getX()) / 2.0f,
            (start->getY() + end->getY()) / 2.0f
        };
        float dx = start->getX() - end->getX();
        float dy = start->getY() - end->getY();

        float d = std::sqrt(dx * dx + dy * dy);

        radius = d / 2;

        float h = std::sqrt(radius * radius - d * d / 4);

        center = Vector2{m.x - h * dy / d, m.y + h * dx / d};

        startAngle = atan2f(dy, dx) * RAD2DEG + 180;
        endAngle = startAngle + 2 * asinf(d / (2 * radius)) * RAD2DEG;
    }

    void draw() {
        DrawRing(
            center, radius - thick / 2, radius + thick / 2, startAngle,
            endAngle, 36.0f, color
        );
    }
};

// Комната, представляющая собой многоугольник
class Room {
private:
    vector<Point *> points; // Вершины многоугольника
    vector<Wall *> walls;   // Стены

public:
    Point *addPoint(float x, float y) {
        points.push_back(new Point(x, y));
        return points.back();
    }

    void addWallLine(Point *start, Point *end) {
        walls.push_back(new WallLine(start, end));
    }

    void addWallRound(Point *start, Point *end) {
        walls.push_back(new WallRound(start, end));
    }

    void movePoint(Point *p, float x, float y) { p->setCoord(Vector2{x, y}); }

    void draw() {
        for (Wall *wall : walls) {
            wall->draw();
        }

        for (Point *point : points) {
            point->draw();
        }
    }
};
