#pragma once
#include <GLM\common.hpp>
struct PreviewStateUtility
{
public:
	float modelPreviewZoomLevel = -5.0f;
	float modelRoughness = 5.0f;
	float modelReflectivity = 0.5f;
	int modelViewMode = 2;
	glm::vec3 diffuseColour = glm::vec3(1, 1, 1);
	glm::vec3 ambientColour = glm::vec3(0.14f, 0.14f, 0.14f);
	glm::vec3 lightColour = glm::vec3(1, 1, 1);
};
struct NormalViewStateUtility
{
public:
	float zoomLevel = 1;
	int mapDrawViewMode = 1;
	float normalMapStrength = 10.0f;
	float specularity = 10.0f;
	float specularityStrength = 0.5f;
	float lightIntensity = 0.5f;
	glm::vec3 lightDirection = glm::vec3(90.0f, 90.0f, 60.0f);
	bool flipX_Ydir = false;
	bool redChannelActive = true;
	bool greenChannelActive = true;
	bool blueChannelActive = true;
	bool methodIndex = false;
	glm::vec3 getNormalizedLightDir() { return glm::vec3(lightDirection.x / 180.0f, lightDirection.y / 180.0f, lightDirection.z / 180.0f); }
};
