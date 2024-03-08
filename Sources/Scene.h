// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "Camera.h"
#include "Mesh.h"
#include "LightSource.h"

class Scene {
public:
	inline Scene () : m_backgroundColor (0.f, 0.f ,0.f) {}
	virtual ~Scene() {}

	inline const glm::vec3 & backgroundColor () const { return m_backgroundColor; }

	inline void setBackgroundColor (const glm::vec3 & color) { m_backgroundColor = color; }
 
	inline void set (std::shared_ptr<Camera> camera) { m_camera = camera; }

	inline const std::shared_ptr<Camera> camera() const { return m_camera; }

	inline std::shared_ptr<Camera> camera() { return m_camera; }

	inline void add (std::shared_ptr<Mesh> mesh) { m_meshes.push_back (mesh); }

	inline size_t numOfMeshes () const { return m_meshes.size (); }

	inline const std::shared_ptr<Mesh> mesh (size_t index) const { return m_meshes[index]; }

	inline std::shared_ptr<Mesh> mesh (size_t index) { return m_meshes[index]; }

	inline std::vector<LightSource> & lightSources () { return m_lightSources; }

	inline const std::vector<LightSource> & lightSources () const { return m_lightSources; }

	inline const LightSource & lightSource (int index) const { return m_lightSources[index]; }

	inline void clear () {
		m_camera.reset ();
		m_meshes.clear ();
		m_lightSources.clear ();
	}

private:
	glm::vec3 m_backgroundColor;
	std::shared_ptr<Camera> m_camera;
	std::vector<std::shared_ptr<Mesh> > m_meshes;
	std::vector<LightSource> m_lightSources;
};