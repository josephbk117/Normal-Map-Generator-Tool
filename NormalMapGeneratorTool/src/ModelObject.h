#pragma once
#include "ShaderProgram.h"
class ModelObject
{
private:
	bool usesElementBuffer = false;
	unsigned int vertexDataCount = 0;
	unsigned int indicesCount = 0;
	unsigned int VBO, VAO, EBO;
public:
	ModelObject(float vertexData[], int count);
	ModelObject(float vertexData[], int vertexDataCount, unsigned int indices[], int indicesCount);
	~ModelObject();
	void draw();
};

