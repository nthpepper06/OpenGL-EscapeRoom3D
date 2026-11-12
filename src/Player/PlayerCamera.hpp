#ifndef PLAYER_PLAYERCAMERA_HPP
#define PLAYER_PLAYERCAMERA_HPP

#include <cmath>
#include <algorithm>
#include "../Math3D.hpp"

namespace Player {

class PlayerCamera {
public:
    Vec3 position = Vec3(0, 5.0f, 10.0f);
    Vec3 forward = Vec3(0, 0, -1.0f);
    Vec3 right = Vec3(1, 0, 0);
    Vec3 up = Vec3(0, 1, 0);

    float yaw = 0.0f;
    float pitch = 0.0f;
    float currentFOV = 55.0f;
    float targetFOV = 55.0f;

    float normalDistance = 4.5f;
    float aimDistance = 2.4f;
    float currentDistance = 4.5f;

    float normalHeight = 2.1f;
    float aimHeight = 1.65f;
    float currentHeight = 2.1f;

    float shoulderOffset = 0.65f;

    Vec3 getForward() const {
        return forward;
    }

    Vec3 getRight() const {
        return right;
    }

    void update(const Vec3& targetPos, bool isAiming, bool isAirborne, float recoilPitch, float dt) {
        // Smooth FOV interpolation
        currentFOV = lerp(Vec3(currentFOV, 0, 0), Vec3(targetFOV, 0, 0), dt * 14.0f).x;

        // Camera distance & height based on state
        float targetDist = isAirborne ? 6.5f : (isAiming ? aimDistance : normalDistance);
        float targetH = isAirborne ? 2.5f : (isAiming ? aimHeight : normalHeight);
        float targetShoulder = isAirborne ? 0.0f : (isAiming ? 0.75f : shoulderOffset);

        currentDistance = lerp(Vec3(currentDistance, 0, 0), Vec3(targetDist, 0, 0), dt * 12.0f).x;
        currentHeight = lerp(Vec3(currentHeight, 0, 0), Vec3(targetH, 0, 0), dt * 12.0f).x;

        float effectivePitch = pitch + recoilPitch;
        float cosP = std::cos(effectivePitch);
        float sinP = std::sin(effectivePitch);
        float cosY = std::cos(yaw);
        float sinY = std::sin(yaw);

        forward = Vec3(sinY * cosP, sinP, -cosY * cosP).normalized();
        right = Vec3(cosY, 0, sinY).normalized();
        up = cross(right, forward).normalized();

        position = targetPos + Vec3(0, currentHeight, 0) - forward * currentDistance + right * targetShoulder;
    }

    void updateDriving(const Vec3& vehiclePos, float vehicleYaw, float vehicleSpeed, bool isNitro, float dt) {
        // Dynamic FOV interpolation based on speed & nitro boost
        float speedRatio = std::clamp(std::abs(vehicleSpeed) / 25.0f, 0.0f, 1.0f);
        float desiredFOV = 58.0f + speedRatio * 8.0f + (isNitro ? 6.0f : 0.0f);
        currentFOV = lerp(Vec3(currentFOV, 0, 0), Vec3(desiredFOV, 0, 0), dt * 10.0f).x;

        // Vehicle camera distance and elevation
        float targetDist = 7.2f + speedRatio * 1.8f;
        float targetH = 2.7f + speedRatio * 0.5f;
        currentDistance = lerp(Vec3(currentDistance, 0, 0), Vec3(targetDist, 0, 0), dt * 10.0f).x;
        currentHeight = lerp(Vec3(currentHeight, 0, 0), Vec3(targetH, 0, 0), dt * 10.0f).x;

        float effectivePitch = pitch;
        float cosP = std::cos(effectivePitch);
        float sinP = std::sin(effectivePitch);
        float cosY = std::cos(yaw);
        float sinY = std::sin(yaw);

        forward = Vec3(sinY * cosP, sinP, -cosY * cosP).normalized();
        right = Vec3(cosY, 0, sinY).normalized();
        up = cross(right, forward).normalized();

        // Third-person chase / orbit position behind vehicle
        position = vehiclePos + Vec3(0, currentHeight, 0) - forward * currentDistance;
    }

    Mat4 getViewMatrix() const {
        return Mat4::lookAt(position, position + forward, Vec3(0, 1, 0));
    }

    Mat4 getProjectionMatrix(float aspect, float nearZ = 0.2f, float farZ = 1400.0f) const {
        return Mat4::perspective(currentFOV * DEG2RAD, aspect, nearZ, farZ);
    }
};

} // namespace Player

#endif // PLAYER_PLAYERCAMERA_HPP
