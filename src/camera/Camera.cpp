#include "camera/Camera.hpp"

Camera::Camera(glm::vec3 startPos, glm::vec3 startUp, float startYaw, float startPitch)
    : position(startPos),
      worldUp(startUp),
      yaw(startYaw),
      pitch(startPitch),
      movementSpeed(3.2f),
      mouseSensitivity(0.1f),
      fov(60.0f) {
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio, float nearPlane, float farPlane) const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime, bool keepEyeLevel) {
    float velocity = movementSpeed * deltaTime;
    glm::vec3 moveDir(0.0f);

    glm::vec3 forwardDir = front;
    if (keepEyeLevel) {
        forwardDir.y = 0.0f;
        forwardDir = glm::normalize(forwardDir);
    }

    glm::vec3 rightDir = right;
    if (keepEyeLevel) {
        rightDir.y = 0.0f;
        rightDir = glm::normalize(rightDir);
    }

    if (direction == CameraMovement::Forward)
        moveDir += forwardDir;
    if (direction == CameraMovement::Backward)
        moveDir -= forwardDir;
    if (direction == CameraMovement::Left)
        moveDir -= rightDir;
    if (direction == CameraMovement::Right)
        moveDir += rightDir;
    if (direction == CameraMovement::Up && !keepEyeLevel)
        moveDir += worldUp;
    if (direction == CameraMovement::Down && !keepEyeLevel)
        moveDir -= worldUp;

    if (glm::length(moveDir) > 0.0001f) {
        position += glm::normalize(moveDir) * velocity;
    }
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (constrainPitch) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset) {
    fov -= yoffset;
    if (fov < 20.0f) fov = 20.0f;
    if (fov > 90.0f) fov = 90.0f;
}

void Camera::clampToBounds(const glm::vec3& minBounds, const glm::vec3& maxBounds) {
    position.x = glm::clamp(position.x, minBounds.x, maxBounds.x);
    position.y = glm::clamp(position.y, minBounds.y, maxBounds.y);
    position.z = glm::clamp(position.z, minBounds.z, maxBounds.z);
}

void Camera::reset(glm::vec3 newPos, float newYaw, float newPitch) {
    position = newPos;
    yaw = newYaw;
    pitch = newPitch;
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}
