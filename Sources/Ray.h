#include <iostream>
#include <glm/glm.hpp>
#include "Hit.h"

class Ray {
public:
    
    Ray(const glm::vec3& origin, const glm::vec3& direction)
        : origin(origin), direction(glm::normalize(direction)) {}

    bool rayTriangleIntersection(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, Hit& hit, bool aux) ;
    glm::vec3 origin;
    glm::vec3 direction;
};