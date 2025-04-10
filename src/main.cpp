#include <SFML/Graphics.hpp>
#include <iostream>
#include "FileChooser.hpp"
#include "Toolbar.hpp"

sf::Font font;
sf::RenderWindow window;

float deltaTime = 1.0f / 60.0f;

State state = {
	.project = "",
	.page = Page::File,
	.input = false,
	.mousePressed = false,
	.pressedChar = 0
};

void handleKeyEvent(const sf::Event::KeyPressed* key)
{
	if (key->code == sf::Keyboard::Key::Enter) { state.enter = true; }
	// if (key->code == sf::Keyboard::Key::F1) { showShortcuts(&window, &font, state.page); }
	// if (state.page == Page::File)
	// {
	// 	if (key->control && key->code == sf::Keyboard::Key::S)
	// 	{
	// 		std::cout << "Saved to " << state.project << std::endl;
	// 	}
	// 	if (key->control && key->code == sf::Keyboard::Key::O)
	// 	{
	// 		state.project = openFile(&window, &font, ".xml");
	// 	}
	// 	if (key->control && key->code == sf::Keyboard::Key::N)
	// 	{
	// 		// CurrentState.project = std::filesystem::current_path().string() + "/" + input(&window, &font);
	// 	}
	// }
}

void execute(std::string args)
{
	if (state.command == "file-open")
	{
		state.project = openFile(&window, &font, ".xml");
		state.command.clear();
	}
	if (state.command == "file-save")
	{
		std::cout << "Saved to " << state.project << std::endl;
		state.command.clear();
	}
	if (state.command == "file-create")
	{
		if (args.empty()) { state.input = true; state.hint = "New file name:"; }
		else { state.project = std::filesystem::current_path().string() + "/" + args; }
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
		state.enter = false;
		state.mousePressed = false;
		state.pressedChar = 0;
		while (auto event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>()) { window.close(); }
			if (auto key = event->getIf<sf::Event::KeyPressed>()) { handleKeyEvent(key); }
			if (auto btn = event->getIf<sf::Event::MouseButtonPressed>())
			{
				if (btn->button == sf::Mouse::Button::Left) { state.mousePressed = true; }
			}
			if (auto in = event->getIf<sf::Event::TextEntered>())
			{
				state.pressedChar = in->unicode;
			}
		}
		deltaTime = deltaClock.restart().asSeconds();
		
		window.clear({127, 127, 127, 255});
		drawToolbar(&window, &state, &font);
		window.display();
	}
	return 0;
}