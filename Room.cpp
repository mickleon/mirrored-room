#include <cstdio>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "nlohmann/json.hpp"
#include "raylib.h"

#include "Room.h"
#include "Utils.h"

using nlohmann::json, std::ofstream, std::runtime_error;
namespace fs = std::filesystem;

const Color Wall::color = BROWN;
const float Wall::thick = 4;
const int Room::minimalDistance = 20;
const int Room::maximumPoints = 9;
const int Room::minimumPoints = 4;

Point::Point(const Vector2 &coord) {
    Point::coord = coord;
}

void Point::setCoord(const Vector2 &coord) {
    Point::coord = coord;
    updateWalls();
}

Vector2 Point::getCoord() {
    return coord;
}

float Point::getX() {
    return coord.x;
}

float Point::getY() {
    return coord.y;
}

void Point::updateWalls() {
    for (auto *wall : walls) {
        wall->updateParams();
    }
}

void Point::addWall(Wall *wall) {
    walls.push_back(wall);
}

void Point::draw() {
    DrawCircleV(coord, 4.0f, BROWN);
}

Wall::Wall(Point *start, Point *end): start(start), end(end) {
    start->addWall(this);
    end->addWall(this);
    updateParams();
}

void WallLine::draw() {
    DrawLineEx(start->getCoord(), end->getCoord(), thick, color);
}

void WallRound::updateParams() {
    Vector2 m = Vector2{
        (start->getX() + end->getX()) / 2.0f,
        (start->getY() + end->getY()) / 2.0f
    };
    float dx = start->getX() - end->getX();
    float dy = start->getY() - end->getY();
    float d = std::sqrt(dx * dx + dy * dy);

    radius = d;

    float h = std::sqrt(radius * radius - d * d / 4);
    if (orient) {
        center = Vector2{m.x + h * dy / d, m.y - h * dx / d};
    } else {
        center = Vector2{m.x - h * dy / d, m.y + h * dx / d};
    }

    updateAngles();
}

void WallRound::updateAngles() {
    startAngle =
        atan2f(start->getY() - center.y, start->getX() - center.x) * RAD2DEG;
    endAngle = atan2f(end->getY() - center.y, end->getX() - center.x) * RAD2DEG;

    if (startAngle < endAngle) {
        startAngle += 360.0f;
    }

    if ((isBig && !orient) || (!isBig && orient)) {
        startAngle -= 360;
    }
}

WallRound::WallRound(Point *start, Point *end): Wall(start, end) {
    isBig = false;
    orient = true;
    updateParams();
}

void WallRound::toggleOrient() {
    orient = !orient;
    updateParams();
}

void WallRound::draw() {
    DrawRing(
        center, radius - thick / 2, radius + thick / 2, startAngle, endAngle,
        36.0f, color
    );
}

Room::Room() {
    points.reserve(maximumPoints + 1);
}

const char *Room::PointsAreTooClose::what() const noexcept {
    return "Точки не могут находиться слишком близко";
}

const char *Room::TooManyPoints::what() const noexcept {
    return TextFormat("Точек не может быть больше, чем %d", maximumPoints);
}

const char *Room::TooFewPoints::what() const noexcept {
    return TextFormat("Точек не может быть меньше, чем %d", minimumPoints);
}

const char *Room::WallsCollision::what() const noexcept {
    return "Стены не могут пересекаться";
}

bool Room::isClosed() {
    if (walls.size() < 3) {
        return false;
    }

    if (walls[0]->getStart() == walls[walls.size() - 1]->getEnd()) {
        return true;
    }

    return false;
}

