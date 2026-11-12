#include "core/Application.hpp"
#include <imgui.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <thread>

Application* Application::instance = nullptr;

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    if (auto app = static_cast<Application*>(glfwGetWindowUserPointer(window))) {
        app->onFramebufferSize(width, height);
    }
}

static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (auto app = static_cast<Application*>(glfwGetWindowUserPointer(window))) {
        app->onMouseMove(xpos, ypos);
    }
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (auto app = static_cast<Application*>(glfwGetWindowUserPointer(window))) {
        app->onMouseButton(button, action, mods);
    }
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (auto app = static_cast<Application*>(glfwGetWindowUserPointer(window))) {
        app->onKey(key, scancode, action, mods);
    }
}

static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (auto app = static_cast<Application*>(glfwGetWindowUserPointer(window))) {
        app->onScroll(yoffset);
    }
}

Application::Application() {
    instance = this;
}

Application::~Application() {
    audioEngine.shutdown();
    guiLayer.shutdown();
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

bool Application::init(int width, int height, const std::string& title) {
    windowWidth = width;
    windowHeight = height;

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);

    // Register GLFW Callbacks
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Initial cursor mode: captured for Play Mode
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!\n";
        return false;
    }

    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Color spaces & Hardware configuration
    glDisable(GL_FRAMEBUFFER_SRGB); // Framebuffer sRGB disabled; tonemapping and sRGB gamma applied once in post_process.frag
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // Seamless cubemap filtering across cube faces

    // Query and log GPU / OpenGL diagnostics
    std::cout << "\n========================================================\n";
    std::cout << "  [HARDWARE & OPENGL DIAGNOSTICS LOG]\n";
    std::cout << "  Vendor:                   " << glGetString(GL_VENDOR) << "\n";
    std::cout << "  Renderer (GPU):           " << glGetString(GL_RENDERER) << "\n";
    std::cout << "  OpenGL Version:           " << glGetString(GL_VERSION) << "\n";
    std::cout << "  GLSL Version:             " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

    GLfloat maxAniso = 1.0f;
    glGetFloatv(0x84FF, &maxAniso); // GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
    std::cout << "  Max Texture Anisotropy:   " << maxAniso << "x\n";
    std::cout << "========================================================\n\n";

    // Initialize IBL Split-Sum Pipeline (Irradiance 32x32, Prefilter 128x128 5 mips, BRDF LUT 512x512 RG16F)
    std::cout << "[IBL Pipeline] Generating procedural interior environment Split-Sum cubemaps...\n";
    iblPipeline.init();

    // Configure SpotLight defaults (Calibrated HDR flashlight with smoothstep cone falloff)
    spotLight.enabled = true;
    spotLight.color = glm::vec3(1.0f, 0.95f, 0.88f); // Warm tungsten/incandescent
    spotLight.intensity = 22.0f; // Calibrated HDR radiance
    spotLight.cutOff = glm::cos(glm::radians(12.5f));
    spotLight.outerCutOff = glm::cos(glm::radians(17.5f));
    spotLight.constant = 1.0f;
    spotLight.linear = 0.09f;
    spotLight.quadratic = 0.032f;

    // Load Main 3D Physically Based Rendering (PBR) Shaders
    std::string vertPath = "assets/shaders/pbr.vert";
    std::string fragPath = "assets/shaders/pbr.frag";
    if (!lightingShader.loadFromFiles(vertPath, fragPath)) {
        vertPath = "pbr.vert";
        fragPath = "pbr.frag";
        if (!lightingShader.loadFromFiles(vertPath, fragPath)) {
            std::cerr << "Warning: Failed to load PBR shaders, attempting fallback to Blinn-Phong...\n";
            if (!lightingShader.loadFromFiles("assets/shaders/blinn_phong.vert", "assets/shaders/blinn_phong.frag")) {
                std::cerr << "Fatal Error: Failed to load 3D scene lighting shaders!\n";
                return false;
            }
        }
    }

    // Load UV Decal Shaders for Early-Z optimized fluorescent decals
    if (!uvDecalShader.loadFromFiles("assets/shaders/uv_decal.vert", "assets/shaders/uv_decal.frag")) {
        uvDecalShader.loadFromFiles("uv_decal.vert", "uv_decal.frag");
    }

    // Initialize Post-Processing FBO Pipeline
    std::string postVert = "assets/shaders/post_process.vert";
    std::string postFrag = "assets/shaders/post_process.frag";
    if (!postProcessor.init(windowWidth, windowHeight, postVert, postFrag)) {
        std::cerr << "Warning: Failed to load assets/shaders/post_process, trying local directory...\n";
        if (!postProcessor.init(windowWidth, windowHeight, "post_process.vert", "post_process.frag")) {
            std::cerr << "Warning: PostProcessor shader initialization failed; fallback direct render will be used.\n";
            postProcessor.enabled = false;
        }
    }

    // Initialize Phase 2 Rendering Systems
    if (!shadowPipeline.init(1024, 512)) {
        std::cerr << "[Warning] ShadowPipeline initialization failed.\n";
    }
    if (!particleSystem.init(450)) {
        std::cerr << "[Warning] ParticleSystem initialization failed.\n";
    }
    if (!ssaoPipeline.init(windowWidth, windowHeight)) {
        std::cerr << "[Warning] SsaoPipeline initialization failed.\n";
    }
    if (!planarReflection.init(512)) {
        std::cerr << "[Warning] PlanarReflection initialization failed.\n";
    }

    // Initialize Dear ImGui UI Layer
    guiLayer.init(window);

    // Initialize Multi-Theme Horror Escape Rooms
    rooms.push_back(std::make_shared<OccultRitualRoom>());
    rooms.push_back(std::make_shared<AbandonedAsylumRoom>());
    rooms.push_back(std::make_shared<MirrorRoom>());

    // Initialize Player Controller
    playerController.init(&camera);
    playerController.onFootstep = [this](const glm::vec3& pos) {
        audioEngine.playFootstep(pos);
    };

    // Initialize 3D Spatial Audio Engine
    audioEngine.init();
    interactionMgr.audioEngine = &audioEngine;

    // Initialize Hardware GPU Profiler
    gpuProfiler.init();

    // Load first room (Occult Ritual Chamber)
    switchRoom(0);

    return true;
}

void Application::switchRoom(int index) {
    if (index < 0 || index >= static_cast<int>(rooms.size())) return;

    currentRoomIndex = index;
    auto room = rooms[currentRoomIndex];

    // Reset interaction manager puzzle states
    interactionMgr.resetGameState();
    interactionMgr.correctSafePin = room->cluePin;

    // Build the room scene hierarchy
    room->build(interactionMgr);
    sceneRoot = room->rootNode;
    pointLight = room->pointLight;
    ambientLight = room->ambientLight;
    sceneFilePath = room->jsonFileName;

    // Reset camera to room's designated spawn point
    camera = Camera(room->spawnPosition, glm::vec3(0.0f, 1.0f, 0.0f), room->spawnYaw, room->spawnPitch);
    playerController.setPosition(camera.position);
    playerController.playerVelocity = glm::vec3(0.0f);
    playerController.setRoomBounds(room->roomBounds);
    interactionMgr.currentRoomId = currentRoomIndex;

    // Close any open modals
    interactionMgr.closeInspect();
    interactionMgr.showKeypadModal = false;

    // Lock cursor to FPS mode
    if (appMode == AppMode::PlayMode && window) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        firstMouse = true;
    }

    // Apply room-specific post-processing baseline
    postProcessor.vignetteStrength = room->baseVignette;
    postProcessor.chromaticAberrationStrength = room->baseChromatic;

    // Synchronize IBL ambient controls
    guiLayer.iblIntensity = room->iblIntensity;
    guiLayer.ambientAO = room->ambientAO;

    // Invalidate static point shadow cubemap to force rebake for the new room
    shadowPipeline.markPointShadowDirty();

    // Save initial room configuration
    SceneSerializer::saveToFile(sceneFilePath, sceneRoot, pointLight, spotLight, ambientLight);

    interactionMgr.setMessage("Switched to Room " + std::to_string(currentRoomIndex + 1) + ": " + room->name + " | [F1-F3] Rooms | [F4] Solve Puzzle", 6.0f);
}

