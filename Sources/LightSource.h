#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Transform.h"

class LightSource : public Transform {
public:
	inline const glm::vec3 & getDirection () const { return direction; }
	inline const glm::vec3 & getColor () const { return m_color; }
	inline void setColor (const glm::vec3 & color) { m_color = color; }
	inline float getIntensity () const { return m_intensity; }
	inline void setIntensity (float intensity) { m_intensity = intensity; }

private:
	glm::vec3 direction = glm::vec3(0.f, 0.f, -1.f);
	glm::vec3 m_color = glm::vec3 (0.f, 0.f, 0.f);
	float m_intensity = 1.f;
};