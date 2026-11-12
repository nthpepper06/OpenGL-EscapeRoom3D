#pragma once

#include "core/Common.hpp"
#include "scene/SceneObject.hpp"
#include "renderer/Light.hpp"

class InteractionManager {
public:
    bool hasGoldenKey = false;
    bool safeUnlocked = false;
    bool exitUnlocked = false;
    bool gameWon = false;
    bool requestRoomTransition = false;

    std::string correctSafePin = "7492";
    std::string enteredPin = "";
    bool showKeypadModal = false;

    std::string statusMessage = "Welcome to 3D Escape Room! Find the clue, unlock the safe, and escape!";
    float messageTimer = 6.0f;

    std::shared_ptr<SceneObject> drawerObject = nullptr;
    std::shared_ptr<SceneObject> safeDoorObject = nullptr;
    std::shared_ptr<SceneObject> exitDoorObject = nullptr;
    std::shared_ptr<SceneObject> keyObject = nullptr;

    void update(float deltaTime);
    void interact(std::shared_ptr<SceneObject> obj, PointLight& ceilingLight);
    std::string getPromptText(const std::shared_ptr<SceneObject>& obj) const;

    void setMessage(const std::string& msg, float duration = 4.0f);
    void resetGameState();

    bool checkPin(const std::string& pin);
};
