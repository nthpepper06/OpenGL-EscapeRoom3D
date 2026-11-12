#ifndef WORLD_ISLANDLAYOUT_HPP
#define WORLD_ISLANDLAYOUT_HPP

#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include "../Math3D.hpp"

namespace World {

// 3D Terrain Heightmap & Elevation Function for 1000m Island
inline float getTerrainHeight(float x, float z) {
    float r = std::sqrt(x * x + z * z);
    if (r > 480.0f) return 0.0f;

    // 1. Central Grand Mountain Summit (PEAK at Y = +26.0m)
    float peak = 26.0f * std::exp(-(x * x + z * z) / (2.0f * 115.0f * 115.0f));

    // 2. Rolling hills and natural mountain ridges
    float hill1 = 5.5f * std::sin(x * 0.016f) * std::cos(z * 0.016f);
    float hill2 = 3.5f * std::sin(x * 0.032f + 1.2f) * std::sin(z * 0.028f + 0.8f);
    float hill3 = 2.0f * std::cos(x * 0.065f) * std::sin(z * 0.065f);

    // 3. POI elevations & plateau hills
    float dClock = std::sqrt((x + 280.0f)*(x + 280.0f) + (z + 180.0f)*(z + 180.0f));
    float clockHill = 7.5f * std::exp(-(dClock * dClock) / (2.0f * 90.0f * 90.0f));

    float dBima = std::sqrt(x*x + (z + 360.0f)*(z + 360.0f));
    float bimaHill = 5.5f * std::exp(-(dBima * dBima) / (2.0f * 80.0f * 80.0f));

    float dCape = std::sqrt((x - 380.0f)*(x - 380.0f) + (z - 20.0f)*(z - 20.0f));
    float capeHill = 6.5f * std::exp(-(dCape * dCape) / (2.0f * 85.0f * 85.0f));

    float rawH = peak + hill1 + hill2 + hill3 + clockHill + bimaHill + capeHill;

    // Smoothstep falloff to beach sand / sea level at island borders (r > 360m)
    if (r > 360.0f) {
        float f = 1.0f - (r - 360.0f) / (480.0f - 360.0f);
        f = std::clamp(f, 0.0f, 1.0f);
        rawH *= (f * f * (3.0f - 2.0f * f));
    }

    return std::max(0.0f, rawH);
}

struct POIInfo {
    std::string name;
    std::string code;
    Vec3 center;
    float radius;
    Vec4 badgeColor;
};

struct BuildingPlacement {
    Vec3 position;
    float yaw;
    int buildingType; // 0: Barracks, 1: Factory Hall, 2: Warehouse, 3: Hangar, 4: ClockTower, 5: RadarTower, 6: Compound Villa, 7: Wilderness Cabin, 8: Concrete Bunker
};

struct ContainerPlacement {
    Vec3 position;
    float yaw;
    Vec4 color;
};

struct MushroomSpawn {
    Vec3 position;
    int tier = 1; // 1: +50 EP (Orange), 2: +100 EP (Yellow), 3: +200 EP (Purple)
    bool isConsumed = false;
};

class IslandLayout {
public:
    std::vector<POIInfo> pois;
    std::vector<BuildingPlacement> buildings;
    std::vector<Vec3> watchtowers;
    std::vector<ContainerPlacement> containers;
    std::vector<Vec3> trees;
    std::vector<Vec3> lootSpawns;
    std::vector<MushroomSpawn> mushrooms;

    void init() {
        pois.clear();
        buildings.clear();
        watchtowers.clear();
        containers.clear();
        trees.clear();
        lootSpawns.clear();
        mushrooms.clear();

        // 1. Define 8 Iconic Military Island POIs (Free Fire Bermuda style)
        pois.push_back({ "PEAK", "PK", Vec3(0.0f, getTerrainHeight(0,0), 0.0f), 115.0f, Vec4(0.95f, 0.25f, 0.25f, 1.0f) });
        pois.push_back({ "CLOCK TOWER", "CT", Vec3(-280.0f, getTerrainHeight(-280, -180), -180.0f), 95.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f) });
        pois.push_back({ "FACTORY", "FC", Vec3(280.0f, getTerrainHeight(280, -180), -180.0f), 105.0f, Vec4(0.35f, 0.75f, 0.95f, 1.0f) });
        pois.push_back({ "AIRBASE", "AB", Vec3(-320.0f, getTerrainHeight(-320, 260), 260.0f), 120.0f, Vec4(0.45f, 0.85f, 0.45f, 1.0f) });
        pois.push_back({ "SHIPYARD", "SY", Vec3(320.0f, getTerrainHeight(320, 260), 260.0f), 110.0f, Vec4(0.85f, 0.35f, 0.95f, 1.0f) });
        pois.push_back({ "BIMASAKTI", "BM", Vec3(0.0f, getTerrainHeight(0, -360), -360.0f), 90.0f, Vec4(0.25f, 0.95f, 0.85f, 1.0f) });
        pois.push_back({ "BARRACKS", "BK", Vec3(-380.0f, getTerrainHeight(-380, 20), 20.0f), 95.0f, Vec4(0.95f, 0.55f, 0.25f, 1.0f) });
        pois.push_back({ "CAPE TOWN", "CP", Vec3(380.0f, getTerrainHeight(380, 20), 20.0f), 95.0f, Vec4(0.65f, 0.65f, 0.95f, 1.0f) });

