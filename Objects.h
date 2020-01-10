#pragma once

#include "Materials.h"

struct HitInfo
{
	float t;
	glm::vec3 position;
	glm::vec3 normal;
};

struct Object
{
	Object(Material* material);
	virtual HitInfo intersects(Ray r) = 0;
	Material* material;
};

struct Sphere : Object
{
	Sphere(glm::vec3 position, float radius, Material* material);
	HitInfo intersects(Ray r);

	glm::vec3 position;
	float radius;
};

struct Light
{
	Light(glm::vec3 position, glm::vec3 colour) : position(position), colour(colour) {}

	glm::vec3 position;
	glm::vec3 colour;
};