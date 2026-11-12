#ifndef PLAYER_PLAYERCONTROLLER_HPP
#define PLAYER_PLAYERCONTROLLER_HPP

#include <string>
#include <algorithm>
#include "../Math3D.hpp"
#include "../Combat/HealthComponent.hpp"
#include "../Combat/IDamageable.hpp"
#include "../Weapons/WeaponController.hpp"
#include "../Inventory/InventorySystem.hpp"
#include "../Skills/CharacterSkill.hpp"
#include "PlayerCamera.hpp"
#include "MobileControls.hpp"

namespace Player {

enum class LocomotionState {
    FREEFALL,
    PARACHUTING,
    GROUNDED,
    CROUCHING,
    JUMPING
};

class PlayerController : public Combat::IDamageable {
public:
    int id = 0;
    std::string name = "OPERATOR_07";
    Vec3 position = Vec3(0, 72.0f, 25.0f);
    Vec3 velocity = Vec3(0, 0, 0);

    float yaw = 0.0f;
    float pitch = 0.0f;

    LocomotionState state = LocomotionState::FREEFALL;
    bool isSprinting = false;
    bool isAiming = false;
    bool isCrouching = false;
    float runAnimTime = 0.0f;

    Combat::HealthComponent health;
    Weapons::WeaponController weapons;
    Inventory::InventorySystem inventory;
    Skills::CharacterSkillManager skills;
    PlayerCamera camera;
    MobileControlLayout mobileControls;

    float verticalVelocity = 0.0f;
    bool isGrounded = false;

    PlayerController() {
        health.reset(200.0f);
    }

    void reset(const Vec3& spawnPos) {
        position = spawnPos;
        velocity = Vec3(0, 0, 0);
        verticalVelocity = 0.0f;
        yaw = 0.0f;
        pitch = 0.0f;
        state = LocomotionState::FREEFALL;
        isSprinting = false;
        isAiming = false;
        isCrouching = false;
        health.reset(200.0f);
        weapons.equipWeapon(Weapons::WeaponID::UNARMED);
        inventory.reset();
    }

    // IDamageable implementation
    void takeDamage(const Combat::DamageInfo& info) override {
        health.applyDamage(info, name, id);
    }

    bool isAlive() const override {
        return health.isAlive;
    }

    int getEntityId() const override {
        return id;
    }

    std::string getEntityName() const override {
        return name;
    }

    Vec3 getPosition() const override {
        return position;
    }

    void deployParachute() {
        if (state == LocomotionState::FREEFALL) {
            state = LocomotionState::PARACHUTING;
        }
    }

    void update(float dt) {
        if (!health.isAlive) return;

        weapons.update(dt);
        skills.update(dt, health);

        // Skydiving & Parachuting Physics
        if (state == LocomotionState::FREEFALL) {
            position.y -= 19.5f * dt;
            if (position.y <= 24.0f) {
                deployParachute();
            }
        } else if (state == LocomotionState::PARACHUTING) {
            position.y -= 4.2f * dt;
            if (position.y <= 0.0f) {
                position.y = 0.0f;
                state = LocomotionState::GROUNDED;
                isGrounded = true;
            }
        }

        // Camera update
        camera.yaw = yaw;
        camera.pitch = pitch;
        camera.targetFOV = isAiming ? weapons.getSpec().zoomFOV : (state != LocomotionState::GROUNDED ? 65.0f : 55.0f);
        camera.update(position, isAiming, state != LocomotionState::GROUNDED, weapons.recoilPitch, dt);
    }
};

} // namespace Player

#endif // PLAYER_PLAYERCONTROLLER_HPP
