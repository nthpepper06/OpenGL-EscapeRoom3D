#ifndef GAMEENGINE_HPP
#define GAMEENGINE_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>

#include "Math3D.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Core/EventBus.hpp"
#include "Core/Time.hpp"
#include "Save/SaveSystem.hpp"
#include "Audio/AudioManager.hpp"
#include "Combat/IDamageable.hpp"
#include "Combat/HealthComponent.hpp"
#include "Combat/DamagePopupSystem.hpp"
#include "Weapons/WeaponData.hpp"
#include "Weapons/WeaponController.hpp"
#include "Inventory/ItemData.hpp"
#include "Inventory/InventorySystem.hpp"
#include "Loot/LootTable.hpp"
#include "Loot/LootSpawner.hpp"
#include "SafeZone/SafeZoneManager.hpp"
#include "Match/MatchManager.hpp"
#include "Player/PlayerController.hpp"
#include "Player/PlayerCamera.hpp"
#include "Player/MobileControls.hpp"
#include "AI/BotController.hpp"
#include "AI/BotSpawner.hpp"
#include "AI/SpatialGrid.hpp"
#include "World/IslandLayout.hpp"
#include "UI/UIManager.hpp"
#include "Debug/DebugMenu.hpp"
#include "Vehicles/Vehicle.hpp"

namespace Engine {

struct BoxCollider {
    Vec3 minBound;
    Vec3 maxBound;
    BoxCollider(const Vec3& minB, const Vec3& maxB) : minBound(minB), maxBound(maxB) {}
};

struct CylinderCollider {
    Vec3 center;
    float radius;
    float height;
    CylinderCollider(const Vec3& c, float r, float h) : center(c), radius(r), height(h) {}
};

struct ActiveIceWall {
    int id;
    Vec3 position;
    float yaw;
    float health = 400.0f;
    float maxHealth = 400.0f;
    float lifetime = 30.0f;
    bool isAlive = true;
};

struct ActiveGrenade {
    int id = 0;
    Vec3 position;
    Vec3 velocity;
    float timer = 2.5f;
    bool isAlive = true;
    std::string throwerName;
    int throwerId = 0;
};

struct ActiveExplosion {
    Vec3 position;
    float radius = 0.5f;
    float maxRadius = 7.5f;
    float lifetime = 0.45f;
    float maxLifetime = 0.45f;
};

class GameEngine {
public:
    GLFWwindow* window = nullptr;
    int screenWidth = 1280;
    int screenHeight = 720;

    // Subsystems
    Save::SaveSystem saveSys;
    Match::MatchManager matchMgr;
    SafeZone::SafeZoneManager safeZone;
    Loot::LootTable lootTable;
    Loot::LootSpawner lootSpawner;
    Combat::CombatFeedbackSystem combatFeedback;
    Player::PlayerController player;
    AI::BotSpawner botSpawner;
    AI::SpatialGrid spatialGrid;
    World::IslandLayout islandLayout;
    Vehicles::VehicleManager vehicleMgr;
    UI::UIManager uiMgr;
    Debug::DebugMenu debugMenu;

    int activeVehicleIdx = -1; // -1: on foot, >= 0: driving vehicle
    int nearestVehicleIdx = -1;

    // Shaders
    Shader worldShader;

    // Meshes
    Mesh meshTerrain;
    Mesh meshOcean;
    Mesh meshSkyDome;
    Mesh meshBuilding;
    Mesh meshWatchtower;
    Mesh meshFactoryHall;
    Mesh meshWarehouse;
    Mesh meshHangar;
    Mesh meshClockTower;
    Mesh meshRadarTower;
    Mesh meshContainerRed;
    Mesh meshContainerBlue;
    Mesh meshContainerGreen;
    Mesh meshContainerYellow;
    Mesh meshTree;
    Mesh meshDropShadow;
    Mesh meshCube;
    Mesh meshCylinder;
    Mesh meshSphere;
    Mesh meshIceWall;
    Mesh meshParachute;
    Mesh meshGunAR;
    Mesh meshGunSMG;
    Mesh meshGunShotgun;
    Mesh meshGunSniper;
    Mesh meshMedkit;
    Mesh meshAmmoBox;
    Mesh meshAirdrop;
    Mesh meshGlooItem;
    Mesh meshVest;
    Mesh meshHelmet;
    Mesh meshVestLv1;
    Mesh meshVestLv2;
    Mesh meshVestLv3;
    Mesh meshHelmetLv1;
    Mesh meshHelmetLv2;
    Mesh meshHelmetLv3;
    Mesh meshBackpackLv1;
    Mesh meshBackpackLv2;
    Mesh meshBackpackLv3;
    Mesh meshGrenade;
    Mesh meshAuraRing;
    Mesh meshMotorbike;
    Mesh meshJeep;

    // World Static & Dynamic Colliders
    std::vector<BoxCollider> staticBoxColliders;
    std::vector<CylinderCollider> staticCylColliders;
    std::vector<ActiveIceWall> iceWalls;
    std::vector<ActiveGrenade> activeGrenades;
    std::vector<ActiveExplosion> activeExplosions;
    int nextIceWallId = 1;
    int nextGrenadeId = 1;

    Vec3 airdropPos = Vec3(0, 0, 0);

    // Input States
    bool keys[1024] = {false};
    bool mouseButtons[8] = {false};
    double lastMouseX = 640.0;
    double lastMouseY = 360.0;
    bool firstMouse = true;
    bool cursorCaptured = false;

    float nearestLootDist = 999.0f;
    int nearestLootIdx = -1;
    bool matchSessionActive = false;

    static GameEngine* instance;

    GameEngine() {
        instance = this;
    }

    bool init(int width = 1280, int height = 720, const char* title = "VANGUARD: ZERO HORIZON [ORIGINAL 3D MOBILE BR]") {
        screenWidth = width;
        screenHeight = height;

        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW\n";
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);

        window = glfwCreateWindow(screenWidth, screenHeight, title, nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window\n";
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // VSync

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD\n";
            return false;
        }

        glViewport(0, 0, screenWidth, screenHeight);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_MULTISAMPLE);

        // Load Persistent Profile
        saveSys.loadFromFile();

        // Initialize Default Loot Table
        lootTable = Loot::LootTable::createDefaultTable();

        // Compile Shaders
        if (!worldShader.compile(Shaders::worldVertex, Shaders::worldFragment)) {
            std::cerr << "Failed to compile World Shader\n";
            return false;
        }
        uiMgr.init();

        // Build Procedural 3D Meshes
        buildMeshes();

        // Build World Level Layout & Colliders
        buildWorldLevel();

        // Setup Window Callbacks
        setupCallbacks();

