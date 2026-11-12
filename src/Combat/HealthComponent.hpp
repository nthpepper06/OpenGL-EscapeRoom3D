#ifndef COMBAT_HEALTHCOMPONENT_HPP
#define COMBAT_HEALTHCOMPONENT_HPP

#include <algorithm>
#include <functional>
#include "ArmorComponent.hpp"
#include "IDamageable.hpp"
#include "../Core/EventBus.hpp"

namespace Combat {

struct EntityKilledEvent : public Core::IEvent {
    int killerId;
    std::string killerName;
    int victimId;
    std::string victimName;
    std::string weaponName;
    bool isHeadshot;
    EntityKilledEvent(int kId, const std::string& kName, int vId, const std::string& vName, const std::string& wName, bool hs)
        : killerId(kId), killerName(kName), victimId(vId), victimName(vName), weaponName(wName), isHeadshot(hs) {}
};

class HealthComponent {
public:
    float currentHealth = 200.0f;
    float maxHealth = 200.0f;
    float shield = 0.0f;
    float maxShield = 100.0f;
    bool isAlive = true;

    ArmorComponent armor;

    float ep = 0.0f;
    float maxEP = 200.0f;

    void reset(float maxHp = 200.0f) {
        maxHealth = maxHp;
        currentHealth = maxHp;
        shield = 50.0f;
        maxShield = 100.0f;
        ep = 0.0f;
        maxEP = 200.0f;
        isAlive = true;
        armor.vestLevel = 0;
        armor.helmetLevel = 0;
    }

    void addEP(float amount) {
        if (!isAlive) return;
        ep = std::min(maxEP, ep + amount);
    }

    void updateEP(float dt) {
        if (!isAlive) return;
        // Free Fire style: when injured, consume EP to auto-regenerate HP at 4.0 HP/s
        if (currentHealth < maxHealth && ep > 0.0f) {
            float healRate = 4.0f * dt;
            float actualDrain = std::min(ep, healRate);
            ep -= actualDrain;
            heal(actualDrain);
        }
    }

    void heal(float amount) {
        if (!isAlive) return;
        currentHealth = std::min(maxHealth, currentHealth + amount);
    }

    void addShield(float amount) {
        if (!isAlive) return;
        shield = std::min(maxShield, shield + amount);
    }

    float applyDamage(const DamageInfo& info, const std::string& victimName, int victimId) {
        if (!isAlive) return 0.0f;

        float mitigated = armor.mitigateDamage(info.rawDamage, info.hitbox);

        // Deduct shield first if available
        if (shield > 0.0f) {
            if (shield >= mitigated) {
                shield -= mitigated;
                mitigated = 0.0f;
            } else {
                mitigated -= shield;
                shield = 0.0f;
            }
        }

        currentHealth -= mitigated;

        if (currentHealth <= 0.0f) {
            currentHealth = 0.0f;
            isAlive = false;

            EntityKilledEvent deathEvent(info.attackerId, info.attackerName, victimId, victimName, info.weaponName, info.isHeadshot);
            Core::EventBus::instance().publish(deathEvent);
        }

        return mitigated;
    }
};

} // namespace Combat

#endif // COMBAT_HEALTHCOMPONENT_HPP
