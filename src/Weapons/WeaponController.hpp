#ifndef WEAPONS_WEAPONCONTROLLER_HPP
#define WEAPONS_WEAPONCONTROLLER_HPP

#include <vector>
#include <algorithm>
#include <iostream>
#include "WeaponData.hpp"
#include "../Combat/IDamageable.hpp"
#include "../Combat/DamagePopupSystem.hpp"
#include "../Core/EventBus.hpp"
#include "../Math3D.hpp"

namespace Weapons {

struct WeaponFiredEvent : public Core::IEvent {
    int shooterId;
    WeaponID weaponId;
    Vec3 fireOrigin;
    Vec3 fireDirection;
    WeaponFiredEvent(int id, WeaponID wId, const Vec3& org, const Vec3& dir)
        : shooterId(id), weaponId(wId), fireOrigin(org), fireDirection(dir) {}
};

class WeaponController {
public:
    WeaponID currentWeaponId = WeaponID::UNARMED;
    int currentMag = 0;
    int reserveAmmo = 0;
    float shootCooldown = 0.0f;
    float reloadTimer = 0.0f;
    bool isReloading = false;
    float recoilPitch = 0.0f;

    WeaponSpec getSpec() const {
        return getWeaponSpec(currentWeaponId);
    }

    void equipWeapon(WeaponID newWeapon, int initialAmmo = -1) {
        currentWeaponId = newWeapon;
        WeaponSpec spec = getSpec();
        isReloading = false;
        reloadTimer = 0.0f;

        if (initialAmmo >= 0) {
            currentMag = std::min(spec.magazineSize, initialAmmo);
            reserveAmmo = initialAmmo - currentMag;
        } else {
            currentMag = spec.magazineSize;
            reserveAmmo = spec.maxReserveAmmo;
        }
    }

    bool canShoot() const {
        return currentWeaponId != WeaponID::UNARMED &&
               shootCooldown <= 0.0f &&
               !isReloading &&
               currentMag > 0;
    }

    bool canFire() const {
        return canShoot();
    }

    void onFired() {
        WeaponSpec spec = getSpec();
        if (currentMag > 0) currentMag--;
        shootCooldown = spec.fireRate;
        float kick = (spec.id == WeaponID::THUNDER_SHOTGUN) ? 0.07f :
                     ((spec.id == WeaponID::APEX_SNIPER) ? 0.12f : 0.032f);
        recoilPitch += kick;
    }

    bool startReload() {
        if (isReloading || currentWeaponId == WeaponID::UNARMED) return false;
        WeaponSpec spec = getSpec();
        if (currentMag >= spec.magazineSize || reserveAmmo <= 0) return false;

        isReloading = true;
        reloadTimer = spec.reloadTime;
        return true;
    }

    void update(float dt) {
        if (shootCooldown > 0.0f) shootCooldown -= dt;

        if (recoilPitch > 0.0f) {
            recoilPitch -= dt * 0.16f;
            if (recoilPitch < 0.0f) recoilPitch = 0.0f;
        }

        if (isReloading) {
            reloadTimer -= dt;
            if (reloadTimer <= 0.0f) {
                isReloading = false;
                WeaponSpec spec = getSpec();
                int needed = spec.magazineSize - currentMag;
                int toLoad = std::min(needed, reserveAmmo);
                currentMag += toLoad;
                reserveAmmo -= toLoad;
            }
        }
    }

    void addAmmo(AmmoType type, int amount) {
        WeaponSpec spec = getSpec();
        if (spec.ammoType == type || type == AmmoType::BARRIER_KIT) {
            reserveAmmo += amount;
        }
    }
};

} // namespace Weapons

#endif // WEAPONS_WEAPONCONTROLLER_HPP
