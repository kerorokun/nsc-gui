#include "text_pipeline.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include "color.hpp"

TextPipeline::TextPipeline()
	: shader("shaders/text.vs", "shaders/text.fs")
{
	auto color = nsc::rendering::Color { 0.0f, 0.0f, 0.0f };
	shader.use();
	shader.set_vec3("color", color.r, color.g, color.b);
	
	float vertices[6][4] = {
		{ 0,      1.0f,    0.0f, 0.0f },            
		{ 0,      0,       0.0f, 1.0f },
		{ 1.0f,   0,       1.0f, 1.0f },

		{ 0,      1.0f,    0.0f, 0.0f },
		{ 1.0f,   0,       1.0f, 1.0f },
		{ 1.0f,   1.0f,    1.0f, 0.0f }
	};
	
	unsigned int VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
}

TextPipeline::~TextPipeline()
{
	
}

void TextPipeline::render(nsc::registry *descs, const glm::mat4 &proj, const glm::mat4 &view)
{
	shader.use();
	shader.set_mat4("projection", proj);
	shader.set_mat4("view", view);

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (auto desc : *(descs->get_group<TextDesc>())) {
		// TODO: Change this render based on the description
		render_centered(desc);
	}

	for (auto desc : *(descs->get_group<RichTextDesc>())) {
		render_rich_text(desc);
	}
}


void TextPipeline::render_text_description(const TextDesc &desc)
{
	auto bounds = desc.bounds;
	auto left = bounds.x;
	auto right = bounds.x + bounds.width;
	auto top = bounds.y + bounds.height;
	auto bottom = bounds.y;

	// TODO: More complicated rendering but for now, just render a single line
	auto x = left;
	auto y = bottom;
	auto text = desc.msg;
	auto font = desc.font;
	auto font_size = desc.font_size;

	auto color = desc.color;
	shader.set_vec3("color", color.r, color.g, color.b);

	auto threshold = font_size < 20 ? 0.15f : 0.0f;
	shader.set_float("threshold", threshold);
	shader.set_float("texture_width", font->texture->width);
	shader.set_float("texture_height", font->texture->height);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, font->texture->texture);
	glBindVertexArray(VAO);

	for (const auto &c : text) {
		render_one_char(font, font_size, c, &x, &y);
	}
}

void TextPipeline::render_one_char(Font *font, int size, char c, float *x, float *y) {
	if (c != ' ') {
		auto bearing_x = font->char_data[c].plane_left * size;
		auto bearing_y = font->char_data[c].plane_bottom * size;
		auto xpos = *x + bearing_x;
		auto ypos = *y + bearing_y;

		auto width = font->char_data[c].plane_right - font->char_data[c].plane_left;
		auto height = font->char_data[c].plane_top - font->char_data[c].plane_bottom;

		auto model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(xpos, ypos, 0.0f));
		model = glm::scale(model, glm::vec3(width * size, height * size, 1.0f));

		shader.set_mat4("model", model);
		shader.set_float("left", font->char_data[c].texture_left);
		shader.set_float("right", font->char_data[c].texture_right);
		shader.set_float("top", font->char_data[c].texture_top);
		shader.set_float("bottom", font->char_data[c].texture_bottom);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	*x += font->char_data[c].advance * size;
}


void TextPipeline::render_centered(const TextDesc &desc)
{
	auto bounds = desc.bounds;
	auto left = bounds.x;
	auto right = bounds.x + bounds.width;
	auto top = bounds.y + bounds.height;
	auto bottom = bounds.y;
	auto text = desc.msg;
	auto font = desc.font;
	auto font_size = desc.font_size;

	// Calculate the width and the height
	auto text_height = 0.f;
	auto text_width = 0.f;
	auto size = desc.font_size;

	for (const auto &c : text) {
		if (c != ' ') {
			text_height = std::max(text_height,
								   (float)(font->char_data[c].plane_top - font->char_data[c].plane_bottom) * (float)size);
		}
	
		text_width += font->char_data[c].advance * size;
	}

	// Now that this is calculated, i need to actually do the rendering
	auto center_x = (left + right) / 2.f;
	auto center_y = (bottom + top) / 2.f;

	// TODO: More complicated rendering but for now, just render a single line
	auto x = center_x - (text_width / 2.f);
	auto y = center_y - (text_height / 2.f);

	auto color = desc.color;
	shader.set_vec3("color", color.r, color.g, color.b);

	auto threshold = 8.0 * ((float)font_size) / 64.0;
	shader.set_float("distance_factor", threshold);
	shader.set_float("texture_width", font->texture->width);
	shader.set_float("texture_height", font->texture->height);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, font->texture->texture);
	glBindVertexArray(VAO);

	for (const auto &c : text) {
		render_one_char(font, font_size, c, &x, &y);
	}
}

void TextPipeline::render_rich_text(const RichTextDesc &desc)
{
	auto bounds = desc.bounds;
	auto left = bounds.x;
	auto right = bounds.x + bounds.width;
	auto top = bounds.y + bounds.height;
	auto bottom = bounds.y;
	
	auto chunks = desc.text_chunks;

	// Calculate the width and the height
	auto x = left;
	auto y = bottom;
	
	if (desc.is_centered_x || desc.is_centered_y) {
		auto text_height = 0.f;
		auto text_width = 0.f;

		for (auto &text_desc : chunks) {
			auto font = text_desc.font;
			auto font_size = text_desc.font_size;
		
			for (const auto &c : text_desc.msg) {
				if (c != ' ') {
					text_height = std::max(text_height,
										   (float)(font->char_data[c].plane_top - font->char_data[c].plane_bottom) * (float)font_size);
				}
	
				text_width += font->char_data[c].advance * (float)font_size;
			}
		}

		// Now that this is calculated, i need to actually do the rendering
		auto center_x = (left + right) / 2.f;
		auto center_y = (bottom + top) / 2.f;

		// TODO: More complicated rendering but for now, just render a single line
		if (desc.is_centered_x) {
			x = center_x - (text_width / 2.f);
		}
		if (desc.is_centered_y) {
			y = center_y - (text_height / 2.f);
		}
	}

	glBindVertexArray(VAO);
	
	for (auto &text_desc : chunks) {
		auto font = text_desc.font;
		auto font_size = text_desc.font_size;
		auto color = text_desc.color;
		shader.set_vec3("color", color.r, color.g, color.b);

		auto threshold = 8.0 * ((float)font_size) / 64.0;
		shader.set_float("distance_factor", threshold);
		shader.set_float("texture_width", font->texture->width);
		shader.set_float("texture_height", font->texture->height);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, font->texture->texture);

		
		for (const auto &c : text_desc.msg) {
			render_one_char(font, font_size, c, &x, &y);
		}
	}
}
