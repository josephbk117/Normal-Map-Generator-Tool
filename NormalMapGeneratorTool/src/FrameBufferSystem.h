#pragma once
class FrameBufferSystem
{
private:
	unsigned int framebuffer;
	unsigned int textureColorbuffer;
public:
	FrameBufferSystem();
	void init(int windowWidth, int windowHeight);
	void BindFrameBuffer();
	void BindBufferTexture();
	unsigned int getBufferTexture();
	void updateTextureDimensions(int windowWidth, int windowHeight);
	~FrameBufferSystem();
};
