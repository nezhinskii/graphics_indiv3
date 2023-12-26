#pragma once
#include <unordered_map>
#include <GL/glew.h>
#include "camera.h"
#include "model.h"
#include "light_sources.h"

class PainterState {
public:
	PainterState(Camera camera) : camera(camera) {}

	Camera camera;

	Model* airship = nullptr;
	Model* snow = nullptr;
	Model* сhristmasTree = nullptr;
	Model* atAt = nullptr;
	Model* clone = nullptr;
	Model* snowspeeder = nullptr;
	Model* rebel = nullptr;

	DirectionalSource directionalSource = DirectionalSource(1.0f, glm::vec3(0, 100, 100));
};
