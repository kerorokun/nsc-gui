#include "font_catalog.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

inline bool file_exists (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}

FontCatalog::FontCatalog()
{
}

FontCatalog::~FontCatalog()
{
}

Font * FontCatalog::load_font(const std::string &path)
{
	if (!path_to_font.count(path)) {
		auto out = calculate_out_name(path);
		auto texture_name = out + ".msdfnt1";
		auto csv_name = out + ".msdfnt2";

		// Check if both files exist
		if (!file_exists(texture_name) || !file_exists(csv_name)) {
			wrapper.load_font(path, texture_name, csv_name);
		}
		
		auto font = Font {};
		font.texture = texture_loader.load_texture(texture_name);
		font.max_height = 0.0f;

		// Load the metadata
		std::ifstream metadata_file(csv_name);
		std::string line;
	
		while(std::getline(metadata_file, line)) {
			Character c;
			c.populate_from_line(line);
			font.char_data[c.c] = c;

			float height = c.plane_top - c.plane_bottom;
			font.max_height = std::max(font.max_height, height);
		}
		path_to_font[path] = font;
	}
	return &path_to_font[path];
}

std::string FontCatalog::calculate_out_name(const std::string &path)
{
	auto out_name = path;
	const auto last_slash_index = path.find_last_of("\\/");
	if (std::string::npos != last_slash_index) {
		out_name.erase(0, last_slash_index + 1);
	}

    // Remove extension if present.
	const auto period_index = out_name.rfind('.');
	if (std::string::npos != period_index) {
		out_name.erase(period_index);
	}

	return ".\\fonts-cache\\" + out_name;
}

void FontCatalog::release_font(const std::string &name)
{
	// TODO: Implement this
	// NOTE: This will likely require you to store the texture_loader
}

TextDesc FontCatalog::create(const std::string &msg,
							 const std::string &font_path, size_t font_size,
							 const nsc::rendering::Color &color,
							 const nsc::ui::Rectangle &bounds)
{
	auto font = load_font(font_path);
	return TextDesc { font, msg, color, font_size, bounds };
}
