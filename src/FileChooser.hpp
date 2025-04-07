#ifndef aeFileChoosed
#define aeFileChoosed

#include <filesystem>

namespace sf { class RenderWindow; class Font; }

std::filesystem::path openFile(sf::RenderWindow* window, sf::Font* font, std::string filetype);

#endif