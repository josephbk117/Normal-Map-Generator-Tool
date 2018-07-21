#include "FrameBufferSystem.h"
#include<iostream>
#include <GL\glew.h>

FrameBufferSystem::FrameBufferSystem()
{}

void FrameBufferSystem::init(int windowWidth, int windowHeight)
{
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenTextures(1, &textureDepthBuffer);
	glBindTexture(GL_TEXTURE_2D, textureDepthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, windowWidth, windowHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textureDepthBuffer, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferSystem::BindFrameBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void FrameBufferSystem::BindBufferTexture()
{
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
}

unsigned int FrameBufferSystem::getBufferTexture()
{
	return textureColorbuffer;
}

void FrameBufferSystem::updateTextureDimensions(int windowWidth, int windowHeight)
{
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}


FrameBufferSystem::~FrameBufferSystem()
{
}
