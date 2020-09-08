#include "renderer.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Renderer::Renderer()
{
    // Setup glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to init GLAD\n";
	}
	
	text_pipeline = std::make_unique<TextPipeline>();
	image_pipeline = std::make_unique<ImagePipeline>();
}

Renderer::~Renderer()
{
}

void Renderer::render(nsc::object_registry *scene, Window *window, Camera *camera)
{
	// Get the list of objects for the pipelines
	auto width = window->get_width();
	auto height = window->get_height();
	auto proj = glm::ortho(0.0f, width * 1.f, 0.0f, height * 1.f, .001f, 100.f);
	auto view = camera->get_view();
	// auto bg = canvas->get_bg_color();
	auto bg = nsc::rendering::Color{ 1.f, 1.f, 1.f };

	glViewport(0, 0, width, height);
	glClearColor(bg.r, bg.g, bg.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	image_pipeline->render(scene->get_all<ImageDesc>(), proj, view);
	text_pipeline->render(scene->get_all<TextDesc>(), proj, view);
	text_pipeline->render(scene->get_all<RichTextDesc>(), proj, view);
}
