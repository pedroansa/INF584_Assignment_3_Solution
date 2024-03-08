#include "Hit.h"

Hit::Hit(glm::vec3 o, glm::vec3 dir, float t)
{
    lightSourcePosition = o;
    lightSourceDirection = dir;
    hitPoint = o + dir * t;
}