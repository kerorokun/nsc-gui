#pragma once

#include <vector>

#include "../rendering/canvas.hpp"
#include "element.hpp"

namespace nsc::ui {

class Scene {
   public:
    Scene() {}
    ~Scene() {}

   private:
    std::vector<Element> elements;
};

}  // namespace nsc::ui