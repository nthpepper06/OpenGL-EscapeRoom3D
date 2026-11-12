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

    // Initialize Dear ImGui UI Layer
    guiLayer.init(window);

    // Initialize Multi-Theme Horror Escape Rooms
    rooms.push_back(std::make_shared<OccultRitualRoom>());
    rooms.push_back(std::make_shared<AbandonedAsylumRoom>());
    rooms.push_back(std::make_shared<MirrorRoom>());

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

    // Apply room-specific post-processing baseline
    postProcessor.vignetteStrength = room->baseVignette;
    postProcessor.chromaticAberrationStrength = room->baseChromatic;

    // Synchronize IBL ambient controls
    guiLayer.iblIntensity = room->iblIntensity;
    guiLayer.ambientAO = room->ambientAO;

    // Save initial room configuration
    SceneSerializer::saveToFile(sceneFilePath, sceneRoot, pointLight, spotLight, ambientLight);

    interactionMgr.setMessage("Da vao: " + room->name + " - " + room->description, 6.0f);
}

void Application::nextRoom() {
    int nextIdx = (currentRoomIndex + 1) % static_cast<int>(rooms.size());
    switchRoom(nextIdx);
}

void Application::run() {
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

        // Check if player unlocked the exit door and triggered a room transition
        if (interactionMgr.requestRoomTransition) {
            interactionMgr.requestRoomTransition = false;
            if (currentRoomIndex < static_cast<int>(rooms.size()) - 1) {
                switchRoom(currentRoomIndex + 1);
            } else {
                interactionMgr.gameWon = true;
                interactionMgr.setMessage("CHIEN THANG CUOI CUNG! Ban da vuot qua toan bo 3 can phong kinh di!", 10.0f);
            }
        }

        // Auto-manage cursor visibility when keypad modal opens or closes
        if (interactionMgr.showKeypadModal && !prevKeypadOpen) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else if (!interactionMgr.showKeypadModal && prevKeypadOpen) {
            if (appMode == AppMode::PlayMode) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true;
            }
        }
        prevKeypadOpen = interactionMgr.showKeypadModal;

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

        // Raycasting for target highlight
        updateRaycast();

        // Update Spotlight to follow character Camera flashlight
        spotLight.position = camera.position;
        spotLight.direction = camera.front;

        // ==========================================
        // PASS 1: RENDER 3D SCENE TO FBO TEXTURE
        // ==========================================
        postProcessor.bypassTonemap = (guiLayer.debugViewMode > 0);
        postProcessor.beginRender();

        lightingShader.use();

        glm::mat4 projection = camera.getProjectionMatrix((float)windowWidth / (float)windowHeight);
        glm::mat4 view = camera.getViewMatrix();
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

        lightingShader.setFloat("iblIntensity", guiLayer.iblIntensity);
        lightingShader.setFloat("ambientAO", guiLayer.ambientAO);
        lightingShader.setInt("debugViewMode", guiLayer.debugViewMode);

        std::string selectedId = guiLayer.selectedObject ? guiLayer.selectedObject->id : "";
        if (sceneRoot) {
            sceneRoot->draw(lightingShader, hoveredObjectId, selectedId);
        }

        postProcessor.endRender();

        // ==========================================
        // PASS 2: HORROR POST-PROCESSING TO SCREEN
        // (Vignette + Chromatic Aberration + Panic Heartbeat)
        // ==========================================
        postProcessor.render(totalTime, currentFearFactor);

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
                        postProcessor,
                        [this](int roomIdx) { switchRoom(roomIdx); },
                        [this]() { saveCurrentScene(); },
                        [this]() { loadSceneFile(); });
        guiLayer.endFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Application::processInput(float dt) {
    if (interactionMgr.showKeypadModal) return; // Freeze movement while typing PIN

    if (appMode == AppMode::PlayMode) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.processKeyboard(CameraMovement::Forward, dt);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.processKeyboard(CameraMovement::Backward, dt);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.processKeyboard(CameraMovement::Left, dt);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.processKeyboard(CameraMovement::Right, dt);

        // Keep player inside room bounds (X: -4.5 to 4.5, Z: -4.5 to 4.5, Y: 1.7)
        camera.clampToBounds(glm::vec3(-4.5f, 1.7f, -4.5f), glm::vec3(4.5f, 1.7f, 4.5f));
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
}

void Application::onMouseMove(double xpos, double ypos) {
    if (interactionMgr.showKeypadModal) return;

    if (appMode == AppMode::PlayMode) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = static_cast<float>(xpos - lastX);
        float yoffset = static_cast<float>(lastY - ypos); // Reversed since y-coordinates go from bottom to top

        lastX = xpos;
        lastY = ypos;

        camera.processMouseMovement(xoffset, yoffset);
    }
}

