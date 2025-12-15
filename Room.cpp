#include "nlohmann/json_fwd.hpp"
#include "raylib.h"

#include "Room.h"
#include "Utils.h"

using nlohmann::json;

const Color Wall::color = BROWN;
const float Wall::thick = 4;
const int Room::minimalDistance = 20;
const int Room::maximumPoints = 9;
const int Room::minimumPoints = 4;

Point::Point(const Vector2 &coord) {
    Point::coord = coord;
}

Point::Point(const json &j) {
    Point::coord = Vector2{j.at("x").get<float>(), j.at("y").get<float>()};
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

json Point::to_json() {
    json j = {{"x", getX()}, {"y", getY()}};

    return j;
}

void Point::draw() {
    DrawCircleV(coord, 4.0f, BROWN);
}

Wall::Wall(Point *start, Point *end): start(start), end(end) {
    start->addWall(this);
    end->addWall(this);
    updateParams();
}

json WallLine::to_json() {
    json j = {{"type", "line"}};

    return j;
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
    chord = std::sqrt(dx * dx + dy * dy);

    radius = chord * radiusCoef;

    float h = std::sqrt(radius * radius - chord * chord / 4);
    if (orient) {
        center = Vector2{m.x - h * dy / chord, m.y + h * dx / chord};
    } else {
        center = Vector2{m.x + h * dy / chord, m.y - h * dx / chord};
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

    if (isBig == orient) {
        startAngle -= 360;
    }
}

WallRound::WallRound(Point *start, Point *end, float radiusCoef = 1):
    Wall(start, end) {
    isBig = false;
    orient = false;
    WallRound::radiusCoef = radiusCoef;
    updateParams();
}

void WallRound::toggleOrient() {
    orient = !orient;
    updateParams();
}

json WallRound::to_json() {
    json j = {{"type", "round"}, {"radiusCoef", radiusCoef}};

    return j;
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

Room::Room(const json &j) {
    if (!j.at("points").is_array() || !j.at("walls").is_array()) {
        throw std::runtime_error("Неверный формат файла");
    }

    points.reserve(maximumPoints + 1);

    points.push_back(Point(j.at("points").at(0)));
    size_t i = 0;
    for (const auto &wall : j.at("walls")) {
        if (j.at("points").size() == i + 1) {
            const auto &point = j.at("points").at(0);
            if (wall.at("type") == "line") {
                addWallLine(Point(point).getCoord());
            } else if (wall.at("type") == "round") {
                addWallRound(
                    Point(point).getCoord(), wall.at("radiusCoef").get<float>()
                );
            }
            break;
        }
        const auto &point = j.at("points").at(++i);
        if (wall.at("type") == "line") {
            addWallLine(Point(point).getCoord());
        } else if (wall.at("type") == "round") {
            addWallRound(
                Point(point).getCoord(), wall.at("radiusCoef").get<float>()
            );
        }
    }

    if (j.at("points").size() == j.at("walls").size()) {
        const auto &point = j.at("points").at(0);
        const auto &wall = j.at("walls").at(i);
        if (wall.at("type") == "line") {
            addWallLine(Point(point).getCoord());
        } else if (wall.at("type") == "round") {
            addWallRound(
                Point(point).getCoord(), wall.at("radiusCoef").get<float>()
            );
        }
    }
}

const char *Room::RoomException::what() const noexcept {
    return "Неизвестная ошибка при работе с комнатой";
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

    if (pointsAmount >= maximumPoints) {
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

void Room::addWallRound(const Vector2 &coord, float radiusCoef) {
    size_t pointsAmount = points.size();

    for (size_t i = 0; i < pointsAmount; ++i) {
        float d = distance(points[i].getCoord(), coord);
        if (d < minimalDistance) {
            if (i == 0 && pointsAmount > 2) {
                if (pointsAmount < minimumPoints) {
                    throw Room::TooFewPoints();
                }
                WallRound *wall = new WallRound(
                    &points[pointsAmount - 1], &points[0], radiusCoef
                );
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
        WallRound *wall = new WallRound(
            &points[pointsAmount - 2], &points[pointsAmount - 1], radiusCoef
        );
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

json Room::to_json() {
    json j = {
        {"points", json::array()},
        {"walls", json::array()},
    };

    for (Point &point : points) {
        j["points"].push_back(point.to_json());
    }

    for (Wall *wall : walls) {
        j["walls"].push_back(wall->to_json());
    }
    return j;
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
