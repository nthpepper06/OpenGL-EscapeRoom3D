#pragma once

#include "core/Common.hpp"
#include "scene/SceneObject.hpp"
#include "renderer/Light.hpp"

class SceneObject;
class PointLight;
class AudioEngine;

enum class InspectType {
    None,
    SecretNote,
    GoldenKey,
    SigilFragment
};

class InteractionManager {
public:
    bool hasGoldenKey = false;
    bool safeUnlocked = false;
    bool exitUnlocked = false;
    bool gameWon = false;
    bool requestRoomTransition = false;

    // Multi-Room Puzzle & Story State
    int currentRoomId = 0;
    int keysCollected = 0;   // 0 to 3
    int sigilsCollected = 0; // 0 to 3
    bool hasSigil[3] = { false, false, false };

    // Room 1: Shadow Totems Puzzle State
    float totemAngles[3] = { 0.0f, 0.0f, 0.0f };
    const float targetTotemAngles[3] = { 90.0f, 180.0f, 270.0f };
    bool altarUnlocked = false;
    std::shared_ptr<SceneObject> altarLidObject = nullptr;

    // Room 2: Circuit Breaker & UV Blacklight State
    int fusesCollected = 0; // 0 to 2
    bool powerRestored = false;
    bool isUvLightActive = false;
    std::shared_ptr<SceneObject> isolationGateObject = nullptr;

    // Room 3: Mirror Anomaly & Symmetry Puzzle State
    float bustAngles[3] = { 0.0f, 0.0f, 0.0f };
    const float targetBustAngles[3] = { 180.0f, 90.0f, 270.0f };
    bool mirrorPuzzleSolved = false;

    std::string correctSafePin = "7492";
    std::string enteredPin = "";
    bool showKeypadModal = false;

    // 3D Inspection Mode
    bool isInspectMode = false;
    InspectType currentInspectType = InspectType::None;
    float inspectYaw = 0.0f;
    float inspectPitch = 0.0f;
    std::string inspectTitle = "";
    std::string inspectBody = "";

    std::string statusMessage = "Welcome to 3D Escape Room! Find the clue, unlock the safe, and escape!";
    float messageTimer = 6.0f;

    std::shared_ptr<SceneObject> drawerObject = nullptr;
    std::shared_ptr<SceneObject> safeDoorObject = nullptr;
    std::shared_ptr<SceneObject> exitDoorObject = nullptr;
    std::shared_ptr<SceneObject> keyObject = nullptr;
    AudioEngine* audioEngine = nullptr;

    void update(float deltaTime);
    void interact(std::shared_ptr<SceneObject> obj, PointLight& ceilingLight);
    std::string getPromptText(const std::shared_ptr<SceneObject>& obj) const;

    void setMessage(const std::string& msg, float duration = 4.0f);
    void resetGameState();

    bool checkPin(const std::string& pin);

    void openInspect(InspectType type, const std::string& title, const std::string& body);
    void closeInspect();
    void rotateInspect(float dx, float dy);
};
