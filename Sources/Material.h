#pragma once

class Material {
public:
	inline const glm::vec3 & getAlbedo () const { return m_albedo; }
	inline void setAlbedo (const glm::vec3 & albedo) { m_albedo = albedo; }
	inline float getRoughness () const { return m_roughness; }
	inline void setRoughness (float roughness) { m_roughness = roughness; }
	inline float getMetallicness () const { return m_metallicness; }
	inline void setMetallicness (float metallicness) { m_metallicness = metallicness; }

private:
	glm::vec3 m_albedo = glm::vec3 (0.5f, 0.5f, 0.5f);
	float m_roughness = 0.01f;
	float m_metallicness = 0.f;
};