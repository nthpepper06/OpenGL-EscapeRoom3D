#pragma once

#include "core/Common.hpp"
#include "renderer/Light.hpp"
#include "scene/SceneObject.hpp"
#include "interaction/InteractionManager.hpp"

class BaseRoom {
public:
    RoomTheme theme = RoomTheme::OccultRitual;
    std::string name = "Horror Room";
    std::string description = "A mysterious haunted room.";
    std::string jsonFileName = "room_scene.json";
    std::string cluePin = "1234";

    glm::vec3 spawnPosition = glm::vec3(0.0f, 1.7f, 2.5f);
    float spawnYaw = -90.0f;
    float spawnPitch = 0.0f;
    glm::vec4 roomBounds = glm::vec4(-5.0f, 5.0f, -5.0f, 5.0f); // minX, maxX, minZ, maxZ

    PointLight pointLight;
    AmbientLight ambientLight;

    // PBR & IBL Ambient settings
    float iblIntensity = 1.2f;
    float ambientAO = 1.0f;

    // Atmosphere & Fog settings
    glm::vec3 fogColor = glm::vec3(0.08f, 0.08f, 0.12f);
    float fogDensity = 0.045f;
    float fogHeightFalloff = 0.4f;
    float inscatterIntensity = 0.8f;

    std::shared_ptr<SceneObject> rootNode;

    // Horror parameters
    float fearDistanceThreshold = 2.8f;
    glm::vec3 fearSourcePosition = glm::vec3(0.0f);
    float baseVignette = 0.45f;
    float baseChromatic = 0.0004f;

    virtual ~BaseRoom() = default;

    virtual void build(InteractionManager& interactionMgr) = 0;

    virtual void update(float deltaTime, float totalTime, const glm::vec3& playerPos) {
        if (rootNode) {
            rootNode->update(deltaTime);
        }
        pointLight.updateFlicker(deltaTime);
    }

    virtual float calculateFear(const glm::vec3& playerPos) const {
        float d = glm::distance(playerPos, fearSourcePosition);
        if (d >= fearDistanceThreshold) return 0.0f;
        float factor = 1.0f - (d / fearDistanceThreshold);
        return glm::clamp(factor * factor, 0.0f, 1.0f);
    }

    virtual std::string getObjectiveHint(const InteractionManager& im) const {
        (void)im;
        return "";
    }

    virtual void reset(InteractionManager& interactionMgr) {
        build(interactionMgr);
    }
};
