#pragma once

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include "painter_state.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include<filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace sf;

class Painter {

	const static GLuint shadersNumber = 3;

	GLuint instancedProgram, defaultProgram;


	bool readFile(const std::string& filename, std::string& content) {
		std::ifstream file(filename);

		if (!file.is_open()) {
			std::cerr << "Unable to open the file: " << filename << std::endl;
			return false;
		}

		std::stringstream buffer;
		buffer << file.rdbuf();
		content = buffer.str();

		file.close();

		return true;
	}

	void ShaderLog(unsigned int shader)
	{
		int infologLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
		if (infologLen > 1)
		{
			int charsWritten = 0;
			std::vector<char> infoLog(infologLen);
			glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog.data());
			std::cout << "InfoLog: " << infoLog.data() << std::endl;
		}
	}

	void InitShader() {
		std::string content;
		const char* cstrContent;

		GLuint instancedVShader, defaultVShader;

		readFile("shaders/shader.vert", content);
		cstrContent = content.c_str();
		defaultVShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(defaultVShader, 1, &cstrContent, NULL);
		glCompileShader(defaultVShader);
		std::cout << "default vertex shader" << std::endl;
		ShaderLog(defaultVShader);

		readFile("shaders/shader_inst.vert", content);
		cstrContent = content.c_str();
		instancedVShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(instancedVShader, 1, &cstrContent, NULL);
		glCompileShader(instancedVShader);
		std::cout << "instanced vertex shader" << std::endl;
		ShaderLog(instancedVShader);

		GLuint fShader;

		readFile("shaders/shader.frag", content);
		cstrContent = content.c_str();
		fShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fShader, 1, &cstrContent, NULL);
		glCompileShader(fShader);
		std::cout << "fragment shader"<< std::endl;
		ShaderLog(fShader);

		defaultProgram = glCreateProgram();
		glAttachShader(defaultProgram, defaultVShader);
		glAttachShader(defaultProgram, fShader);
		glLinkProgram(defaultProgram);
		int link_ok;
		glGetProgramiv(defaultProgram, GL_LINK_STATUS, &link_ok);
		if (!link_ok) {
			std::cout << "error attach shaders \n";
			return;
		}

		instancedProgram = glCreateProgram();
		glAttachShader(instancedProgram, instancedVShader);
		glAttachShader(instancedProgram, fShader);
		glLinkProgram(instancedProgram);
		glGetProgramiv(instancedProgram, GL_LINK_STATUS, &link_ok);
		if (!link_ok) {
			std::cout << "error attach shaders \n";
			return;
		}
	}

	/*void InitShader() {
		GLuint vShaders[shadersNumber];
		std::filesystem::path currentPath = std::filesystem::current_path();
		std::cout << "Current Path: " << currentPath.string() << std::endl;

		std::string  vertShaderText = get_file_contents((currentPath.string() + "\\shaders\\shader.vert").c_str());
		const char* vertShader = vertShaderText.c_str();
		for (int i = 0; i < shadersNumber; i++) {
			vShaders[i] = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vShaders[i], 1, &vertShader, NULL);

			glCompileShader(vShaders[i]);
			std::cout << "vertex shader" << i << std::endl;
			ShaderLog(vShaders[i]);
		}

		GLuint fShaders[shadersNumber];
		std::string  fragShaderText = get_file_contents((currentPath.string() + "\\shaders\\shader.frag").c_str());
		const char* fragShader = vertShaderText.c_str();
		for (int i = 0; i < shadersNumber; i++) {
			fShaders[i] = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fShaders[i], 1, &fragShader, NULL);
			glCompileShader(fShaders[i]);
			std::cout << "fragment shader" << i << std::endl;
			ShaderLog(fShaders[i]);
		}


		for (int i = 0; i < shadersNumber; i++) {
			Programs[i] = glCreateProgram();
			glAttachShader(Programs[i], vShaders[i]);
			glAttachShader(Programs[i], fShaders[i]);
			glLinkProgram(Programs[i]);
			int link_ok;
			glGetProgramiv(Programs[i], GL_LINK_STATUS, &link_ok);
			if (!link_ok) {
				std::cout << "error attach shaders \n";
				return;
			}
		}
	}*/

	void ReleaseShader() {
		glUseProgram(0);
		glDeleteProgram(instancedProgram);
		glDeleteProgram(defaultProgram);
	}

	GLfloat deegressToRadians(GLfloat deegres) {
		return deegres * 3.141592f / 180.0f;
	}