void Room::addWallLine(const Vector2 &coord) {
    size_t pointsAmount = points.size();

    for (size_t i = 0; i < pointsAmount; ++i) {
        float d = distance(points[i].getCoord(), coord);
        if (d < minimalDistance) {
            if (i == 0 && pointsAmount > 2) {
                if (pointsAmount < minimumPoints) {
                    throw Room::TooFewPoints();
                }
                WallLine *wall =
                    new WallLine(&points[pointsAmount - 1], &points[0]);
                walls.push_back(wall);
                return;
            } else {
                throw Room::PointsAreTooClose();
            }
        }
    }

    if (pointsAmount > maximumPoints) {
        throw Room::TooManyPoints();
    }

    if (walls.size()) {
        Vector2 collision = {-1, -1};
        for (Wall *wall : walls) {
            if (CheckCollisionLines(
                    wall->getStart()->getCoord(), wall->getEnd()->getCoord(),
                    points[pointsAmount - 1].getCoord(), coord, &collision
                ) &&
                collision.x != points[pointsAmount - 1].getX() &&
                collision.y != points[pointsAmount - 1].getY()) {
                throw Room::WallsCollision();
            }
        }
    }

    points.push_back(coord);
    ++pointsAmount;
    if (pointsAmount > 1) {
        WallLine *wall =
            new WallLine(&points[pointsAmount - 2], &points[pointsAmount - 1]);
        walls.push_back(wall);
    }
}

void Room::addWallRound(const Vector2 &coord) {
    size_t pointsAmount = points.size();

    for (size_t i = 0; i < pointsAmount; ++i) {
        float d = distance(points[i].getCoord(), coord);
        if (d < minimalDistance) {
            if (i == 0 && pointsAmount > 2) {
                if (pointsAmount < minimumPoints) {
                    throw Room::TooFewPoints();
                }
                WallRound *wall =
                    new WallRound(&points[pointsAmount - 1], &points[0]);
                walls.push_back(wall);
                return;
            } else {
                throw Room::PointsAreTooClose();
            }
        }
    }

    if (pointsAmount > maximumPoints) {
        throw Room::TooManyPoints();
    }

    if (walls.size()) {
        Vector2 collision = {-1, -1};
        for (Wall *wall : walls) {
            if (CheckCollisionLines(
                    wall->getStart()->getCoord(), wall->getEnd()->getCoord(),
                    points[pointsAmount - 1].getCoord(), coord, &collision
                ) &&
                collision.x != points[pointsAmount - 1].getX() &&
                collision.y != points[pointsAmount - 1].getY()) {
                throw Room::WallsCollision();
            }
        }
    }

    points.push_back(coord);
    ++pointsAmount;
    if (pointsAmount > 1) {
        WallRound *wall =
            new WallRound(&points[pointsAmount - 2], &points[pointsAmount - 1]);
        walls.push_back(wall);
    }
}

void Room::movePoint(Point &p, const Vector2 &coord) {
    p.setCoord(coord);
}

void Room::draw() {
    for (Wall *wall : walls) {
        wall->draw();
    }

    for (Point &point : points) {
        point.draw();
    }
}

void Room::load(fs::path filePath) {
    std::string fileName = filePath.filename().string();
    if (!fs::exists(filePath)) {
        throw std::runtime_error("Файл не найден: " + fileName);
    }

    if (!fs::is_regular_file(filePath)) {
        throw runtime_error("Указанный путь не является файлом: " + fileName);
    }

    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw runtime_error("Не удалось открыть файл для чтения: " + fileName);
    }

    json dump;

    try {
        file >> dump;
    } catch (json::exception &e) {
        throw runtime_error("Ошибка формата файла: " + fileName);
    }

    if (file.bad()) {
        throw runtime_error("Ошибка чтения файла: " + fileName);
    }

    if (!file.eof() && file.fail()) {
        throw runtime_error("Ошибка формата файла: " + fileName);
    }

    file.close();

    clear();
}

void Room::save(fs::path filePath) {
    std::string fileName = filePath.filename().string();
    if (!fs::exists(filePath.parent_path())) {
        throw runtime_error("Некоррректное имя файла: " + fileName);
    }

    ofstream file(filePath);

    if (!file.is_open()) {
        throw runtime_error("Не удалось открыть файл для записи: " + fileName);
    }

    json dump = {
        {"happy", true},
        {"pi", 3.141},
    };

    file << dump.dump(4) << '\n';

    if (file.fail()) {
        throw runtime_error("Нет прав на запись в файл: " + fileName);
    }

    file.close();
}

void Room::clear() {
    points.clear();
    for (Wall *wall : walls) {
        delete wall;
    }
    walls.clear();
}

Room::~Room() {
    for (Wall *wall : walls) {
        delete wall;
    }
}
