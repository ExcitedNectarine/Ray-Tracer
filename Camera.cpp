#include "Camera.h"

Camera::Camera() :
	horizontal(4.0f, 0.0f, 0.0f),
	vertical(0.0f, 2.0f, 0.0f),
	origin(0.0f, 0.0f, 0.0f),
	lower_left(-2.0f, -1.0f, -1.0f) {}

Ray Camera::getRay(float u, float v) { return Ray(origin, lower_left + u * horizontal + v * vertical); }