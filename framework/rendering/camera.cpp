#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
{
	auto camera_pos = glm::vec3(0.0f, 0.0f, 3.0f);
	auto camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
	auto camera_up = glm::vec3(0.0f, 1.0f, 0.0f);

	view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
}

Camera::~Camera()
{
}

glm::mat4 Camera::get_view()
{
	return view;
}
