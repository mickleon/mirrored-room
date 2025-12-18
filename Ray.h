#pragma once

#include "nlohmann/json_fwd.hpp"
#include "raylib.h"

#include "Room.h"

using nlohmann::json;

class Wall;
class WallLine;
class WallRound;
class Room;

// Класс сегмента луча
class RaySegment {
private:
    Vector2 start; // Точка начала
    Vector2 end;   // Точка конца (большие координаты чтобы гарантированно найти
                   // пересечение)
    bool hasHit;   // Было ли столкновение со стеной
    Vector2 hitPoint; // Точка столкновения (если есть)
    Wall *hitWall;    // Стена, с которой произошло столкновение
    RaySegment *next; // Следующий сегмент луча
    int depth;        // Число переторажений

    Vector2 intersectionWithWallLine(WallLine *wall);
    Vector2 intersectionWithWallRound(WallRound *wall);

public:
    RaySegment(const Vector2 &start, const Vector2 &end, int depth = 1);
    void updateParameters(Room *room);
    void draw() const;
    ~RaySegment();
};

// Класс вершины луча
class RayStart {
    Vector2 start; // Точка начала луча
    float angle; // Угол относительно родительской стены (от 1 до 179 градусов )
    Wall *wall;  // Родительская стена
    float t;
    RaySegment *ray = nullptr;
    bool inverted;

public:
    RayStart(const Vector2 &point, Wall *wall, float angle);

    class InvalidAngle: public std::exception { // Исключение, выбрасывается,
                                                // когда неверный угол

    public:
        const char *what() const noexcept;
    };

    class CantStartInCorner:
        public std::exception { // Исключение, выбрасывается при попытке
                                // разместить начало луча в углу комнаты

    public:
        const char *what() const noexcept;
    };

    float getAngle() { return angle; }

    Vector2 getStart() { return start; }

    Wall *getWall() { return wall; }

    void setAngle(float angle);
    void setWall(Wall *wall);
    void inverseT();
    void inverseDirection();
    void updateRaySegments();
    void updateParams();

    json toJson(); // Экспорт в json

    void draw();

    ~RayStart();
};

// Класс области цели (круг)
class AimArea {
private:
    Vector2 center; // Центр круга
    float radius;   // Радиус кругa

public:
    AimArea(const Vector2 &center, float radius = 20.0f);

    Vector2 getCenter() { return center; }

    float getRadius() { return radius; }

    void setCenter(const Vector2 &center);

    bool intersectsWithRay(
        const Vector2 &rayStart, const Vector2 &rayEnd,
        Vector2 &intersectionPoint
    );

    bool containsPoint(
        const Vector2 &point
    ); // Проверка, находится ли точка внутри области

    json toJson();

    void draw();
};
