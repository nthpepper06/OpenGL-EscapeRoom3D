#ifndef SKILLS_CHARACTERSKILL_HPP
#define SKILLS_CHARACTERSKILL_HPP

#include <string>
#include <vector>
#include <algorithm>
#include "../Math3D.hpp"
#include "../Combat/HealthComponent.hpp"
#include "../Audio/AudioManager.hpp"

namespace Skills {

enum class SkillID {
    VANGUARD_AURA, // Healing Aura + Speed Boost + Defense
    PHANTOM_DASH,  // Instant Dash + Semi-Cloaking
    SONAR_RECON    // 360 Recon Ping + 20% Bonus Damage
};

struct SkillSpec {
    SkillID id = SkillID::VANGUARD_AURA;
    std::string name;
    std::string shortName;
    std::string description;
    float duration = 6.0f;
    float cooldown = 25.0f;
    Vec4 themeColor = Vec4(0.2f, 0.85f, 1.0f, 1.0f);
};

class CharacterSkillManager {
public:
    SkillID currentSkillId = SkillID::VANGUARD_AURA;
    float cooldownTimer = 0.0f;
    float activeTimer = 0.0f;
    float auraRadius = 6.0f;

    SkillSpec getSpec(SkillID id) const {
        SkillSpec s;
        if (id == SkillID::VANGUARD_AURA) {
            s.id = SkillID::VANGUARD_AURA;
            s.name = "VANGUARD AURA";
            s.shortName = "AURA";
            s.description = "+40% Speed, +15 HP/s, +20% Armor";
            s.duration = 6.0f;
            s.cooldown = 25.0f;
            s.themeColor = Vec4(0.2f, 0.95f, 0.85f, 1.0f); // Cyan Gold
        } else if (id == SkillID::PHANTOM_DASH) {
            s.id = SkillID::PHANTOM_DASH;
            s.name = "PHANTOM DASH";
            s.shortName = "DASH";
            s.description = "12m Warp Dash + 70% Cloaking";
            s.duration = 4.0f;
            s.cooldown = 20.0f;
            s.themeColor = Vec4(0.75f, 0.25f, 0.95f, 1.0f); // Purple Neon
        } else {
            s.id = SkillID::SONAR_RECON;
            s.name = "SONAR RECON";
            s.shortName = "SCAN";
            s.description = "70m Enemy Radar Ping + 20% DMG";
            s.duration = 8.0f;
            s.cooldown = 30.0f;
            s.themeColor = Vec4(0.95f, 0.65f, 0.15f, 1.0f); // Amber Orange
        }
        return s;
    }

    SkillSpec getCurrentSpec() const {
        return getSpec(currentSkillId);
    }

    bool isReady() const {
        return cooldownTimer <= 0.0f;
    }

    bool isActive() const {
        return activeTimer > 0.0f;
    }

    float getCooldownProgress() const {
        SkillSpec spec = getCurrentSpec();
        if (cooldownTimer <= 0.0f) return 1.0f;
        return 1.0f - (cooldownTimer / spec.cooldown);
    }

    void cycleSkill() {
        if (currentSkillId == SkillID::VANGUARD_AURA) currentSkillId = SkillID::PHANTOM_DASH;
        else if (currentSkillId == SkillID::PHANTOM_DASH) currentSkillId = SkillID::SONAR_RECON;
        else currentSkillId = SkillID::VANGUARD_AURA;
        cooldownTimer = 0.0f;
        activeTimer = 0.0f;
        Audio::AudioManager::instance().playButtonClick();
    }

    bool activate(Vec3& playerPos, const Vec3& forwardDir, Combat::HealthComponent& health) {
        if (!isReady()) return false;

        SkillSpec spec = getCurrentSpec();
        activeTimer = spec.duration;
        cooldownTimer = spec.cooldown;

        if (currentSkillId == SkillID::VANGUARD_AURA) {
            Audio::AudioManager::instance().playSkillSurge();
        } else if (currentSkillId == SkillID::PHANTOM_DASH) {
            // Instant 12m Dash forward
            Vec3 dashDir = forwardDir;
            dashDir.y = 0.0f;
            if (dashDir.lengthSq() > 1e-4f) {
                playerPos += dashDir.normalized() * 12.0f;
            }
            Audio::AudioManager::instance().playSkillDash();
        } else if (currentSkillId == SkillID::SONAR_RECON) {
            Audio::AudioManager::instance().playSkillScan();
        }

        return true;
    }

    void update(float dt, Combat::HealthComponent& health) {
        if (cooldownTimer > 0.0f) {
            cooldownTimer -= dt;
            if (cooldownTimer < 0.0f) cooldownTimer = 0.0f;
        }

        if (activeTimer > 0.0f) {
            activeTimer -= dt;
            if (activeTimer < 0.0f) activeTimer = 0.0f;

            // Vanguard Aura active tick: Heal 15 HP/Shield per second
            if (currentSkillId == SkillID::VANGUARD_AURA) {
                health.heal(15.0f * dt);
                health.addShield(10.0f * dt);
            }
        }
    }
};

} // namespace Skills

#endif // SKILLS_CHARACTERSKILL_HPP
