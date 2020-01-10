#pragma once

#include <GLM/glm.hpp>

struct Ray
{
	Ray(glm::vec3 origin, glm::vec3 direction);
	glm::vec3 point(float t);

	glm::vec3 origin;
	glm::vec3 direction;
};