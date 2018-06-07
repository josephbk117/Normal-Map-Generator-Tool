#include <iostream>
#include <GL\glew.h>
#include <SFML/Window.hpp>
#include "DrawingPanel.h"
#include "TextureLoader.h"
#include "ShaderProgram.h"
int main()
{
	sf::Window window(sf::VideoMode(500, 500, 32), "Normal Map Editor", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
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
	float strValue = 4.0f;
	int normalMapMode = 1;
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
				float aspectRatio = 1;
				float width = window.getSize().x;
				float height = window.getSize().y;
				glViewport(0, 0, width, height);
				break;
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
				normalMapMode *= -1;
			}
		}
		glClearColor(0.3f, 0.6f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		shader.applyShaderFloat(strengthValue, strValue);
		shader.applyShaderInt(normalMapModeOnUniform, normalMapMode);
		shader.use();
		drawingPanel.draw();
		window.display();
	}
	window.close();

	return EXIT_SUCCESS;
}