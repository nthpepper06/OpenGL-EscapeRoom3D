#ifndef MULTIPLAYER_INETWORKSERVICE_HPP
#define MULTIPLAYER_INETWORKSERVICE_HPP

#include <string>
#include <vector>
#include "../Math3D.hpp"
#include "../Weapons/WeaponData.hpp"

namespace Multiplayer {

struct PlayerNetworkSnapshot {
    int playerId = 0;
    Vec3 position;
    Vec3 velocity;
    float yaw = 0.0f;
    float pitch = 0.0f;
    float health = 200.0f;
    float shield = 50.0f;
    Weapons::WeaponID activeWeapon = Weapons::WeaponID::UNARMED;
    int currentMag = 0;
    bool isFiring = false;
    bool isAiming = false;
    bool isAlive = true;
    uint32_t serverTimestamp = 0;
};

struct DamageValidationRequest {
    int shooterId;
    int targetId;
    Weapons::WeaponID weaponId;
    Vec3 hitLocation;
    float clientDamage;
    uint32_t tickNumber;
};

class INetworkService {
public:
    virtual ~INetworkService() = default;
    virtual bool connect(const std::string& serverAddress, int port) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual void sendPlayerInput(const Vec3& moveDir, float yaw, float pitch, bool fire, bool aim) = 0;
    virtual void sendDamageReport(const DamageValidationRequest& report) = 0;
    virtual void pollNetworkEvents() = 0;
    virtual float getPing() const = 0;
};

// Local mock authoritative simulation for offline / prototype gameplay
class LocalAuthoritativeNetworkService : public INetworkService {
private:
    bool connected = true;
    float simulatedPing = 24.0f; // ms

public:
    bool connect(const std::string&, int) override { connected = true; return true; }
    void disconnect() override { connected = false; }
    bool isConnected() const override { return connected; }
    void sendPlayerInput(const Vec3&, float, float, bool, bool) override {}
    void sendDamageReport(const DamageValidationRequest&) override {}
    void pollNetworkEvents() override {}
    float getPing() const override { return simulatedPing; }
};

} // namespace Multiplayer

#endif // MULTIPLAYER_INETWORKSERVICE_HPP
