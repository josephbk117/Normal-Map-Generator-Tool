#pragma once
#include "Transform.h"
#include <GLM\glm.hpp>
class DrawingPanel
{
public:
	DrawingPanel();
	~DrawingPanel();
	void init(float width, float height);
	Transform* getTransform();
	void setTextureID(unsigned int textureID);
	unsigned int getTextureID()const noexcept;
	bool isPointInPanel(float xpos, float ypos);
	glm::vec4 getPanelWorldDimension();
	void draw();
private:
	Transform transform;
	float width, height;
	unsigned int vaoID;
	unsigned int vboID;
	unsigned int textureID;
};