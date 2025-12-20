#include <math.h>
#include <climits>

#include "nlohmann/json_fwd.hpp"
#include "raylib.h"
#include "raymath.h"

#include "Ray.h"

RaySegment::RaySegment(const Vector2 &start, const Vector2 &end, int depth):
    start(start),
    end(end),
    hasHit(false),
    hitWall(nullptr),
    next(nullptr),
    depth(depth) {}

RaySegment::~RaySegment() {
    if (next) {
        delete next;
    }
}

void RaySegment::draw() const {
    DrawLineEx(start, hasHit ? hitPoint : end, 4.0f, ORANGE);
    if (next) {
        next->draw();
    }
}

Vector2 RaySegment::intersectionWithWallLine(WallLine *wall) {
    Vector2 wallStart = wall->getStart()->getCoord();
    Vector2 wallEnd = wall->getEnd()->getCoord();

    float denominator = (start.x - end.x) * (wallStart.y - wallEnd.y) -
                        (start.y - end.y) * (wallStart.x - wallEnd.x);

    if (fabsf(denominator) < 0.0001f) {
        return Vector2{NAN, NAN};
    }

    float t = ((start.x - wallStart.x) * (wallStart.y - wallEnd.y) -
               (start.y - wallStart.y) * (wallStart.x - wallEnd.x)) /
              denominator;
    float u = -((start.x - end.x) * (start.y - wallStart.y) -
                (start.y - end.y) * (start.x - wallStart.x)) /
              denominator;

    if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
        return Vector2{
            start.x + t * (end.x - start.x), start.y + t * (end.y - start.y)
        };
    }

    return Vector2{NAN, NAN};
}

Vector2 RaySegment::intersectionWithWallRound(WallRound *wall) {
    Vector2 center = wall->getCenter();
    float radius = wall->getRadius();

    Vector2 d = Vector2Subtract(end, start);
    Vector2 f = Vector2Subtract(start, center);

    float a = Vector2DotProduct(d, d);
    float b = 2 * Vector2DotProduct(f, d);
    float c = Vector2DotProduct(f, f) - radius * radius;

    float D = b * b - 4 * a * c;

    if (D < 0) {
        return Vector2{NAN, NAN};
    }

    D = sqrtf(D);
    float t1 = (-b - D) / (2 * a);
    float t2 = (-b + D) / (2 * a);

    Vector2 closestIntersection = {NAN, NAN};
    float minT = FLT_MAX;

    for (float t : {t1, t2}) {
        if (t >= 0.0001f && t <= 1.0f && t < minT) {
            Vector2 point = Vector2{start.x + t * d.x, start.y + t * d.y};

            if (wall->isPointOnArc(point, 0.1f)) {
                closestIntersection = point;
                minT = t;
            }
        }
    }

    return closestIntersection;
}

void RaySegment::updateParameters(Room *room) {
    Vector2 closestHit = {NAN, NAN};
    float minDist = FLT_MAX;
    Wall *closestWall = nullptr;
    bool hitAimArea = false;

    if (room->aim) {
        Vector2 aimIntersection;
        if (room->isRayInAim(start, end, aimIntersection)) {
            float dist = Vector2Distance(start, aimIntersection);
            if (dist > 0.1f && dist < minDist) {
                minDist = dist;
                closestHit = aimIntersection;
                hitAimArea = true; // Помечаем, что попали в область цели
            }
        }
    }

    for (Wall *wall : room->getWalls()) {
        if (!hitAimArea || Vector2Distance(start, closestHit) > 0) {
            Vector2 intersection = {NAN, NAN};

            WallLine *wallLine = dynamic_cast<WallLine *>(wall);
            WallRound *wallRound = dynamic_cast<WallRound *>(wall);

            if (wallLine) {
                intersection = intersectionWithWallLine(wallLine);
            } else if (wallRound) {
                intersection = intersectionWithWallRound(wallRound);
            }

            if (!isnan(intersection.x) && !isnan(intersection.y)) {
                float dist = Vector2Distance(start, intersection);
                if (dist > 0.5f && dist < minDist) {
                    minDist = dist;
                    closestHit = intersection;
                    closestWall = wall;
                    hitAimArea = false;
                }
            }
        }
    }

    if (!isnan(closestHit.x) && !isnan(closestHit.y)) {
        hasHit = true;
        hitPoint = closestHit;
        hitWall = closestWall;

        if (hitAimArea) {
            return;
        }

        if (hitWall && depth <= hitWall->room->maximumRayDepth) {
            Vector2 normal = hitWall->getNormal(hitPoint);
            Vector2 incident =
                Vector2Normalize(Vector2Subtract(hitPoint, start));

            float dotProduct = Vector2DotProduct(incident, normal);
            Vector2 reflected =
                Vector2Subtract(incident, Vector2Scale(normal, 2 * dotProduct));

            Vector2 nextEnd =
                Vector2Add(hitPoint, Vector2Scale(reflected, 10000.0f));
            next = new RaySegment(hitPoint, nextEnd, depth + 1);
            next->updateParameters(room);
        }
    } else {
        hasHit = false;
    }
}

