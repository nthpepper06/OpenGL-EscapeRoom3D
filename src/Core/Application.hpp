#pragma once

#include "core/Common.hpp"
#include "camera/Camera.hpp"
#include "camera/PlayerController.hpp"
#include "renderer/Shader.hpp"
#include "renderer/Light.hpp"
#include "renderer/PostProcessor.hpp"
#include "renderer/IblPipeline.hpp"
#include "renderer/ShadowPipeline.hpp"
#include "renderer/ParticleSystem.hpp"
#include "renderer/SsaoPipeline.hpp"
#include "scene/SceneObject.hpp"
#include "scene/SceneSerializer.hpp"
#include "interaction/InteractionManager.hpp"
#include "interaction/Raycaster.hpp"
#include "ui/GuiLayer.hpp"
#include "rooms/BaseRoom.hpp"
#include "rooms/OccultRitualRoom.hpp"
#include "rooms/AbandonedAsylumRoom.hpp"
#include "rooms/MirrorRoom.hpp"
#include "renderer/PlanarReflection.hpp"

class Application {
public:
    static Application* instance;

    Application();
    ~Application();

    bool init(int width = 1280, int height = 720, const std::string& title = "Multi-Theme Horror Escape Room 3D - Modern OpenGL 3.3");
    void run();

    // Callbacks
    void onFramebufferSize(int width, int height);
    void onMouseMove(double xpos, double ypos);
    void onMouseButton(int button, int action, int mods);
    void onKey(int key, int scancode, int action, int mods);
    void onScroll(double yoffset);

    // Multi-Room Switching
    void switchRoom(int index);
    void nextRoom();

    // Automated Diagnostics & Evidence Benchmark
    bool captureMode = false;
    bool testBugMode = false;
    void runBenchmarkAndCaptureScreenshots();
    void runBugDiagnostics();
    static bool saveScreenshot(const std::string& filepath, int width, int height);

private:
    GLFWwindow* window = nullptr;
    int windowWidth = 1280;
    int windowHeight = 720;

    AppMode appMode = AppMode::PlayMode;

    Camera camera;
    PlayerController playerController;
    Shader lightingShader;
    PostProcessor postProcessor;
    IblPipeline iblPipeline;
    ShadowPipeline shadowPipeline;
    ParticleSystem particleSystem;
    SsaoPipeline ssaoPipeline;
    PlanarReflection planarReflection;

    PointLight pointLight;
    SpotLight spotLight;
    AmbientLight ambientLight;

    std::shared_ptr<SceneObject> sceneRoot;
    InteractionManager interactionMgr;
    GuiLayer guiLayer;

    // Multi-room horror collection
    std::vector<std::shared_ptr<BaseRoom>> rooms;
    int currentRoomIndex = 0;

    // Horror dynamic state
    float currentFearFactor = 0.0f;

    std::string hoveredObjectId = "";
    std::string sceneFilePath = "occult_room.json";

    bool firstMouse = true;
    double lastX = 640.0;
    double lastY = 360.0;

    float deltaTime = 0.0f;
    float lastFrameTime = 0.0f;
    float totalTime = 0.0f;

    bool prevKeypadOpen = false;

    void processInput(float dt);
    void updateRaycast();
    Ray getMouseRay(double mouseX, double mouseY);

    void saveCurrentScene();
    void loadSceneFile();
};
