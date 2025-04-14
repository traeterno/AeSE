#include <SFML/Graphics.hpp>
#include "FileChooser.hpp"
#include "Toolbar.hpp"
#include "Skeleton.hpp"

sf::Font font;
sf::RenderWindow window;
Skeleton skeleton;
sf::Vector2f cameraPos;

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
	.bonePath = {},
	.currentTexture = "",
	.currentDrawable = ""
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
	else if (state.command == "file-save")
	{
		if (!args.empty())
		{
			skeleton.save(&state);
			state.input = true;
			state.hint = "Saved; press Enter";
		}
	}
	else if (state.command == "file-create")
	{
		if (args.empty()) { state.input = true; state.hint = "New file name:"; }
		else
		{
			state.project = std::filesystem::current_path().string() + "/" + args;
			state.texture = "";
			skeleton = Skeleton();
		}
	}
	else if (state.command == "file-loadTex")
	{
		state.texture = openFile(&window, &font, ".png");
		skeleton.updateTexture(&state);
	}

	else if (state.command == "bone-select")
	{
		state.bonePath = parsePath(&skeleton.root, args);
		state.page = Page::BoneDetails;
	}
	else if (state.command == "bone-name")
	{
		if (args.empty()) { state.input = true; state.hint = "New name:"; }
		else { getBone(&skeleton.root, state.bonePath)->name = args; }
	}
	else if (state.command == "bone-length")
	{
		if (args.empty()) { state.input = true; state.hint = "New length:"; }
		else { getBone(&skeleton.root, state.bonePath)->length = std::stof(args); }
	}
	else if (state.command == "bone-angle")
	{
		if (args.empty()) { state.input = true; state.hint = "New angle:"; }
		else { getBone(&skeleton.root, state.bonePath)->angle = std::stof(args); }
	}
	else if (state.command == "bone-visible")
	{
		if (args.empty()) { state.input = true; state.hint = "VBone name:"; }
		else { getBone(&skeleton.root, state.bonePath)->visible = args; }
	}
	else if (state.command == "bone-add")
	{
		if (args.empty()) { state.input = true; state.hint = "New bone name:"; }
		else
		{
			getBone(&skeleton.root, state.bonePath)->children.push_back(Bone {.name=args, .visible="none"});
			state.page = Page::Bones;
		}
	}
	else if (state.command == "bone-destroy")
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
			state.command = "texture-rect";
			execute("current-" + args);
		}
	}
	else if (state.command == "texture-remove")
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
	else if (state.command == "texture-name")
	{
		if (args.find("current-") != std::string::npos)
		{
			state.input = true;
			state.hint = "New name";
			state.currentTexture = args.substr(args.find("-") + 1);
		}
		else
		{
			for (int i = 0; i < skeleton.textures.size(); i++)
			{
				if (skeleton.textures[i].name == state.currentTexture)
				{
					skeleton.textures[i].name = args;
					state.currentTexture = "";
					break;
				}
			}
		}
	}
	else if (state.command == "texture-rect")
	{
		if (args.find("current-") != std::string::npos)
		{
			state.input = true;
			state.hint = "New rect + origin:";
			state.currentTexture = args.substr(args.find("-") + 1);
		}
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
					auto h = args.substr(0, args.find(" "));
					args = args.substr(args.find(" ") + 1);
					auto ox = args.substr(0, args.find(" "));
					args = args.substr(args.find(" ") + 1);
					skeleton.textures[i].rect = sf::IntRect(
						{std::stoi(x), std::stoi(y)},
						{std::stoi(w), std::stoi(h)}
					);
					skeleton.textures[i].origin = {
						std::stof(ox), std::stof(args)
					};
					state.currentTexture = "";
					break;
				}
			}
		}
	}

	if (state.command == "drawable-add")
	{
		if (args.empty()) { state.input = true; state.hint = "VBone name:"; }
		else
		{
			skeleton.visible.push_back(VisibleBone {.name=args, .texture="none"});
			state.command = "drawable-texture";
			execute("current-" + args);
		}
	}
	else if (state.command == "drawable-remove")
	{
		if (args.empty()) { state.input = true; state.hint = "VBone name:"; }
		else
		{
			for (int i = 0; i < skeleton.visible.size(); i++)
			{
				if (skeleton.visible[i].name == args)
				{
					skeleton.visible.erase(skeleton.visible.begin() + i);
					break;
				}
			}
		}
	}
	else if (state.command == "drawable-name")
	{
		if (args.find("current-") != std::string::npos)
		{
			state.input = true;
			state.hint = "New name:";
			state.currentDrawable = args.substr(args.find("-") + 1);
		}
		else
		{
			skeleton.getVisible(state.currentDrawable)->name = args;
			state.currentDrawable = "";
		}
	}
	else if (state.command == "drawable-texture")
	{
		if (args.find("current-") != std::string::npos)
		{
			state.input = true;
			state.hint = "Texture + layer:";
			state.currentDrawable = args.substr(args.find("-") + 1);
		}
		else
		{
			skeleton.getVisible(state.currentDrawable)->texture = args.substr(0, args.find(" "));
			skeleton.getVisible(state.currentDrawable)->layer = std::stoi(args.substr(args.find(" ") + 1));
			state.currentDrawable = "";
		}
	}
	else if (true) {}
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

	sf::RenderTexture view({window.getSize().x / 4 * 3, window.getSize().y});
	sf::View cam(cameraPos, (sf::Vector2f)view.getSize());

	sf::Clock deltaClock;

	float zoom = 1;
	float speed = 500;

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
			if (auto wheel = event->getIf<sf::Event::MouseWheelScrolled>())
			{
				cam.zoom(wheel->delta > 0 ? 2 : 0.5);
				zoom *= (wheel->delta > 0 ? 2 : 0.5);
			}
		}
		deltaTime = deltaClock.restart().asSeconds();

		if (!state.input)
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
			{
				cameraPos.x -= speed * zoom * deltaTime;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
			{
				cameraPos.x += speed * zoom * deltaTime;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
			{
				cameraPos.y -= speed * zoom * deltaTime;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
			{
				cameraPos.y += speed * zoom * deltaTime;
			}
		}
		
		window.clear({127, 127, 127, 255});
		drawToolbar(&window, &state, &font, &skeleton);
		cam.setCenter(cameraPos);
		view.setView(cam);
		view.clear({127, 127, 127});
		skeleton.draw(&view, &state);
		view.display();
		window.draw(sf::Sprite(view.getTexture()));
		window.display();
	}
	return 0;
}