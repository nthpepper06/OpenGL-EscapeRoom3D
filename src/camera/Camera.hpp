#pragma once

#include "core/Common.hpp"

enum class CameraMovement {
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};

class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;

    float movementSpeed;
    float mouseSensitivity;
    float fov;

    Camera(glm::vec3 startPos = glm::vec3(0.0f, 1.7f, 3.5f),
           glm::vec3 startUp = glm::vec3(0.0f, 1.0f, 0.0f),
           float startYaw = -90.0f,
           float startPitch = 0.0f);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio, float nearPlane = 0.1f, float farPlane = 100.0f) const;

    void processKeyboard(CameraMovement direction, float deltaTime, bool keepEyeLevel = true);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);

    void clampToBounds(const glm::vec3& minBounds, const glm::vec3& maxBounds);
    void reset(glm::vec3 newPos = glm::vec3(0.0f, 1.7f, 3.5f), float newYaw = -90.0f, float newPitch = 0.0f);

private:
    void updateCameraVectors();
};
