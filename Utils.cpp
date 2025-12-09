#include "raylib.h"
#include <cmath>

float distance(Vector2 a, Vector2 b) {
    float dy = a.x - b.x;
    float dx = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}
