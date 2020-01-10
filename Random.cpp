#include "Random.h"

inline double random_double() { return rand() / (RAND_MAX + 1.0f); }
glm::vec3 random_point_in_sphere()
{
	glm::vec3 p;
	do { p = 2.0f * glm::vec3(random_double(), random_double(), random_double()) - glm::vec3(1.0f, 1.0f, 1.0f); } while (glm::length2(p) >= 1.0f); // randomly selects points in unit cube and culls points outside of sphere
	return p;
}