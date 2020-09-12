#pragma once

#include "../core/registry.hpp"
#include "color.hpp"

namespace nsc::rendering {

class Canvas {
   public:
    Canvas() {}

    ~Canvas() {
        // TODO: Clear the primitives (maybe not needed given the destructor)
    }

    template <typename Primitive, typename... Args>
    nsc::obj_handle create_primitive(Args&&... args) {
        return primitives.create<Primitive>(std::forward<Args>(args)...);
    }

    template <typename Primitive, typename Loader, typename... Args>
    nsc::obj_handle create_primitive(Loader *loader, Args&&... args) {
        return primitives.create<Primitive, Loader>(loader, std::forward<Args>(args)...)
    }

    template <typename Primitive>
    void remove_primitive(const nsc::obj_handle &handle) {
        primitives.remove<Primitive>(handle);
    }

    void set_background_color(const nsc::rendering::Color color) {
        bg_color = color;
    }

   private:
    nsc::registry primitives;
    nsc::rendering::Color bg_color;
};

}  // namespace nsc::rendering