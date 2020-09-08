#ifndef TEXTURE_CATALOG_HPP
#define TEXTURE_CATALOG_HPP

#include <unordered_map>
#include <string>
#include <utility>
#include "texture.hpp"
#include "descriptions.hpp"
#include "../ui/rectangle.hpp"

class TextureCatalog
{
public:
	TextureCatalog();
	~TextureCatalog();

	nsc::rendering::Texture *load_texture(const std::string &path, bool alpha=false);
	void release_texture(const std::string &path);

	ImageDesc create(const std::string &image_path, const nsc::rendering::Color &color, const nsc::ui::Rectangle &bounds);
	
private:
	std::unordered_map<std::string, nsc::rendering::Texture> path_to_texture;
};

#endif
