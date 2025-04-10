#define ToolbarImpl
#include "Toolbar.hpp"
#include "Skeleton.hpp"
#include <SFML/Graphics.hpp>

sf::String inputBox;
Bone* currentBone;

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
		case BoneDetails: currentBone = nullptr; return Bones; break;
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

void handleButton(sf::Vector2f mPos, State* state, sf::Text* txt, sf::String cmd, sf::String args = "")
{
	if (txt->getGlobalBounds().contains(mPos))
	{
		txt->setFillColor(sf::Color(hoverClr, hoverClr, hoverClr));
		if (state->mousePressed) { state->command = cmd; execute(args); }
	}
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
	if (state->texture.empty())
	{
		window->draw(drawText(font, "No texture", {ox + tw / 2, 90}, Center));
	}
	else
	{
		window->draw(drawText(font, "Texture:", {ox + tw / 2, 90}, Center));
		window->draw(drawText(font, state->texture.filename().string(), {ox + tw / 2, 120}, Center));
	}

	auto open = drawText(font, "Open file", {ox + tw / 2, 180}, Center);
	auto save = drawText(font, "Save file", {ox + tw / 2, 210}, Center);
	auto create = drawText(font, "New file", {ox + tw / 2, 240}, Center);
	auto loadTex = drawText(font, "Load texture", {ox + tw / 2, 270}, Center);

	auto mPos = (sf::Vector2f)sf::Mouse::getPosition(*window);

	handleButton(mPos, state, &open, "file-open");
	handleButton(mPos, state, &save, "file-save", "btn");
	handleButton(mPos, state, &create, "file-create");
	handleButton(mPos, state, &loadTex, "file-loadTex");

	window->draw(open);
	window->draw(save);
	window->draw(create);
	window->draw(loadTex);
}

void iterateBones(sf::RenderWindow* window, float ox, Bone* b, int layer, sf::Font* font, float* y, State* state, sf::String path)
{
	auto entry = drawText(font, b->name, {ox + layer * 16, *y});
	auto mPos = (sf::Vector2f)sf::Mouse::getPosition(*window);
	handleButton(mPos, state, &entry, "bone-select", path + b->name + "/");
	window->draw(entry);
	*y += 30;
	for (auto x: b->children)
	{
		iterateBones(window, ox, &x, layer + 1, font, y, state, path + b->name + "/");
	}
}

void drawBonesPage(sf::RenderWindow* window, State* state, Skeleton* s, sf::Font* font, float ox, float tw)
{
	float y = 60;
	iterateBones(window, ox, &s->root, 0, font, &y, state, "/");
}

void drawBoneDetailsPage(sf::RenderWindow* window, State* state, Skeleton* s, sf::Font* font, float ox, float tw)
{
	if (!currentBone) currentBone = getBone(&s->root, state->bonePath);
	auto mPos = (sf::Vector2f)sf::Mouse::getPosition(*window);

	window->draw(drawText(font, "Bone:", {ox + tw / 2, 30}, Center));
	window->draw(drawText(font, "Length:", {ox + tw / 2, 120}, Center));
	window->draw(drawText(font, "Angle:", {ox + tw / 2, 210}, Center));
	window->draw(drawText(font, "Texture:", {ox + tw / 2, 300}, Center));
	
	auto name = drawText(font, currentBone->name, {ox + tw / 2, 60}, Center);
	auto length = drawText(font, std::to_string(currentBone->length), {ox + tw / 2, 150}, Center);
	auto angle = drawText(font, std::to_string(currentBone->angle), {ox + tw / 2, 240}, Center);
	auto tex = drawText(font, currentBone->texture, {ox + tw / 2, 330}, Center);
	auto add = drawText(font, "New child", {ox + tw / 2, 390}, Center);
	auto destroy = drawText(font, "Delete bone", {ox + tw / 2, 420}, Center);
	
	handleButton(mPos, state, &name, "bone-name");
	handleButton(mPos, state, &length, "bone-length");
	handleButton(mPos, state, &angle, "bone-angle");
	handleButton(mPos, state, &tex, "bone-texture");
	handleButton(mPos, state, &add, "bone-add");
	handleButton(mPos, state, &destroy, "bone-destroy", "no");

	window->draw(name);
	window->draw(length);
	window->draw(angle);
	window->draw(tex);
	window->draw(add);
	window->draw(destroy);
}

void drawToolbar(sf::RenderWindow *window, State* state, sf::Font* font, Skeleton* s)
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
			if (state->pressedChar == 27)
			{
				inputBox = "";
				state->input = false;
				state->hint = "";
			}
			else if (state->pressedChar == 8)
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

		return;
	}

	switch (state->page)
	{
		case File: drawFilePage(window, state, font, ox, tw); break;
		case Bones: drawBonesPage(window, state, s, font, ox, tw); break;
		case BoneDetails: drawBoneDetailsPage(window, state, s, font, ox, tw); break;
		default: break;
	}
}