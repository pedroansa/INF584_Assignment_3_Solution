#pragma once

#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <sstream>

class FboShadowMap {
public:
    GLuint getTextureId() const { return _depthMapTexture; }

   bool allocate(unsigned int width=1024, unsigned int height=768)
{
    glGenFramebuffers(1, &_depthMapFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _depthMapFbo);

    _depthMapTextureWidth = width;
    _depthMapTextureHeight = height;

    // Variance shadow mapping texture
    glGenTextures(1, &_depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, _depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Attach it as a color attachment
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _depthMapTexture, 0);

    // Create a renderbuffer object for depth attachment
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // Check if FBO is complete
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return true;
    } else {
        std::cout << "PROBLEM IN FBO FboShadowMap::allocate(): FBO NOT successfully created" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }
}


    void bindFbo()
    {
        glViewport(0, 0, _depthMapTextureWidth, _depthMapTextureHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, _depthMapFbo);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void free() { glDeleteFramebuffers(1, &_depthMapFbo); }

    // The savePpmFile function would need significant changes to work with RG32F textures,
    // it might involve reading both channels and then deciding how to visualize them.
    // This part is left as an exercise or optional implementation detail.

private:
    GLuint _depthMapFbo;
    GLuint _depthMapTexture;
    unsigned int _depthMapTextureWidth;
    unsigned int _depthMapTextureHeight;
};