const char *RayStart::InvalidAngle::what() const noexcept {
    return "Угол может быть от 1 до 179";
}

const char *RayStart::CantStartInCorner::what() const noexcept {
    return "Нельзя разместить начало луча в углу команаты";
}

RayStart::RayStart(
    const Vector2 &point, Wall *wall, float angle = PI / 2, bool inverted
) {
    t = wall->getTByPoint(point);
    if (wall) {
        RayStart::wall = wall;
    }

    RayStart::start = Vector2(point);
    if (start == wall->getStart()->getCoord() ||
        start == wall->getEnd()->getCoord()) {
        throw CantStartInCorner();
    }
    if (angle * RAD2DEG < 1 || angle * RAD2DEG > 179) {
        throw InvalidAngle();
    }
    RayStart::angle = angle;
    RayStart::inverted = inverted;
    updateRaySegments();
}

void RayStart::setAngle(float angle) {
    if (angle * RAD2DEG < 1 || angle * RAD2DEG > 179) {
        throw InvalidAngle();
    }
    RayStart::angle = angle;
    updateRaySegments();
}

void RayStart::setWall(Wall *wall) {
    RayStart::wall = wall;
    updateParams();
}

void RayStart::inverseT() {
    RayStart::t = 1 - t;
    updateParams();
}

void RayStart::inverseDirection() {
    inverted = !inverted;
    updateRaySegments();
}

void RayStart::updateRaySegments() {
    if (ray) {
        delete ray;
    }
    Vector2 normal = wall->getNormal(start);
    if (inverted) {
        normal = Vector2Scale(normal, -1.0f);
    }
    Vector2 rayDir = Vector2Rotate(normal, angle - PI / 2);
    Vector2 rayEnd = Vector2Add(start, Vector2Scale(rayDir, 10000.0f));
    ray = new RaySegment(start, rayEnd, 1);
    ray->updateParameters(wall->room);
}

void RayStart::updateParams() {
    start = wall->getPointByT(t);
    updateRaySegments();
}

json RayStart::toJson() {
    return {
        {"angle", angle},
        {"start", {{"x", start.x}, {"y", start.y}}},
        {"inverted", inverted}
    };
}

void RayStart::draw() {
    DrawCircleV(start, 10, ORANGE);
    ray->draw();
}

RayStart::~RayStart() {
    delete ray;
}

AimArea::AimArea(const Vector2 &center, float radius):
    center(center),
    radius(radius) {}

void AimArea::setCenter(const Vector2 &center) {
    this->center = center;
}

bool AimArea::intersectsWithRay(
    const Vector2 &rayStart, const Vector2 &rayEnd, Vector2 &intersectionPoint
) {
    Vector2 d = Vector2Subtract(rayEnd, rayStart);
    Vector2 f = Vector2Subtract(rayStart, center);

    float a = Vector2DotProduct(d, d);
    float b = 2 * Vector2DotProduct(f, d);
    float c = Vector2DotProduct(f, f) - radius * radius;

    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return false;
    }

    discriminant = sqrtf(discriminant);
    float t1 = (-b - discriminant) / (2 * a);
    float t2 = (-b + discriminant) / (2 * a);

    float t = FLT_MAX;

    if (t1 >= 0 && t1 <= 1 && t1 < t) {
        t = t1;
    }
    if (t2 >= 0 && t2 <= 1 && t2 < t) {
        t = t2;
    }

    if (t < FLT_MAX) {
        intersectionPoint = Vector2{rayStart.x + t * d.x, rayStart.y + t * d.y};
        return true;
    }

    return false;
}

bool AimArea::containsPoint(const Vector2 &point) {
    return Vector2Distance(point, center) <= radius;
}

json AimArea::toJson() {
    return {{"center", {{"x", center.x}, {"y", center.y}}}, {"radius", radius}};
}

void AimArea::draw() {
    DrawCircleV(center, radius, Fade(GREEN, 0.3f));
}
