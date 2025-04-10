#ifndef aeToolbar
#define aeToolbar

#include <filesystem>

namespace sf { class RenderWindow; class Font; class String; }

enum Page
{
	File,
	Textures,
	Bones,
	Animations,
	TexDetails,
	BoneDetails,
	AnimDetails
};

struct State
{
	std::filesystem::path project;
	Page page;
	bool input;
	bool mousePressed;
	unsigned int pressedChar;
	bool enter;
	std::string command;
	std::string hint;
};

struct Button { std::string text, cmd; };

void drawToolbar(sf::RenderWindow* window, State* state, sf::Font* font);
void execute(std::string cmd);

#endif