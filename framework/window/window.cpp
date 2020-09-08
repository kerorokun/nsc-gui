#include "window.hpp"

#include <iostream>
#include <algorithm>

Window::Window(int width, int height)
{
	this->width = width;
	this->height = height;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DECORATED, true);

	window = glfwCreateWindow(width, height, "TIMER", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create a window\n";
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(window);
	glfwSetWindowPos(window, 100, 100);

	glfwSetWindowUserPointer(window, this);
	glfwSetMouseButtonCallback(window, [](GLFWwindow *w, int button, int action, int mods) {
		static_cast<Window *>(glfwGetWindowUserPointer(w))->handle_mouse_click(button, action, mods);
	});
	glfwSetCharCallback(window, [](GLFWwindow *w, unsigned int codepoint) {
		static_cast<Window *>(glfwGetWindowUserPointer(w))->handle_char_input(codepoint);
	});
	glfwSetKeyCallback(window, [](GLFWwindow *w, int key, int scancode, int action, int mods) {
		static_cast<Window *>(glfwGetWindowUserPointer(w))->handle_key(key, scancode, action, mods);
	});
	glfwSetWindowSizeCallback(window, [](GLFWwindow *w, int width, int height) {
		static_cast<Window *>(glfwGetWindowUserPointer(w))->handle_window_resize(width, height);
	});

	glfwSwapInterval(1);
	hand_cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
	text_cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
}

Window::~Window()
{
}

void Window::handle_mouse_click(int button, int action, int mods)
{
	if (mouse_click_callback) {
		(*mouse_click_callback)(button, action, mods);
	}
}

void Window::handle_char_input(unsigned int codepoint)
{
	if (char_input_callback) {
		(*char_input_callback)(codepoint);
	}
}

void Window::handle_key(int key, int scancode, int action, int mods)
{
	if (key_callback) {
		(*key_callback)(key, scancode, action, mods);
	}
}

void Window::handle_window_resize(int width, int height)
{
	if (window_resize_callback) {
		(*window_resize_callback)(width, height);
	}
}

void Window::close()
{
	glfwTerminate();
}

void Window::pre_update()
{
	auto curr_frame = glfwGetTime();
	delta_time = curr_frame - last_frame;
	last_frame = curr_frame;

	double new_cursor_x, new_cursor_y;
	glfwGetCursorPos(window, &new_cursor_x, &new_cursor_y);
	new_cursor_y = (double)height - new_cursor_y;

	cursor_x = (float)new_cursor_x;
	cursor_y = (float)new_cursor_y;
}

void Window::update()
{
}

void Window::post_update()
{
	glfwPollEvents();
	glfwSwapBuffers(window);
}

void Window::register_mouse_click_callback(std::function<void(int, int, int)> callback) { mouse_click_callback = callback; }
void Window::register_char_input_callback(std::function<void(unsigned int)> callback) { char_input_callback = callback; }
void Window::register_key_callback(std::function<void(int, int, int, int)> callback) { key_callback = callback; }
void Window::register_window_resize_callback(std::function<void(int, int)> callback) { window_resize_callback = callback; }

void Window::set_cursor(CursorType type)
{
	GLFWcursor *cursor = NULL;
	switch (type) {
	case ARROW:
		cursor = NULL;
		break;
	case HAND:
		cursor = hand_cursor;
		break;
	case TEXT:
		cursor = text_cursor;
		break;
	}

	glfwSetCursor(window, cursor);
}

int Window::get_width() { return width; }
int Window::get_height() { return height; }
float Window::get_cursor_x() { return cursor_x; }
float Window::get_cursor_y() { return cursor_y; }
float Window::get_delta_time() { return delta_time; }
bool Window::should_close() { return glfwWindowShouldClose(window); }