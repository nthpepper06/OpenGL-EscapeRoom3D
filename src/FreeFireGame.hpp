#ifndef FREEFIRE_GAME_HPP
#define FREEFIRE_GAME_HPP

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "Math3D.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Sound.hpp"

constexpr float MAP_SIZE = 160.0f; // X: -80 to +80, Z: -80 to +80
constexpr float HALF_MAP = MAP_SIZE * 0.5f;

enum class FlightState {
    FREEFALL,
    PARACHUTING,
    LANDED
};

enum class WeaponType {
    NONE,
    AK47,
    MP40,
    M1887,
    AWM
};

struct WeaponStats {
    std::string name;
    float damage;
    float headshotMultiplier;
    float fireRate;
    float spread;
    int magSize;
    int reserveAmmo;
    float reloadTime;
    float range;
    float zoomFOV;
};

inline WeaponStats getWeaponStats(WeaponType type) {
    switch (type) {
        case WeaponType::AK47:
            return {"AK-47 RONG LUA", 36.0f, 4.5f, 0.12f, 0.032f, 30, 120, 2.0f, 130.0f, 42.0f};
        case WeaponType::MP40:
            return {"MP40 TIA CHOP", 22.0f, 4.0f, 0.08f, 0.052f, 32, 160, 1.6f, 80.0f, 45.0f};
        case WeaponType::M1887:
            return {"M1887 NUI LUA", 28.0f, 3.5f, 0.65f, 0.11f, 2, 24, 1.8f, 48.0f, 50.0f};
        case WeaponType::AWM:
            return {"AWM SAT THU", 150.0f, 3.0f, 1.4f, 0.005f, 5, 15, 2.5f, 260.0f, 18.0f};
        case WeaponType::NONE:
        default:
            return {"TAY KHONG", 15.0f, 2.0f, 0.5f, 0.05f, 0, 0, 0.0f, 2.5f, 55.0f};
    }
}

enum class LootType {
    WEAPON_AK47,
    WEAPON_MP40,
    WEAPON_M1887,
    WEAPON_AWM,
    AMMO_RIFLE,
    AMMO_SMG,
    AMMO_SHOTGUN,
    AMMO_SNIPER,
    MEDKIT,
    GLOO_WALL_ITEM,
    ARMOR_VEST
};

struct LootItem {
    int id;
    Vec3 pos;
    LootType type;
    int count;
    std::string name;
    bool isPickedUp = false;
    float rotAngle = 0.0f;
};

struct DamagePopup {
    Vec3 worldPos;
    int damage;
    bool isHeadshot;
    float timer = 0.0f;
    float maxTime = 0.85f;
    float offsetY = 0.0f;
};

struct BulletTracer {
    Vec3 start;
    Vec3 end;
    float life = 0.0f;
    float maxLife = 0.12f;
};

struct KillFeedEntry {
    std::string killer;
    std::string victim;
    std::string weapon;
    bool isHeadshot;
    float timer = 0.0f;
    float maxTime = 4.0f;
};

struct GlooWall {
    Vec3 pos;
    float angle;
    float hp = 350.0f;
    float maxHp = 350.0f;
    float life = 18.0f;
};

struct Airdrop {
    Vec3 pos;
    float targetY = 0.0f;
    bool landed = false;
    bool looted = false;
    float smokeTimer = 0.0f;
};

struct Character {
    int id;
    std::string name;
    bool isBot;
    bool isAlive = true;

    Vec3 pos = Vec3(0, 70.0f, 0);
    Vec3 vel = Vec3(0, 0, 0);
    float yaw = 0.0f;
    float pitch = 0.0f;

    FlightState flightState = FlightState::FREEFALL;

    float hp = 200.0f;
    float maxHp = 200.0f;
    float ep = 50.0f;

    WeaponType currentWeapon = WeaponType::NONE;
    int currentMag = 0;
    int reserveAmmo = 0;
    float shootCooldown = 0.0f;
    float reloadTimer = 0.0f;
    bool isReloading = false;

    int glooWallCount = 2;
    float glooCooldown = 0.0f;
    int medkitCount = 2;
    float medkitUseTimer = 0.0f;
    bool isUsingMedkit = false;

    bool isGrounded = false;
    bool isSprinting = false;
    bool isAiming = false;
    float runAnim = 0.0f;

    Vec3 botTarget = Vec3(0, 0, 0);
    float botActionTimer = 0.0f;

    Character(int _id, const std::string& _name, bool _isBot, const Vec3& _pos)
        : id(_id), name(_name), isBot(_isBot), pos(_pos) {
        currentWeapon = WeaponType::NONE;
        currentMag = 0;
        reserveAmmo = 0;
    }
};

class FreeFireGame {
public:
    // Game State
    bool isGameOver = false;
    bool isBooyah = false;
    float booyahTimer = 0.0f;
    int playerKills = 0;
    int totalDamageDealt = 0;
    bool isPaused = false;
    bool showHelp = false;

    // Safe Zone (Vòng Bo)
    Vec3 safeZoneCenter = Vec3(0, 0, 0);
    float safeZoneRadius = 75.0f;
    Vec3 nextZoneCenter = Vec3(0, 0, 0);
    float nextZoneRadius = 45.0f;
    float safeZoneTimer = 30.0f;
    float safeZoneShrinkDuration = 20.0f;
    bool isZoneShrinking = false;
    int safeZonePhase = 1;

    // Entities
    Character player = Character(0, "YOU", false, Vec3(0, 72.0f, 25.0f));
    std::vector<Character> bots;
    std::vector<LootItem> lootItems;
    std::vector<GlooWall> glooWalls;
    std::vector<Airdrop> airdrops;

    // Visual FX & Combat
    std::vector<DamagePopup> damagePopups;
    std::vector<BulletTracer> tracers;
    std::vector<KillFeedEntry> killFeed;
    float hitmarkerTimer = 0.0f;
    bool hitmarkerIsHeadshot = false;
    float muzzleFlashTimer = 0.0f;
    float screenDamageVignette = 0.0f;
    float recoilPitch = 0.0f;

    // World Static Obstacles
    struct Obstacle {
        Vec3 pos;
        Vec3 size;
        int type; // 0: Building, 1: Watchtower, 2: Tree, 3: Crate
        float angle;
    };
    std::vector<Obstacle> obstacles;

    // Meshes
    Mesh meshTerrain;
    Mesh meshSkyDome;
    Mesh meshShadow;
    Mesh meshBuilding;
    Mesh meshWatchtower;
    Mesh meshTree;
    Mesh meshAirdrop;
    Mesh meshGlooWall;
    Mesh meshCrate;
    Mesh meshCylinder;
    Mesh meshSphere;
    Mesh meshQuad;
    Mesh meshPlayerTorso;
    Mesh meshPlayerHead;
    Mesh meshPlayerLimb;
    Mesh meshAK47;
    Mesh meshMP40;
    Mesh meshM1887;
    Mesh meshAWM;
    Mesh meshParachute;
    Mesh meshMedkit;
    Mesh meshAmmoBox;
    Mesh meshGlooItem;

    // Shaders
    Shader worldShader;
    Shader uiShader;

    // Camera
    Vec3 camPos;
    Vec3 camForward;
    float currentFOV = 55.0f;

    // Nearest loot item indicator
    int nearestLootIdx = -1;

    FreeFireGame() {
        initMatch();
    }

    void initMatch() {
        isGameOver = false;
        isBooyah = false;
        booyahTimer = 0.0f;
        playerKills = 0;
        totalDamageDealt = 0;
        damagePopups.clear();
        tracers.clear();
        killFeed.clear();
        glooWalls.clear();
        airdrops.clear();
        lootItems.clear();

        // Safe Zone setup
        safeZoneCenter = Vec3(0, 0, 0);
        safeZoneRadius = 75.0f;
        nextZoneCenter = Vec3(((rand() % 40) - 20.0f), 0, ((rand() % 40) - 20.0f));
        nextZoneRadius = 45.0f;
        safeZoneTimer = 28.0f;
        isZoneShrinking = false;
        safeZonePhase = 1;

        // Player Reset (Spawn high in the sky for Skydiving!)
        player = Character(0, "YOU", false, Vec3(0, 72.0f, 25.0f));
        player.flightState = FlightState::FREEFALL;
        player.currentWeapon = WeaponType::NONE;
        player.glooWallCount = 2;
        player.medkitCount = 2;

        // Spawn 9 Iconic Free Fire Bots in the Sky
        bots.clear();
        std::vector<std::string> botNames = {
            "Kelly_TheSwift", "Alok_DJ", "Hayato_Bushido", "Chrono_Shield",
            "Moco_Hacker", "Maxim_Gluttony", "Kla_MuayThai", "Dasha_Party", "Wukong_Monkey"
        };

        for (size_t i = 0; i < botNames.size(); ++i) {
            float angle = (float)i / (float)botNames.size() * 2.0f * PI;
            float dist = 28.0f + (rand() % 25);
            Vec3 botSpawn(std::cos(angle) * dist, 70.0f + (rand() % 10), std::sin(angle) * dist);
            Character b((int)(i + 1), botNames[i], true, botSpawn);
            b.flightState = FlightState::FREEFALL;
            b.currentWeapon = WeaponType::NONE;
            b.yaw = (float)(rand() % 360) * DEG2RAD;
            bots.push_back(b);
        }

        // Spawn 45+ Loot Items across the map
        spawnGroundLoot();

        // Spawn Airdrop
        Airdrop ad;
        ad.pos = Vec3(((rand() % 30) - 15.0f), 35.0f, ((rand() % 30) - 15.0f));
        ad.targetY = 0.0f;
        airdrops.push_back(ad);
    }

