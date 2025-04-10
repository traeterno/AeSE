#ifndef aeSkeleton
#define aeSkeleton

#include <vector>
#include <SFML/Graphics/Texture.hpp>

typedef std::vector<unsigned short> BonePath;

namespace sf { class RenderTarget; }

struct State;

struct Bone
{
	float length, angle;
	std::string name, texture;
	std::vector<Bone> children;
};

struct Texture { std::string name; sf::IntRect rect; };

class Skeleton
{
public:
	Skeleton();
	void load(State* state);
	void save(State* state);
	void updateTexture(State* state);
	void draw(sf::RenderTarget* target, State* state);
	Bone root;
	std::vector<Texture> textures;
	sf::Texture texture;
};

BonePath parsePath(Bone* main, std::string path);

Bone* getBone(Bone* root, BonePath path);

#endif