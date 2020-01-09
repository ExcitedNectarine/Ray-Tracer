#include "MCG_GFX_Lib.h"
#include <cstdlib>
#include <ctime>
#include <vector>
#include <future>
#include <iostream>
#include <chrono>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

#define USE_ASYNC 1
#define USE_THREADS 0
#define USE_SINGLE 0

struct Light
{
	Light(glm::vec3 position, glm::vec3 colour) : position(position), colour(colour) {}

	glm::vec3 position;
	glm::vec3 colour;
};

struct Material;
struct Sphere
{
	Sphere(glm::vec3 position, float radius, Material* material) : position(position), radius(radius), material(material) {}

	glm::vec3 position;
	float radius;
	Material* material;
};

struct Ray
{
	Ray(glm::vec3 origin, glm::vec3 direction) : origin(origin), direction(direction) {}
	glm::vec3 point(float t) { return origin + t * direction; }

	glm::vec3 origin;
	glm::vec3 direction;
};

struct Camera
{
	Camera() :
		horizontal(4.0f, 0.0f, 0.0f),
		vertical(0.0f, 2.0f, 0.0f),
		origin(0.0f, 0.0f, 0.0f),
		lower_left(-2.0f, -1.0f, -1.0f) {}

	Ray getRay(float u, float v) { return Ray(origin, lower_left + u * horizontal + v * vertical); }

	glm::vec3 origin;
	glm::vec3 horizontal;
	glm::vec3 vertical;
	glm::vec3 lower_left;
};

struct HitInfo
{
	float t;
	glm::vec3 position;
	glm::vec3 normal;
};

inline double random_double() { return rand() / (RAND_MAX + 1.0f); }
glm::vec3 random_point_in_sphere()
{
	glm::vec3 p;
	do { p = 2.0f * glm::vec3(random_double(), random_double(), random_double()) - glm::vec3(1.0f, 1.0f, 1.0f); } while (glm::length2(p) >= 1.0f);
	return p;
}

struct Material
{
	virtual glm::vec3 scatter(Ray r, HitInfo i) = 0;
};

struct Matte : Material
{
	glm::vec3 scatter(Ray r, HitInfo i)
	{
		glm::vec3 target = i.position + i.normal + random_point_in_sphere();
		return i.position - target;
	}
};

struct Metal : Material
{
	glm::vec3 scatter(Ray r, HitInfo i)
	{
		return glm::reflect(glm::normalize(r.direction), i.normal);
	}
};

HitInfo ray_sphere_intersect(Ray r, Sphere s)
{
	glm::vec3 oc = r.origin - s.position;
	float a = glm::dot(r.direction, r.direction);
	float b = 2.0f * glm::dot(oc, r.direction);
	float c = glm::dot(oc, oc) - s.radius * s.radius;
	float d = b * b - 4 * a * c;

	HitInfo info;
	info.t = d < 0 ? -1.0f : (-b - glm::sqrt(d)) / (2.0f * a);
	info.position = r.point(info.t);
	info.normal = (info.position - s.position) / s.radius;

	return info;
}

glm::vec3 lighting(const std::vector<Light>& lights, HitInfo i)
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

glm::vec3 colour(Ray r, const std::vector<Sphere>& spheres, const std::vector<Light>& lights)
{
	for (auto& s : spheres)
	{
		HitInfo i = ray_sphere_intersect(r, s);
		if (i.t > 0.0f) // collides
			return lighting(lights, i) * colour(Ray(i.position, s.material->scatter(r, i)), spheres, lights); // send out another ray directed by the spheres material
	}

	float t = float(0.5 * (glm::normalize(r.direction).y + 1.0f));
	return (1.0f - t) * glm::vec3(1.0f, 1.0f, 1.0f) + t * glm::vec3(0.5f, 0.7f, 1.0f); // sky colour with gradient
}