    void spawnGroundLoot() {
        int lootId = 0;

        auto addLoot = [&](Vec3 p, LootType t, int cnt, const std::string& n) {
            LootItem item;
            item.id = lootId++;
            item.pos = p;
            item.type = t;
            item.count = cnt;
            item.name = n;
            item.isPickedUp = false;
            lootItems.push_back(item);
        };

        // 1. Inside & Around 4 Barracks / Houses
        Vec3 housePositions[4] = {
            Vec3(-35.0f, 0, -35.0f),
            Vec3( 35.0f, 0, -35.0f),
            Vec3(-35.0f, 0,  35.0f),
            Vec3( 35.0f, 0,  35.0f)
        };
        for (int h = 0; h < 4; ++h) {
            Vec3 hp = housePositions[h];
            addLoot(hp + Vec3(-2.5f, 0.4f, -1.5f), LootType::WEAPON_AK47, 30, "AK-47 RONG LUA (+30 DAN)");
            addLoot(hp + Vec3( 2.5f, 0.4f, -1.5f), LootType::WEAPON_MP40, 32, "MP40 TIA CHOP (+32 DAN)");
            addLoot(hp + Vec3( 0.0f, 0.4f, -2.5f), LootType::AMMO_RIFLE, 60, "HOP DAN SUNG TRUONG (x60)");
            addLoot(hp + Vec3(-2.0f, 0.4f,  2.0f), LootType::MEDKIT, 1, "TUI CUU THUONG (+75 HP)");
            addLoot(hp + Vec3( 2.0f, 0.4f,  2.0f), LootType::GLOO_WALL_ITEM, 1, "BOM KEO (+1)");
            addLoot(hp + Vec3( 0.0f, 0.4f,  3.5f), LootType::ARMOR_VEST, 1, "GIAP CHONG DAN (+100 EP)");
        }

        // 2. On Watchtowers (Sniper loot!)
        addLoot(Vec3(0.0f, 7.3f, -42.0f), LootType::WEAPON_AWM, 5, "AWM SAT THU (+5 DAN)");
        addLoot(Vec3(1.2f, 7.3f, -42.0f), LootType::AMMO_SNIPER, 15, "DAN AWM (x15)");
        addLoot(Vec3(0.0f, 7.3f,  42.0f), LootType::WEAPON_AWM, 5, "AWM SAT THU (+5 DAN)");
        addLoot(Vec3(-1.2f, 7.3f, 42.0f), LootType::AMMO_SNIPER, 15, "DAN AWM (x15)");

        // 3. Central Compound & Scattered Fields
        addLoot(Vec3(0, 0.4f, 0), LootType::WEAPON_M1887, 8, "M1887 NUI LUA (+8 DAN)");
        addLoot(Vec3(3.0f, 0.4f, 0), LootType::AMMO_SHOTGUN, 16, "DAN SHOTGUN (x16)");
        addLoot(Vec3(-3.0f, 0.4f, 0), LootType::GLOO_WALL_ITEM, 2, "BOM KEO (x2)");
        addLoot(Vec3(0, 0.4f, 4.0f), LootType::MEDKIT, 2, "TUI CUU THUONG (x2)");

        // 4. Random Loot spots across fields
        for (int i = 0; i < 18; ++i) {
            float rx = ((rand() % 110) - 55.0f);
            float rz = ((rand() % 110) - 55.0f);
            int rndType = rand() % 6;
            if (rndType == 0) addLoot(Vec3(rx, 0.4f, rz), LootType::WEAPON_AK47, 30, "AK-47 RONG LUA (+30 DAN)");
            else if (rndType == 1) addLoot(Vec3(rx, 0.4f, rz), LootType::WEAPON_MP40, 32, "MP40 TIA CHOP (+32 DAN)");
            else if (rndType == 2) addLoot(Vec3(rx, 0.4f, rz), LootType::AMMO_SMG, 60, "DAN SMG (x60)");
            else if (rndType == 3) addLoot(Vec3(rx, 0.4f, rz), LootType::MEDKIT, 1, "TUI CUU THUONG (+75 HP)");
            else if (rndType == 4) addLoot(Vec3(rx, 0.4f, rz), LootType::GLOO_WALL_ITEM, 1, "BOM KEO (+1)");
            else addLoot(Vec3(rx, 0.4f, rz), LootType::ARMOR_VEST, 1, "GIAP CHONG DAN (+100 EP)");
        }
    }

    void initGraphics() {
        worldShader.compile(Shaders::worldVertex, Shaders::worldFragment);
        uiShader.compile(Shaders::uiVertex, Shaders::uiFragment);

        meshTerrain = MeshGen::createPlane(MAP_SIZE, MAP_SIZE, 40, 40);
        meshSkyDome = MeshGen::createSkyDome(320.0f, 14, 28);
        meshShadow = MeshGen::createDropShadow(0.85f);
        meshBuilding = MeshGen::createBuilding(12.0f, 4.5f, 10.0f);
        meshWatchtower = MeshGen::createWatchtower(7.5f, 4.2f);
        meshTree = MeshGen::createTree(7.0f);
        meshAirdrop = MeshGen::createAirdropBox(1.8f);
        meshGlooWall = MeshGen::createGlooWall(3.2f, 2.9f, 0.65f);
        meshCrate = MeshGen::createCube(1.6f, 1.4f, 1.6f, Vec4(0.35f, 0.45f, 0.35f, 1.0f));
        meshCylinder = MeshGen::createCylinder(1.0f, 1.0f, 1.0f, 32);
        meshSphere = MeshGen::createSphere(1.0f, 14, 18);
        meshQuad = MeshGen::createQuad2D(1.0f, 1.0f);

        meshPlayerTorso = MeshGen::createCube(0.85f, 1.05f, 0.45f);
        meshPlayerHead = MeshGen::createSphere(0.35f, 12, 16);
        meshPlayerLimb = MeshGen::createCylinder(0.16f, 0.13f, 0.75f, 10);

        meshAK47 = MeshGen::createDetailedGun(1);
        meshMP40 = MeshGen::createDetailedGun(2);
        meshM1887 = MeshGen::createDetailedGun(3);
        meshAWM = MeshGen::createDetailedGun(4);

        meshParachute = MeshGen::createParachute(4.2f, 2.4f, 24);
        meshMedkit = MeshGen::createMedkit(0.8f);
        meshAmmoBox = MeshGen::createAmmoBox(Vec4(0.95f, 0.75f, 0.15f, 1.0f));
        meshGlooItem = MeshGen::createGlooItem();

        buildMapObstacles();
    }

    void buildMapObstacles() {
        obstacles.clear();
        // 4 Barracks Buildings with open doorways
        obstacles.push_back({Vec3(-35.0f, 0, -35.0f), Vec3(12.0f, 4.5f, 10.0f), 0, 0.0f});
        obstacles.push_back({Vec3( 35.0f, 0, -35.0f), Vec3(12.0f, 4.5f, 10.0f), 0, 45.0f * DEG2RAD});
        obstacles.push_back({Vec3(-35.0f, 0,  35.0f), Vec3(12.0f, 4.5f, 10.0f), 0, -30.0f * DEG2RAD});
        obstacles.push_back({Vec3( 35.0f, 0,  35.0f), Vec3(12.0f, 4.5f, 10.0f), 0, 90.0f * DEG2RAD});

        // 2 Watchtowers
        obstacles.push_back({Vec3(0.0f, 0, -42.0f), Vec3(4.2f, 7.5f, 4.2f), 1, 0.0f});
        obstacles.push_back({Vec3(0.0f, 0,  42.0f), Vec3(4.2f, 7.5f, 4.2f), 1, 0.0f});

        // 28 Natural Trees
        for (int i = 0; i < 28; ++i) {
            float x = ((rand() % 130) - 65.0f);
            float z = ((rand() % 130) - 65.0f);
            if (std::abs(x) < 9.0f && std::abs(z) < 9.0f) continue;
            obstacles.push_back({Vec3(x, 0, z), Vec3(2.5f, 7.0f, 2.5f), 2, (float)(rand() % 360) * DEG2RAD});
        }

        // 16 Tactical Crates
        for (int i = 0; i < 16; ++i) {
            float x = ((rand() % 110) - 55.0f);
            float z = ((rand() % 110) - 55.0f);
            obstacles.push_back({Vec3(x, 0, z), Vec3(1.6f, 1.4f, 1.6f), 3, (float)(rand() % 360) * DEG2RAD});
        }
    }

    // Resolves rigid body collisions against all buildings, watchtowers, crates, trees, and Gloo walls
    void resolveWorldCollisions(Vec3& pos, float radius = 0.55f) {
        // 1. Boundary
        pos.x = std::max(-HALF_MAP + 1.5f, std::min(HALF_MAP - 1.5f, pos.x));
        pos.z = std::max(-HALF_MAP + 1.5f, std::min(HALF_MAP - 1.5f, pos.z));

        // 2. Static Obstacles
        for (const auto& obs : obstacles) {
            float cosA = std::cos(-obs.angle);
            float sinA = std::sin(-obs.angle);
            float dx = pos.x - obs.pos.x;
            float dz = pos.z - obs.pos.z;
            Vec3 loc(dx * cosA - dz * sinA, pos.y - obs.pos.y, dx * sinA + dz * cosA);

            bool hit = false;

            if (obs.type == 0) {
                // Building (Solid 5 Sub-walls leaving a 2.6m doorway)
                float hw = obs.size.x * 0.5f; // 6.0m
                float hd = obs.size.z * 0.5f; // 5.0m
                float wallT = 0.5f;
                float doorW = 2.6f;
                float sideW = (obs.size.x - doorW) * 0.5f;

                AABB backWall(Vec3(-hw, 0, -hd - wallT*0.5f), Vec3(hw, obs.size.y, -hd + wallT*0.5f));
                AABB leftWall(Vec3(-hw - wallT*0.5f, 0, -hd), Vec3(-hw + wallT*0.5f, obs.size.y, hd));
                AABB rightWall(Vec3(hw - wallT*0.5f, 0, -hd), Vec3(hw + wallT*0.5f, obs.size.y, hd));
                AABB frontLeft(Vec3(-hw, 0, hd - wallT*0.5f), Vec3(-hw + sideW, obs.size.y, hd + wallT*0.5f));
                AABB frontRight(Vec3(hw - sideW, 0, hd - wallT*0.5f), Vec3(hw, obs.size.y, hd + wallT*0.5f));

                hit |= backWall.resolveCircle(loc, radius);
                hit |= leftWall.resolveCircle(loc, radius);
                hit |= rightWall.resolveCircle(loc, radius);
                hit |= frontLeft.resolveCircle(loc, radius);
                hit |= frontRight.resolveCircle(loc, radius);
            } else if (obs.type == 1) {
                // Watchtower 4 Legs
                float poleR = 0.35f;
                float hDeck = 1.9f;
                AABB leg1(Vec3(-hDeck - poleR, 0, -hDeck - poleR), Vec3(-hDeck + poleR, 7.5f, -hDeck + poleR));
                AABB leg2(Vec3( hDeck - poleR, 0, -hDeck - poleR), Vec3( hDeck + poleR, 7.5f, -hDeck + poleR));
                AABB leg3(Vec3(-hDeck - poleR, 0,  hDeck - poleR), Vec3(-hDeck + poleR, 7.5f,  hDeck + poleR));
                AABB leg4(Vec3( hDeck - poleR, 0,  hDeck - poleR), Vec3( hDeck + poleR, 7.5f,  hDeck + poleR));

                hit |= leg1.resolveCircle(loc, radius);
                hit |= leg2.resolveCircle(loc, radius);
                hit |= leg3.resolveCircle(loc, radius);
                hit |= leg4.resolveCircle(loc, radius);
            } else if (obs.type == 2) {
                // Tree trunk
                AABB treeTrunk(Vec3(-0.55f, 0, -0.55f), Vec3(0.55f, 5.0f, 0.55f));
                hit |= treeTrunk.resolveCircle(loc, radius);
            } else if (obs.type == 3) {
                // Tactical crate
                AABB crateBox(Vec3(-0.8f, 0, -0.8f), Vec3(0.8f, 1.4f, 0.8f));
                hit |= crateBox.resolveCircle(loc, radius);
            }

            if (hit) {
                float cosW = std::cos(obs.angle);
                float sinW = std::sin(obs.angle);
                pos.x = obs.pos.x + loc.x * cosW - loc.z * sinW;
                pos.z = obs.pos.z + loc.x * sinW + loc.z * cosW;
            }
        }

        // 3. Gloo Walls (Solid Ice Arc Bunker)
        for (const auto& gw : glooWalls) {
            if (pos.y < 0.0f || pos.y > 3.0f) continue;
            float cosG = std::cos(-gw.angle);
            float sinG = std::sin(-gw.angle);
            float dx = pos.x - gw.pos.x;
            float dz = pos.z - gw.pos.z;
            float lx = dx * cosG - dz * sinG;
            float lz = dx * sinG + dz * cosG;

            float r = std::sqrt(lx * lx + lz * lz);
            float phi = std::atan2(lx, lz); // Arc angle relative to forward direction
            float halfArc = (135.0f * 0.5f + 10.0f) * DEG2RAD;

            if (std::abs(phi) <= halfArc && r > 0.01f) {
                float wallR = 3.2f;
                float halfThick = 0.35f;
                float minR = wallR - halfThick - radius;
                float maxR = wallR + halfThick + radius;

                if (r >= minR && r <= maxR) {
                    float targetR = (r >= wallR) ? maxR : minR;
                    lx = std::sin(phi) * targetR;
                    lz = std::cos(phi) * targetR;

                    float cosW = std::cos(gw.angle);
                    float sinW = std::sin(gw.angle);
                    pos.x = gw.pos.x + lx * cosW - lz * sinW;
                    pos.z = gw.pos.z + lx * sinW + lz * cosW;
                }
            }
        }
    }

