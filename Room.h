#pragma once

#include <cmath>
#include <exception>
#include <filesystem>
#include <vector>

#include "raylib.h"

using std::vector;
namespace fs = std::filesystem;
class Wall;

// Класс точек между зеркальнымы стенами
class Point {
private:
    Vector2 coord;        // Ее координаты
    vector<Wall *> walls; // Связанные с ней стены

public:
    Point(const Vector2 &coord);

    void setCoord(const Vector2 &coord);

    Vector2 getCoord();
    float getX();
    float getY();

    void updateWalls(); // Обновление параметров свзанных стен

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

    virtual void updateParams() {} // Обновление парамтеров стены

    virtual void draw() {}

    Point *getStart() { return start; }

    Point *getEnd() { return end; }
};

// Прямая стена
class WallLine: public Wall {
public:
    WallLine(Point *start, Point *end): Wall(start, end) {}

    virtual void updateParams() {}

    void draw();
};

// Дуга
class WallRound: public Wall {
    float startAngle; // Угол начала относительно горизонтальной прямой
    float endAngle;   // Угол конца относительно горизонтальной прямой

    float radius;   // Радиус дуги
    Vector2 center; // Центр дуги
    bool isBig;     // БОльшая или меньшая дуга
    bool orient; // 0 - выпуклое, 1 - вогнутое при рисовании выпуклой фиугры по
                 // часовой стрелке

    void updateParams();
    void updateAngles(); // Обновление углов

public:
    WallRound(Point *start, Point *end);

    void toggleOrient();

    void draw();
};

// Комната, представляющая собой многоугольник
class Room {
private:
    vector<Point> points; // Вершины многоугольника
    vector<Wall *> walls; // Стены

public:
    Room();

    const int static minimalDistance; // Минимальное расстояние, на котором
                                      // рядом могут находиться точки

    const int static maximumPoints; // Максимальное число точек в комнате
    const int static minimumPoints; // Максимальное число точек в комнате

    class PointsAreTooClose:
        public std::exception { // Исключение, выбрасывается, когда 2 точки
                                // слишком близки
    public:
        const char *what() const noexcept override;
    };

    class TooManyPoints:
        public std::exception { // Исключение, выбрасывается, когда точек больше
                                // установленного количества
    public:
        const char *what() const noexcept override;
    };

    class TooFewPoints: public std::exception { // Исключение, выбрасывается,
                                                // когда точек слишком мало
    public:
        const char *what() const noexcept override;
    };

    class WallsCollision: public std::exception { // Исключение, выбрасывается,
                                                  // если стены пересекаются
    public:
        const char *what() const noexcept override;
    };

    bool isClosed(); // Замкнутая ли комната

    void addWallLine( // Добавить в конец ломаной прямую стену
        const Vector2 &coord
    );
    void addWallRound( // Добавить в конец ломаной cферическую стену
        const Vector2 &coord
    );

    void movePoint(
        Point &p, const Vector2 &coord
    ); // Переместить точку на заданные координаты

    void draw();

    void load(fs::path filePath); // Импорт из json
    void save(fs::path filePath); // Экспорт в json
    void clear();                 // Очистка комнаты

    ~Room();
};
