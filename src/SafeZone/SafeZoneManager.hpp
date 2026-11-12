#ifndef SAFEZONE_SAFEZONEMANAGER_HPP
#define SAFEZONE_SAFEZONEMANAGER_HPP

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include "../Math3D.hpp"
#include "../Core/EventBus.hpp"

namespace SafeZone {

struct ZonePhaseChangedEvent : public Core::IEvent {
    int newPhase;
    float currentRadius;
    ZonePhaseChangedEvent(int phase, float radius) : newPhase(phase), currentRadius(radius) {}
};

class SafeZoneManager {
public:
    Vec3 currentCenter = Vec3(0, 0, 0);
    float currentRadius = 480.0f;
    Vec3 nextCenter = Vec3(0, 0, 0);
    float nextRadius = 260.0f;

    int currentPhase = 1;
    int maxPhases = 5;

    float phaseTimer = 55.0f;
    float shrinkDuration = 30.0f;
    bool isShrinking = false;
    float damagePerSecond = 3.0f;

    void reset() {
        currentCenter = Vec3(0, 0, 0);
        currentRadius = 480.0f;
        float ox = ((rand() % 160) - 80.0f);
        float oz = ((rand() % 160) - 80.0f);
        nextCenter = Vec3(ox, 0, oz);
        nextRadius = 260.0f;
        currentPhase = 1;
        phaseTimer = 55.0f;
        shrinkDuration = 30.0f;
        isShrinking = false;
        damagePerSecond = 3.0f;
    }

    void update(float dt) {
        phaseTimer -= dt;

        if (phaseTimer <= 0.0f) {
            if (!isShrinking) {
                isShrinking = true;
                phaseTimer = shrinkDuration;
            } else {
                isShrinking = false;
                currentRadius = nextRadius;
                currentCenter = nextCenter;
                currentPhase++;

                damagePerSecond += 3.5f; // Scale damage per phase
                if (currentPhase == 2) nextRadius = 140.0f;
                else if (currentPhase == 3) nextRadius = 70.0f;
                else if (currentPhase == 4) nextRadius = 30.0f;
                else nextRadius = 10.0f;

                float offsetRange = (currentRadius - nextRadius) * 0.65f;
                float ox = ((rand() % 100) / 50.0f - 1.0f) * offsetRange;
                float oz = ((rand() % 100) / 50.0f - 1.0f) * offsetRange;
                nextCenter = currentCenter + Vec3(ox, 0, oz);

                if (currentPhase == 2) { phaseTimer = 45.0f; shrinkDuration = 25.0f; }
                else if (currentPhase == 3) { phaseTimer = 35.0f; shrinkDuration = 20.0f; }
                else if (currentPhase == 4) { phaseTimer = 25.0f; shrinkDuration = 15.0f; }
                else { phaseTimer = 20.0f; shrinkDuration = 12.0f; }

                ZonePhaseChangedEvent evt(currentPhase, currentRadius);
                Core::EventBus::instance().publish(evt);
            }
        }

        if (isShrinking) {
            float t = dt / shrinkDuration;
            currentRadius = lerp(Vec3(currentRadius, 0, 0), Vec3(nextRadius, 0, 0), t).x;
            currentCenter = lerp(currentCenter, nextCenter, t);
        }
    }

    bool isInsideZone(const Vec3& position) const {
        float dx = position.x - currentCenter.x;
        float dz = position.z - currentCenter.z;
        return (dx * dx + dz * dz) <= (currentRadius * currentRadius);
    }

    float getDistanceToBorder(const Vec3& position) const {
        float dx = position.x - currentCenter.x;
        float dz = position.z - currentCenter.z;
        float distFromCenter = std::sqrt(dx * dx + dz * dz);
        return distFromCenter - currentRadius;
    }
};

} // namespace SafeZone

#endif // SAFEZONE_SAFEZONEMANAGER_HPP
