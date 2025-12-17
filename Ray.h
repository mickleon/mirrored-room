#pragma once

#include "Room.h"
#include "raylib.h"

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
    int depth;        // Порядок сегмента луча

    Vector2 intersectionWithWallLine(WallLine *wall);
    Vector2 intersectionWithWallRound(WallRound *wall);

public:
    RaySegment(const Vector2 &start, const Vector2 &end, int depth = 0);
    void findIntersections(Room *room, Wall *originWall);
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
    void updateRaySegments();
    void updateParams();

    void draw();

    ~RayStart();
};