    void deployParachute(Character& c) {
        if (c.flightState == FlightState::FREEFALL) {
            c.flightState = FlightState::PARACHUTING;
            if (!c.isBot) {
                SoundManager::instance().playParachuteOpen();
            }
        }
    }

    void deployGlooWall(Character& user) {
        if (user.flightState != FlightState::LANDED) return;
        if (user.glooWallCount <= 0 || user.glooCooldown > 0.0f) return;

        user.glooWallCount--;
        user.glooCooldown = 0.8f;

        Vec3 forward(std::sin(user.yaw), 0, -std::cos(user.yaw));
        Vec3 wallPos = user.pos + forward * 2.8f;
        wallPos.y = 0.0f;

        GlooWall gw;
        gw.pos = wallPos;
        gw.angle = user.yaw;
        glooWalls.push_back(gw);

        SoundManager::instance().playGlooWall();
    }

    void useMedkit(Character& c) {
        if (c.medkitCount <= 0 || c.hp >= c.maxHp || c.isUsingMedkit) return;
        c.isUsingMedkit = true;
        c.medkitUseTimer = 1.6f;
        if (!c.isBot) {
            SoundManager::instance().playMedkit();
        }
    }

    void pickupNearestLoot(Character& c) {
        if (c.flightState != FlightState::LANDED) return;

        float minDist = 2.8f;
        int bestIdx = -1;

        for (size_t i = 0; i < lootItems.size(); ++i) {
            if (lootItems[i].isPickedUp) continue;
            float d = (c.pos - lootItems[i].pos).length();
            if (d < minDist) {
                minDist = d;
                bestIdx = (int)i;
            }
        }

        if (bestIdx != -1) {
            LootItem& item = lootItems[bestIdx];
            item.isPickedUp = true;

            if (item.type == LootType::WEAPON_AK47) {
                c.currentWeapon = WeaponType::AK47;
                WeaponStats st = getWeaponStats(c.currentWeapon);
                c.currentMag = st.magSize;
                c.reserveAmmo = std::max(c.reserveAmmo, 90);
            } else if (item.type == LootType::WEAPON_MP40) {
                c.currentWeapon = WeaponType::MP40;
                WeaponStats st = getWeaponStats(c.currentWeapon);
                c.currentMag = st.magSize;
                c.reserveAmmo = std::max(c.reserveAmmo, 120);
            } else if (item.type == LootType::WEAPON_M1887) {
                c.currentWeapon = WeaponType::M1887;
                WeaponStats st = getWeaponStats(c.currentWeapon);
                c.currentMag = st.magSize;
                c.reserveAmmo = std::max(c.reserveAmmo, 20);
            } else if (item.type == LootType::WEAPON_AWM) {
                c.currentWeapon = WeaponType::AWM;
                WeaponStats st = getWeaponStats(c.currentWeapon);
                c.currentMag = st.magSize;
                c.reserveAmmo = std::max(c.reserveAmmo, 15);
            } else if (item.type == LootType::AMMO_RIFLE || item.type == LootType::AMMO_SMG || item.type == LootType::AMMO_SHOTGUN || item.type == LootType::AMMO_SNIPER) {
                c.reserveAmmo += item.count;
            } else if (item.type == LootType::MEDKIT) {
                c.medkitCount += item.count;
            } else if (item.type == LootType::GLOO_WALL_ITEM) {
                c.glooWallCount += item.count;
            } else if (item.type == LootType::ARMOR_VEST) {
                c.ep = 100.0f;
            }

            if (!c.isBot) {
                SoundManager::instance().playPickup();
            }
        }
    }

    void switchWeapon(int slot) {
        if (slot == 1) player.currentWeapon = WeaponType::AK47;
        else if (slot == 2) player.currentWeapon = WeaponType::MP40;
        else if (slot == 3) player.currentWeapon = WeaponType::M1887;
        else if (slot == 4) player.currentWeapon = WeaponType::AWM;

        WeaponStats st = getWeaponStats(player.currentWeapon);
        if (player.currentMag == 0 && player.reserveAmmo > 0) {
            player.currentMag = std::min(st.magSize, player.reserveAmmo);
            player.reserveAmmo -= player.currentMag;
        }
        player.isReloading = false;
        player.reloadTimer = 0.0f;
    }

    void reloadWeapon(Character& c) {
        if (c.isReloading || c.currentWeapon == WeaponType::NONE) return;
        WeaponStats st = getWeaponStats(c.currentWeapon);
        if (c.currentMag >= st.magSize || c.reserveAmmo <= 0) return;

        c.isReloading = true;
        c.reloadTimer = st.reloadTime;
        if (!c.isBot) {
            SoundManager::instance().playReload();
        }
    }

    void fireWeapon(Character& shooter) {
        if (shooter.flightState != FlightState::LANDED) return;
        if (shooter.currentWeapon == WeaponType::NONE) return;
        if (shooter.shootCooldown > 0.0f || shooter.isReloading) return;
        if (shooter.currentMag <= 0) {
            reloadWeapon(shooter);
            return;
        }

        WeaponStats stats = getWeaponStats(shooter.currentWeapon);
        shooter.currentMag--;
        shooter.shootCooldown = stats.fireRate;

        if (!shooter.isBot) {
            muzzleFlashTimer = 0.06f;
            recoilPitch += (shooter.isAiming ? 0.015f : 0.035f);

            if (shooter.currentWeapon == WeaponType::AK47) SoundManager::instance().playAK47();
            else if (shooter.currentWeapon == WeaponType::MP40) SoundManager::instance().playMP40();
            else if (shooter.currentWeapon == WeaponType::M1887) SoundManager::instance().playShotgun();
            else if (shooter.currentWeapon == WeaponType::AWM) SoundManager::instance().playAWM();
        }

        int pellets = (shooter.currentWeapon == WeaponType::M1887) ? 6 : 1;

        for (int p = 0; p < pellets; ++p) {
            float spreadX = ((rand() % 100) / 50.0f - 1.0f) * (shooter.isAiming ? stats.spread * 0.4f : stats.spread);
            float spreadY = ((rand() % 100) / 50.0f - 1.0f) * (shooter.isAiming ? stats.spread * 0.4f : stats.spread);

            Vec3 shootDir;
            Vec3 shootOrigin;

            if (!shooter.isBot) {
                shootOrigin = camPos;
                shootDir = (camForward + Vec3(spreadX, spreadY, 0)).normalized();
            } else {
                shootOrigin = shooter.pos + Vec3(0, 1.4f, 0);
                Vec3 fwd(std::sin(shooter.yaw), shooter.pitch, -std::cos(shooter.yaw));
                shootDir = (fwd + Vec3(spreadX, spreadY, spreadX)).normalized();
            }

            Ray ray(shootOrigin, shootDir);
            float closestT = stats.range;
            int hitTargetId = -1;
            bool hitHead = false;
            bool hitGloo = false;
            int hitGlooIdx = -1;

            // 1. Ray vs Gloo Walls (100% Solid Ice Intercept)
            for (size_t i = 0; i < glooWalls.size(); ++i) {
                float gwAngle = glooWalls[i].angle;
                float cosG = std::cos(-gwAngle);
                float sinG = std::sin(-gwAngle);

                Vec3 ro = ray.origin - glooWalls[i].pos;
                Vec3 locO(ro.x * cosG - ro.z * sinG, ro.y, ro.x * sinG + ro.z * cosG);
                Vec3 locD(ray.dir.x * cosG - ray.dir.z * sinG, ray.dir.y, ray.dir.x * sinG + ray.dir.z * cosG);
                Ray locRay(locO, locD);

                for (int k = -2; k <= 2; ++k) {
                    float a = (float)k * 27.0f * DEG2RAD;
                    Vec3 segCenter(std::sin(a) * 3.2f, 1.45f, std::cos(a) * 3.2f);
                    AABB segBox(segCenter - Vec3(0.9f, 1.45f, 0.45f), segCenter + Vec3(0.9f, 1.45f, 0.45f));
                    float t;
                    if (segBox.intersectRay(locRay, t) && t < closestT && t > 0.1f) {
                        closestT = t;
                        hitGloo = true;
                        hitGlooIdx = (int)i;
                        hitTargetId = -2;
                    }
                }
            }

            // 2. Ray vs Solid Obstacles (Buildings, Watchtowers, Crates, Trees)
            for (const auto& obs : obstacles) {
                float cosA = std::cos(-obs.angle);
                float sinA = std::sin(-obs.angle);
                Vec3 ro = ray.origin - obs.pos;
                Vec3 locO(ro.x * cosA - ro.z * sinA, ro.y, ro.x * sinA + ro.z * cosA);
                Vec3 locD(ray.dir.x * cosA - ray.dir.z * sinA, ray.dir.y, ray.dir.x * sinA + ray.dir.z * cosA);
                Ray locRay(locO, locD);

                if (obs.type == 0) {
                    // Building 5 sub-walls
                    float hw = obs.size.x * 0.5f, hd = obs.size.z * 0.5f;
                    float wallT = 0.5f, doorW = 2.6f, sideW = (obs.size.x - doorW) * 0.5f;
                    AABB bw(Vec3(-hw, 0, -hd - wallT*0.5f), Vec3(hw, obs.size.y, -hd + wallT*0.5f));
                    AABB lw(Vec3(-hw - wallT*0.5f, 0, -hd), Vec3(-hw + wallT*0.5f, obs.size.y, hd));
                    AABB rw(Vec3(hw - wallT*0.5f, 0, -hd), Vec3(hw + wallT*0.5f, obs.size.y, hd));
                    AABB fl(Vec3(-hw, 0, hd - wallT*0.5f), Vec3(-hw + sideW, obs.size.y, hd + wallT*0.5f));
                    AABB fr(Vec3(hw - sideW, 0, hd - wallT*0.5f), Vec3(hw, obs.size.y, hd + wallT*0.5f));

                    float t;
                    if (bw.intersectRay(locRay, t) && t < closestT && t > 0.1f) { closestT = t; hitTargetId = -3; hitGloo = false; }
                    if (lw.intersectRay(locRay, t) && t < closestT && t > 0.1f) { closestT = t; hitTargetId = -3; hitGloo = false; }
                    if (rw.intersectRay(locRay, t) && t < closestT && t > 0.1f) { closestT = t; hitTargetId = -3; hitGloo = false; }
                    if (fl.intersectRay(locRay, t) && t < closestT && t > 0.1f) { closestT = t; hitTargetId = -3; hitGloo = false; }
                    if (fr.intersectRay(locRay, t) && t < closestT && t > 0.1f) { closestT = t; hitTargetId = -3; hitGloo = false; }
                } else if (obs.type == 1) {
                    float poleR = 0.35f, hDeck = 1.9f;
                    AABB l1(Vec3(-hDeck-poleR, 0, -hDeck-poleR), Vec3(-hDeck+poleR, 7.5f, -hDeck+poleR));
                    AABB l2(Vec3( hDeck-poleR, 0, -hDeck-poleR), Vec3( hDeck+poleR, 7.5f, -hDeck+poleR));
                    AABB l3(Vec3(-hDeck-poleR, 0,  hDeck-poleR), Vec3(-hDeck+poleR, 7.5f,  hDeck+poleR));
                    AABB l4(Vec3( hDeck-poleR, 0,  hDeck-poleR), Vec3( hDeck+poleR, 7.5f,  hDeck+poleR));
                    AABB deck(Vec3(-2.3f, 7.0f, -2.3f), Vec3(2.3f, 7.5f, 2.3f));
                    float t;
                    if (l1.intersectRay(locRay, t) && t < closestT && t > 0.1f) { closestT = t; hitTargetId = -3; hitGloo = false; }
                    if (l2.intersectRay(locRay, t) && t < closestT && t > 0.1f) { closestT = t; hitTargetId = -3; hitGloo = false; }
                    if (l3.intersectRay(locRay, t) && t < closestT && t > 0.1f) { closestT = t; hitTargetId = -3; hitGloo = false; }
                    if (l4.intersectRay(locRay, t) && t < closestT && t > 0.1f) { closestT = t; hitTargetId = -3; hitGloo = false; }
                    if (deck.intersectRay(locRay, t) && t < closestT && t > 0.1f) { closestT = t; hitTargetId = -3; hitGloo = false; }
                } else if (obs.type == 2) {
                    AABB tr(Vec3(-0.55f, 0, -0.55f), Vec3(0.55f, 5.0f, 0.55f));
                    float t;
                    if (tr.intersectRay(locRay, t) && t < closestT && t > 0.1f) { closestT = t; hitTargetId = -3; hitGloo = false; }
                } else if (obs.type == 3) {
                    AABB cr(Vec3(-0.8f, 0, -0.8f), Vec3(0.8f, 1.4f, 0.8f));
                    float t;
                    if (cr.intersectRay(locRay, t) && t < closestT && t > 0.1f) { closestT = t; hitTargetId = -3; hitGloo = false; }
                }
            }

            // 3. Ray vs Characters
            auto checkHitChar = [&](Character& target) {
                if (!target.isAlive || target.id == shooter.id || target.flightState != FlightState::LANDED) return;

                Vec3 headCenter = target.pos + Vec3(0, 2.1f, 0);
                float tHead;
                if (intersectSphere(ray, headCenter, 0.42f, tHead) && tHead < closestT && tHead > 0.2f) {
                    closestT = tHead;
                    hitTargetId = target.id;
                    hitHead = true;
                    hitGloo = false;
                }

                AABB bodyBox(target.pos + Vec3(-0.55f, 0, -0.55f), target.pos + Vec3(0.55f, 1.8f, 0.55f));
                float tBody;
                if (bodyBox.intersectRay(ray, tBody) && tBody < closestT && tBody > 0.2f) {
                    closestT = tBody;
                    hitTargetId = target.id;
                    hitHead = false;
                    hitGloo = false;
                }
            };

            if (shooter.isBot) {
                checkHitChar(player);
            } else {
                for (auto& b : bots) checkHitChar(b);
            }

            Vec3 hitPoint = ray.origin + ray.dir * closestT;

            if (hitGloo && hitGlooIdx >= 0 && hitGlooIdx < (int)glooWalls.size()) {
                glooWalls[hitGlooIdx].hp -= stats.damage;
            } else if (hitTargetId >= 0) {
                float finalDmg = stats.damage * (hitHead ? stats.headshotMultiplier : 1.0f);
                int dmgInt = (int)finalDmg;

                Character* targetChar = (hitTargetId == 0) ? &player : nullptr;
                if (!targetChar) {
                    for (auto& b : bots) {
                        if (b.id == hitTargetId) { targetChar = &b; break; }
                    }
                }

                if (targetChar && targetChar->isAlive) {
                    targetChar->hp -= finalDmg;

                    DamagePopup dp;
                    dp.worldPos = targetChar->pos + Vec3(0, 2.2f, 0);
                    dp.damage = dmgInt;
                    dp.isHeadshot = hitHead;
                    damagePopups.push_back(dp);

                    if (!shooter.isBot) {
                        hitmarkerTimer = 0.15f;
                        hitmarkerIsHeadshot = hitHead;
                        totalDamageDealt += dmgInt;
                        if (hitHead) SoundManager::instance().playHeadshot();
                        else SoundManager::instance().playHitmarker();
                    }

                    if (targetChar->hp <= 0.0f) {
                        targetChar->hp = 0.0f;
                        targetChar->isAlive = false;

                        if (!shooter.isBot) {
                            playerKills++;
                        }

                        KillFeedEntry kfe;
                        kfe.killer = shooter.name;
                        kfe.victim = targetChar->name;
                        kfe.weapon = stats.name;
                        kfe.isHeadshot = hitHead;
                        killFeed.push_back(kfe);

                        checkVictoryCondition();
                    }
                }
            }

            BulletTracer bt;
            bt.start = shooter.pos + Vec3(0, 1.3f, 0);
            bt.end = hitPoint;
            tracers.push_back(bt);
        }
    }

