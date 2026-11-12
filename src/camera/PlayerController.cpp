#include "camera/PlayerController.hpp"
#include <cmath>
#include <algorithm>

PlayerController::PlayerController() {}

void PlayerController::init(Camera* cam) {
    camera = cam;
    if (camera) {
        currentPosition = camera->position;
        baseEyeHeight = camera->position.y;
        fov = camera->fov;
        mouseSensitivity = camera->mouseSensitivity;
    }
}

void PlayerController::setPosition(const glm::vec3& newPos) {
    currentPosition = newPos;
    playerVelocity = glm::vec3(0.0f);
    if (camera) {
        camera->position = newPos;
    }
}

glm::vec3 PlayerController::getPosition() const {
    return currentPosition;
}

void PlayerController::handleKeyboardInput(GLFWwindow* window, float dt, glm::vec3& wishDir) {
    wishDir = glm::vec3(0.0f);
    if (!camera) return;

    glm::vec3 flatFront = glm::normalize(glm::vec3(camera->front.x, 0.0f, camera->front.z));
    glm::vec3 flatRight = glm::normalize(glm::vec3(camera->right.x, 0.0f, camera->right.z));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) wishDir += flatFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) wishDir -= flatFront;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) wishDir += flatRight;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) wishDir -= flatRight;

    if (glm::length(wishDir) > 0.001f) {
        wishDir = glm::normalize(wishDir);
        isMoving = true;
    } else {
        isMoving = false;
    }

    isSprinting = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
}

void PlayerController::collectStaticColliders(const std::shared_ptr<SceneObject>& node, std::vector<AABB>& colliders) {
    if (!node || !node->isVisible) return;

    // Obstacles with solid bounding boxes (excluding floor, ceiling, triggers, and open drawers)
    if (node->id != "floor" && node->id != "floor_mirror" && node->id != "floor_occult" && node->id != "floor_asylum" &&
        node->id != "ceiling" && node->id != "ceiling_mirror" && node->id != "ceiling_occult" && node->id != "ceiling_asylum" &&
        node->id != "room_root" && node->id != "room_occult_root" && node->id != "room_asylum_root" && node->id != "room_mirror_root" &&
        node->id != "golden_key" && node->id != "key_shaft" && node->id != "key_bow" && node->id != "key_ring" &&
        node->id != "clue_note" && node->id != "desk_drawer" && node->id != "safe_hinge" && node->id != "safe_door_panel") {

        // Check if it's a prominent furniture or pedestal obstacle
        if (node->id.find("altar") != std::string::npos ||
            node->id.find("stand") != std::string::npos ||
            node->id.find("cabinet") != std::string::npos ||
            node->id.find("gurney") != std::string::npos ||
            node->id.find("desk") != std::string::npos ||
            node->id.find("pillar") != std::string::npos) {
            
            AABB box = node->getWorldAABB();
            // Only add substantial vertical colliders
            if ((box.max.y - box.min.y) > 0.3f && (box.max.x - box.min.x) > 0.2f) {
                colliders.push_back(box);
            }
        }
    }

    for (const auto& child : node->children) {
        collectStaticColliders(child, colliders);
    }
}

