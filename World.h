#pragma once

#include <cstdlib>
#include <ctime>
#include <vector>
#include <future>
#include <iostream>
#include <chrono>
#include <memory>
#include "MCG_GFX_Lib.h"
#include "Objects.h"
#include "Camera.h"
#include "Random.h"

struct World
{
	World(glm::ivec2 size, int samples);
	glm::vec3 lighting(HitInfo i);
	glm::vec3 colour(Ray r);
	void runAsync();
	void runThreaded(int cores);
	void runSingle();

	Camera camera;
	std::vector<std::shared_ptr<Object>> objects;
	std::vector<Light> lights;
	glm::ivec2 size;
	int samples;
};