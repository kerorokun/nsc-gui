#include "image_pipeline.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

ImagePipeline::ImagePipeline()
	: shader("shaders/basic.vs", "shaders/basic.fs")
{
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

ImagePipeline::~ImagePipeline()
{
}

void ImagePipeline::render(const std::vector<ImageDesc> *descs,
						   const glm::mat4 &proj, const glm::mat4 &view)
{
	shader.use();
	shader.set_mat4("projection", proj);
	shader.set_mat4("view", view);

	for (auto desc : *descs) {
		auto bounds = desc.bounds;
		
		auto model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(bounds.x, bounds.y, 0.0f));
		model = glm::scale(model, glm::vec3(bounds.width, bounds.height, 1.0f));
		shader.set_mat4("model", model);

		auto color = desc.color;
		shader.set_float("r", color.r);
		shader.set_float("g", color.g);
		shader.set_float("b", color.b);
		shader.set_float("a", color.a);
	
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, desc.texture->texture);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}