void Application::onMouseButton(int button, int action, int mods) {
    if (action != GLFW_PRESS) return;

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

    // Direct hardware keyboard input for Electronic Safe Keypad
    if (interactionMgr.showKeypadModal) {
        // Digits 0-9 (top row)
        if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
            if (interactionMgr.enteredPin.size() < 4) {
                interactionMgr.enteredPin += static_cast<char>('0' + (key - GLFW_KEY_0));
            }
            return;
        }
        // Digits 0-9 (numpad)
        if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9) {
            if (interactionMgr.enteredPin.size() < 4) {
                interactionMgr.enteredPin += static_cast<char>('0' + (key - GLFW_KEY_KP_0));
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

    // Toggle Flashlight via F key
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        spotLight.enabled = !spotLight.enabled;
        interactionMgr.setMessage(spotLight.enabled ? "Da BAT den pin (Flashlight)." : "Da TAT den pin.", 2.0f);
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
            interactionMgr.setMessage("Da luu cau hinh phong vao " + sceneFilePath, 3.0f);
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
            interactionMgr.setMessage("Da nap cau hinh phong tu " + sceneFilePath, 3.0f);
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

        postProcessor.bypassTonemap = (guiLayer.debugViewMode > 0);
        postProcessor.beginRender();

        lightingShader.use();
        glm::mat4 projection = camera.getProjectionMatrix((float)windowWidth / (float)windowHeight);
        glm::mat4 view = camera.getViewMatrix();
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

        lightingShader.setFloat("iblIntensity", guiLayer.iblIntensity);
        lightingShader.setFloat("ambientAO", guiLayer.ambientAO);
        lightingShader.setInt("debugViewMode", guiLayer.debugViewMode);

        std::string selectedId = guiLayer.selectedObject ? guiLayer.selectedObject->id : "";
        if (sceneRoot) {
            sceneRoot->draw(lightingShader, hoveredObjectId, selectedId);
        }
        postProcessor.endRender();

        postProcessor.render(t, currentFearFactor);

        // Render UI HUD on top
        std::string currentRoomName = (currentRoomIndex >= 0 && currentRoomIndex < static_cast<int>(rooms.size())) 
                                      ? rooms[currentRoomIndex]->name : "Horror Room";
        guiLayer.beginFrame();
        guiLayer.render(appMode, sceneRoot, pointLight, spotLight, ambientLight,
                        interactionMgr, camera, hoveredObjectId, windowWidth, windowHeight,
                        currentRoomIndex, currentRoomName, currentFearFactor, postProcessor,
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
    std::cout << "  [SYSTEMATIC DEBUGGING: DRAWER & CABINET TRACE]\n";
    std::cout << "========================================================\n\n";

    for (int r = 0; r < 3; ++r) {
        switchRoom(r);
        std::cout << ">>> TESTING ROOM " << r << ": " << rooms[r]->name << " <<<\n";

        // 1. Trace Drawer
        std::cout << "\n--- STEP 1: TRACING DRAWER (id: desk_drawer) ---\n";
        auto drawer = sceneRoot->findById("desk_drawer");
        auto drawerFront = sceneRoot->findById("drawer_front");
        auto drawerBottom = sceneRoot->findById("drawer_bottom");
        auto clueNote = sceneRoot->findById("clue_note");

        if (!drawer) {
            std::cout << "[FAIL] desk_drawer NOT FOUND in scene hierarchy!\n";
        } else {
            std::cout << "[1.1 Object Info] desk_drawer found. interactType=" << static_cast<int>(drawer->interactType)
                      << " isInteractable=" << drawer->isInteractable() << "\n";
            AABB initialBox = drawer->getWorldAABB();
            std::cout << "[1.2 Collider AABB] min=(" << initialBox.min.x << ", " << initialBox.min.y << ", " << initialBox.min.z
                      << ") max=(" << initialBox.max.x << ", " << initialBox.max.y << ", " << initialBox.max.z << ")\n";

            glm::vec3 drawerCenter = (initialBox.min + initialBox.max) * 0.5f;
            glm::vec3 testOrigin = drawerCenter + glm::vec3(0.0f, 0.2f, 1.8f);
            glm::vec3 testDir = glm::normalize(drawerCenter - testOrigin);
            Ray testRay = { testOrigin, testDir };

            std::vector<std::shared_ptr<SceneObject>> objects;
            sceneRoot->collectAll(objects);
            RaycastHit hit = Raycaster::castRay(testRay, objects, 4.5f, true);

            std::cout << "[1.3 Raycast Test] Ray origin=(" << testOrigin.x << ", " << testOrigin.y << ", " << testOrigin.z
                      << ") dir=(" << testDir.x << ", " << testDir.y << ", " << testDir.z << ")\n";
            if (hit.hasHit && hit.object) {
                std::cout << "[1.3 Raycast Result] HIT object=" << hit.object->id << " (" << hit.object->name << ") dist=" << hit.distance << "\n";
                std::string prompt = interactionMgr.getPromptText(hit.object);
                std::cout << "[1.4 Prompt Text] \"" << prompt << "\"\n";
            } else {
                std::cout << "[1.3 Raycast Result] MISS! No interactable object hit by raycast!\n";
            }

            glm::mat4 drawerMat0 = drawer->getWorldMatrix();
            glm::vec3 drawerPos0 = glm::vec3(drawerMat0[3]);
            glm::mat4 frontMat0 = drawerFront ? drawerFront->getWorldMatrix() : glm::mat4(1.0f);
            glm::vec3 frontPos0 = glm::vec3(frontMat0[3]);
            glm::mat4 bottomMat0 = drawerBottom ? drawerBottom->getWorldMatrix() : glm::mat4(1.0f);
            glm::vec3 bottomPos0 = glm::vec3(bottomMat0[3]);
            glm::mat4 noteMat0 = clueNote ? clueNote->getWorldMatrix() : glm::mat4(1.0f);
            glm::vec3 notePos0 = glm::vec3(noteMat0[3]);

            std::cout << "[1.5 Initial Transforms at t=0]\n";
            std::cout << "    drawer world pos:       (" << drawerPos0.x << ", " << drawerPos0.y << ", " << drawerPos0.z << ")\n";
            std::cout << "    drawer_front world pos: (" << frontPos0.x << ", " << frontPos0.y << ", " << frontPos0.z << ")\n";
            std::cout << "    drawer_bottom world pos:(" << bottomPos0.x << ", " << bottomPos0.y << ", " << bottomPos0.z << ")\n";
            std::cout << "    clue_note world pos:    (" << notePos0.x << ", " << notePos0.y << ", " << notePos0.z << ")\n";

            std::cout << "[1.6 Input Key E -> interactionMgr.interact]\n";
            interactionMgr.interact(drawer, pointLight);
            std::cout << "    drawer->isOpen = " << (drawer->isOpen ? "true" : "false")
                      << " statusMessage = \"" << interactionMgr.statusMessage << "\"\n";

            for (int f = 0; f < 30; ++f) {
                rooms[r]->update(0.016f, f * 0.016f, camera.position);
            }

            glm::mat4 drawerMat1 = drawer->getWorldMatrix();
            glm::vec3 drawerPos1 = glm::vec3(drawerMat1[3]);
            glm::mat4 frontMat1 = drawerFront ? drawerFront->getWorldMatrix() : glm::mat4(1.0f);
            glm::vec3 frontPos1 = glm::vec3(frontMat1[3]);
            glm::mat4 bottomMat1 = drawerBottom ? drawerBottom->getWorldMatrix() : glm::mat4(1.0f);
            glm::vec3 bottomPos1 = glm::vec3(bottomMat1[3]);
            glm::mat4 noteMat1 = clueNote ? clueNote->getWorldMatrix() : glm::mat4(1.0f);
            glm::vec3 notePos1 = glm::vec3(noteMat1[3]);

            std::cout << "[1.7 Updated Transforms after 30 frames of update]\n";
            std::cout << "    drawer animProgress:    " << drawer->animProgress << "\n";
            std::cout << "    drawer world pos:       (" << drawerPos1.x << ", " << drawerPos1.y << ", " << drawerPos1.z << ")"
                      << " Delta=" << glm::distance(drawerPos0, drawerPos1) << "\n";
            std::cout << "    drawer_front world pos: (" << frontPos1.x << ", " << frontPos1.y << ", " << frontPos1.z << ")"
                      << " Delta=" << glm::distance(frontPos0, frontPos1) << "\n";
            std::cout << "    drawer_bottom world pos:(" << bottomPos1.x << ", " << bottomPos1.y << ", " << bottomPos1.z << ")"
                      << " Delta=" << glm::distance(bottomPos0, bottomPos1) << "\n";
            std::cout << "    clue_note world pos:    (" << notePos1.x << ", " << notePos1.y << ", " << notePos1.z << ")"
                      << " Delta=" << glm::distance(notePos0, notePos1) << "\n";

            if (glm::distance(frontPos0, frontPos1) < 0.001f) {
                std::cout << ">>> [BUG DETECTED IN DRAWER] drawer_front Delta is 0.0! Visible meshes of drawer DID NOT MOVE!\n";
            } else {
                std::cout << ">>> [SUCCESS] Drawer front moved by " << glm::distance(frontPos0, frontPos1) << " units.\n";
            }
        }

        // 2. Trace Safe / Locked Cabinet
        std::cout << "\n--- STEP 2: TRACING LOCKED CABINET / SAFE (id: safe_hinge / safe_door_panel) ---\n";
        auto safeHinge = sceneRoot->findById("safe_hinge");
        auto safeDoor = sceneRoot->findById("safe_door_panel");

        if (!safeHinge || !safeDoor) {
            std::cout << "[FAIL] safe_hinge or safe_door_panel NOT FOUND!\n";
        } else {
            std::cout << "[2.1 Object Info] safeDoor found. interactType=" << static_cast<int>(safeDoor->interactType)
                      << " safeHinge->isLocked=" << safeHinge->isLocked << " safeUnlocked=" << interactionMgr.safeUnlocked << "\n";
            AABB doorBox = safeDoor->getWorldAABB();
            std::cout << "[2.2 Safe Door AABB] min=(" << doorBox.min.x << ", " << doorBox.min.y << ", " << doorBox.min.z
                      << ") max=(" << doorBox.max.x << ", " << doorBox.max.y << ", " << doorBox.max.z << ")\n";

            glm::vec3 doorCenter = (doorBox.min + doorBox.max) * 0.5f;
            glm::vec3 testOrigin = doorCenter + glm::vec3(0.0f, 0.0f, 1.5f);
            glm::vec3 testDir = glm::normalize(doorCenter - testOrigin);
            Ray testRay = { testOrigin, testDir };

            std::vector<std::shared_ptr<SceneObject>> objects;
            sceneRoot->collectAll(objects);
            RaycastHit hit = Raycaster::castRay(testRay, objects, 4.5f, true);

            if (hit.hasHit && hit.object) {
                std::cout << "[2.3 Raycast Result] HIT object=" << hit.object->id << " (" << hit.object->name << ") dist=" << hit.distance << "\n";
                std::cout << "[2.4 Prompt Text] \"" << interactionMgr.getPromptText(hit.object) << "\"\n";
            }

            glm::mat4 hingeMat0 = safeHinge->getWorldMatrix();
            glm::mat4 doorMat0 = safeDoor->getWorldMatrix();
            glm::vec3 doorPos0 = glm::vec3(doorMat0[3]);

            std::cout << "[2.5 Initial Transforms at t=0]\n";
            std::cout << "    safe_hinge rotation:    (" << safeHinge->rotation.x << ", " << safeHinge->rotation.y << ", " << safeHinge->rotation.z << ")\n";
            std::cout << "    safe_door_panel pos:    (" << doorPos0.x << ", " << doorPos0.y << ", " << doorPos0.z << ")\n";

            std::cout << "[2.6 Submitting PIN: " << rooms[r]->cluePin << "]\n";
            interactionMgr.checkPin(rooms[r]->cluePin);
            std::cout << "    safeUnlocked=" << interactionMgr.safeUnlocked << " safeHinge->isOpen=" << safeHinge->isOpen << "\n";

            for (int f = 0; f < 30; ++f) {
                rooms[r]->update(0.016f, f * 0.016f, camera.position);
            }

            glm::mat4 hingeMat1 = safeHinge->getWorldMatrix();
            glm::mat4 doorMat1 = safeDoor->getWorldMatrix();
            glm::vec3 doorPos1 = glm::vec3(doorMat1[3]);

            std::cout << "[2.7 Updated Transforms after 30 frames of update]\n";
            std::cout << "    safe_hinge animProgress: " << safeHinge->animProgress << "\n";
            std::cout << "    safe_hinge rotation:     (" << safeHinge->rotation.x << ", " << safeHinge->rotation.y << ", " << safeHinge->rotation.z << ")\n";
            std::cout << "    safe_door_panel pos:     (" << doorPos1.x << ", " << doorPos1.y << ", " << doorPos1.z << ")"
                      << " Delta=" << glm::distance(doorPos0, doorPos1) << "\n";

            if (glm::distance(doorPos0, doorPos1) < 0.001f) {
                std::cout << ">>> [BUG DETECTED IN SAFE] safe_door_panel Delta is 0.0! Door slab DID NOT ROTATE OR MOVE!\n";
            } else {
                std::cout << ">>> [SUCCESS] Safe door moved by " << glm::distance(doorPos0, doorPos1) << " units.\n";
            }
        }
        std::cout << "\n--------------------------------------------------------\n";
    }
}
