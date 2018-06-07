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
		std::cout << "Open GL init error"<<std::endl;
		return EXIT_FAILURE;
	}
	bool running = true;
	DrawingPanel drawingPanel;
	drawingPanel.init(1.5f, 1.5f);
	unsigned int texId = TextureManager::loadTextureFromFile("Resources\\goli.png", "poke", false);
	drawingPanel.setTextureID(texId);
	ShaderProgram shader;
	shader.compileShaders("Resources\\spriteBase.vs", "Resources\\spriteBase.fs");
	shader.linkShaders();
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
			}
		}
		glClearColor(0.3f, 0.6f, 0.9f, 1.0f);
		glClear( GL_COLOR_BUFFER_BIT );
		shader.use();
		drawingPanel.draw();
		window.display();
	}
	window.close();

	return EXIT_SUCCESS;
}