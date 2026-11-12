#ifndef INVENTORY_INVENTORYSYSTEM_HPP
#define INVENTORY_INVENTORYSYSTEM_HPP

#include <vector>
#include <string>
#include <algorithm>
#include "ItemData.hpp"
#include "../Weapons/WeaponData.hpp"
#include "../Core/EventBus.hpp"

namespace Inventory {

struct ItemPickedUpEvent : public Core::IEvent {
    int collectorId;
    ItemData item;
    ItemPickedUpEvent(int id, const ItemData& it) : collectorId(id), item(it) {}
};

class InventorySystem {
public:
    int maxSlots = 8;
    std::vector<ItemData> items;

    // Direct Equipped Slots
    Weapons::WeaponID primaryWeapon = Weapons::WeaponID::UNARMED;
    Weapons::WeaponID secondaryWeapon = Weapons::WeaponID::UNARMED;
    Weapons::WeaponID sidearmWeapon = Weapons::WeaponID::UNARMED;

    int medkitCount = 2;
    int energyDrinkCount = 1;
    int barrierCount = 2;
    int grenadeCount = 2;

    int lightAmmo = 0;
    int mediumAmmo = 0;
    int heavyAmmo = 0;
    int sniperAmmo = 0;

    int vestLevel = 0;
    int helmetLevel = 0;
    int backpackLevel = 1; // 1: Standard (100 cap), 2: Assault (200 cap), 3: Titan (300 cap)

    void reset() {
        items.clear();
        primaryWeapon = Weapons::WeaponID::UNARMED;
        secondaryWeapon = Weapons::WeaponID::UNARMED;
        sidearmWeapon = Weapons::WeaponID::UNARMED;
        medkitCount = 2;
        energyDrinkCount = 1;
        barrierCount = 2;
        grenadeCount = 2;
        lightAmmo = 0;
        mediumAmmo = 0;
        heavyAmmo = 0;
        sniperAmmo = 0;
        vestLevel = 0;
        helmetLevel = 0;
        backpackLevel = 1;
    }

    int getMaxCapacity() const {
        if (backpackLevel <= 1) return 150;
        if (backpackLevel == 2) return 250;
        return 400;
    }

    bool addItem(const ItemData& item, int collectorId = 0) {
        if (item.category == ItemCategory::WEAPON) {
            if (primaryWeapon == Weapons::WeaponID::UNARMED) {
                primaryWeapon = item.weaponId;
            } else if (secondaryWeapon == Weapons::WeaponID::UNARMED) {
                secondaryWeapon = item.weaponId;
            } else {
                primaryWeapon = item.weaponId; // Replace primary
            }
        } else if (item.category == ItemCategory::MEDKIT) {
            medkitCount += item.count;
        } else if (item.category == ItemCategory::ENERGY_DRINK) {
            energyDrinkCount += item.count;
        } else if (item.category == ItemCategory::TACTICAL_BARRIER) {
            barrierCount += item.count;
        } else if (item.category == ItemCategory::GRENADE) {
            grenadeCount += item.count;
        } else if (item.category == ItemCategory::ARMOR_VEST) {
            vestLevel = std::max(vestLevel, item.tierLevel);
        } else if (item.category == ItemCategory::HELMET) {
            helmetLevel = std::max(helmetLevel, item.tierLevel);
        } else if (item.category == ItemCategory::BACKPACK) {
            backpackLevel = std::max(backpackLevel, item.tierLevel);
        } else if (item.category == ItemCategory::AMMO) {
            if (item.ammoType == Weapons::AmmoType::LIGHT_AMMO) lightAmmo += item.count;
            else if (item.ammoType == Weapons::AmmoType::MEDIUM_AMMO) mediumAmmo += item.count;
            else if (item.ammoType == Weapons::AmmoType::HEAVY_AMMO) heavyAmmo += item.count;
            else if (item.ammoType == Weapons::AmmoType::SNIPER_AMMO) sniperAmmo += item.count;
        }

        ItemPickedUpEvent evt(collectorId, item);
        Core::EventBus::instance().publish(evt);
        return true;
    }

    bool useMedkit() {
        if (medkitCount > 0) {
            medkitCount--;
            return true;
        }
        return false;
    }

    bool useEnergyDrink() {
        if (energyDrinkCount > 0) {
            energyDrinkCount--;
            return true;
        }
        return false;
    }

    bool useBarrier() {
        if (barrierCount > 0) {
            barrierCount--;
            return true;
        }
        return false;
    }

    bool useGrenade() {
        if (grenadeCount > 0) {
            grenadeCount--;
            return true;
        }
        return false;
    }
};

} // namespace Inventory

#endif // INVENTORY_INVENTORYSYSTEM_HPP
