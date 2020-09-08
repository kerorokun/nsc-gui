#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "texture.hpp"

struct Character {
    char c;
    double advance;

    double plane_left;
    double plane_bottom;
    double plane_right;
    double plane_top;

    float texture_left;
    float texture_bottom;
    float texture_right;
    float texture_top;

    void populate_from_line(std::string line) {
        std::istringstream ss(line);
        std::string field;

        std::getline(ss, field, ',');
        this->c = std::stoi(field);

        std::getline(ss, field, ',');
        this->advance = std::stod(field);
        std::getline(ss, field, ',');
        this->plane_left = std::stod(field);
        std::getline(ss, field, ',');
        this->plane_bottom = std::stod(field);
        std::getline(ss, field, ',');
        this->plane_right = std::stod(field);
        std::getline(ss, field, ',');
        this->plane_top = std::stod(field);

        std::getline(ss, field, ',');
        this->texture_left = std::stod(field);
        std::getline(ss, field, ',');
        this->texture_bottom = std::stod(field);
        std::getline(ss, field, ',');
        this->texture_right = std::stod(field);
        std::getline(ss, field, ',');
        this->texture_top = std::stod(field);
    }
};

struct Font {
    std::unordered_map<char, Character> char_data;
    float max_height;
    nsc::rendering::Texture *texture;
    nsc::rendering::Texture *fallback_texture;
};
