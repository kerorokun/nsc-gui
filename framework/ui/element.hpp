#pragma once

#include <cstdint>

#include "../core/event_handler.hpp"
#include "../core/registry.hpp"
#include "rectangle.hpp"

namespace nsc::ui {
class Element {
   public:
    Element(uint32_t id, const Rectangle &bounds) 
        : bounds(bounds) {
        this->id = id;
    }
    ~Element() {}

    template <typename Renderable, typename... Args>
    void add_renderable(Args ...args) {
        // Call the rendering context's rendering
    }

    template <typename Event, typename Connection>
    nsc::event_handle subscribe_on(Connection &&conn) {
        return event_handler.subscribe<Event>(std::move(conn));
    }

    template <typename Event>
    void unsubscribe_on(const nsc::event_handle handle) {
        return event_handler.unsubscribe(handle);
    }

    template <typename Event>
    void publish_on(const Event &e) {
        return event_handler.publish(e);
    }

    nsc::event_handler event_handler;
    Rectangle bounds;
    uint32_t id;
};
}
