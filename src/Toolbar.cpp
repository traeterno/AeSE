#define ToolbarImpl
#include "Toolbar.hpp"
#include <SFML/Graphics.hpp>

sf::String inputBox;

enum TextAnchor { Left, Right, Center, Top, Bottom};

const int hoverClr = 200;

sf::String pageToName(Page page)
{
	switch (page)
	{
		case File: return "File"; break;
		case Textures: return "Textures"; break;
		case Bones: return "Bones"; break;
		case Animations: return "Animations"; break;
		case TexDetails: return "Texture info"; break;
		case BoneDetails: return "Bone info"; break;
		case AnimDetails: return "Animation info"; break;
		default: return "???"; break;
	}
}

bool isMainPage(Page current)
{
	return (
		current == File ||
		current == Textures ||
		current == Bones ||
		current == Animations
	);
}

Page togglePage(Page current, bool next)
{
	switch (current)
	{
		case File: return next ? Textures : Animations; break;
		case Textures: return next ? Bones : File; break;
		case Bones: return next ? Animations : Textures; break;
		case Animations: return next ? File : Bones; break;
		case TexDetails: return Textures; break;
		case BoneDetails: return Bones; break;
		case AnimDetails: return Animations; break;
		default: return Page::File; break;
	}
}

sf::Text drawText(sf::Font* font, sf::String str, sf::Vector2f pos, TextAnchor x = Left, TextAnchor y = Top)
{
	sf::Text out(*font, str);
	out.setPosition(pos);
	out.setFillColor(sf::Color::White);
	sf::Vector2f origin = out.getGlobalBounds().size;
	switch (x)
	{
		case TextAnchor::Center: origin.x *= 0.5; break;
		case TextAnchor::Right: origin.x += 8; break;
		default: origin.x = 0; break;
	}
	switch (y)
	{
		case TextAnchor::Center: origin.y *= 0.5; break;
		case TextAnchor::Bottom: origin.y += 16; break;
		default: origin.y = 0; break;
	}

	out.setOrigin(origin);
	return out;
}

void drawFilePage(sf::RenderWindow* window, State* state, sf::Font* font, float ox, float tw)
{
	if (state->project.empty())
	{
		window->draw(drawText(font, "No file opened", {ox + tw / 2, 30}, Center));
	}
	else
	{
		window->draw(drawText(font, "Current file:", {ox + tw / 2, 30}, Center));
		window->draw(drawText(font, state->project.filename().string(), {ox + tw / 2, 60}, Center));
	}

	auto open = drawText(font, "Open file", {ox + tw / 2, 120}, Center);
	auto save = drawText(font, "Save file", {ox + tw / 2, 150}, Center);
	auto create = drawText(font, "New file", {ox + tw / 2, 180}, Center);

	auto mPos = (sf::Vector2f)sf::Mouse::getPosition(*window);

	if (open.getGlobalBounds().contains(mPos))
	{
		open.setFillColor(sf::Color(hoverClr, hoverClr, hoverClr));
		if (state->mousePressed) { state->command = "file-open"; execute(""); }
	}
	if (save.getGlobalBounds().contains(mPos))
	{
		save.setFillColor(sf::Color(hoverClr, hoverClr, hoverClr));
		if (state->mousePressed) { state->command = "file-save"; execute(""); }
	}
	if (create.getGlobalBounds().contains(mPos))
	{
		create.setFillColor(sf::Color(hoverClr, hoverClr, hoverClr));
		if (state->mousePressed) { state->command = "file-create"; execute(""); }
	}

	window->draw(open);
	window->draw(save);
	window->draw(create);
}

void drawToolbar(sf::RenderWindow *window, State* state, sf::Font* font)
{
	auto ox = (float)window->getSize().x / 4 * 3;
	auto tw = (float)window->getSize().x / 4;
	auto wh = (float)window->getSize().y;
	sf::RectangleShape bg({(float)window->getSize().x / 4, (float)window->getSize().y});
	bg.setPosition({ox, 0});
	bg.setFillColor(sf::Color::Black);
	window->draw(bg);

	auto prevTab = drawText(font, "<--", {ox, 0});
	auto nextTab = drawText(font, "-->", {ox + tw, 0}, Right);
	auto mPos = (sf::Vector2f)sf::Mouse::getPosition(*window);
	if (prevTab.getGlobalBounds().contains(mPos))
	{
		prevTab.setFillColor(sf::Color(hoverClr, hoverClr, hoverClr));
		if (state->mousePressed) state->page = togglePage(state->page, false);
	}
	if (isMainPage(state->page) && nextTab.getGlobalBounds().contains(mPos))
	{
		nextTab.setFillColor(sf::Color(hoverClr, hoverClr, hoverClr));
		if (state->mousePressed) state->page = togglePage(state->page, true);
	}
	window->draw(prevTab);
	if (isMainPage(state->page)) window->draw(nextTab);
	window->draw(drawText(font, pageToName(state->page), {ox + tw / 2, 0}, Center));

	if (state->input)
	{
		if (state->pressedChar != 0 && state->pressedChar != 13)
		{
			if (state->pressedChar == 8)
			{
				inputBox = inputBox.substring(0, std::max(inputBox.getSize() - 1, 0ul));
			}
			else inputBox += sf::String((char32_t)state->pressedChar);
		}
		if (state->enter)
		{
			state->hint = "";
			state->input = false;
			execute(inputBox);
			inputBox = "";
		}

		window->draw(drawText(font, state->hint, {ox, wh - 30}, Left, Bottom));
		window->draw(drawText(font, inputBox, {ox, wh}, Left, Bottom));
	}

	switch (state->page)
	{
		case File: drawFilePage(window, state, font, ox, tw); break;
		default: break;
	}
}