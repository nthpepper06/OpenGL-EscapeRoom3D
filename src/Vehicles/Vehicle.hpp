#ifndef VEHICLES_VEHICLE_HPP
#define VEHICLES_VEHICLE_HPP

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include "../Math3D.hpp"

namespace Vehicles {

enum class VehicleType {
    MOTORBIKE,
    TACTICAL_JEEP,
    MONSTER_BUGGY
};

struct Vehicle {
    int id = 0;
    VehicleType type = VehicleType::MOTORBIKE;
    Vec3 position;
    Vec3 velocity;
    float yaw = 0.0f;          // Facing angle in radians
    float steerAngle = 0.0f;   // Steering wheel / handlebar angle
    float currentSpeed = 0.0f; // Current forward speed in m/s
    float maxForwardSpeed = 24.5f; // ~88 km/h
    float maxReverseSpeed = 8.5f;  // ~30 km/h
    float acceleration = 18.0f;
    float braking = 24.0f;
    float turnSpeed = 2.6f;
    float health = 500.0f;
    float maxHealth = 500.0f;
    bool isAlive = true;
    bool isOccupied = false;
    int driverId = -1; // -1: None, 0: Player, >0: Bot
    float wheelRotation = 0.0f;
    std::string name = "MOTORBIKE";
    Vec4 themeColor = Vec4(0.95f, 0.25f, 0.15f, 1.0f);

    float getSpeedKmh() const {
        return std::abs(currentSpeed) * 3.6f;
    }

    void applyDamage(float dmg) {
        if (!isAlive) return;
        health -= dmg;
        if (health <= 0.0f) {
            health = 0.0f;
            isAlive = false;
        }
    }
};

class VehicleManager {
public:
    std::vector<Vehicle> vehicles;
    int nextId = 1;

    void spawnVehicle(VehicleType type, const Vec3& pos, float yaw = 0.0f) {
        Vehicle v;
        v.id = nextId++;
        v.type = type;
        v.position = pos;
        v.position.y = 0.0f;
        v.velocity = Vec3(0, 0, 0);
        v.yaw = yaw;
        v.steerAngle = 0.0f;
        v.currentSpeed = 0.0f;
        v.isAlive = true;
        v.isOccupied = false;
        v.driverId = -1;
        v.wheelRotation = 0.0f;

        if (type == VehicleType::MOTORBIKE) {
            v.name = "TACTICAL MOTORBIKE";
            v.maxForwardSpeed = 26.5f; // ~95 km/h
            v.maxReverseSpeed = 7.5f;
            v.acceleration = 22.0f;
            v.braking = 28.0f;
            v.turnSpeed = 2.8f;
            v.health = 450.0f;
            v.maxHealth = 450.0f;
            v.themeColor = Vec4(0.95f, 0.25f, 0.15f, 1.0f); // Crimson Red
        } else if (type == VehicleType::TACTICAL_JEEP) {
            v.name = "MILITARY 4X4 JEEP";
            v.maxForwardSpeed = 19.5f; // ~70 km/h
            v.maxReverseSpeed = 8.5f;
            v.acceleration = 16.0f;
            v.braking = 24.0f;
            v.turnSpeed = 2.2f;
            v.health = 900.0f;
            v.maxHealth = 900.0f;
            v.themeColor = Vec4(0.25f, 0.45f, 0.25f, 1.0f); // Military Camo Olive
        } else {
            v.name = "DESERT BUGGY";
            v.maxForwardSpeed = 22.5f; // ~81 km/h
            v.maxReverseSpeed = 8.0f;
            v.acceleration = 19.0f;
            v.braking = 26.0f;
            v.turnSpeed = 2.5f;
            v.health = 600.0f;
            v.maxHealth = 600.0f;
            v.themeColor = Vec4(0.95f, 0.65f, 0.15f, 1.0f); // Desert Orange
        }

        vehicles.push_back(v);
    }

