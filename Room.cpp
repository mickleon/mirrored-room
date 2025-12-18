#include "nlohmann/json_fwd.hpp"
#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <cstddef>

#include "Ray.h"
#include "Room.h"

using nlohmann::json;

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

json Point::toJson() {
    json j = {{"x", getX()}, {"y", getY()}};

    return j;
}

void Point::draw() {
    DrawCircleV(coord, 4.0f, BROWN);
}

void Point::clear(Wall *wall) {
    walls.erase(std::remove(walls.begin(), walls.end(), wall), walls.end());
}

Wall::Wall(Point *start, Point *end, Room *room):
    start(start),
    end(end),
    room(room) {
    start->addWall(this);
    end->addWall(this);
    updateParams();
}

Wall::~Wall() {
    start->clear(this);
    end->clear(this);
}

json WallLine::toJson() {
    json j = {{"type", "line"}};

    return j;
}

void WallLine::draw() {
    DrawLineEx(start->getCoord(), end->getCoord(), 4, BROWN);
}

const char *WallRound::InvalidRadiusCoef::what() const noexcept {
    return "Значение коэффициента для вычисления радиуса должно быть от 0 до "
           "100";
}

void WallRound::updateParams() {
    Vector2 m = Vector2{
        (start->getX() + end->getX()) / 2.0f,
        (start->getY() + end->getY()) / 2.0f
    };
    float dx = start->getX() - end->getX();
    float dy = start->getY() - end->getY();
    chord = std::sqrt(dx * dx + dy * dy);

    radius = chord * (77.0 / 2 / (radiusCoef + 10) + 3.0 / 20);

    float h = std::sqrt(radius * radius - chord * chord / 4);
    if (orient) {
        center = Vector2{m.x - h * dy / chord, m.y + h * dx / chord};
    } else {
        center = Vector2{m.x + h * dy / chord, m.y - h * dx / chord};
    }

    updateAngles();
    if (room->rayStart) {
        room->rayStart->updateParams();
    }
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

    if (startAngle > endAngle) {
        std::swap(startAngle, endAngle);
    }
}

WallRound::WallRound(
    Point *start, Point *end, Room *room, float radiusCoef = 50
):
    Wall(start, end, room) {
    isBig = false;
    orient = false;
    if (radiusCoef < 0 || radiusCoef > 100) {
        throw InvalidRadiusCoef();
    }
    WallRound::radiusCoef = radiusCoef;
    updateParams();
}

void WallRound::toggleOrient() {
    orient = !orient;
    if (room->rayStart) {
        room->rayStart->inverseT();
    }
    updateParams();
}

void WallRound::setRadiusCoef(float radiusCoef) {
    if (radiusCoef < 0 || radiusCoef > 100) {
        throw InvalidRadiusCoef();
    }
    WallRound::radiusCoef = radiusCoef;
    updateParams();
}

json WallRound::toJson() {
    json j = {{"type", "round"}, {"radiusCoef", radiusCoef}};

    return j;
}

void WallRound::draw() {
    DrawRing(
        center, radius - 2, radius + 2, startAngle, endAngle, 36.0f, BROWN
    );
}

float Wall::distanceToWall(const Vector2 &point) {
    return Vector2Distance(closestPoint(point), point);
}

Vector2 WallLine::closestPoint(const Vector2 &point) {
    Vector2 start = getStart()->getCoord();
    Vector2 end = getEnd()->getCoord();
    Vector2 toPoint = Vector2Subtract(point, start);

    Vector2 wallVec = Vector2Subtract(end, start);
    float wallLengthSq = Vector2LengthSqr(wallVec);

    if (wallLengthSq == 0.0f) {
        return start;
    }

    float t = Vector2DotProduct(toPoint, wallVec) / wallLengthSq;
    t = fmaxf(0.0f, fminf(1.0f, t));
    Vector2 closestPoint = {start.x + t * wallVec.x, start.y + t * wallVec.y};

    return closestPoint;
}

float WallLine::getTByPoint(const Vector2 &point, float presision) {
    Vector2 start = getStart()->getCoord();
    Vector2 end = getEnd()->getCoord();
    Vector2 wallVec = Vector2Subtract(end, start);

    float wallLengthSq = Vector2LengthSqr(wallVec);
    if (wallLengthSq == 0.0f) {
        return Vector2Distance(point, start) <= presision ? 0.0f : -1.0f;
    }

    Vector2 toPoint = Vector2Subtract(point, start);
    float t = Vector2DotProduct(toPoint, wallVec) / wallLengthSq;

    if (t >= -presision && t <= 1.0f + presision) {
        Vector2 closest = Vector2Add(start, Vector2Scale(wallVec, t));
        float distance = Vector2Distance(point, closest);

        if (distance <= presision) {
            return fmaxf(0.0f, fminf(1.0f, t));
        }
    }

    return -1.0f;
}

