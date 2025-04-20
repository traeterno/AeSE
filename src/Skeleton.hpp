#ifndef aeSkeleton
#define aeSkeleton

#include <vector>
#include <SFML/Graphics/Texture.hpp>

typedef std::vector<unsigned short> BonePath;

namespace sf { class RenderTarget; }
namespace pugi { class xml_node; }

struct State;

class Skeleton;

struct Bone
{
	float length, angle;
	std::string name, visible;
	std::vector<Bone> children;
	void update(Skeleton* s, float x, float y, float a);
	void toNode(pugi::xml_node node);
	void parse(pugi::xml_node node);
};

struct Texture { std::string name; sf::IntRect rect; sf::Vector2f origin; };

struct VisibleBone { std::string name, texture; float x, y, a; int layer; };

struct Frame
{
	float duration;
	float angle;
};

struct Change
{
	BonePath path;
	float currentTime;
	int currentFrame;
	std::vector<Frame> frames;
};

struct Animation
{
	std::string name;
	bool repeat;
	std::vector<Change> changes;
	void update(State* state, Skeleton* s);
};

class Skeleton
{
public:
	Skeleton();
	void load(State* state);
	void save(State* state);
	void updateTexture(State* state);
	void update(State* state);
	void draw(sf::RenderTarget* target, State* state);
	Texture* getTexture(std::string name);
	VisibleBone* getVisible(std::string name);
	Bone root;
	std::vector<Texture> textures;
	std::vector<VisibleBone> visible;
	std::vector<Animation> anims;
	sf::Texture texture;
};

BonePath parsePath(Bone* main, std::string path);

Bone* getBone(Bone* root, BonePath path);

#endif