#ifndef COMBAT_IDAMAGEABLE_HPP
#define COMBAT_IDAMAGEABLE_HPP

#include <string>
#include "../Math3D.hpp"

namespace Combat {

enum class HitboxType {
    HEAD,
    BODY,
    LIMB
};

struct DamageInfo {
    float rawDamage = 0.0f;
    float finalDamage = 0.0f;
    HitboxType hitbox = HitboxType::BODY;
    int attackerId = -1;
    std::string attackerName = "UNKNOWN";
    std::string weaponName = "WEAPON";
    Vec3 hitPoint = Vec3(0, 0, 0);
    bool isHeadshot = false;
};

class IDamageable {
public:
    virtual ~IDamageable() = default;
    virtual void takeDamage(const DamageInfo& info) = 0;
    virtual bool isAlive() const = 0;
    virtual int getEntityId() const = 0;
    virtual std::string getEntityName() const = 0;
    virtual Vec3 getPosition() const = 0;
};

} // namespace Combat

#endif // COMBAT_IDAMAGEABLE_HPP
