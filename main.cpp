#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <GL/glew.h>

#include "imgui.h" // necessary for ImGui::*, imgui-SFML.h doesn't include imgui.h

#include "imgui-SFML.h" // for ImGui::SFML::* functions and SFML-specific overloads

#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <iostream>
#include "camera.h"
#include "painter.h"
#include "lib/ImGuiFileDialog/ImGuiFileDialog.h"
#include "painter_state.h"
#include<filesystem>

using namespace sf;

void floatPicker(GLfloat* val, std::string label, std::string nameSpace) {
	ImGui::Text(label.c_str());
	ImGui::DragFloat(("##" + label + nameSpace).c_str(), val, 0.1F);
}

void vectorPicker(glm::vec3* vec, std::string label, std::string nameSpace) {
	ImGui::Text(label.c_str());
	ImGui::DragFloat(("X##" + label + nameSpace).c_str(), &(vec->x), 0.1F);
	ImGui::DragFloat(("Y##" + label + nameSpace).c_str(), &(vec->y), 0.1F);
	ImGui::DragFloat(("Z##" + label + nameSpace).c_str(), &(vec->z), 0.1F);
}

void intencityPicker(GLfloat* val, std::string label, std::string nameSpace) {
	ImGui::Text(label.c_str());
	ImGui::DragFloat(("##" + label + nameSpace).c_str(), val, 0.01, 0, 1);
}

void conePicker(GLfloat* val, std::string label, std::string nameSpace) {
	ImGui::Text(label.c_str());
	ImGui::DragFloat(("##" + label + nameSpace).c_str(), val, 1.0, 0.1, 180);
}


void pointSourceEditor(GLfloat* intensity, glm::vec3* pos) {
	if (ImGui::CollapsingHeader("Point source")) {
		intencityPicker(intensity, "Intensity", "pointSource");
		vectorPicker(pos, "Position", "pointSource");
	}
}

void directionalSourceEditor(GLfloat* intensity, glm::vec3* direction) {
	if (ImGui::CollapsingHeader("Directional source")) {
		intencityPicker(intensity, "Intensity", "directionalSource");
		vectorPicker(direction, "Direction", "directionalSource");
	}
}

void spotlightSourceEditor(GLfloat* intensity, glm::vec3* pos, glm::vec3* direction, GLfloat* cone) {
	if (ImGui::CollapsingHeader("Spotlight source")) {
		intencityPicker(intensity, "Intensity", "spotlightSource");
		vectorPicker(pos, "Position", "spotlightSource");
		vectorPicker(direction, "View point", "spotlightSource");
		conePicker(cone, "Cone", "spotlightSource");
	}
}

void shadingPicker(std::string title, int* picked) {
	if (ImGui::CollapsingHeader(title.c_str())) {
		ImGui::RadioButton(("Phong##" + title).c_str(), picked, Shading::Phong);
		ImGui::RadioButton(("Toon##" + title).c_str(), picked, Shading::Toon);
		ImGui::RadioButton(("Rim##" + title).c_str(), picked, Shading::Rim);
	}
}

std::string vec3ToStr(glm::vec3 vec3) {
	return "(" +
		std::to_string(vec3.x) + ", " +
		std::to_string(vec3.y) + ", " +
		std::to_string(vec3.z) + ")";
}

