#pragma once

namespace nsc::ui {
class Rectangle {
   public:
    Rectangle()
        : x(0), y(0), width(0), height(0) {
    }
    Rectangle(float x, float y, float width, float height)
        : x(x), y(y), width(width), height(height) {
    }

    ~Rectangle() {}

    bool is_inside(float x, float y) {
        return ((x >= this->x && x <= this->x + this->width) &&
                (y >= this->y && y <= this->y + this->height));
    }

    float x;
    float y;
    float width;
    float height;
};
}  // namespace btn::ui