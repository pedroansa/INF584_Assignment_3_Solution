// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#include "Rasterizer.h"
#include <glad/glad.h>
#include <algorithm>
#include "Resources.h"
#include "Error.h"

bool saveShadowMapsPpm = true;

void Rasterizer::init (const std::string & basePath, const std::shared_ptr<Scene> scenePtr) {
	glCullFace (GL_BACK);     // Specifies the faces to cull (here the ones pointing away from the camera)
	glEnable (GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
	glDepthFunc (GL_LESS); // Specify the depth test for the z-buffer
	glEnable (GL_DEPTH_TEST); // Enable the z-buffer test in the rasterization
	const glm::vec3 & bgColor = scenePtr->backgroundColor ();
	glClearColor (bgColor[0], bgColor[1], bgColor[2], 1.f);
	// GPU resources
	initScreeQuad ();

	loadShaderProgram (basePath);
	initDisplayedImage ();
	// Allocate GPU ressources for the heavy data components of the scene 
	size_t numOfMeshes = scenePtr->numOfMeshes ();
	for (size_t i = 0; i < numOfMeshes; i++) 
		m_vaos.push_back (toGPU (scenePtr->mesh (i)));


}

void Rasterizer::setResolution (int width, int height)  {
	glViewport (0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

void Rasterizer::loadShaderProgram (const std::string & basePath) {
	m_pbrShaderProgramPtr.reset ();
	try {
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_pbrShaderProgramPtr = ShaderProgram::genBasicShaderProgram (shaderPath + "/PBRVertexShader.glsl",
													         	 	  shaderPath + "/PBRFragmentShader.glsl");
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading shader program]") + e.what ());
	}
	m_displayShaderProgramPtr.reset ();
	try {
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_displayShaderProgramPtr = ShaderProgram::genBasicShaderProgram (shaderPath + "/DisplayVertexShader.glsl",
													         	 		  shaderPath + "/DisplayFragmentShader.glsl");
		m_displayShaderProgramPtr->set ("imageTex", 0);
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading display shader program]") + e.what ());
	}

	m_shadowMapingShaderProgramPtr.reset ();
	try {
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_shadowMapingShaderProgramPtr = ShaderProgram::genBasicShaderProgram (shaderPath + "/ShadowMappingVertexShader.glsl",
													         	 		  shaderPath + "/ShadowMappingFragmentShader.glsl");
		//m_displayShaderProgramPtr->set ("imageTex", 0);
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading display shader program]") + e.what ());
	}
}

void Rasterizer::updateDisplayedImageTexture (std::shared_ptr<Image> imagePtr) {
	glBindTexture (GL_TEXTURE_2D, m_displayImageTex);
   	// Uploading the image data to GPU memory
	glTexImage2D (
		GL_TEXTURE_2D, 
		0, 
   		GL_RGB, // We assume only greyscale or RGB pixels
   		static_cast<GLsizei> (imagePtr->width()), 
   		static_cast<GLsizei> (imagePtr->height()), 
   		0, 
   		GL_RGB, // We assume only greyscale or RGB pixels
   		GL_FLOAT, 
   		imagePtr->pixels().data());
   	// Generating mipmaps for filtered texture fetch
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, 0);
}

void Rasterizer::initDisplayedImage () {
	// Creating and configuring the GPU texture that will contain the image to display
	glGenTextures (1, &m_displayImageTex);
	glBindTexture (GL_TEXTURE_2D, m_displayImageTex);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture (GL_TEXTURE_2D, 0);
}

void printMat4(const glm::mat4& matrix) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

void printVec4(const glm::vec3& vector) {
    std::cout << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")" << std::endl;
}

void Draw(std::shared_ptr<ShaderProgram> shader, std::shared_ptr<Scene> scenePtr, std::vector<GLuint> m_vaos){
	//CHANGE TO INCLUDE MORE MESHES
	// Bind shader to be able to access uniforms
	glUseProgram(shader->id());
	glBindVertexArray (m_vaos[0]);

	glDrawElements(GL_TRIANGLES, scenePtr->mesh (0)->triangleIndices().size(), GL_UNSIGNED_INT, 0);

}

