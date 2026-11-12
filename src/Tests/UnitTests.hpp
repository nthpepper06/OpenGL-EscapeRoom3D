#ifndef TESTS_UNITTESTS_HPP
#define TESTS_UNITTESTS_HPP

#include <iostream>
#include <cassert>
#include "../Combat/HealthComponent.hpp"
#include "../Inventory/InventorySystem.hpp"
#include "../SafeZone/SafeZoneManager.hpp"
#include "../Match/MatchManager.hpp"
#include "../Weapons/WeaponData.hpp"
#include "../Skills/CharacterSkill.hpp"
#include "../AI/BotSpawner.hpp"
#include "../AI/SpatialGrid.hpp"
#include "../World/IslandLayout.hpp"
#include "../Vehicles/Vehicle.hpp"

namespace Tests {

class TestRunner {
public:
    static bool runAllTests() {
        std::cout << "\n========================================\n";
        std::cout << "   RUNNING AUTOMATED UNIT TEST SUITE    \n";
        std::cout << "========================================\n";

        int passed = 0;
        int total = 0;

        auto testAssert = [&](bool condition, const std::string& testName) {
            total++;
            if (condition) {
                std::cout << " [PASS] " << testName << "\n";
                passed++;
            } else {
                std::cerr << " [FAIL] " << testName << "\n";
            }
        };

        // 1. Health & Damage Mitigation Test
        {
            Combat::HealthComponent health;
            health.reset(200.0f);
            health.armor.equipVest(2); // Level 2 vest (45% body mitigation)

            Combat::DamageInfo info;
            info.rawDamage = 100.0f;
            info.hitbox = Combat::HitboxType::BODY;
            info.attackerId = 1;
            info.attackerName = "Bot";
            info.weaponName = "AR-44";

            float taken = health.applyDamage(info, "Player", 0);
            testAssert(taken <= 55.1f && health.currentHealth < 200.0f, "Combat: Armor Mitigates Body Damage Correctly");
        }

        // 2. Inventory & Equipping Test
        {
            Inventory::InventorySystem inv;
            inv.reset();

            Inventory::ItemData ar;
            ar.category = Inventory::ItemCategory::WEAPON;
            ar.weaponId = Weapons::WeaponID::VANGUARD_AR44;
            ar.count = 30;

            inv.addItem(ar);
            testAssert(inv.primaryWeapon == Weapons::WeaponID::VANGUARD_AR44, "Inventory: Weapon Equips to Primary Slot");

            Inventory::ItemData med;
            med.category = Inventory::ItemCategory::MEDKIT;
            med.count = 2;
            inv.addItem(med);
            testAssert(inv.medkitCount == 4, "Inventory: Medkits Stack Correctly");
        }

        // 3. Safe Zone Distance Calculation Test
        {
            SafeZone::SafeZoneManager zone;
            zone.reset();
            zone.currentCenter = Vec3(0, 0, 0);
            zone.currentRadius = 480.0f;

            testAssert(zone.isInsideZone(Vec3(200, 0, 200)), "SafeZone: Inside Zone Radius Check");
            testAssert(!zone.isInsideZone(Vec3(520, 0, 0)), "SafeZone: Outside Zone Radius Check");
        }

        // 4. Helmet Headshot Mitigation Test
        {
            Combat::HealthComponent health;
            health.reset(200.0f);
            health.shield = 0.0f;
            health.armor.equipHelmet(2); // Level 2 helmet (40% headshot reduction)

            Combat::DamageInfo headshot;
            headshot.rawDamage = 150.0f;
            headshot.hitbox = Combat::HitboxType::HEAD;
            headshot.attackerId = 1;
            headshot.attackerName = "SniperBot";
            headshot.weaponName = "APEX-50";

            float taken = health.applyDamage(headshot, "Player", 0);
            testAssert(taken <= 90.1f && health.currentHealth < 200.0f, "Combat: Helmet Mitigates Headshot Damage Correctly");
        }

        // 5. Match State & 100-Player Victory Condition Test
        {
            Match::MatchManager match;
            match.startMatch();
            testAssert(match.currentState == Match::MatchState::DEPLOYING, "Match: Start Transitions to Deploying");
            testAssert(match.totalPlayers == 100 && match.alivePlayers == 100, "Match: 100 Players Initialized in Match");

            // Simulate eliminating all 99 bots
            for (int i = 0; i < 99; ++i) {
                match.onEntityEliminated(false);
            }
            testAssert(match.currentState == Match::MatchState::VICTORY, "Match: Last Survivor (#1 of 100) Triggers Victory State");

            match.calculateResults();
            testAssert(match.earnedXP > 0 && match.playerRank == 1, "Match: Results & XP Calculated for Rank #1 / 100");
        }

        // 6. Backpack Level Capacity Scaling Test
        {
            Inventory::InventorySystem inv;
            inv.reset();
            inv.backpackLevel = 1;
            testAssert(inv.getMaxCapacity() == 150, "Backpack: Level 1 Capacity is 150");

            inv.backpackLevel = 2;
            testAssert(inv.getMaxCapacity() == 250, "Backpack: Level 2 Capacity is 250");

            inv.backpackLevel = 3;
            testAssert(inv.getMaxCapacity() == 400, "Backpack: Level 3 Capacity is 400");
        }

        // 7. Frag Grenade Item System Test
        {
            Inventory::InventorySystem inv;
            inv.reset();
            inv.grenadeCount = 2;

            bool used1 = inv.useGrenade();
            testAssert(used1 && inv.grenadeCount == 1, "Grenade: First Grenade Consumed");

            bool used2 = inv.useGrenade();
            testAssert(used2 && inv.grenadeCount == 0, "Grenade: Second Grenade Consumed");

            bool used3 = inv.useGrenade();
            testAssert(!used3 && inv.grenadeCount == 0, "Grenade: Cannot Use When Empty");
        }

        // 8. Character Skill System Test
        {
            Skills::CharacterSkillManager skills;

            Combat::HealthComponent health;
            health.reset(200.0f);
            health.currentHealth = 50.0f; // Damaged health to verify healing
            health.shield = 20.0f;

            testAssert(skills.currentSkillId == Skills::SkillID::VANGUARD_AURA, "Skills: Default Skill is Vanguard Aura");

            Vec3 pos(0, 0, 0);
            Vec3 fwd(0, 0, -1);
            bool activated = skills.activate(pos, fwd, health);
            testAssert(activated && skills.isActive(), "Skills: Activation Triggers Active State");

            // Tick active aura duration (heals 15 HP/s)
            skills.update(1.0f, health);
            testAssert(health.currentHealth > 50.0f, "Skills: Vanguard Aura Restores Health Over Time");

            // Complete duration and test cooldown
            skills.update(9.0f, health);
            testAssert(!skills.isActive() && skills.cooldownTimer > 0.0f, "Skills: Cooldown Begins After Active Duration");

            // Skill swapping
            skills.cycleSkill();
            testAssert(skills.currentSkillId == Skills::SkillID::PHANTOM_DASH, "Skills: Cycle Switches to Phantom Dash");
        }

        // 9. Bot-vs-Bot Combat Damage & Elimination Test
        {
            Combat::HealthComponent bot1Health;
            Combat::HealthComponent bot2Health;
            bot1Health.reset(100.0f);
            bot2Health.reset(100.0f);
            bot2Health.shield = 0.0f;

            Combat::DamageInfo botShot;
            botShot.rawDamage = 60.0f;
            botShot.hitbox = Combat::HitboxType::BODY;
            botShot.attackerId = 101;
            botShot.attackerName = "GhostBot";
            botShot.weaponName = "Vanguard AR-44";

            float taken1 = bot2Health.applyDamage(botShot, "ViperBot", 102);
            testAssert(taken1 > 0.0f && bot2Health.currentHealth < 100.0f, "Bot-vs-Bot: Bot Takes Damage From Another Bot");

            float taken2 = bot2Health.applyDamage(botShot, "ViperBot", 102);
            testAssert(!bot2Health.isAlive, "Bot-vs-Bot: Bot Eliminates Another Bot");
        }

        // 10. 99 AI Bot Spawner Roster Test
        {
            AI::BotSpawner spawner;
            spawner.spawnRoster(99);
            testAssert(spawner.bots.size() == 99, "AI Spawner: Exactly 99 Tactical AI Bots Spawned");
            testAssert(spawner.bots[0].position.y >= 100.0f, "AI Spawner: Bots Spawn High Altitude in Airborne State");
            testAssert(spawner.bots[0].state == AI::BotState::AIRBORNE, "AI Spawner: Initial State is AIRBORNE");
        }

        // 11. Spatial Hash Grid O(1) Radius Query Test
        {
            AI::SpatialGrid grid(40.0f);
            grid.clear();
            grid.insert(0, Vec3(0, 0, 0), true, true);
            grid.insert(1, Vec3(15, 0, 15), false, true);
            grid.insert(2, Vec3(350, 0, 350), false, true);

            auto nearEnts = grid.queryRadius(Vec3(0, 0, 0), 30.0f);
            testAssert(nearEnts.size() == 2, "SpatialGrid: Radius Query Returns Only Entities Within 30m");

            auto farEnts = grid.queryRadius(Vec3(350, 0, 350), 20.0f);
            testAssert(farEnts.size() == 1 && farEnts[0].id == 2, "SpatialGrid: Far Sector Query Returns Correct Entity");
        }

        // 12. 1000m Military Island Layout Test (8 POIs & 220+ Loot Spawns)
        {
            World::IslandLayout layout;
            layout.init();
            testAssert(layout.pois.size() == 8, "IslandLayout: 8 Iconic Military POIs Initialized (Peak, Clock Tower, Factory, etc.)");
            testAssert(layout.lootSpawns.size() >= 200, "IslandLayout: 200+ Loot Spawn Anchors Distributed Across Island");
            testAssert(layout.buildings.size() >= 15, "IslandLayout: Key Landmark Structures Placed (Hangar, Clock Tower, Radar)");
        }

        // 13. 5-Phase 1000m Safe Zone Schedule Test
        {
            SafeZone::SafeZoneManager zone;
            zone.reset();
            testAssert(zone.currentRadius == 480.0f, "SafeZone: Starting Phase 1 Radius is 480m");
            testAssert(zone.maxPhases == 5, "SafeZone: Configured for 5 Shrinking Phases");
        }

        // 14. Vehicle System Test (Physics, Speeds, Damage & 16 Island Spawns)
        {
            Vehicles::VehicleManager vm;
            vm.init();
            testAssert(vm.vehicles.size() == 16, "Vehicles: Exactly 16 Island Vehicles Spawned Across 8 POIs");

            auto& moto = vm.vehicles[0];
            testAssert(moto.type == Vehicles::VehicleType::MOTORBIKE, "Vehicles: First Spawn is Tactical Motorbike");
            moto.currentSpeed = 20.0f; // 20 m/s = 72 km/h
            testAssert(moto.getSpeedKmh() >= 71.9f && moto.getSpeedKmh() <= 72.1f, "Vehicles: Speedometer Kmh Conversion Accurate");

            moto.applyDamage(100.0f);
            testAssert(moto.health == 350.0f && moto.isAlive, "Vehicles: Vehicle Takes Damage Correctly");

            moto.applyDamage(400.0f);
            testAssert(!moto.isAlive && moto.health == 0.0f, "Vehicles: Vehicle Destroys at 0 HP");

            int nearest = vm.findNearestVehicle(Vec3(-18.0f, 0, -12.0f), 5.0f);
            testAssert(nearest >= 0, "Vehicles: Spatial Search Finds Nearest Vehicle at POI");
        }

        // 15. Armor Tier 1, 2, 3 Damage Mitigation Scaling Test
        {
            Combat::ArmorComponent armor;
            armor.equipVest(1); // 30%
            armor.equipHelmet(1); // 25%
            testAssert(armor.getVestReduction() == 0.30f, "Armor: Tier 1 Vest Mitigates 30% Damage");
            testAssert(armor.getHelmetReduction() == 0.25f, "Armor: Tier 1 Helmet Mitigates 25% Damage");

            armor.equipVest(2); // 45%
            armor.equipHelmet(2); // 40%
            testAssert(armor.getVestReduction() == 0.45f, "Armor: Tier 2 Vest Mitigates 45% Damage");
            testAssert(armor.getHelmetReduction() == 0.40f, "Armor: Tier 2 Helmet Mitigates 40% Damage");

            armor.equipVest(3); // 65%
            armor.equipHelmet(3); // 55%
            testAssert(armor.getVestReduction() == 0.65f, "Armor: Tier 3 Vest Mitigates 65% Damage");
            testAssert(armor.getHelmetReduction() == 0.55f, "Armor: Tier 3 Helmet Mitigates 55% Damage");
        }

        std::cout << "----------------------------------------\n";
        std::cout << " TESTS SUMMARY: " << passed << " / " << total << " PASSED.\n";
        std::cout << "========================================\n\n";

        return (passed == total);
    }
};

} // namespace Tests

#endif // TESTS_UNITTESTS_HPP
