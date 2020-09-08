#ifndef FONT_CATALOG_HPP
#define FONT_CATALOG_HPP

#include <unordered_map>
#include <string>
#include "font.hpp"
#include "texture_catalog.hpp"
#include "msdf_wrapper.hpp"
#include "descriptions.hpp"

class FontCatalog
{
public:
	FontCatalog();
	~FontCatalog();
	
	Font * load_font(const std::string &path);
	void release_font(const std::string &name);

	TextDesc create(const std::string &msg, const std::string &font_path, size_t font_size, const nsc::rendering::Color &color, const nsc::ui::Rectangle &bounds);

private:
	std::string calculate_out_name(const std::string &path);
	
	std::unordered_map<std::string, Font> path_to_font;
	MsdfWrapper wrapper;
	TextureCatalog texture_loader;
};


#endif