Vector2 WallLine::getPointByT(float t) {
    Vector2 start = getStart()->getCoord();
    Vector2 end = getEnd()->getCoord();

    t = fmaxf(0.0f, fminf(1.0f, t));

    return Vector2{
        start.x + t * (end.x - start.x), start.y + t * (end.y - start.y)
    };
}

Vector2 WallRound::closestPoint(const Vector2 &point) {
    float distanceToCenter = Vector2Distance(point, center);
    Vector2 diff = Vector2Subtract(point, center);
    float pointAngle = atan2f(diff.y, diff.x) * RAD2DEG;
    if (pointAngle < 0) {
        pointAngle += 360.0f;
    }

    float normalizedStart = fmodf(startAngle, 360.0f);
    float normalizedEnd = fmodf(endAngle, 360.0f);
    if (normalizedStart < 0) {
        normalizedStart += 360.0f;
    }
    if (normalizedEnd < 0) {
        normalizedEnd += 360.0f;
    }

    bool isWithinArc = false;
    if (normalizedEnd >= normalizedStart) {
        isWithinArc =
            (pointAngle >= normalizedStart && pointAngle <= normalizedEnd);
    } else {
        isWithinArc =
            (pointAngle >= normalizedStart || pointAngle <= normalizedEnd);
    }

    if (isWithinArc) {
        if (distanceToCenter > 0) {
            float scale = radius / distanceToCenter;
            return Vector2{
                center.x + diff.x * scale, center.y + diff.y * scale
            };
        } else {
            float angleRad = normalizedStart * DEG2RAD;
            return Vector2{
                center.x + radius * cosf(angleRad),
                center.y + radius * sinf(angleRad)
            };
        }
    } else {
        float startRad = startAngle * DEG2RAD;
        float endRad = endAngle * DEG2RAD;

        Vector2 startPoint = {
            center.x + radius * cosf(startRad),
            center.y + radius * sinf(startRad)
        };

        Vector2 endPoint = {
            center.x + radius * cosf(endRad), center.y + radius * sinf(endRad)
        };

        float distToStart = Vector2Distance(point, startPoint);
        float distToEnd = Vector2Distance(point, endPoint);

        return (distToStart < distToEnd) ? startPoint : endPoint;
    }
}

Vector2 WallLine::getNormal(const Vector2 &point) {
    Vector2 wallVec = Vector2Subtract(end->getCoord(), start->getCoord());
    Vector2 normal = Vector2Normalize({-wallVec.y, wallVec.x});
    return normal;
}

Vector2 WallRound::getNormal(const Vector2 &point) {
    Vector2 toCenter = Vector2Subtract(center, point);
    return Vector2Normalize(toCenter);
}

float normalizeAngle(float angle) {
    float normalized = fmodf(angle, 360.0f);
    if (normalized < 0) {
        normalized += 360.0f;
    }
    return normalized;
}

bool WallRound::crossesZeroDeg() {
    float normStart = normalizeAngle(startAngle);
    float normEnd = normalizeAngle(endAngle);
    return normEnd < normStart;
}

float WallRound::getAngularLength() {
    float normStart = normalizeAngle(startAngle);
    float normEnd = normalizeAngle(endAngle);

    if (normEnd >= normStart) {
        return normEnd - normStart;
    } else {
        return (360.0f - normStart) + normEnd;
    }
}

float WallRound::getAngleByT(float t) {
    t = fmaxf(0.0f, fminf(1.0f, t));

    float normStart = normalizeAngle(startAngle);
    float span = getAngularLength();

    float angle = normStart + t * span;

    if (angle >= 360.0f) {
        angle -= 360.0f;
    }

    return angle;
}

Vector2 WallRound::getPointByT(float t) {
    float angleDeg = getAngleByT(t);
    float angleRad = angleDeg * DEG2RAD;

    return Vector2{
        center.x + radius * cosf(angleRad), center.y + radius * sinf(angleRad)
    };
}

float WallRound::getTByAngle(float angleDeg) {
    float normAngle = normalizeAngle(angleDeg);
    float normStart = normalizeAngle(startAngle);
    float span = getAngularLength();

    float t = 0.0f;

    if (!crossesZeroDeg()) {
        if (normAngle >= normStart && normAngle <= normStart + span) {
            t = (normAngle - normStart) / span;
        } else {
            return -1.0f;
        }
    } else {
        if (normAngle >= normStart) {
            t = (normAngle - normStart) / span;
        } else if (normAngle <= normStart + span - 360.0f) {
            t = (360.0f - normStart + normAngle) / span;
        } else {
            return -1.0f;
        }
    }

    return fmaxf(0.0f, fminf(1.0f, t));
}

