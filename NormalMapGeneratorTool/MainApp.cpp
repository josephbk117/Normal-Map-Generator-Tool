#include <iostream>
#include <string>
#include <GL\glew.h>
#include <SFML/Window.hpp>
#include "DrawingPanel.h"
#include "TextureLoader.h"
#include "ShaderProgram.h"
#include "Transform.h"

bool saveScreenshot(std::string filename, int w, int h);
int main()
{
	sf::Window window(sf::VideoMode(1024, 1024, 32), "Normal Map Editor", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Open GL init error" << std::endl;
		return EXIT_FAILURE;
	}
	bool running = true;
	DrawingPanel drawingPanel;
	drawingPanel.init(0.8f, 0.8f);
	unsigned int texId = TextureManager::loadTextureFromFile("Resources\\goli.png", "poke", false);
	drawingPanel.setTextureID(texId);
	ShaderProgram shader;
	shader.compileShaders("Resources\\spriteBase.vs", "Resources\\spriteBase.fs");
	shader.linkShaders();
	int strengthValue = shader.getUniformLocation("_HeightmapStrength");
	int normalMapModeOnUniform = shader.getUniformLocation("_normalMapModeOn");
	int modelMatrix = shader.getUniformLocation("model");
	float strValue = 4.0f;
	float contrastStrength = 1.0f;
	int normalMapMode = 1;
	Transform transform;
	transform.setPosition(0, 0);
	float zoomLevel = 0.001f;
	while (running)
	{
		sf::Event windowEvent;
		while (window.pollEvent(windowEvent))
		{
			switch (windowEvent.type)
			{
			case sf::Event::Closed:
				running = false;
				break;
			case sf::Event::Resized:
				float width = window.getSize().x;
				float height = window.getSize().y;
				float aspectRatio = width / height;
				if (width < height)
					transform.setScale(glm::vec2(width / aspectRatio, height * aspectRatio) * zoomLevel);
				else
					transform.setScale(glm::vec2(width / aspectRatio, height * aspectRatio) * zoomLevel);
				glViewport(0, 0, width, height);
				break;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::G))
			{
				transform.setRotation(transform.getRotation() + 0.01f);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			{
				transform.setX(transform.getPosition().x - 0.005f);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			{
				transform.setX(transform.getPosition().x + 0.005f);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			{
				transform.setY(transform.getPosition().y - 0.005f);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			{
				transform.setY(transform.getPosition().y + 0.005f);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::V))
			{
				zoomLevel += 0.00001f;
				zoomLevel = glm::clamp(zoomLevel, 0.00001f, 0.01f);
				float width = window.getSize().x;
				float height = window.getSize().y;
				float aspectRatio = width / height;
				if (width < height)
					transform.setScale(glm::vec2(width / aspectRatio, height * aspectRatio) * zoomLevel);
				else
					transform.setScale(glm::vec2(width / aspectRatio, height * aspectRatio) * zoomLevel);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::B))
			{
				zoomLevel -= 0.00001f;
				zoomLevel = glm::clamp(zoomLevel, 0.00001f, 0.01f);
				float width = window.getSize().x;
				float height = window.getSize().y;
				float aspectRatio = width / height;
				if (width < height)
					transform.setScale(glm::vec2(width / aspectRatio, height * aspectRatio) * zoomLevel);
				else
					transform.setScale(glm::vec2(width / aspectRatio, height * aspectRatio) * zoomLevel);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			{
				strValue -= 0.3f;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			{
				strValue += 0.3f;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			{
				normalMapMode = (normalMapMode + 1) % 3;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
			{
				if (saveScreenshot("D:\\scr.tga", window.getSize().x - 50, window.getSize().y - 50))
					std::cout << "Saved";
			}
		}
		transform.setX(glm::clamp(transform.getPosition().x, -0.3f, 0.3f));
		transform.setY(glm::clamp(transform.getPosition().y, -0.3f, 0.3f));
		transform.update();
		glClearColor(0.3f, 0.6f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		shader.applyShaderUniformMatrix(modelMatrix, transform.getMatrix());
		shader.applyShaderFloat(strengthValue, strValue);
		shader.applyShaderInt(normalMapModeOnUniform, normalMapMode);
		shader.use();
		drawingPanel.draw();
		window.display();
	}
	window.close();

	return EXIT_SUCCESS;
}
bool saveScreenshot(std::string filename, int w, int h)
{
	//This prevents the images getting padded 
	// when the width multiplied by 3 is not a multiple of 4
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	int nSize = w * h * 3;
	// First let's create our buffer, 3 channels per Pixel
	char* dataBuffer = (char*)malloc(nSize * sizeof(char));

	if (!dataBuffer) return false;

	// Let's fetch them from the backbuffer	
	// We request the pixels in GL_BGR format, thanks to Berzeger for the tip
	glReadPixels((GLint)0, (GLint)0,
		(GLint)w, (GLint)h,
		GL_BGR, GL_UNSIGNED_BYTE, dataBuffer);

	//Now the file creation
#pragma warning(suppress : 4996)
	FILE *filePtr = fopen(filename.c_str(), "wb");
	if (!filePtr) return false;


	unsigned char TGAheader[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };
	unsigned char header[6] = { w % 256,w / 256,
		h % 256,h / 256,
		24,0 };
	// We write the headers
	fwrite(TGAheader, sizeof(unsigned char), 12, filePtr);
	fwrite(header, sizeof(unsigned char), 6, filePtr);
	// And finally our image data
	fwrite(dataBuffer, sizeof(GLubyte), nSize, filePtr);
	fclose(filePtr);

	free(dataBuffer);

	return true;
}