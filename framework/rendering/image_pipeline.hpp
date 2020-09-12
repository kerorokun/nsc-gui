#ifndef IMAGE_PIPELINE_HPP_
#define IMAGE_PIPELINE_HPP_

#include <vector>

#include "../core/registry.hpp"
#include "descriptions.hpp"
#include "shader.hpp"

class ImagePipeline {
   public:
    ImagePipeline();
    ~ImagePipeline();

    void render(nsc::registry *descs,
                const glm::mat4 &proj, const glm::mat4 &view);

   private:
    unsigned int VAO;
    Shader shader;
};

#endif