        auto addBldg = [&](float x, float z, float yaw, int type) {
            float y = getTerrainHeight(x, z);
            buildings.push_back({ Vec3(x, y, z), yaw, type });
        };

        // 2. Build Structural Placements (50+ Walkable Houses & Compounds)
        // -------------------------------------------------------------
        // POI 1: PEAK (Central Grand Mountain Summit)
        addBldg( -28.0f, -24.0f, 0.0f, 6 ); // Multi-room Compound Villa
        addBldg(  28.0f, -24.0f, 0.0f, 6 ); // Multi-room Compound Villa
        addBldg( -28.0f,  24.0f, 0.0f, 0 ); // Barracks
        addBldg(  28.0f,  24.0f, 0.0f, 0 ); // Barracks
        addBldg(   0.0f,  38.0f, 1.57f, 8 ); // Concrete Bunker
        addBldg(   0.0f, -38.0f, -1.57f, 8 ); // Concrete Bunker
        watchtowers.push_back(Vec3( -48.0f, getTerrainHeight(-48, 0),   0.0f));
        watchtowers.push_back(Vec3(  48.0f, getTerrainHeight( 48, 0),   0.0f));

        // POI 2: CLOCK TOWER
        addBldg(-280.0f, -180.0f, 0.0f, 4); // Clock Tower landmark
        addBldg(-320.0f, -150.0f, 0.5f, 6); // Compound Villa
        addBldg(-240.0f, -150.0f, -0.5f, 6); // Compound Villa
        addBldg(-280.0f, -230.0f, 0.0f, 0); // Barracks
        addBldg(-310.0f, -220.0f, 1.2f, 7); // Wooden Cabin
        addBldg(-250.0f, -220.0f, -1.2f, 7); // Wooden Cabin
        watchtowers.push_back(Vec3(-335.0f, getTerrainHeight(-335, -195), -195.0f));
        watchtowers.push_back(Vec3(-225.0f, getTerrainHeight(-225, -195), -195.0f));

        // POI 3: FACTORY
        addBldg( 280.0f, -180.0f, 0.0f, 1 ); // Factory Main Hall
        addBldg( 335.0f, -145.0f, 1.57f, 2 ); // Warehouse
        addBldg( 225.0f, -145.0f, 1.57f, 2 ); // Warehouse
        addBldg( 280.0f, -120.0f, 0.0f, 6 ); // Compound Office
        addBldg( 340.0f, -210.0f, 0.0f, 8 ); // Bunker
        containers.push_back({ Vec3( 270.0f, getTerrainHeight(270, -220), -220.0f), 0.0f, Vec4(0.85f, 0.2f, 0.2f, 1.0f) });
        containers.push_back({ Vec3( 285.0f, getTerrainHeight(285, -220), -220.0f), 0.0f, Vec4(0.2f, 0.4f, 0.85f, 1.0f) });
        containers.push_back({ Vec3( 300.0f, getTerrainHeight(300, -220), -220.0f), 0.0f, Vec4(0.2f, 0.75f, 0.3f, 1.0f) });
        watchtowers.push_back(Vec3( 350.0f, getTerrainHeight(350, -210), -210.0f));

        // POI 4: AIRBASE & HANGARS
        addBldg(-350.0f,  230.0f, 0.0f, 3 ); // Hangar 1
        addBldg(-280.0f,  230.0f, 0.0f, 3 ); // Hangar 2
        addBldg(-315.0f,  295.0f, 0.0f, 6 ); // Flight Control Compound
        addBldg(-370.0f,  280.0f, 1.57f, 0 ); // Barracks
        addBldg(-250.0f,  280.0f, -1.57f, 0 ); // Barracks
        watchtowers.push_back(Vec3(-390.0f, getTerrainHeight(-390, 240),  240.0f));
        watchtowers.push_back(Vec3(-230.0f, getTerrainHeight(-230, 240),  240.0f));

