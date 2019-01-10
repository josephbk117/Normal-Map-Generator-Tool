#pragma once
#include "Transform.h"
#include <GLM\glm.hpp>
class DrawingPanel
{
public:
	DrawingPanel();
	~DrawingPanel();
	//Initialize drawing panel with certain resolution
	void init(float width, float height) noexcept;
	Transform* getTransform() noexcept;
	//Set which texture is linked to this drawing panel
	void setTextureID(unsigned int textureID) noexcept;
	//Get the texture linked to this drawing panel
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