void Application::autoSolveCurrentRoom() {
    if (currentRoomIndex == 0) {
        // Room 0: Occult Ritual Chamber (Shadow Totem Alignment)
        interactionMgr.totemAngles[0] = 90.0f;
        interactionMgr.totemAngles[1] = 180.0f;
        interactionMgr.totemAngles[2] = 270.0f;
        if (sceneRoot) {
            auto t1 = sceneRoot->findById("totem_1");
            if (t1) t1->setRotation(glm::vec3(0.0f, 90.0f, 0.0f));
            auto t2 = sceneRoot->findById("totem_2");
            if (t2) t2->setRotation(glm::vec3(0.0f, 180.0f, 0.0f));
            auto t3 = sceneRoot->findById("totem_3");
            if (t3) t3->setRotation(glm::vec3(0.0f, 270.0f, 0.0f));
            auto key = sceneRoot->findById("golden_key");
            if (key) key->isVisible = false;
        }
        interactionMgr.altarUnlocked = true;
        if (interactionMgr.altarLidObject) {
            interactionMgr.altarLidObject->isOpen = true;
        }
        interactionMgr.hasGoldenKey = true;
        interactionMgr.hasSigil[0] = true;
        interactionMgr.sigilsCollected = std::max(interactionMgr.sigilsCollected, 1);
        interactionMgr.keysCollected = std::max(interactionMgr.keysCollected, 1);
        if (interactionMgr.exitDoorObject) {
            interactionMgr.exitDoorObject->isLocked = false;
            interactionMgr.exitDoorObject->isOpen = true;
        }
        audioEngine.playSafeUnlock(glm::vec3(0.0f, 1.0f, 0.0f));
        interactionMgr.setMessage("Room 1 Auto-Solved! Totems aligned, altar opened, Ritual Key acquired, exit unlocked.", 6.0f);
    } else if (currentRoomIndex == 1) {
        // Room 1: Abandoned Asylum (Breaker Box, UV Ciphers & Safe PIN 4108)
        interactionMgr.fusesCollected = 2;
        interactionMgr.powerRestored = true;
        pointLight.color = glm::vec3(0.75f, 0.95f, 0.92f);
        pointLight.intensity = 11.0f;
        if (interactionMgr.isolationGateObject) {
            interactionMgr.isolationGateObject->isOpen = true;
        }
        interactionMgr.safeUnlocked = true;
        if (interactionMgr.safeDoorObject) {
            interactionMgr.safeDoorObject->isOpen = true;
        }
        interactionMgr.hasGoldenKey = true;
        interactionMgr.hasSigil[1] = true;
        interactionMgr.sigilsCollected = std::max(interactionMgr.sigilsCollected, 2);
        interactionMgr.keysCollected = std::max(interactionMgr.keysCollected, 2);
        if (interactionMgr.exitDoorObject) {
            interactionMgr.exitDoorObject->isLocked = false;
            interactionMgr.exitDoorObject->isOpen = true;
        }
        if (sceneRoot) {
            auto key = sceneRoot->findById("golden_key");
            if (key) key->isVisible = false;
            auto f1 = sceneRoot->findById("fuse_1");
            if (f1) f1->isVisible = false;
            auto f2 = sceneRoot->findById("fuse_2");
            if (f2) f2->isVisible = false;
        }
        audioEngine.playSafeUnlock(glm::vec3(0.0f, 1.0f, 0.0f));
        interactionMgr.setMessage("Room 2 Auto-Solved! Power restored, security gate opened, PIN 4108 verified, exit unlocked.", 6.0f);
    } else if (currentRoomIndex == 2) {
        // Room 2: Haunted Mirror Sanctuary (Mirror World Bust Symmetry)
        interactionMgr.bustAngles[0] = 180.0f;
        interactionMgr.bustAngles[1] = 90.0f;
        interactionMgr.bustAngles[2] = 270.0f;
        if (sceneRoot) {
            auto b1 = sceneRoot->findById("bust_1");
            if (b1) b1->setRotation(glm::vec3(0.0f, 180.0f, 0.0f));
            auto b2 = sceneRoot->findById("bust_2");
            if (b2) b2->setRotation(glm::vec3(0.0f, 90.0f, 0.0f));
            auto b3 = sceneRoot->findById("bust_3");
            if (b3) b3->setRotation(glm::vec3(0.0f, 270.0f, 0.0f));
            auto key = sceneRoot->findById("golden_key");
            if (key) key->isVisible = false;
        }
        interactionMgr.mirrorPuzzleSolved = true;
        interactionMgr.hasGoldenKey = true;
        interactionMgr.hasSigil[2] = true;
        interactionMgr.sigilsCollected = 3;
        interactionMgr.keysCollected = 3;
        interactionMgr.gameWon = true;
        if (interactionMgr.exitDoorObject) {
            interactionMgr.exitDoorObject->isLocked = false;
            interactionMgr.exitDoorObject->isOpen = true;
        }
        audioEngine.playDoorCreak(glm::vec3(0.0f, 1.5f, 0.0f));
        interactionMgr.setMessage("Room 3 Auto-Solved! Mirror anomalies matched, Sanctuary seal broken, Exit Door open!", 6.0f);
    }
}

void Application::nextRoom() {
    int nextIdx = (currentRoomIndex + 1) % static_cast<int>(rooms.size());
    switchRoom(nextIdx);
}

