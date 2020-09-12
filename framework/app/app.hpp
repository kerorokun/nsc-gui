#ifndef APP_HPP_
#define APP_HPP_

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "../rendering/camera.hpp"
#include "../core/registry.hpp"
#include "../ui/element.hpp"
#include "../core/event_handler.hpp"
#include "../ui/events.hpp"
#include "../rendering/renderer.hpp"
#include "../window/window.hpp"

class App {
   public:
    App(int width, int height);
    ~App();

    void start();
    void add(nsc::ui::Element *element);
    void remove(nsc::ui::Element *element);
    void process_callbacks();

    void set_background_color(const nsc::rendering::Color &color);

    template <typename Event, typename Connection>
    nsc::event_handle subscribe(Connection &&conn) {
        return event_handler.subscribe<Event>(std::move(conn));
    }

    template <typename Event>
    void unsubscribe(const nsc::event_handle handle) {
        return event_handler.unsubscribe(handle);
    }

   protected:
    virtual void update();
    virtual void render();
    float get_delta_time();
    std::unique_ptr<nsc::registry> canvas;

   private:
    void main_loop();
    void handle_mouse_click(int button, int action, int mods);
    void handle_char_input(unsigned int codepoint);
    void handle_key(int key, int scancode, int action, int mods);
    void handle_window_resize(int width, int height);

    nsc::event_handler event_handler;

    std::vector<nsc::ui::Element *> elements;
    nsc::ui::Element *curr_focused;
    nsc::ui::Element *curr_hover;

    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Window> window;
    std::unique_ptr<Camera> camera;

    float cursor_x;
    float cursor_y;
};

#endif