// USED FOR ASYNC
std::vector<glm::vec3> raytraceRow(glm::ivec2 size, int samples, int y, Camera& camera, const std::vector<Sphere>& spheres, const std::vector<Light>& lights)
{
	std::vector<glm::vec3> colours;

	for (int x = 0; x < size.x; x++)
	{
		// ANTI ALIASING
		glm::vec3 c(0.0f, 0.0f, 0.0f);
		for (int s = 0; s < samples; s++) // takes n samples ~1 pixel around the pixel, and gives the average colour of the samples
		{
			float u = float(x + random_double()) / float(size.x);
			float v = float(y + random_double()) / float(size.y);

			c += colour(camera.getRay(u, v), spheres, lights);
		}
		c /= float(samples);

		colours.push_back(c * 255.0f);
	}

	return colours;
}

// USED FOR THREADS
std::mutex mtx;
void raytraceSector(glm::ivec2 size, int samples, int start_y, int end_y, Camera& camera, const std::vector<Sphere>& spheres, const std::vector<Light>& lights)
{
	for (int y = start_y; y < end_y; y++)
	{
		for (int x = 0; x < size.x; x++)
		{
			// ANTI ALIASING
			glm::vec3 c(0.0f, 0.0f, 0.0f);
			for (int s = 0; s < samples; s++) // takes n samples ~1 pixel around the pixel, and gives the average colour of the samples
			{
				float u = float(x + random_double()) / float(size.x);
				float v = float(y + random_double()) / float(size.y);

				c += colour(camera.getRay(u, v), spheres, lights);
			}
			c /= float(samples);

			mtx.lock();
			MCG::DrawPixel({ x, y }, c * 255.0f);
			mtx.unlock();
		}
	}
}

int main(int argc, char *argv[])
{
	srand(unsigned int(time(NULL)));

	int samples = 25; // anti aliasing samples
	glm::ivec2 size(1600, 800);
	MCG::Init(size);

	Camera camera;

	Matte matte;
	Metal metal;

	std::vector<Sphere> spheres;
	spheres.push_back(Sphere(glm::vec3(-0.75f, 0.0f, -1.0f), 0.5f, &matte));
	spheres.push_back(Sphere(glm::vec3(0.75f, 0.0f, -1.0f), 0.5f, &metal));
	spheres.push_back(Sphere(glm::vec3(0.0f, -50.5f, -1.0f), 50.0f, &metal));

	std::vector<Light> lights;
	lights.push_back(Light(glm::vec3(25.0f, 50.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	lights.push_back(Light(glm::vec3(-25.0f, 50.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	lights.push_back(Light(glm::vec3(0.0f, 50.0f, -25.0f), glm::vec3(0.0f, 0.3f, 0.0f)));

	auto start = std::chrono::high_resolution_clock::now();

#if USE_THREADS == 1

	// THREADS MULTITHREADING
	std::vector<std::thread> threads;
	int cores = std::thread::hardware_concurrency();
	for (int i = 0; i < cores; i++)
		threads.emplace_back(raytraceSector, size, samples, (size.y / cores) * i, (size.y / cores) * (i + 1), camera, spheres, lights);

	for (int i = 0; i < cores; i++)
		threads[i].join();

#elif USE_ASYNC == 1

	// ASYNC MULTITHREADING
	std::vector<std::future<std::vector<glm::vec3>>> futures;
	for (int y = size.y - 1; y >= 0; y--)
		futures.push_back(std::async(std::launch::async, raytraceRow, size, samples, y, camera, spheres, lights));

	for (int y = size.y - 1; y >= 0; y--)
	{
		auto& pixels = futures[y].get();
		for (int x = 0; x < size.x; x++)
			MCG::DrawPixel({ x, y }, pixels[x]);
	}

#elif USE_SINGLE == 1

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

				c += colour(camera.getRay(u, v), spheres, lights);
			}
			c /= float(samples);

			MCG::DrawPixel({ x, y }, c * 255.0f);
		}
	}

#endif

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	std::cout << "Time elapsed: " << duration << "ms" << std::endl;

	return MCG::ShowAndHold();
}