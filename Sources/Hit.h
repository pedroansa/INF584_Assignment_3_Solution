#include <glm/glm.hpp>

#ifndef HIT_H
#define HIT_H

class Hit {
public:
    Hit(glm::vec3 o, glm::vec3 dir, float t);

    glm::vec3 getLightSourcePosition() const { return lightSourcePosition; }
    glm::vec3 getLightSourceDirection() const { return lightSourceDirection; }
    glm::vec3 getHitPoint() const { return hitPoint; }
    int getMesh() const { return meshIndex; }
    int getSimp() const { return simpIndex; }

    void setLightSourcePosition(const glm::vec3& position) { lightSourcePosition = position; }
    void setLightSourceDirection(const glm::vec3& direction) { lightSourceDirection = direction; }
    void setHitPoint(const glm::vec3& point) { hitPoint = point; }
    void setMesh(int index) {meshIndex = index;}
    void setSimp(int index) {simpIndex = index;}

    float distanceFromCamera(const glm::vec3& cameraPosition) const {
        return glm::distance(cameraPosition, hitPoint);
    }
    float u, v, t;

private:
    glm::vec3 lightSourcePosition;
    glm::vec3 lightSourceDirection;
    glm::vec3 hitPoint;
    int meshIndex = -1;
    int simpIndex = -1;
};

#endif // HIT_H