public:
	Painter(PainterState& painterState) : state(painterState) {}

	PainterState state;

	GLfloat shipTilt = 0.0f;
	

	void Draw() {
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		if (shipTilt > glm::pi<float>() / 4) {
			shipTilt = glm::pi<float>() / 4;
		}
		if (shipTilt < -glm::pi<float>() / 4) {
			shipTilt = -glm::pi<float>() / 4;
		}

		if (state.airship != nullptr) {
			glUseProgram(defaultProgram);
			glm::vec3 projectedFront = glm::vec3(state.camera.front.x, 0.0f, state.camera.front.z);
			projectedFront = glm::normalize(projectedFront);
			GLfloat horizontalAngle = glm::acos(glm::dot(projectedFront, glm::vec3(0, 0, -1)));
			GLfloat verticalAngle = glm::acos(glm::dot(state.camera.front, state.camera.worldUp)) - glm::pi<float>() / 2;

			glm::vec3 crossProduct = glm::cross(projectedFront, glm::vec3(0, 0, -1));
			if (crossProduct.y > 0) {
				horizontalAngle = -horizontalAngle;
			}
			glm::mat4 shipMat = glm::translate(glm::mat4(1.0f), state.camera.position)
				* glm::rotate(glm::mat4(1.0f), horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), verticalAngle, glm::vec3(-1.0f, 0.0f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), shipTilt, glm::vec3(0.0f, 0.0f, -1.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(0.1f))
				* glm::rotate(glm::mat4(1.0f), deegressToRadians(90), glm::vec3(-1.0f, 0.0f, 0.0f));
			(state.airship->Draw(defaultProgram, shipMat, state.camera, state.directionalSource));
			glUseProgram(0);
		}
		
		if (state.snow != nullptr) {
			glUseProgram(instancedProgram);
			glm::mat4 snowMat = glm::scale(glm::mat4(1.0f), glm::vec3(5.f))
				* glm::rotate(glm::mat4(1.0f), deegressToRadians(90), glm::vec3(-1.0f, 0.0f, 0.0f));
			(state.snow->Draw(instancedProgram, snowMat,  state.camera, state.directionalSource));
			glUseProgram(0);
		}

		if (state.сhristmasTree != nullptr) {
			glUseProgram(defaultProgram);
			glm::mat4 treeMat = glm::scale(glm::mat4(1.0f), glm::vec3(40.f))
				* glm::rotate(glm::mat4(1.0f), deegressToRadians(90), glm::vec3(-1.0f, 0.0f, 0.0f));
			(state.сhristmasTree->Draw(defaultProgram, treeMat, state.camera, state.directionalSource));
			glUseProgram(0);
		}

		if (state.atAt != nullptr) {
			glUseProgram(instancedProgram);
			glm::mat4 atAtMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(0.7f))
				* glm::rotate(glm::mat4(1.0f), deegressToRadians(90), glm::vec3(-1.0f, 0.0f, 0.0f));
			(state.atAt->Draw(instancedProgram, atAtMat, state.camera, state.directionalSource));
			glUseProgram(0);
		}

		if (state.clone != nullptr) {
			glUseProgram(instancedProgram);
			glm::mat4 cloneMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f))
				* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), deegressToRadians(90), glm::vec3(0.0f, -1.0f, 0.0f));
			(state.clone->Draw(instancedProgram, cloneMat, state.camera, state.directionalSource));
			glUseProgram(0);
		}

		if (state.rebel != nullptr) {
			glUseProgram(instancedProgram);
			glm::mat4 snowspeederMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f))
				* glm::rotate(glm::mat4(1.0f), deegressToRadians(90), glm::vec3(0.0f, 1.0f, 0.0f));
			(state.rebel->Draw(instancedProgram, snowspeederMat, state.camera, state.directionalSource));
			glUseProgram(0);
		}

		if (state.snowspeeder != nullptr) {
			glUseProgram(instancedProgram);
			glm::mat4 snowspeederMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.3f))
				* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), deegressToRadians(90), glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), deegressToRadians(90), glm::vec3(-1.0f, 0.0f, 0.0f));
			(state.snowspeeder->Draw(instancedProgram, snowspeederMat, state.camera, state.directionalSource));
			glUseProgram(0);
		}

		glUseProgram(0);
	}

	void Init() {
		glewInit();
		InitShader();
	}

	void Release() {
		ReleaseShader();
	}

};