    void init() {
        vehicles.clear();

        // 16 Tactical Vehicles strategically stationed across the 8 Military POIs and Crossroads
        // 1. PEAK (Central Summit)
        spawnVehicle(VehicleType::MOTORBIKE, Vec3(18.0f, 0, 12.0f), 0.0f);
        spawnVehicle(VehicleType::TACTICAL_JEEP, Vec3(-18.0f, 0, -12.0f), 1.57f);

        // 2. CLOCK TOWER
        spawnVehicle(VehicleType::MOTORBIKE, Vec3(-260.0f, 0, -150.0f), 0.4f);
        spawnVehicle(VehicleType::TACTICAL_JEEP, Vec3(-305.0f, 0, -200.0f), -0.8f);

        // 3. FACTORY
        spawnVehicle(VehicleType::TACTICAL_JEEP, Vec3(255.0f, 0, -150.0f), 0.0f);
        spawnVehicle(VehicleType::MOTORBIKE, Vec3(315.0f, 0, -210.0f), -1.57f);

        // 4. AIRBASE
        spawnVehicle(VehicleType::TACTICAL_JEEP, Vec3(-290.0f, 0, 260.0f), 0.0f);
        spawnVehicle(VehicleType::MOTORBIKE, Vec3(-350.0f, 0, 270.0f), 1.57f);

        // 5. SHIPYARD
        spawnVehicle(VehicleType::TACTICAL_JEEP, Vec3(290.0f, 0, 260.0f), 0.0f);
        spawnVehicle(VehicleType::MOTORBIKE, Vec3(345.0f, 0, 240.0f), 2.2f);

        // 6. BIMASAKTI RADAR
        spawnVehicle(VehicleType::TACTICAL_JEEP, Vec3(0.0f, 0, -320.0f), 0.0f);
        spawnVehicle(VehicleType::MOTORBIKE, Vec3(35.0f, 0, -370.0f), -0.7f);

        // 7. BARRACKS
        spawnVehicle(VehicleType::TACTICAL_JEEP, Vec3(-350.0f, 0, 0.0f), 0.0f);
        spawnVehicle(VehicleType::MOTORBIKE, Vec3(-400.0f, 0, 45.0f), 1.2f);

        // 8. CAPE TOWN
        spawnVehicle(VehicleType::TACTICAL_JEEP, Vec3(350.0f, 0, 0.0f), 0.0f);
        spawnVehicle(VehicleType::MOTORBIKE, Vec3(400.0f, 0, 45.0f), -1.2f);
    }

    void update(float dt) {
        for (auto& v : vehicles) {
            if (!v.isAlive) continue;

            // Rolling friction deceleration when unoccupied
            if (!v.isOccupied) {
                if (std::abs(v.currentSpeed) > 0.1f) {
                    float drag = 8.0f * dt;
                    if (v.currentSpeed > 0) v.currentSpeed = std::max(0.0f, v.currentSpeed - drag);
                    else v.currentSpeed = std::min(0.0f, v.currentSpeed + drag);
                } else {
                    v.currentSpeed = 0.0f;
                }
            }

            // Calculate forward direction vector based on vehicle yaw
            Vec3 forward(std::sin(v.yaw), 0, -std::cos(v.yaw));
            v.velocity = forward * v.currentSpeed;
            v.position += v.velocity * dt;
            v.position.y = 0.0f;

            // Wheel spinning animation
            v.wheelRotation += v.currentSpeed * dt * 4.0f;

            // Boundary clamping to 1000m Island (from -485 to +485)
            v.position.x = std::clamp(v.position.x, -485.0f, 485.0f);
            v.position.z = std::clamp(v.position.z, -485.0f, 485.0f);
        }
    }

    int findNearestVehicle(const Vec3& pos, float maxDist = 3.8f) {
        int bestIdx = -1;
        float bestDist = maxDist;

        for (size_t i = 0; i < vehicles.size(); ++i) {
            if (!vehicles[i].isAlive || vehicles[i].isOccupied) continue;
            float dist = (vehicles[i].position - pos).length();
            if (dist < bestDist) {
                bestDist = dist;
                bestIdx = (int)i;
            }
        }
        return bestIdx;
    }
};

} // namespace Vehicles

#endif // VEHICLES_VEHICLE_HPP