void Application::run() {
    if (smokeTestMode) {
        runSmokeTest();
        return;
    }
    if (captureMode) {
        runBenchmarkAndCaptureScreenshots();
        return;
    }
    if (testBugMode) {
        runBugDiagnostics();
        return;
    }

    lastFrameTime = static_cast<float>(glfwGetTime());
    totalTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrameTime;
        lastFrameTime = currentFrame;
        totalTime += deltaTime;

        gpuProfiler.beginFrame();

        // Check if player unlocked the exit door and triggered a room transition
        if (interactionMgr.requestRoomTransition) {
            interactionMgr.requestRoomTransition = false;
            if (currentRoomIndex < static_cast<int>(rooms.size()) - 1) {
                switchRoom(currentRoomIndex + 1);
            } else {
                interactionMgr.gameWon = true;
                interactionMgr.setMessage("FINAL VICTORY! You have solved all 3 horror rooms and escaped Dr. Vance's asylum!", 10.0f);
            }
        }

        // Auto-manage cursor visibility when keypad modal or 3D inspect modal opens or closes
        bool modalActive = interactionMgr.showKeypadModal || interactionMgr.isInspectMode;
        if (modalActive && !prevKeypadOpen) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else if (!modalActive && prevKeypadOpen) {
            if (appMode == AppMode::PlayMode) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true;
            }
        }
        prevKeypadOpen = modalActive;

        // Process inputs
        processInput(deltaTime);

        // Update active room (animations, light flickering, dynamic horror fear)
        if (currentRoomIndex >= 0 && currentRoomIndex < static_cast<int>(rooms.size())) {
            rooms[currentRoomIndex]->update(deltaTime, totalTime, camera.position);
            pointLight = rooms[currentRoomIndex]->pointLight; // Sync flickering intensity
            currentFearFactor = rooms[currentRoomIndex]->calculateFear(camera.position);
        }

        // Update interaction timers
        interactionMgr.update(deltaTime);

        // Update 3D Spatial Audio listener and horror atmosphere
        audioEngine.updateListener(camera.position, camera.front, camera.up);
        audioEngine.updateAtmosphere(currentFearFactor, pointLight.position, pointLight.flickerEnabled);

        // Raycasting for target highlight
        updateRaycast();

        // Update Spotlight to follow character Camera flashlight
        spotLight.position = camera.position;
        spotLight.direction = camera.front;

        // Check if dynamic objects moved to invalidate static point shadow cubemap
        bool dynamicPropsMoving = false;
        if (interactionMgr.drawerObject && interactionMgr.drawerObject->animProgress > 0.0f && interactionMgr.drawerObject->animProgress < 1.0f) {
            dynamicPropsMoving = true;
        }
        if (interactionMgr.safeDoorObject && interactionMgr.safeDoorObject->animProgress > 0.0f && interactionMgr.safeDoorObject->animProgress < 1.0f) {
            dynamicPropsMoving = true;
        }
        if (dynamicPropsMoving) {
            shadowPipeline.markPointShadowDirty();
        }

        // ==========================================
        // PASS 0A: SPOTLIGHT SHADOW MAP PASS
        // ==========================================
        gpuProfiler.beginPass(GpuPass::SpotShadow);
        if (spotLight.enabled) {
            shadowPipeline.renderSpotShadow(spotLight, sceneRoot);
        }
        gpuProfiler.endPass(GpuPass::SpotShadow);

        // ==========================================
        // PASS 0B: OMNIDIRECTIONAL POINT SHADOW CUBEMAP PASS (CACHED)
        // ==========================================
        gpuProfiler.beginPass(GpuPass::PointShadow);
        if (pointLight.enabled && shadowPipeline.pointShadowEnabled) {
            shadowPipeline.renderPointShadow(pointLight, sceneRoot);
        }
        gpuProfiler.endPass(GpuPass::PointShadow);

        glm::mat4 projection = camera.getProjectionMatrix((float)windowWidth / (float)windowHeight);
        glm::mat4 view = camera.getViewMatrix();

        // ==========================================
        // PASS 0C: SSAO GEOMETRY PREPASS & COMPUTATION
        // ==========================================
        gpuProfiler.beginPass(GpuPass::SSAO);
        if (ssaoPipeline.enabled) {
            ssaoPipeline.renderGeometryPrepass(view, projection, sceneRoot);
            ssaoPipeline.renderSSAO(projection);
        }
        gpuProfiler.endPass(GpuPass::SSAO);

        // ==========================================
        // PASS 0D: PLANAR REFLECTION PASS (ROOM 2: MIRROR ROOM)
        // ==========================================
        gpuProfiler.beginPass(GpuPass::PlanarReflection);
        if (currentRoomIndex == 2 && planarReflection.enabled) {
            glm::mat4 refView, refProj;
            glm::vec3 mirrorPos(0.0f, 2.4f, -7.84f);
            glm::vec3 mirrorNormal(0.0f, 0.0f, 1.0f);
            planarReflection.computeReflectionMatrices(camera, mirrorPos, mirrorNormal, 1.0f, refView, refProj);

            planarReflection.beginRender();
            lightingShader.use();
            lightingShader.setMat4("projection", refProj);
            lightingShader.setMat4("view", refView);
            lightingShader.setVec3("viewPos", camera.position);
            lightingShader.setBool("usePlanarReflection", false);
            lightingShader.setBool("isUvLightActive", interactionMgr.isUvLightActive);
            lightingShader.setFloat("iblIntensity", guiLayer.iblIntensity);
            lightingShader.setFloat("ambientAO", guiLayer.ambientAO);
            lightingShader.setInt("debugViewMode", 0);

            if (sceneRoot) {
                sceneRoot->draw(lightingShader, "", "", RenderLayer::MirrorOnly);
            }
            planarReflection.endRender();
        }
        gpuProfiler.endPass(GpuPass::PlanarReflection);

        // ==========================================
        // PASS 1: RENDER 3D SCENE TO FBO TEXTURE (HDR)
        // ==========================================
        gpuProfiler.beginPass(GpuPass::PbrScene);
        postProcessor.bypassTonemap = (guiLayer.debugViewMode > 0);
        postProcessor.beginRender();

        lightingShader.use();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setVec3("viewPos", camera.position);

        // Global Ambient
        lightingShader.setVec3("ambientColor", ambientLight.color);
        lightingShader.setFloat("ambientIntensity", ambientLight.intensity);

        // Point Light (Ceiling / Altar Flickering Lamp)
        lightingShader.setBool("pointLight.enabled", pointLight.enabled);
        lightingShader.setVec3("pointLight.position", pointLight.position);
        lightingShader.setVec3("pointLight.color", pointLight.color);
        lightingShader.setFloat("pointLight.intensity", pointLight.intensity);
        lightingShader.setFloat("pointLight.constant", pointLight.constant);
        lightingShader.setFloat("pointLight.linear", pointLight.linear);
        lightingShader.setFloat("pointLight.quadratic", pointLight.quadratic);

        // Spotlight (Camera Flashlight)
        lightingShader.setBool("spotLight.enabled", spotLight.enabled);
        lightingShader.setVec3("spotLight.position", spotLight.position);
        lightingShader.setVec3("spotLight.direction", spotLight.direction);
        lightingShader.setVec3("spotLight.color", spotLight.color);
        lightingShader.setFloat("spotLight.intensity", spotLight.intensity);
        lightingShader.setFloat("spotLight.cutOff", spotLight.cutOff);
        lightingShader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);
        lightingShader.setFloat("spotLight.constant", spotLight.constant);
        lightingShader.setFloat("spotLight.linear", spotLight.linear);
        lightingShader.setFloat("spotLight.quadratic", spotLight.quadratic);

        // IBL Split-Sum Samplers & Controls
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_CUBE_MAP, iblPipeline.getIrradianceMap());
        lightingShader.setInt("irradianceMap", 6);

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_CUBE_MAP, iblPipeline.getPrefilterMap());
        lightingShader.setInt("prefilterMap", 7);

        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, iblPipeline.getBrdfLUT());
        lightingShader.setInt("brdfLUT", 8);

        // Phase 2: Shadows
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, shadowPipeline.getSpotShadowMap());
        lightingShader.setInt("spotShadowMap", 9);
        lightingShader.setMat4("spotLightMatrix", shadowPipeline.getSpotLightMatrix());
        lightingShader.setBool("spotShadowEnabled", spotLight.enabled);
        lightingShader.setFloat("spotBiasMin", shadowPipeline.spotBiasMin);
        lightingShader.setFloat("spotBiasMax", shadowPipeline.spotBiasMax);
        lightingShader.setFloat("spotNormalBias", shadowPipeline.spotNormalBias);
        lightingShader.setFloat("spotPcfRadius", shadowPipeline.spotPcfRadius);
        lightingShader.setInt("spotPcfTaps", shadowPipeline.spotPcfTaps);

        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadowPipeline.getPointShadowCubemap());
        lightingShader.setInt("pointShadowCubemap", 10);
        lightingShader.setBool("pointShadowEnabled", shadowPipeline.pointShadowEnabled && pointLight.enabled);
        lightingShader.setFloat("pointLightFarPlane", shadowPipeline.getPointFarPlane());
        lightingShader.setFloat("pointBiasMin", shadowPipeline.pointBiasMin);
        lightingShader.setFloat("pointBiasMax", shadowPipeline.pointBiasMax);
        lightingShader.setFloat("pointNormalBias", shadowPipeline.pointNormalBias);

        // Phase 2: SSAO
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_2D, ssaoPipeline.getSsaoTexture());
        lightingShader.setInt("ssaoTexture", 11);
        lightingShader.setBool("ssaoEnabled", ssaoPipeline.enabled);
        lightingShader.setVec2("screenSize", glm::vec2((float)windowWidth, (float)windowHeight));

        // Phase 3: Planar Reflection
        glActiveTexture(GL_TEXTURE12);
        glBindTexture(GL_TEXTURE_2D, planarReflection.getReflectionTexture());
        lightingShader.setInt("planarReflectionMap", 12);
        lightingShader.setBool("usePlanarReflection", (currentRoomIndex == 2) && planarReflection.enabled);

        // Phase 2: Fog & Atmosphere
        if (currentRoomIndex >= 0 && currentRoomIndex < static_cast<int>(rooms.size())) {
            lightingShader.setVec3("fogColor", rooms[currentRoomIndex]->fogColor);
            lightingShader.setFloat("fogDensity", rooms[currentRoomIndex]->fogDensity);
        } else {
            lightingShader.setVec3("fogColor", glm::vec3(0.04f, 0.03f, 0.05f));
            lightingShader.setFloat("fogDensity", 0.04f);
        }

        lightingShader.setFloat("iblIntensity", guiLayer.iblIntensity);
        lightingShader.setFloat("ambientAO", guiLayer.ambientAO);
        lightingShader.setInt("debugViewMode", guiLayer.debugViewMode);
        lightingShader.setBool("isUvLightActive", interactionMgr.isUvLightActive);
        lightingShader.setBool("uIsUVMode", interactionMgr.isUvLightActive);

        std::string selectedId = guiLayer.selectedObject ? guiLayer.selectedObject->id : "";
        bool inspectingItem = interactionMgr.isInspectMode && 
                              (interactionMgr.currentInspectType == InspectType::GoldenKey || interactionMgr.currentInspectType == InspectType::SigilFragment) &&
                              interactionMgr.keyObject != nullptr;
        bool origKeyVis = true;
        if (inspectingItem && interactionMgr.keyObject) {
            origKeyVis = interactionMgr.keyObject->isVisible;
            interactionMgr.keyObject->isVisible = false; // Hide from world pass while inspecting
        }

        if (sceneRoot) {
            sceneRoot->draw(lightingShader, hoveredObjectId, selectedId, RenderLayer::RealOnly);
        }

        // ------------------------------------------------------------------------
        // PASS 1B: UV FLUORESCENT DECAL OVERLAY (Additive Blend + Polygon Offset)
        // ------------------------------------------------------------------------
        if (interactionMgr.isUvLightActive || currentRoomIndex == 1) {
            uvDecalShader.use();
            uvDecalShader.setMat4("projection", projection);
            uvDecalShader.setMat4("view", view);
            uvDecalShader.setVec3("spotLight.position", spotLight.position);
            uvDecalShader.setVec3("spotLight.direction", spotLight.direction);
            uvDecalShader.setFloat("spotLight.cutOff", spotLight.cutOff);
            uvDecalShader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);
            uvDecalShader.setBool("spotLight.enabled", spotLight.enabled);
            uvDecalShader.setBool("uIsUVMode", interactionMgr.isUvLightActive);
            uvDecalShader.setBool("isUvLightActive", interactionMgr.isUvLightActive);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive fluorescent glow
            glDepthMask(GL_FALSE);             // Don't overwrite opaque depth buffer
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-1.0f, -1.0f);     // Zero Z-fighting against walls

            if (sceneRoot) {
                sceneRoot->draw(uvDecalShader, "", "", RenderLayer::UVDecalsOnly);
            }

            glDisable(GL_POLYGON_OFFSET_FILL);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        // ------------------------------------------------------------------------
        // PASS 1C: INSPECT MODE DEPTH ISOLATION (Turntable without scene clipping)
        // ------------------------------------------------------------------------
        if (inspectingItem && interactionMgr.keyObject) {
            glClear(GL_DEPTH_BUFFER_BIT); // Depth buffer isolation: never clipped by walls or desks

            glm::mat4 inspectProj = glm::perspective(glm::radians(45.0f), (float)windowWidth / (float)windowHeight, 0.05f, 10.0f);
            glm::mat4 inspectView = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.25f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            lightingShader.use();
            lightingShader.setMat4("projection", inspectProj);
            lightingShader.setMat4("view", inspectView);
            lightingShader.setVec3("viewPos", glm::vec3(0.0f, 0.0f, 1.25f));

            // Dedicated inspect lighting
            lightingShader.setBool("pointLight.enabled", true);
            lightingShader.setVec3("pointLight.position", glm::vec3(0.8f, 1.2f, 1.5f));
            lightingShader.setVec3("pointLight.color", glm::vec3(1.0f, 0.96f, 0.90f));
            lightingShader.setFloat("pointLight.intensity", 12.0f);
            lightingShader.setFloat("pointLight.constant", 1.0f);
            lightingShader.setFloat("pointLight.linear", 0.09f);
            lightingShader.setFloat("pointLight.quadratic", 0.032f);

            lightingShader.setBool("spotLight.enabled", false);
            lightingShader.setBool("pointShadowEnabled", false);
            lightingShader.setBool("spotShadowEnabled", false);
            lightingShader.setBool("usePlanarReflection", false);
            lightingShader.setBool("isUvLightActive", false);
            lightingShader.setBool("uIsUVMode", false);
            lightingShader.setFloat("highlightFactor", 0.0f);

            glm::vec3 origPos = interactionMgr.keyObject->position;
            glm::vec3 origRot = interactionMgr.keyObject->rotation;
            glm::vec3 origScale = interactionMgr.keyObject->scale;

            interactionMgr.keyObject->isVisible = true;
            interactionMgr.keyObject->position = glm::vec3(0.0f, 0.0f, 0.0f);
            interactionMgr.keyObject->rotation = glm::vec3(interactionMgr.inspectPitch, interactionMgr.inspectYaw, 0.0f);
            interactionMgr.keyObject->scale = glm::vec3(0.48f);

            interactionMgr.keyObject->draw(lightingShader, "", "", RenderLayer::Both);

            interactionMgr.keyObject->position = origPos;
            interactionMgr.keyObject->rotation = origRot;
            interactionMgr.keyObject->scale = origScale;
            interactionMgr.keyObject->isVisible = origKeyVis;
        }

        // Render Atmospheric Dust Motes
        if (particleSystem.enabled) {
            particleSystem.update(deltaTime, totalTime, camera.position);
            particleSystem.render(view, projection, camera.position, spotLight);
        }

        postProcessor.endRender();
        gpuProfiler.endPass(GpuPass::PbrScene);

        // ==========================================
        // PASS 2: HORROR POST-PROCESSING TO SCREEN
        // (Bloom + Vignette + Chromatic Aberration + Panic Heartbeat + Film Grain)
        // ==========================================
        gpuProfiler.beginPass(GpuPass::PostProcess);
        postProcessor.render(totalTime, currentFearFactor);
        gpuProfiler.endPass(GpuPass::PostProcess);

        // ==========================================
        // PASS 3: DEAR IMGUI UI OVERLAY (HUD / INSPECTOR)
        // ==========================================
        std::string currentRoomName = (currentRoomIndex >= 0 && currentRoomIndex < static_cast<int>(rooms.size())) 
                                      ? rooms[currentRoomIndex]->name : "Horror Room";

        guiLayer.beginFrame();
        guiLayer.render(appMode,
                        sceneRoot,
                        pointLight,
                        spotLight,
                        ambientLight,
                        interactionMgr,
                        camera,
                        hoveredObjectId,
                        windowWidth,
                        windowHeight,
                        currentRoomIndex,
                        currentRoomName,
                        currentFearFactor,
                        rooms[currentRoomIndex]->getObjectiveHint(interactionMgr),
                        postProcessor,
                        shadowPipeline,
                        particleSystem,
                        ssaoPipeline,
                        audioEngine,
                        gpuProfiler,
                        graphicsSettings,
                        [this](int roomIdx) { switchRoom(roomIdx); },
                        [this]() { saveCurrentScene(); },
                        [this]() { loadSceneFile(); });
        guiLayer.endFrame();

        gpuProfiler.endFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Application::processInput(float dt) {
    bool freeze = (interactionMgr.showKeypadModal || interactionMgr.isInspectMode);
    if (appMode == AppMode::PlayMode) {
        playerController.update(dt, window, freeze, sceneRoot);
    }
}

