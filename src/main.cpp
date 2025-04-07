#include <SFML/Graphics.hpp>
#include <iostream>
#include "FileChooser.hpp"
#include "Toolbar.hpp"

sf::Font font;
sf::RenderWindow window;

float deltaTime = 1.0f / 60.0f;

struct
{
	std::filesystem::path project;
} CurrentState;

void handleKeyEvent(const sf::Event::KeyPressed* key)
{
	if (key->control && key->code == sf::Keyboard::Key::S)
	{
		std::cout << "Saved to " << CurrentState.project << std::endl;
	}
	if (key->control && key->code == sf::Keyboard::Key::O)
	{
		CurrentState.project = openFile(&window, &font, ".xml");
		std::cout << "Opened " << CurrentState.project << std::endl;
	}
}

int main()
{
	window.create(
		sf::VideoMode::getDesktopMode(),
		"Skeleton Editor",
		sf::Style::None,
		sf::State::Fullscreen
	);
	window.setVerticalSyncEnabled(true);

	if (!font.openFromFile("res/b52.ttf")) return 1;

	sf::Clock deltaClock;
	while (window.isOpen())
	{
		while (auto event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>()) { window.close(); }
			if (auto key = event->getIf<sf::Event::KeyPressed>()) { handleKeyEvent(key); }
		}
		deltaTime = deltaClock.restart().asSeconds();
		
		window.clear({127, 127, 127, 255});
		drawToolbar(&window);
		window.display();
	}
	return 0;
}