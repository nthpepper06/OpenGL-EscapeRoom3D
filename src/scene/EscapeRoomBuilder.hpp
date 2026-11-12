#pragma once

#include "core/Common.hpp"
#include "scene/SceneObject.hpp"
#include "interaction/InteractionManager.hpp"

class EscapeRoomBuilder {
public:
    static std::shared_ptr<SceneObject> buildDefaultRoom(InteractionManager& interactionMgr);

    static std::shared_ptr<SceneObject> buildHeavyVaultSafe(InteractionManager& interactionMgr,
                                                            const std::string& prefix,
                                                            const glm::vec3& position,
                                                            float rotationY,
                                                            unsigned int metalTex,
                                                            unsigned int stoneTex,
                                                            const glm::vec3& primaryColor = glm::vec3(0.20f, 0.22f, 0.25f),
                                                            const glm::vec3& accentColor = glm::vec3(0.85f, 0.85f, 0.90f));

    static std::shared_ptr<SceneObject> buildOrnateGoldenKey(InteractionManager& interactionMgr);

    static std::shared_ptr<SceneObject> buildExitDoor(InteractionManager& interactionMgr,
                                                      const std::string& prefix,
                                                      const glm::vec3& position,
                                                      float rotationY,
                                                      unsigned int frameTex,
                                                      unsigned int doorTex,
                                                      const glm::vec3& frameColor = glm::vec3(0.40f, 0.40f, 0.45f),
                                                      const glm::vec3& doorColor = glm::vec3(0.70f, 0.25f, 0.20f),
                                                      const glm::vec3& signGlowColor = glm::vec3(0.1f, 1.0f, 0.35f));
};
