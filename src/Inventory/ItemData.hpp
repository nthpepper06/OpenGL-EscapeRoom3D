#ifndef INVENTORY_ITEMDATA_HPP
#define INVENTORY_ITEMDATA_HPP

#include <string>
#include "../Weapons/WeaponData.hpp"
#include "../Math3D.hpp"

namespace Inventory {

enum class ItemCategory {
    WEAPON,
    AMMO,
    ARMOR_VEST,
    HELMET,
    BACKPACK,
    GRENADE,
    MEDKIT,
    ENERGY_DRINK,
    TACTICAL_BARRIER
};

enum class ItemRarity {
    COMMON,       // White
    UNCOMMON,     // Green
    RARE,         // Blue
    EPIC,         // Purple
    LEGENDARY     // Gold
};

inline Vec4 getRarityColor(ItemRarity rarity) {
    switch (rarity) {
        case ItemRarity::LEGENDARY: return Vec4(0.95f, 0.75f, 0.15f, 1.0f); // Gold
        case ItemRarity::EPIC:      return Vec4(0.75f, 0.25f, 0.95f, 1.0f); // Purple
        case ItemRarity::RARE:      return Vec4(0.20f, 0.65f, 0.95f, 1.0f); // Cyan/Blue
        case ItemRarity::UNCOMMON:  return Vec4(0.25f, 0.85f, 0.35f, 1.0f); // Green
        case ItemRarity::COMMON:
        default:                    return Vec4(0.85f, 0.85f, 0.88f, 1.0f); // White
    }
}

struct ItemData {
    int id = 0;
    std::string name;
    std::string description;
    ItemCategory category = ItemCategory::WEAPON;
    ItemRarity rarity = ItemRarity::COMMON;
    int count = 1;
    int maxStack = 1;
    Weapons::WeaponID weaponId = Weapons::WeaponID::UNARMED;
    Weapons::AmmoType ammoType = Weapons::AmmoType::NONE;
    int tierLevel = 1; // 1, 2, 3 for Vest/Helmet

    Vec4 getRarityColor() const {
        return Inventory::getRarityColor(rarity);
    }
};

} // namespace Inventory

#endif // INVENTORY_ITEMDATA_HPP