        // POI 5: SHIPYARD DOCKS
        addBldg( 330.0f,  220.0f, 0.0f, 2 ); // Warehouse 1
        addBldg( 275.0f,  220.0f, 0.0f, 2 ); // Warehouse 2
        addBldg( 330.0f,  170.0f, 1.57f, 6 ); // Harbor Master Villa
        addBldg( 275.0f,  170.0f, -1.57f, 0 ); // Crew Barracks
        containers.push_back({ Vec3( 310.0f, getTerrainHeight(310, 270),  270.0f), 0.0f, Vec4(0.85f, 0.2f, 0.2f, 1.0f) });
        containers.push_back({ Vec3( 325.0f, getTerrainHeight(325, 270),  270.0f), 0.0f, Vec4(0.85f, 0.65f, 0.15f, 1.0f) });
        containers.push_back({ Vec3( 340.0f, getTerrainHeight(340, 270),  270.0f), 0.0f, Vec4(0.2f, 0.45f, 0.85f, 1.0f) });
        containers.push_back({ Vec3( 310.0f, getTerrainHeight(310, 285),  285.0f), 0.0f, Vec4(0.2f, 0.75f, 0.35f, 1.0f) });
        containers.push_back({ Vec3( 325.0f, getTerrainHeight(325, 285),  285.0f), 0.0f, Vec4(0.85f, 0.2f, 0.2f, 1.0f) });
        watchtowers.push_back(Vec3( 365.0f, getTerrainHeight(365, 280),  280.0f));

        // POI 6: BIMASAKTI RADAR & VALLEY
        addBldg(   0.0f, -360.0f, 0.0f, 5 ); // Giant Radar Tower
        addBldg( -35.0f, -330.0f, 0.3f, 6 ); // Radar Control Center
        addBldg(  35.0f, -330.0f, -0.3f, 6 ); // Radar Control Center
        addBldg(   0.0f, -400.0f, 0.0f, 8 ); // Defense Bunker
        addBldg( -60.0f, -360.0f, 1.57f, 7 ); // Wilderness Cabin
        addBldg(  60.0f, -360.0f, -1.57f, 7 ); // Wilderness Cabin
        watchtowers.push_back(Vec3( -55.0f, getTerrainHeight(-55, -370), -370.0f));
        watchtowers.push_back(Vec3(  55.0f, getTerrainHeight( 55, -370), -370.0f));

        // POI 7: RIVERSIDE BARRACKS
        addBldg(-390.0f,  -30.0f, 0.0f, 0 );
        addBldg(-390.0f,   20.0f, 0.0f, 0 );
        addBldg(-390.0f,   70.0f, 0.0f, 0 );
        addBldg(-345.0f,   20.0f, 1.57f, 6 ); // Officers Villa
        addBldg(-345.0f,  -30.0f, 1.57f, 7 ); // Guard Cabin
        watchtowers.push_back(Vec3(-420.0f, getTerrainHeight(-420, 20), 20.0f));

        // POI 8: CAPE TOWN OUTPOST
        addBldg( 390.0f,  -30.0f, 0.0f, 0 );
        addBldg( 390.0f,   20.0f, 0.0f, 0 );
        addBldg( 390.0f,   70.0f, 0.0f, 0 );
        addBldg( 345.0f,   20.0f, 1.57f, 6 ); // Coastal Villa
        addBldg( 345.0f,  -30.0f, 1.57f, 7 ); // Fisher Cabin
        watchtowers.push_back(Vec3( 420.0f, getTerrainHeight(420, 20), 20.0f));

        // EXTRA SETTLEMENTS & WILDERNESS CABINS (Between POIs)
        // 9. North Hill Outpost (between Peak & Bimasakti)
        addBldg(   0.0f, -190.0f, 0.0f, 6 );
        addBldg( -35.0f, -190.0f, 0.5f, 7 );
        addBldg(  35.0f, -190.0f, -0.5f, 7 );

        // 10. South Valley Outpost (between Peak & Ocean)
        addBldg(   0.0f,  190.0f, 0.0f, 6 );
        addBldg( -35.0f,  190.0f, 0.8f, 7 );
        addBldg(  35.0f,  190.0f, -0.8f, 7 );

