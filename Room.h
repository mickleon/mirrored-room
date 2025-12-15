#pragma once

#include <cmath>
#include <exception>
#include <vector>

#include "nlohmann/json.hpp"
#include "raylib.h"

using std::vector, nlohmann::json;

class Wall;

// Класс точек между зеркальнымы стенами
class Point {
private:
    Vector2 coord;        // Ее координаты
    vector<Wall *> walls; // Связанные с ней стены

public:
    Point(const Vector2 &coord);
    Point(const json &j); // Конструктор из json

    void setCoord(const Vector2 &coord);

    Vector2 getCoord();
    float getX();
    float getY();

    void updateWalls(); // Обновление параметров свзанных стен

    void addWall(Wall *wall);

    json to_json(); // Экспорт в json

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

    virtual void updateParams() {} // Обновление параметров стены

    virtual void draw() {}

    virtual json to_json() { return json{}; } // Экспорт в json

    Point *getStart() { return start; }

    Point *getEnd() { return end; }
};

// Прямая стена
class WallLine: public Wall {
public:
    WallLine(Point *start, Point *end): Wall(start, end) {}

    WallLine(const json &j); // Конструктор из json

    void updateParams() {}

    json to_json();

    void draw();
};

// Дуга
class WallRound: public Wall {
    float startAngle; // Угол начала относительно горизонтальной прямой
    float endAngle;   // Угол конца относительно горизонтальной прямой

    float radiusCoef; // Коэфициент, на который нужно умножить хорду, чтобы
                      // получить радиус
    float chord;      // Длина хорды между двумя точками
    float radius;     // Радиус дуги
    Vector2 center;   // Центр дуги
    bool isBig;       // БОльшая или меньшая дуга
    bool orient; // 0 - выпуклое, 1 - вогнутое при рисовании выпуклой фиугры по
                 // часовой стрелке

    void updateParams();
    void updateAngles(); // Обновление углов

public:
    WallRound(Point *start, Point *end, float radiusCoef);

    void toggleOrient();

    json to_json();

    void draw();
};

// Комната, представляющая собой многоугольник
class Room {
private:
    vector<Point> points; // Вершины многоугольника
    vector<Wall *> walls; // Стены

public:
    Room();
    Room(const json &j); // Конструктор из json

    const int static minimalDistance; // Минимальное расстояние, на котором
                                      // рядом могут находиться точки

    const int static maximumPoints; // Максимальное число точек в комнате
    const int static minimumPoints; // Максимальное число точек в комнате

    class RoomException: public std::exception {
    public:
        virtual const char *what() const noexcept override;
    };

    class PointsAreTooClose:
        public Room::RoomException { // Исключение, выбрасывается, когда 2 точки
                                     // слишком близки

    public:
        const char *what() const noexcept;
    };

    class TooManyPoints:
        public Room::RoomException { // Исключение, выбрасывается, когда точек
                                     // больше установленного количества

    public:
        const char *what() const noexcept;
    };

    class TooFewPoints:
        public Room::RoomException { // Исключение, выбрасывается,
                                     // когда точек слишком мало

    public:
        const char *what() const noexcept;
    };

    class WallsCollision:
        public Room::RoomException { // Исключение, выбрасывается,
                                     // если стены пересекаются

    public:
        const char *what() const noexcept;
    };

    bool isClosed(); // Замкнутая ли комната

    void addWallLine( // Добавить в конец ломаной прямую стену
        const Vector2 &coord
    );
    void addWallRound( // Добавить в конец ломаной cферическую стену
        const Vector2 &coord, float radiusCoef = 1
    );

    void movePoint(
        Point &p, const Vector2 &coord
    ); // Переместить точку на заданные координаты

    void draw();

    json to_json(); // Экспорт в json

    void clear(); // Очистка комнаты

    ~Room();
};