        std::cout << "Vanguard GameEngine Initialized Successfully!\n";
        return true;
    }

    void buildMeshes() {
        meshTerrain = MeshGen::createPlane(1000.0f, 1000.0f, 64, 64, Vec4(0.32f, 0.58f, 0.25f, 1.0f));
        meshOcean = MeshGen::createPlane(2400.0f, 2400.0f, 16, 16, Vec4(0.12f, 0.35f, 0.65f, 0.85f));
        meshSkyDome = MeshGen::createSkyDome(1200.0f, 18, 36);
        meshBuilding = MeshGen::createBuilding(14.0f, 4.8f, 12.0f);
        meshWatchtower = MeshGen::createWatchtower(7.8f, 4.4f);
        meshTree = MeshGen::createTree(7.0f);
        meshDropShadow = MeshGen::createDropShadow(0.85f);
        meshCube = MeshGen::createCube(1.0f, 1.0f, 1.0f);
        meshCylinder = MeshGen::createCylinder(1.0f, 1.0f, 1.0f, 24);
        meshSphere = MeshGen::createSphere(1.0f, 14, 20);
        meshIceWall = MeshGen::createGlooWall(3.2f, 2.9f, 0.65f);
        meshParachute = MeshGen::createParachute(4.2f, 2.4f, 24);
        meshGunAR = MeshGen::createDetailedGun(1);
        meshGunSMG = MeshGen::createDetailedGun(2);
        meshGunShotgun = MeshGen::createDetailedGun(3);
        meshGunSniper = MeshGen::createDetailedGun(4);
        meshMedkit = MeshGen::createMedkit(0.85f);
        meshAmmoBox = MeshGen::createAmmoBox();
        meshAirdrop = MeshGen::createAirdropBox(1.8f);
        meshGlooItem = MeshGen::createGlooItem();
        meshVest = MeshGen::createVestItem(1);
        meshHelmet = MeshGen::createHelmetItem(1);
        meshVestLv1 = MeshGen::createVestItem(1);
        meshVestLv2 = MeshGen::createVestItem(2);
        meshVestLv3 = MeshGen::createVestItem(3);
        meshHelmetLv1 = MeshGen::createHelmetItem(1);
        meshHelmetLv2 = MeshGen::createHelmetItem(2);
        meshHelmetLv3 = MeshGen::createHelmetItem(3);
        meshBackpackLv1 = MeshGen::createBackpackItem(1);
        meshBackpackLv2 = MeshGen::createBackpackItem(2);
        meshBackpackLv3 = MeshGen::createBackpackItem(3);
        meshGrenade = MeshGen::createGrenadeItem();
        meshAuraRing = MeshGen::createAuraRing(4.5f);
        meshMotorbike = MeshGen::createMotorbike();
        meshJeep = MeshGen::createTacticalJeep();

        // Landmark POI meshes
        meshFactoryHall = MeshGen::createFactoryHall(38.0f, 12.0f, 24.0f);
        meshWarehouse = MeshGen::createBuilding(22.0f, 6.5f, 16.0f);
        meshHangar = MeshGen::createHangar(34.0f, 10.0f, 26.0f);
        meshClockTower = MeshGen::createClockTower(28.0f);
        meshRadarTower = MeshGen::createRadarTower(42.0f);
        meshContainerRed = MeshGen::createShippingContainer(6.0f, 2.6f, 2.4f, Vec4(0.85f, 0.22f, 0.2f, 1.0f));
        meshContainerBlue = MeshGen::createShippingContainer(6.0f, 2.6f, 2.4f, Vec4(0.2f, 0.42f, 0.85f, 1.0f));
        meshContainerGreen = MeshGen::createShippingContainer(6.0f, 2.6f, 2.4f, Vec4(0.2f, 0.72f, 0.3f, 1.0f));
        meshContainerYellow = MeshGen::createShippingContainer(6.0f, 2.6f, 2.4f, Vec4(0.85f, 0.65f, 0.15f, 1.0f));
    }

    void buildWorldLevel() {
        islandLayout.init();

        staticBoxColliders.clear();
        staticCylColliders.clear();

        // 1. Generate Colliders for Island Buildings
        for (const auto& b : islandLayout.buildings) {
            float hw = 7.0f, hh = 4.8f, hd = 6.0f;
            if (b.buildingType == 1) { hw = 19.0f; hh = 12.0f; hd = 12.0f; }
            else if (b.buildingType == 2) { hw = 11.0f; hh = 6.5f; hd = 8.0f; }
            else if (b.buildingType == 3) { hw = 17.0f; hh = 10.0f; hd = 13.0f; }
            else if (b.buildingType == 4) { hw = 6.0f; hh = 28.0f; hd = 6.0f; }
            else if (b.buildingType == 5) { hw = 5.5f; hh = 42.0f; hd = 5.5f; }

            staticBoxColliders.emplace_back(
                Vec3(b.position.x - hw, 0.0f, b.position.z - hd),
                Vec3(b.position.x + hw, hh, b.position.z + hd)
            );
        }

        // 2. Watchtower Leg Colliders
        for (const auto& wp : islandLayout.watchtowers) {
            float d = 2.0f;
            staticCylColliders.emplace_back(wp + Vec3(-d, 0, -d), 0.45f, 7.8f);
            staticCylColliders.emplace_back(wp + Vec3( d, 0, -d), 0.45f, 7.8f);
            staticCylColliders.emplace_back(wp + Vec3(-d, 0,  d), 0.45f, 7.8f);
            staticCylColliders.emplace_back(wp + Vec3( d, 0,  d), 0.45f, 7.8f);
        }

        // 3. Shipping Container Colliders
        for (const auto& cp : islandLayout.containers) {
            staticBoxColliders.emplace_back(
                Vec3(cp.position.x - 3.0f, 0.0f, cp.position.z - 1.2f),
                Vec3(cp.position.x + 3.0f, 2.6f, cp.position.z + 1.2f)
            );
        }

        // 4. Tree Trunk Colliders
        for (const auto& tp : islandLayout.trees) {
            staticCylColliders.emplace_back(tp, 0.7f, 5.5f);
        }
    }

    void setupCallbacks() {
        glfwSetWindowUserPointer(window, this);

        glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
            auto* self = (GameEngine*)glfwGetWindowUserPointer(w);
            if (key >= 0 && key < 1024) {
                if (action == GLFW_PRESS) self->keys[key] = true;
                else if (action == GLFW_RELEASE) self->keys[key] = false;
            }
            if (action == GLFW_PRESS) {
                self->onKeyPress(key);
            }
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow* w, double xpos, double ypos) {
            auto* self = (GameEngine*)glfwGetWindowUserPointer(w);
            self->onMouseMove(xpos, ypos);
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
            auto* self = (GameEngine*)glfwGetWindowUserPointer(w);
            if (button >= 0 && button < 8) {
                if (action == GLFW_PRESS) self->mouseButtons[button] = true;
                else if (action == GLFW_RELEASE) self->mouseButtons[button] = false;
            }
            if (action == GLFW_PRESS) {
                self->onMouseClick(button);
            }
        });

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
            auto* self = (GameEngine*)glfwGetWindowUserPointer(w);
            self->screenWidth = width;
            self->screenHeight = height;
            glViewport(0, 0, width, height);
        });
    }

    void captureCursor(bool capture) {
        cursorCaptured = capture;
        glfwSetInputMode(window, GLFW_CURSOR, capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        firstMouse = true;
    }

    void onKeyPress(int key) {
        if (key == GLFW_KEY_F3) {
            debugMenu.toggle();
        }

        if (matchMgr.currentState == Match::MatchState::SPLASH) {
            if (key == GLFW_KEY_SPACE || key == GLFW_KEY_ENTER || key == GLFW_KEY_F) {
                Audio::AudioManager::instance().playButtonClick();
                matchMgr.setState(Match::MatchState::LOBBY);
            }
            return;
        }

        if (matchMgr.currentState == Match::MatchState::LOBBY) {
            if (key == GLFW_KEY_SPACE || key == GLFW_KEY_ENTER || key == GLFW_KEY_F) {
                Audio::AudioManager::instance().playButtonClick();
                matchMgr.setState(Match::MatchState::MATCHMAKING);
            }
            return;
        }

        if (matchMgr.currentState == Match::MatchState::RESULTS) {
            if (key == GLFW_KEY_SPACE || key == GLFW_KEY_ENTER || key == GLFW_KEY_F) {
                Audio::AudioManager::instance().playButtonClick();
                matchMgr.setState(Match::MatchState::LOBBY);
            }
            return;
        }

        if (matchMgr.currentState == Match::MatchState::DEPLOYING || matchMgr.currentState == Match::MatchState::PLAYING) {
            // Parachute deploy
            if (player.state == Player::LocomotionState::FREEFALL && (key == GLFW_KEY_SPACE || key == GLFW_KEY_F)) {
                player.deployParachute();
                Audio::AudioManager::instance().playButtonClick();
            }

            // Weapon slots
            if (key == GLFW_KEY_1) {
                if (player.inventory.primaryWeapon != Weapons::WeaponID::UNARMED) {
                    player.weapons.equipWeapon(player.inventory.primaryWeapon);
                    Audio::AudioManager::instance().playReload();
                }
            } else if (key == GLFW_KEY_2) {
                if (player.inventory.secondaryWeapon != Weapons::WeaponID::UNARMED) {
                    player.weapons.equipWeapon(player.inventory.secondaryWeapon);
                    Audio::AudioManager::instance().playReload();
                }
            } else if (key == GLFW_KEY_3) {
                if (player.inventory.sidearmWeapon != Weapons::WeaponID::UNARMED) {
                    player.weapons.equipWeapon(player.inventory.sidearmWeapon);
                    Audio::AudioManager::instance().playReload();
                }
            } else if (key == GLFW_KEY_4) {
                throwPlayerGrenade();
            } else if (key == GLFW_KEY_G) {
                deployIceWall(player.position, player.yaw);
            } else if (key == GLFW_KEY_E) {
                player.skills.activate(player.position, player.camera.getForward(), player.health);
            } else if (key == GLFW_KEY_K) {
                player.skills.cycleSkill();
            } else if (key == GLFW_KEY_5 || key == GLFW_KEY_H) {
                if (player.inventory.useMedkit()) {
                    player.health.heal(75.0f);
                    Audio::AudioManager::instance().playPickup();
                }
            } else if (key == GLFW_KEY_R) {
                if (player.weapons.startReload()) {
                    Audio::AudioManager::instance().playReload();
                }
            } else if (key == GLFW_KEY_F || key == GLFW_KEY_ENTER) {
                if (activeVehicleIdx >= 0 && activeVehicleIdx < (int)vehicleMgr.vehicles.size()) {
                    // Exit Vehicle
                    auto& v = vehicleMgr.vehicles[activeVehicleIdx];
                    v.isOccupied = false;
                    v.driverId = -1;
                    Vec3 sideDir(std::cos(v.yaw), 0, std::sin(v.yaw));
                    player.position = v.position + sideDir * 1.8f;
                    player.position.y = 0.0f;
                    activeVehicleIdx = -1;
                    Audio::AudioManager::instance().playButtonClick();
                } else if (nearestVehicleIdx >= 0 && nearestVehicleIdx < (int)vehicleMgr.vehicles.size()) {
                    // Enter Nearest Vehicle
                    auto& v = vehicleMgr.vehicles[nearestVehicleIdx];
                    if (v.isAlive && !v.isOccupied) {
                        activeVehicleIdx = nearestVehicleIdx;
                        v.isOccupied = true;
                        v.driverId = 0;
                        Audio::AudioManager::instance().playButtonClick();
                    }
                } else {
                    tryPickupLoot();
                }
            } else if (key == GLFW_KEY_M || key == GLFW_KEY_TAB) {
                uiMgr.toggleFullscreenMap();
                Audio::AudioManager::instance().playButtonClick();
            } else if (key == GLFW_KEY_ESCAPE && uiMgr.showFullscreenMap) {
                uiMgr.showFullscreenMap = false;
                Audio::AudioManager::instance().playButtonClick();
            }
        }
    }

    void onMouseMove(double xpos, double ypos) {
        if (!cursorCaptured) return;

        if (firstMouse) {
            lastMouseX = xpos;
            lastMouseY = ypos;
            firstMouse = false;
        }

        float xoffset = (float)(xpos - lastMouseX);
        float yoffset = (float)(lastMouseY - ypos);
        lastMouseX = xpos;
        lastMouseY = ypos;

        float sens = saveSys.settings.lookSensitivity * 0.08f;
        if (player.isAiming) sens *= saveSys.settings.adsSensitivity;

        player.yaw += xoffset * sens * DEG2RAD;
        player.pitch += yoffset * sens * DEG2RAD;

        if (player.pitch > 85.0f * DEG2RAD) player.pitch = 85.0f * DEG2RAD;
        if (player.pitch < -85.0f * DEG2RAD) player.pitch = -85.0f * DEG2RAD;
    }

    void onMouseClick(int button) {
        if (matchMgr.currentState == Match::MatchState::SPLASH) {
            Audio::AudioManager::instance().playButtonClick();
            matchMgr.setState(Match::MatchState::LOBBY);
            return;
        }

        if (matchMgr.currentState == Match::MatchState::LOBBY) {
            Audio::AudioManager::instance().playButtonClick();
            matchMgr.setState(Match::MatchState::MATCHMAKING);
            return;
        }

        if (matchMgr.currentState == Match::MatchState::RESULTS) {
            Audio::AudioManager::instance().playButtonClick();
            matchMgr.setState(Match::MatchState::LOBBY);
            return;
        }

        if (matchMgr.currentState == Match::MatchState::PLAYING || matchMgr.currentState == Match::MatchState::DEPLOYING) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                // Fire single-shot or initial bullet
                firePlayerWeapon();
            }
        }
    }

    void deployIceWall(const Vec3& sourcePos, float yaw) {
        if (player.inventory.barrierCount <= 0) return;
        player.inventory.barrierCount--;

        float spawnDist = 2.8f;
        Vec3 forward(std::sin(yaw), 0, -std::cos(yaw));
        Vec3 wallPos = sourcePos + forward * spawnDist;
        wallPos.y = 0.0f;

        ActiveIceWall wall;
        wall.id = nextIceWallId++;
        wall.position = wallPos;
        wall.yaw = yaw;
        wall.health = 400.0f;
        wall.maxHealth = 400.0f;
        wall.lifetime = 30.0f;
        wall.isAlive = true;

        iceWalls.push_back(wall);
        Audio::AudioManager::instance().playBarrierDeploy();
    }

    void deployBotIceWall(const Vec3& botPos, float yaw) {
        float spawnDist = 2.8f;
        Vec3 forward(std::sin(yaw), 0, -std::cos(yaw));
        Vec3 wallPos = botPos + forward * spawnDist;
        wallPos.y = 0.0f;

        ActiveIceWall wall;
        wall.id = nextIceWallId++;
        wall.position = wallPos;
        wall.yaw = yaw;
        wall.health = 400.0f;
        wall.maxHealth = 400.0f;
        wall.lifetime = 25.0f;
        wall.isAlive = true;

        iceWalls.push_back(wall);
        Audio::AudioManager::instance().playBarrierDeploy();
    }

    void throwPlayerGrenade() {
        if (!player.inventory.useGrenade()) return;

        ActiveGrenade g;
        g.id = nextGrenadeId++;
        g.position = player.position + Vec3(0, 1.45f, 0);
        Vec3 fwd = player.camera.getForward();
        g.velocity = fwd * 19.0f + Vec3(0, 5.5f, 0);
        g.timer = 2.5f;
        g.isAlive = true;
        g.throwerName = player.name;
        g.throwerId = player.id;

        activeGrenades.push_back(g);
        Audio::AudioManager::instance().playButtonClick();
    }

    void tryPickupLoot() {
        if (nearestLootIdx >= 0 && nearestLootIdx < (int)lootSpawner.worldItems.size()) {
            auto& wi = lootSpawner.worldItems[nearestLootIdx];
            if (!wi.isPickedUp) {
                wi.isPickedUp = true;
                player.inventory.addItem(wi.data);

                if (wi.data.category == Inventory::ItemCategory::WEAPON) {
                    player.weapons.equipWeapon(wi.data.weaponId, 150);
                } else if (wi.data.category == Inventory::ItemCategory::ARMOR_VEST) {
                    player.health.armor.equipVest(wi.data.tierLevel);
                    player.inventory.vestLevel = wi.data.tierLevel;
                    player.health.addShield(50.0f);
                } else if (wi.data.category == Inventory::ItemCategory::HELMET) {
                    player.health.armor.equipHelmet(wi.data.tierLevel);
                    player.inventory.helmetLevel = wi.data.tierLevel;
                    player.health.addShield(25.0f);
                } else if (wi.data.category == Inventory::ItemCategory::BACKPACK) {
                    player.inventory.backpackLevel = std::max(player.inventory.backpackLevel, wi.data.tierLevel);
                } else if (wi.data.category == Inventory::ItemCategory::GRENADE) {
                    player.inventory.grenadeCount += wi.data.count;
                } else if (wi.data.category == Inventory::ItemCategory::AMMO) {
                    player.weapons.reserveAmmo += 60;
                }
                Audio::AudioManager::instance().playPickup();
            }
        }
    }

    void resolveCollisions(Vec3& pos, float radius = 0.45f) {
        for (const auto& box : staticBoxColliders) {
            Vec3 closest(
                std::clamp(pos.x, box.minBound.x, box.maxBound.x),
                std::clamp(pos.y + 0.8f, box.minBound.y, box.maxBound.y),
                std::clamp(pos.z, box.minBound.z, box.maxBound.z)
            );

            Vec3 diff = Vec3(pos.x, pos.y + 0.8f, pos.z) - closest;
            diff.y = 0.0f;
            float distSq = diff.lengthSq();

            if (distSq < radius * radius && distSq > 1e-6f) {
                float dist = std::sqrt(distSq);
                Vec3 pushDir = diff / dist;
                float penetration = radius - dist;
                pos.x += pushDir.x * penetration;
                pos.z += pushDir.z * penetration;
            }
        }

        for (const auto& cyl : staticCylColliders) {
            if (pos.y > cyl.center.y + cyl.height || pos.y + 1.8f < cyl.center.y) continue;

            Vec2 diff(pos.x - cyl.center.x, pos.z - cyl.center.z);
            float distSq = diff.lengthSq();
            float minAllowedDist = radius + cyl.radius;

            if (distSq < minAllowedDist * minAllowedDist && distSq > 1e-6f) {
                float dist = std::sqrt(distSq);
                Vec2 pushDir = diff / dist;
                float penetration = minAllowedDist - dist;
                pos.x += pushDir.x * penetration;
                pos.z += pushDir.y * penetration;
            }
        }

        for (const auto& wall : iceWalls) {
            if (!wall.isAlive) continue;

            Vec2 diff(pos.x - wall.position.x, pos.z - wall.position.z);
            float distSq = diff.lengthSq();
            float wallRadius = 3.2f;

            if (distSq < (wallRadius + radius) * (wallRadius + radius) && distSq > 1e-6f) {
                float dist = std::sqrt(distSq);
                Vec2 pushDir = diff / dist;
                float penetration = (wallRadius + radius) - dist;
                pos.x += pushDir.x * penetration;
                pos.z += pushDir.y * penetration;
            }
        }

        pos.x = std::clamp(pos.x, -485.0f, 485.0f);
        pos.z = std::clamp(pos.z, -485.0f, 485.0f);
    }

    bool rayIntersectsIceWall(const Vec3& rayOrigin, const Vec3& rayDir, float maxDist, int& hitWallIdx, float& hitDist, Vec3& hitPoint) {
        hitWallIdx = -1;
        hitDist = maxDist;

        for (size_t i = 0; i < iceWalls.size(); ++i) {
            const auto& wall = iceWalls[i];
            if (!wall.isAlive) continue;

            Vec3 relOrigin = rayOrigin - wall.position;
            float cosY = std::cos(-wall.yaw);
            float sinY = std::sin(-wall.yaw);

            Vec3 localOrigin(
                relOrigin.x * cosY - relOrigin.z * sinY,
                relOrigin.y,
                relOrigin.x * sinY + relOrigin.z * cosY
            );
            Vec3 localDir(
                rayDir.x * cosY - rayDir.z * sinY,
                rayDir.y,
                rayDir.x * sinY + rayDir.z * cosY
            );

            // Gloo Wall local OBB bounding box:
            Vec3 bmin(-2.6f, 0.0f, -0.6f);
            Vec3 bmax( 2.6f, 3.2f,  1.8f);

            float tmin = 0.0f;
            float tmax = maxDist;
            bool hit = true;

            for (int a = 0; a < 3; ++a) {
                float o = (a == 0) ? localOrigin.x : ((a == 1) ? localOrigin.y : localOrigin.z);
                float d = (a == 0) ? localDir.x : ((a == 1) ? localDir.y : localDir.z);
                float minV = (a == 0) ? bmin.x : ((a == 1) ? bmin.y : bmin.z);
                float maxV = (a == 0) ? bmax.x : ((a == 1) ? bmax.y : bmax.z);

                if (std::abs(d) < 1e-6f) {
                    if (o < minV || o > maxV) { hit = false; break; }
                } else {
                    float t1 = (minV - o) / d;
                    float t2 = (maxV - o) / d;
                    if (t1 > t2) std::swap(t1, t2);
                    tmin = std::max(tmin, t1);
                    tmax = std::min(tmax, t2);
                    if (tmin > tmax) { hit = false; break; }
                }
            }

            if (hit && tmin < hitDist && tmin > 0.05f) {
                hitDist = tmin;
                hitWallIdx = (int)i;
                hitPoint = rayOrigin + rayDir * tmin;
            }
        }

        return (hitWallIdx >= 0);
    }

    bool isOccludedByWorld(const Vec3& from, const Vec3& to) {
        Vec3 dir = to - from;
        float totalDist = dir.length();
        if (totalDist <= 1e-4f) return false;
        dir = dir / totalDist;

        int hitWall = -1;
        float hitDist = totalDist;
        Vec3 hitPt;
        if (rayIntersectsIceWall(from, dir, totalDist, hitWall, hitDist, hitPt)) {
            return true;
        }

        for (const auto& box : staticBoxColliders) {
            float tmin = 0.0f, tmax = totalDist;
            for (int i = 0; i < 3; ++i) {
                float o = (i == 0) ? from.x : ((i == 1) ? from.y : from.z);
                float d = (i == 0) ? dir.x : ((i == 1) ? dir.y : dir.z);
                float bmin = (i == 0) ? box.minBound.x : ((i == 1) ? box.minBound.y : box.minBound.z);
                float bmax = (i == 0) ? box.maxBound.x : ((i == 1) ? box.maxBound.y : box.maxBound.z);

                if (std::abs(d) < 1e-6f) {
                    if (o < bmin || o > bmax) { tmin = 1e9f; break; }
                } else {
                    float t1 = (bmin - o) / d;
                    float t2 = (bmax - o) / d;
                    if (t1 > t2) std::swap(t1, t2);
                    tmin = std::max(tmin, t1);
                    tmax = std::min(tmax, t2);
                    if (tmin > tmax) break;
                }
            }
            if (tmin <= tmax && tmin < totalDist && tmin > 0.1f) {
                return true;
            }
        }

        return false;
    }

    void firePlayerWeapon() {
        if (!player.weapons.canFire()) return;
        player.weapons.onFired();

        Weapons::WeaponSpec spec = player.weapons.getSpec();

        if (spec.id == Weapons::WeaponID::VANGUARD_AR44) Audio::AudioManager::instance().playARFire();
        else if (spec.id == Weapons::WeaponID::SPECTRE_SMG) Audio::AudioManager::instance().playSMGFire();
        else if (spec.id == Weapons::WeaponID::THUNDER_SHOTGUN) Audio::AudioManager::instance().playShotgunFire();
        else if (spec.id == Weapons::WeaponID::APEX_SNIPER) Audio::AudioManager::instance().playSniperFire();

        Vec3 forward = player.camera.getForward();
        Vec3 shootOrigin = player.position + Vec3(0, 1.45f, 0);

        int pellets = spec.pelletsPerShot;
        for (int p = 0; p < pellets; ++p) {
            Vec3 fireDir = forward;
            if (pellets > 1 || !player.isAiming) {
                float spread = player.isAiming ? spec.spread * 0.35f : spec.spread;
                float rx = ((rand() % 2000) / 1000.0f - 1.0f) * spread;
                float ry = ((rand() % 2000) / 1000.0f - 1.0f) * spread;
                fireDir.x += rx;
                fireDir.y += ry;
                fireDir = fireDir.normalized();
            }

            float closestDist = spec.effectiveRange;
            int hitBotIdx = -1;
            int hitWallIdx = -1;
            bool isHeadshot = false;
            Vec3 hitPoint = shootOrigin + fireDir * spec.effectiveRange;

            // 1. Check Ice Wall Interception
            float wallHitDist = spec.effectiveRange;
            Vec3 wallHitPt;
            if (rayIntersectsIceWall(shootOrigin, fireDir, spec.effectiveRange, hitWallIdx, wallHitDist, wallHitPt)) {
                closestDist = wallHitDist;
                hitPoint = wallHitPt;
            }

            // 2. Check Bot Hit
            for (size_t b = 0; b < botSpawner.bots.size(); ++b) {
                auto& bot = botSpawner.bots[b];
                if (!bot.isAlive()) continue;

                Vec3 botPos = bot.position;
                Vec3 toBot = botPos + Vec3(0, 1.0f, 0) - shootOrigin;
                float proj = dot(toBot, fireDir);

                if (proj > 0.0f && proj < closestDist) {
                    Vec3 closest = shootOrigin + fireDir * proj;
                    float distToCenter = (closest - (botPos + Vec3(0, 1.0f, 0))).length();

                    if (distToCenter < 0.85f) {
                        if (!isOccludedByWorld(shootOrigin, closest)) {
                            closestDist = proj;
                            hitBotIdx = (int)b;
                            hitWallIdx = -1;
                            hitPoint = closest;
                            isHeadshot = (closest.y - botPos.y > 1.42f);
                        }
                    }
                }
            }

            combatFeedback.addTracer(shootOrigin + player.camera.getRight() * 0.25f - Vec3(0, 0.2f, 0), hitPoint);

            if (hitWallIdx >= 0 && hitWallIdx < (int)iceWalls.size()) {
                auto& wall = iceWalls[hitWallIdx];
                wall.health -= spec.damage;
                Audio::AudioManager::instance().playIceWallHit();
                if (wall.health <= 0.0f) wall.isAlive = false;
            } else if (hitBotIdx >= 0) {
                auto& targetBot = botSpawner.bots[hitBotIdx];
                Combat::DamageInfo dmg;
                float baseDmg = spec.damage;
                if (player.skills.isActive() && player.skills.currentSkillId == Skills::SkillID::SONAR_RECON) {
                    baseDmg *= 1.2f;
                }
                dmg.rawDamage = baseDmg;
                dmg.hitbox = isHeadshot ? Combat::HitboxType::HEAD : Combat::HitboxType::BODY;
                dmg.attackerId = 0;
                dmg.attackerName = player.name;
                dmg.weaponName = spec.displayName;

                float actualDmg = targetBot.health.applyDamage(dmg, targetBot.name, targetBot.id);

                combatFeedback.addPopup(hitPoint + Vec3(0, 0.4f, 0), (int)actualDmg, isHeadshot);
                combatFeedback.triggerHitmarker(isHeadshot);

                if (isHeadshot) Audio::AudioManager::instance().playHeadshot();
                else Audio::AudioManager::instance().playHitmarker();

                if (!targetBot.isAlive()) {
                    targetBot.state = AI::BotState::DEAD;
                    combatFeedback.addKillFeed(player.name, targetBot.name, spec.displayName, isHeadshot);
                    matchMgr.onPlayerKilledTarget((int)actualDmg);
                    matchMgr.onEntityEliminated(false);
                }
            }
        }

        // Sound Alert: Gunshot sound alerts nearby AI Bots within 130m to rush player
        for (auto& bot : botSpawner.bots) {
            if (bot.isAlive() && (bot.position - player.position).length() < 130.0f) {
                bot.state = AI::BotState::ENGAGE_TARGET;
                bot.targetEntityId = 0; // Player
                bot.moveTarget = player.position;
                bot.decisionTimer = 3.5f;
            }
        }
    }

    void fireBotWeapon(AI::BotController& bot, const Vec3& targetPos, int targetId) {
        if (!bot.weapons.canFire()) {
            if (bot.weapons.currentMag <= 0 && bot.weapons.reserveAmmo > 0 && !bot.weapons.isReloading) {
                bot.weapons.startReload();
            }
            return;
        }
        bot.weapons.onFired();

        Weapons::WeaponSpec spec = bot.weapons.getSpec();

        // Audio feedback for bot firing
        if (spec.id == Weapons::WeaponID::VANGUARD_AR44) Audio::AudioManager::instance().playARFire();
        else if (spec.id == Weapons::WeaponID::SPECTRE_SMG) Audio::AudioManager::instance().playSMGFire();
        else if (spec.id == Weapons::WeaponID::THUNDER_SHOTGUN) Audio::AudioManager::instance().playShotgunFire();
        else if (spec.id == Weapons::WeaponID::APEX_SNIPER) Audio::AudioManager::instance().playSniperFire();

        Vec3 shootOrigin = bot.position + Vec3(0, 1.35f, 0);
        float dist = (targetPos - shootOrigin).length();

        // Realistic combat hit probability curve
        float hitChance = (dist < 25.0f) ? 0.65f : ((dist < 60.0f) ? 0.45f : 0.30f);
        bool isHitRoll = ((rand() % 100) / 100.0f) < hitChance;

        Vec3 dir;
        if (isHitRoll) {
            dir = (targetPos + Vec3(0, 1.1f, 0) - shootOrigin).normalized();
        } else {
            dir = (targetPos + Vec3(0, 1.1f, 0) - shootOrigin).normalized();
            float inaccuracy = 0.08f;
            dir.x += ((rand() % 2000) / 1000.0f - 1.0f) * inaccuracy;
            dir.y += ((rand() % 2000) / 1000.0f - 1.0f) * inaccuracy;
            dir = dir.normalized();
        }

        Vec3 hitPoint = shootOrigin + dir * std::min(80.0f, dist + 6.0f);

        // Check if Ice Wall blocks the bot's shot
        int hitWallIdx = -1;
        float hitDist = 80.0f;
        Vec3 wallHitPt;
        if (rayIntersectsIceWall(shootOrigin, dir, 80.0f, hitWallIdx, hitDist, wallHitPt)) {
            hitPoint = wallHitPt;
            auto& wall = iceWalls[hitWallIdx];
            wall.health -= spec.damage;
            Audio::AudioManager::instance().playIceWallHit();
            if (wall.health <= 0.0f) wall.isAlive = false;
            combatFeedback.addTracer(shootOrigin, hitPoint);
            return;
        }

        combatFeedback.addTracer(shootOrigin, hitPoint);

        // If target is Player (targetId == 0)
        if (targetId == 0) {
            if (player.skills.isActive() && player.skills.currentSkillId == Skills::SkillID::PHANTOM_DASH) {
                return; // Player is cloaked!
            }
            if (player.isAlive() && (player.state == Player::LocomotionState::GROUNDED || activeVehicleIdx >= 0)) {
                if (isHitRoll && !isOccludedByWorld(shootOrigin, player.position + Vec3(0, 1.0f, 0))) {
                    bool isHeadshot = (rand() % 100) < 20; // 20% headshot chance
                    Combat::DamageInfo dmg;
                    float rawDmg = spec.damage * 0.75f;
                    if (player.skills.isActive() && player.skills.currentSkillId == Skills::SkillID::VANGUARD_AURA) {
                        rawDmg *= 0.8f;
                    }
                    dmg.rawDamage = rawDmg;
                    dmg.hitbox = isHeadshot ? Combat::HitboxType::HEAD : Combat::HitboxType::BODY;
                    dmg.attackerId = bot.id;
                    dmg.attackerName = bot.name;
                    dmg.weaponName = spec.displayName;

                    if (activeVehicleIdx >= 0 && activeVehicleIdx < (int)vehicleMgr.vehicles.size()) {
                        vehicleMgr.vehicles[activeVehicleIdx].applyDamage(rawDmg * 0.6f);
                        dmg.rawDamage *= 0.4f;
                    }

                    if (!debugMenu.godMode) {
                        float actual = player.health.applyDamage(dmg, player.name, player.id);
                        combatFeedback.addPopup(player.position + Vec3(0, 1.6f, 0), (int)actual, isHeadshot);

                        if (!player.isAlive()) {
                            combatFeedback.addKillFeed(bot.name, player.name, spec.displayName, isHeadshot);
                            matchMgr.onEntityEliminated(true);
                        }
                    }
                }
            }
        } else {
            // Target is another Bot!
            for (auto& otherBot : botSpawner.bots) {
                if (otherBot.id == targetId && otherBot.isAlive()) {
                    if (isHitRoll && !isOccludedByWorld(shootOrigin, otherBot.position + Vec3(0, 1.0f, 0))) {
                        bool isHeadshot = (rand() % 100) < 20;
                        Combat::DamageInfo dmg;
                        dmg.rawDamage = spec.damage * 0.75f;
                        dmg.hitbox = isHeadshot ? Combat::HitboxType::HEAD : Combat::HitboxType::BODY;
                        dmg.attackerId = bot.id;
                        dmg.attackerName = bot.name;
                        dmg.weaponName = spec.displayName;

                        float actual = otherBot.health.applyDamage(dmg, otherBot.name, otherBot.id);
                        combatFeedback.addPopup(otherBot.position + Vec3(0, 1.6f, 0), (int)actual, isHeadshot);

                        if (!otherBot.isAlive()) {
                            otherBot.state = AI::BotState::DEAD;
                            combatFeedback.addKillFeed(bot.name, otherBot.name, spec.displayName, isHeadshot);
                            matchMgr.onEntityEliminated(false);
                        }
                    }
                    break;
                }
            }
        }
    }

    void startMatchSession() {
        safeZone.reset();
        lootSpawner.populateMapLoot(lootTable, islandLayout.lootSpawns);
        botSpawner.spawnRoster(99);

        // Equip initial weapons & loadout to 99 AI Bots
        for (size_t i = 0; i < botSpawner.bots.size(); ++i) {
            auto& b = botSpawner.bots[i];
            if (i % 4 == 0) b.weapons.equipWeapon(Weapons::WeaponID::VANGUARD_AR44, 150);
            else if (i % 4 == 1) b.weapons.equipWeapon(Weapons::WeaponID::SPECTRE_SMG, 180);
            else if (i % 4 == 2) b.weapons.equipWeapon(Weapons::WeaponID::THUNDER_SHOTGUN, 32);
            else b.weapons.equipWeapon(Weapons::WeaponID::APEX_SNIPER, 15);
            b.inventory.backpackLevel = (i % 3 == 0) ? 2 : ((i % 3 == 1) ? 1 : 3);
            b.inventory.grenadeCount = 1 + (rand() % 2);
            b.inventory.medkitCount = 2;
            b.inventory.barrierCount = 2;
        }

        // Reset Player Position to High-Altitude Airborne Spawn & Equip Starting Loadout
        player.reset(Vec3(0, 145.0f, 0.0f));
        player.inventory.primaryWeapon = Weapons::WeaponID::VANGUARD_AR44;
        player.inventory.sidearmWeapon = Weapons::WeaponID::FALCON_PISTOL;
        player.weapons.equipWeapon(Weapons::WeaponID::VANGUARD_AR44, 150);
        player.inventory.backpackLevel = 2;
        player.inventory.grenadeCount = 2;
        player.inventory.barrierCount = 3;
        player.inventory.medkitCount = 2;
        player.health.shield = 100.0f;
        player.health.armor.equipVest(1);
        player.health.armor.equipHelmet(1);
        player.inventory.vestLevel = 1;
        player.inventory.helmetLevel = 1;
        airdropPos = Vec3(((rand() % 600) - 300.0f), 0, ((rand() % 600) - 300.0f));
        iceWalls.clear();
        activeGrenades.clear();
        activeExplosions.clear();
        // Initialize Vehicles on Island
        vehicleMgr.init();
        activeVehicleIdx = -1;
        nearestVehicleIdx = -1;

        captureCursor(true);
    }

    void update(float dt) {
        matchMgr.update(dt);

        // Auto trigger match session setup reliably
        if (matchMgr.currentState == Match::MatchState::DEPLOYING) {
            if (!matchSessionActive) {
                startMatchSession();
                matchSessionActive = true;
            }
        } else if (matchMgr.currentState == Match::MatchState::LOBBY ||
                   matchMgr.currentState == Match::MatchState::SPLASH ||
                   matchMgr.currentState == Match::MatchState::RESULTS) {
            matchSessionActive = false;
            if (cursorCaptured) captureCursor(false);
            return;
        }

        if (matchMgr.currentState == Match::MatchState::MATCHMAKING) {
            if (cursorCaptured) captureCursor(false);
            return;
        }

        // 1. Player Input, Locomotion & Vehicle Driving Update
        if (player.isAlive()) {
            if (activeVehicleIdx >= 0 && activeVehicleIdx < (int)vehicleMgr.vehicles.size()) {
                // DRIVING MODE
                auto& v = vehicleMgr.vehicles[activeVehicleIdx];
                if (!v.isAlive) {
                    activeVehicleIdx = -1;
                } else {
                    float accelRate = v.acceleration;
                    if (keys[GLFW_KEY_LEFT_SHIFT]) accelRate *= 1.5f; // Nitro Boost

                    if (keys[GLFW_KEY_W]) {
                        v.currentSpeed += accelRate * dt;
                    } else if (keys[GLFW_KEY_S]) {
                        if (v.currentSpeed > 0.5f) v.currentSpeed -= v.braking * dt;
                        else v.currentSpeed -= accelRate * 0.7f * dt; // Reverse
                    } else {
                        // Natural rolling friction
                        if (std::abs(v.currentSpeed) > 0.1f) {
                            float drag = 6.0f * dt;
                            if (v.currentSpeed > 0) v.currentSpeed = std::max(0.0f, v.currentSpeed - drag);
                            else v.currentSpeed = std::min(0.0f, v.currentSpeed + drag);
                        } else {
                            v.currentSpeed = 0.0f;
                        }
                    }

                    // Handbrake
                    if (keys[GLFW_KEY_SPACE]) {
                        float hb = v.braking * 1.6f * dt;
                        if (v.currentSpeed > 0) v.currentSpeed = std::max(0.0f, v.currentSpeed - hb);
                        else v.currentSpeed = std::min(0.0f, v.currentSpeed + hb);
                    }

                    float maxFwd = v.maxForwardSpeed * (keys[GLFW_KEY_LEFT_SHIFT] ? 1.35f : 1.0f);
                    v.currentSpeed = std::clamp(v.currentSpeed, -v.maxReverseSpeed, maxFwd);

                    // Steering
                    if (std::abs(v.currentSpeed) > 0.2f) {
                        float turnMod = (v.currentSpeed >= 0) ? 1.0f : -1.0f;
                        if (keys[GLFW_KEY_A]) v.yaw += v.turnSpeed * dt * turnMod;
                        if (keys[GLFW_KEY_D]) v.yaw -= v.turnSpeed * dt * turnMod;
                    }

                    // Bind player position & orientation to vehicle
                    player.position = v.position;
                    player.position.y = (v.type == Vehicles::VehicleType::MOTORBIKE) ? 0.35f : 0.45f;
                    player.yaw = v.yaw;
                    player.isGrounded = true;

                    // Roadkill / Vehicle Ramming collision against bots
                    if (std::abs(v.currentSpeed) > 5.0f) {
                        for (auto& b : botSpawner.bots) {
                            if (!b.isAlive()) continue;
                            float d = (b.position - v.position).length();
                            if (d < 2.5f) {
                                Combat::DamageInfo rdmg;
                                float ramDmg = 160.0f + std::abs(v.currentSpeed) * 4.0f;
                                rdmg.rawDamage = ramDmg;
                                rdmg.hitbox = Combat::HitboxType::BODY;
                                rdmg.attackerId = 0;
                                rdmg.attackerName = player.name;
                                rdmg.weaponName = v.name + " [RAM]";

                                float actual = b.health.applyDamage(rdmg, b.name, b.id);
                                combatFeedback.addPopup(b.position + Vec3(0, 1.6f, 0), (int)actual, false);
                                Audio::AudioManager::instance().playExplosion();

                                if (!b.isAlive()) {
                                    b.state = AI::BotState::DEAD;
                                    combatFeedback.addKillFeed(player.name, b.name, v.name + " [RAM]", false);
                                    matchMgr.onPlayerKilledTarget((int)actual);
                                    matchMgr.onEntityEliminated(false);
                                }
                            }
                        }
                    }

                    resolveCollisions(v.position, 1.2f);
                }
            } else {
                // ON-FOOT MODE
                Vec3 moveDir(0, 0, 0);
                Vec3 fwd = player.camera.getForward(); fwd.y = 0; fwd = fwd.normalized();
                Vec3 right = player.camera.getRight(); right.y = 0; right = right.normalized();

                if (keys[GLFW_KEY_W]) moveDir += fwd;
                if (keys[GLFW_KEY_S]) moveDir -= fwd;
                if (keys[GLFW_KEY_D]) moveDir += right;
                if (keys[GLFW_KEY_A]) moveDir -= right;

                player.isSprinting = keys[GLFW_KEY_LEFT_SHIFT];
                player.isCrouching = keys[GLFW_KEY_C];
                player.isAiming = mouseButtons[GLFW_MOUSE_BUTTON_RIGHT];

                float speed = player.isCrouching ? 3.2f : (player.isSprinting ? 7.8f : 5.2f);
                if (player.isAiming) speed *= 0.55f;

                if (moveDir.lengthSq() > 1e-4f) {
                    moveDir = moveDir.normalized();
                    player.velocity = moveDir * speed;
                    player.runAnimTime += dt * (player.isSprinting ? 14.0f : 9.0f);
                } else {
                    player.velocity = Vec3(0, 0, 0);
                }

                if (player.state == Player::LocomotionState::GROUNDED) {
                    if (keys[GLFW_KEY_SPACE] && player.isGrounded) {
                        player.verticalVelocity = 6.2f;
                        player.isGrounded = false;
                    }

                    if (!player.isGrounded) {
                        player.verticalVelocity -= 18.0f * dt;
                        player.position.y += player.verticalVelocity * dt;
                        if (player.position.y <= 0.0f) {
                            player.position.y = 0.0f;
                            player.verticalVelocity = 0.0f;
                            player.isGrounded = true;
                        }
                    }
                }

                player.position += player.velocity * dt;
                player.update(dt);

                // Transition from DEPLOYING to PLAYING once landed
                if (player.state == Player::LocomotionState::GROUNDED && matchMgr.currentState == Match::MatchState::DEPLOYING) {
                    matchMgr.setState(Match::MatchState::PLAYING);
                }

                // Automatic Weapon Spray when holding Left Click
                if (mouseButtons[GLFW_MOUSE_BUTTON_LEFT] && player.state == Player::LocomotionState::GROUNDED) {
                    if (player.weapons.getSpec().isAutomatic && player.weapons.canFire()) {
                        firePlayerWeapon();
                    }
                }

                // Resolve Player Collisions with Walls & Obstacles
                resolveCollisions(player.position, 0.45f);

                // Check nearest vehicle when on foot
                nearestVehicleIdx = vehicleMgr.findNearestVehicle(player.position, 3.8f);
            }

            // Safe Zone Damage Tick on Player
            if (!safeZone.isInsideZone(player.position) && (player.state == Player::LocomotionState::GROUNDED || activeVehicleIdx >= 0)) {
                Combat::DamageInfo zdmg;
                zdmg.rawDamage = safeZone.damagePerSecond * dt;
                zdmg.hitbox = Combat::HitboxType::BODY;
                zdmg.attackerName = "SAFE_ZONE";
                zdmg.weaponName = "ELECTRIC_RING";
                if (!debugMenu.godMode) {
                    player.health.applyDamage(zdmg, player.name, player.id);
                    if (!player.isAlive()) {
                        combatFeedback.addKillFeed("SAFE_ZONE", player.name, "ELECTRIC_RING", false);
                        matchMgr.onEntityEliminated(true);
                    }
                }
            }
        }

        // Vehicle Manager Physics Tick
        vehicleMgr.update(dt);

        // 2. Safe Zone Update
        safeZone.update(dt);

        // 3. World Items & Nearest Loot Distance
        lootSpawner.update(dt);
        nearestLootDist = 999.0f;
        nearestLootIdx = -1;
        for (size_t i = 0; i < lootSpawner.worldItems.size(); ++i) {
            const auto& wi = lootSpawner.worldItems[i];
            if (!wi.isPickedUp) {
                float dist = (wi.position - player.position).length();
                if (dist < 3.2f && dist < nearestLootDist) {
                    nearestLootDist = dist;
                    nearestLootIdx = (int)i;
                }
            }
        }

        // 4. Update Dynamic Aegis Ice Walls
        for (auto& wall : iceWalls) {
            if (wall.isAlive) {
                wall.lifetime -= dt;
                if (wall.lifetime <= 0.0f) wall.isAlive = false;
            }
        }

        // 5. Update Active Throwing Grenades
        for (auto& g : activeGrenades) {
            if (!g.isAlive) continue;
            g.timer -= dt;
            g.velocity.y -= 18.0f * dt; // Gravity
            g.position += g.velocity * dt;
            if (g.position.y <= 0.15f) {
                g.position.y = 0.15f;
                g.velocity.y = -g.velocity.y * 0.45f; // Ground bounce
                g.velocity.x *= 0.7f;
                g.velocity.z *= 0.7f;
            }
            resolveCollisions(g.position, 0.2f);

            if (g.timer <= 0.0f) {
                g.isAlive = false;

                // Spawn 3D Explosion VFX
                ActiveExplosion exp;
                exp.position = g.position;
                exp.radius = 0.5f;
                exp.maxRadius = 7.5f;
                exp.lifetime = 0.45f;
                exp.maxLifetime = 0.45f;
                activeExplosions.push_back(exp);
                Audio::AudioManager::instance().playExplosion();

                float maxRadius = 7.5f;

                // Frag AoE Damage: Player
                // Rebuild Spatial Grid for explosive radius queries
                spatialGrid.clear();
                if (player.isAlive()) spatialGrid.insert(0, player.position, true, true);
                for (const auto& b : botSpawner.bots) {
                    if (b.isAlive()) spatialGrid.insert(b.id, b.position, false, true);
                }

                auto nearbyVictims = spatialGrid.queryRadius(g.position, maxRadius);
                for (const auto& vic : nearbyVictims) {
                    float dist = (vic.position - g.position).length();
                    float dmgVal = 180.0f * (1.0f - std::min(1.0f, dist / maxRadius));

                    if (vic.id == 0 && player.isAlive() && player.state == Player::LocomotionState::GROUNDED) {
                        Combat::DamageInfo dmg;
                        dmg.rawDamage = dmgVal;
                        dmg.hitbox = Combat::HitboxType::BODY;
                        dmg.attackerId = g.throwerId;
                        dmg.attackerName = g.throwerName;
                        dmg.weaponName = "FRAG_GRENADE";
                        if (!debugMenu.godMode) {
                            float actual = player.health.applyDamage(dmg, player.name, player.id);
                            combatFeedback.addPopup(player.position + Vec3(0, 1.6f, 0), (int)actual, false);
                            if (!player.isAlive()) {
                                combatFeedback.addKillFeed(g.throwerName, player.name, "FRAG_GRENADE", false);
                                matchMgr.onEntityEliminated(true);
                            }
                        }
                    } else if (vic.id > 0) {
                        for (auto& bot : botSpawner.bots) {
                            if (bot.id == vic.id && bot.isAlive()) {
                                Combat::DamageInfo dmg;
                                dmg.rawDamage = dmgVal;
                                dmg.hitbox = Combat::HitboxType::BODY;
                                dmg.attackerId = g.throwerId;
                                dmg.attackerName = g.throwerName;
                                dmg.weaponName = "FRAG_GRENADE";
                                float actual = bot.health.applyDamage(dmg, bot.name, bot.id);
                                combatFeedback.addPopup(bot.position + Vec3(0, 1.6f, 0), (int)actual, false);
                                if (!bot.isAlive()) {
                                    bot.state = AI::BotState::DEAD;
                                    combatFeedback.addKillFeed(g.throwerName, bot.name, "FRAG_GRENADE", false);
                                    if (g.throwerId == 0) matchMgr.onPlayerKilledTarget((int)actual);
                                    matchMgr.onEntityEliminated(false);
                                }
                                break;
                            }
                        }
                    }
                }

                // Frag AoE Damage: Ice Walls
                for (auto& wall : iceWalls) {
                    if (!wall.isAlive) continue;
                    float dist = (wall.position - g.position).length();
                    if (dist < maxRadius) {
                        wall.health -= 250.0f * (1.0f - dist / maxRadius);
                        if (wall.health <= 0.0f) wall.isAlive = false;
                    }
                }
            }
        }

        // 6. Update Active Explosions
        for (auto& exp : activeExplosions) {
            exp.lifetime -= dt;
            float progress = 1.0f - std::max(0.0f, exp.lifetime / exp.maxLifetime);
            exp.radius = progress * exp.maxRadius;
        }
        activeExplosions.erase(std::remove_if(activeExplosions.begin(), activeExplosions.end(), [](const ActiveExplosion& e){ return e.lifetime <= 0.0f; }), activeExplosions.end());
        activeGrenades.erase(std::remove_if(activeGrenades.begin(), activeGrenades.end(), [](const ActiveGrenade& g){ return !g.isAlive; }), activeGrenades.end());

        // Rebuild Spatial Grid for 100 Combatants Tick
        spatialGrid.clear();
        if (player.isAlive()) {
            spatialGrid.insert(0, player.position, true, true);
        }
        for (const auto& b : botSpawner.bots) {
            if (b.isAlive()) {
                spatialGrid.insert(b.id, b.position, false, true);
            }
        }

        // 7. Update 99 Tactical AI Bots (Spatial Partitioning Free-for-All Combat)
        for (auto& bot : botSpawner.bots) {
            if (!bot.isAlive()) continue;

            bot.weapons.update(dt);

            if (bot.state == AI::BotState::AIRBORNE) {
                bot.position.y -= (bot.isParachuting ? 5.5f : 24.0f) * dt;
                if (bot.position.y <= 30.0f && !bot.isParachuting) {
                    bot.isParachuting = true;
                }
                if (bot.position.y <= 0.0f) {
                    bot.position.y = 0.0f;
                    bot.state = AI::BotState::SEARCH_LOOT;
                    bot.decisionTimer = 3.0f + (rand() % 4);
                }
                continue;
            }

            bot.decisionTimer -= dt;

            // Periodic AI Decision Tree (Spatial Hash Query O(1))
            if (bot.decisionTimer <= 0.0f) {
                bot.decisionTimer = 1.0f + (rand() % 10) * 0.1f;

                if (!safeZone.isInsideZone(bot.position)) {
                    bot.state = AI::BotState::ROTATE_ZONE;
                    bot.moveTarget = safeZone.currentCenter;
                } else if (bot.health.currentHealth < 90.0f && bot.inventory.medkitCount > 0) {
                    bot.state = AI::BotState::HEAL;
                } else {
                    // Query nearby combatants within 110m radius using Spatial Hash Grid
                    auto nearby = spatialGrid.queryRadius(bot.position, 110.0f);
                    float bestDist = 110.0f;
                    int targetId = -1;
                    Vec3 targetPos;

                    for (const auto& ent : nearby) {
                        if (ent.id == bot.id) continue;
                        float d = (ent.position - bot.position).length();
                        if (d < bestDist) {
                            if (ent.id == 0) { // Player
                                if (player.isAlive() && (player.state == Player::LocomotionState::GROUNDED || activeVehicleIdx >= 0)) {
                                    bool playerCloaked = player.skills.isActive() && player.skills.currentSkillId == Skills::SkillID::PHANTOM_DASH;
                                    if (!playerCloaked && !isOccludedByWorld(bot.position, player.position)) {
                                        bestDist = d;
                                        targetId = 0;
                                        targetPos = player.position;
                                    }
                                }
                            } else { // Another Bot
                                if (!isOccludedByWorld(bot.position, ent.position)) {
                                    bestDist = d;
                                    targetId = ent.id;
                                    targetPos = ent.position;
                                }
                            }
                        }
                    }

                    if (targetId != -1) {
                        bot.state = AI::BotState::ENGAGE_TARGET;
                        bot.targetEntityId = targetId;
                        bot.moveTarget = targetPos;
                    } else {
                        if (bot.weapons.currentWeaponId == Weapons::WeaponID::UNARMED) {
                            bot.state = AI::BotState::SEARCH_LOOT;
                            if (!islandLayout.buildings.empty()) {
                                int bIdx = rand() % (int)islandLayout.buildings.size();
                                bot.moveTarget = islandLayout.buildings[bIdx].position + Vec3((rand()%6)-3, 0, (rand()%6)-3);
                            }
                        } else {
                            bot.state = AI::BotState::EXPLORE;
                            float rx = ((rand() % 160) - 80.0f);
                            float rz = ((rand() % 160) - 80.0f);
                            bot.moveTarget = safeZone.currentCenter + Vec3(rx, 0, rz);
                        }
                    }
                }
            }

            // Execute AI Behavior State
            if (bot.state == AI::BotState::HEAL) {
                if (bot.inventory.useMedkit()) {
                    bot.health.heal(75.0f);
                }
                bot.state = AI::BotState::EXPLORE;
            } else if (bot.state == AI::BotState::ENGAGE_TARGET) {
                Vec3 targetPos;
                bool validTarget = false;
                if (bot.targetEntityId == 0) {
                    if (player.isAlive() && (player.state == Player::LocomotionState::GROUNDED || activeVehicleIdx >= 0)) {
                        targetPos = player.position;
                        validTarget = true;
                    }
                } else {
                    for (auto& other : botSpawner.bots) {
                        if (other.id == bot.targetEntityId && other.isAlive()) {
                            targetPos = other.position;
                            validTarget = true;
                            break;
                        }
                    }
                }

                if (validTarget) {
                    Vec3 dir = (targetPos - bot.position);
                    float targetDist = dir.length();
                    bot.yaw = std::atan2(-dir.x, -dir.z);

                    if (bot.health.currentHealth < 70.0f && (rand() % 100) < 15 && bot.inventory.barrierCount > 0) {
                        bot.inventory.barrierCount--;
                        deployBotIceWall(bot.position, bot.yaw);
                    }

                    fireBotWeapon(bot, targetPos, bot.targetEntityId);

                    // Push and rush the player/target aggressively
                    if (targetDist > 14.0f) {
                        Vec3 fwd = dir.normalized();
                        bot.position += fwd * 4.4f * dt;
                        bot.runAnim += dt * 10.0f;
                    } else {
                        Vec3 strafe(-dir.z, 0, dir.x);
                        bot.position += strafe.normalized() * 2.6f * dt;
                        bot.runAnim += dt * 8.0f;
                    }
                } else {
                    bot.state = AI::BotState::EXPLORE;
                }
            } else if (bot.state == AI::BotState::SEARCH_LOOT || bot.state == AI::BotState::EXPLORE || bot.state == AI::BotState::ROTATE_ZONE) {
                Vec3 moveDir = (bot.moveTarget - bot.position);
                moveDir.y = 0;
                if (moveDir.lengthSq() > 1.0f) {
                    moveDir = moveDir.normalized();
                    bot.position += moveDir * 4.8f * dt;
                    bot.yaw = std::atan2(-moveDir.x, -moveDir.z);
                    bot.runAnim += dt * 10.0f;
                }
            }

            resolveCollisions(bot.position, 0.45f);

            if (!safeZone.isInsideZone(bot.position)) {
                Combat::DamageInfo zdmg;
                zdmg.rawDamage = safeZone.damagePerSecond * dt;
                zdmg.hitbox = Combat::HitboxType::BODY;
                zdmg.attackerName = "SAFE_ZONE";
                zdmg.weaponName = "ELECTRIC_RING";
                bot.health.applyDamage(zdmg, bot.name, bot.id);
                if (!bot.isAlive()) {
                    bot.state = AI::BotState::DEAD;
                    combatFeedback.addKillFeed("SAFE_ZONE", bot.name, "ELECTRIC_RING", false);
                    matchMgr.onEntityEliminated(false);
                }
            }
        }

        combatFeedback.update(dt);
    }

    void render() {
        glClearColor(0.08f, 0.12f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Mat4 orthoProj = Mat4::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight, -1.0f, 1.0f);

        // ==========================================
        // 3D WORLD RENDERING (Camera Setup)
        // ==========================================
        Mat4 view;
        Mat4 proj;
        Vec3 camPos;

        if (matchMgr.currentState == Match::MatchState::SPLASH) {
            float t = (float)glfwGetTime() * 0.2f;
            camPos = Vec3(std::sin(t) * 65.0f, 32.0f, std::cos(t) * 65.0f);
            view = Mat4::lookAt(camPos, Vec3(0, 3.0f, 0), Vec3(0, 1, 0));
            proj = Mat4::perspective(55.0f * DEG2RAD, (float)screenWidth / (float)screenHeight, 0.2f, 500.0f);
        } else if (matchMgr.currentState == Match::MatchState::LOBBY) {
            float t = (float)glfwGetTime() * 0.4f;
            camPos = Vec3(std::sin(t) * 4.2f, 1.5f, std::cos(t) * 4.2f);
            view = Mat4::lookAt(camPos, Vec3(0, 1.2f, 0), Vec3(0, 1, 0));
            proj = Mat4::perspective(50.0f * DEG2RAD, (float)screenWidth / (float)screenHeight, 0.2f, 400.0f);
        } else {
            view = player.camera.getViewMatrix();
            proj = player.camera.getProjectionMatrix((float)screenWidth / (float)screenHeight);
            camPos = player.camera.position;
        }

        worldShader.use();
        worldShader.setMat4("uView", view);
        worldShader.setMat4("uProjection", proj);
        worldShader.setVec3("uViewPos", camPos);
        worldShader.setFloat("uTime", (float)glfwGetTime());

        // 1. Panoramic Sky Atmosphere (Disable depth write so it's always background)
        glDepthMask(GL_FALSE);
        worldShader.setInt("uShadingType", 7);
        worldShader.setVec4("uColor", Vec4(1, 1, 1, 1));
        worldShader.setMat4("uModel", Mat4::translate(camPos));
        meshSkyDome.draw();
        glDepthMask(GL_TRUE);

        // 2. Surrounding Ocean Water Plane
        worldShader.setInt("uShadingType", 4);
        worldShader.setVec4("uColor", Vec4(0.12f, 0.42f, 0.72f, 0.85f));
        worldShader.setMat4("uModel", Mat4::translate(Vec3(0, -0.2f, 0)));
        meshOcean.draw();

        // 3. High Quality 1000m Island Terrain
        worldShader.setInt("uShadingType", 1);
        worldShader.setVec4("uColor", Vec4(1, 1, 1, 1));
        worldShader.setMat4("uModel", Mat4::translate(Vec3(0, 0.0f, 0)));
        meshTerrain.draw();

        // 4. Island POI Buildings with Distance Culling
        worldShader.setInt("uShadingType", 0);
        for (const auto& b : islandLayout.buildings) {
            float dist = (b.position - camPos).length();
            float maxDist = (b.buildingType == 4 || b.buildingType == 5) ? 950.0f : 450.0f;
            if (dist > maxDist) continue;

            Mat4 bModel = Mat4::translate(b.position) * Mat4::rotateY(b.yaw);
            worldShader.setMat4("uModel", bModel);

            if (b.buildingType == 0) meshBuilding.draw();
            else if (b.buildingType == 1) meshFactoryHall.draw();
            else if (b.buildingType == 2) meshWarehouse.draw();
            else if (b.buildingType == 3) meshHangar.draw();
            else if (b.buildingType == 4) meshClockTower.draw();
            else if (b.buildingType == 5) meshRadarTower.draw();
        }

        // 5. Watchtowers (Cull > 450m)
        for (const auto& wp : islandLayout.watchtowers) {
            if ((wp - camPos).length() > 450.0f) continue;
            worldShader.setMat4("uModel", Mat4::translate(wp));
            meshWatchtower.draw();
        }

        // 6. Shipping Containers (Cull > 350m)
        for (const auto& cp : islandLayout.containers) {
            if ((cp.position - camPos).length() > 350.0f) continue;
            Mat4 cModel = Mat4::translate(cp.position) * Mat4::rotateY(cp.yaw);
            worldShader.setMat4("uModel", cModel);
            meshContainerRed.draw();
        }

        // 7. Trees (Cull > 350m)
        for (const auto& tp : islandLayout.trees) {
            if ((tp - camPos).length() > 350.0f) continue;
            worldShader.setMat4("uModel", Mat4::translate(tp));
            meshTree.draw();
        }

        // 8. Airdrop Crate & Red Smoke Beacon
        if (matchMgr.currentState != Match::MatchState::SPLASH && matchMgr.currentState != Match::MatchState::LOBBY) {
            worldShader.setInt("uShadingType", 0);
            worldShader.setVec4("uColor", Vec4(1, 1, 1, 1));
            worldShader.setMat4("uModel", Mat4::translate(airdropPos));
            meshAirdrop.draw();

            // Red Smoke Column
            worldShader.setInt("uShadingType", 6);
            worldShader.setVec4("uColor", Vec4(0.95f, 0.15f, 0.15f, 0.75f));
            Mat4 smokeModel = Mat4::translate(airdropPos + Vec3(0, 30.0f, 0)) * Mat4::scale(Vec3(0.85f, 60.0f, 0.85f));
            worldShader.setMat4("uModel", smokeModel);
            meshCylinder.draw();
        }

        // 9. Active Aegis Ice Walls (Bom Keo)
        worldShader.setInt("uShadingType", 5);
        for (const auto& wall : iceWalls) {
            if (wall.isAlive) {
                Mat4 model = Mat4::translate(wall.position) * Mat4::rotateY(wall.yaw);
                worldShader.setMat4("uModel", model);
                meshIceWall.draw();
            }
        }

        // 10. World Loot Items (Distance Culling < 180m)
        if (matchMgr.currentState != Match::MatchState::SPLASH && matchMgr.currentState != Match::MatchState::LOBBY) {
            for (const auto& wi : lootSpawner.worldItems) {
                if (!wi.isPickedUp) {
                    if ((wi.position - camPos).length() > 180.0f) continue;

                    float hoverY = 0.4f + std::sin(wi.hoverTime) * 0.12f;
                    Mat4 model = Mat4::translate(wi.position + Vec3(0, hoverY, 0)) * Mat4::rotateY(wi.rotation);

                    worldShader.setInt("uShadingType", 0);
                    worldShader.setVec4("uColor", wi.data.getRarityColor());

                    if (wi.data.category == Inventory::ItemCategory::WEAPON) {
                        worldShader.setMat4("uModel", model * Mat4::scale(Vec3(1.2f, 1.2f, 1.2f)));
                        if (wi.data.weaponId == Weapons::WeaponID::VANGUARD_AR44) meshGunAR.draw();
                        else if (wi.data.weaponId == Weapons::WeaponID::SPECTRE_SMG) meshGunSMG.draw();
                        else if (wi.data.weaponId == Weapons::WeaponID::THUNDER_SHOTGUN) meshGunShotgun.draw();
                        else meshGunSniper.draw();
                    } else if (wi.data.category == Inventory::ItemCategory::ARMOR_VEST) {
                        worldShader.setMat4("uModel", model * Mat4::scale(Vec3(1.2f, 1.2f, 1.2f)));
                        if (wi.data.tierLevel == 1) meshVestLv1.draw();
                        else if (wi.data.tierLevel == 2) meshVestLv2.draw();
                        else meshVestLv3.draw();
                    } else if (wi.data.category == Inventory::ItemCategory::HELMET) {
                        worldShader.setMat4("uModel", model * Mat4::scale(Vec3(1.2f, 1.2f, 1.2f)));
                        if (wi.data.tierLevel == 1) meshHelmetLv1.draw();
                        else if (wi.data.tierLevel == 2) meshHelmetLv2.draw();
                        else meshHelmetLv3.draw();
                    } else if (wi.data.category == Inventory::ItemCategory::BACKPACK) {
                        worldShader.setMat4("uModel", model * Mat4::scale(Vec3(1.1f, 1.1f, 1.1f)));
                        if (wi.data.tierLevel == 1) meshBackpackLv1.draw();
                        else if (wi.data.tierLevel == 2) meshBackpackLv2.draw();
                        else meshBackpackLv3.draw();
                    } else if (wi.data.category == Inventory::ItemCategory::GRENADE) {
                        worldShader.setMat4("uModel", model * Mat4::scale(Vec3(1.3f, 1.3f, 1.3f)));
                        meshGrenade.draw();
                    } else if (wi.data.category == Inventory::ItemCategory::MEDKIT) {
                        worldShader.setMat4("uModel", model);
                        meshMedkit.draw();
                    } else if (wi.data.category == Inventory::ItemCategory::TACTICAL_BARRIER) {
                        worldShader.setMat4("uModel", model);
                        meshGlooItem.draw();
                    } else {
                        worldShader.setMat4("uModel", model);
                        meshAmmoBox.draw();
                    }
                }
            }

            // 11. Render Active Grenades
            for (const auto& g : activeGrenades) {
                if (g.isAlive) {
                    worldShader.setInt("uShadingType", 0);
                    worldShader.setVec4("uColor", Vec4(0.2f, 0.25f, 0.18f, 1.0f));
                    Mat4 gModel = Mat4::translate(g.position) * Mat4::scale(Vec3(1.4f, 1.4f, 1.4f));
                    worldShader.setMat4("uModel", gModel);
                    meshGrenade.draw();
                }
            }

            // 12. Render Active Explosions (Expanding Fireball VFX)
            for (const auto& exp : activeExplosions) {
                worldShader.setInt("uShadingType", 6);
                float alpha = exp.lifetime / exp.maxLifetime;
                worldShader.setVec4("uColor", Vec4(1.0f, 0.45f, 0.1f, alpha * 0.85f));
                Mat4 expModel = Mat4::translate(exp.position) * Mat4::scale(Vec3(exp.radius, exp.radius, exp.radius));
                worldShader.setMat4("uModel", expModel);
                meshSphere.draw();
            }

            // 13. Render 16 Island Tactical Vehicles (Cull > 350m)
            worldShader.setInt("uShadingType", 0);
            for (const auto& v : vehicleMgr.vehicles) {
                if (!v.isAlive) continue;
                if ((v.position - camPos).length() > 350.0f) continue;
                Mat4 vModel = Mat4::translate(v.position) * Mat4::rotateY(v.yaw);
                worldShader.setMat4("uModel", vModel);
                worldShader.setVec4("uColor", v.themeColor);
                if (v.type == Vehicles::VehicleType::MOTORBIKE) {
                    meshMotorbike.draw();
                } else {
                    meshJeep.draw();
                }
            }

            // 14. Render 99 AI Bots (Distance Culling < 220m or Airborne)
            for (const auto& bot : botSpawner.bots) {
                if (bot.isAlive()) {
                    float dist = (bot.position - camPos).length();
                    if (dist < 220.0f || bot.state == AI::BotState::AIRBORNE) {
                        int botVest = (bot.id % 3) + 1;
                        int botHelm = (bot.id % 3) + 1;
                        renderCharacter(bot.position, bot.yaw, bot.runAnim, false, bot.weapons.currentWeaponId, bot.state == AI::BotState::AIRBORNE && bot.isParachuting, bot.inventory.backpackLevel, false, botVest, botHelm);
                    }
                }
            }
        }

        // 15. Render Player Character
        if (matchMgr.currentState == Match::MatchState::LOBBY) {
            renderCharacter(Vec3(0, 0, 0), (float)glfwGetTime() * 0.3f, 0.0f, true, Weapons::WeaponID::VANGUARD_AR44, false, 2, false, 2, 2);
        } else if (player.isAlive()) {
            bool isCloaked = player.skills.isActive() && player.skills.currentSkillId == Skills::SkillID::PHANTOM_DASH;
            renderCharacter(player.position, player.yaw, player.runAnimTime, true, player.weapons.currentWeaponId, player.state == Player::LocomotionState::PARACHUTING, player.inventory.backpackLevel, isCloaked, player.inventory.vestLevel, player.inventory.helmetLevel);

            // Vanguard Aura 3D Holographic Ring Under Player
            if (player.skills.isActive() && player.skills.currentSkillId == Skills::SkillID::VANGUARD_AURA) {
                worldShader.setInt("uShadingType", 6);
                float pulse = 0.85f + 0.15f * std::sin((float)glfwGetTime() * 8.0f);
                worldShader.setVec4("uColor", Vec4(0.15f, 0.95f, 0.45f, 0.75f * pulse));
                Mat4 auraModel = Mat4::translate(player.position + Vec3(0, 0.08f, 0)) * Mat4::rotateY((float)glfwGetTime() * 2.5f);
                worldShader.setMat4("uModel", auraModel);
                meshAuraRing.draw();
            }
        }

        // 16. Safe Zone Forcefield (Electric Cylinder)
        if (matchMgr.currentState == Match::MatchState::DEPLOYING || matchMgr.currentState == Match::MatchState::PLAYING) {
            worldShader.setInt("uShadingType", 4);
            Mat4 zoneModel = Mat4::translate(safeZone.currentCenter + Vec3(0, 25.0f, 0)) * Mat4::scale(Vec3(safeZone.currentRadius, 50.0f, safeZone.currentRadius));
            worldShader.setMat4("uModel", zoneModel);
            meshCylinder.draw();
        }

        // 17. Bullet Tracers
        worldShader.setInt("uShadingType", 2);
        for (const auto& tr : combatFeedback.tracers) {
            Vec3 dir = tr.end - tr.start;
            float len = dir.length();
            if (len > 0.1f) {
                Mat4 trModel = Mat4::translate(tr.start + dir * 0.5f) * Mat4::scale(Vec3(0.04f, 0.04f, len));
                worldShader.setVec4("uColor", Vec4(1.0f, 0.95f, 0.35f, 0.9f));
                worldShader.setMat4("uModel", trModel);
                meshCube.draw();
            }
        }

        // ==========================================
        // 2D UI OVERLAYS
        // ==========================================
        if (matchMgr.currentState == Match::MatchState::SPLASH) {
            uiMgr.renderSplash(screenWidth, screenHeight, orthoProj);
            return;
        }

        if (matchMgr.currentState == Match::MatchState::LOBBY) {
            uiMgr.renderLobby(screenWidth, screenHeight, orthoProj, saveSys.profile);
            return;
        }

        if (matchMgr.currentState == Match::MatchState::MATCHMAKING) {
            uiMgr.renderMatchmaking(screenWidth, screenHeight, orthoProj, matchMgr.stateTimer);
            return;
        }

        if (matchMgr.currentState == Match::MatchState::RESULTS) {
            uiMgr.renderResults(screenWidth, screenHeight, orthoProj, matchMgr, saveSys.profile);
            return;
        }

        // In-Game HUD
        std::string lootName = (nearestLootIdx >= 0) ? lootSpawner.worldItems[nearestLootIdx].data.name : "";
        std::string vehiclePrompt = "";
        if (activeVehicleIdx < 0 && nearestVehicleIdx >= 0 && nearestVehicleIdx < (int)vehicleMgr.vehicles.size()) {
            vehiclePrompt = vehicleMgr.vehicles[nearestVehicleIdx].name;
        }
        float vSpeed = 0.0f, vHp = 0.0f, vMaxHp = 1.0f;
        if (activeVehicleIdx >= 0 && activeVehicleIdx < (int)vehicleMgr.vehicles.size()) {
            vSpeed = vehicleMgr.vehicles[activeVehicleIdx].getSpeedKmh();
            vHp = vehicleMgr.vehicles[activeVehicleIdx].health;
            vMaxHp = vehicleMgr.vehicles[activeVehicleIdx].maxHealth;
        }

        uiMgr.renderHUD(screenWidth, screenHeight, orthoProj, player, safeZone, matchMgr.alivePlayers, matchMgr.playerKills, safeZone.phaseTimer, safeZone.isShrinking, nearestLootIdx, lootName, combatFeedback.hitmarkerTimer, combatFeedback.hitmarkerIsHeadshot, activeVehicleIdx, vehiclePrompt, vSpeed, vHp, vMaxHp);

        // 3D Damage Popups
        for (const auto& pop : combatFeedback.popups) {
            Vec4 clipPos = proj * view * Vec4(pop.worldPos + Vec3(0, pop.offsetY, 0), 1.0f);
            if (clipPos.w > 0.1f) {
                float ndcX = clipPos.x / clipPos.w;
                float ndcY = clipPos.y / clipPos.w;
                float sx = (ndcX * 0.5f + 0.5f) * screenWidth;
                float sy = (ndcY * 0.5f + 0.5f) * screenHeight;

                Vec4 col = pop.isHeadshot ? Vec4(1.0f, 0.15f, 0.15f, 1.0f) : Vec4(1.0f, 0.95f, 0.2f, 1.0f);
                uiMgr.drawText(std::to_string(pop.damage), sx - 12.0f, sy, pop.isHeadshot ? 22.0f : 16.0f, col, orthoProj);
            }
        }

        // Kill Feed (Top-Right)
        float kfY = screenHeight - 65.0f;
        for (const auto& kf : combatFeedback.killFeed) {
            std::string feedText = kf.killer + " [" + kf.weapon + "] " + kf.victim;
            uiMgr.drawRect(screenWidth - 380.0f, kfY - 4.0f, 360.0f, 24.0f, Vec4(0.08f, 0.12f, 0.18f, 0.85f), orthoProj);
            uiMgr.drawText(feedText, screenWidth - 370.0f, kfY + 2.0f, 12.0f, kf.isHeadshot ? Vec4(1.0f, 0.3f, 0.3f, 1.0f) : Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj);
            kfY -= 28.0f;
        }

        // Victory Banner
        if (matchMgr.currentState == Match::MatchState::VICTORY) {
            float vW = 440.0f, vH = 90.0f;
            float vX = (screenWidth - vW) * 0.5f, vY = screenHeight * 0.5f + 80.0f;
            uiMgr.drawRect(vX, vY, vW, vH, Vec4(0.08f, 0.12f, 0.18f, 0.96f), orthoProj);
            uiMgr.drawRect(vX, vY + vH - 5.0f, vW, 5.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
            uiMgr.drawText("BOOYAH! #1 VICTORY", vX + 45.0f, vY + 30.0f, 28.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        }

        // Defeat Banner
        if (matchMgr.currentState == Match::MatchState::ELIMINATED) {
            float eW = 420.0f, eH = 80.0f;
            float eX = (screenWidth - eW) * 0.5f, eY = screenHeight * 0.5f + 80.0f;
            uiMgr.drawRect(eX, eY, eW, eH, Vec4(0.08f, 0.12f, 0.18f, 0.96f), orthoProj);
            uiMgr.drawRect(eX, eY + eH - 5.0f, eW, 5.0f, Vec4(0.95f, 0.2f, 0.2f, 1.0f), orthoProj);
            uiMgr.drawText("ELIMINATED", eX + 110.0f, eY + 25.0f, 26.0f, Vec4(0.95f, 0.2f, 0.2f, 1.0f), orthoProj);
        }

        // Fullscreen Tactical Map Overlay [M / TAB]
        if (uiMgr.showFullscreenMap && (matchMgr.currentState == Match::MatchState::DEPLOYING || matchMgr.currentState == Match::MatchState::PLAYING)) {
            uiMgr.renderFullscreenMap(screenWidth, screenHeight, orthoProj, player.position, player.yaw, safeZone, airdropPos, islandLayout, vehicleMgr.vehicles);
        }
    }

    void renderCharacter(const Vec3& pos, float yaw, float animTime, bool isPlayer, Weapons::WeaponID weaponId, bool isParachuting, int backpackLevel = 1, bool isCloaked = false, int vestLevel = 1, int helmetLevel = 1) {
        worldShader.setInt("uShadingType", isCloaked ? 6 : 0);

        Mat4 root = Mat4::translate(pos) * Mat4::rotateY(yaw);

        // 1. Drop Shadow
        if (!isCloaked) {
            worldShader.setInt("uShadingType", 3);
            worldShader.setVec4("uColor", Vec4(0.02f, 0.05f, 0.03f, 0.55f));
            worldShader.setMat4("uModel", Mat4::translate(Vec3(pos.x, 0.02f, pos.z)));
            meshDropShadow.draw();
            worldShader.setInt("uShadingType", 0);
        }

        // Character Color Palette (Camo Tactical Outfit)
        Vec4 skinCol = isCloaked ? Vec4(0.2f, 0.8f, 1.0f, 0.35f) : Vec4(0.92f, 0.76f, 0.62f, 1.0f);
        Vec4 shirtCol = isCloaked ? Vec4(0.2f, 0.6f, 1.0f, 0.35f) : (isPlayer ? Vec4(0.18f, 0.42f, 0.28f, 1.0f) : Vec4(0.45f, 0.22f, 0.22f, 1.0f));
        Vec4 pantsCol = isCloaked ? Vec4(0.1f, 0.4f, 0.8f, 0.35f) : Vec4(0.18f, 0.22f, 0.26f, 1.0f);

        // Head Base
        worldShader.setVec4("uColor", skinCol);
        worldShader.setMat4("uModel", root * Mat4::translate(Vec3(0, 1.62f, 0)) * Mat4::scale(Vec3(0.32f, 0.35f, 0.32f)));
        meshSphere.draw();

        // Helmet (Level 1, 2, 3)
        if (helmetLevel > 0) {
            Mat4 hModel = root * Mat4::translate(Vec3(0, 1.58f, 0)) * Mat4::scale(Vec3(0.95f, 0.95f, 0.95f));
            worldShader.setMat4("uModel", hModel);
            if (helmetLevel == 1) meshHelmetLv1.draw();
            else if (helmetLevel == 2) meshHelmetLv2.draw();
            else meshHelmetLv3.draw();
        }

        // Torso Base
        worldShader.setVec4("uColor", shirtCol);
        worldShader.setMat4("uModel", root * Mat4::translate(Vec3(0, 1.15f, 0)) * Mat4::scale(Vec3(0.55f, 0.65f, 0.35f)));
        meshCube.draw();

        // Armor Vest (Level 1, 2, 3)
        if (vestLevel > 0) {
            Mat4 vModel = root * Mat4::translate(Vec3(0, 1.05f, 0)) * Mat4::scale(Vec3(0.95f, 0.95f, 0.95f));
            worldShader.setMat4("uModel", vModel);
            if (vestLevel == 1) meshVestLv1.draw();
            else if (vestLevel == 2) meshVestLv2.draw();
            else meshVestLv3.draw();
        }

        // Backpack on Character Back
        if (backpackLevel > 0) {
            Mat4 bpModel = root * Mat4::translate(Vec3(0, 1.18f, 0.22f)) * Mat4::scale(Vec3(0.9f, 0.9f, 0.9f));
            worldShader.setMat4("uModel", bpModel);
            if (backpackLevel == 1) meshBackpackLv1.draw();
            else if (backpackLevel == 2) meshBackpackLv2.draw();
            else meshBackpackLv3.draw();
        }

        // Legs with Running Animation
        float legSwing = std::sin(animTime) * 0.35f;
        worldShader.setVec4("uColor", pantsCol);

        // Left Leg
        Mat4 lLeg = root * Mat4::translate(Vec3(-0.16f, 0.45f, 0)) * Mat4::rotateX(legSwing) * Mat4::scale(Vec3(0.2f, 0.85f, 0.22f));
        worldShader.setMat4("uModel", lLeg);
        meshCube.draw();

        // Right Leg
        Mat4 rLeg = root * Mat4::translate(Vec3(0.16f, 0.45f, 0)) * Mat4::rotateX(-legSwing) * Mat4::scale(Vec3(0.2f, 0.85f, 0.22f));
        worldShader.setMat4("uModel", rLeg);
        meshCube.draw();

        // Arms & Equipped Gun
        worldShader.setVec4("uColor", skinCol);
        Mat4 rArm = root * Mat4::translate(Vec3(0.36f, 1.15f, -0.2f)) * Mat4::scale(Vec3(0.16f, 0.55f, 0.18f));
        worldShader.setMat4("uModel", rArm);
        meshCube.draw();

        // Attached Gun Model
        if (weaponId != Weapons::WeaponID::UNARMED) {
            Mat4 gunModel = root * Mat4::translate(Vec3(0.28f, 1.1f, -0.45f)) * Mat4::scale(Vec3(1.0f, 1.0f, 1.0f));
            worldShader.setMat4("uModel", gunModel);
            if (weaponId == Weapons::WeaponID::VANGUARD_AR44) meshGunAR.draw();
            else if (weaponId == Weapons::WeaponID::SPECTRE_SMG) meshGunSMG.draw();
            else if (weaponId == Weapons::WeaponID::THUNDER_SHOTGUN) meshGunShotgun.draw();
            else meshGunSniper.draw();
        }

        // Parachute Canopy when gliding
        if (isParachuting) {
            worldShader.setInt("uShadingType", 0);
            Mat4 chModel = root * Mat4::translate(Vec3(0, 3.2f, 0));
            worldShader.setMat4("uModel", chModel);
            meshParachute.draw();
        }
    }

    void run() {
        double lastTime = glfwGetTime();

        while (!glfwWindowShouldClose(window)) {
            double currentTime = glfwGetTime();
            float dt = (float)(currentTime - lastTime);
            if (dt > 0.1f) dt = 0.1f;
            lastTime = currentTime;

            glfwPollEvents();

            update(dt);
            render();

            glfwSwapBuffers(window);
        }

        saveSys.saveToFile();
    }
};

inline GameEngine* GameEngine::instance = nullptr;

} // namespace Engine

#endif // GAMEENGINE_HPP
