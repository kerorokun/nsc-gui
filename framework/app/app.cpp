#include "app.hpp"

#include <algorithm>
#include <functional>

App::App(int window_width, int window_height) {
    window = std::make_unique<Window>(window_width, window_height);
    canvas = std::make_unique<nsc::object_registry>();
    renderer = std::make_unique<Renderer>();
    camera = std::make_unique<Camera>();

    window->register_window_resize_callback(
        std::bind(&App::handle_window_resize, this, std::placeholders::_1,
                  std::placeholders::_2));
    window->register_char_input_callback(
        std::bind(&App::handle_char_input, this, std::placeholders::_1));
    window->register_key_callback(std::bind(
        &App::handle_key, this, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4));
    window->register_mouse_click_callback(
        std::bind(&App::handle_mouse_click, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
}

App::~App() {}

void App::start() { main_loop(); }

void App::main_loop() {
    while (!window->should_close()) {
        update();
        render();
    }

    // perform cleanup
    window->close();
}

void App::update() {
    window->pre_update();
    window->update();
    process_callbacks();
    window->post_update();
}

void App::render() {
    renderer->render(canvas.get(), window.get(), camera.get());
}

void App::process_callbacks() {
    // Check cursors
    cursor_x = window->get_cursor_x();
    cursor_y = window->get_cursor_y();

    nsc::ui::Element *focused = nullptr;
    for (auto element : elements) {
        if (element->bounds.is_inside(cursor_x, cursor_y)) {
            focused = element;
        }
    }

    if (curr_hover != focused) {
        // Mouse enter
        if (focused != nullptr) {
            auto element_id = focused->id;
            auto event = MouseEnterEvent{element_id, cursor_x, cursor_y};
            // event_handler.publish(event);
            focused->publish_on<MouseEnterEvent>(event);
        }

        // Mouse exit
        if (curr_hover != nullptr) {
            auto element_id = curr_hover->id;
            auto event = MouseExitEvent{element_id, cursor_x, cursor_y};
            // event_handler.publish(event);
            curr_hover->publish_on<MouseExitEvent>(event);
        }

        curr_hover = focused;
    }

    if (curr_hover) {
        window->set_cursor(CursorType::HAND);
    } else {
        window->set_cursor(CursorType::ARROW);
    }
}

void App::handle_mouse_click(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
        // Use curr_hover to determine what was being pressed
        if (curr_hover != curr_focused) {
            if (curr_focused) {
                // event_handler.publish<FocusLossEvent>(FocusLossEvent{
                // curr_focused->id });
                curr_focused->publish_on<FocusLossEvent>(
                    FocusLossEvent{curr_focused->id});
            }
            curr_focused = curr_hover;
            if (curr_focused) {
                // event_handler.publish<FocusGainEvent>(FocusGainEvent{
                // curr_focused->id });
                curr_focused->publish_on<FocusGainEvent>(
                    FocusGainEvent{curr_focused->id});
            }
        }

        if (curr_focused) {
            //     event_handler.publish<MouseClickEvent>(
            //        MouseClickEvent{curr_focused->id, cursor_x, cursor_y});
            curr_focused->publish_on<MouseClickEvent>(
                MouseClickEvent{curr_focused->id, cursor_x, cursor_y});
        }
    }
}

void App::handle_char_input(unsigned int codepoint) {
    if (curr_focused) {
        // event_handler.publish<CharInputEvent>(
        //    CharInputEvent{curr_focused->id, (char)codepoint});
        curr_focused->publish_on<CharInputEvent>(
            CharInputEvent{curr_focused->id, (char)codepoint});
    }
}
void App::handle_key(int key, int scancode, int action, int mods) {
    if (curr_focused && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        event_handler.publish<KeyPressEvent>(
            KeyPressEvent{curr_focused->id, key});
    }
}
void App::handle_window_resize(int width, int height) {
    event_handler.publish<WindowResizeEvent>(WindowResizeEvent{width, height});
}

void App::add(nsc::ui::Element *element) { elements.push_back(element); }

void App::remove(nsc::ui::Element *element) {
    elements.erase(std::remove(elements.begin(), elements.end(), element),
                   elements.end());
}

void App::set_background_color(const nsc::rendering::Color &color) {
    // canvas->set_bg_color(color);
}

float App::get_delta_time() { return window->get_delta_time(); }
