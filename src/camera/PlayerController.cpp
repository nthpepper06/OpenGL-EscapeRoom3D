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

void PlayerController::setRoomBounds(const glm::vec4& bounds) {
    roomBounds = bounds;
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

    // Skip items, triggers, open doors, and mirror-only ghostly objects
    if (node->renderLayer == RenderLayer::MirrorOnly) return;

    bool isFloorOrCeiling = (node->id.find("floor") != std::string::npos || node->id.find("ceiling") != std::string::npos);
    bool isPickupOrSmall = (node->id.find("key") != std::string::npos ||
                            node->id.find("fuse") != std::string::npos ||
                            node->id.find("note") != std::string::npos ||
                            node->id.find("flame") != std::string::npos ||
                            node->id.find("skull") != std::string::npos ||
                            node->id.find("rune") != std::string::npos ||
                            node->id.find("clue") != std::string::npos);
    bool isOpenPassable = (node->isOpen && (node->id.find("door") != std::string::npos ||
                                            node->id.find("gate") != std::string::npos ||
                                            node->id.find("drawer") != std::string::npos));

    if (!isFloorOrCeiling && !isPickupOrSmall && !isOpenPassable && node->primitiveType != PrimitiveType::None) {
        // Check if it's a solid architectural or obstacle element
        if (node->id.find("pillar") != std::string::npos ||
            node->id.find("altar") != std::string::npos ||
            node->id.find("totem") != std::string::npos ||
            node->id.find("partition") != std::string::npos ||
            node->id.find("bars") != std::string::npos ||
            (node->id.find("gate") != std::string::npos && !node->isOpen) ||
            node->id.find("wall_inner") != std::string::npos ||
            node->id.find("breaker") != std::string::npos ||
            node->id.find("gurney") != std::string::npos ||
            node->id.find("trolley") != std::string::npos ||
            node->id.find("basin") != std::string::npos ||
            node->id.find("sink") != std::string::npos ||
            node->id.find("pedestal") != std::string::npos ||
            node->id.find("bust") != std::string::npos ||
            node->id.find("desk") != std::string::npos ||
            node->id.find("cabinet") != std::string::npos ||
            node->id.find("safe") != std::string::npos) {

            AABB box = node->getWorldAABB();
            // Solid barrier dimensions check
            if ((box.max.y - box.min.y) > 0.25f &&
                (box.max.x - box.min.x) > 0.12f &&
                (box.max.z - box.min.z) > 0.12f) {
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
        float roomMinX = roomBounds.x + playerRadius;
        float roomMaxX = roomBounds.y - playerRadius;
        float roomMinZ = roomBounds.z + playerRadius;
        float roomMaxZ = roomBounds.w - playerRadius;

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

    // Head-bobbing & Footstep audio trigger
    float bobOffset = 0.0f;
    float currentHorizontalSpeed = glm::length(glm::vec2(playerVelocity.x, playerVelocity.z));

    if (currentHorizontalSpeed > 0.2f) {
        float bobSpeedMult = isSprinting ? 1.4f : 1.0f;
        if (headBobEnabled) {
            headBobTimer += deltaTime * headBobFrequency * bobSpeedMult;
            bobOffset = std::sin(headBobTimer) * headBobAmplitude;
        }

        stepTimer += deltaTime * bobSpeedMult;
        if (stepTimer >= 0.44f) {
            stepTimer = 0.0f;
            if (onFootstep) {
                onFootstep(currentPosition);
            }
        }
    } else {
        headBobTimer = 0.0f;
        bobOffset = 0.0f;
        stepTimer = 0.35f;
    }

    currentPosition.y = baseEyeHeight;
    camera->position = currentPosition + glm::vec3(0.0f, bobOffset, 0.0f);
    camera->fov = fov;
    camera->mouseSensitivity = mouseSensitivity;
}
