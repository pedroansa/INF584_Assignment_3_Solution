#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Transform.h"
#include "FboShadowMap.h"
#include "ShaderProgram.h"

#include <GLFW/glfw3.h>

class LightSource : public Transform {
public:
	inline const glm::vec3 & getDirection () const { return direction; }
	inline const glm::vec3 & getColor () const { return m_color; }
	inline void setColor (const glm::vec3 & color) { m_color = color; }
	inline void setShadowMapTex (const int & shadowMap) { m_shadowMapTexOnGPU = shadowMap; }
	inline int getShadowMapTex () const { return m_shadowMapTexOnGPU; }
	inline float getIntensity () const { return m_intensity; }
	inline void setIntensity (float intensity) { m_intensity = intensity; }
	inline glm::mat4 getDepthMVP() const { return depthMVP ;}
	void allocateShadowMapFbo(unsigned int w=800, unsigned int h=600){ m_shadowMap.allocate(w, h); }
    void bindShadowMap(){ m_shadowMap.bindFbo(); }
	void setupCameraForShadowMapping(std::shared_ptr<ShaderProgram> shader_shadow_map_Ptr, std::shared_ptr<Camera> camera);
	FboShadowMap m_shadowMap;

private:
	glm::vec3 direction = glm::vec3(0.f, 0.f, -1.f);
	glm::vec3 m_color = glm::vec3 (0.f, 0.f, 0.f);
	float m_intensity = 1.f;
	int m_shadowMapTexOnGPU;
	glm::mat4 depthMVP;
};