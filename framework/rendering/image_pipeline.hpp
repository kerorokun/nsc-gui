#ifndef IMAGE_PIPELINE_HPP_
#define IMAGE_PIPELINE_HPP_

#include "shader.hpp"
#include <vector>
#include "descriptions.hpp"

class ImagePipeline
{
public:
	ImagePipeline();
	~ImagePipeline();

	void render(const std::vector<ImageDesc> *descs,
				const glm::mat4 &proj, const glm::mat4 &view);

private:
	unsigned int VAO;
	Shader shader;
};

#endif
