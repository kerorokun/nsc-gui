#include "texture_catalog.hpp"
#include "stb_image.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

TextureCatalog::TextureCatalog()
{
}

TextureCatalog::~TextureCatalog()
{
}

nsc::rendering::Texture * TextureCatalog::load_texture(const std::string &path, bool alpha)
{
	if (!path_to_texture.count(path)) {
		stbi_set_flip_vertically_on_load(false);

		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		int width, height, num_channels;
		auto data = stbi_load(path.c_str(), &width, &height, &num_channels, 0);

		auto image_format = alpha ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, image_format, width, height, 0, image_format, GL_UNSIGNED_BYTE, data);
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(data);

		auto texture_obj = nsc::rendering::Texture { texture, width, height, num_channels };
		path_to_texture[path] = texture_obj;
	}
	return &path_to_texture[path];
}

void TextureCatalog::release_texture(const std::string& path)
{
	
}

ImageDesc TextureCatalog::create(const std::string& image_path, const nsc::rendering::Color& color, const nsc::ui::Rectangle& bounds)
{
	auto texture = load_texture(image_path);
	return ImageDesc{ texture, color, bounds };
}