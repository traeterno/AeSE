#include "Skeleton.hpp"
#include <SFML/Graphics.hpp>

#include "Toolbar.hpp"
#include <pugixml.hpp>
#include <math.h>

void Bone::update(Skeleton* s, float x, float y, float a)
{
	auto vb = s->getVisible(visible);
	if (vb)
	{
		vb->a = a + angle;
		vb->x = x;
		vb->y = y;
	}
	auto x2 = x + cos(sf::degrees(a + angle + 90).asRadians()) * length;
	auto y2 = y + sin(sf::degrees(a + angle + 90).asRadians()) * length;
	for (int i = 0; i < children.size(); i++)
	{
		children[i].update(s, x2, y2, a + angle);
	}
}

void Bone::toNode(pugi::xml_node node)
{
	node.append_attribute("name") = name;
	node.append_attribute("visible") = visible;
	node.append_attribute("length") = length;
	node.append_attribute("angle") = angle;
	for (auto child: children)
	{
		child.toNode(node.append_child("bone"));
	}
}

void Bone::parse(pugi::xml_node node)
{
	name = node.attribute("name").as_string();
	visible = node.attribute("visible").as_string();
	length = node.attribute("length").as_float();
	angle = node.attribute("angle").as_float();
	for (auto child: node.children())
	{
		if (std::string(child.name()) == "bone")
		{
			Bone b;
			b.parse(child);
			children.push_back(b);
		}
	}
}

void Animation::update(State* state, Skeleton* s)
{
	for (int i = 0; i < this->changes.size(); i++)
	{
		auto c = &changes[i];
		c->currentTime += state->deltaTime;
		
		auto cf = &c->frames[c->currentFrame];
		if (c->currentTime > cf->duration)
		{
			c->currentTime = 0;
			c->currentFrame++;
			if (c->currentFrame == c->frames.size()) { c->currentFrame = 0; }
			cf = &c->frames[c->currentFrame];
		}

		auto nf = &c->frames[c->currentFrame + 1];
		if (c->currentFrame >= c->frames.size() - 1)
		{
			nf = &c->frames[0];
		}

		auto b = getBone(&s->root, c->path);

		b->angle = cf->angle + (nf->angle - cf->angle) * (c->currentTime / cf->duration);
	}
}

Skeleton::Skeleton()
{
	texture = sf::Texture();
	root = {
		.length = 0,
		.angle = 0,
		.name = "root",
		.visible = "none",
		.children = std::vector<Bone>()
	};
}

void Skeleton::save(State* state)
{
	pugi::xml_document doc;
	auto save = doc.append_child("skeleton");
	save.append_attribute("texture") = state->texture.filename().string();
	root.toNode(save.append_child("bone"));
	for (int i = 0; i < textures.size(); i++)
	{
		auto t = &textures[i];
		auto tex = save.append_child("texture");
		tex.append_attribute("name") = t->name;
		tex.append_attribute("rect") =
			std::to_string(t->rect.position.x) + " " +
			std::to_string(t->rect.position.y) + " " +
			std::to_string(t->rect.size.x) + " " +
			std::to_string(t->rect.size.y);
		tex.append_attribute("origin") =
			std::to_string(t->origin.x) + " " +
			std::to_string(t->origin.y);
	}
	for (int i = 0; i < visible.size(); i++)
	{
		auto v = &visible[i];
		auto vb = save.append_child("visible");
		vb.append_attribute("name") = v->name;
		vb.append_attribute("texture") = v->texture;
		vb.append_attribute("layer") = v->layer;
	}
	doc.save_file(state->project.string().c_str());
}

void Skeleton::load(State* state)
{
	*this = Skeleton();
	pugi::xml_document doc;
	doc.load_file(state->project.string().c_str());
	auto skeleton = doc.child("skeleton");
	state->texture = skeleton.attribute("texture").as_string();
	if (!texture.loadFromFile(state->texture)) { state->texture = ""; }

	for (auto node: skeleton.children())
	{
		std::string name = node.name();
		if (name == "bone") { root.parse(node); }
		else if (name == "texture")
		{
			Texture t;
			t.name = node.attribute("name").as_string();
			
			std::string rect = node.attribute("rect").as_string();
			auto x = rect.substr(0, rect.find(" "));
			rect = rect.substr(rect.find(" ") + 1);
			auto y = rect.substr(0, rect.find(" "));
			rect = rect.substr(rect.find(" ") + 1);
			auto w = rect.substr(0, rect.find(" "));
			rect = rect.substr(rect.find(" ") + 1);
			t.rect = sf::IntRect(
				{std::stoi(x), std::stoi(y)},
				{std::stoi(w), std::stoi(rect)}
			);
			
			std::string origin = node.attribute("origin").as_string();
			t.origin = {
				std::stof(origin.substr(0, origin.find(" "))),
				std::stof(origin.substr(origin.find(" ") + 1))
			};

			textures.push_back(t);
		}
		else if (name == "visible")
		{
			VisibleBone vb;
			vb.name = node.attribute("name").as_string();
			vb.texture = node.attribute("texture").as_string();
			vb.layer = node.attribute("layer").as_int();
			visible.push_back(vb);
		}
	}
}

void Skeleton::updateTexture(State* state)
{
	if (!texture.loadFromFile(state->texture)) { state->texture = ""; }
}

void Skeleton::update(State* state)
{
	if (anims.size() && state->activeAnimation) anims[state->currentAnimation].update(state, this);
	root.update(this, 100, 100, 0);
}

void Skeleton::draw(sf::RenderTarget* target, State* state)
{
	if (state->texture == "") return;

	update(state);

	sf::Sprite spr(texture);
	for (int layer = 0; layer < 3; layer++)
	{
		for (int i = 0; i < visible.size(); i++)
		{
			if (visible[i].layer != layer) { continue; }
			auto tex = getTexture(visible[i].texture);
			if (!tex) continue;
			spr.setTextureRect(tex->rect);
			spr.setPosition({visible[i].x, visible[i].y});
			spr.setRotation(sf::degrees(visible[i].a));
			spr.setOrigin(tex->origin);
			target->draw(spr);
		}
	}
}

Texture* Skeleton::getTexture(std::string name)
{
	for (int i = 0; i < textures.size(); i++)
	{
		if (textures[i].name == name) return &textures[i];
	}
	return nullptr;
}

VisibleBone* Skeleton::getVisible(std::string name)
{
	for (int i = 0; i < visible.size(); i++)
	{
		if (visible[i].name == name) return &visible[i];
	}
	return nullptr;
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