#pragma once

#include "core/Common.hpp"
#include "camera/Camera.hpp"
#include "scene/SceneObject.hpp"
#include <vector>

class PlayerController {
public:
    PlayerController();

    void init(Camera* cam);
    void update(float deltaTime, GLFWwindow* window, bool freezeInput, const std::shared_ptr<SceneObject>& sceneRoot);

    // Movement settings
    float walkSpeed = 3.2f;
    float sprintSpeed = 5.2f;
    float acceleration = 24.0f;
    float friction = 14.0f;
    float mouseSensitivity = 0.08f;
    float fov = 65.0f;

    // Head-bobbing
    bool headBobEnabled = true;
    float headBobFrequency = 8.5f;
    float headBobAmplitude = 0.04f;
    float headBobTimer = 0.0f;

    // Collision capsule
    float playerRadius = 0.32f;
    float playerHeight = 1.68f;
    glm::vec3 playerVelocity{0.0f};
    bool isMoving = false;
    bool isSprinting = false;

    // Audio callback for footsteps
    std::function<void(const glm::vec3&)> onFootstep = nullptr;
    float stepTimer = 0.0f;

    void setPosition(const glm::vec3& newPos);
    glm::vec3 getPosition() const;

private:
    Camera* camera = nullptr;
    glm::vec3 currentPosition{0.0f, 1.68f, 2.5f};
    float baseEyeHeight = 1.68f;

    void handleKeyboardInput(GLFWwindow* window, float dt, glm::vec3& wishDir);
    void resolveCollisions(glm::vec3& pos, glm::vec3& vel, float dt, const std::vector<AABB>& colliders);
    void collectStaticColliders(const std::shared_ptr<SceneObject>& node, std::vector<AABB>& colliders);
};