    void checkVictoryCondition() {
        int aliveCount = (player.isAlive ? 1 : 0);
        for (const auto& b : bots) {
            if (b.isAlive) aliveCount++;
        }

        if (aliveCount <= 1 && player.isAlive && !isBooyah) {
            isBooyah = true;
            booyahTimer = 4.0f;
            SoundManager::instance().playBooyah();
        } else if (!player.isAlive) {
            isGameOver = true;
        }
    }

    void update(float dt) {
        if (isPaused) return;

        if (screenDamageVignette > 0.0f) screenDamageVignette -= dt * 2.0f;
        if (hitmarkerTimer > 0.0f) hitmarkerTimer -= dt;
        if (muzzleFlashTimer > 0.0f) muzzleFlashTimer -= dt;

        if (recoilPitch > 0.0f) {
            float recDecay = dt * 0.15f;
            recoilPitch -= recDecay;
            if (recoilPitch < 0.0f) recoilPitch = 0.0f;
        }

        // Rotate Ground Loot
        for (auto& item : lootItems) {
            item.rotAngle += dt * 2.5f;
        }

        // Check nearest loot for player HUD
        nearestLootIdx = -1;
        float minDist = 2.8f;
        for (size_t i = 0; i < lootItems.size(); ++i) {
            if (lootItems[i].isPickedUp) continue;
            float d = (player.pos - lootItems[i].pos).length();
            if (d < minDist) {
                minDist = d;
                nearestLootIdx = (int)i;
            }
        }

        updateSafeZone(dt);
        updatePlayer(dt);
        updateBots(dt);

        // Gloo walls update
        for (size_t i = 0; i < glooWalls.size(); ) {
            glooWalls[i].life -= dt;
            if (glooWalls[i].life <= 0.0f || glooWalls[i].hp <= 0.0f) {
                glooWalls[i] = glooWalls.back();
                glooWalls.pop_back();
            } else {
                ++i;
            }
        }

        // Airdrops update
        for (auto& ad : airdrops) {
            if (!ad.landed) {
                ad.pos.y -= 4.5f * dt;
                if (ad.pos.y <= ad.targetY) {
                    ad.pos.y = ad.targetY;
                    ad.landed = true;
                }
            } else {
                ad.smokeTimer += dt;
            }

            if (ad.landed && !ad.looted && (player.pos - ad.pos).length() < 3.2f) {
                ad.looted = true;
                player.currentWeapon = WeaponType::AWM;
                WeaponStats st = getWeaponStats(player.currentWeapon);
                player.currentMag = st.magSize;
                player.reserveAmmo = st.reserveAmmo;
                player.hp = 200.0f;
                player.ep = 100.0f;
                player.glooWallCount += 3;
                player.medkitCount += 2;
                SoundManager::instance().playPickup();
            }
        }

        // Damage Popups update
        for (size_t i = 0; i < damagePopups.size(); ) {
            damagePopups[i].timer += dt;
            damagePopups[i].offsetY += dt * 1.8f;
            if (damagePopups[i].timer >= damagePopups[i].maxTime) {
                damagePopups[i] = damagePopups.back();
                damagePopups.pop_back();
            } else {
                ++i;
            }
        }

        // Bullet Tracers update
        for (size_t i = 0; i < tracers.size(); ) {
            tracers[i].life += dt;
            if (tracers[i].life >= tracers[i].maxLife) {
                tracers[i] = tracers.back();
                tracers.pop_back();
            } else {
                ++i;
            }
        }

        // Kill Feed update
        for (size_t i = 0; i < killFeed.size(); ) {
            killFeed[i].timer += dt;
            if (killFeed[i].timer >= killFeed[i].maxTime) {
                killFeed[i] = killFeed.back();
                killFeed.pop_back();
            } else {
                ++i;
            }
        }
    }

    void updateSafeZone(float dt) {
        safeZoneTimer -= dt;

        if (safeZoneTimer <= 0.0f) {
            if (!isZoneShrinking) {
                isZoneShrinking = true;
                safeZoneTimer = safeZoneShrinkDuration;
                SoundManager::instance().playSafeZoneAlarm();
            } else {
                isZoneShrinking = false;
                safeZoneRadius = nextZoneRadius;
                safeZoneCenter = nextZoneCenter;
                safeZonePhase++;

                nextZoneRadius = std::max(6.0f, safeZoneRadius * 0.55f);
                nextZoneCenter = safeZoneCenter + Vec3(
                    ((rand() % 100) / 50.0f - 1.0f) * (safeZoneRadius - nextZoneRadius) * 0.6f,
                    0,
                    ((rand() % 100) / 50.0f - 1.0f) * (safeZoneRadius - nextZoneRadius) * 0.6f
                );
                safeZoneTimer = 22.0f;
            }
        }

        if (isZoneShrinking) {
            float t = dt / safeZoneShrinkDuration;
            safeZoneRadius = lerp(Vec3(safeZoneRadius,0,0), Vec3(nextZoneRadius,0,0), t).x;
            safeZoneCenter = lerp(safeZoneCenter, nextZoneCenter, t);
        }

        auto applyZoneDmg = [&](Character& c) {
            if (!c.isAlive || c.flightState != FlightState::LANDED) return;
            float distFromCenter = (Vec3(c.pos.x, 0, c.pos.z) - Vec3(safeZoneCenter.x, 0, safeZoneCenter.z)).length();
            if (distFromCenter > safeZoneRadius) {
                c.hp -= 6.0f * dt;
                if (&c == &player) screenDamageVignette = 0.5f;
                if (c.hp <= 0.0f) {
                    c.hp = 0.0f;
                    c.isAlive = false;
                    KillFeedEntry kfe = {"SAFE_ZONE", c.name, "ELECTRIC_ZONE", false, 0.0f, 4.0f};
                    killFeed.push_back(kfe);
                    checkVictoryCondition();
                }
            }
        };

        applyZoneDmg(player);
        for (auto& b : bots) applyZoneDmg(b);
    }

