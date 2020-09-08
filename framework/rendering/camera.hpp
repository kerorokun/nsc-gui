#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include <glm/glm.hpp>

class Camera
{
public:
	Camera();
	~Camera();
	
	glm::mat4 get_view();
private:
	glm::mat4 view;
};

#endif
