#pragma once

#include "Ray.h"

struct Camera
{
	Camera();
	Ray getRay(float u, float v);

	glm::vec3 origin;
	glm::vec3 horizontal;
	glm::vec3 vertical;
	glm::vec3 lower_left;
};