        // 11. West Hilltop Compound (between Peak & Clock Tower)
        addBldg(-150.0f,  -90.0f, 0.4f, 6 );
        addBldg(-185.0f, -105.0f, 1.2f, 7 );
        addBldg(-120.0f,  -75.0f, -0.6f, 8 );

        // 12. East Plateau Compound (between Peak & Factory)
        addBldg( 150.0f,  -90.0f, -0.4f, 6 );
        addBldg( 185.0f, -105.0f, -1.2f, 7 );
        addBldg( 120.0f,  -75.0f, 0.6f, 8 );

        // 13. South-West Ridge (between Clock Tower & Airbase)
        addBldg(-240.0f,   70.0f, 0.7f, 7 );
        addBldg(-270.0f,   95.0f, -0.3f, 7 );

        // 14. South-East Ridge (between Factory & Shipyard)
        addBldg( 240.0f,   70.0f, -0.7f, 7 );
        addBldg( 270.0f,   95.0f, 0.3f, 7 );

        // 3. Procedural Forest of Trees Across the 1000m Island
        for (int i = 0; i < 220; ++i) {
            float angle = (float)i / 220.0f * 2.0f * PI + ((rand() % 100) / 450.0f);
            float dist = 35.0f + (rand() % 415);
            float tx = std::cos(angle) * dist;
            float tz = std::sin(angle) * dist;

            // Don't place tree inside buildings
            bool blocked = false;
            for (const auto& b : buildings) {
                if ((Vec2(tx, tz) - Vec2(b.position.x, b.position.z)).length() < 18.0f) {
                    blocked = true;
                    break;
                }
            }
            if (!blocked) {
                float ty = getTerrainHeight(tx, tz);
                trees.push_back(Vec3(tx, ty, tz));
            }
        }

        // 4. Generate 60+ Edible Energy Mushrooms (Nấm EP Lv1, Lv2, Lv3)
        // Lv1 (+50 EP): Orange, Lv2 (+100 EP): Yellow, Lv3 (+200 EP): Purple
        for (int m = 0; m < 65; ++m) {
            float angle = (float)m / 65.0f * 2.0f * PI + ((rand() % 100) / 300.0f);
            float dist = 25.0f + (rand() % 390);
            float mx = std::cos(angle) * dist;
            float mz = std::sin(angle) * dist;

            // Don't spawn inside buildings
            bool insideBldg = false;
            for (const auto& b : buildings) {
                if ((Vec2(mx, mz) - Vec2(b.position.x, b.position.z)).length() < 10.0f) {
                    insideBldg = true;
                    break;
                }
            }
            if (!insideBldg) {
                float my = getTerrainHeight(mx, mz);
                int tier = (m % 5 == 0) ? 3 : ((m % 2 == 0) ? 2 : 1);
                mushrooms.push_back({ Vec3(mx, my, mz), tier, false });
            }
        }

        // 5. Generate 280+ High Quality Loot Spawn Anchors
        // A) POI Centers & Building Interiors (Inside Walkable Rooms!)
        for (const auto& b : buildings) {
            lootSpawns.push_back(b.position + Vec3( 2.5f, 0.1f,  2.0f));
            lootSpawns.push_back(b.position + Vec3(-2.5f, 0.1f,  2.0f));
            lootSpawns.push_back(b.position + Vec3( 0.0f, 0.1f, -2.5f));
            lootSpawns.push_back(b.position + Vec3( 3.5f, 0.1f, -2.0f));
        }

        // B) Watchtowers
        for (const auto& w : watchtowers) {
            lootSpawns.push_back(w + Vec3(0, 0.1f, 0));
            lootSpawns.push_back(w + Vec3(1.8f, 0.1f, 1.8f));
        }

        // C) Containers
        for (const auto& c : containers) {
            lootSpawns.push_back(c.position + Vec3(0, 0.1f, 3.2f));
            lootSpawns.push_back(c.position + Vec3(0, 0.1f, -3.2f));
        }

        // D) Field Camps & Crossroads
        for (const auto& poi : pois) {
            for (int k = 0; k < 6; ++k) {
                float a = (float)k / 6.0f * 2.0f * PI;
                float d = 30.0f + (rand() % 40);
                float lx = poi.center.x + std::cos(a) * d;
                float lz = poi.center.z + std::sin(a) * d;
                float ly = getTerrainHeight(lx, lz) + 0.1f;
                lootSpawns.push_back(Vec3(lx, ly, lz));
            }
        }
    }
};

} // namespace World

#endif // WORLD_ISLANDLAYOUT_HPP
