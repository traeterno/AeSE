#define ToolbarImpl
#include "Toolbar.hpp"
#include "Skeleton.hpp"
#include <SFML/Graphics.hpp>

sf::String inputBox;
Bone* currentBone;
Texture* currentTexture;

enum TextAnchor { Left, Right, Center, Top, Bottom};

const int hoverClr = 200;
const int height = 20;

sf::String pageToName(Page page)
{
	switch (page)
	{
		case File: return "File"; break;
		case Textures: return "Textures"; break;
		case Bones: return "Bones"; break;
		case Animations: return "Animations"; break;
		case Drawables: return "Visible bones"; break;
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
		current == Animations ||
		current == Drawables
	);
}

Page togglePage(Page current, bool next)
{
	switch (current)
	{
		case File: return next ? Bones : Drawables; break;
		case Bones: return next ? Textures : File; break;
		case Textures: return next ? Animations : Bones; break;
		case Animations: return next ? Drawables : Textures; break;
		case Drawables: return next ? File : Animations; break;
		case BoneDetails: return Bones; break;
		case AnimDetails: return Animations; break;
		default: return Page::File; break;
	}
}

sf::Text drawText(sf::Font* font, sf::String str, sf::Vector2f pos, TextAnchor x = Left, TextAnchor y = Top)
{
	sf::Text out(*font, str, height);
	out.setPosition({pos.x, pos.y * height});
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
		window->draw(drawText(font, "No file opened", {ox + tw / 2, 1}, Center));
	}
	else
	{
		window->draw(drawText(font, "Current file:", {ox + tw / 2, 1}, Center));
		window->draw(drawText(font, state->project.filename().string(), {ox + tw / 2, 2}, Center));
	}
	if (state->texture.empty())
	{
		window->draw(drawText(font, "No texture", {ox + tw / 2, 3}, Center));
	}
	else
	{
		window->draw(drawText(font, "Texture:", {ox + tw / 2, 3}, Center));
		window->draw(drawText(font, state->texture.filename().string(), {ox + tw / 2, 4}, Center));
	}

	auto open = drawText(font, "Open file", {ox + tw / 2, 6}, Center);
	auto save = drawText(font, "Save file", {ox + tw / 2, 7}, Center);
	auto create = drawText(font, "New file", {ox + tw / 2, 8}, Center);
	auto loadTex = drawText(font, "Load texture", {ox + tw / 2, 9}, Center);

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
	*y += 1;
	for (auto x: b->children)
	{
		iterateBones(window, ox, &x, layer + 1, font, y, state, path + b->name + "/");
	}
}

void drawBonesPage(sf::RenderWindow* window, State* state, Skeleton* s, sf::Font* font, float ox, float tw)
{
	currentBone = nullptr;
	float y = 2;
	iterateBones(window, ox, &s->root, 0, font, &y, state, "/");
}

void drawBoneDetailsPage(sf::RenderWindow* window, State* state, Skeleton* s, sf::Font* font, float ox, float tw)
{
	if (!currentBone) currentBone = getBone(&s->root, state->bonePath);
	auto mPos = (sf::Vector2f)sf::Mouse::getPosition(*window);

	window->draw(drawText(font, "Bone:", {ox + tw / 2, 1}, Center));
	window->draw(drawText(font, "Length:", {ox + tw / 2, 4}, Center));
	window->draw(drawText(font, "Angle:", {ox + tw / 2, 7}, Center));
	window->draw(drawText(font, "Visible bone:", {ox + tw / 2, 10}, Center));
	
	auto name = drawText(font, currentBone->name, {ox + tw / 2, 2}, Center);
	auto length = drawText(font, std::to_string(currentBone->length), {ox + tw / 2, 5}, Center);
	auto angle = drawText(font, std::to_string(currentBone->angle), {ox + tw / 2, 8}, Center);
	auto tex = drawText(font, currentBone->visible, {ox + tw / 2, 11}, Center);
	auto add = drawText(font, "New child", {ox + tw / 2, 13}, Center);
	auto destroy = drawText(font, "Delete bone", {ox + tw / 2, 14}, Center);
	
	handleButton(mPos, state, &name, "bone-name");
	handleButton(mPos, state, &length, "bone-length");
	handleButton(mPos, state, &angle, "bone-angle");
	handleButton(mPos, state, &tex, "bone-visible");
	handleButton(mPos, state, &add, "bone-add");
	handleButton(mPos, state, &destroy, "bone-destroy", "no");

	window->draw(name);
	window->draw(length);
	window->draw(angle);
	window->draw(tex);
	window->draw(add);
	window->draw(destroy);
}