int main() {
	sf::RenderWindow window(sf::VideoMode(600, 600), "Lab 13", sf::Style::Default, sf::ContextSettings(24));
	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);
	window.setActive(true);
	std::filesystem::path currentPath = std::filesystem::current_path();

	Camera camera = Camera(glm::vec3(0.0f, 5.0f, 20.0f), 1.0f);
	auto state = PainterState(camera);
	auto painter = Painter(state);
	std::vector<glm::vec3> snowTranslations;
	for (int i = -50; i < 50; ++i) {
		for (int j = -50; j < 50; ++j) {
			snowTranslations.push_back(glm::vec3(i * 10.0f, 0.0f, j * 10.0f));
		}
	}

	srand(30);
	std::vector<glm::vec3> atAtTranslations;
	for (int i = 0; i < 5; ++i) {
		atAtTranslations.push_back(glm::vec3((float)(10 + rand() % 40), 0.0f, (-10 + rand() % 40)));
	}

	std::vector<glm::vec3> cloneTranslations;
	for (int i = 0; i < 50; ++i) {
		cloneTranslations.push_back(glm::vec3((float)(10 + rand() % 60), 0.0f, (-20 + rand() % 60)));
	}

	std::vector<glm::vec3> snowspeederTranslations;
	for (int i = 0; i < 8; ++i) {
		snowspeederTranslations.push_back(glm::vec3((float)(-10 - rand() % 40), 0.0f, (-10 + rand() % 40)));
	}

	std::vector<glm::vec3> rebelTranslations;
	for (int i = 0; i < 50; ++i) {
		rebelTranslations.push_back(glm::vec3((float)(-10 - rand() % 60), 0.0f, (-20 + rand() % 60)));
	}

	painter.Init();
	painter.state.airship = new Model(currentPath.string() + "\\ship\\scene.gltf");
	painter.state.snow = new Model(currentPath.string() + "\\snow\\scene.gltf", snowTranslations.data(), snowTranslations.size());
	painter.state.atAt = new Model(currentPath.string() + "\\atAt\\scene.gltf", atAtTranslations.data(), atAtTranslations.size());
	painter.state.clone = new Model(currentPath.string() + "\\clone\\scene.gltf", cloneTranslations.data(), cloneTranslations.size());
	painter.state.snowspeeder = new Model(currentPath.string() + "\\snowspeeder\\scene.gltf", snowspeederTranslations.data(), snowspeederTranslations.size());
	painter.state.rebel = new Model(currentPath.string() + "\\rebel\\scene.gltf", rebelTranslations.data(), rebelTranslations.size());
	painter.state.сhristmasTree = new Model(currentPath.string() + "\\сhristmasTree\\scene.gltf");

	GLboolean firstMouse = true;
	GLfloat lastX = 0, lastY = 0;
	GLboolean isFocused = false;
	sf::Vector2i centerWindow;

	if (!ImGui::SFML::Init(window)) return -1;

	int shade1 = Shading::Phong;
	int shade2 = Shading::Toon;

	GLfloat deltaTiltTime = 0.0f;
	sf::Clock deltaClock;
	sf::Clock titlClock;
	while (window.isOpen()) {

		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(window, event);
			bool isImGuiHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
			
			if (event.type == sf::Event::Closed)
				window.close();
			else if (event.type == sf::Event::Resized) {
				glViewport(
					0,
					0,
					event.size.width,
					event.size.height
				);
				painter.state.camera.processResize(event.size.width, event.size.height);
			}
			else if (event.type == sf::Event::MouseMoved && isFocused) {
				GLfloat xoffset = event.mouseMove.x - centerWindow.x;
				GLfloat yoffset = centerWindow.y - event.mouseMove.y;
				lastX = event.mouseMove.x;
				lastY = event.mouseMove.y;
				sf::Mouse::setPosition(centerWindow, window);
				painter.state.camera.processMouseMovement(xoffset, yoffset);
				if (abs(xoffset) > 10) {
					deltaTiltTime = titlClock.getElapsedTime().asSeconds();
					if (xoffset > 0) {
						painter.shipTilt += glm::pi<float>() / 100;
					}
					else {
						painter.shipTilt -= glm::pi<float>() / 100;
					}
				}
			}

			if (event.type == sf::Event::KeyPressed) {
				painter.state.camera.processKeyboard(event.key.code);
				if (event.key.code == sf::Keyboard::A) {
					deltaTiltTime = titlClock.getElapsedTime().asSeconds();
					painter.shipTilt -= glm::pi<float>() / 50;
				}	
				else if (event.key.code == sf::Keyboard::D) {
					deltaTiltTime = titlClock.getElapsedTime().asSeconds();
					painter.shipTilt += glm::pi<float>() / 50;
				}
					
			} 
			if (!isImGuiHovered && event.type == sf::Event::MouseButtonPressed) {
				isFocused = true;
				window.setMouseCursorVisible(false);
				window.setMouseCursorGrabbed(true);
				centerWindow.x = window.getSize().x / 2;
				centerWindow.y = window.getSize().y / 2;
				sf::Mouse::setPosition(centerWindow, window);
				auto pos = sf::Mouse::getPosition();
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape || event.type == sf::Event::LostFocus) {
				window.setMouseCursorVisible(true);
				window.setMouseCursorGrabbed(false);
				isFocused = false;
				firstMouse = true;
			}
		}

		//painter.state.camera.position += painter.state.camera.front * 0.1f;

		if (titlClock.getElapsedTime().asSeconds() - deltaTiltTime > 0.2) {
			painter.shipTilt *= 0.80;
		}


		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		painter.Draw();

		window.display();
	}

	painter.Release();
	return 0;
}
