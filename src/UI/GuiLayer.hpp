#pragma once

#include "core/Common.hpp"
#include "scene/SceneObject.hpp"
#include "renderer/Light.hpp"
#include "renderer/PostProcessor.hpp"
#include "renderer/ShadowPipeline.hpp"
#include "renderer/ParticleSystem.hpp"
#include "renderer/SsaoPipeline.hpp"
#include "interaction/InteractionManager.hpp"
#include "camera/Camera.hpp"

class GuiLayer {
public:
    std::shared_ptr<SceneObject> selectedObject = nullptr;
    bool showDemoWindow = false;
    int debugViewMode = 0; // 0: Final, 1: Albedo, 2: Normal, 3: Roughness, 4: Metallic, 5: AO, 6: IBL-only, 7: Shadow-only, 8: SSAO-only
    float iblIntensity = 1.2f;
    float ambientAO = 1.0f;

    void init(GLFWwindow* window);
    void shutdown();

    void beginFrame();
    void endFrame();

    void render(AppMode appMode,
                std::shared_ptr<SceneObject>& root,
                PointLight& pointLight,
                SpotLight& spotLight,
                AmbientLight& ambientLight,
                InteractionManager& interactionMgr,
                Camera& camera,
                const std::string& hoveredObjId,
                int windowWidth,
                int windowHeight,
                int currentRoomIndex,
                const std::string& currentRoomName,
                float fearFactor,
                PostProcessor& postProcessor,
                ShadowPipeline& shadowPipeline,
                ParticleSystem& particleSystem,
                SsaoPipeline& ssaoPipeline,
                std::function<void(int)> onSwitchRoom,
                std::function<void()> onSaveScene,
                std::function<void()> onLoadScene);

private:
    void renderHierarchy(std::shared_ptr<SceneObject>& root);
    void renderHierarchyNode(std::shared_ptr<SceneObject>& obj);
    void renderInspector(InteractionManager& interactionMgr);
    void renderLighting(PointLight& pointLight, SpotLight& spotLight, AmbientLight& ambientLight,
                        PostProcessor& postProcessor, ShadowPipeline& shadowPipeline,
                        ParticleSystem& particleSystem, SsaoPipeline& ssaoPipeline, float fearFactor);
    void renderGameHUD(AppMode appMode,
                       const InteractionManager& interactionMgr,
                       const std::string& promptText,
                       const std::string& roomName,
                       float fearFactor,
                       int windowWidth,
                       int windowHeight);
    void renderKeypadModal(InteractionManager& interactionMgr);
    void renderInspectModal(InteractionManager& interactionMgr, int windowWidth, int windowHeight);
    void renderVictoryModal(InteractionManager& interactionMgr, std::function<void(int)> onSwitchRoom);
};