void Application::updateRaycast() {
    if (!sceneRoot) return;

    if (appMode == AppMode::PlayMode && !interactionMgr.showKeypadModal) {
        Ray ray = { camera.position, camera.front };
        std::vector<std::shared_ptr<SceneObject>> objects;
        sceneRoot->collectAll(objects);

        RaycastHit hit = Raycaster::castRay(ray, objects, 4.5f, true);
        if (hit.hasHit && hit.object) {
            hoveredObjectId = hit.object->id;
        } else {
            hoveredObjectId = "";
        }
    } else {
        hoveredObjectId = "";
    }
}

Ray Application::getMouseRay(double mouseX, double mouseY) {
    float x = (2.0f * static_cast<float>(mouseX)) / windowWidth - 1.0f;
    float y = 1.0f - (2.0f * static_cast<float>(mouseY)) / windowHeight;

    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);
    glm::mat4 invProj = glm::inverse(camera.getProjectionMatrix((float)windowWidth / (float)windowHeight));
    glm::vec4 rayEye = invProj * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    glm::mat4 invView = glm::inverse(camera.getViewMatrix());
    glm::vec3 rayWorld = glm::vec3(invView * rayEye);
    rayWorld = glm::normalize(rayWorld);

    return { camera.position, rayWorld };
}

void Application::onFramebufferSize(int width, int height) {
    if (width <= 0 || height <= 0) return;
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    postProcessor.resize(width, height);
    ssaoPipeline.resize(width, height);
}

void Application::onMouseMove(double xpos, double ypos) {
    if (interactionMgr.showKeypadModal) return;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - lastX);
    float yoffset = static_cast<float>(lastY - ypos); // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (interactionMgr.isInspectMode) {
        interactionMgr.rotateInspect(xoffset, yoffset);
        return;
    }

    if (appMode == AppMode::PlayMode) {
        camera.processMouseMovement(xoffset, yoffset);
    }
}

void Application::onMouseButton(int button, int action, int mods) {
    if (action != GLFW_PRESS) return;

    // Play Mode: Right click toggles UV Blacklight mode
    if (appMode == AppMode::PlayMode && button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (!spotLight.enabled) {
            spotLight.enabled = true;
            interactionMgr.isUvLightActive = false;
            spotLight.color = glm::vec3(1.0f, 0.95f, 0.88f);
            spotLight.intensity = 22.0f;
            interactionMgr.setMessage("[NORMAL FLASHLIGHT] Restored white beam. Press [Right-Click / F / V] for UV Blacklight!", 3.5f);
        } else {
            interactionMgr.isUvLightActive = !interactionMgr.isUvLightActive;
            if (interactionMgr.isUvLightActive) {
                spotLight.color = glm::vec3(0.45f, 0.12f, 1.0f);
                spotLight.intensity = 18.0f;
                interactionMgr.setMessage("[UV BLACKLIGHT ACTIVE] Violet spectrum reveals hidden fluorescent ciphers and blood trails!", 4.0f);
            } else {
                spotLight.color = glm::vec3(1.0f, 0.95f, 0.88f);
                spotLight.intensity = 22.0f;
                interactionMgr.setMessage("[NORMAL FLASHLIGHT] Restored standard white beam.", 3.0f);
            }
        }
        audioEngine.playKeypadBeep(true);
        return;
    }

    // Viewport Picking in Edit Mode
    if (appMode == AppMode::EditMode && button == GLFW_MOUSE_BUTTON_LEFT) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) return;

        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        Ray pickRay = getMouseRay(mx, my);

        if (sceneRoot) {
            std::vector<std::shared_ptr<SceneObject>> objects;
            sceneRoot->collectAll(objects);
            RaycastHit hit = Raycaster::castRay(pickRay, objects, 100.0f, false);
            if (hit.hasHit) {
                guiLayer.selectedObject = hit.object;
            } else {
                guiLayer.selectedObject = nullptr;
            }
        }
    }
}

