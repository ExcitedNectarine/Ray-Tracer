#include "Objects.h"

Object::Object(Material* material) : material(material) {}
Sphere::Sphere(glm::vec3 position, float radius, Material* material) : Object(material), position(position), radius(radius) {}

HitInfo Sphere::intersects(Ray r)
{
	// uses part of quadratic formula to find intersection

	glm::vec3 oc = r.origin - position;
	float a = glm::dot(r.direction, r.direction);
	float b = 2.0f * glm::dot(oc, r.direction);
	float c = glm::dot(oc, oc) - radius * radius;
	float d = b * b - 4 * a * c; // discriminent

	HitInfo info;
	info.t = d < 0 ? -1.0f : (-b - glm::sqrt(d)) / (2.0f * a);
	info.position = r.point(info.t);
	info.normal = (info.position - position) / radius;

	return info;
}