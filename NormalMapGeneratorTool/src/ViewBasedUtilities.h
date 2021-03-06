#pragma once
#include <GLM\common.hpp>
struct PreviewStateUtility
{
public:
	float modelPreviewZoomLevel = -5.0f;
	float metalness = 0.04f;
	float roughness = 0.5f;
	int modelViewMode = 3;
	bool useMatcap = false;
	float lightIntensity = 2.5f;
	bool showNormals = false;
	bool showGrid = true;
	float normDisplayThickness = 4.0f;
	float normDisplayLineLength = 0.25f;
	glm::vec2 lightLocation = glm::vec2(0.0f, 0.0f);
	glm::vec3 diffuseColour = glm::vec3(1, 1, 1);
	glm::vec3 lightColour = glm::vec3(1, 1, 1);
};
struct NormalViewStateUtility
{
public:
	float zoomLevel = 1;
	int mapDrawViewMode = 1;
	float normalMapStrength = 2.0f;
	float specularity = 5.0f;
	float specularityStrength = 1.0f;
	float lightIntensity = 0.5f;
	glm::vec3 lightDirection = glm::vec3(90.0f, 90.0f, 220.0f);
	bool flipX_Ydir = false;
	bool redChannelActive = true;
	bool greenChannelActive = true;
	bool blueChannelActive = true;
	bool methodIndex = false;
	glm::vec3 getNormalizedLightDir() { return glm::vec3(lightDirection.x / 180.0f, lightDirection.y / 180.0f, lightDirection.z / 180.0f); }
};