    void updatePlayer(float dt) {
        if (!player.isAlive) return;

        // Skydiving & Parachuting Descent Physics
        if (player.flightState == FlightState::FREEFALL) {
            player.pos.y -= 19.5f * dt;
            if (player.pos.y <= 24.0f) {
                deployParachute(player);
            }
        } else if (player.flightState == FlightState::PARACHUTING) {
            player.pos.y -= 4.2f * dt;
            if (player.pos.y <= 0.0f) {
                player.pos.y = 0.0f;
                player.flightState = FlightState::LANDED;
                player.isGrounded = true;
                SoundManager::instance().playLand();
            }
        } else {
            // Rigid body collision pushout against walls, watchtowers, crates, trees, Gloo walls
            resolveWorldCollisions(player.pos, 0.55f);
        }

        // Medkit healing timer
        if (player.isUsingMedkit) {
            player.medkitUseTimer -= dt;
            if (player.medkitUseTimer <= 0.0f) {
                player.isUsingMedkit = false;
                player.medkitCount--;
                player.hp = std::min(player.maxHp, player.hp + 75.0f);
            }
        }

        // Reload timer
        if (player.isReloading) {
            player.reloadTimer -= dt;
            if (player.reloadTimer <= 0.0f) {
                player.isReloading = false;
                WeaponStats st = getWeaponStats(player.currentWeapon);
                int needed = st.magSize - player.currentMag;
                int toLoad = std::min(needed, player.reserveAmmo);
                player.currentMag += toLoad;
                player.reserveAmmo -= toLoad;
            }
        }

        if (player.shootCooldown > 0.0f) player.shootCooldown -= dt;
        if (player.glooCooldown > 0.0f) player.glooCooldown -= dt;

        WeaponStats st = getWeaponStats(player.currentWeapon);
        float targetFOV = player.isAiming ? st.zoomFOV : (player.flightState != FlightState::LANDED ? 65.0f : 55.0f);
        currentFOV = lerp(Vec3(currentFOV,0,0), Vec3(targetFOV,0,0), dt * 14.0f).x;

        // TPS Camera Positioning
        float camDist = (player.flightState != FlightState::LANDED) ? 6.5f : (player.isAiming ? 2.4f : 4.5f);
        float camHeight = (player.flightState != FlightState::LANDED) ? 2.5f : (player.isAiming ? 1.65f : 2.1f);
        float camShoulder = (player.flightState != FlightState::LANDED) ? 0.0f : (player.isAiming ? 0.75f : 0.6f);

        float effectivePitch = player.pitch + recoilPitch;
        float cosP = std::cos(effectivePitch);
        float sinP = std::sin(effectivePitch);
        float cosY = std::cos(player.yaw);
        float sinY = std::sin(player.yaw);

        Vec3 fwd(sinY * cosP, sinP, -cosY * cosP);
        Vec3 right(cosY, 0, sinY);
        camForward = fwd.normalized();

        camPos = player.pos + Vec3(0, camHeight, 0) - camForward * camDist + right * camShoulder;
    }

    void updateBots(float dt) {
        for (auto& b : bots) {
            if (!b.isAlive) continue;

            // Bot Skydiving Physics
            if (b.flightState == FlightState::FREEFALL) {
                b.pos.y -= 19.5f * dt;
                Vec3 fwd(std::sin(b.yaw), 0, -std::cos(b.yaw));
                b.pos += fwd * (12.0f * dt);
                if (b.pos.y <= 24.0f) {
                    b.flightState = FlightState::PARACHUTING;
                }
                continue;
            } else if (b.flightState == FlightState::PARACHUTING) {
                b.pos.y -= 4.2f * dt;
                Vec3 fwd(std::sin(b.yaw), 0, -std::cos(b.yaw));
                b.pos += fwd * (7.0f * dt);
                if (b.pos.y <= 0.0f) {
                    b.pos.y = 0.0f;
                    b.flightState = FlightState::LANDED;
                }
                continue;
            }

            if (b.shootCooldown > 0.0f) b.shootCooldown -= dt;
            if (b.botActionTimer > 0.0f) b.botActionTimer -= dt;

            // Bot decision cycle
            if (b.botActionTimer <= 0.0f) {
                b.botActionTimer = 1.2f + (rand() % 100) / 80.0f;

                // Priority 1: If has no weapon, seek nearest loot item!
                if (b.currentWeapon == WeaponType::NONE) {
                    float closestLootD = 999.0f;
                    Vec3 targetLootPos = b.pos;
                    for (const auto& li : lootItems) {
                        if (li.isPickedUp) continue;
                        float d = (b.pos - li.pos).length();
                        if (d < closestLootD) {
                            closestLootD = d;
                            targetLootPos = li.pos;
                        }
                    }
                    b.botTarget = targetLootPos;
                } else {
                    float distToZone = (Vec3(b.pos.x, 0, b.pos.z) - Vec3(safeZoneCenter.x, 0, safeZoneCenter.z)).length();
                    if (distToZone > safeZoneRadius * 0.8f) {
                        b.botTarget = safeZoneCenter + Vec3(((rand() % 30) - 15.0f), 0, ((rand() % 30) - 15.0f));
                    } else {
                        float distToPlayer = (player.pos - b.pos).length();
                        if (player.isAlive && distToPlayer < 45.0f && player.flightState == FlightState::LANDED) {
                            b.botTarget = player.pos + Vec3(((rand() % 16) - 8.0f), 0, ((rand() % 16) - 8.0f));
                        } else {
                            b.botTarget = b.pos + Vec3(((rand() % 40) - 20.0f), 0, ((rand() % 40) - 20.0f));
                        }
                    }
                }
            }

            // Move towards bot target
            Vec3 toTarget = b.botTarget - b.pos;
            toTarget.y = 0;
            float dist = toTarget.length();

            if (dist > 1.2f) {
                Vec3 moveDir = toTarget.normalized();
                b.vel = moveDir * 9.5f;
                b.yaw = std::atan2(moveDir.x, -moveDir.z);
                b.pos += b.vel * dt;
                b.runAnim += dt * 12.0f;
            } else {
                b.vel = Vec3(0, 0, 0);
                pickupNearestLoot(b);
            }

            // Rigid body collision pushout for Bots
            resolveWorldCollisions(b.pos, 0.55f);

            // Combat engagement
            if (b.currentWeapon != WeaponType::NONE && player.isAlive && player.flightState == FlightState::LANDED && (player.pos - b.pos).length() < 38.0f) {
                Vec3 toPlayer = player.pos - b.pos;
                b.yaw = std::atan2(toPlayer.x, -toPlayer.z);
                b.pitch = std::asin(std::max(-0.8f, std::min(0.8f, (player.pos.y - b.pos.y) / toPlayer.length())));

                if (b.hp < 75.0f && b.glooWallCount > 0 && rand() % 100 < 30) {
                    deployGlooWall(b);
                }

                if (b.shootCooldown <= 0.0f) {
                    fireWeapon(b);
                }
            }
        }
    }

