#pragma once
#include <GL\glew.h>
class FrameBuffer
{
private:
	GLuint framebufferName = 0;
	GLuint renderedTexture = 0;
public:
	FrameBuffer();
	void renderToTexture();
	GLuint getTexture();
	~FrameBuffer();
};

