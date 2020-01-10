#pragma once

#include "Ray.h"
#include "Random.h"

struct HitInfo;
struct Material
{
	virtual glm::vec3 scatter(Ray r, HitInfo i) = 0;
};

struct Matte : Material
{
	glm::vec3 scatter(Ray r, HitInfo i);
};

struct Metal : Material
{
	glm::vec3 scatter(Ray r, HitInfo i);
};