void LightSource::setupCameraForShadowMapping(
    std::shared_ptr<ShaderProgram> shader_shadow_map_Ptr,
    const glm::vec3 scene_center,
	const float scene_radius)
  {
	float near_plane = 0.1f, far_plane = 10.0f;
	float s = 2.f * scene_radius;
	glm::mat4 depthProjectionMatrix = glm::ortho(-s, s, -s, s, 0.1f, s); 
    glm::mat4 depthViewMatrix = glm::lookAt(getTranslation(), scene_center, glm::vec3(0.0, 1.0, 0.0));  
	shader_shadow_map_Ptr->set("depthMVP", depthProjectionMatrix * depthViewMatrix * glm::mat4(1.0f));
  }

// The main rendering call
void Rasterizer::render (std::shared_ptr<Scene> scenePtr) {
	const auto & lightSources = scenePtr->lightSources();
	int numOfLightSources = std::min (8, int (lightSources.size ()));

	glEnable(GL_DEPTH_TEST);
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	m_shadowMapingShaderProgramPtr->use();

	
	for(size_t i = 0; i < scenePtr->lightSources().size(); ++i) {
      LightSource li = lightSources[i];
	  
      li.setupCameraForShadowMapping(m_shadowMapingShaderProgramPtr, glm::vec3(0), 1.f);
	  li.bindShadowMap();

      // TODO: render the objects in the scene
      m_shadowMapingShaderProgramPtr->set("model", glm::mat4(1.0f));
      draw (0, scenePtr->mesh (0)->triangleIndices().size ());
	  if(saveShadowMapsPpm) {
		std::cout << "Saving Shadow Map for Light " << i << std::endl;
        li.m_shadowMap.savePpmFile(std::string("shadom_map_")+std::to_string(i)+std::string(".ppm"));
      }
	  
    }
	saveShadowMapsPpm = false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1024, 768);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
    glCullFace(GL_BACK);


	// const glm::vec3 & bgColor = scenePtr->backgroundColor ();
	// glClearColor (bgColor[0], bgColor[1], bgColor[2], 1.f);

	m_pbrShaderProgramPtr->set ("numOfLightSources", numOfLightSources);
	for (size_t i = 0; i < numOfLightSources; ++i) {
		const auto & li = lightSources[i];
		std::string lstring = "lightSourceSet[" + std::to_string (i) + "]";
		m_pbrShaderProgramPtr->set (lstring + ".position", li.getTranslation ());
		m_pbrShaderProgramPtr->set (lstring + ".color", li.getColor ());
		m_pbrShaderProgramPtr->set (lstring + ".intensity", li.getIntensity ());
		m_pbrShaderProgramPtr->set(std::string("shadowMap") + std::to_string(i), li.getShadowMapTex());
      	m_pbrShaderProgramPtr->set(std::string("shadowMVP[") + std::to_string(i) + std::string("]"), li.getDepthMVP());
	}
	
	glm::mat4 viewMatrix = scenePtr->camera()->computeViewMatrix ();
	m_pbrShaderProgramPtr->set ("viewMat", viewMatrix);
	glm::mat4 projectionMatrix = scenePtr->camera()->computeProjectionMatrix ();
	m_pbrShaderProgramPtr->set ("projectionMat", projectionMatrix); // Compute the projection matrix of the camera and pass it to the GPU program

	size_t numOfMeshes = scenePtr->numOfMeshes ();
	for (size_t i = 0; i < numOfMeshes; i++) {
		// Passing mesh-specific matrices
		glm::mat4 modelMatrix = scenePtr->mesh (i)->computeTransformMatrix ();
		glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
		glm::mat4 normalMatrix = glm::transpose (glm::inverse (modelViewMatrix));

		m_pbrShaderProgramPtr->set ("modelMat", modelMatrix);
		m_pbrShaderProgramPtr->set ("modelViewMat", modelViewMatrix);
		m_pbrShaderProgramPtr->set ("normalMat", normalMatrix);

		// Passing material
		m_pbrShaderProgramPtr->set ("material.albedo", scenePtr->mesh (i)->material ().getAlbedo());
		m_pbrShaderProgramPtr->set ("material.roughness", scenePtr->mesh (i)->material ().getRoughness());
		m_pbrShaderProgramPtr->set ("material.metallicness", scenePtr->mesh (i)->material ().getMetallicness());
		float aux = (1.0 - scenePtr->mesh (i)->material ().getMetallicness());
		//printVec4(aux  * scenePtr->mesh (i)->material ().getAlbedo());
		
		draw (i, scenePtr->mesh (i)->triangleIndices().size ());

	}
	m_pbrShaderProgramPtr->stop ();

	// unsigned int shadowMapWidth = 2048, shadowMapHeight = 2048;
	// glViewport(0, 0, shadowMapWidth, shadowMapHeight);
	// glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//glClear(GL_DEPTH_BUFFER_BIT);

	//Draw (m_shadowMapingShaderProgramPtr, scenePtr, m_vaos);
	
}

