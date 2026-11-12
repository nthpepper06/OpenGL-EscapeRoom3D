#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <nlohmann/json.hpp>

enum class AppMode {
    PlayMode,   // Cursor captured, WASD + Mouse look, Crosshair active, Raycasting active
    EditMode    // Cursor free, ImGui active, Object selection and transform editing
};

enum class RoomTheme {
    OccultRitual,       // Phòng tế lễ hắc ám (tường đá, vòng tròn máu, nến)
    AbandonedAsylum,    // Bệnh viện tâm thần bỏ hoang (tường tróc sơn, cáng cứu thương, tủ hồ sơ)
    MirrorRoom          // Phòng gương ma quái (gương cổ, phản chiếu kì bí)
};

enum class PrimitiveType {
    None,
    Cube,
    Cylinder,
    Plane
};

enum class InteractionType {
    None,
    LightSwitch,    // Toggles ceiling point light
    DeskDrawer,     // Slides drawer open / closed
    SecretNote,     // Read note with safe PIN
    SafeKeypad,     // Enter PIN to unlock safe
    SafeDoor,       // Swings safe door open
    GoldenKey,      // Collect key for exit door
    ExitDoor        // Escape room exit door
};
