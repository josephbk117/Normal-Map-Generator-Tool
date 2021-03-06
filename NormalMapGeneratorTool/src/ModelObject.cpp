#include "ModelObject.h"
#include <GL\glew.h>
#include <iostream>

ModelObject::ModelObject()
{

}
ModelObject::ModelObject(float vertexData[], int vertexDataCount)
{
	usesElementBuffer = false;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexDataCount, vertexData, GL_STATIC_DRAW);

	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

ModelObject::ModelObject(float vertexData[], int vertexDataCount, unsigned int indices[], int indicesCount)
{
	updateMeshData(vertexData, vertexDataCount, indices, indicesCount);
}

void ModelObject::updateMeshData(float vertexData[], int vertexDataCount, unsigned int indices[], int indicesCount)
{
	std::cout<<"\nUpdated Mesh";
	usesElementBuffer = true;
	this->vertexDataCount = vertexDataCount;
	this->indicesCount = indicesCount;

	if (VAO != 0)
	{
		std::cout << "\nPrevious deleted\n";
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, this->vertexDataCount, &vertexData[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indicesCount * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ModelObject::draw() const
{
	glBindVertexArray(VAO);
	if (usesElementBuffer)
		glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, (void *)0);
	else
		glDrawArrays(GL_TRIANGLES, 0, vertexDataCount);

	glBindVertexArray(0);
}

ModelObject::~ModelObject() 
{
	if (VAO != 0)
	{
		std::cout << "\nRemoved Model From Memory\n";
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}
}