void drawTexturesPage(sf::RenderWindow* window, State* state, Skeleton* s, sf::Font* font, float ox, float tw)
{
	currentTexture = nullptr;
	auto mPos = (sf::Vector2f)sf::Mouse::getPosition(*window);

	auto add = drawText(font, "New", {ox + tw / 2, 1}, Center);
	auto remove = drawText(font, "Delete", {ox + tw / 2, 2}, Center);

	handleButton(mPos, state, &add, "texture-add");
	handleButton(mPos, state, &remove, "texture-remove");

	window->draw(add);
	window->draw(remove);

	for (int i = 0; i < s->textures.size(); i++)
	{
		auto t = &s->textures[i];
		auto x = std::to_string(t->origin.x); x = x.substr(0, x.find(".") + 2);
		auto y = std::to_string(t->origin.y); y = y.substr(0, y.find(".") + 2);
		auto name = drawText(font, t->name, {ox, 4.0f + i});
		auto rect = drawText(font,
			std::to_string(t->rect.position.x) + " " +
			std::to_string(t->rect.position.y) + " " +
			std::to_string(t->rect.size.x) + " " +
			std::to_string(t->rect.size.y) + "/" +
			x + " " + y,
			{ox + tw, 4.0f + i},
			Right
		);
		handleButton(mPos, state, &name, "texture-name", "current-" + t->name);
		handleButton(mPos, state, &rect, "texture-rect", "current-" + t->name);
		window->draw(name);
		window->draw(rect);
	}
}

void drawDrawablesPage(sf::RenderWindow* window, State* state, Skeleton* s, sf::Font* font, float ox, float tw)
{
	currentTexture = nullptr;
	auto mPos = (sf::Vector2f)sf::Mouse::getPosition(*window);

	auto add = drawText(font, "New", {ox + tw / 2, 1}, Center);
	auto remove = drawText(font, "Delete", {ox + tw / 2, 2}, Center);

	handleButton(mPos, state, &add, "drawable-add");
	handleButton(mPos, state, &remove, "drawable-remove");

	window->draw(add);
	window->draw(remove);

	for (int i = 0; i < s->visible.size(); i++)
	{
		auto v = &s->visible[i];
		auto name = drawText(font, v->name, {ox, 4.0f + i});
		auto rect = drawText(font, v->texture + "/" + std::to_string(v->layer), {ox + tw, 4.0f + i}, Right);
		handleButton(mPos, state, &name, "drawable-name", "current-" + v->name);
		handleButton(mPos, state, &rect, "drawable-texture", "current-" + v->name);
		window->draw(name);
		window->draw(rect);
	}
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

		window->draw(drawText(font, state->hint, {ox, (wh - height) / height}, Left, Bottom));
		window->draw(drawText(font, inputBox, {ox, wh / height}, Left, Bottom));

		return;
	}

	switch (state->page)
	{
		case File: drawFilePage(window, state, font, ox, tw); break;
		case Bones: drawBonesPage(window, state, s, font, ox, tw); break;
		case BoneDetails: drawBoneDetailsPage(window, state, s, font, ox, tw); break;
		case Textures: drawTexturesPage(window, state, s, font, ox, tw); break;
		case Drawables: drawDrawablesPage(window, state, s, font, ox, tw); break;
		default: break;
	}
}