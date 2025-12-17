#pragma once

#include <cmath>
#include <exception>
#include <vector>

#include "nlohmann/json.hpp"
#include "raylib.h"

#include "Ray.h"

using std::vector, nlohmann::json;

class Wall;
class Room;
class WallLine;
class WallRound;
class RayStart;

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
    Room *room;
    Wall(Point *start, Point *end, Room *room);

    virtual void updateParams() {} // Обновление параметров стены

    virtual void draw() {}

    virtual json to_json() { return json{}; } // Экспорт в json

    Point *getStart() { return start; }

    Point *getEnd() { return end; }

    virtual Vector2 getNormal(const Vector2 &point) = 0;

    virtual Vector2 closestPoint // Возвращает ближайшую точку к `point`
        (const Vector2 &point) = 0;

    virtual float distanceToWall // Возвращает расстояние до `point`
        (const Vector2 &point);

    virtual float getTByPoint(
        const Vector2 &point, float presicion = 0.1f
    ) // Получить параметр t в диапазоне [0,1] по точке на стене
        = 0;
    virtual Vector2 getPointByT(
        float t
    ) = 0; // Получить точку на стене по параметру  t в диапазоне [0,1]
};

// Прямая стена
class WallLine: public Wall {
public:
    WallLine(Point *start, Point *end, Room *room): Wall(start, end, room) {}

    WallLine(const json &j); // Конструктор из json

    void updateParams() {}

    Vector2 closestPoint(const Vector2 &point);
    Vector2 getNormal(const Vector2 &point);

    float getTByPoint(const Vector2 &point, float presicion = 0.1f);
    Vector2 getPointByT(float t);

    json to_json();

    void draw();
};

// Дуга
class WallRound: public Wall {
    float startAngle; // Угол начала относительно горизонтальной прямой
    float endAngle;   // Угол конца относительно горизонтальной прямой

    float radiusCoef; // Коэфициент для вычисления радиуса (от 0 до 100)
    float chord;      // Длина хорды между двумя точками
    float radius;     // Радиус дуги
    Vector2 center;   // Центр дуги
    bool isBig;       // БОльшая или меньшая дуга
    bool orient; // 0 - выпуклое, 1 - вогнутое при рисовании выпуклой фиугры по
                 // часовой стрелке

    void updateParams();
    void updateAngles(); // Обновление углов

    bool crossesZeroDeg();

    float getAngularLength();

    float getAngleByT(float t);

    float getTByAngle(float angleDeg);

public:
    WallRound(Point *start, Point *end, Room *room, float radiusCoef);

    class InvalidRadiusCoef:
        public std::exception { // Исключение, выбрасывается, когда неверный
                                // radiusCoef

    public:
        const char *what() const noexcept;
    };

    void toggleOrient();

    float getRadiusCoef() { return radiusCoef; }

    void setRadiusCoef(float radiusCoef);

    Vector2 closestPoint(const Vector2 &point);

    Vector2 getNormal(const Vector2 &point);

    Vector2 getPointByT(float t);
    float getTByPoint(const Vector2 &point, float precision = 0.1f);

    bool isAngleInArc(float angleDeg, float precision = 0.0f);

    bool isPointOnArc(const Vector2 &point, float precision = 0.1f);

    Vector2 getCenter();
    float getRadius();
    float getStartAngle();
    float getEndAngle();

    json to_json();

    void draw();
};

// Комната, представляющая собой многоугольник
class Room {
private:
    vector<Point> points; // Вершины многоугольника
    vector<Wall *> walls; // Стены

public:
    RayStart *rayStart = nullptr;
    float defaultRayAngle = PI / 2;

    Room();
    Room(const json &j); // Конструктор из json

    Wall *closestWall(const Vector2 &point);
    RayStart *closestRay(const Vector2 &point);

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

    bool isClosed(); // Замкнутая ли комната

    WallLine *addWallLine( // Добавить в конец ломаной прямую стену
        const Vector2 &coord
    );
    WallRound* addWallRound( // Добавить в конец ломаной cферическую стену
        const Vector2 &coord, float radiusCoef = 50
    );

    Wall *changeWallType(Wall *wall);

    void movePoint(
        Point &p, const Vector2 &coord
    ); // Переместить точку на заданные координаты

    void draw();

    json to_json(); // Экспорт в json

    void addRay(const Vector2 &point);

    vector<Wall *> &getWalls(); // Получить доступ к стенам

    void clear(); // Очистка комнаты

    ~Room();
};