void Rasterizer::display (std::shared_ptr<Image> imagePtr) {
	updateDisplayedImageTexture (imagePtr);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
	m_displayShaderProgramPtr->use (); // Activate the program to be used for upcoming primitive
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, m_displayImageTex);
	glBindVertexArray (m_screenQuadVao); // Activate the VAO storing geometry data
	glDrawElements (GL_TRIANGLES, static_cast<GLsizei> (6), GL_UNSIGNED_INT, 0);
	m_displayShaderProgramPtr->stop ();
}

void Rasterizer::clear () {
	for (unsigned int i = 0; i < m_posVbos.size (); i++) {
		GLuint vbo = m_posVbos[i];
		glDeleteBuffers (1, &vbo);
	}
	m_posVbos.clear ();
	for (unsigned int i = 0; i < m_normalVbos.size (); i++) {
		GLuint vbo = m_normalVbos[i];
		glDeleteBuffers (1, &vbo);
	}
	m_normalVbos.clear ();
	for (unsigned int i = 0; i < m_ibos.size (); i++) {
		GLuint ibo = m_ibos[i];
		glDeleteBuffers (1, &ibo);
	}
	m_ibos.clear ();
	for (unsigned int i = 0; i < m_vaos.size (); i++) {
		GLuint vao = m_vaos[i];
		glDeleteVertexArrays (1, &vao);
	}
	m_vaos.clear ();
}

GLuint Rasterizer::genGPUBuffer (size_t elementSize, size_t numElements, const void * data) {
	GLuint vbo;
	glGenBuffers (1, &vbo); // Generate a GPU buffer to store the positions of the vertices
	size_t size = elementSize * numElements; // Gather the size of the buffer from the CPU-side vector
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	return vbo;
}

GLuint Rasterizer::genGPUVertexArray (GLuint posVbo, GLuint ibo, bool hasNormals, GLuint normalVbo) {
	GLuint vao;
	glGenVertexArrays (1, &vao); // Create a single handle that joins together attributes (vertex positions, normals) and connectivity (triangles indices)
	glBindVertexArray (vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, posVbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
	if (hasNormals) {
		glEnableVertexAttribArray (1);
		glBindBuffer (GL_ARRAY_BUFFER, normalVbo);
		glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
	}
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindVertexArray (0); // Desactive the VAO just created. Will be activated at rendering time.
	return vao;
}


GLuint Rasterizer::toGPU (std::shared_ptr<Mesh> meshPtr) {
	GLuint posVbo = genGPUBuffer (3 * sizeof (float), meshPtr->vertexPositions().size(), meshPtr->vertexPositions().data ()); // Position GPU vertex buffer
	GLuint normalVbo = genGPUBuffer (3 * sizeof (float), meshPtr->vertexNormals().size(), meshPtr->vertexNormals().data ()); // Normal GPU vertex buffer
	GLuint ibo = genGPUBuffer (sizeof (glm::uvec3), meshPtr->triangleIndices().size(), meshPtr->triangleIndices().data ()); // triangle GPU index buffer
	GLuint vao = genGPUVertexArray (posVbo, ibo, true, normalVbo);
	return vao;
}

void Rasterizer::initScreeQuad () {
	std::vector<float> pData = {-1.0, -1.0, 0.0, 1.0, -1.0, 0.0, 1.0, 1.0, 0.0, -1.0, 1.0, 0.0};
	std::vector<unsigned int> iData = {0, 1, 2, 0, 2, 3};
	m_screenQuadVao = genGPUVertexArray (
		genGPUBuffer (3*sizeof(float), 4, pData.data()),
		genGPUBuffer (3*sizeof(unsigned int), 2, iData.data()),
		false,
		0
	);
}

void Rasterizer::draw (size_t meshId, size_t triangleCount) {

	glBindVertexArray (m_vaos[meshId]); // Activate the VAO storing geometry data
	glDrawElements (GL_TRIANGLES, static_cast<GLsizei> (triangleCount * 3), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
}
