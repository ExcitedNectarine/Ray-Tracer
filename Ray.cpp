#include "Ray.h"

Ray::Ray(glm::vec3 origin, glm::vec3 direction) : origin(origin), direction(direction) {}
glm::vec3 Ray::point(float t) { return origin + t * direction; }