#include <iostream>
#include "World.h"

int getCores()
{
	int cores = 0;
	std::cout << "How many cores should be used? Select 0 to detect hardware cores." << std::endl;
	std::cin >> cores;

	if (cores == 0) cores = std::thread::hardware_concurrency();

	return cores;
}

int main(int argc, char *argv[])
{
	int i = 0;
	std::cout << "---------- RAYTRACER ----------" << std::endl
		<< "Press 1 for std::async raytrace" << std::endl
		<< "Press 2 for threaded raytrace" << std::endl
		<< "Press 3 for single core raytrace" << std::endl;
	std::cin >> i;
	int cores = getCores();

	World world(glm::ivec2(1600, 800), 25);

	Matte matte;
	Metal metal;

	world.objects.push_back(std::make_shared<Sphere>(glm::vec3(-0.75f, 0.0f, -1.0f), 0.5f, &matte));
	world.objects.push_back(std::make_shared<Sphere>(glm::vec3(0.75f, 0.0f, -1.0f), 0.5f, &metal));
	world.objects.push_back(std::make_shared<Sphere>(glm::vec3(0.0f, -50.5f, -1.0f), 50.0f, &metal));

	world.lights.push_back(Light(glm::vec3(25.0f, 50.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	world.lights.push_back(Light(glm::vec3(-25.0f, 50.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	world.lights.push_back(Light(glm::vec3(0.0f, 50.0f, -25.0f), glm::vec3(0.0f, 0.3f, 0.0f)));

	auto start = std::chrono::high_resolution_clock::now();

	switch (i)
	{
	case 1: world.runAsync(); break;
	case 2: world.runThreaded(cores); break;
	case 3: world.runSingle(); break;
	};

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	std::cout << "Time elapsed: " << duration << "ms" << std::endl;
	std::cin.get();

	return MCG::ShowAndHold();
}