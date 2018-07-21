#pragma once
#include <GLM\glm.hpp>
#include <GLM\gtc\matrix_transform.hpp>
#include "Transform.h"

class Camera
{
public:
	Camera();
	~Camera();
	void init(const glm::vec2& screenDimensions)noexcept;
	void init(float dimensionX, float dimensionY) noexcept;
	void setScale(float newScale);
	const float& getScale()const noexcept;
	const glm::mat4& getOrthoMatrix()const noexcept;
	bool isObjectInCameraView(const glm::vec2 & position, const glm::vec2& dimensions)const noexcept;
	void setScreenRatio(const glm::vec2& screenDimension) noexcept;
	const glm::vec2 convertScreenPointToWorldPoint(const glm::vec2& screenPosition, const glm::vec2& screenDimensions) noexcept;
private:
	glm::vec2 screenDimensions;
	bool needsUpdate;
	Transform* transform;
	Transform previousTransformData;
	glm::mat4 orthographicMatrix;
	glm::mat4 viewMatrix;
	float scale;
};