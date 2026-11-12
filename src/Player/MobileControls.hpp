#ifndef PLAYER_MOBILECONTROLS_HPP
#define PLAYER_MOBILECONTROLS_HPP

#include "../Math3D.hpp"

namespace Player {

struct VirtualTouchButton {
    float x, y, radius;
    bool isPressed = false;
    bool isTriggered = false;

    bool contains(float touchX, float touchY) const {
        float dx = touchX - x;
        float dy = touchY - y;
        return (dx * dx + dy * dy) <= (radius * radius);
    }
};

struct VirtualJoystick {
    float centerX = 140.0f;
    float centerY = 140.0f;
    float radius = 70.0f;
    float handleX = 140.0f;
    float handleY = 140.0f;
    bool isActive = false;
    Vec2 inputVector = Vec2(0, 0);

    void updateTouch(float touchX, float touchY, bool touching) {
        if (!touching) {
            isActive = false;
            handleX = centerX;
            handleY = centerY;
            inputVector = Vec2(0, 0);
            return;
        }

        isActive = true;
        float dx = touchX - centerX;
        float dy = touchY - centerY;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist > radius) {
            dx = (dx / dist) * radius;
            dy = (dy / dist) * radius;
        }

        handleX = centerX + dx;
        handleY = centerY + dy;
        inputVector = Vec2(dx / radius, dy / radius);
    }
};

class MobileControlLayout {
public:
    VirtualJoystick moveStick;

    // Mobile Virtual Touch Buttons (Screen coordinate centers)
    VirtualTouchButton btnFire;
    VirtualTouchButton btnAim;
    VirtualTouchButton btnJump;
    VirtualTouchButton btnCrouch;
    VirtualTouchButton btnReload;
    VirtualTouchButton btnMedkit;
    VirtualTouchButton btnBarrier;
    VirtualTouchButton btnWeaponSlot1;
    VirtualTouchButton btnWeaponSlot2;

    void setupLayout(int screenW, int screenH) {
        float sw = (float)screenW;
        float sh = (float)screenH;

        moveStick.centerX = 130.0f;
        moveStick.centerY = 130.0f;
        moveStick.radius = 65.0f;

        btnFire = { sw - 120.0f, 150.0f, 44.0f, false, false };
        btnAim = { sw - 70.0f, 250.0f, 36.0f, false, false };
        btnJump = { sw - 60.0f, 85.0f, 32.0f, false, false };
        btnCrouch = { sw - 140.0f, 65.0f, 30.0f, false, false };
        btnReload = { sw - 190.0f, 150.0f, 28.0f, false, false };

        btnMedkit = { sw * 0.5f - 110.0f, 45.0f, 28.0f, false, false };
        btnBarrier = { sw * 0.5f + 110.0f, 45.0f, 28.0f, false, false };

        btnWeaponSlot1 = { sw - 230.0f, sh - 60.0f, 50.0f, false, false };
        btnWeaponSlot2 = { sw - 110.0f, sh - 60.0f, 50.0f, false, false };
    }
};

} // namespace Player

#endif // PLAYER_MOBILECONTROLS_HPP
