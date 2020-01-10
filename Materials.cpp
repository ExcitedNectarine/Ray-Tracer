#include "Materials.h"
#include "Objects.h"

glm::vec3 Matte::scatter(Ray r, HitInfo i)
{
	glm::vec3 target = i.position + i.normal + random_point_in_sphere();
	return i.position - target;
}

glm::vec3 Metal::scatter(Ray r, HitInfo i)
{
	return glm::reflect(glm::normalize(r.direction), i.normal);
}