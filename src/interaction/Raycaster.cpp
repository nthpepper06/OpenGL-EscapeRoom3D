#include "interaction/Raycaster.hpp"

bool Raycaster::intersectAABB(const Ray& ray, const AABB& aabb, float maxDist, float& outDist) {
    float tmin = 0.001f;
    float tmax = maxDist;

    for (int i = 0; i < 3; ++i) {
        if (std::abs(ray.direction[i]) < 1e-6f) {
            if (ray.origin[i] < aabb.min[i] || ray.origin[i] > aabb.max[i]) {
                return false;
            }
        } else {
            float invD = 1.0f / ray.direction[i];
            float t0 = (aabb.min[i] - ray.origin[i]) * invD;
            float t1 = (aabb.max[i] - ray.origin[i]) * invD;
            if (invD < 0.0f) std::swap(t0, t1);

            tmin = t0 > tmin ? t0 : tmin;
            tmax = t1 < tmax ? t1 : tmax;
            if (tmax <= tmin) return false;
        }
    }

    outDist = tmin;
    return true;
}

RaycastHit Raycaster::castRay(const Ray& ray,
                              const std::vector<std::shared_ptr<SceneObject>>& objects,
                              float maxDistance,
                              bool interactableOnly) {
    RaycastHit bestHit;
    bestHit.distance = maxDistance;

    for (const auto& obj : objects) {
        if (!obj || !obj->isVisible) continue;
        if (interactableOnly && !obj->isInteractable()) continue;

        AABB box = obj->getWorldAABB();
        float dist = 0.0f;
        if (intersectAABB(ray, box, maxDistance, dist)) {
            if (dist < bestHit.distance) {
                bestHit.hasHit = true;
                bestHit.distance = dist;
                bestHit.object = obj;
                bestHit.hitPoint = ray.origin + ray.direction * dist;
            }
        }
    }

    return bestHit;
}
