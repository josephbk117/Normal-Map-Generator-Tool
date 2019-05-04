#pragma once
#include <GL\glew.h>
#include <GLM\common.hpp>

enum TextureType
{
	TEXTURE_1D = GL_TEXTURE_1D, //Images in this texture all are 1 - dimensional.They have width, but no height or depth.
	TEXTURE_2D = GL_TEXTURE_2D, //Images in this texture all are 2 - dimensional.They have width and height, but no depth.
	TEXTURE_3D = GL_TEXTURE_3D, //Images in this texture all are 3 - dimensional.They have width, height, and depth.
	TEXTURE_RECTANGLE = GL_TEXTURE_RECTANGLE, //The image in this texture(only one image.No mipmapping) is 2 - dimensional.Texture coordinates used for these textures are not normalized.
	TEXTURE_BUFFER = GL_TEXTURE_BUFFER, //The image in this texture(only one image.No mipmapping) is 1 - dimensional.The storage for this data comes from a Buffer Object.
	TEXTURE_CUBE_MAP = GL_TEXTURE_CUBE_MAP, //There are exactly 6 distinct sets of 2D images, all of the same size.They act as 6 faces of a cube.
	TEXTURE_1D_ARRAY = GL_TEXTURE_1D_ARRAY, //Images in this texture all are 1 - dimensional.However, it contains multiple sets of 1 - dimensional images, all within one texture.The array length is part of the texture's size.
	TEXTURE_2D_ARRAY = GL_TEXTURE_2D_ARRAY, //Images in this texture all are 2 - dimensional.However, it contains multiple sets of 2 - dimensional images, all within one texture.The array length is part of the texture's size.
	TEXTURE_CUBE_MAP_ARRAY = GL_TEXTURE_CUBE_MAP_ARRAY, //Images in this texture are all cube maps.It contains multiple sets of cube maps, all within one texture.The array length * 6 (number of cube faces) is part of the texture size.
	TEXTURE_2D_MULTISAMPLE = GL_TEXTURE_2D_MULTISAMPLE, //The image in this texture(only one image.No mipmapping) is 2 - dimensional.Each pixel in these images contains multiple samples instead of just one value.
	TEXTURE_2D_MULTISAMPLE_ARRAY = GL_TEXTURE_2D_MULTISAMPLE_ARRAY //Combines 2D array and 2D multisample types.No mipmapping.
};

enum Capability
{
	ALPHA_TEST = GL_ALPHA_TEST,
	AUTO_NORMAL = GL_AUTO_NORMAL,
	BLEND = GL_BLEND,
	LOGIC_OP = GL_COLOR_LOGIC_OP,
	CULL_FACE = GL_CULL_FACE,
	DEPTH_TEST = GL_DEPTH_TEST,
	SCISSOR_TEST = GL_SCISSOR_TEST,
	STENCIL_TEST = GL_STENCIL_TEST,
	SEAMLES_CUBE_MAP = GL_TEXTURE_CUBE_MAP_SEAMLESS
};

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
	static inline void clear(FrameBufferAttachment frameBufferAttachement) noexcept
	{
		glClear(frameBufferAttachement);
	}

	static inline void setClearColour(const glm::vec3& colour) noexcept
	{
		glClearColor(colour.r, colour.g, colour.b, 1.0);
	}

	static inline void setClearColour(float r, float g, float b) noexcept
	{
		glClearColor(r, g, b, 1.0);
	}

	static inline void setViewport(int x, int y, int width, int height) noexcept
	{
		glViewport(x, y, width, height);
	}

	static inline void setViewport(const glm::ivec2& position, const glm::ivec2& dimension) noexcept
	{
		glViewport(position.x, position.y, dimension.x, dimension.y);
	}

	static inline void enableCapability(Capability capability) noexcept
	{
		glEnable(capability);
	}

	static inline void disableCapability(Capability capability) noexcept
	{
		glDisable(capability);
	}

	static inline void enableFaceCulling() noexcept
	{
		enableCapability(Capability::CULL_FACE);
	}

	static inline void disableFaceCulling() noexcept
	{
		disableCapability(Capability::CULL_FACE);
	}

	static inline void setFaceCullingMode(FaceCullingMode faceCullingMode) noexcept
	{
		glCullFace(faceCullingMode);
	}

	static inline void enableBlending() noexcept
	{
		glEnable(GL_BLEND);
	}

	static inline void disableBlending() noexcept
	{
		glDisable(GL_BLEND);
	}

	static inline void enableDepthTest() noexcept
	{
		glEnable(GL_DEPTH_TEST);
	}

	static inline void disableDepthTest() noexcept
	{
		glDisable(GL_DEPTH_TEST);
	}

	static inline void setDepthTestMode(DepthTestMode depthTestMode) noexcept
	{
		glDepthFunc(depthTestMode);
	}

	static inline void setActiveTextureIndex(unsigned char textureIndex)noexcept
	{
		glActiveTexture(GL_TEXTURE0 + textureIndex);
	}

	static inline void bindVertexArray(unsigned int vaoId) noexcept
	{
		glBindVertexArray(vaoId);
	}

};