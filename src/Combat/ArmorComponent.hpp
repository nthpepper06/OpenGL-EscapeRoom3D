#ifndef COMBAT_ARMORCOMPONENT_HPP
#define COMBAT_ARMORCOMPONENT_HPP

#include <algorithm>
#include "IDamageable.hpp"

namespace Combat {

class ArmorComponent {
public:
    int vestLevel = 0;       // 0: None, 1: 30% reduction, 2: 45% reduction, 3: 60% reduction
    float vestDurability = 0.0f;
    float maxVestDurability = 0.0f;

    int helmetLevel = 0;     // 0: None, 1: 25% reduction, 2: 40% reduction, 3: 50% reduction
    float helmetDurability = 0.0f;
    float maxHelmetDurability = 0.0f;

    float getVestReduction() const {
        if (vestLevel == 1) return 0.30f;
        if (vestLevel == 2) return 0.45f;
        if (vestLevel >= 3) return 0.65f;
        return 0.0f;
    }

    float getHelmetReduction() const {
        if (helmetLevel == 1) return 0.25f;
        if (helmetLevel == 2) return 0.40f;
        if (helmetLevel >= 3) return 0.55f;
        return 0.0f;
    }

    void equipVest(int level) {
        vestLevel = level;
        maxVestDurability = level * 100.0f;
        vestDurability = maxVestDurability;
    }

    void equipHelmet(int level) {
        helmetLevel = level;
        maxHelmetDurability = level * 80.0f;
        helmetDurability = maxHelmetDurability;
    }

    float mitigateDamage(float damage, HitboxType hitbox) {
        if (hitbox == HitboxType::HEAD) {
            if (helmetLevel > 0 && helmetDurability > 0.0f) {
                float reductionRatio = getHelmetReduction();
                float mitigated = damage * (1.0f - reductionRatio);
                helmetDurability = std::max(0.0f, helmetDurability - damage * 0.5f);
                if (helmetDurability <= 0.0f) helmetLevel = 0;
                return mitigated;
            }
        } else if (hitbox == HitboxType::BODY) {
            if (vestLevel > 0 && vestDurability > 0.0f) {
                float reductionRatio = getVestReduction();
                float mitigated = damage * (1.0f - reductionRatio);
                vestDurability = std::max(0.0f, vestDurability - damage * 0.5f);
                if (vestDurability <= 0.0f) vestLevel = 0;
                return mitigated;
            }
        } else if (hitbox == HitboxType::LIMB) {
            return damage * 0.75f; // Limbs naturally take 25% less damage
        }
        return damage;
    }
};

} // namespace Combat

#endif // COMBAT_ARMORCOMPONENT_HPP
