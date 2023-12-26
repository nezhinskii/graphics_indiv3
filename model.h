#pragma once
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "lib/stb_image.h";

#include <iostream>;
#include <vector>

#include "camera.h";
#include "light_sources.h"

struct ObjVertex {
	glm::vec3 coords;
	glm::vec3 color;
	glm::vec2 textCoords;
	glm::vec3 normal;

	ObjVertex(aiVector3D aiCoords, aiColor3D color, aiVector3D aiTextCoords, aiVector3D normal) :
		coords(aiCoords.x, aiCoords.y, aiCoords.z),
		color(color.r, color.g, color.b),
		textCoords(aiTextCoords.x, aiTextCoords.y),
		normal(normal.x, normal.y, normal.z)
	{}
};

struct Material {
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 emission;
	GLfloat shininess;
	Material(aiColor3D ambient, aiColor3D diffuse, aiColor3D specular, aiColor3D emission, GLfloat shininess) :
		ambient(ambient.r, ambient.g, ambient.b),
		diffuse(diffuse.r, diffuse.g, diffuse.b),
		specular(specular.r, specular.g, specular.b),
		emission(emission.r, emission.g, emission.b),
		shininess(shininess)
	{}
	Material(){}
};

class Mesh {
	std::vector<ObjVertex> vertices;
	std::vector<GLuint> indices;
	std::vector<GLuint> textures;
	glm::vec3* translations;
	GLuint trLength;
	GLuint VBO, EBO;

	void loadTexture(const char* texturePath, GLuint& textureID) {
		int width, height, channels;
		unsigned char* image = stbi_load(texturePath, &width, &height, &channels, STBI_rgb);

		if (!image) {
			std::cerr << "Failed to load texture: " << texturePath << std::endl;
			return;
		}

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void setupBuffers() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ObjVertex), &vertices[0], GL_STATIC_DRAW);

		// coords
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ObjVertex), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		// color
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ObjVertex), (GLvoid*)offsetof(ObjVertex, color));
		glEnableVertexAttribArray(1);

		// textCoords
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ObjVertex), (GLvoid*)offsetof(ObjVertex, textCoords));
		glEnableVertexAttribArray(2);

		// normals
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(ObjVertex), (GLvoid*)offsetof(ObjVertex, normal));
		glEnableVertexAttribArray(3);

		if (this->trLength > 0) {
			GLuint instanceVBO;
			glGenBuffers(1, &instanceVBO);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * trLength, &translations[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glVertexAttribDivisor(4, 1);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	GLfloat deegressToRadians(GLfloat deegres) {
		return deegres * 3.141592f / 180.0f;
	}

public:
	GLuint VAO;
	Material material;

	Mesh(aiMesh* mesh, aiMaterial*  material, const std::string& modelDirectory, glm::vec3* translations, GLuint trLength) {
		this->translations = translations;
		this->trLength = trLength;

		aiColor3D ambient(0.10f, 0.10f, 0.10f);
		material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
		aiColor3D diffuse(0.8f, 0.8f, 0.8f);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
		aiColor3D specular(0.6f, 0.6f, 0.6f);
		material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
		aiColor3D emission(0.f, 0.f, 0.f);
		material->Get(AI_MATKEY_COLOR_EMISSIVE, emission);
		GLfloat shininess = 16.f;
		material->Get(AI_MATKEY_SHININESS, shininess);
		Material mat(ambient, diffuse, specular, emission, shininess);
		this->material = mat;

		for (GLuint j = 0; j < AI_TEXTURE_TYPE_MAX; ++j) {
			aiTextureType textureType = static_cast<aiTextureType>(j);
			aiString texturePath;
			if (material->GetTexture(textureType, 0, &texturePath) == AI_SUCCESS) {
				std::string fullPath = modelDirectory + '\\' + texturePath.C_Str();
				GLuint textureID;
				loadTexture(fullPath.c_str(), textureID);
				textures.push_back(textureID);
			}
		}

		aiColor3D color(1.f, 1.f, 1.f);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, color);

		for (GLuint i = 0; i < mesh->mNumVertices; ++i) {

			ObjVertex vertex(
				mesh->mVertices[i],
				color,
				mesh->mTextureCoords[0][i],
				mesh->mNormals[i]
			);
			vertices.push_back(vertex);
		}

		for (unsigned int j = 0; j < mesh->mNumFaces; ++j) {
			aiFace face = mesh->mFaces[j];
			for (unsigned int k = 0; k < face.mNumIndices; ++k) {
				indices.push_back(face.mIndices[k]);
			}
		}

		setupBuffers();
	}


	void Draw(const GLuint& shaderId, const glm::mat4& model, const Camera& camera, const DirectionalSource& dSource) {
		glUniform1i(glGetUniformLocation(shaderId, "numTextures"), textures.size());

		for (int i = 0; i < textures.size(); ++i) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, textures[i]);
			glUniform1i(glGetUniformLocation(shaderId, ("textures" + std::to_string(i)).c_str()), i);
		}

		glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		glBindVertexArray(VAO);
		glUniformMatrix4fv(glGetUniformLocation(shaderId, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shaderId, "view"), 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(shaderId, "projection"), 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));

		glUniform4f(glGetUniformLocation(shaderId, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform3f(glGetUniformLocation(shaderId, "camPos"), camera.position.x, camera.position.y, camera.position.z);

		glUniform3fv(glGetUniformLocation(shaderId, "material.ambient"), 1, glm::value_ptr(material.ambient));
		glUniform3fv(glGetUniformLocation(shaderId, "material.diffuse"), 1, glm::value_ptr(material.diffuse));
		glUniform3fv(glGetUniformLocation(shaderId, "material.specular"), 1, glm::value_ptr(material.specular));
		glUniform3fv(glGetUniformLocation(shaderId, "material.emission"), 1, glm::value_ptr(material.emission));
		glUniform1f(glGetUniformLocation(shaderId, "material.shininess"), material.shininess);

		glUniform1f(glGetUniformLocation(shaderId, "dSource.intensity"), dSource.intensity);
		glUniform3fv(glGetUniformLocation(shaderId, "dSource.direction"), 1, glm::value_ptr(dSource.direction));

		if (trLength > 0) {
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLuint>(indices.size()), GL_UNSIGNED_INT, 0, trLength);
		}
		else {
			glDrawElements(GL_TRIANGLES, static_cast<GLuint>(indices.size()), GL_UNSIGNED_INT, 0);
		}

		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);
	}
};


enum Shading : int {
	Phong,
	Toon,
	Rim
};

class Model {
public:
	glm::vec3 position = glm::vec3(0.0f, 5.0f, 0.0f);
	int shading = (int)Shading::Phong;
	std::vector<Mesh> meshes;

	Model(const std::string& path, glm::vec3* translations = nullptr, GLuint trLength = 0) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cerr << "Error loading model: " << importer.GetErrorString() << std::endl;
			return;
		}

		std::string modelDirectory = path;
		modelDirectory = modelDirectory.substr(0, modelDirectory.find_last_of('\\'));

		for (GLuint i = 0; i < scene->mNumMeshes; ++i) {
			Mesh mesh(scene->mMeshes[i], scene->mMaterials[scene->mMeshes[i]->mMaterialIndex], modelDirectory, translations, trLength);
			meshes.push_back(mesh);
		}
	}

	void Draw(const GLuint& shaderId, const glm::mat4& model, const Camera& camera, const DirectionalSource& dSource) {
		for (int i = 0; i < meshes.size(); ++i) {
			meshes[i].Draw(shaderId, model, camera, dSource);
		}
	}
};