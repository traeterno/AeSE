#include "Skeleton.hpp"
#include <SFML/Graphics.hpp>

#include "Toolbar.hpp"
#include <pugixml.hpp>

Skeleton::Skeleton()
{
	texture = sf::Texture();
	root = {
		.length = 0,
		.angle = 0,
		.name = "root",
		.texture = "",
		.children = std::vector<Bone>()
	};
}

void Skeleton::save(State* state)
{
	pugi::xml_document doc;
	auto root = doc.append_child("skeleton");
	root.append_attribute("texture") = state->texture.filename().string();
	doc.save_file(state->project.string().c_str());
}

void Skeleton::load(State* state)
{
	pugi::xml_document doc;
	doc.load_file(state->project.string().c_str());
	auto root = doc.child("skeleton");
	state->texture = root.attribute("texture").as_string();
	if (!texture.loadFromFile(state->texture)) { state->texture = ""; }
}

void Skeleton::updateTexture(State* state)
{
	if (!texture.loadFromFile(state->texture)) { state->texture = ""; }
}

void Skeleton::draw(sf::RenderTarget* target, State* state)
{
	if (state->texture == "") return;
	sf::Sprite tester(texture);
	target->draw(tester);
}

BonePath parsePath(Bone* current, std::string path)
{
	BonePath out;
	path.erase(0, path.find("/", 1) + 1);
	while (path.find("/") != std::string::npos)
	{
		auto name = path.substr(0, path.find("/"));
		bool found = false;
		for (int i = 0; i < current->children.size(); i++)
		{
			if (current->children[i].name == name) { out.push_back(i); found = true; current = &current->children[i]; break; }
		}
		if (!found) return {};
		path.erase(0, path.find("/") + 1);
	}
	return out;
}

Bone* getBone(Bone* root, BonePath path)
{
	if (path.empty()) { return root; }
	auto i = path[0];
	path.erase(path.begin());
	return getBone(&root->children[i], path);
}