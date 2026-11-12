#ifndef LOOT_LOOTSPAWNER_HPP
#define LOOT_LOOTSPAWNER_HPP

#include <vector>
#include <string>
#include "../Inventory/ItemData.hpp"
#include "LootTable.hpp"
#include "../Math3D.hpp"

namespace Loot {

struct WorldItem {
    int id = 0;
    Vec3 position;
    Inventory::ItemData data;
    bool isPickedUp = false;
    float rotation = 0.0f;
    float hoverTime = 0.0f;
};

class LootSpawner {
public:
    std::vector<WorldItem> worldItems;
    int nextId = 1;

    void spawnItem(const Vec3& pos, const Inventory::ItemData& item) {
        WorldItem wi;
        wi.id = nextId++;
        wi.position = pos;
        wi.data = item;
        wi.isPickedUp = false;
        wi.rotation = (float)(rand() % 360);
        worldItems.push_back(wi);
    }

    void populateMapLoot(const LootTable& table, const std::vector<Vec3>& customSpawns = {}) {
        worldItems.clear();

        // 1. If custom POI spawn anchors are provided, place curated high-tier items
        if (!customSpawns.empty()) {
            for (size_t i = 0; i < customSpawns.size(); ++i) {
                Vec3 base = customSpawns[i];
                Inventory::ItemData it = table.getRandomItem();
                int mod = i % 10;
                if (mod == 0) {
                    it.name = "VANGUARD AR-44 (+30 AMMO)";
                    it.category = Inventory::ItemCategory::WEAPON;
                    it.rarity = Inventory::ItemRarity::RARE;
                    it.weaponId = Weapons::WeaponID::VANGUARD_AR44;
                    it.count = 30;
                } else if (mod == 1) {
                    it.name = "SPECTRE-9 SMG (+35 AMMO)";
                    it.category = Inventory::ItemCategory::WEAPON;
                    it.rarity = Inventory::ItemRarity::UNCOMMON;
                    it.weaponId = Weapons::WeaponID::SPECTRE_SMG;
                    it.count = 35;
                } else if (mod == 2) {
                    it.name = "APEX-50 BMG SNIPER (+5 AMMO)";
                    it.category = Inventory::ItemCategory::WEAPON;
                    it.rarity = Inventory::ItemRarity::LEGENDARY;
                    it.weaponId = Weapons::WeaponID::APEX_SNIPER;
                    it.count = 5;
                } else if (mod == 3) {
                    it.name = "THUNDER-12 SHOTGUN (+8 AMMO)";
                    it.category = Inventory::ItemCategory::WEAPON;
                    it.rarity = Inventory::ItemRarity::RARE;
                    it.weaponId = Weapons::WeaponID::THUNDER_SHOTGUN;
                    it.count = 8;
                } else if (mod == 4) {
                    it.name = "LIGHT VEST LV1 (+30% DEFENSE)";
                    it.category = Inventory::ItemCategory::ARMOR_VEST;
                    it.rarity = Inventory::ItemRarity::UNCOMMON;
                    it.tierLevel = 1;
                } else if (mod == 5) {
                    it.name = "TACTICAL VEST LV2 (+45% DEFENSE)";
                    it.category = Inventory::ItemCategory::ARMOR_VEST;
                    it.rarity = Inventory::ItemRarity::RARE;
                    it.tierLevel = 2;
                } else if (mod == 6) {
                    it.name = "TITAN HEAVY VEST LV3 (+65% DEFENSE)";
                    it.category = Inventory::ItemCategory::ARMOR_VEST;
                    it.rarity = Inventory::ItemRarity::LEGENDARY;
                    it.tierLevel = 3;
                } else if (mod == 7) {
                    it.name = "COMBAT HELMET LV1 (+25% HEAD DEFENSE)";
                    it.category = Inventory::ItemCategory::HELMET;
                    it.rarity = Inventory::ItemRarity::UNCOMMON;
                    it.tierLevel = 1;
                } else if (mod == 8) {
                    it.name = "SWAT HELMET LV2 (+40% HEAD DEFENSE)";
                    it.category = Inventory::ItemCategory::HELMET;
                    it.rarity = Inventory::ItemRarity::RARE;
                    it.tierLevel = 2;
                } else {
                    it.name = "SPETSNAZ TITAN HELMET LV3 (+55% HEAD DEFENSE)";
                    it.category = Inventory::ItemCategory::HELMET;
                    it.rarity = Inventory::ItemRarity::LEGENDARY;
                    it.tierLevel = 3;
                }
                spawnItem(base, it);
            }
        }

        // 2. Extra procedural drops across the 1000m island (50 additional drops)
        for (int i = 0; i < 50; ++i) {
            float rx = ((rand() % 800) - 400.0f);
            float rz = ((rand() % 800) - 400.0f);
            Inventory::ItemData item = table.getRandomItem();
            spawnItem(Vec3(rx, 0.4f, rz), item);
        }
    }

    void update(float dt) {
        for (auto& wi : worldItems) {
            if (!wi.isPickedUp) {
                wi.rotation += dt * 3.0f;
                wi.hoverTime += dt * 2.5f;
            }
        }
    }
};

} // namespace Loot

#endif // LOOT_LOOTSPAWNER_HPP
