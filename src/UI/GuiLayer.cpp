#include "ui/GuiLayer.hpp"
#include "audio/AudioEngine.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void GuiLayer::init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Dark sleek theme
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.PopupRounding = 6.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void GuiLayer::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GuiLayer::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GuiLayer::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GuiLayer::render(AppMode appMode,
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
                      const std::string& objectiveHint,
                      PostProcessor& postProcessor,
                      ShadowPipeline& shadowPipeline,
                      ParticleSystem& particleSystem,
                      SsaoPipeline& ssaoPipeline,
                      AudioEngine& audioEngine,
                      GpuProfiler& gpuProfiler,
                      GraphicsSettings& graphicsSettings,
                      std::function<void(int)> onSwitchRoom,
                      std::function<void()> onSaveScene,
                      std::function<void()> onLoadScene) {

    // HUD is always rendered (Crosshair in PlayMode, notifications, horror meter)
    renderGameHUD(appMode, interactionMgr,
                  interactionMgr.getPromptText(root ? root->findById(hoveredObjId) : nullptr),
                  currentRoomName, fearFactor, objectiveHint,
                  windowWidth, windowHeight);

    // Keypad dialog if open
    if (interactionMgr.showKeypadModal) {
        renderKeypadModal(interactionMgr);
    }

    // 3D Inspection dialog if active
    if (interactionMgr.isInspectMode) {
        renderInspectModal(interactionMgr, windowWidth, windowHeight);
    }

    // Victory celebration if escaped
    if (interactionMgr.gameWon) {
        renderVictoryModal(interactionMgr, onSwitchRoom);
    }

    // Edit Mode Windows
    if (appMode == AppMode::EditMode) {
        // Main Dock/Menu Bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save Scene JSON", "Ctrl+S")) {
                    if (onSaveScene) onSaveScene();
                }
                if (ImGui::MenuItem("Load Scene JSON", "Ctrl+O")) {
                    if (onLoadScene) onLoadScene();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Room State")) {
                    interactionMgr.resetGameState();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Rooms (Select)")) {
                if (ImGui::MenuItem("[1] Occult Ritual Chamber", "1", currentRoomIndex == 0)) {
                    if (onSwitchRoom) onSwitchRoom(0);
                }
                if (ImGui::MenuItem("[2] Abandoned Psychiatric Ward", "2", currentRoomIndex == 1)) {
                    if (onSwitchRoom) onSwitchRoom(1);
                }
                if (ImGui::MenuItem("[3] Haunted Mirror Sanctuary", "3", currentRoomIndex == 2)) {
                    if (onSwitchRoom) onSwitchRoom(2);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Lighting & Horror FX")) {
                ImGui::MenuItem("Ceiling Light ON/OFF", nullptr, &pointLight.enabled);
                ImGui::MenuItem("Point Light Flicker ON/OFF", nullptr, &pointLight.flickerEnabled);
                ImGui::MenuItem("Flashlight (F) ON/OFF", nullptr, &spotLight.enabled);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Audio (3D Spatial)")) {
                ImGui::MenuItem("Audio Engine Active", nullptr, &audioEngine.enabled);
                ImGui::MenuItem("Mute", nullptr, &audioEngine.muted);
                ImGui::SliderFloat("Master Volume", &audioEngine.masterVolume, 0.0f, 1.0f);
                ImGui::SliderFloat("SFX Volume", &audioEngine.sfxVolume, 0.0f, 1.0f);
                ImGui::SliderFloat("Ambient Volume", &audioEngine.ambientVolume, 0.0f, 1.0f);
                ImGui::Separator();
                if (ImGui::Button("Test Footstep")) {
                    audioEngine.playFootstep(camera.position);
                }
                ImGui::SameLine();
                if (ImGui::Button("Test Beep")) {
                    audioEngine.playKeypadBeep(true);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Camera")) {
                if (ImGui::MenuItem("Reset Camera View")) {
                    camera.reset();
                }
                ImGui::EndMenu();
            }

            ImGui::SameLine(ImGui::GetWindowWidth() - 320.0f);
            ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.4f, 1.0f), "[EDIT MODE] TAB: Play | [1,2,3]: Room");

            ImGui::EndMainMenuBar();
        }

        renderHierarchy(root);
        renderInspector(interactionMgr);
        renderLighting(pointLight, spotLight, ambientLight, postProcessor, shadowPipeline, particleSystem, ssaoPipeline, fearFactor);
        renderPerformanceAndQuality(gpuProfiler, graphicsSettings, shadowPipeline, ssaoPipeline, postProcessor, particleSystem);
    }
}

