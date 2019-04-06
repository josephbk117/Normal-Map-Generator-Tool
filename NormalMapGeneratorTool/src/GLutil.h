#pragma once
#include <GL\glew.h>
#include <GLM\common.hpp>

enum FrameBufferAttachment
{
	COLOUR_BUFFER = GL_COLOR_BUFFER_BIT,
	DEPTH_BUFFER = GL_DEPTH_BUFFER_BIT,
	COLOUR_AND_DEPTH_BUFFER = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
};

enum FaceCullingMode
{
	BACK_FACE_CULLING = GL_BACK,
	FRONT_FACE_CULLING = GL_FRONT,
	BACK_AND_FRONT_FACE_CULLING = GL_FRONT_AND_BACK
};

enum DepthTestMode
{
	NEVER_PASS = GL_NEVER,
	DEPTH_LESS = GL_LESS,
	DEPTH_LESS_OR_EQUAL = GL_LEQUAL,
	DEPTH_EQUAL = GL_EQUAL,
	DEPTH_GREATER_OR_EQUAL = GL_GEQUAL,
	DEPTH_GREATER = GL_GREATER,
	DEPTH_NOT_EQUAL = GL_NOTEQUAL,
	ALWAYS_PASS = GL_ALWAYS
};

class GL
{
public:
	static void clear(FrameBufferAttachment frameBufferAttachement)
	{
		glClear(frameBufferAttachement);
	}

	static void setClearColour(glm::vec3 colour)
	{
		glClearColor(colour.r, colour.g, colour.b, 1.0);
	}

	static void setClearColour(float r, float g, float b)
	{
		glClearColor(r, g, b, 1.0);
	}

	static void enableFaceCulling()
	{
		glEnable(GL_CULL_FACE);
	}

	static void disableFaceCulling()
	{
		glDisable(GL_CULL_FACE);
	}

	static void setFaceCullingMode(FaceCullingMode faceCullingMode)
	{
		glCullFace(faceCullingMode);
	}

	static void enableBlending()
	{
		glEnable(GL_BLEND);
	}

	static void disableBlending()
	{
		glDisable(GL_BLEND);
	}

	static void enableDepthTest()
	{
		glEnable(GL_DEPTH_TEST);
	}

	static void disableDepthTest()
	{
		glDisable(GL_DEPTH_TEST);
	}

	static void setDepthTestMode(DepthTestMode depthTestMode)
	{
		glDepthFunc(depthTestMode);
	}

};