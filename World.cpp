#include "World.h"

World::World(glm::ivec2 size, int samples) : size(size), samples(samples)
{
	srand(unsigned int(time(NULL)));
	MCG::Init(size);
}

glm::vec3 World::lighting(HitInfo i)
{
	glm::vec3 total_light(0.0f, 0.0f, 0.0f);
	for (auto& l : lights)
	{
		glm::vec3 d = glm::normalize(l.position - i.position);
		float a = glm::max(0.0f, glm::dot(d, i.normal)); // make sure side facing light is lit

		total_light += l.colour * a; // add the light falling on the object to the total light
	}

	return total_light;
}

glm::vec3 World::colour(Ray r)
{
	for (auto& obj : objects)
	{
		HitInfo i = obj->intersects(r);
		if (i.t > 0.0f) // collides
			return lighting(i) * colour(Ray(i.position, obj->material->scatter(r, i))); // send out another ray directed by the spheres material and apply lighting to result
	}

	return glm::vec3(0.5f, 0.7f, 1.0f); // return sky colour
}

// USED FOR ASYNC
std::vector<glm::vec3> raytraceRow(World* w, int y)
{
	std::vector<glm::vec3> colours;

	for (int x = 0; x < w->size.x; x++)
	{
		// ANTI ALIASING
		glm::vec3 c(0.0f, 0.0f, 0.0f);
		for (int s = 0; s < w->samples; s++) // takes n samples ~1 pixel around the pixel, and gives the average colour of the samples
		{
			float u = float(x + random_double()) / float(w->size.x);
			float v = float(y + random_double()) / float(w->size.y);

			c += w->colour(w->camera.getRay(u, v));
		}
		c /= float(w->samples);

		colours.push_back(c * 255.0f);
	}

	return colours;
}

// USED FOR THREADS
std::mutex mtx;
void raytraceSector(World* w, int start_y, int end_y)
{
	for (int y = end_y; y >= start_y; y--)
	{
		for (int x = 0; x < w->size.x; x++)
		{
			// ANTI ALIASING
			glm::vec3 c(0.0f, 0.0f, 0.0f);
			for (int s = 0; s < w->samples; s++) // takes n samples ~1 pixel around the pixel, and gives the average colour of the samples
			{
				float u = float(x + random_double()) / float(w->size.x);
				float v = float(y + random_double()) / float(w->size.y);

				c += w->colour(w->camera.getRay(u, v));
			}
			c /= float(w->samples);

			mtx.lock();
			MCG::DrawPixel({ x, y }, c * 255.0f);
			mtx.unlock();
		}
	}
}

void World::runAsync()
{
	// ASYNC MULTITHREADING
	std::vector<std::future<std::vector<glm::vec3>>> futures;
	for (int y = size.y - 1; y >= 0; y--)
		futures.push_back(std::async(std::launch::async, raytraceRow, this, y));

	for (int y = size.y - 1; y >= 0; y--)
	{
		auto& pixels = futures[y].get();
		for (int x = 0; x < size.x; x++)
			MCG::DrawPixel({ x, y }, pixels[x]);
	}
}

void World::runThreaded(int cores)
{
	// THREADS MULTITHREADING
	std::vector<std::thread> threads;
	for (int i = 0; i < cores; i++)
		threads.emplace_back(raytraceSector, this, (size.y / cores) * i, (size.y / cores) * (i + 1));

	for (int i = 0; i < cores; i++)
		threads[i].join();
}

void World::runSingle()
{
	// NO MULTITHREADING
	for (int y = size.y - 1; y >= 0; y--)
	{
		for (int x = 0; x < size.x; x++)
		{
			// ANTI ALIASING
			glm::vec3 c(0.0f, 0.0f, 0.0f);
			for (int s = 0; s < samples; s++) // takes n samples ~1 pixel around the pixel, and gives the average colour of the samples
			{
				float u = float(x + random_double()) / float(size.x);
				float v = float(y + random_double()) / float(size.y);

				c += colour(camera.getRay(u, v));
			}
			c /= float(samples);

			MCG::DrawPixel({ x, y }, c * 255.0f);
		}
	}
}