#ifndef WEAPONS_WEAPONDATA_HPP
#define WEAPONS_WEAPONDATA_HPP

#include <string>
#include <vector>

namespace Weapons {

enum class WeaponCategory {
    MELEE,
    PISTOL,
    SMG,
    ASSAULT_RIFLE,
    SHOTGUN,
    SNIPER_RIFLE,
    TACTICAL_BARRIER
};

enum class WeaponID {
    UNARMED,
    KATANA_VANGUARD,
    FALCON_PISTOL,
    SPECTRE_SMG,
    VANGUARD_AR44,
    THUNDER_SHOTGUN,
    APEX_SNIPER,
    AEGIS_BARRIER
};

enum class AmmoType {
    NONE,
    LIGHT_AMMO,    // 9mm for SMG/Pistol
    MEDIUM_AMMO,   // 5.56 for AR
    HEAVY_AMMO,    // 12 Gauge for Shotgun
    SNIPER_AMMO,   // .50 cal for Sniper
    BARRIER_KIT    // Deployable Barrier Core
};

struct WeaponSpec {
    WeaponID id = WeaponID::UNARMED;
    WeaponCategory category = WeaponCategory::MELEE;
    std::string displayName = "TAY KHONG";
    std::string skinName = "DEFAULT";
    float damage = 15.0f;
    float headshotMultiplier = 2.0f;
    float limbMultiplier = 0.75f;
    float fireRate = 0.45f;        // Seconds between shots
    float spread = 0.04f;          // Bullet deviation
    int magazineSize = 0;
    int maxReserveAmmo = 0;
    float reloadTime = 0.0f;
    float effectiveRange = 3.0f;
    float zoomFOV = 55.0f;         // Camera FOV when ADS
    int pelletsPerShot = 1;        // Multi-pellet for Shotgun
    AmmoType ammoType = AmmoType::NONE;
    bool isAutomatic = false;
};

inline WeaponSpec getWeaponSpec(WeaponID id) {
    switch (id) {
        case WeaponID::VANGUARD_AR44:
            return {
                WeaponID::VANGUARD_AR44, WeaponCategory::ASSAULT_RIFLE,
                "VANGUARD AR-44", "PHOENIX FLAME",
                34.0f, 2.5f, 0.75f, 0.11f, 0.032f, 30, 150, 2.1f, 130.0f, 42.0f, 1,
                AmmoType::MEDIUM_AMMO, true
            };
        case WeaponID::SPECTRE_SMG:
            return {
                WeaponID::SPECTRE_SMG, WeaponCategory::SMG,
                "SPECTRE-9 SMG", "CYAN OVERDRIVE",
                21.0f, 2.2f, 0.75f, 0.075f, 0.052f, 35, 180, 1.6f, 75.0f, 46.0f, 1,
                AmmoType::LIGHT_AMMO, true
            };
        case WeaponID::THUNDER_SHOTGUN:
            return {
                WeaponID::THUNDER_SHOTGUN, WeaponCategory::SHOTGUN,
                "THUNDER-12", "MAGMA IMPACT",
                26.0f, 2.0f, 0.75f, 0.65f, 0.11f, 2, 28, 1.8f, 42.0f, 50.0f, 6,
                AmmoType::HEAVY_AMMO, false
            };
        case WeaponID::APEX_SNIPER:
            return {
                WeaponID::APEX_SNIPER, WeaponCategory::SNIPER_RIFLE,
                "APEX-50 BMG", "EMERALD FURY",
                145.0f, 2.8f, 0.75f, 1.35f, 0.003f, 5, 20, 2.6f, 280.0f, 18.0f, 1,
                AmmoType::SNIPER_AMMO, false
            };
        case WeaponID::FALCON_PISTOL:
            return {
                WeaponID::FALCON_PISTOL, WeaponCategory::PISTOL,
                "FALCON-45", "TACTICAL STEALTH",
                28.0f, 2.2f, 0.75f, 0.22f, 0.035f, 15, 60, 1.4f, 65.0f, 48.0f, 1,
                AmmoType::LIGHT_AMMO, false
            };
        case WeaponID::AEGIS_BARRIER:
            return {
                WeaponID::AEGIS_BARRIER, WeaponCategory::TACTICAL_BARRIER,
                "AEGIS ICE BARRIER", "FROST CITADEL",
                0.0f, 1.0f, 1.0f, 0.8f, 0.0f, 1, 5, 0.0f, 10.0f, 55.0f, 1,
                AmmoType::BARRIER_KIT, false
            };
        case WeaponID::UNARMED:
        default:
            return {
                WeaponID::UNARMED, WeaponCategory::MELEE,
                "TAY KHONG", "DEFAULT",
                18.0f, 1.5f, 0.75f, 0.45f, 0.05f, 0, 0, 0.0f, 2.5f, 55.0f, 1,
                AmmoType::NONE, false
            };
    }
}

} // namespace Weapons

#endif // WEAPONS_WEAPONDATA_HPP