float WallRound::getTByPoint(const Vector2 &point, float precision) {
    Vector2 diff = Vector2Subtract(point, center);
    float distanceToCenter = Vector2Length(diff);

    if (fabsf(distanceToCenter - radius) > precision) {
        return -1.0f;
    }

    float pointAngle = atan2f(diff.y, diff.x) * RAD2DEG;

    return getTByAngle(pointAngle);
}

bool WallRound::isAngleInArc(float angleDeg, float precision) {
    float t = getTByAngle(angleDeg);
    return t >= -precision && t <= 1.0f + precision;
}

bool WallRound::isPointOnArc(const Vector2 &point, float precision) {
    Vector2 diff = Vector2Subtract(point, center);
    float distanceToCenter = Vector2Length(diff);

    if (fabsf(distanceToCenter - radius) > precision) {
        return false;
    }

    float t = getTByPoint(point, precision);
    return t >= 0.0f && t <= 1.0f;
}

Vector2 WallRound::getCenter() {
    return center;
}

float WallRound::getRadius() {
    return radius;
}

float WallRound::getStartAngle() {
    return startAngle;
}

float WallRound::getEndAngle() {
    return endAngle;
}

Wall *Room::closestWall(const Vector2 &point) {
    Wall *closeWall = nullptr;
    float minDist = MAXFLOAT;

    for (Wall *wall : walls) {
        float dist = wall->distanceToWall(point);

        if (dist < minDist && dist < 15) {
            minDist = dist;
            closeWall = wall;
        }
    }

    return closeWall;
}

RayStart *Room::closestRay(const Vector2 &point) {
    if (rayStart && Vector2Distance(point, rayStart->getStart()) < 20) {
        return rayStart;
    }
    return nullptr;
}

Room::Room() {
    points.reserve(maximumPoints + 1);
}

