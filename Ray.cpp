#include <cmath>

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
    float minT = MAXFLOAT;

    for (float t : {t1, t2}) {
        if (t >= 0.0f && t <= 1.0f && t < minT) {
            Vector2 point = Vector2{start.x + t * d.x, start.y + t * d.y};

            Vector2 diff = Vector2Subtract(point, center);
            float angle = atan2f(diff.y, diff.x) * RAD2DEG;

            if (angle < 0) {
                angle += 360.0f;
            }

            float startAngle = wall->getStartAngle();
            float endAngle = wall->getEndAngle();

            float normStart = fmodf(startAngle, 360.0f);
            float normEnd = fmodf(endAngle, 360.0f);
            if (normStart < 0) {
                normStart += 360.0f;
            }
            if (normEnd < 0) {
                normEnd += 360.0f;
            }

            bool isInArc = false;

            if (normEnd >= normStart) {
                isInArc = (angle >= normStart && angle <= normEnd);
            } else {
                isInArc = (angle >= normStart || angle <= normEnd);
            }

            if (isInArc) {
                closestIntersection = point;
                minT = t;
            }
        }
    }

    return closestIntersection;
}

void RaySegment::findIntersections(Room *room, Wall *originWall) {
    Vector2 closestHit = {NAN, NAN};
    float minDist = MAXFLOAT;
    Wall *closestWall = nullptr;

    for (Wall *wall : room->getWalls()) {
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
            if (dist > 0.1f && dist < minDist) {
                minDist = dist;
                closestHit = intersection;
                closestWall = wall;
            }
        }
    }

    if (!isnan(closestHit.x) && !isnan(closestHit.y)) {
        hasHit = true;
        hitPoint = closestHit;
        hitWall = closestWall;

        // Создаем следующий сегмент, если не превышен лимит глубины
        if (depth < 3) {
            Vector2 normal = hitWall->getNormal(hitPoint);
            Vector2 incident =
                Vector2Normalize(Vector2Subtract(hitPoint, start));

            // Отражение: r = d - 2(d·n)n
            float dotProduct = Vector2DotProduct(incident, normal);
            Vector2 reflected =
                Vector2Subtract(incident, Vector2Scale(normal, 2 * dotProduct));

            Vector2 nextEnd =
                Vector2Add(hitPoint, Vector2Scale(reflected, 10000.0f));
            next = new RaySegment(hitPoint, nextEnd, depth + 1);
            next->findIntersections(room, hitWall);
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

RayStart::RayStart(const Vector2 &point, Wall *wall, float angle = PI / 2) {
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

void RayStart::updateRaySegments() {
    if (ray) {
        delete ray;
    }
    Vector2 normal = wall->getNormal(start);
    Vector2 rayDir = Vector2Rotate(normal, angle - PI / 2);
    Vector2 rayEnd = Vector2Add(start, Vector2Scale(rayDir, 10000.0f));
    ray = new RaySegment(start, rayEnd, 0);
    ray->findIntersections(wall->room, wall);
}

void RayStart::updateParams() {
    start = wall->getPointByT(t);
    updateRaySegments();
}

void RayStart::draw() {
    DrawCircleV(start, 10, ORANGE);
    ray->draw();
}

RayStart::~RayStart() {
    delete ray;
}
