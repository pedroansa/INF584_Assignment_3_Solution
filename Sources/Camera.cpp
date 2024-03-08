// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#include "Camera.h"

 Ray Camera::rayAt(float x, float y) const{
        // Convert pixel coordinates to normalized device coordinates (NDC)
        glm::mat4 viewMat = inverse (computeViewMatrix()); // View Mat with Right Up Front and position
        glm::vec3 viewRight = normalize (glm::vec3 (viewMat[0]));
        glm::vec3 viewUp = normalize (glm::vec3 (viewMat[1]));
        glm::vec3 viewFront = -normalize (glm::vec3 (viewMat[2]));
        glm::vec3 eye = glm::vec3 (viewMat[3]);

        float w = 2.0*float (tan (glm::radians (m_fov/2.0)));
        glm::vec3 rayDir = normalize (viewFront + ((x - 0.5f) * m_aspectRatio * w) * viewRight + ((1.f-y) - 0.5f) * w * viewUp); 
        return Ray (eye, rayDir);
    }