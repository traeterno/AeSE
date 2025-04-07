#include "Toolbar.hpp"

#include <SFML/Graphics.hpp>

void drawToolbar(sf::RenderWindow *window)
{
	sf::RectangleShape bg({(float)window->getSize().x / 4, (float)window->getSize().y});
	bg.setPosition({(float)window->getSize().x / 4 * 3, 0});
	bg.setFillColor(sf::Color::Black);
	
	window->draw(bg);
}