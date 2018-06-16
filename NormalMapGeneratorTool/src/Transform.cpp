#include <iostream>
#include "Transform.h"
#include <GLM\gtc\matrix_transform.hpp>

Transform::Transform(const glm::vec2 & position, float rotation, const glm::vec2 & scale) noexcept
{
	this->position = position;
	this->rotation = rotation;
	this->scale = scale;
	setModelMatrix();
}

Transform::Transform() noexcept
{
	setModelMatrix();
}

Transform::Transform(const Transform & copy) noexcept
{
	this->position = copy.position;
	this->rotation = copy.rotation;
	this->scale = copy.scale;
}

const glm::mat4& Transform::getMatrix()const
{
	return modelMatrix;
}
void Transform::start()
{
}
void Transform::update()
{
	if (needsUpdate)
	{
		setModelMatrix();
		needsUpdate = false;
	}
}
void Transform::setPosition(const glm::vec2 & position)
{
	needsUpdate = true;
	this->position = position;
}
void Transform::setPosition(float xCoord, float yCoord)
{
	position = glm::vec2(xCoord, yCoord);
}

void Transform::translate(float x, float y)
{
	this->position = glm::vec2(position.x + x, position.y + y);
}

void Transform::setX(float xValue)
{
	setPosition(glm::vec2(xValue, position.y));
}
void Transform::setY(float yValue)
{
	setPosition(glm::vec2(position.x, yValue));
}
void Transform::setRotation(float rotation)
{
	needsUpdate = true;
	this->rotation = rotation;
}
void Transform::rotate(float rotation)
{
	this->rotation += rotation;
}
void Transform::setScale(const glm::vec2& scale)
{
	needsUpdate = true;
	this->scale = scale;
}

const glm::vec2 & Transform::getPosition()const noexcept
{
	return position;
}
float Transform::getRotation()const noexcept
{
	return rotation;
}
const glm::vec2& Transform::getScale()const noexcept
{
	return scale;
}

bool Transform::operator==(const Transform & transform)const noexcept
{
	return (position == transform.position && rotation == transform.rotation && scale == transform.scale);
}
bool Transform::operator!=(const Transform & transform)const noexcept
{
	return (position != transform.position || rotation != transform.rotation || scale != transform.scale);
}
void Transform::setModelMatrix()
{
	modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0));
	modelMatrix = glm::rotate(modelMatrix, rotation, glm::vec3(0.8f, 0.8f, 0.7f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(scale.x, scale.y, 0));
}
