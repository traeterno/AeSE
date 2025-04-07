#include "FileChooser.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>

typedef std::pair<bool, sf::String> Entry;

std::vector<Entry> getEntries(std::filesystem::path dir, std::string filetype)
{
	std::vector<Entry> out;
	out.push_back({true, "../"});
	for (auto x: std::filesystem::directory_iterator(dir))
	{
		if (x.path().filename().string().c_str()[0] == '.') { continue; }
		if (x.is_regular_file() && x.path().extension() != filetype) { continue; }
		out.push_back({x.is_directory(), x.path().filename().string()});
	}
	return out;
}

std::filesystem::path openFile(sf::RenderWindow* window, sf::Font* font, std::string filetype)
{
	std::filesystem::path dir = std::filesystem::current_path();

	sf::View view;
	view.setSize((sf::Vector2f)window->getSize());
	view.setCenter(view.getSize() / 2.0f);

	sf::Text entry(*font);
	sf::Text hint(*font, "Current path:\n" + dir.string() + "\nSearching for \"" + filetype + "\"");
	auto hintSize = hint.getGlobalBounds().size;
	hint.setOrigin({0, hintSize.y + 16});
	hint.setPosition({0, (float)window->getSize().y});
	hint.setFillColor(sf::Color::White);


	auto entries = getEntries(dir, filetype);

	while (window->isOpen())
	{
		while (auto event = window->pollEvent())
		{
			if (event->is<sf::Event::Closed>()) { window->close(); return "None"; }
			if (auto key = event->getIf<sf::Event::KeyPressed>())
			{
				if (key->code == sf::Keyboard::Key::Escape) { return "None"; }
			}
			if (auto wheel = event->getIf<sf::Event::MouseWheelScrolled>())
			{
				if (wheel->wheel != sf::Mouse::Wheel::Vertical ||
					window->getSize().y >= entries.size() * 30) continue;

				auto y = view.getCenter().y - wheel->delta * 10;

				view.setCenter({
					view.getCenter().x,
					std::clamp(y, view.getSize().y / 2, (entries.size() + 4) * 30 - view.getSize().y / 2)
				});

				hint.setPosition({0, view.getCenter().y + view.getSize().y / 2});
			}
			if (auto btn = event->getIf<sf::Event::MouseButtonPressed>())
			{
				auto pos = window->mapPixelToCoords(btn->position);
				auto e = (int)pos.y / 30;
				if (e == 0 && dir.has_parent_path()) { dir = dir.parent_path(); entries = getEntries(dir, filetype); }
				else if (e == std::clamp(e, 1, (int)entries.size() - 1))
				{
					if (entries[e].first)
					{
						dir = dir.string() + "/" + entries[e].second;
						entries = getEntries(dir, filetype);
						view.setCenter(view.getSize() / 2.0f);
					}
					else { return dir.string() + "/" + entries[e].second.toAnsiString(); }
				}
				hint.setString("Current path:\n" + dir.string() + "\nSearching for \"" + filetype + "\"");
			}
		}

		if (entries.size() * 30 > view.getSize().y) view.setCenter({
			view.getCenter().x,
			std::clamp(view.getCenter().y, view.getSize().y / 2, (entries.size() + 4) * 30 - view.getSize().y / 2)
		});

		window->setView(view);

		window->clear();
		entry.setPosition({0, 0});
		auto m = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
		for (auto x: entries)
		{
			entry.setString(x.second);
			entry.setFillColor(entry.getGlobalBounds().contains(m) ?
				sf::Color::White :
				sf::Color(200, 200, 200, 255)
			);
			window->draw(entry);
			entry.move({0, 30});
		}
		window->draw(hint);
		window->display();
	}
	return "None";
}