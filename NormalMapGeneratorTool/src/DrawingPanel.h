#pragma once
#include "Transform.h"
#include <GLM\glm.hpp>
class DrawingPanel
{
public:
	DrawingPanel();
	~DrawingPanel();
	void init(float width, float height) noexcept;
	Transform* getTransform() noexcept;
	void setTextureID(unsigned int textureID) noexcept;
	unsigned int getTextureID()const noexcept;
	bool isPointInPanel(float xpos, float ypos)noexcept;
	glm::vec4 getPanelWorldDimension()noexcept;
	void draw()noexcept;
private:
	Transform transform;
	float width, height;
	unsigned int vaoID;
	unsigned int vboID;
	unsigned int textureID;
};