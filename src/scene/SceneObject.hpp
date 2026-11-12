#pragma once

#include "core/Common.hpp"
#include "renderer/Mesh.hpp"
#include "renderer/Material.hpp"
#include "renderer/Shader.hpp"

class SceneObject : public std::enable_shared_from_this<SceneObject> {
public:
    std::string id;
    std::string name;

    PrimitiveType primitiveType = PrimitiveType::None;
    std::shared_ptr<Mesh> mesh;
    Material material;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f); // Euler in degrees
    glm::vec3 scale = glm::vec3(1.0f);

    void setPosition(const glm::vec3& newPos);
    void setRotation(const glm::vec3& newRot);
    void setScale(const glm::vec3& newScale);

    bool isVisible = true;

    // Parent-Child hierarchy
    SceneObject* parent = nullptr;
    std::vector<std::shared_ptr<SceneObject>> children;

    // Interaction & Animation properties
    InteractionType interactType = InteractionType::None;
    bool isOpen = false;
    bool isLocked = false;
    float animProgress = 0.0f; // 0.0 to 1.0
    float animSpeed = 2.5f;

    glm::vec3 basePosition = glm::vec3(0.0f);
    glm::vec3 baseRotation = glm::vec3(0.0f);
    glm::vec3 openPositionOffset = glm::vec3(0.0f);
    glm::vec3 openRotationOffset = glm::vec3(0.0f);

    SceneObject(const std::string& objId, const std::string& objName, PrimitiveType type = PrimitiveType::None);

    void addChild(std::shared_ptr<SceneObject> child);
    bool removeChild(const std::string& childId);

    glm::mat4 getLocalMatrix() const;
    glm::mat4 getWorldMatrix() const;
    glm::mat3 getNormalMatrix() const;
    void markTransformDirty();
    AABB getWorldAABB() const;

    bool isInteractable() const { return interactType != InteractionType::None; }

    void update(float deltaTime);
    void draw(const Shader& shader, const std::string& hoveredId = "", const std::string& selectedId = "") const;

    std::shared_ptr<SceneObject> findById(const std::string& targetId);
    void collectAll(std::vector<std::shared_ptr<SceneObject>>& outList);

private:
    mutable glm::mat4 cachedWorldMatrix{1.0f};
    mutable glm::mat3 cachedNormalMatrix{1.0f};
    mutable glm::vec3 lastPosition{0.0f};
    mutable glm::vec3 lastRotation{0.0f};
    mutable glm::vec3 lastScale{1.0f};
    mutable bool isTransformDirty = true;
};