void PlayerController::resolveCollisions(glm::vec3& pos, glm::vec3& vel, float dt, const std::vector<AABB>& colliders) {
    const int SUB_STEPS = 3;
    float subDt = dt / static_cast<float>(SUB_STEPS);

    for (int step = 0; step < SUB_STEPS; ++step) {
        // Integrate X motion
        pos.x += vel.x * subDt;
        AABB playerBoxX = {
            glm::vec3(pos.x - playerRadius, 0.1f, pos.z - playerRadius),
            glm::vec3(pos.x + playerRadius, playerHeight, pos.z + playerRadius)
        };

        for (const auto& box : colliders) {
            bool overlap = (playerBoxX.min.x < box.max.x && playerBoxX.max.x > box.min.x &&
                            playerBoxX.min.y < box.max.y && playerBoxX.max.y > box.min.y &&
                            playerBoxX.min.z < box.max.z && playerBoxX.max.z > box.min.z);
            if (overlap) {
                if (vel.x > 0.0f) {
                    pos.x = box.min.x - playerRadius - 0.001f;
                } else if (vel.x < 0.0f) {
                    pos.x = box.max.x + playerRadius + 0.001f;
                }
                vel.x = 0.0f; // Slide along Z
                break;
            }
        }

        // Integrate Z motion
        pos.z += vel.z * subDt;
        AABB playerBoxZ = {
            glm::vec3(pos.x - playerRadius, 0.1f, pos.z - playerRadius),
            glm::vec3(pos.x + playerRadius, playerHeight, pos.z + playerRadius)
        };

        for (const auto& box : colliders) {
            bool overlap = (playerBoxZ.min.x < box.max.x && playerBoxZ.max.x > box.min.x &&
                            playerBoxZ.min.y < box.max.y && playerBoxZ.max.y > box.min.y &&
                            playerBoxZ.min.z < box.max.z && playerBoxZ.max.z > box.min.z);
            if (overlap) {
                if (vel.z > 0.0f) {
                    pos.z = box.min.z - playerRadius - 0.001f;
                } else if (vel.z < 0.0f) {
                    pos.z = box.max.z + playerRadius + 0.001f;
                }
                vel.z = 0.0f; // Slide along X
                break;
            }
        }

        // Enforce room outer perimeter bounds
        float roomMinX = -4.5f + playerRadius;
        float roomMaxX =  4.5f - playerRadius;
        float roomMinZ = -4.5f + playerRadius;
        float roomMaxZ =  4.5f - playerRadius;

        if (pos.x < roomMinX) { pos.x = roomMinX; vel.x = 0.0f; }
        if (pos.x > roomMaxX) { pos.x = roomMaxX; vel.x = 0.0f; }
        if (pos.z < roomMinZ) { pos.z = roomMinZ; vel.z = 0.0f; }
        if (pos.z > roomMaxZ) { pos.z = roomMaxZ; vel.z = 0.0f; }
    }
}

void PlayerController::update(float deltaTime, GLFWwindow* window, bool freezeInput, const std::shared_ptr<SceneObject>& sceneRoot) {
    if (!camera) return;

    if (freezeInput) {
        playerVelocity = glm::vec3(0.0f);
        return;
    }

    glm::vec3 wishDir(0.0f);
    handleKeyboardInput(window, deltaTime, wishDir);

    float targetSpeed = isSprinting ? sprintSpeed : walkSpeed;
    glm::vec3 targetVelocity = wishDir * targetSpeed;

    // Smooth acceleration & friction
    if (isMoving) {
        playerVelocity = glm::mix(playerVelocity, targetVelocity, std::min(1.0f, acceleration * deltaTime));
    } else {
        playerVelocity = glm::mix(playerVelocity, glm::vec3(0.0f), std::min(1.0f, friction * deltaTime));
    }

    // Collect static colliders from scene
    std::vector<AABB> colliders;
    if (sceneRoot) {
        collectStaticColliders(sceneRoot, colliders);
    }

    // Resolve collision with sliding
    resolveCollisions(currentPosition, playerVelocity, deltaTime, colliders);

    // Head-bobbing
    float bobOffset = 0.0f;
    float currentHorizontalSpeed = glm::length(glm::vec2(playerVelocity.x, playerVelocity.z));

    if (headBobEnabled && currentHorizontalSpeed > 0.2f) {
        float bobSpeedMult = isSprinting ? 1.4f : 1.0f;
        headBobTimer += deltaTime * headBobFrequency * bobSpeedMult;
        bobOffset = std::sin(headBobTimer) * headBobAmplitude;
    } else {
        headBobTimer = 0.0f;
        bobOffset = 0.0f;
    }

    currentPosition.y = baseEyeHeight;
    camera->position = currentPosition + glm::vec3(0.0f, bobOffset, 0.0f);
    camera->fov = fov;
    camera->mouseSensitivity = mouseSensitivity;
}
