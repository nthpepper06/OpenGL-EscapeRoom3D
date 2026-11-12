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

#define GL_CHECK_ERROR() \
    do { \
        GLenum err; \
        while ((err = glGetError()) != GL_NO_ERROR) { \
            std::cerr << "[OpenGL Error] (0x" << std::hex << err << std::dec \
                      << ") at " << __FILE__ << ":" << __LINE__ << "\n"; \
        } \
    } while (0)

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

enum class RenderLayer {
    Both = 0,
    RealOnly = 1,
    MirrorOnly = 2
};

enum class InteractionType {
    None,
    LightSwitch,    // Toggles ceiling point light
    DeskDrawer,     // Slides drawer open / closed
    SecretNote,     // Read note with safe PIN
    SafeKeypad,     // Enter PIN to unlock safe
    SafeDoor,       // Swings safe door open
    GoldenKey,      // Collect key for exit door
    ExitDoor,       // Escape room exit door
    Totem,          // Room 1: Rotate shadow-casting totem (0, 90, 180, 270)
    FuseItem,       // Room 2: Collectible electrical fuse
    BreakerBox,     // Room 2: Circuit breaker to restore power
    IsolationGate,  // Room 2: Iron bars security gate
    MirrorBust      // Room 3: Rotatable bust/candelabra for symmetry puzzle
};
