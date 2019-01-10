#pragma once
#include "ShaderProgram.h"
class ModelObject
{
private:
	bool usesElementBuffer = false;
	unsigned int vertexDataCount = 0;
	unsigned int indicesCount = 0;
	unsigned int VBO = 0, VAO = 0, EBO = 0;
public:
	ModelObject();
	ModelObject(float vertexData[], int count);
	ModelObject(float vertexData[], int vertexDataCount, unsigned int indices[], int indicesCount);
	~ModelObject();

	//Provide new data to existing model
	void UpdateMeshData(float vertexData[], int vertexDataCount, unsigned int indices[], int indicesCount);
	void draw();
};