Room::Room(const json &j) {
    if (!j.at("points").is_array() || !j.at("walls").is_array()) {
        throw std::runtime_error("Неверный формат файла");
    }

    points.reserve(maximumPoints + 1);

    if (j.at("points").size() > 0) {
        points.push_back(Point(j.at("points").at(0)));
        size_t i = 0;
        for (const auto &wall : j.at("walls")) {
            if (j.at("points").size() == i + 1) {
                const auto &point = j.at("points").at(0);
                if (wall.at("type") == "line") {
                    addWallLine(Point(point).getCoord());
                } else if (wall.at("type") == "round") {
                    addWallRound(
                        Point(point).getCoord(),
                        wall.at("radiusCoef").get<float>()
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

    if (j.contains("rayStart")) {
        json ray = j["rayStart"];
        addRay(Vector2{ray.at("start").at("x"), ray.at("start").at("y")});
        rayStart->setAngle(ray.at("angle"));
    }

    if (j.contains("aim")) {
        json aim = j["aim"];
        addAim(
            {aim.at("center").at("x"), aim.at("center").at("y")},
            aim.at("radius")
        );
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

bool Room::isClosed() {
    if (walls.size() < 3) {
        return false;
    }

    if (walls[0]->getStart() == walls[walls.size() - 1]->getEnd()) {
        return true;
    }

    return false;
}

WallLine *Room::addWallLine(const Vector2 &coord) {
    int pointsAmount = points.size();

    for (size_t i = 0; i < pointsAmount; ++i) {
        float d = Vector2Distance(points[i].getCoord(), coord);
        if (d < minimalDistance) {
            if (i == 0 && pointsAmount > 2) {
                if (pointsAmount < minimumPoints) {
                    throw Room::TooFewPoints();
                }
                WallLine *wall =
                    new WallLine(&points[pointsAmount - 1], &points[0], this);
                walls.push_back(wall);
                if (rayStart) {
                    rayStart->updateParams();
                }
                return wall;
            } else {
                throw Room::PointsAreTooClose();
            }
        }
    }

    if (pointsAmount >= maximumPoints) {
        throw Room::TooManyPoints();
    }

    // if (walls.size()) {
    //     Vector2 collision = {-1, -1};
    //     for (Wall *wall : walls) {
    //         if (CheckCollisionLines(
    //                 wall->getStart()->getCoord(), wall->getEnd()->getCoord(),
    //                 points[pointsAmount - 1].getCoord(), coord, &collision
    //             ) &&
    //             collision.x != points[pointsAmount - 1].getX() &&
    //             collision.y != points[pointsAmount - 1].getY()) {
    //             throw Room::WallsCollision();
    //         }
    //     }
    // }

    points.push_back(coord);
    ++pointsAmount;
    if (pointsAmount > 1) {
        WallLine *wall = new WallLine(
            &points[pointsAmount - 2], &points[pointsAmount - 1], this
        );
        walls.push_back(wall);
        if (rayStart) {
            rayStart->updateParams();
        }
        return wall;
    }

    return nullptr;
}

WallRound *Room::addWallRound(const Vector2 &coord, float radiusCoef) {
    int pointsAmount = points.size();

    for (size_t i = 0; i < pointsAmount; ++i) {
        float d = Vector2Distance(points[i].getCoord(), coord);
        if (d < minimalDistance) {
            if (i == 0 && pointsAmount > 2) {
                if (pointsAmount < minimumPoints) {
                    throw Room::TooFewPoints();
                }
                WallRound *wall = new WallRound(
                    &points[pointsAmount - 1], &points[0], this, radiusCoef
                );
                walls.push_back(wall);
                if (rayStart) {
                    rayStart->updateParams();
                }
                return wall;
            } else {
                throw Room::PointsAreTooClose();
            }
        }
    }

    if (pointsAmount >= maximumPoints) {
        throw Room::TooManyPoints();
    }

    // if (walls.size()) {
    //     Vector2 collision = {-1, -1};
    //     for (Wall *wall : walls) {
    //         if (CheckCollisionLines(
    //                 wall->getStart()->getCoord(), wall->getEnd()->getCoord(),
    //                 points[pointsAmount - 1].getCoord(), coord, &collision
    //             ) &&
    //             collision.x != points[pointsAmount - 1].getX() &&
    //             collision.y != points[pointsAmount - 1].getY()) {
    //             throw Room::WallsCollision();
    //         }
    //     }
    // }

    points.push_back(coord);
    ++pointsAmount;
    if (pointsAmount > 1) {
        WallRound *wall = new WallRound(
            &points[pointsAmount - 2], &points[pointsAmount - 1], this,
            radiusCoef
        );
        walls.push_back(wall);
        if (rayStart) {
            rayStart->updateParams();
        }
        return wall;
    }

    return nullptr;
}

Wall *Room::changeWallType(Wall *wall) {
    bool updateRay = rayStart ? (rayStart->getWall() == wall) : false;
    RayStart *ray = rayStart ? rayStart : nullptr;
    if (updateRay) {
        rayStart = nullptr;
    }

    size_t index = 0;
    for (size_t i = 0; i < walls.size(); ++i) {
        if (wall == walls[i]) {
            index = i;
            break;
        }
    }

    Point *start = wall->getStart();
    Point *end = wall->getEnd();
    bool isRound = dynamic_cast<WallRound *>(wall) != nullptr;

    Wall *bind = wall;

    if (isRound) {
        wall = new WallLine(start, end, this);
    } else {
        wall = new WallRound(start, end, this);
    }
    delete bind;
    walls[index] = wall;

    if (updateRay) {
        rayStart = ray;
        rayStart->setWall(wall);
    }

    if (rayStart) {
        rayStart->updateParams();
    }

    return wall;
}

void Room::movePoint(Point &p, const Vector2 &coord) {
    p.setCoord(coord);
}

void Room::draw() {
    if (aim) {
        aim->draw();
    }

    for (Wall *wall : walls) {
        wall->draw();
    }

    for (Point &point : points) {
        point.draw();
    }

    if (rayStart) {
        rayStart->draw();
    }
}

json Room::toJson() {
    json j = {
        {"points", json::array()},
        {"walls", json::array()},
    };

    if (rayStart) {
        j["rayStart"] = rayStart->toJson();
    }

    for (Point &point : points) {
        j["points"].push_back(point.toJson());
    }

    for (Wall *wall : walls) {
        j["walls"].push_back(wall->toJson());
    }
    return j;
}

void Room::addRay(const Vector2 &point) {
    Wall *closestWall = Room::closestWall(point);
    if (closestWall) {
        Vector2 closestPoint = closestWall->closestPoint(point);
        if (rayStart) {
            delete rayStart;
        }
        rayStart = new RayStart(closestPoint, closestWall, defaultRayAngle);
    }
}

vector<Wall *> &Room::getWalls() {
    return walls;
}

void Room::clear() {
    points.clear();
    for (Wall *wall : walls) {
        delete wall;
    }
    walls.clear();
    delete rayStart;
    rayStart = nullptr;
}

Room::~Room() {
    for (Wall *wall : walls) {
        delete wall;
    }
}

void Room::addAim(const Vector2 &center, float radius) {
    if (aim) {
        delete aim;
    }
    aim = new AimArea(center, radius);
}

void Room::moveAim(const Vector2 &newCenter) {
    if (aim) {
        aim->setCenter(newCenter);
    }
}

bool Room::isRayInAim(
    const Vector2 &rayStart, const Vector2 &rayEnd, Vector2 &intersectionPoint
) {
    if (!aim) {
        return false;
    }
    return aim->intersectsWithRay(rayStart, rayEnd, intersectionPoint);
}
