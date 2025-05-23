#ifndef aeToolbar
#define aeToolbar

#include <filesystem>
#include <vector>

namespace sf { class RenderWindow; class Font; class String; }

class Skeleton;

enum Page
{
	File,
	Textures,
	Bones,
	BoneDetails,
	Animations,
	AnimDetails,
	Drawables
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
	std::filesystem::path texture;
	std::vector<unsigned short> bonePath;
	std::string currentTexture;
	std::string currentDrawable;
	int currentAnimation;
	float deltaTime;
	bool activeAnimation;
};

struct Button { std::string text, cmd; };

void drawToolbar(sf::RenderWindow* window, State* state, sf::Font* font, Skeleton* s);
void execute(std::string cmd);

#endif