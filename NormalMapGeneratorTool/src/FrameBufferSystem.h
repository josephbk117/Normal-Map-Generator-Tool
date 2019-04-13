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
	//Initialize frame buffer with certain resolution
	void init(const glm::vec2& windowRes, const glm::vec2& maxBufferResolution);
	//Initialize frame buffer with certain resolution
	void init(int windowWidth, int windowHeight, int maxBufferWidth, int maxBufferHeight);
	//Make the this the new bound frame buffer
	void bindFrameBuffer() noexcept;
	//Bind the colour buffer texture which is linked with the framebuffer
	void bindColourTexture() noexcept;
	//Get colour buffer texture
	unsigned int getColourTexture()const noexcept;
	//Bind the depth buffer texture which is linked with the framebuffer
	void bindDepthTexture() noexcept;
	//Get depth buffer texture
	unsigned int getDepthTexture()const noexcept;
	//Change resolution of existing frame buffer
	void updateTextureDimensions(int windowWidth, int windowHeight) noexcept;
	//Change resolution of existing frame buffer
	void updateTextureDimensions(const glm::vec2 &windowRes) noexcept;
	//Get the frame buffer id
	unsigned int getFrameBufferId() const;
	//Get the FBO that is current bound
	static unsigned int getCurrentlyBoundFBO() noexcept;
	static void blit(const FrameBufferSystem& source, const FrameBufferSystem& destination,
		const glm::vec2& srcStartCoord, const glm::vec2& srcEndCoord, const glm::vec2& destStartCoord, 
		const glm::vec2& destEndCoord)noexcept;
	static void blit(const FrameBufferSystem& source, const FrameBufferSystem& destination, const glm::vec2 screenRes)noexcept;
	static void bindDefaultFrameBuffer()noexcept;
	~FrameBufferSystem();
};

