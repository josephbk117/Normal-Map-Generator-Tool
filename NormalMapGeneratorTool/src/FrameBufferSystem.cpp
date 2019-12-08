#include "FrameBufferSystem.h"
#include <iostream>
#include <GL\glew.h>
unsigned int FrameBufferSystem::currentlyBoundFBO;
FrameBufferSystem::FrameBufferSystem() {}

void FrameBufferSystem::init(const glm::ivec2 & windowRes, const glm::ivec2& maxBufferResolution)
{
	init(windowRes.x, windowRes.y, maxBufferResolution.x, maxBufferResolution.y);
}

void FrameBufferSystem::init(int windowWidth, int windowHeight, int maxBufferWidth, int maxBufferHeight)
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, maxBufferWidth, maxBufferHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textureDepthBuffer, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferSystem::bindFrameBuffer() const noexcept
{
	currentlyBoundFBO = framebuffer;
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void FrameBufferSystem::bindColourTexture() noexcept
{
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
}

unsigned int FrameBufferSystem::getColourTexture()const noexcept
{
	return textureColorbuffer;
}

void FrameBufferSystem::bindDepthTexture() noexcept
{
	glBindTexture(GL_TEXTURE_2D, textureDepthBuffer);
}

unsigned int FrameBufferSystem::getDepthTexture()const noexcept
{
	return textureDepthBuffer;
}

void FrameBufferSystem::updateTextureDimensions(int windowWidth, int windowHeight) noexcept
{
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBufferSystem::updateTextureDimensions(const glm::ivec2 & windowRes) noexcept
{
	updateTextureDimensions(windowRes.x, windowRes.y);
}

unsigned int FrameBufferSystem::getFrameBufferId() const
{
	return framebuffer;
}

unsigned int FrameBufferSystem::getCurrentlyBoundFBO() noexcept
{
	return currentlyBoundFBO;
}

void FrameBufferSystem::blit(const FrameBufferSystem& source, const FrameBufferSystem& destination,
	const glm::ivec2& srcStartCoord, const glm::ivec2& srcEndCoord, const glm::ivec2& destStartCoord,
	const glm::ivec2& destEndCoord) noexcept
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination.getFrameBufferId());
	glBindFramebuffer(GL_READ_FRAMEBUFFER, source.getFrameBufferId());
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(srcStartCoord.x, srcStartCoord.y, srcEndCoord.x, srcEndCoord.y,
					  destStartCoord.x, destStartCoord.y, destEndCoord.x, destEndCoord.y,
					  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, currentlyBoundFBO);
}

void FrameBufferSystem::blit(const FrameBufferSystem& source, const FrameBufferSystem& destination, const glm::ivec2 screenRes)noexcept
{
	blit(source, destination, glm::ivec2(0, 0), screenRes, glm::ivec2(0, 0), screenRes);
}
void FrameBufferSystem::bindDefaultFrameBuffer()noexcept
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
FrameBufferSystem::~FrameBufferSystem()
{
}
