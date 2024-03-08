#include "Ray.h"

constexpr float epsilon = 1e-6;


bool Ray::rayTriangleIntersection(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, Hit& hit, bool aux) {
    glm::vec3 e0 = p1 - p0;
    glm::vec3 e1 = p2 - p0;
    glm::vec3 n = glm::normalize(glm::cross(e0, e1));
    
    glm::vec3 q = glm::cross(this->direction, e1);
    float a = glm::dot(e0, q);

    if (fabs(a) < epsilon)
        return false; // No intersection

    glm::vec3 s = (this->origin - p0);
    glm::vec3 r = glm::cross(s, e0);

    float b0 = glm::dot(s, q)/a;
    float b1 = glm::dot(r, this->direction)/a;
    float t = glm::dot(e1, r)/ a;

    hit.setHitPoint(this->origin + t * this->direction);
    hit.u = b0;
    hit.v = b1;
    hit.t = t;

    if (b0 < 0.f || b0 > 1.f)
		return false;
	if (b1 >= 0.f && b1 + b0 <= 1.f)
		return true;
	return false;
}