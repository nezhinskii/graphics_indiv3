#pragma once
#include <GL/glew.h>

struct LightSource {
	GLfloat intensity;

	LightSource(GLfloat intensity) : intensity(intensity) {}
};

struct PointSource : LightSource {
	glm::vec3 pos;

	PointSource(GLfloat intensity, glm::vec3 pos) : pos(pos), LightSource(intensity) {}
};

struct DirectionalSource : LightSource {
	glm::vec3 direction;

	DirectionalSource(GLfloat intensity, glm::vec3 direction) : direction(direction), LightSource(intensity) {}
};

struct SpotlightSource : LightSource {
	glm::vec3 pos;
	glm::vec3 viewPoint;
	GLfloat cone;

	SpotlightSource(GLfloat intensity, glm::vec3 pos, glm::vec3 direction, GLfloat cone) : pos(pos), viewPoint(direction), cone(cone), LightSource(intensity) {}
};