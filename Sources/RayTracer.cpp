// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#include "RayTracer.h"
#include <algorithm>

#include "Console.h"
#include "Camera.h"
#include "Hit.h"

RayTracer::RayTracer() : 
	m_imagePtr (std::make_shared<Image>()) {}

RayTracer::~RayTracer() {}

void RayTracer::init (const std::shared_ptr<Scene> scenePtr) {
}


bool rayTriangleIntersection(glm::mat4 modelMatrix, Ray ray, const std::vector<glm::vec3>& vertexPositions, const glm::uvec3& triangleIndices, Hit& hit) {
	const glm::vec3& p0 = glm::vec3(modelMatrix * glm::vec4(vertexPositions[triangleIndices.x], 1.0));
    const glm::vec3& p1 = glm::vec3(modelMatrix * glm::vec4(vertexPositions[triangleIndices.y], 1.0));
    const glm::vec3& p2 = glm::vec3(modelMatrix * glm::vec4(vertexPositions[triangleIndices.z], 1.0));

	return ray.rayTriangleIntersection(p0, p1, p2, hit, false);
}


void printVec3(const glm::vec3& vector) {
    std::cout << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")" << std::endl;
}

glm::vec3 barycentricInterpolation (const glm::vec3 & p0, const glm::vec3 & p1, const glm::vec3 & p2, float w, float u, float v)  {	return w * p0 + u * p1 + v * p2; }

void printMat41(const glm::mat4& matrix) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

glm::vec3 attenuation (LightSource l, glm::vec3 lightPosition, glm::vec3 p){
	float d = distance (lightPosition, p);
	return l.getIntensity() * l.getColor()/(d*d);
}
glm::vec3 diffuseBRDF(Material material){
	float aux = 1.0 - material.getMetallicness();
	aux /= PI;
	return aux * (material.getAlbedo());
}

float D_GGX(float NoH, float a) {
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * f * f);
}

glm::vec3 F_Schlick(float u, glm::vec3 f0) {
	return f0 + ((glm::vec3(1.0f) - f0) * pow(1.0f - u, 5.0f));
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
    float a2 = a * a;
    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (GGXV + GGXL);
}


glm::vec3 microfacetBRDF(Material m, glm::vec3 normal, glm::vec3 wo, glm::vec3 wi){
	glm::vec3 h = normalize(wo + wi);
    float NoV = abs(dot(normal, wo)) + 1e-5;
    float NoL = clamp(dot(normal, wi), 0.0f, 1.0f);
    float NoH = clamp(dot(normal, h), 0.0f, 1.0f);
    float LoH = clamp(dot(wi, h), 0.0f, 1.0f);

    // perceptually linear roughness to roughness (see parameterization)
    float roughness = m.getRoughness() * m.getRoughness();

    float D = D_GGX(NoH, roughness);
    float reflectance  = 1.0;
    glm::vec3 f0 = 0.16f * reflectance * reflectance * (1.0f - m.getMetallicness()) + m.getAlbedo() * m.getMetallicness();
    glm::vec3  F = F_Schlick(LoH, f0);
    float V = V_SmithGGXCorrelated(NoV, NoL, roughness);
    return D * V * F;
}

