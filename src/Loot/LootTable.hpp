#ifndef LOOT_LOOTTABLE_HPP
#define LOOT_LOOTTABLE_HPP

#include <vector>
#include <cstdlib>
#include "../Inventory/ItemData.hpp"

namespace Loot {

struct LootEntry {
    Inventory::ItemData item;
    int weight = 10;
};

class LootTable {
private:
    std::vector<LootEntry> entries;
    int totalWeight = 0;

public:
    void addEntry(const Inventory::ItemData& item, int weight) {
        entries.push_back({item, weight});
        totalWeight += weight;
    }

    Inventory::ItemData getRandomItem() const {
        if (entries.empty() || totalWeight <= 0) return {};

        int rnd = rand() % totalWeight;
        int accumulated = 0;
        for (const auto& entry : entries) {
            accumulated += entry.weight;
            if (rnd < accumulated) {
                return entry.item;
            }
        }
        return entries.back().item;
    }

    static LootTable createDefaultTable() {
        LootTable table;
        // Assault Rifle
        Inventory::ItemData ar;
        ar.name = "VANGUARD AR-44";
        ar.category = Inventory::ItemCategory::WEAPON;
        ar.rarity = Inventory::ItemRarity::RARE;
        ar.weaponId = Weapons::WeaponID::VANGUARD_AR44;
        ar.count = 30;
        table.addEntry(ar, 25);

        // SMG
        Inventory::ItemData smg;
        smg.name = "SPECTRE-9 SMG";
        smg.category = Inventory::ItemCategory::WEAPON;
        smg.rarity = Inventory::ItemRarity::UNCOMMON;
        smg.weaponId = Weapons::WeaponID::SPECTRE_SMG;
        smg.count = 35;
        table.addEntry(smg, 30);

        // Shotgun
        Inventory::ItemData sg;
        sg.name = "THUNDER-12";
        sg.category = Inventory::ItemCategory::WEAPON;
        sg.rarity = Inventory::ItemRarity::RARE;
        sg.weaponId = Weapons::WeaponID::THUNDER_SHOTGUN;
        sg.count = 8;
        table.addEntry(sg, 20);

        // Sniper
        Inventory::ItemData sniper;
        sniper.name = "APEX-50 BMG";
        sniper.category = Inventory::ItemCategory::WEAPON;
        sniper.rarity = Inventory::ItemRarity::LEGENDARY;
        sniper.weaponId = Weapons::WeaponID::APEX_SNIPER;
        sniper.count = 5;
        table.addEntry(sniper, 10);

        // Medkit
        Inventory::ItemData med;
        med.name = "MEDKIT (+75 HP)";
        med.category = Inventory::ItemCategory::MEDKIT;
        med.rarity = Inventory::ItemRarity::UNCOMMON;
        med.count = 1;
        table.addEntry(med, 35);

        // Kinetic Barrier
        Inventory::ItemData barrier;
        barrier.name = "AEGIS BARRIER (+1)";
        barrier.category = Inventory::ItemCategory::TACTICAL_BARRIER;
        barrier.rarity = Inventory::ItemRarity::EPIC;
        barrier.count = 1;
        table.addEntry(barrier, 25);

        // Armor Vest Tier 1, 2, 3
        Inventory::ItemData vest1;
        vest1.name = "LIGHT VEST LV1 (+30% ARMOR)";
        vest1.category = Inventory::ItemCategory::ARMOR_VEST;
        vest1.rarity = Inventory::ItemRarity::UNCOMMON;
        vest1.tierLevel = 1;
        table.addEntry(vest1, 30);

        Inventory::ItemData vest2;
        vest2.name = "TACTICAL VEST LV2 (+45% ARMOR)";
        vest2.category = Inventory::ItemCategory::ARMOR_VEST;
        vest2.rarity = Inventory::ItemRarity::RARE;
        vest2.tierLevel = 2;
        table.addEntry(vest2, 22);

        Inventory::ItemData vest3;
        vest3.name = "TITAN HEAVY VEST LV3 (+65% ARMOR)";
        vest3.category = Inventory::ItemCategory::ARMOR_VEST;
        vest3.rarity = Inventory::ItemRarity::LEGENDARY;
        vest3.tierLevel = 3;
        table.addEntry(vest3, 12);

        // Tactical Helmet Tier 1, 2, 3
        Inventory::ItemData helm1;
        helm1.name = "COMBAT HELMET LV1 (+25% HEAD DEFENSE)";
        helm1.category = Inventory::ItemCategory::HELMET;
        helm1.rarity = Inventory::ItemRarity::UNCOMMON;
        helm1.tierLevel = 1;
        table.addEntry(helm1, 30);

        Inventory::ItemData helm2;
        helm2.name = "SWAT HELMET LV2 (+40% HEAD DEFENSE)";
        helm2.category = Inventory::ItemCategory::HELMET;
        helm2.rarity = Inventory::ItemRarity::RARE;
        helm2.tierLevel = 2;
        table.addEntry(helm2, 22);

        Inventory::ItemData helm3;
        helm3.name = "SPETSNAZ TITAN HELMET LV3 (+55% HEAD DEFENSE)";
        helm3.category = Inventory::ItemCategory::HELMET;
        helm3.rarity = Inventory::ItemRarity::LEGENDARY;
        helm3.tierLevel = 3;
        table.addEntry(helm3, 12);

        // Backpack Tier 1, 2, 3
        Inventory::ItemData bag1;
        bag1.name = "RECON BACKPACK LV1 (150 CAP)";
        bag1.category = Inventory::ItemCategory::BACKPACK;
        bag1.rarity = Inventory::ItemRarity::UNCOMMON;
        bag1.tierLevel = 1;
        table.addEntry(bag1, 25);

        Inventory::ItemData bag2;
        bag2.name = "MILITARY BACKPACK LV2 (250 CAP)";
        bag2.category = Inventory::ItemCategory::BACKPACK;
        bag2.rarity = Inventory::ItemRarity::RARE;
        bag2.tierLevel = 2;
        table.addEntry(bag2, 20);

        Inventory::ItemData bag3;
        bag3.name = "TITAN EXPEDITION BACKPACK LV3 (400 CAP)";
        bag3.category = Inventory::ItemCategory::BACKPACK;
        bag3.rarity = Inventory::ItemRarity::LEGENDARY;
        bag3.tierLevel = 3;
        table.addEntry(bag3, 10);

        // Frag Grenades
        Inventory::ItemData nade;
        nade.name = "FRAG GRENADE (+180 AOE DMG)";
        nade.category = Inventory::ItemCategory::GRENADE;
        nade.rarity = Inventory::ItemRarity::UNCOMMON;
        nade.count = 2;
        table.addEntry(nade, 25);

        return table;
    }
};

} // namespace Loot

#endif // LOOT_LOOTTABLE_HPP
