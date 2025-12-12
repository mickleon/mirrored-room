#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include "raylib.h"

#include "Room.h"
#include "Utils.h"

using nlohmann::json, std::ofstream, std::cout;
namespace fs = std::filesystem;

const Color Wall::color = BROWN;
const float Wall::thick = 4;
const int Room::minimalDistance = 20;

Point::Point(const Vector2 &coord) {
    Point::coord = coord;
}

void Point::setCoord(const Vector2 &coord) {
    Point::coord = coord;
    updateWalls();
}

const Vector2 &Point::getCoord() {
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
    DrawLineEx(start->getCoord(), end->getCoord(), thick, BROWN);
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

    if (isBig) {
        startAngle -= 360;
    }
}

WallRound::WallRound(Point *start, Point *end): Wall(start, end) {
    isBig = false;
    orient = false;
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

void Room::addPoint(const Vector2 &coord) {
    for (Point &point : points) {
        float d = distance(point.getCoord(), coord);
        if (d < minimalDistance) {
            throw Room::PointsAreTooClose();
        }
    }
    points.push_back(coord);
}

void Room::addWallLine(Point &start, Point &end) {
    WallLine wall = WallLine(&start, &end);
    walls.push_back(wall);
}

void Room::addWallRound(Point &start, Point &end) {
    WallRound wall = WallRound(&start, &end);
    walls.push_back(wall);
}

void Room::movePoint(Point &p, float x, float y) {
    p.setCoord(Vector2{x, y});
}

void Room::draw() {
    for (Wall &wall : walls) {
        wall.draw();
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
        throw std::runtime_error(
            "Указанный путь не является файлом: " + fileName
        );
    }

    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Не удалось открыть файл для чтения: " + fileName
        );
    }

    json dump;
    file >> dump;

    if (file.bad()) {
        throw std::runtime_error("Ошибка чтения файла: " + fileName);
    }

    if (!file.eof() && file.fail()) {
        throw std::runtime_error("Ошибка формата файла: " + fileName);
    }

    file.close();

    clear();
}

void Room::save(fs::path filePath) {
    std::string fileName = filePath.filename().string();
    if (!fs::exists(filePath.parent_path())) {
        throw std::runtime_error("Некоррректное имя файла: " + fileName);
    }

    std::ofstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Не удалось открыть файл для записи: " + fileName
        );
    }

    json dump = {
        {"happy", true},
        {"pi", 3.141},
    };

    file << dump.dump(4) << '\n';

    if (file.fail()) {
        throw std::runtime_error("Нет прав на запись в файл: " + fileName);
    }

    file.close();
}

void Room::clear() {
    points.clear();
    walls.clear();
}
