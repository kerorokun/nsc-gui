#ifndef DESCRIPTIONS_HPP_
#define DESCRIPTIONS_HPP_

#include <string>
#include "texture.hpp"
#include "../ui/rectangle.hpp"
#include "font.hpp"
#include "color.hpp"

struct ImageDesc
{
	nsc::rendering::Texture* texture;
	nsc::rendering::Color color;
	nsc::ui::Rectangle bounds;
};

struct TextDesc
{
	Font *font;
	std::string msg;
	nsc::rendering::Color color;
	size_t font_size;
	nsc::ui::Rectangle bounds;
};

struct RichTextDesc
{
	RichTextDesc() {
		is_centered_x = false;
		is_centered_y = false;
	}

	RichTextDesc(const nsc::ui::Rectangle &bounds, bool is_centered_x, bool is_centered_y) {
		this->bounds = bounds;
		this->is_centered_x = is_centered_x;
		this->is_centered_y = is_centered_y;
	}

	~RichTextDesc() {
	}

	nsc::ui::Rectangle bounds;
	bool is_centered_x;
	bool is_centered_y;
	std::vector<TextDesc> text_chunks;
};

#endif
