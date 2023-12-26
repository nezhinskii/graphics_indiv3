#pragma once
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera(glm::vec3 position, GLfloat aspectRatio) :
        position(position),
        front(glm::vec3(0.0f, 0.0f, -1.0f)),
        up(glm::vec3(0.0f, 1.0f, 0.0f)),
        right(glm::vec3(1.0f, 0.0f, 0.0f)),
        worldUp(up),
        yaw(-90.0f),
        pitch(0.0f),
        speed(0.1f),
        sensitivity(0.025f),
        fov(45),
        aspectRatio(aspectRatio),
        nearPlane(0.1f),
        farPlane(1000.0f)
    {
        updateCameraVectors();
    }

    glm::vec3 getOffsetPosition() const {
        return position - glm::normalize(front) * 2.5f + glm::normalize(up) * 1.5f;
    }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(getOffsetPosition(), position, worldUp);
    }

    glm::mat4 getProjectionMatrix() const {
        return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    }

    void processResize(GLuint width, GLuint height) {
        aspectRatio = (GLfloat)width / height;
    }

    void processKeyboard(sf::Keyboard::Key key) {
        float velocity = speed;
        if (key == sf::Keyboard::W)
            position += front * velocity;
        if (key == sf::Keyboard::S)
            position -= front * velocity;
        if (key == sf::Keyboard::A)
            position -= right * velocity;
        if (key == sf::Keyboard::D)
            position += right * velocity;
    }

    void processMouseMovement(GLfloat xoffset, GLfloat yoffset) {
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
        updateCameraVectors();
    }
    glm::vec3 position;
    glm::vec3 front, up, right, worldUp;
    GLfloat yaw, pitch, speed, sensitivity, aspectRatio, fov, nearPlane, farPlane;
private:

    void updateCameraVectors() {
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(front);
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};