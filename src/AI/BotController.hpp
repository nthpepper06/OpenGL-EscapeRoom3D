#ifndef AI_BOTCONTROLLER_HPP
#define AI_BOTCONTROLLER_HPP

#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include "../Math3D.hpp"
#include "../Combat/HealthComponent.hpp"
#include "../Combat/IDamageable.hpp"
#include "../Weapons/WeaponController.hpp"
#include "../Inventory/InventorySystem.hpp"

namespace AI {

enum class BotState {
    AIRBORNE,
    SEARCH_LOOT,
    EXPLORE,
    ENGAGE_TARGET,
    TAKE_COVER,
    HEAL,
    ROTATE_ZONE,
    DEAD
};

class BotController : public Combat::IDamageable {
public:
    int id;
    std::string name;
    Vec3 position;
    Vec3 velocity;
    float yaw = 0.0f;
    float pitch = 0.0f;

    BotState state = BotState::AIRBORNE;
    bool isParachuting = false;
    float runAnim = 0.0f;

    Combat::HealthComponent health;
    Weapons::WeaponController weapons;
    Inventory::InventorySystem inventory;

    Vec3 moveTarget = Vec3(0, 0, 0);
    float decisionTimer = 0.0f;
    float targetScanTimer = 0.0f;
    int targetEntityId = -1;

    BotController(int _id, const std::string& _name, const Vec3& spawnPos)
        : id(_id), name(_name), position(spawnPos) {
        health.reset(200.0f);
        weapons.equipWeapon(Weapons::WeaponID::UNARMED);
    }

    // IDamageable implementation
    void takeDamage(const Combat::DamageInfo& info) override {
        health.applyDamage(info, name, id);
        if (!health.isAlive) {
            state = BotState::DEAD;
        } else {
            // React to being shot: turn towards shooter or take cover
            if (health.currentHealth < 80.0f && inventory.barrierCount > 0) {
                state = BotState::TAKE_COVER;
            } else {
                targetEntityId = info.attackerId;
                state = BotState::ENGAGE_TARGET;
            }
        }
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
};

} // namespace AI

#endif // AI_BOTCONTROLLER_HPP
