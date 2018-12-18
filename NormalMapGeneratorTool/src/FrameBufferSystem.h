#pragma once
#include <GLM\common.hpp>
class FrameBufferSystem
{
private:
	unsigned int framebuffer = 0;
	unsigned int textureColorbuffer = 0;
	unsigned int textureDepthBuffer = 0;
	static unsigned int currentlyBoundFBO;
public:
	FrameBufferSystem();
	void init(const glm::vec2 &windowRes);
	void init(int windowWidth, int windowHeight);
	void BindFrameBuffer() noexcept;
	void BindBufferTexture() noexcept;
	unsigned int getBufferTexture() noexcept;
	void updateTextureDimensions(int windowWidth, int windowHeight) noexcept;
	static int GetCurrentlyBoundFBO() noexcept;
	~FrameBufferSystem();
};