    void render(int screenW, int screenH) {
        glViewport(0, 0, screenW, screenH);
        glClearColor(0.22f, 0.45f, 0.75f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float aspect = (float)screenW / (float)screenH;
        Mat4 projection = Mat4::perspective(currentFOV * DEG2RAD, aspect, 0.2f, 400.0f);
        Mat4 view = Mat4::lookAt(camPos, camPos + camForward, Vec3(0, 1, 0));

        worldShader.use();
        worldShader.setMat4("uProjection", projection);
        worldShader.setMat4("uView", view);
        worldShader.setVec3("uViewPos", camPos);
        worldShader.setFloat("uTime", (float)glfwGetTime());

        // 1. Render Sky Dome Atmosphere (Shading type 7)
        glDisable(GL_CULL_FACE);
        worldShader.setInt("uShadingType", 7);
        worldShader.setMat4("uModel", Mat4::translate(camPos));
        meshSkyDome.draw();

        // 2. Render Terrain
        worldShader.setInt("uShadingType", 1);
        worldShader.setMat4("uModel", Mat4::translate(Vec3(0, -0.05f, 0)));
        worldShader.setVec4("uColor", Vec4(1, 1, 1, 1));
        meshTerrain.draw();

        // 3. Render Drop Shadows (Beneath characters, crates, airdrops)
        renderDropShadows();

        // 4. Render Obstacles (Buildings, Watchtowers, Trees, Crates)
        renderObstacles();

        // 5. Render Ground Loot Items
        renderGroundLoot();

        // 6. Render Airdrop & Red Smoke
        renderAirdrops();

        // 7. Render Gloo Walls (Bom Keo Ice Crystals)
        worldShader.setInt("uShadingType", 5);
        for (const auto& gw : glooWalls) {
            Mat4 gMat = Mat4::translate(gw.pos) * Mat4::rotateY(gw.angle);
            worldShader.setMat4("uModel", gMat);
            meshGlooWall.draw();
        }

        // 8. Render Player & Bots (including 3D Parachutes!)
        renderCharacters();

        // 9. Render Bullet Tracers
        worldShader.setInt("uShadingType", 2);
        for (const auto& tr : tracers) {
            Vec3 dir = tr.end - tr.start;
            float len = dir.length();
            Mat4 trMat = Mat4::translate((tr.start + tr.end) * 0.5f) * Mat4::scale(Vec3(0.06f, 0.06f, len));
            worldShader.setMat4("uModel", trMat);
            worldShader.setVec4("uColor", Vec4(1.0f, 0.85f, 0.2f, 0.9f));
            meshQuad.draw();
        }

        // 10. Render Safe Zone Forcefield (Vòng Bo)
        worldShader.setInt("uShadingType", 4);
        Mat4 szMat = Mat4::translate(safeZoneCenter) * Mat4::scale(Vec3(safeZoneRadius, 32.0f, safeZoneRadius));
        worldShader.setMat4("uModel", szMat);
        meshCylinder.draw();

        // 11. Render 2D HUD
        glDisable(GL_DEPTH_TEST);
        renderHUD(screenW, screenH);
    }

    void renderDropShadows() {
        worldShader.setInt("uShadingType", 3);
        worldShader.setVec4("uColor", Vec4(0, 0, 0, 0.6f));

        auto drawShadow = [&](Vec3 pos, float r) {
            Mat4 m = Mat4::translate(Vec3(pos.x, 0.02f, pos.z)) * Mat4::scale(Vec3(r, 1.0f, r));
            worldShader.setMat4("uModel", m);
            meshShadow.draw();
        };

        if (player.isAlive && player.flightState == FlightState::LANDED) {
            drawShadow(player.pos, 0.85f);
        }
        for (const auto& b : bots) {
            if (b.isAlive && b.flightState == FlightState::LANDED) drawShadow(b.pos, 0.85f);
        }
        for (const auto& ad : airdrops) {
            if (ad.landed) drawShadow(ad.pos, 1.6f);
        }
        for (const auto& obs : obstacles) {
            if (obs.type == 3) drawShadow(obs.pos, 1.4f);
        }
    }

    void renderObstacles() {
        worldShader.setInt("uShadingType", 0);

        for (const auto& obs : obstacles) {
            Mat4 model = Mat4::translate(obs.pos) * Mat4::rotateY(obs.angle);
            worldShader.setMat4("uModel", model);

            if (obs.type == 0) meshBuilding.draw();
            else if (obs.type == 1) meshWatchtower.draw();
            else if (obs.type == 2) meshTree.draw();
            else if (obs.type == 3) meshCrate.draw();
        }
    }

    void renderGroundLoot() {
        for (const auto& item : lootItems) {
            if (item.isPickedUp) continue;

            float hoverY = std::sin(item.rotAngle * 2.0f) * 0.15f;
            Mat4 m = Mat4::translate(item.pos + Vec3(0, hoverY, 0)) * Mat4::rotateY(item.rotAngle);

            if (item.type == LootType::WEAPON_AK47) {
                worldShader.setInt("uShadingType", 0);
                worldShader.setMat4("uModel", m * Mat4::scale(Vec3(1.3f, 1.3f, 1.3f)));
                meshAK47.draw();
            } else if (item.type == LootType::WEAPON_MP40) {
                worldShader.setInt("uShadingType", 0);
                worldShader.setMat4("uModel", m * Mat4::scale(Vec3(1.3f, 1.3f, 1.3f)));
                meshMP40.draw();
            } else if (item.type == LootType::WEAPON_M1887) {
                worldShader.setInt("uShadingType", 0);
                worldShader.setMat4("uModel", m * Mat4::scale(Vec3(1.3f, 1.3f, 1.3f)));
                meshM1887.draw();
            } else if (item.type == LootType::WEAPON_AWM) {
                worldShader.setInt("uShadingType", 0);
                worldShader.setMat4("uModel", m * Mat4::scale(Vec3(1.3f, 1.3f, 1.3f)));
                meshAWM.draw();
            } else if (item.type == LootType::MEDKIT) {
                worldShader.setInt("uShadingType", 0);
                worldShader.setMat4("uModel", m);
                meshMedkit.draw();
            } else if (item.type == LootType::GLOO_WALL_ITEM) {
                worldShader.setInt("uShadingType", 5);
                worldShader.setMat4("uModel", m);
                meshGlooItem.draw();
            } else if (item.type == LootType::AMMO_RIFLE || item.type == LootType::AMMO_SMG || item.type == LootType::AMMO_SHOTGUN || item.type == LootType::AMMO_SNIPER) {
                worldShader.setInt("uShadingType", 0);
                worldShader.setMat4("uModel", m);
                meshAmmoBox.draw();
            } else if (item.type == LootType::ARMOR_VEST) {
                worldShader.setInt("uShadingType", 0);
                worldShader.setMat4("uModel", m * Mat4::scale(Vec3(0.9f, 0.9f, 0.5f)));
                worldShader.setVec4("uColor", Vec4(0.35f, 0.4f, 0.5f, 1.0f));
                meshPlayerTorso.draw();
            }
        }
    }

    void renderAirdrops() {
        for (const auto& ad : airdrops) {
            worldShader.setInt("uShadingType", 0);
            Mat4 adMat = Mat4::translate(ad.pos);
            worldShader.setMat4("uModel", adMat);
            meshAirdrop.draw();

            if (ad.landed && !ad.looted) {
                worldShader.setInt("uShadingType", 6);
                for (int s = 0; s < 8; ++s) {
                    float sY = (float)s * 2.5f + 1.0f;
                    float sWave = std::sin(ad.smokeTimer * 3.0f + s) * 0.4f;
                    Mat4 smkMat = Mat4::translate(ad.pos + Vec3(sWave, sY, 0)) * Mat4::scale(Vec3(1.2f + s * 0.3f, 1.2f + s * 0.3f, 1.2f + s * 0.3f));
                    worldShader.setMat4("uModel", smkMat);
                    worldShader.setVec4("uColor", Vec4(0.95f, 0.15f, 0.15f, 0.55f));
                    meshSphere.draw();
                }
            }
        }
    }

    void renderCharacters() {
        auto drawCharModel = [&](const Character& c, Vec4 shirtCol, Vec4 pantCol) {
            if (!c.isAlive) return;

            Mat4 root = Mat4::translate(c.pos) * Mat4::rotateY(c.yaw);

            // If Freefalling, tilt body forward horizontally
            if (c.flightState == FlightState::FREEFALL) {
                root = root * Mat4::rotateX(-60.0f * DEG2RAD) * Mat4::translate(Vec3(0, 0, -0.6f));
            }

            worldShader.setInt("uShadingType", 0);

            // Torso
            Mat4 torsoMat = root * Mat4::translate(Vec3(0, 1.35f, 0));
            worldShader.setMat4("uModel", torsoMat);
            worldShader.setVec4("uColor", shirtCol);
            meshPlayerTorso.draw();

            // Head
            Mat4 headMat = root * Mat4::translate(Vec3(0, 2.1f, 0));
            worldShader.setMat4("uModel", headMat);
            worldShader.setVec4("uColor", Vec4(0.92f, 0.74f, 0.6f, 1.0f));
            meshPlayerHead.draw();

            // Hair/Cap
            Mat4 capMat = root * Mat4::translate(Vec3(0, 2.22f, 0)) * Mat4::scale(Vec3(0.85f, 0.45f, 0.85f));
            worldShader.setMat4("uModel", capMat);
            worldShader.setVec4("uColor", Vec4(0.15f, 0.15f, 0.18f, 1.0f));
            meshPlayerHead.draw();

            // Legs
            float legSwing = (c.flightState == FlightState::LANDED) ? std::sin(c.runAnim) * 0.55f : -0.2f;
            Mat4 lLeg = root * Mat4::translate(Vec3(0.24f, 0.85f, 0)) * Mat4::rotateX(legSwing) * Mat4::translate(Vec3(0, -0.38f, 0));
            worldShader.setMat4("uModel", lLeg);
            worldShader.setVec4("uColor", pantCol);
            meshPlayerLimb.draw();

            Mat4 rLeg = root * Mat4::translate(Vec3(-0.24f, 0.85f, 0)) * Mat4::rotateX(-legSwing) * Mat4::translate(Vec3(0, -0.38f, 0));
            worldShader.setMat4("uModel", rLeg);
            worldShader.setVec4("uColor", pantCol);
            meshPlayerLimb.draw();

            // Detailed 3D Weapon in hands
            if (c.currentWeapon != WeaponType::NONE && c.flightState == FlightState::LANDED) {
                Mat4 gunMat = root * Mat4::translate(Vec3(0.42f, 1.35f, 0.65f)) * Mat4::rotateX(c.pitch);
                worldShader.setMat4("uModel", gunMat);

                if (c.currentWeapon == WeaponType::AK47) meshAK47.draw();
                else if (c.currentWeapon == WeaponType::MP40) meshMP40.draw();
                else if (c.currentWeapon == WeaponType::M1887) meshM1887.draw();
                else if (c.currentWeapon == WeaponType::AWM) meshAWM.draw();

                if (&c == &player && muzzleFlashTimer > 0.0f) {
                    worldShader.setInt("uShadingType", 2);
                    Mat4 mfMat = gunMat * Mat4::translate(Vec3(0, 0, 0.85f)) * Mat4::scale(Vec3(0.45f, 0.45f, 0.45f));
                    worldShader.setMat4("uModel", mfMat);
                    worldShader.setVec4("uColor", Vec4(1.0f, 0.85f, 0.2f, 1.0f));
                    meshSphere.draw();
                    worldShader.setInt("uShadingType", 0);
                }
            }

            // 3D Parachute Canopy if Parachuting
            if (c.flightState == FlightState::PARACHUTING) {
                worldShader.setInt("uShadingType", 0);
                Mat4 chuteMat = Mat4::translate(c.pos);
                worldShader.setMat4("uModel", chuteMat);
                meshParachute.draw();
            }
        };

        drawCharModel(player, Vec4(0.95f, 0.85f, 0.15f, 1.0f), Vec4(0.12f, 0.12f, 0.15f, 1.0f));

        for (const auto& b : bots) {
            drawCharModel(b, Vec4(0.85f, 0.22f, 0.22f, 1.0f), Vec4(0.25f, 0.35f, 0.22f, 1.0f));
        }
    }

    void drawRect(float x, float y, float w, float h, Vec4 color, Mat4 orthoProj, int shapeType = 0) {
        uiShader.setMat4("uProjection", orthoProj);
        uiShader.setInt("uShapeType", shapeType);
        uiShader.setVec4("uColor", color);

        Mat4 model = Mat4::translate(Vec3(x, y, 0)) * Mat4::scale(Vec3(w, h, 1));
        uiShader.setMat4("uProjection", orthoProj * model);
        meshQuad.draw();
    }

    void drawChar5x7(char c, float x, float y, float size, Vec4 color, Mat4 orthoProj) {
        if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';

        uint8_t rows[7] = {0};
        switch (c) {
            case 'A': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x1F; rows[4]=0x11; rows[5]=0x11; rows[6]=0x11; break;
            case 'B': rows[0]=0x1E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x1E; rows[4]=0x11; rows[5]=0x11; rows[6]=0x1E; break;
            case 'C': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x10; rows[3]=0x10; rows[4]=0x10; rows[5]=0x11; rows[6]=0x0E; break;
            case 'D': rows[0]=0x1C; rows[1]=0x12; rows[2]=0x11; rows[3]=0x11; rows[4]=0x11; rows[5]=0x12; rows[6]=0x1C; break;
            case 'E': rows[0]=0x1F; rows[1]=0x10; rows[2]=0x10; rows[3]=0x1E; rows[4]=0x10; rows[5]=0x10; rows[6]=0x1F; break;
            case 'F': rows[0]=0x1F; rows[1]=0x10; rows[2]=0x10; rows[3]=0x1E; rows[4]=0x10; rows[5]=0x10; rows[6]=0x10; break;
            case 'G': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x10; rows[3]=0x17; rows[4]=0x11; rows[5]=0x11; rows[6]=0x0F; break;
            case 'H': rows[0]=0x11; rows[1]=0x11; rows[2]=0x11; rows[3]=0x1F; rows[4]=0x11; rows[5]=0x11; rows[6]=0x11; break;
            case 'I': rows[0]=0x1F; rows[1]=0x04; rows[2]=0x04; rows[3]=0x04; rows[4]=0x04; rows[5]=0x04; rows[6]=0x1F; break;
            case 'J': rows[0]=0x07; rows[1]=0x02; rows[2]=0x02; rows[3]=0x02; rows[4]=0x12; rows[5]=0x12; rows[6]=0x0C; break;
            case 'K': rows[0]=0x11; rows[1]=0x12; rows[2]=0x14; rows[3]=0x18; rows[4]=0x14; rows[5]=0x12; rows[6]=0x11; break;
            case 'L': rows[0]=0x10; rows[1]=0x10; rows[2]=0x10; rows[3]=0x10; rows[4]=0x10; rows[5]=0x10; rows[6]=0x1F; break;
            case 'M': rows[0]=0x11; rows[1]=0x1B; rows[2]=0x15; rows[3]=0x11; rows[4]=0x11; rows[5]=0x11; rows[6]=0x11; break;
            case 'N': rows[0]=0x11; rows[1]=0x19; rows[2]=0x15; rows[3]=0x13; rows[4]=0x11; rows[5]=0x11; rows[6]=0x11; break;
            case 'O': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x11; rows[4]=0x11; rows[5]=0x11; rows[6]=0x0E; break;
            case 'P': rows[0]=0x1E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x1E; rows[4]=0x10; rows[5]=0x10; rows[6]=0x10; break;
            case 'Q': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x11; rows[4]=0x15; rows[5]=0x12; rows[6]=0x0D; break;
            case 'R': rows[0]=0x1E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x1E; rows[4]=0x14; rows[5]=0x12; rows[6]=0x11; break;
            case 'S': rows[0]=0x0F; rows[1]=0x10; rows[2]=0x10; rows[3]=0x0E; rows[4]=0x01; rows[5]=0x01; rows[6]=0x1E; break;
            case 'T': rows[0]=0x1F; rows[1]=0x04; rows[2]=0x04; rows[3]=0x04; rows[4]=0x04; rows[5]=0x04; rows[6]=0x04; break;
            case 'U': rows[0]=0x11; rows[1]=0x11; rows[2]=0x11; rows[3]=0x11; rows[4]=0x11; rows[5]=0x11; rows[6]=0x0E; break;
            case 'V': rows[0]=0x11; rows[1]=0x11; rows[2]=0x11; rows[3]=0x11; rows[4]=0x11; rows[5]=0x0A; rows[6]=0x04; break;
            case 'W': rows[0]=0x11; rows[1]=0x11; rows[2]=0x11; rows[3]=0x11; rows[4]=0x15; rows[5]=0x1B; rows[6]=0x11; break;
            case 'X': rows[0]=0x11; rows[1]=0x11; rows[2]=0x0A; rows[3]=0x04; rows[4]=0x0A; rows[5]=0x11; rows[6]=0x11; break;
            case 'Y': rows[0]=0x11; rows[1]=0x11; rows[2]=0x0A; rows[3]=0x04; rows[4]=0x04; rows[5]=0x04; rows[6]=0x04; break;
            case 'Z': rows[0]=0x1F; rows[1]=0x01; rows[2]=0x02; rows[3]=0x04; rows[4]=0x08; rows[5]=0x10; rows[6]=0x1F; break;
            case '0': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x13; rows[3]=0x15; rows[4]=0x19; rows[5]=0x11; rows[6]=0x0E; break;
            case '1': rows[0]=0x04; rows[1]=0x0C; rows[2]=0x04; rows[3]=0x04; rows[4]=0x04; rows[5]=0x04; rows[6]=0x0E; break;
            case '2': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x01; rows[3]=0x06; rows[4]=0x08; rows[5]=0x10; rows[6]=0x1F; break;
            case '3': rows[0]=0x1F; rows[1]=0x02; rows[2]=0x04; rows[3]=0x02; rows[4]=0x01; rows[5]=0x11; rows[6]=0x0E; break;
            case '4': rows[0]=0x02; rows[1]=0x06; rows[2]=0x0A; rows[3]=0x12; rows[4]=0x1F; rows[5]=0x02; rows[6]=0x02; break;
            case '5': rows[0]=0x1F; rows[1]=0x10; rows[2]=0x1E; rows[3]=0x01; rows[4]=0x01; rows[5]=0x11; rows[6]=0x0E; break;
            case '6': rows[0]=0x06; rows[1]=0x08; rows[2]=0x10; rows[3]=0x1E; rows[4]=0x11; rows[5]=0x11; rows[6]=0x0E; break;
            case '7': rows[0]=0x1F; rows[1]=0x01; rows[2]=0x02; rows[3]=0x04; rows[4]=0x08; rows[5]=0x08; rows[6]=0x08; break;
            case '8': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x0E; rows[4]=0x11; rows[5]=0x11; rows[6]=0x0E; break;
            case '9': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x0F; rows[4]=0x01; rows[5]=0x02; rows[6]=0x0C; break;
            case '!': rows[0]=0x04; rows[1]=0x04; rows[2]=0x04; rows[3]=0x04; rows[4]=0x04; rows[5]=0x00; rows[6]=0x04; break;
            case '+': rows[0]=0x00; rows[1]=0x04; rows[2]=0x04; rows[3]=0x1F; rows[4]=0x04; rows[5]=0x04; rows[6]=0x00; break;
            case ':': rows[0]=0x00; rows[1]=0x04; rows[2]=0x04; rows[3]=0x00; rows[4]=0x04; rows[5]=0x04; rows[6]=0x00; break;
            case '-': rows[0]=0x00; rows[1]=0x00; rows[2]=0x00; rows[3]=0x1F; rows[4]=0x00; rows[5]=0x00; rows[6]=0x00; break;
            case '/': rows[0]=0x01; rows[1]=0x02; rows[2]=0x04; rows[3]=0x08; rows[4]=0x10; rows[5]=0x00; rows[6]=0x00; break;
            case '(': rows[0]=0x02; rows[1]=0x04; rows[2]=0x08; rows[3]=0x08; rows[4]=0x08; rows[5]=0x04; rows[6]=0x02; break;
            case ')': rows[0]=0x08; rows[1]=0x04; rows[2]=0x02; rows[3]=0x02; rows[4]=0x02; rows[5]=0x04; rows[6]=0x08; break;
            case '[': rows[0]=0x0E; rows[1]=0x08; rows[2]=0x08; rows[3]=0x08; rows[4]=0x08; rows[5]=0x08; rows[6]=0x0E; break;
            case ']': rows[0]=0x0E; rows[1]=0x02; rows[2]=0x02; rows[3]=0x02; rows[4]=0x02; rows[5]=0x02; rows[6]=0x0E; break;
            case '#': rows[0]=0x0A; rows[1]=0x0A; rows[2]=0x1F; rows[3]=0x0A; rows[4]=0x1F; rows[5]=0x0A; rows[6]=0x0A; break;
            default: return;
        }

        float pW = size / 5.0f;
        float pH = size / 7.0f;

        for (int r = 0; r < 7; ++r) {
            float py = y + (6 - r) * pH;
            for (int c_idx = 0; c_idx < 5; ++c_idx) {
                if (rows[r] & (1 << (4 - c_idx))) {
                    float px = x + c_idx * pW;
                    drawRect(px, py, pW, pH, color, orthoProj);
                }
            }
        }
    }

    void drawText(const std::string& text, float x, float y, float size, Vec4 color, Mat4 orthoProj) {
        float curX = x;
        float spacing = size * 0.72f;
        for (char c : text) {
            if (c != ' ') {
                drawChar5x7(c, curX, y, size, color, orthoProj);
            }
            curX += spacing;
        }
    }

    void renderHUD(int screenW, int screenH) {
        uiShader.use();
        Mat4 orthoProj = Mat4::ortho(0.0f, (float)screenW, 0.0f, (float)screenH, -1.0f, 1.0f);

        // 1. Crosshair & Hitmarker (Only when landed)
        if (player.flightState == FlightState::LANDED) {
            float cx = screenW * 0.5f;
            float cy = screenH * 0.5f;
            float chSpread = player.isAiming ? 4.0f : (player.vel.lengthSq() > 1.0f ? 14.0f : 8.0f);

            drawRect(cx - 1.0f, cy + chSpread, 2.0f, 8.0f, Vec4(1, 1, 1, 0.85f), orthoProj);
            drawRect(cx - 1.0f, cy - chSpread - 8.0f, 2.0f, 8.0f, Vec4(1, 1, 1, 0.85f), orthoProj);
            drawRect(cx + chSpread, cy - 1.0f, 8.0f, 2.0f, Vec4(1, 1, 1, 0.85f), orthoProj);
            drawRect(cx - chSpread - 8.0f, cy - 1.0f, 8.0f, 2.0f, Vec4(1, 1, 1, 0.85f), orthoProj);
            drawRect(cx - 1.0f, cy - 1.0f, 2.0f, 2.0f, Vec4(1, 0.2f, 0.2f, 0.9f), orthoProj);

            if (hitmarkerTimer > 0.0f) {
                Vec4 hmCol = hitmarkerIsHeadshot ? Vec4(1.0f, 0.15f, 0.15f, 1.0f) : Vec4(1.0f, 1.0f, 1.0f, 1.0f);
                drawRect(cx - 8.0f, cy - 8.0f, 4.0f, 4.0f, hmCol, orthoProj);
                drawRect(cx + 4.0f, cy - 8.0f, 4.0f, 4.0f, hmCol, orthoProj);
                drawRect(cx - 8.0f, cy + 4.0f, 4.0f, 4.0f, hmCol, orthoProj);
                drawRect(cx + 4.0f, cy + 4.0f, 4.0f, 4.0f, hmCol, orthoProj);
            }
        }

        // 2. Skydiving Altitude / Parachute Prompt
        if (player.flightState != FlightState::LANDED) {
            float skyPromptW = 280.0f;
            float skyPromptX = (screenW - skyPromptW) * 0.5f;
            float skyPromptY = 120.0f;

            drawRect(skyPromptX, skyPromptY, skyPromptW, 44.0f, Vec4(0.06f, 0.08f, 0.12f, 0.92f), orthoProj);
            drawRect(skyPromptX, skyPromptY + 41.0f, skyPromptW, 3.0f, Vec4(0.95f, 0.85f, 0.15f, 1.0f), orthoProj);

            std::string altStr = "DO CAO: " + std::to_string((int)player.pos.y) + "M";
            drawText(altStr, skyPromptX + 75.0f, skyPromptY + 24.0f, 14.0f, Vec4(1.0f, 1.0f, 1.0f, 0.9f), orthoProj);

            if (player.flightState == FlightState::FREEFALL) {
                drawText("[F / SPACE] BUNG DU", skyPromptX + 55.0f, skyPromptY + 8.0f, 15.0f, Vec4(0.95f, 0.85f, 0.15f, 1.0f), orthoProj);
            } else {
                drawText("DANG LUON DU XUONG DAO", skyPromptX + 40.0f, skyPromptY + 8.0f, 14.0f, Vec4(0.2f, 0.9f, 1.0f, 1.0f), orthoProj);
            }
        }

        // 3. Loot Interaction Prompt (Free Fire Style!)
        if (nearestLootIdx >= 0 && nearestLootIdx < (int)lootItems.size() && player.flightState == FlightState::LANDED) {
            const LootItem& item = lootItems[nearestLootIdx];
            float lootPromptW = 340.0f;
            float lootPromptH = 45.0f;
            float lootPromptX = (screenW - lootPromptW) * 0.5f;
            float lootPromptY = 140.0f;

            drawRect(lootPromptX, lootPromptY, lootPromptW, lootPromptH, Vec4(0.05f, 0.08f, 0.14f, 0.92f), orthoProj);
            drawRect(lootPromptX, lootPromptY, 6.0f, lootPromptH, Vec4(0.95f, 0.85f, 0.15f, 1.0f), orthoProj);
            drawText("[F] NHAT: " + item.name, lootPromptX + 18.0f, lootPromptY + 15.0f, 14.0f, Vec4(1.0f, 0.9f, 0.2f, 1.0f), orthoProj);
        }

        // 4. Health & EP Bars
        float barW = 280.0f;
        float barH = 14.0f;
        float barX = (screenW - barW) * 0.5f;
        float barY = 40.0f;

        drawRect(barX - 2.0f, barY - 2.0f, barW + 4.0f, barH + 4.0f, Vec4(0.05f, 0.05f, 0.08f, 0.85f), orthoProj);
        float hpFrac = std::max(0.0f, player.hp / player.maxHp);
        drawRect(barX, barY, barW * hpFrac, barH, Vec4(0.95f, 0.85f, 0.15f, 1.0f), orthoProj);
        drawText("HP " + std::to_string((int)player.hp) + "/" + std::to_string((int)player.maxHp), barX + 80.0f, barY + 1.0f, 12.0f, Vec4(0, 0, 0, 1.0f), orthoProj);

        drawRect(barX - 2.0f, barY + barH + 3.0f, barW + 4.0f, 8.0f, Vec4(0.05f, 0.05f, 0.08f, 0.85f), orthoProj);
        float epFrac = std::max(0.0f, player.ep / 100.0f);
        drawRect(barX, barY + barH + 5.0f, barW * epFrac, 4.0f, Vec4(0.95f, 0.45f, 0.15f, 1.0f), orthoProj);

        // 5. Weapon HUD & Inventory (Bottom-Right)
        float wpHudX = screenW - 220.0f;
        float wpHudY = 40.0f;
        drawRect(wpHudX, wpHudY, 195.0f, 65.0f, Vec4(0.06f, 0.08f, 0.12f, 0.88f), orthoProj);
        drawRect(wpHudX, wpHudY + 62.0f, 195.0f, 3.0f, Vec4(0.95f, 0.85f, 0.15f, 1.0f), orthoProj);

        WeaponStats curWpStats = getWeaponStats(player.currentWeapon);
        drawText(curWpStats.name, wpHudX + 10.0f, wpHudY + 40.0f, 16.0f, Vec4(1.0f, 0.9f, 0.2f, 1.0f), orthoProj);

        std::string ammoStr = (player.currentWeapon == WeaponType::NONE) ? "CHUA CO SUNG" :
                              (player.isReloading ? "RELOADING..." : (std::to_string(player.currentMag) + " / " + std::to_string(player.reserveAmmo)));
        drawText(ammoStr, wpHudX + 14.0f, wpHudY + 14.0f, 15.0f, Vec4(1.0f, 1.0f, 1.0f, 0.95f), orthoProj);

        // Gloo Wall Icon & Count
        drawRect(wpHudX - 85.0f, wpHudY, 70.0f, 65.0f, Vec4(0.08f, 0.22f, 0.35f, 0.88f), orthoProj);
        drawText("GLOO", wpHudX - 78.0f, wpHudY + 40.0f, 14.0f, Vec4(0.4f, 0.85f, 1.0f, 1.0f), orthoProj);
        drawText("X" + std::to_string(player.glooWallCount), wpHudX - 65.0f, wpHudY + 14.0f, 20.0f, Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj);

        // Medkit Icon & Count
        drawRect(wpHudX - 170.0f, wpHudY, 75.0f, 65.0f, Vec4(0.12f, 0.32f, 0.18f, 0.88f), orthoProj);
        drawText("[5] MED", wpHudX - 165.0f, wpHudY + 40.0f, 13.0f, Vec4(0.3f, 1.0f, 0.4f, 1.0f), orthoProj);
        drawText("X" + std::to_string(player.medkitCount), wpHudX - 150.0f, wpHudY + 14.0f, 20.0f, Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj);

        // 6. Alive & Kill Counters
        int aliveCount = (player.isAlive ? 1 : 0);
        for (const auto& b : bots) if (b.isAlive) aliveCount++;

        drawRect(20.0f, screenH - 65.0f, 140.0f, 45.0f, Vec4(0.06f, 0.08f, 0.12f, 0.88f), orthoProj);
        drawText("ALIVE: " + std::to_string(aliveCount), 30.0f, screenH - 42.0f, 16.0f, Vec4(0.95f, 0.85f, 0.15f, 1.0f), orthoProj);
        drawText("KILLS: " + std::to_string(playerKills), 30.0f, screenH - 58.0f, 14.0f, Vec4(1.0f, 0.3f, 0.3f, 1.0f), orthoProj);

        // 7. Safe Zone Warning
        if (isZoneShrinking) {
            float warnW = 280.0f;
            float warnX = (screenW - warnW) * 0.5f;
            drawRect(warnX, screenH - 55.0f, warnW, 30.0f, Vec4(0.9f, 0.15f, 0.15f, 0.88f), orthoProj);
            drawText("VONG BO DANG THU HEP!", warnX + 22.0f, screenH - 46.0f, 15.0f, Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj);
        } else {
            std::string timeStr = "BO THU TRONG: " + std::to_string((int)safeZoneTimer) + "S";
            drawText(timeStr, screenW * 0.5f - 80.0f, screenH - 45.0f, 14.0f, Vec4(0.4f, 0.85f, 1.0f, 0.9f), orthoProj);
        }

        // 8. Kill Feed
        float kfY = screenH - 40.0f;
        for (const auto& kf : killFeed) {
            std::string line = kf.killer + " [" + kf.weapon + "] " + kf.victim;
            float lineW = (float)line.length() * 9.0f + 16.0f;
            drawRect(screenW - lineW - 20.0f, kfY - 4.0f, lineW, 20.0f, Vec4(0.04f, 0.05f, 0.08f, 0.85f), orthoProj);
            Vec4 kfColor = (kf.killer == "YOU" || kf.killer == "PLAYER") ? Vec4(1.0f, 0.85f, 0.1f, 1.0f) : Vec4(0.9f, 0.9f, 0.9f, 0.9f);
            drawText(line, screenW - lineW - 12.0f, kfY, 12.0f, kfColor, orthoProj);
            kfY -= 24.0f;
        }

        // 9. Radar Minimap
        renderMinimap(screenW, screenH, orthoProj);

        // 10. Damage Popups
        for (const auto& dp : damagePopups) {
            Mat4 proj = Mat4::perspective(currentFOV * DEG2RAD, (float)screenW / (float)screenH, 0.2f, 350.0f);
            Mat4 v = Mat4::lookAt(camPos, camPos + camForward, Vec3(0, 1, 0));
            Vec4 clipPos = proj * v * Vec4(dp.worldPos + Vec3(0, dp.offsetY, 0), 1.0f);

            if (clipPos.w > 0.1f) {
                float ndcX = clipPos.x / clipPos.w;
                float ndcY = clipPos.y / clipPos.w;
                if (ndcX >= -1.0f && ndcX <= 1.0f && ndcY >= -1.0f && ndcY <= 1.0f) {
                    float scrX = (ndcX * 0.5f + 0.5f) * screenW;
                    float scrY = (ndcY * 0.5f + 0.5f) * screenH;
                    Vec4 dmgCol = dp.isHeadshot ? Vec4(1.0f, 0.15f, 0.15f, 1.0f) : Vec4(1.0f, 0.85f, 0.15f, 1.0f);
                    float sz = dp.isHeadshot ? 22.0f : 16.0f;
                    drawText(std::to_string(dp.damage), scrX - 10.0f, scrY, sz, dmgCol, orthoProj);
                }
            }
        }

        // 11. BOOYAH! Banner
        if (isBooyah) {
            float bW = 540.0f;
            float bH = 140.0f;
            float bX = (screenW - bW) * 0.5f;
            float bY = (screenH - bH) * 0.5f + 50.0f;

            float pulse = 1.0f + 0.06f * std::sin((float)glfwGetTime() * 10.0f);
            drawRect(bX - (bW*(pulse-1.0f)*0.5f), bY - (bH*(pulse-1.0f)*0.5f), bW * pulse, bH * pulse, Vec4(0.95f, 0.75f, 0.05f, 0.95f), orthoProj);
            drawRect(bX + 10.0f, bY + 10.0f, bW - 20.0f, bH - 20.0f, Vec4(0.06f, 0.08f, 0.12f, 0.88f), orthoProj);

            drawText("BOOYAH!", bX + 130.0f, bY + 65.0f, 52.0f, Vec4(1.0f, 0.85f, 0.1f, 1.0f), orthoProj);
            drawText("BAN DA CHIEN THANG! TOP #1", bX + 135.0f, bY + 30.0f, 16.0f, Vec4(1.0f, 1.0f, 1.0f, 0.95f), orthoProj);
            drawText("KILLS: " + std::to_string(playerKills) + "  |  DAMAGE: " + std::to_string(totalDamageDealt), bX + 145.0f, bY + 10.0f, 14.0f, Vec4(0.4f, 0.9f, 1.0f, 0.9f), orthoProj);
        }

        // 12. Game Over Banner
        if (isGameOver && !isBooyah) {
            float gW = 460.0f;
            float gH = 120.0f;
            float gX = (screenW - gW) * 0.5f;
            float gY = (screenH - gH) * 0.5f;

            drawRect(gX, gY, gW, gH, Vec4(0.04f, 0.06f, 0.1f, 0.95f), orthoProj);
            drawRect(gX, gY + gH - 6.0f, gW, 6.0f, Vec4(0.95f, 0.2f, 0.2f, 1.0f), orthoProj);
            drawText("ELIMINATED", gX + 115.0f, gY + 65.0f, 38.0f, Vec4(0.95f, 0.25f, 0.25f, 1.0f), orthoProj);
            drawText("XEP HANG: #" + std::to_string(aliveCount + 1), gX + 165.0f, gY + 35.0f, 16.0f, Vec4(1.0f, 1.0f, 1.0f, 0.9f), orthoProj);
            drawText("NHAN [R] DE CHOI LAI TRAN MOI", gX + 90.0f, gY + 12.0f, 13.0f, Vec4(1.0f, 0.85f, 0.2f, 0.9f), orthoProj);
        }

        if (showHelp) {
            renderHelpMenu(screenW, screenH, orthoProj);
        }
    }

    void renderMinimap(int screenW, int screenH, Mat4 orthoProj) {
        float rW = 140.0f;
        float rH = 140.0f;
        float rX = 20.0f;
        float rY = screenH - 220.0f;

        drawRect(rX, rY, rW, rH, Vec4(0.12f, 0.22f, 0.15f, 0.85f), orthoProj);
        drawRect(rX, rY, rW, 2.0f, Vec4(0.9f, 0.9f, 0.9f, 0.8f), orthoProj);
        drawRect(rX, rY + rH - 2.0f, rW, 2.0f, Vec4(0.9f, 0.9f, 0.9f, 0.8f), orthoProj);
        drawRect(rX, rY, 2.0f, rH, Vec4(0.9f, 0.9f, 0.9f, 0.8f), orthoProj);
        drawRect(rX + rW - 2.0f, rY, 2.0f, rH, Vec4(0.9f, 0.9f, 0.9f, 0.8f), orthoProj);

        float szNormX = (safeZoneCenter.x / HALF_MAP) * 0.5f + 0.5f;
        float szNormZ = (safeZoneCenter.z / HALF_MAP) * 0.5f + 0.5f;
        float szRadFrac = (safeZoneRadius / HALF_MAP) * rW * 0.5f;
        drawRect(rX + szNormX * rW - szRadFrac, rY + (1.0f - szNormZ) * rH - szRadFrac, szRadFrac * 2.0f, szRadFrac * 2.0f, Vec4(0.2f, 0.6f, 1.0f, 0.35f), orthoProj, 1);

        float pNormX = (player.pos.x / HALF_MAP) * 0.5f + 0.5f;
        float pNormZ = (player.pos.z / HALF_MAP) * 0.5f + 0.5f;
        drawRect(rX + pNormX * rW - 3.5f, rY + (1.0f - pNormZ) * rH - 3.5f, 7.0f, 7.0f, Vec4(1.0f, 0.9f, 0.1f, 1.0f), orthoProj, 1);
    }

    void renderHelpMenu(int screenW, int screenH, Mat4 orthoProj) {
        float menuW = 560.0f;
        float menuH = 410.0f;
        float menuX = (screenW - menuW) * 0.5f;
        float menuY = (screenH - menuH) * 0.5f;

        drawRect(menuX, menuY, menuW, menuH, Vec4(0.04f, 0.06f, 0.1f, 0.95f), orthoProj);
        drawRect(menuX, menuY + menuH - 38.0f, menuW, 38.0f, Vec4(0.95f, 0.75f, 0.1f, 1.0f), orthoProj);
        drawText("FREE FIRE 3D - HUONG DAN", menuX + 140.0f, menuY + menuH - 26.0f, 18.0f, Vec4(0.05f, 0.05f, 0.08f, 1.0f), orthoProj);

        float lineY = menuY + menuH - 70.0f;
        float step = 28.0f;
        Vec4 keyCol(1.0f, 0.88f, 0.25f, 1.0f);
        Vec4 descCol(0.9f, 0.95f, 1.0f, 0.9f);

        drawText("[F / SPACE]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("BUNG DU (TREN TROI) / NHAT DO (MAT DAT)", menuX + 230.0f, lineY, 13.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[W, A, S, D]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("DI CHUYEN / LUON DU", menuX + 230.0f, lineY, 14.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[CHUOT TRAI]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("BAN SUNG (AUTOMATIC FIRE)", menuX + 230.0f, lineY, 14.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[CHUOT PHAI]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("NGAM BAN (ZOOM / ADS)", menuX + 230.0f, lineY, 14.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[G / E]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("DAT BOM KEO (GLOO WALL)", menuX + 230.0f, lineY, 14.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[5]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("SU DUNG TUI CUU THUONG (+75 HP)", menuX + 230.0f, lineY, 14.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[1, 2, 3, 4]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("DOI SUNG (AK47, MP40, M1887, AWM)", menuX + 230.0f, lineY, 14.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[R]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("NAP DAN (RELOAD) / CHOI LAI", menuX + 230.0f, lineY, 14.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[P / ESC] TAM DUNG  [H] DONG MENU", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
    }
};

#endif // FREEFIRE_GAME_HPP
