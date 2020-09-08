#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include <glad/glad.h>
#include <memory>

#include "../window/window.hpp"
#include "camera.hpp"
#include "text_pipeline.hpp"
#include "image_pipeline.hpp"
#include "../core/object_registry.hpp"

class Renderer
{
public:
	Renderer();
	~Renderer();

	void render(nsc::object_registry *scene, Window *window, Camera *camera);
private:
	std::unique_ptr<TextPipeline> text_pipeline;
	std::unique_ptr<ImagePipeline> image_pipeline;
};

#endif
