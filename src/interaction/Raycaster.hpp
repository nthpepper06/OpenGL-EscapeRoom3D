#pragma once

#include "core/Common.hpp"
#include "scene/SceneObject.hpp"

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

struct RaycastHit {
    bool hasHit = false;
    float distance = 1e9f;
    std::shared_ptr<SceneObject> object = nullptr;
    glm::vec3 hitPoint = glm::vec3(0.0f);
};

class Raycaster {
public:
    static bool intersectAABB(const Ray& ray, const AABB& aabb, float maxDist, float& outDist);

    static RaycastHit castRay(const Ray& ray,
                              const std::vector<std::shared_ptr<SceneObject>>& objects,
                              float maxDistance = 3.5f,
                              bool interactableOnly = true);
};
