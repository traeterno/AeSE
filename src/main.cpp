#include <SFML/Graphics.hpp>
#include "FileChooser.hpp"
#include "Toolbar.hpp"
#include "Skeleton.hpp"

sf::Font font;
sf::RenderWindow window;
Skeleton skeleton;

float deltaTime = 1.0f / 60.0f;

State state = {
	.project = "",
	.page = Page::File,
	.input = false,
	.mousePressed = false,
	.pressedChar = 0,
	.enter = false,
	.command = "",
	.hint = "",
	.texture = "",
	.bonePath = {}
};

void handleKeyEvent(const sf::Event::KeyPressed* key)
{
	if (key->code == sf::Keyboard::Key::Enter) { state.enter = true; }
}

void execute(std::string args)
{
	if (state.command == "file-open")
	{
		state.project = openFile(&window, &font, ".xml");
		skeleton.load(&state);
	}
	if (state.command == "file-save")
	{
		if (!args.empty())
		{
			skeleton.save(&state);
			state.input = true;
			state.hint = "Saved; press Enter";
		}
	}
	if (state.command == "file-create")
	{
		if (args.empty()) { state.input = true; state.hint = "New file name:"; }
		else
		{
			state.project = std::filesystem::current_path().string() + "/" + args;
			state.texture = "";
			skeleton = Skeleton();
		}
	}
	if (state.command == "file-loadTex")
	{
		state.texture = openFile(&window, &font, ".png");
		skeleton.updateTexture(&state);
	}

	if (state.command == "bone-select")
	{
		state.bonePath = parsePath(&skeleton.root, args);
		state.page = Page::BoneDetails;
	}
	if (state.command == "bone-name")
	{
		if (args.empty()) { state.input = true; state.hint = "New name:"; }
		else { getBone(&skeleton.root, state.bonePath)->name = args; }
	}
	if (state.command == "bone-length")
	{
		if (args.empty()) { state.input = true; state.hint = "New length:"; }
		else { getBone(&skeleton.root, state.bonePath)->length = std::stof(args); }
	}
	if (state.command == "bone-angle")
	{
		if (args.empty()) { state.input = true; state.hint = "New angle:"; }
		else { getBone(&skeleton.root, state.bonePath)->angle = std::stof(args); }
	}
	if (state.command == "bone-texture")
	{
		if (args.empty()) { state.input = true; state.hint = "Texture name:"; }
		else { getBone(&skeleton.root, state.bonePath)->texture = args; }
	}
	if (state.command == "bone-add")
	{
		if (args.empty()) { state.input = true; state.hint = "New bone name:"; }
		else
		{
			getBone(&skeleton.root, state.bonePath)->children.push_back(Bone {.name=args});
			state.page = Page::Bones;
		}
	}
	if (state.command == "bone-destroy")
	{
		if (!args.empty()) { state.input = true; state.hint = "Enter/Escape:"; }
		else
		{
			if (!state.bonePath.size()) return;
			auto index = state.bonePath.back();
			state.bonePath.pop_back();
			auto bone = getBone(&skeleton.root, state.bonePath);
			bone->children.erase(bone->children.begin() + index);
			state.page = Page::Bones;
		}
	}

	if (state.command == "texture-add")
	{
		if (args.empty()) { state.input = true; state.hint = "Texture name:"; }
		else
		{
			skeleton.textures.push_back(Texture {.name=args});
			state.page = Page::TexDetails;
			state.currentTexture = args;
		}
	}
	if (state.command == "texture-remove")
	{
		if (args.empty()) { state.input = true; state.hint = "Texture name:"; }
		else
		{
			for (int i = 0; i < skeleton.textures.size(); i++)
			{
				if (skeleton.textures[i].name == args)
				{
					skeleton.textures.erase(skeleton.textures.begin() + i);
					state.page = Page::Textures;
					break;
				}
			}
		}
	}
	if (state.command == "texture-select")
	{
		state.currentTexture = args;
		state.page = Page::TexDetails;
	}
	if (state.command == "texture-name")
	{
		if (args.empty()) { state.input = true; state.hint = "New name"; }
		else
		{
			for (int i = 0; i < skeleton.textures.size(); i++)
			{
				if (skeleton.textures[i].name == state.currentTexture)
				{
					skeleton.textures[i].name = args;
					state.currentTexture = args;
					break;
				}
			}
		}
	}
	if (state.command == "texture-rect")
	{
		if (args.empty()) { state.input = true; state.hint = "New rect:"; }
		else
		{
			for (int i = 0; i < skeleton.textures.size(); i++)
			{
				if (skeleton.textures[i].name == state.currentTexture)
				{
					auto x = args.substr(0, args.find(" "));
					args = args.substr(args.find(" ") + 1);
					auto y = args.substr(0, args.find(" "));
					args = args.substr(args.find(" ") + 1);
					auto w = args.substr(0, args.find(" "));
					args = args.substr(args.find(" ") + 1);
					skeleton.textures[i].rect = sf::IntRect(
						{std::stoi(x), std::stoi(y)},
						{std::stoi(w), std::stoi(args)}
					);
					break;
				}
			}
		}
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
		drawToolbar(&window, &state, &font, &skeleton);
		skeleton.draw(&window, &state);
		window.display();
	}
	return 0;
}