void GuiLayer::renderHierarchy(std::shared_ptr<SceneObject>& root) {
    ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 450), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Scene Hierarchy", nullptr)) {
        ImGui::TextDisabled("Select an object to inspect & edit");
        ImGui::Separator();

        if (root) {
            renderHierarchyNode(root);
        }
    }
    ImGui::End();
}

void GuiLayer::renderHierarchyNode(std::shared_ptr<SceneObject>& obj) {
    if (!obj) return;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (obj->children.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (selectedObject && selectedObject->id == obj->id) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    std::string label = obj->name + " (" + obj->id + ")";
    bool nodeOpen = ImGui::TreeNodeEx(obj->id.c_str(), flags, "%s", label.c_str());

    if (ImGui::IsItemClicked()) {
        selectedObject = obj;
    }

    if (nodeOpen) {
        for (auto& child : obj->children) {
            renderHierarchyNode(child);
        }
        ImGui::TreePop();
    }
}

void GuiLayer::renderInspector(InteractionManager& interactionMgr) {
    ImGui::SetNextWindowPos(ImVec2(960, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(310, 520), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Object Inspector", nullptr)) {
        if (!selectedObject) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No SceneObject selected.\nClick an object in Hierarchy or\nin the 3D viewport to inspect.");
            ImGui::End();
            return;
        }

        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Object: %s", selectedObject->name.c_str());
        ImGui::TextDisabled("ID: %s", selectedObject->id.c_str());
        ImGui::Checkbox("Visible", &selectedObject->isVisible);
        ImGui::Separator();

        // Transforms
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Position", &selectedObject->position.x, 0.05f);
            ImGui::DragFloat3("Rotation", &selectedObject->rotation.x, 1.0f, -360.0f, 360.0f);
            ImGui::DragFloat3("Scale", &selectedObject->scale.x, 0.05f, 0.01f, 50.0f);
        }

        // Material (PBR & Legacy)
        if (ImGui::CollapsingHeader("PBR Material", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::ColorEdit3("Albedo Color", &selectedObject->material.albedoColor.r);
            ImGui::SliderFloat("Metallic", &selectedObject->material.metallic, 0.0f, 1.0f);
            ImGui::SliderFloat("Roughness", &selectedObject->material.roughness, 0.04f, 1.0f);
            ImGui::SliderFloat("Ambient Occlusion", &selectedObject->material.ao, 0.0f, 1.0f);
            ImGui::ColorEdit3("Emissive Color", &selectedObject->material.emissiveColor.r);
            ImGui::SliderFloat("Emissive Intensity", &selectedObject->material.emissiveIntensity, 0.0f, 10.0f);

            ImGui::Separator();
            ImGui::TextDisabled("PBR Texture Maps Active:");
            ImGui::Checkbox("Use Albedo Map", &selectedObject->material.useAlbedoMap);
            ImGui::Checkbox("Use Normal Map", &selectedObject->material.useNormalMap);
            ImGui::Checkbox("Use Metallic Map", &selectedObject->material.useMetallicMap);
            ImGui::Checkbox("Use Roughness Map", &selectedObject->material.useRoughnessMap);
            ImGui::Checkbox("Use AO Map", &selectedObject->material.useAoMap);
            ImGui::Checkbox("Use Emissive Map", &selectedObject->material.useEmissiveMap);

            if (ImGui::TreeNode("Legacy Blinn-Phong Properties")) {
                ImGui::ColorEdit3("Diffuse", &selectedObject->material.diffuseColor.r);
                ImGui::ColorEdit3("Specular", &selectedObject->material.specularColor.r);
                ImGui::SliderFloat("Shininess", &selectedObject->material.shininess, 1.0f, 256.0f);
                ImGui::Checkbox("Use Legacy Texture", &selectedObject->material.useTexture);
                ImGui::TreePop();
            }
        }

        // Interaction Properties
        if (ImGui::CollapsingHeader("Interaction & Animation", ImGuiTreeNodeFlags_DefaultOpen)) {
            const char* interactNames[] = { "None", "LightSwitch", "DeskDrawer", "SecretNote", "SafeKeypad", "SafeDoor", "GoldenKey", "ExitDoor" };
            int currentType = (int)selectedObject->interactType;
            if (ImGui::Combo("Interact Type", &currentType, interactNames, IM_ARRAYSIZE(interactNames))) {
                selectedObject->interactType = (InteractionType)currentType;
            }

            if (selectedObject->interactType != InteractionType::None) {
                ImGui::Checkbox("Is Open", &selectedObject->isOpen);
                ImGui::Checkbox("Is Locked", &selectedObject->isLocked);
                ImGui::DragFloat3("Open Pos Offset", &selectedObject->openPositionOffset.x, 0.05f);
                ImGui::DragFloat3("Open Rot Offset", &selectedObject->openRotationOffset.x, 1.0f);
                ImGui::SliderFloat("Anim Progress", &selectedObject->animProgress, 0.0f, 1.0f);
            }
        }
    }
    ImGui::End();
}

void GuiLayer::renderLighting(PointLight& pointLight, SpotLight& spotLight, AmbientLight& ambientLight,
                              PostProcessor& postProcessor, ShadowPipeline& shadowPipeline,
                              ParticleSystem& particleSystem, SsaoPipeline& ssaoPipeline, float fearFactor) {
    ImGui::SetNextWindowPos(ImVec2(10, 470), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(380, 420), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Lighting & Atmosphere FX", nullptr)) {
        // Point light (Ceiling / Altar lamp)
        if (ImGui::TreeNode("Horror Point Light (Flickering)")) {
            ImGui::Checkbox("Enabled", &pointLight.enabled);
            ImGui::Checkbox("Flicker Effect (Sin + Noise)", &pointLight.flickerEnabled);
            ImGui::SliderFloat("Flicker Speed", &pointLight.flickerSpeed, 1.0f, 20.0f);
            ImGui::SliderFloat("Flicker Noise Depth", &pointLight.flickerNoiseDepth, 0.0f, 1.0f);
            ImGui::DragFloat3("Position", &pointLight.position.x, 0.1f);
            ImGui::ColorEdit3("Color", &pointLight.color.r);
            ImGui::SliderFloat("Base Intensity", &pointLight.baseIntensity, 0.0f, 25.0f);
            ImGui::TextDisabled("Live Intensity: %.2f", pointLight.intensity);
            ImGui::TreePop();
        }

        // Flashlight (Spotlight)
        if (ImGui::TreeNode("Camera Flashlight (F)")) {
            ImGui::Checkbox("Flashlight Enabled##spot", &spotLight.enabled);
            ImGui::ColorEdit3("Color##spot", &spotLight.color.r);
            ImGui::SliderFloat("Intensity##spot", &spotLight.intensity, 0.0f, 40.0f);
            ImGui::TreePop();
        }

        // Dynamic Shadows (Phase 2)
        if (ImGui::TreeNodeEx("Dynamic Shadows (Phase 2)", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Spot Shadow (Rotated Poisson PCF)");
            ImGui::SliderFloat("Spot Bias Min", &shadowPipeline.spotBiasMin, 0.0001f, 0.01f, "%.4f");
            ImGui::SliderFloat("Spot Bias Max", &shadowPipeline.spotBiasMax, 0.001f, 0.02f, "%.4f");
            ImGui::SliderFloat("Spot Normal Bias", &shadowPipeline.spotNormalBias, 0.0f, 0.1f, "%.3f");
            ImGui::SliderFloat("PCF Filter Radius", &shadowPipeline.spotPcfRadius, 0.5f, 4.0f, "%.1f");
            ImGui::SliderInt("PCF Taps", &shadowPipeline.spotPcfTaps, 4, 16);

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Ceiling Omnidirectional Shadow Cubemap");
            ImGui::Checkbox("Point Shadow Enabled", &shadowPipeline.pointShadowEnabled);
            ImGui::SliderFloat("Point Bias Min", &shadowPipeline.pointBiasMin, 0.001f, 0.02f, "%.4f");
            ImGui::SliderFloat("Point Bias Max", &shadowPipeline.pointBiasMax, 0.005f, 0.05f, "%.4f");
            ImGui::SliderFloat("Point Normal Bias", &shadowPipeline.pointNormalBias, 0.0f, 0.1f, "%.3f");
            if (ImGui::Button("Force Rebake Point Shadow Cubemap")) {
                shadowPipeline.markPointShadowDirty();
            }
            ImGui::TreePop();
        }

        // SSAO Pipeline (Phase 2)
        if (ImGui::TreeNodeEx("Screen Space Ambient Occlusion (Phase 2)", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Enable SSAO", &ssaoPipeline.enabled);
            ImGui::SliderFloat("SSAO Radius", &ssaoPipeline.radius, 0.1f, 1.5f, "%.2f");
            ImGui::SliderFloat("SSAO Bias", &ssaoPipeline.bias, 0.005f, 0.08f, "%.3f");
            ImGui::SliderFloat("SSAO Power", &ssaoPipeline.power, 0.5f, 4.0f, "%.1f");
            ImGui::SliderInt("SSAO Kernel Samples", &ssaoPipeline.kernelSize, 8, 32);
            ImGui::TreePop();
        }

        // HDR Bloom & Post-Processing (Phase 2)
        if (ImGui::TreeNodeEx("HDR Bloom & Film Grain (Phase 2)", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Enable Bloom", &postProcessor.bloomEnabled);
            ImGui::SliderFloat("Bloom Intensity", &postProcessor.bloomIntensity, 0.0f, 3.0f, "%.2f");
            ImGui::SliderFloat("Bloom Threshold", &postProcessor.bloomThreshold, 0.5f, 3.0f, "%.2f");
            ImGui::SliderFloat("Film Grain Strength", &postProcessor.filmGrainStrength, 0.0f, 0.10f, "%.3f");
            ImGui::SliderFloat("Vignette Strength", &postProcessor.vignetteStrength, 0.0f, 1.0f);
            ImGui::SliderFloat("Chromatic Aberration", &postProcessor.chromaticAberrationStrength, 0.0f, 0.02f, "%.4f");
            ImGui::ProgressBar(fearFactor, ImVec2(0.0f, 0.0f), "Fear Level");
            ImGui::TreePop();
        }

        // Atmospheric Dust Particles (Phase 2)
        if (ImGui::TreeNode("Atmospheric Dust Motes (Phase 2)")) {
            ImGui::Checkbox("Dust Particles Enabled", &particleSystem.enabled);
            ImGui::SliderInt("Particle Count", &particleSystem.activeCount, 50, 450);
            ImGui::SliderFloat("Particle Size", &particleSystem.particleBaseSize, 0.01f, 0.10f, "%.3f");
            ImGui::TreePop();
        }

        // Linear HDR & ACES Tone Mapping
        if (ImGui::TreeNodeEx("Linear HDR & Color Pipeline", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Exposure", &postProcessor.exposure, 0.1f, 5.0f);
            ImGui::Checkbox("ACES Filmic Tonemap", &postProcessor.toneMappingEnabled);
            ImGui::Checkbox("sRGB Gamma Correction", &postProcessor.gammaCorrectionEnabled);
            ImGui::TreePop();
        }

        // PBR Diagnostic Debug Views & IBL Split-Sum Controls
        if (ImGui::TreeNodeEx("PBR Shading & Debug Views", ImGuiTreeNodeFlags_DefaultOpen)) {
            const char* debugModes[] = {
                "0: Final (PBR + Lighting + IBL + Emissive)",
                "1: Albedo (Base Color Linear)",
                "2: Normal (World Normal)",
                "3: Roughness (Linear)",
                "4: Metallic (Linear)",
                "5: Ambient Occlusion (AO)",
                "6: IBL Contribution Only",
                "7: Direct Shadows Only",
                "8: SSAO Factor Only"
            };
            ImGui::Combo("Debug View", &debugViewMode, debugModes, IM_ARRAYSIZE(debugModes));
            ImGui::SliderFloat("IBL Split-Sum Intensity", &iblIntensity, 0.0f, 5.0f, "%.2f");
            ImGui::SliderFloat("Ambient AO Modulation", &ambientAO, 0.0f, 2.0f, "%.2f");
            ImGui::TreePop();
        }

        // Ambient Light
        if (ImGui::TreeNode("Ambient Lighting")) {
            ImGui::ColorEdit3("Ambient Color", &ambientLight.color.r);
            ImGui::SliderFloat("Ambient Intensity", &ambientLight.intensity, 0.0f, 1.0f);
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void GuiLayer::renderPerformanceAndQuality(GpuProfiler& gpuProfiler, GraphicsSettings& graphicsSettings,
                                          ShadowPipeline& shadowPipeline, SsaoPipeline& ssaoPipeline,
                                          PostProcessor& postProcessor, ParticleSystem& particleSystem) {
    ImGui::SetNextWindowPos(ImVec2(10, 490), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(340, 220), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Profiler & Quality Presets (Phase 6)", nullptr)) {
        ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.4f, 1.0f), "Quality Preset: %s", graphicsSettings.presetName.c_str());
        if (ImGui::Button("Low")) {
            graphicsSettings.applyPreset(QualityLevel::Low);
            shadowPipeline.spotPcfTaps = graphicsSettings.pcfSamples;
            ssaoPipeline.enabled = graphicsSettings.ssaoEnabled;
            ssaoPipeline.kernelSize = graphicsSettings.ssaoSamples;
            particleSystem.activeCount = graphicsSettings.particleCount;
        }
        ImGui::SameLine();
        if (ImGui::Button("Medium")) {
            graphicsSettings.applyPreset(QualityLevel::Medium);
            shadowPipeline.spotPcfTaps = graphicsSettings.pcfSamples;
            ssaoPipeline.enabled = graphicsSettings.ssaoEnabled;
            ssaoPipeline.kernelSize = graphicsSettings.ssaoSamples;
            particleSystem.activeCount = graphicsSettings.particleCount;
        }
        ImGui::SameLine();
        if (ImGui::Button("High")) {
            graphicsSettings.applyPreset(QualityLevel::High);
            shadowPipeline.spotPcfTaps = graphicsSettings.pcfSamples;
            ssaoPipeline.enabled = graphicsSettings.ssaoEnabled;
            ssaoPipeline.kernelSize = graphicsSettings.ssaoSamples;
            particleSystem.activeCount = graphicsSettings.particleCount;
        }
        ImGui::SameLine();
        if (ImGui::Button("Ultra")) {
            graphicsSettings.applyPreset(QualityLevel::Ultra);
            shadowPipeline.spotPcfTaps = graphicsSettings.pcfSamples;
            ssaoPipeline.enabled = graphicsSettings.ssaoEnabled;
            ssaoPipeline.kernelSize = graphicsSettings.ssaoSamples;
            particleSystem.activeCount = graphicsSettings.particleCount;
        }

        ImGui::Separator();
        float totalGpu = gpuProfiler.getTotalGpuTimeMs();
        float fps = ImGui::GetIO().Framerate;
        ImGui::Text("Framerate: %.1f FPS (%.2f ms/frame)", fps, 1000.0f / (fps > 0.0f ? fps : 60.0f));
        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), "Total GPU Time: %.3f ms", totalGpu);

        if (ImGui::TreeNodeEx("GPU Timeline Breakdown (GL_TIME_ELAPSED)", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (size_t p = 0; p < static_cast<size_t>(GpuPass::Count); ++p) {
                GpuPass pass = static_cast<GpuPass>(p);
                float ms = gpuProfiler.getPassTimeMs(pass);
                float pct = (totalGpu > 0.0001f) ? (ms / totalGpu * 100.0f) : 0.0f;
                ImGui::Text("%-28s: %6.3f ms (%4.1f%%)", GpuProfiler::getPassName(pass), ms, pct);
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void GuiLayer::renderGameHUD(AppMode appMode,
                             const InteractionManager& interactionMgr,
                             const std::string& promptText,
                             const std::string& roomName,
                             float fearFactor,
                             const std::string& objectiveHint,
                             int windowWidth,
                             int windowHeight) {
    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    float centerX = windowWidth * 0.5f;
    float centerY = windowHeight * 0.5f;

    // 1. Crosshair in PlayMode
    if (appMode == AppMode::PlayMode) {
        ImU32 crossColor = promptText.empty() ? IM_COL32(255, 255, 255, 180) : IM_COL32(255, 220, 40, 255);
        float lineLen = promptText.empty() ? 8.0f : 14.0f;

        // Center dot
        drawList->AddCircleFilled(ImVec2(centerX, centerY), promptText.empty() ? 2.0f : 3.0f, crossColor);

        // Crosshair lines
        drawList->AddLine(ImVec2(centerX - lineLen, centerY), ImVec2(centerX - 3.0f, centerY), crossColor, 1.5f);
        drawList->AddLine(ImVec2(centerX + 3.0f, centerY), ImVec2(centerX + lineLen, centerY), crossColor, 1.5f);
        drawList->AddLine(ImVec2(centerX, centerY - lineLen), ImVec2(centerX, centerY - 3.0f), crossColor, 1.5f);
        drawList->AddLine(ImVec2(centerX, centerY + 3.0f), ImVec2(centerX, centerY + lineLen), crossColor, 1.5f);

        // Interaction Prompt Text Banner
        if (!promptText.empty()) {
            ImVec2 textSize = ImGui::CalcTextSize(promptText.c_str());
            ImVec2 textPos(centerX - textSize.x * 0.5f, centerY + 24.0f);

            // Glowing background pill
            drawList->AddRectFilled(ImVec2(textPos.x - 12.0f, textPos.y - 6.0f),
                                   ImVec2(textPos.x + textSize.x + 12.0f, textPos.y + textSize.y + 6.0f),
                                   IM_COL32(15, 15, 20, 220), 6.0f);
            drawList->AddRect(ImVec2(textPos.x - 12.0f, textPos.y - 6.0f),
                              ImVec2(textPos.x + textSize.x + 12.0f, textPos.y + textSize.y + 6.0f),
                              IM_COL32(255, 220, 50, 220), 6.0f);

            drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), promptText.c_str());
        }
    }

    // 2. Status message banner
    if (!interactionMgr.statusMessage.empty()) {
        std::string msg = interactionMgr.statusMessage;
        ImVec2 msgSize = ImGui::CalcTextSize(msg.c_str());
        float bannerY = windowHeight - 70.0f;
        ImVec2 bannerPos(centerX - msgSize.x * 0.5f, bannerY);

        drawList->AddRectFilled(ImVec2(bannerPos.x - 16.0f, bannerPos.y - 8.0f),
                               ImVec2(bannerPos.x + msgSize.x + 16.0f, bannerPos.y + msgSize.y + 8.0f),
                               IM_COL32(10, 10, 15, 230), 8.0f);
        drawList->AddText(bannerPos, IM_COL32(255, 230, 100, 255), msg.c_str());
    }

    // 3. Top-left Status Badges
    std::string modeStatus = (appMode == AppMode::PlayMode) ? "PLAY [TAB: Edit]" : "EDIT [TAB: Play]";
    std::string hudTop = "[" + roomName + "] | " + modeStatus + " | Relics: " + std::to_string(interactionMgr.keysCollected) + "/3";
    ImVec2 topSize = ImGui::CalcTextSize(hudTop.c_str());
    drawList->AddRectFilled(ImVec2(10.0f, 10.0f), ImVec2(10.0f + topSize.x + 16.0f, 32.0f),
                           IM_COL32(15, 15, 20, 200), 4.0f);
    drawList->AddText(ImVec2(18.0f, 13.0f), IM_COL32(210, 235, 255, 240), hudTop.c_str());

    // Flashlight status badge
    std::string uvStatus = interactionMgr.isUvLightActive 
        ? "Flashlight: [UV BLACKLIGHT ACTIVE] (Reveals hidden fluorescent clues)"
        : "Flashlight: [NORMAL] (Right-Click / [V] for UV Blacklight)";
    ImU32 flashColor = interactionMgr.isUvLightActive ? IM_COL32(183, 148, 244, 255) : IM_COL32(210, 235, 255, 230);
    ImVec2 flashSize = ImGui::CalcTextSize(uvStatus.c_str());
    float flashX = 10.0f + topSize.x + 24.0f;
    drawList->AddRectFilled(ImVec2(flashX, 10.0f), ImVec2(flashX + flashSize.x + 16.0f, 32.0f),
                           IM_COL32(15, 15, 20, 200), 4.0f);
    drawList->AddText(ImVec2(flashX + 8.0f, 13.0f), flashColor, uvStatus.c_str());

    // Dynamic Objective Hint Banner
    if (!objectiveHint.empty()) {
        std::string objGuide = ">> [OBJECTIVE] " + objectiveHint;
        ImVec2 guideSize = ImGui::CalcTextSize(objGuide.c_str());
        drawList->AddRectFilled(ImVec2(10.0f, 38.0f), ImVec2(10.0f + guideSize.x + 16.0f, 60.0f),
                               IM_COL32(15, 30, 42, 230), 4.0f);
        drawList->AddRect(ImVec2(10.0f, 38.0f), ImVec2(10.0f + guideSize.x + 16.0f, 60.0f),
                          IM_COL32(50, 210, 255, 240), 4.0f);
        drawList->AddText(ImVec2(18.0f, 41.0f), IM_COL32(120, 235, 255, 255), objGuide.c_str());
    }

    // 4. Horror Heartbeat / Fear Indicator (Bottom-left)
    int bpm = static_cast<int>(75.0f + fearFactor * 85.0f);
    std::string bpmText = "HEART RATE: " + std::to_string(bpm) + " BPM";
    if (fearFactor > 0.05f) {
        bpmText += " (FEAR: " + std::to_string(static_cast<int>(fearFactor * 100.0f)) + "%)";
    }
    ImVec2 bpmSize = ImGui::CalcTextSize(bpmText.c_str());
    float bpmY = windowHeight - 40.0f;
    ImU32 heartColor = (fearFactor > 0.4f) ? IM_COL32(255, 60, 60, 240) : IM_COL32(200, 200, 210, 200);

    drawList->AddRectFilled(ImVec2(12.0f, bpmY - 4.0f), ImVec2(24.0f + bpmSize.x, bpmY + bpmSize.y + 4.0f),
                           IM_COL32(15, 15, 20, 180), 4.0f);
    drawList->AddText(ImVec2(18.0f, bpmY), heartColor, bpmText.c_str());
}

void GuiLayer::renderKeypadModal(InteractionManager& interactionMgr) {
    ImGui::SetNextWindowSize(ImVec2(360, 420), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - 180.0f,
                                   ImGui::GetIO().DisplaySize.y * 0.5f - 210.0f), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin("Electronic Security Keypad", &interactionMgr.showKeypadModal, flags)) {
        ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.4f, 1.0f), "ENTER 4-DIGIT SECURITY PIN:");
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "Direct Keyboard: [0-9], [Backspace], [Enter]");
        ImGui::Spacing();

        // Display screen
        std::string display = interactionMgr.enteredPin;
        while (display.size() < 4) display += "_";
        ImGui::SetWindowFontScale(1.8f);
        ImGui::Text("   [ %s ]", display.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // 3x4 Keypad buttons
        const char* keys[4][3] = {
            {"1", "2", "3"},
            {"4", "5", "6"},
            {"7", "8", "9"},
            {"C", "0", "OK"}
        };

        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 3; ++c) {
                if (c > 0) ImGui::SameLine();
                const char* k = keys[r][c];
                if (ImGui::Button(k, ImVec2(92, 42))) {
                    if (std::string(k) == "C") {
                        interactionMgr.enteredPin.clear();
                    } else if (std::string(k) == "OK") {
                        interactionMgr.checkPin(interactionMgr.enteredPin);
                    } else {
                        if (interactionMgr.enteredPin.size() < 4) {
                            interactionMgr.enteredPin += k;
                        }
                    }
                }
            }
        }

        ImGui::Spacing();
        if (ImGui::Button("Cancel / Close [ESC]", ImVec2(290, 32))) {
            interactionMgr.showKeypadModal = false;
        }
    }
    ImGui::End();
}

void GuiLayer::renderInspectModal(InteractionManager& interactionMgr, int windowWidth, int windowHeight) {
    float modalW = 540.0f;
    float modalH = 400.0f;
    ImGui::SetNextWindowSize(ImVec2(modalW, modalH), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(float(windowWidth) * 0.5f - modalW * 0.5f,
                                   float(windowHeight) * 0.5f - modalH * 0.5f), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin("3D INSPECT MODE [ESC / E to close]", nullptr, flags)) {
        ImGui::Spacing();
        if (interactionMgr.currentInspectType == InspectType::SecretNote) {
            ImGui::TextColored(ImVec4(1.0f, 0.88f, 0.45f, 1.0f), "%s", interactionMgr.inspectTitle.c_str());
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.14f, 0.12f, 0.10f, 0.95f));
            if (ImGui::BeginChild("NoteBody", ImVec2(modalW - 30.0f, 240.0f), true)) {
                ImGui::Spacing();
                ImGui::TextWrapped("%s", interactionMgr.inspectBody.c_str());
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "%s", interactionMgr.inspectTitle.c_str());
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 1.0f, 1.0f), "[3D Turntable: Drag mouse to rotate | Yaw: %.1f deg, Pitch: %.1f deg]",
                               interactionMgr.inspectYaw, interactionMgr.inspectPitch);
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.12f, 0.16f, 0.95f));
            if (ImGui::BeginChild("ArtifactBody", ImVec2(modalW - 30.0f, 210.0f), true)) {
                ImGui::Spacing();
                ImGui::TextWrapped("%s", interactionMgr.inspectBody.c_str());
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.85f, 0.75f, 0.35f, 1.0f), "Artifact details: 28cm length | Ancient golden relic glowing with HDR");
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        if (ImGui::Button("Close Inspection [ESC / E]", ImVec2(modalW - 30.0f, 36.0f))) {
            interactionMgr.closeInspect();
        }
    }
    ImGui::End();
}

void GuiLayer::renderVictoryModal(InteractionManager& interactionMgr, std::function<void(int)> onSwitchRoom) {
    ImGui::SetNextWindowSize(ImVec2(500, 260), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - 250.0f,
                                   ImGui::GetIO().DisplaySize.y * 0.5f - 130.0f), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin("MISSION ACCOMPLISHED - ESCAPED THE NIGHTMARE!", nullptr, flags)) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "CONGRATULATIONS! YOU HAVE ESCAPED!");
        ImGui::TextWrapped("You solved all 3 horror escape rooms and collected all 3 Golden Keys & Sigils of Dr. Elias Vance!");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "The demonic mirror seal has shattered. Sanctuary is cleansed.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (interactionMgr.currentRoomId < 2) {
            if (ImGui::Button("Proceed to Next Room", ImVec2(230, 42))) {
                interactionMgr.gameWon = false;
                interactionMgr.requestRoomTransition = true;
            }
            ImGui::SameLine();
        }
        if (ImGui::Button("Restart from Room 1", ImVec2(230, 42))) {
            interactionMgr.resetGameState();
            if (onSwitchRoom) onSwitchRoom(0);
        }
    }
    ImGui::End();
}