glm::vec3 shade (const std::shared_ptr<Scene> scenePtr, Ray ray, Hit hit) {
	const auto mesh = scenePtr->mesh(hit.getMesh());
	//const std::shared_ptr<Material> materialPtr = scenePtr->material(scenePtr->mesh2material(hit.m_meshIndex));
	const auto& P = mesh->vertexPositions();
	const auto& N = mesh->vertexNormals();
	const glm::uvec3 & triangle = mesh->triangleIndices()[hit.getSimp()];

	float w = 1.f - hit.u - hit.v;

	glm::mat4 modelMatrix = mesh->computeTransformMatrix ();
	glm::mat4 viewMatrix = scenePtr->camera()->computeViewMatrix ();
	glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;

	glm::vec3 hitPoint = glm::vec3 (modelViewMatrix * glm::vec4 (hit.getHitPoint(), 1.0));
	glm::vec3 unormalizedHitNormal = barycentricInterpolation(N[triangle[0]], N[triangle[1]], N[triangle[2]], w, hit.u, hit.v);
	glm::mat4 normalMatrix = glm::transpose (glm::inverse (modelViewMatrix));
	glm::vec3 hitNormal = normalize (glm::vec3 (normalMatrix * glm::vec4 (normalize (unormalizedHitNormal), 1.0)));

	glm::vec3 wo = normalize(-ray.direction);
	glm::vec3 colorResponse (0.f, 0.f, 0.f);

	for (size_t i = 0; i < scenePtr->lightSources().size(); ++i) {
		LightSource light = scenePtr->lightSource(i);
		glm::vec3 lightPosition = glm::vec3(viewMatrix * glm::vec4(light.getTranslation(), 1.0));
		glm::vec3 wi = normalize(lightPosition - hitPoint);
		glm::vec3 li = attenuation (light, lightPosition, hitPoint);
		glm::vec3 fd = diffuseBRDF(mesh->material());
		glm::vec3 fs = microfacetBRDF (mesh->material(), hitNormal, wo, wi);
		glm::vec3 fr = fd+fs;

		float wiDotN = max(0.f, dot(wi, hitNormal));
		if (wiDotN <= 0.f)
			continue;

		colorResponse += li * fr * wiDotN;
	}
	return colorResponse;
}


glm::vec3 PerPixel (const std::shared_ptr<Scene> scenePtr, Ray ray) {
	glm::mat4 viewMat = scenePtr->camera()->computeViewMatrix();
	glm::vec3 cameraPosition = glm::vec3(viewMat[3]);
	bool found = false;
	bool target = false;
	float minimun = std::numeric_limits<float>::infinity();
	Hit lastHit = {glm::vec3(1.0), glm::vec3(1.0), minimun};

	for(int m = 0; m < scenePtr->numOfMeshes(); m++){
			const auto& positions = scenePtr->mesh(m)->vertexPositions();
			const auto& indices = scenePtr->mesh(m)->triangleIndices();
			glm::mat4 modelMatrix = scenePtr->mesh(m)->computeTransformMatrix ();
			glm::mat4 normalMatrix = glm::transpose (glm::inverse (modelMatrix));
			
			for(int simp = 0; simp < indices.size(); simp++){
				bool target = false;
				Hit actualHit = {glm::vec3(1.0), glm::vec3(1.0), minimun};
				target = rayTriangleIntersection(modelMatrix, ray, positions, indices[simp], actualHit);
				
				if(target && actualHit.t < minimun){
					found = true;
					minimun = actualHit.t;
					actualHit.setMesh(m);
					actualHit.setSimp(simp);
					lastHit = actualHit;
					//return (glm::vec3(1.0));
				}

			}
	}

	if(found && lastHit.t > 0.0f){
		//std::cout << lastHit.u << std::endl;
		return shade(scenePtr, ray, lastHit);
	}

	else{
		return scenePtr->backgroundColor();
	}

}

void RayTracer::render (const std::shared_ptr<Scene> scenePtr) {
	size_t width = m_imagePtr->width();
	size_t height = m_imagePtr->height();
	std::chrono::high_resolution_clock clock;
	Console::print ("Start ray tracing at " + std::to_string (width) + "x" + std::to_string (height) + " resolution...");
	std::chrono::time_point<std::chrono::high_resolution_clock> before = clock.now();
	m_imagePtr->clear (scenePtr->backgroundColor ());

	glm::mat4 viewMat = scenePtr->camera()->computeViewMatrix();
	// Camera Position in the world
	glm::vec3 cameraPosition = glm::vec3(viewMat[3]);
	bool aux = false;
	// <---- Ray tracing code ---->
	for(float j = 0; j < height; j++){
		for(float i = 0; i < width; i++){
			glm::vec3 color (0.f, 0.f, 0.f);
			Ray ray = scenePtr->camera()->rayAt((float(i) + 0.5) / width, 1.f - (float(j) + 0.5) / height);
			m_imagePtr->operator()(i, j) = PerPixel(scenePtr, ray);
		}
		
	}


	std::chrono::time_point<std::chrono::high_resolution_clock> after = clock.now();
	double elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
	Console::print ("Ray tracing executed in " + std::to_string(elapsedTime) + "ms");
}