void Application::onKey(int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    // Direct inspection handling
    if (interactionMgr.isInspectMode) {
        if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_E || key == GLFW_KEY_I) {
            interactionMgr.closeInspect();
            return;
        }
        return;
    }

    // Direct hardware keyboard input for Electronic Safe Keypad
    if (interactionMgr.showKeypadModal) {
        // Digits 0-9 (top row)
        if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
            if (interactionMgr.enteredPin.size() < 4) {
                interactionMgr.enteredPin += static_cast<char>('0' + (key - GLFW_KEY_0));
                audioEngine.playKeypadBeep(true);
            }
            return;
        }
        // Digits 0-9 (numpad)
        if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9) {
            if (interactionMgr.enteredPin.size() < 4) {
                interactionMgr.enteredPin += static_cast<char>('0' + (key - GLFW_KEY_KP_0));
                audioEngine.playKeypadBeep(true);
            }
            return;
        }
        // Backspace
        if (key == GLFW_KEY_BACKSPACE) {
            if (!interactionMgr.enteredPin.empty()) {
                interactionMgr.enteredPin.pop_back();
            }
            return;
        }
        // Enter / Return / Keypad Enter: Submit PIN
        if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) {
            interactionMgr.checkPin(interactionMgr.enteredPin);
            return;
        }
        // Clear via C
        if (key == GLFW_KEY_C) {
            interactionMgr.enteredPin.clear();
            return;
        }
        // Close modal via Escape
        if (key == GLFW_KEY_ESCAPE) {
            interactionMgr.showKeypadModal = false;
            return;
        }
        return; // Suppress other shortcuts while typing PIN
    }

    // Toggle Play Mode / Edit Mode via Tab
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        if (appMode == AppMode::PlayMode) {
            appMode = AppMode::EditMode;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            appMode = AppMode::PlayMode;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
        return;
    }

    // Fast Demo Hotkeys: F1, F2, F3 for room switching
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        switchRoom(0);
        return;
    }
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
        switchRoom(1);
        return;
    }
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
        switchRoom(2);
        return;
    }

    // Fast Demo Hotkey: F4 to auto-solve current room puzzle
    if (key == GLFW_KEY_F4 && action == GLFW_PRESS) {
        autoSolveCurrentRoom();
        return;
    }

    // Toggle Flashlight & UV Blacklight via F or V key
    if ((key == GLFW_KEY_F || key == GLFW_KEY_V) && action == GLFW_PRESS) {
        if (!spotLight.enabled) {
            spotLight.enabled = true;
            interactionMgr.isUvLightActive = (key == GLFW_KEY_V);
            spotLight.color = interactionMgr.isUvLightActive ? glm::vec3(0.45f, 0.12f, 1.0f) : glm::vec3(1.0f, 0.95f, 0.88f);
            spotLight.intensity = interactionMgr.isUvLightActive ? 18.0f : 22.0f;
            interactionMgr.setMessage(interactionMgr.isUvLightActive ? "[UV BLACKLIGHT ACTIVE] Violet spectrum reveals hidden fluorescent ciphers!"
                                                                     : "[NORMAL FLASHLIGHT] Restored white beam. Press [V / F] for UV Blacklight!", 3.5f);
        } else {
            interactionMgr.isUvLightActive = !interactionMgr.isUvLightActive;
            if (interactionMgr.isUvLightActive) {
                spotLight.color = glm::vec3(0.45f, 0.12f, 1.0f);
                spotLight.intensity = 18.0f;
                interactionMgr.setMessage("[UV BLACKLIGHT ACTIVE] Violet spectrum reveals hidden fluorescent ciphers and blood trails!", 4.0f);
            } else {
                spotLight.color = glm::vec3(1.0f, 0.95f, 0.88f);
                spotLight.intensity = 22.0f;
                interactionMgr.setMessage("[NORMAL FLASHLIGHT] Restored standard white beam.", 3.0f);
            }
        }
        audioEngine.playKeypadBeep(true);
        return;
    }

    // Quick room switching via Hotkeys 1, 2, 3
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        switchRoom(0);
        return;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        switchRoom(1);
        return;
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        switchRoom(2);
        return;
    }

    // Interaction key E (Play Mode only)
    if (key == GLFW_KEY_E && action == GLFW_PRESS && appMode == AppMode::PlayMode) {
        if (!hoveredObjectId.empty() && sceneRoot) {
            auto targetObj = sceneRoot->findById(hoveredObjectId);
            if (targetObj) {
                interactionMgr.interact(targetObj, pointLight);
            }
        }
        return;
    }

    // Inspect key I (Play Mode only)
    if (key == GLFW_KEY_I && action == GLFW_PRESS && appMode == AppMode::PlayMode) {
        if (interactionMgr.hasGoldenKey) {
            interactionMgr.openInspect(InspectType::GoldenKey, "GOLDEN KEY & SIGIL FRAGMENT",
                                       "Radiant ancient talisman reflecting the room environment in HDR.\n"
                                       "Required to unlock the Exit Door and escape.");
        } else if (interactionMgr.sigilsCollected > 0) {
            interactionMgr.openInspect(InspectType::SigilFragment, "ANCIENT SIGIL FRAGMENT",
                                       "An occult talisman capturing the tormented soul of an asylum victim.");
        } else {
            interactionMgr.openInspect(InspectType::SecretNote, "JOURNAL OF DR. ELIAS VANCE",
                                       "Ritual notes and security override PIN for this room:\n"
                                       "Security PIN: " + interactionMgr.correctSafePin);
        }
        return;
    }

    // Escape key handling
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (appMode == AppMode::PlayMode) {
            appMode = AppMode::EditMode;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void Application::onScroll(double yoffset) {
    if (appMode == AppMode::PlayMode) {
        camera.processMouseScroll(static_cast<float>(yoffset));
    }
}

void Application::saveCurrentScene() {
    if (sceneRoot) {
        if (SceneSerializer::saveToFile(sceneFilePath, sceneRoot, pointLight, spotLight, ambientLight)) {
            interactionMgr.setMessage("Saved room configuration to " + sceneFilePath, 3.0f);
        }
    }
}

void Application::loadSceneFile() {
    if (SceneSerializer::loadFromFile(sceneFilePath, sceneRoot, pointLight, spotLight, ambientLight)) {
        if (sceneRoot) {
            interactionMgr.drawerObject = sceneRoot->findById("desk_drawer");
            interactionMgr.safeDoorObject = sceneRoot->findById("safe_hinge");
            interactionMgr.exitDoorObject = sceneRoot->findById("door_hinge");
            interactionMgr.keyObject = sceneRoot->findById("golden_key");
            interactionMgr.setMessage("Loaded room configuration from " + sceneFilePath, 3.0f);
        }
    } else {
        interactionMgr.setMessage("Khong the nap file: " + sceneFilePath, 3.0f);
    }
}

bool Application::saveScreenshot(const std::string& filepath, int width, int height) {
    std::vector<unsigned char> pixels(width * height * 4);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    std::vector<unsigned char> flipped(width * height * 4);
    for (int y = 0; y < height; ++y) {
        memcpy(&flipped[y * width * 4], &pixels[(height - 1 - y) * width * 4], width * 4);
    }

    std::filesystem::path p(filepath);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }

    int res = stbi_write_png(filepath.c_str(), width, height, 4, flipped.data(), width * 4);
    return res != 0;
}

