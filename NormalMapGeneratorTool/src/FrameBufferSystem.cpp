#include "FrameBufferSystem.h"
#include <iostream>
#include <GL\glew.h>
unsigned int FrameBufferSystem::currentlyBoundFBO;
FrameBufferSystem::FrameBufferSystem(){}

void FrameBufferSystem::init(const glm::vec2 & windowRes)
{
	init(windowRes.x, windowRes.y);
}

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 1920, 1080, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textureDepthBuffer, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferSystem::BindFrameBuffer()
{
	currentlyBoundFBO = framebuffer;
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
	/*if(currentlyBoundFBO != framebuffer)
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);*/
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, textureDepthBuffer);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, windowWidth, windowHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	//glBindFramebuffer(GL_FRAMEBUFFER, currentlyBoundFBO);
}

int FrameBufferSystem::GetCurrentlyBoundFBO()
{
	return currentlyBoundFBO;
}

FrameBufferSystem::~FrameBufferSystem()
{
}
