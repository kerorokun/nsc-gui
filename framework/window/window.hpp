#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <GLFW/glfw3.h>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <vector>
#include <string>
#include "../ui/events.hpp"
#include <optional>

enum CursorType
{
	ARROW,
	HAND,
	TEXT
};

// The window layer should be in charge of handling all window stuff
// This inputs user inputs and styling the window
class Window
{
public:
	Window(int width, int height);
	~Window();
 
	bool should_close();
	void close();

	void pre_update();
	void update();
	void post_update();

	void handle_mouse_click(int button, int action, int mods);
	void handle_char_input(unsigned int codepoint);
	void handle_key(int key, int scancode, int action, int mods);
	void handle_window_resize(int width, int height);

	void register_mouse_click_callback(std::function<void(int, int, int)> callback);
	void register_char_input_callback(std::function<void(unsigned int)> callback);
	void register_key_callback(std::function<void(int, int, int, int)> callback);
	void register_window_resize_callback(std::function<void(int, int)> callback);

	void set_cursor(CursorType type);
	int get_width();
	int get_height();
	float get_delta_time();
	float get_cursor_x();
	float get_cursor_y();
private:
	GLFWwindow *window;
	GLFWcursor *hand_cursor;
	GLFWcursor *text_cursor;

	std::optional<std::function<void(int, int, int)>> mouse_click_callback;
	std::optional<std::function<void(unsigned int)>> char_input_callback;
	std::optional<std::function<void(int, int, int, int)>> key_callback;
	std::optional<std::function<void(int, int)>> window_resize_callback;

	float delta_time, last_frame;
	float cursor_x, cursor_y;
	int width, height;
};

#endif