void Application::runBenchmarkAndCaptureScreenshots() {
    std::cout << "\n========================================================\n";
    std::cout << "  [AUTOMATED GRAPHICS BENCHMARK & SCREENSHOT CAPTURE]\n";
    std::cout << "========================================================\n";

    std::string artifactDir = "C:/Users/PC/.gemini/antigravity/brain/952df3f3-6c84-4b26-bd18-4a23b4a39fe2/screenshots";
    std::string localDir = "assets/screenshots";
    std::filesystem::create_directories(localDir);
    std::filesystem::create_directories(artifactDir);

    bool overridePointLight = false;
    bool suppressRaycast = false;

    auto renderSingleFrame = [this, &overridePointLight, &suppressRaycast](float dt, float t) {
        if (currentRoomIndex >= 0 && currentRoomIndex < static_cast<int>(rooms.size())) {
            rooms[currentRoomIndex]->update(dt, t, camera.position);
            pointLight = rooms[currentRoomIndex]->pointLight;
            if (overridePointLight) {
                pointLight.enabled = false;
            }
            currentFearFactor = rooms[currentRoomIndex]->calculateFear(camera.position);
        }
        interactionMgr.update(dt);
        if (!suppressRaycast) {
            updateRaycast();
        } else {
            hoveredObjectId = "";
        }

        spotLight.position = camera.position;
        spotLight.direction = camera.front;

        if (spotLight.enabled) {
            shadowPipeline.renderSpotShadow(spotLight, sceneRoot);
        }
        if (pointLight.enabled && shadowPipeline.pointShadowEnabled) {
            shadowPipeline.renderPointShadow(pointLight, sceneRoot);
        }

        glm::mat4 projection = camera.getProjectionMatrix((float)windowWidth / (float)windowHeight);
        glm::mat4 view = camera.getViewMatrix();

        if (ssaoPipeline.enabled) {
            ssaoPipeline.renderGeometryPrepass(view, projection, sceneRoot);
            ssaoPipeline.renderSSAO(projection);
        }

        if (currentRoomIndex == 2 && planarReflection.enabled) {
            glm::mat4 refView, refProj;
            glm::vec3 mirrorPos(0.0f, 2.4f, -7.84f);
            glm::vec3 mirrorNormal(0.0f, 0.0f, 1.0f);
            planarReflection.computeReflectionMatrices(camera, mirrorPos, mirrorNormal, 1.0f, refView, refProj);

            planarReflection.beginRender();
            lightingShader.use();
            lightingShader.setMat4("projection", refProj);
            lightingShader.setMat4("view", refView);
            lightingShader.setVec3("viewPos", camera.position);
            lightingShader.setBool("usePlanarReflection", false);
            lightingShader.setBool("isUvLightActive", interactionMgr.isUvLightActive);
            lightingShader.setFloat("iblIntensity", guiLayer.iblIntensity);
            lightingShader.setFloat("ambientAO", guiLayer.ambientAO);
            lightingShader.setInt("debugViewMode", 0);

            if (sceneRoot) {
                sceneRoot->draw(lightingShader, "", "", RenderLayer::MirrorOnly);
            }
            planarReflection.endRender();
        }

        postProcessor.bypassTonemap = (guiLayer.debugViewMode > 0);
        postProcessor.beginRender();

        lightingShader.use();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setVec3("viewPos", camera.position);

        lightingShader.setVec3("ambientColor", ambientLight.color);
        lightingShader.setFloat("ambientIntensity", ambientLight.intensity);

        lightingShader.setBool("pointLight.enabled", pointLight.enabled);
        lightingShader.setVec3("pointLight.position", pointLight.position);
        lightingShader.setVec3("pointLight.color", pointLight.color);
        lightingShader.setFloat("pointLight.intensity", pointLight.intensity);
        lightingShader.setFloat("pointLight.constant", pointLight.constant);
        lightingShader.setFloat("pointLight.linear", pointLight.linear);
        lightingShader.setFloat("pointLight.quadratic", pointLight.quadratic);

        lightingShader.setBool("spotLight.enabled", spotLight.enabled);
        lightingShader.setVec3("spotLight.position", spotLight.position);
        lightingShader.setVec3("spotLight.direction", spotLight.direction);
        lightingShader.setVec3("spotLight.color", spotLight.color);
        lightingShader.setFloat("spotLight.intensity", spotLight.intensity);
        lightingShader.setFloat("spotLight.cutOff", spotLight.cutOff);
        lightingShader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);
        lightingShader.setFloat("spotLight.constant", spotLight.constant);
        lightingShader.setFloat("spotLight.linear", spotLight.linear);
        lightingShader.setFloat("spotLight.quadratic", spotLight.quadratic);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_CUBE_MAP, iblPipeline.getIrradianceMap());
        lightingShader.setInt("irradianceMap", 6);

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_CUBE_MAP, iblPipeline.getPrefilterMap());
        lightingShader.setInt("prefilterMap", 7);

        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, iblPipeline.getBrdfLUT());
        lightingShader.setInt("brdfLUT", 8);

        // Phase 2: Shadows
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, shadowPipeline.getSpotShadowMap());
        lightingShader.setInt("spotShadowMap", 9);
        lightingShader.setMat4("spotLightMatrix", shadowPipeline.getSpotLightMatrix());
        lightingShader.setBool("spotShadowEnabled", spotLight.enabled);
        lightingShader.setFloat("spotBiasMin", shadowPipeline.spotBiasMin);
        lightingShader.setFloat("spotBiasMax", shadowPipeline.spotBiasMax);
        lightingShader.setFloat("spotNormalBias", shadowPipeline.spotNormalBias);
        lightingShader.setFloat("spotPcfRadius", shadowPipeline.spotPcfRadius);
        lightingShader.setInt("spotPcfTaps", shadowPipeline.spotPcfTaps);

        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadowPipeline.getPointShadowCubemap());
        lightingShader.setInt("pointShadowCubemap", 10);
        lightingShader.setBool("pointShadowEnabled", shadowPipeline.pointShadowEnabled && pointLight.enabled);
        lightingShader.setFloat("pointLightFarPlane", shadowPipeline.getPointFarPlane());
        lightingShader.setFloat("pointBiasMin", shadowPipeline.pointBiasMin);
        lightingShader.setFloat("pointBiasMax", shadowPipeline.pointBiasMax);
        lightingShader.setFloat("pointNormalBias", shadowPipeline.pointNormalBias);

        // Phase 2: SSAO
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_2D, ssaoPipeline.getSsaoTexture());
        lightingShader.setInt("ssaoTexture", 11);
        lightingShader.setBool("ssaoEnabled", ssaoPipeline.enabled);
        lightingShader.setVec2("screenSize", glm::vec2((float)windowWidth, (float)windowHeight));

        // Phase 3: Planar Reflection
        glActiveTexture(GL_TEXTURE12);
        glBindTexture(GL_TEXTURE_2D, planarReflection.getReflectionTexture());
        lightingShader.setInt("planarReflectionMap", 12);
        lightingShader.setBool("usePlanarReflection", (currentRoomIndex == 2) && planarReflection.enabled);

        if (currentRoomIndex >= 0 && currentRoomIndex < static_cast<int>(rooms.size())) {
            lightingShader.setVec3("fogColor", rooms[currentRoomIndex]->fogColor);
            lightingShader.setFloat("fogDensity", rooms[currentRoomIndex]->fogDensity);
        } else {
            lightingShader.setVec3("fogColor", glm::vec3(0.04f, 0.03f, 0.05f));
            lightingShader.setFloat("fogDensity", 0.04f);
        }

        lightingShader.setFloat("iblIntensity", guiLayer.iblIntensity);
        lightingShader.setFloat("ambientAO", guiLayer.ambientAO);
        lightingShader.setInt("debugViewMode", guiLayer.debugViewMode);
        lightingShader.setBool("isUvLightActive", interactionMgr.isUvLightActive);
        lightingShader.setBool("uIsUVMode", interactionMgr.isUvLightActive);

        std::string selectedId = guiLayer.selectedObject ? guiLayer.selectedObject->id : "";
        if (sceneRoot) {
            sceneRoot->draw(lightingShader, hoveredObjectId, selectedId, RenderLayer::RealOnly);
        }

        // UV Decal overlay pass during benchmark capture
        if (interactionMgr.isUvLightActive || currentRoomIndex == 1) {
            uvDecalShader.use();
            uvDecalShader.setMat4("projection", projection);
            uvDecalShader.setMat4("view", view);
            uvDecalShader.setVec3("spotLight.position", spotLight.position);
            uvDecalShader.setVec3("spotLight.direction", spotLight.direction);
            uvDecalShader.setFloat("spotLight.cutOff", spotLight.cutOff);
            uvDecalShader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);
            uvDecalShader.setBool("spotLight.enabled", spotLight.enabled);
            uvDecalShader.setBool("uIsUVMode", interactionMgr.isUvLightActive);
            uvDecalShader.setBool("isUvLightActive", interactionMgr.isUvLightActive);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glDepthMask(GL_FALSE);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-1.0f, -1.0f);

            if (sceneRoot) {
                sceneRoot->draw(uvDecalShader, "", "", RenderLayer::UVDecalsOnly);
            }

            glDisable(GL_POLYGON_OFFSET_FILL);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        if (particleSystem.enabled) {
            particleSystem.update(dt, t, camera.position);
            particleSystem.render(view, projection, camera.position, spotLight);
        }

        postProcessor.endRender();

        postProcessor.render(t, currentFearFactor);

        // Render UI HUD on top
        std::string currentRoomName = (currentRoomIndex >= 0 && currentRoomIndex < static_cast<int>(rooms.size())) 
                                      ? rooms[currentRoomIndex]->name : "Horror Room";
        guiLayer.beginFrame();
        guiLayer.render(appMode, sceneRoot, pointLight, spotLight, ambientLight,
                        interactionMgr, camera, hoveredObjectId, windowWidth, windowHeight,
                        currentRoomIndex, currentRoomName, currentFearFactor,
                        rooms[currentRoomIndex]->getObjectiveHint(interactionMgr),
                        postProcessor,
                        shadowPipeline, particleSystem, ssaoPipeline, audioEngine,
                        gpuProfiler, graphicsSettings,
                        [this](int r) { switchRoom(r); },
                        [this]() { saveCurrentScene(); },
                        [this]() { loadSceneFile(); });
        guiLayer.endFrame();
    };

    auto captureCurrentView = [&](const std::string& name) {
        for (int f = 0; f < 5; ++f) {
            renderSingleFrame(0.016f, f * 0.016f);
            if (f < 4) {
                glfwSwapBuffers(window);
                glfwPollEvents();
            }
        }
        std::string localPath = localDir + "/" + name;
        std::string artifactPath = artifactDir + "/" + name;
        saveScreenshot(localPath, windowWidth, windowHeight);
        saveScreenshot(artifactPath, windowWidth, windowHeight);
        std::cout << "[Screenshot Captured] " << name << " -> " << localPath << "\n";
        glfwSwapBuffers(window);
        glfwPollEvents();
    };

    // 1. CAPTURE 3 ROOMS (Final, Albedo, Normal)
    for (int r = 0; r < 3; ++r) {
        switchRoom(r);
        std::cout << "\n[Capturing Diagnostics] Room " << r << " (" << rooms[r]->name << ")...\n";
        
        // Final
        guiLayer.debugViewMode = 0;
        captureCurrentView("room" + std::to_string(r) + "_final.png");

        // Albedo
        guiLayer.debugViewMode = 1;
        captureCurrentView("room" + std::to_string(r) + "_albedo.png");

        // Normal
        guiLayer.debugViewMode = 2;
        captureCurrentView("room" + std::to_string(r) + "_normal.png");
    }

    // 2. CLOSE-UPS IN ROOM 0
    switchRoom(0);
    guiLayer.debugViewMode = 0;
    spotLight.enabled = true;
    suppressRaycast = true;
    hoveredObjectId = "";

    // Safe Close-up: position camera in front of safe, facing closed safe
    std::cout << "\n[Capturing Close-up] Vault Safe in Room 0...\n";
    if (interactionMgr.safeDoorObject) {
        interactionMgr.safeDoorObject->isOpen = false;
        interactionMgr.safeDoorObject->animProgress = 0.0f;
        interactionMgr.safeDoorObject->rotation = interactionMgr.safeDoorObject->baseRotation;
        interactionMgr.safeUnlocked = false;
    }
    camera = Camera(glm::vec3(3.2f, 1.45f, 1.15f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -5.0f);
    captureCurrentView("safe_closeup.png");

    // Golden Key Close-up: open safe door and focus on glowing golden key
    std::cout << "[Capturing Close-up] Golden Key inside Safe...\n";
    if (interactionMgr.safeDoorObject) {
        interactionMgr.safeDoorObject->isOpen = true;
        interactionMgr.safeDoorObject->animProgress = 1.0f;
        interactionMgr.safeDoorObject->rotation = interactionMgr.safeDoorObject->baseRotation + interactionMgr.safeDoorObject->openRotationOffset;
        interactionMgr.safeUnlocked = true;
    }
    camera = Camera(glm::vec3(3.2f, 1.55f, -0.45f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -8.0f);
    captureCurrentView("golden_key_closeup.png");

    // Flashlight on Wall: Stand close to brick wall facing north, turn ceiling light off to isolate flashlight
    std::cout << "[Capturing Close-up] Spotlight Smoothstep Falloff on Wall...\n";
    camera = Camera(glm::vec3(0.0f, 2.2f, -2.5f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
    overridePointLight = true;
    captureCurrentView("flashlight_wall.png");
    overridePointLight = false;
    suppressRaycast = false;

    // 3. PERFORMANCE BENCHMARK (120 frames in Room 0)
    std::cout << "\n[Running 120-Frame Performance Benchmark in Room 0]...\n";
    switchRoom(0);
    guiLayer.debugViewMode = 0;

    auto startTime = std::chrono::high_resolution_clock::now();
    const int benchmarkFrames = 120;
    for (int i = 0; i < benchmarkFrames; ++i) {
        renderSingleFrame(0.016f, i * 0.016f);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> totalElapsedMs = endTime - startTime;
    double avgFrameTimeMs = totalElapsedMs.count() / benchmarkFrames;
    double measuredFps = 1000.0 / avgFrameTimeMs;

    std::cout << "\n========================================================\n";
    std::cout << "  [PERFORMANCE BENCHMARK RESULTS]\n";
    std::cout << "  Total Frames Rendered:    " << benchmarkFrames << "\n";
    std::cout << "  Average Frame Time:       " << std::fixed << std::setprecision(2) << avgFrameTimeMs << " ms\n";
    std::cout << "  Estimated Average FPS:    " << std::fixed << std::setprecision(1) << measuredFps << " FPS\n";
    std::cout << "  Viewport Resolution:      " << windowWidth << "x" << windowHeight << "\n";
    std::cout << "========================================================\n\n";

    std::cout << "[Automated Benchmark & Capture Completed Successfully.]\n";
}

void Application::runBugDiagnostics() {
    std::cout << "\n========================================================\n";
    std::cout << "  [SYSTEMATIC REGRESSION TEST: 3 UNIQUE ROOM MECHANICS]\n";
    std::cout << "========================================================\n\n";

    // ==========================================
    // ROOM 0: OCCULT RITUAL ROOM (SHADOW TOTEM ALIGNMENT)
    // ==========================================
    {
        switchRoom(0);
        std::cout << ">>> [TEST 1/3] ROOM 0: " << rooms[0]->name << " <<<\n";
        std::cout << "Checking Room Dimensions: 12m x 12m, bounds=(" 
                  << rooms[0]->roomBounds.x << ", " << rooms[0]->roomBounds.y << ", "
                  << rooms[0]->roomBounds.z << ", " << rooms[0]->roomBounds.w << ")\n";

        auto altarLid = sceneRoot->findById("altar_lid");
        auto totem1 = sceneRoot->findById("totem_1");
        auto totem2 = sceneRoot->findById("totem_2");
        auto totem3 = sceneRoot->findById("totem_3");
        auto ritualKey = sceneRoot->findById("golden_key");
        auto exitDoor = sceneRoot->findById("occult_door");

        if (!altarLid || !totem1 || !totem2 || !totem3 || !ritualKey || !exitDoor) {
            std::cerr << "[FAIL] Core puzzle objects missing in Room 0!\n";
            exit(1);
        }

        std::cout << "[1.1] Verifying 3 Rotatable Totems interactType...\n";
        if (totem1->interactType != InteractionType::Totem ||
            totem2->interactType != InteractionType::Totem ||
            totem3->interactType != InteractionType::Totem) {
            std::cerr << "[FAIL] Totems do not have InteractionType::Totem!\n";
            exit(1);
        }

        std::cout << "[1.2] Rotating Totems to Target Angles (Totem1=90, Totem2=180, Totem3=270)...\n";
        // Totem 1: rotate once (0 -> 90)
        interactionMgr.interact(totem1, pointLight);
        // Totem 2: rotate twice (0 -> 90 -> 180)
        interactionMgr.interact(totem2, pointLight);
        interactionMgr.interact(totem2, pointLight);
        // Totem 3: rotate thrice (0 -> 90 -> 180 -> 270)
        interactionMgr.interact(totem3, pointLight);
        interactionMgr.interact(totem3, pointLight);
        interactionMgr.interact(totem3, pointLight);

        if (!interactionMgr.altarUnlocked) {
            std::cerr << "[FAIL] Altar failed to unlock after totems reached target angles!\n";
            exit(1);
        }
        std::cout << ">>> [SUCCESS] Altar unlocked! altarUnlocked=" << interactionMgr.altarUnlocked << "\n";

        // Simulate 30 frames of update for smooth sliding lid animation
        glm::vec3 lidPos0 = glm::vec3(altarLid->getWorldMatrix()[3]);
        for (int f = 0; f < 30; ++f) {
            rooms[0]->update(0.016f, f * 0.016f, camera.position);
        }
        glm::vec3 lidPos1 = glm::vec3(altarLid->getWorldMatrix()[3]);
        float lidDelta = glm::distance(lidPos0, lidPos1);
        std::cout << "[1.3] Altar Lid Slide Animation Delta: " << lidDelta << " meters.\n";
        if (lidDelta < 0.1f) {
            std::cerr << "[FAIL] Altar lid did not slide open!\n";
            exit(1);
        }

        // Pick up Ritual Key & Sigil #1
        std::cout << "[1.4] Collecting Ritual Key...\n";
        interactionMgr.interact(ritualKey, pointLight);
        if (!interactionMgr.hasGoldenKey || interactionMgr.keysCollected < 1) {
            std::cerr << "[FAIL] Failed to collect Ritual Key!\n";
            exit(1);
        }
        std::cout << ">>> [SUCCESS] Ritual Key collected! Total keys: " << interactionMgr.keysCollected << "/3\n";

        // Interact with Exit Door
        std::cout << "[1.5] Unlocking South Exit Door with Key...\n";
        interactionMgr.interact(exitDoor, pointLight);
        if (!interactionMgr.requestRoomTransition) {
            std::cerr << "[FAIL] Exit door did not trigger room transition!\n";
            exit(1);
        }
        std::cout << ">>> [PASS] Room 0 Shadow Totem Alignment verified 100%.\n\n";
    }

    // ==========================================
    // ROOM 1: ABANDONED ASYLUM (FUSES, BREAKER & UV BLACKLIGHT)
    // ==========================================
    {
        switchRoom(1);
        std::cout << ">>> [TEST 2/3] ROOM 1: " << rooms[1]->name << " <<<\n";
        std::cout << "Checking Room Dimensions: 14m x 8m, bounds=(" 
                  << rooms[1]->roomBounds.x << ", " << rooms[1]->roomBounds.y << ", "
                  << rooms[1]->roomBounds.z << ", " << rooms[1]->roomBounds.w << ")\n";

        auto fuse1 = sceneRoot->findById("fuse_1");
        auto fuse2 = sceneRoot->findById("fuse_2");
        auto breakerBox = sceneRoot->findById("breaker_box");
        auto ironGate = sceneRoot->findById("iron_bars_gate");
        auto uvPanel = sceneRoot->findById("uv_wall_runes");
        auto exitDoor = sceneRoot->findById("asylum_door");

        if (!fuse1 || !fuse2 || !breakerBox || !ironGate || !uvPanel || !exitDoor) {
            std::cerr << "[FAIL] Core puzzle objects missing in Room 1!\n";
            exit(1);
        }

        std::cout << "[2.1] Collecting 2 Fuses (Trolley & Washbasin)...\n";
        interactionMgr.interact(fuse1, pointLight);
        interactionMgr.interact(fuse2, pointLight);
        if (interactionMgr.fusesCollected != 2) {
            std::cerr << "[FAIL] Fuses collected count is not 2! Count=" << interactionMgr.fusesCollected << "\n";
            exit(1);
        }
        std::cout << ">>> [SUCCESS] Both fuses collected (2/2).\n";

        std::cout << "[2.2] Installing Fuses into Breaker Box & Restoring Power...\n";
        interactionMgr.interact(breakerBox, pointLight);
        if (!interactionMgr.powerRestored) {
            std::cerr << "[FAIL] Power not restored after interacting with breaker box!\n";
            exit(1);
        }
        if (!ironGate->isOpen) {
            std::cerr << "[FAIL] Security gate did not open after power restoration!\n";
            exit(1);
        }
        std::cout << ">>> [SUCCESS] Power restored! Ceiling lights fluorescent, security gate opened.\n";

        // Simulate 30 frames for iron gate sliding
        glm::vec3 gatePos0 = glm::vec3(ironGate->getWorldMatrix()[3]);
        for (int f = 0; f < 30; ++f) {
            rooms[1]->update(0.016f, f * 0.016f, camera.position);
        }
        glm::vec3 gatePos1 = glm::vec3(ironGate->getWorldMatrix()[3]);
        float gateDelta = glm::distance(gatePos0, gatePos1);
        std::cout << "[2.3] Iron Gate Slide Animation Delta: " << gateDelta << " meters.\n";
        if (gateDelta < 0.1f) {
            std::cerr << "[FAIL] Security gate did not slide open!\n";
            exit(1);
        }

        std::cout << "[2.4] Verifying UV Fluorescent Wall Panel...\n";
        if (!uvPanel->material.isFluorescent) {
            std::cerr << "[FAIL] Wall runes material is not marked isFluorescent!\n";
            exit(1);
        }
        interactionMgr.isUvLightActive = true;
        std::cout << ">>> [SUCCESS] UV Blacklight active. Fluorescent color=("
                  << uvPanel->material.fluorescentColor.r << ", "
                  << uvPanel->material.fluorescentColor.g << ", "
                  << uvPanel->material.fluorescentColor.b << ")\n";

        std::cout << "[2.5] Submitting Clue PIN 4108 to Isolation Exit Door Keypad...\n";
        bool pinOk = interactionMgr.checkPin("4108");
        if (!pinOk || !interactionMgr.safeUnlocked) {
            std::cerr << "[FAIL] Submitting correct PIN 4108 failed!\n";
            exit(1);
        }
        std::cout << ">>> [SUCCESS] Door unlocked via PIN 4108!\n";

        auto asylumKey = sceneRoot->findById("golden_key");
        if (asylumKey) {
            interactionMgr.interact(asylumKey, pointLight);
        }
        std::cout << ">>> [PASS] Room 1 Circuit Breaker & UV Blacklight verified 100%.\n\n";
    }

    // ==========================================
    // ROOM 2: HAUNTED MIRROR GALLERY (MIRROR-ONLY ANOMALIES & SYMMETRY)
    // ==========================================
    {
        switchRoom(2);
        std::cout << ">>> [TEST 3/3] ROOM 2: " << rooms[2]->name << " <<<\n";
        std::cout << "Checking Room Dimensions: 8m x 16m, bounds=(" 
                  << rooms[2]->roomBounds.x << ", " << rooms[2]->roomBounds.y << ", "
                  << rooms[2]->roomBounds.z << ", " << rooms[2]->roomBounds.w << ")\n";

        auto bust1 = sceneRoot->findById("bust_1");
        auto bust2 = sceneRoot->findById("bust_2");
        auto bust3 = sceneRoot->findById("bust_3");
        auto ghost1 = sceneRoot->findById("mirror_ghost_guide_1");
        auto ghost2 = sceneRoot->findById("mirror_ghost_guide_2");
        auto ghost3 = sceneRoot->findById("mirror_ghost_guide_3");
        auto centerMirror = sceneRoot->findById("center_mirror");
        auto exitDoor = sceneRoot->findById("mirror_door");

        if (!bust1 || !bust2 || !bust3 || !ghost1 || !ghost2 || !ghost3 || !centerMirror || !exitDoor) {
            std::cerr << "[FAIL] Core puzzle objects missing in Room 2!\n";
            exit(1);
        }

        std::cout << "[3.1] Verifying RenderLayer separation (RealOnly vs MirrorOnly)...\n";
        if (ghost1->renderLayer != RenderLayer::MirrorOnly ||
            ghost2->renderLayer != RenderLayer::MirrorOnly ||
            ghost3->renderLayer != RenderLayer::MirrorOnly) {
            std::cerr << "[FAIL] Ghost guides are not set to RenderLayer::MirrorOnly!\n";
            exit(1);
        }
        std::cout << ">>> [SUCCESS] Ghost guides verified as RenderLayer::MirrorOnly.\n";

        std::cout << "[3.2] Rotating Busts to Match Mirror World Symmetry (Bust1=180, Bust2=90, Bust3=270)...\n";
        // Bust 1: rotate twice (0 -> 90 -> 180)
        interactionMgr.interact(bust1, pointLight);
        interactionMgr.interact(bust1, pointLight);
        // Bust 2: rotate once (0 -> 90)
        interactionMgr.interact(bust2, pointLight);
        // Bust 3: rotate thrice (0 -> 90 -> 180 -> 270)
        interactionMgr.interact(bust3, pointLight);
        interactionMgr.interact(bust3, pointLight);
        interactionMgr.interact(bust3, pointLight);

        if (!interactionMgr.mirrorPuzzleSolved) {
            std::cerr << "[FAIL] Mirror symmetry puzzle did not solve after rotating busts!\n";
            exit(1);
        }
        std::cout << ">>> [SUCCESS] Mirror symmetry puzzle solved! mirrorPuzzleSolved=" << interactionMgr.mirrorPuzzleSolved << "\n";

        auto finalKey = sceneRoot->findById("golden_key");
        if (finalKey) {
            interactionMgr.interact(finalKey, pointLight);
        }
        interactionMgr.interact(exitDoor, pointLight);

        if (!interactionMgr.gameWon) {
            std::cerr << "[FAIL] gameWon is false after solving all 3 rooms!\n";
            exit(1);
        }
        std::cout << ">>> [SUCCESS] GAME WON! All 3 rooms completed and verified!\n";
        std::cout << ">>> [PASS] Room 2 Mirror Anomaly & Symmetry verified 100%.\n\n";
    }

    std::cout << "========================================================\n";
    std::cout << ">>> [ALL REGRESSION TESTS PASSED]: 3/3 rooms verified 100%.\n";
    std::cout << "========================================================\n";
}

void Application::runSmokeTest() {
    std::cout << "\n========================================================\n";
    std::cout << "  [HEADLESS / SMOKE TEST: ALL 3 ROOMS & RENDER CYCLES]\n";
    std::cout << "========================================================\n";

    GLenum initialErr = glGetError();
    if (initialErr != GL_NO_ERROR) {
        std::cerr << "[SMOKE TEST ERROR] Initial GL error present: 0x" << std::hex << initialErr << std::dec << "\n";
    }

    for (int r = 0; r < 3; ++r) {
        std::cout << "\n[Smoke Test] Loading Room " << r << " (" << rooms[r]->name << ")..." << std::endl;
        switchRoom(r);

        for (int frame = 0; frame < 10; ++frame) {
            float dt = 0.016f;
            float time = frame * dt;

            // Update room
            rooms[r]->update(dt, time, camera.position);

            // Execute passes
            if (spotLight.enabled) {
                shadowPipeline.renderSpotShadow(spotLight, sceneRoot);
            }
            if (pointLight.enabled && shadowPipeline.pointShadowEnabled) {
                shadowPipeline.renderPointShadow(pointLight, sceneRoot);
            }

            glm::mat4 proj = camera.getProjectionMatrix((float)windowWidth / (float)windowHeight);
            glm::mat4 view = camera.getViewMatrix();

            if (ssaoPipeline.enabled) {
                ssaoPipeline.renderGeometryPrepass(view, proj, sceneRoot);
                ssaoPipeline.renderSSAO(proj);
            }

            if (r == 2 && planarReflection.enabled) {
                glm::mat4 refView, refProj;
                planarReflection.computeReflectionMatrices(camera, glm::vec3(0.0f, 2.4f, -7.84f), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f, refView, refProj);
                planarReflection.beginRender();
                planarReflection.endRender();
            }

            postProcessor.beginRender();
            lightingShader.use();
            if (sceneRoot) {
                sceneRoot->draw(lightingShader, "", "", RenderLayer::Both);
            }
            postProcessor.endRender();

            postProcessor.render(time, 0.0f);

            glfwSwapBuffers(window);
            glfwPollEvents();

            GLenum err = glGetError();
            if (err != GL_NO_ERROR) {
                std::cerr << "[SMOKE TEST FAILED] GL Error 0x" << std::hex << err << std::dec 
                          << " in Room " << r << " at frame " << frame << "!\n";
                exit(1);
            }
        }
        std::cout << ">>> [PASS] Room " << r << " rendered 10 frames cleanly with 0 GL errors." << std::endl;
    }

    std::cout << "\n========================================================\n";
    std::cout << ">>> [ALL SMOKE TESTS PASSED]: 3/3 rooms verified with 0 GL errors.\n";
    std::cout << "========================================================\n";
}
