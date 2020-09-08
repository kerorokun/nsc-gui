#ifndef EVENTS_HPP_
#define EVENTS_HPP_

#include <cstdint>

struct MouseClickEvent
{
	uint32_t element_id;
	float cursor_x;
	float cursor_y;
};

struct MouseEnterEvent
{
	uint32_t element_id;
	float cursor_x;
	float cursor_y;
};

struct MouseExitEvent
{
	uint32_t element_id;
	float cursor_x;
	float cursor_y;
};

struct CharInputEvent
{
	uint32_t element_id;
	char input;
};

struct WindowResizeEvent
{
	int width;
	int height;
};

struct FocusGainEvent
{
	uint32_t element_id;
};

struct FocusLossEvent
{
	uint32_t element_id;
};

struct KeyPressEvent
{
	uint32_t element_id;
	//Key key;
	int key;
};

#endif
