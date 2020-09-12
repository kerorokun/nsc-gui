#ifndef TEXT_PIPELINE_HPP_
#define TEXT_PIPELINE_HPP_

#include "shader.hpp"
#include <vector>
#include "font.hpp"
#include "descriptions.hpp"
#include "../core/registry.hpp"

class TextPipeline
{
public:
	TextPipeline();
	~TextPipeline();

	void render(nsc::registry *descs, const glm::mat4 &proj, const glm::mat4 &view);

	void render(const std::vector<TextDesc> *descs,
				const glm::mat4 &proj, const glm::mat4 &view);

	void render(const std::vector<RichTextDesc> *descs,
				const glm::mat4 &proj, const glm::mat4 &view);
private:
	void render_text_description(const TextDesc &desc);
	void render_one_char(Font *font, int size, char c, float *x, float *y);

	void render_centered(const TextDesc &desc);
	void render_rich_text(const RichTextDesc &desc);

	unsigned int VAO;
	Shader shader;
};


#endif
