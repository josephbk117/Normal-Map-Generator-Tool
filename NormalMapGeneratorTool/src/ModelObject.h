#pragma once
#include "ShaderProgram.h"
class ModelObject
{
private:
	unsigned int VBO, VAO;
public:
	ModelObject(float vertexData[], int count);
	~ModelObject();
	void draw();
};

