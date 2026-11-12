#ifndef COMBAT_DAMAGEPOPUPSYSTEM_HPP
#define COMBAT_DAMAGEPOPUPSYSTEM_HPP

#include <vector>
#include <string>
#include "../Math3D.hpp"

namespace Combat {

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

class CombatFeedbackSystem {
public:
    std::vector<DamagePopup> popups;
    std::vector<BulletTracer> tracers;
    std::vector<KillFeedEntry> killFeed;

    float hitmarkerTimer = 0.0f;
    bool hitmarkerIsHeadshot = false;

    void addPopup(const Vec3& pos, int dmg, bool headshot) {
        DamagePopup p;
        p.worldPos = pos;
        p.damage = dmg;
        p.isHeadshot = headshot;
        popups.push_back(p);
    }

    void addTracer(const Vec3& start, const Vec3& end) {
        BulletTracer t;
        t.start = start;
        t.end = end;
        tracers.push_back(t);
    }

    void addKillFeed(const std::string& killer, const std::string& victim, const std::string& weapon, bool headshot) {
        KillFeedEntry k;
        k.killer = killer;
        k.victim = victim;
        k.weapon = weapon;
        k.isHeadshot = headshot;
        killFeed.push_back(k);
    }

    void triggerHitmarker(bool headshot) {
        hitmarkerTimer = 0.15f;
        hitmarkerIsHeadshot = headshot;
    }

    void update(float dt) {
        if (hitmarkerTimer > 0.0f) hitmarkerTimer -= dt;

        for (size_t i = 0; i < popups.size(); ) {
            popups[i].timer += dt;
            popups[i].offsetY += dt * 1.8f;
            if (popups[i].timer >= popups[i].maxTime) {
                popups[i] = popups.back();
                popups.pop_back();
            } else {
                ++i;
            }
        }

        for (size_t i = 0; i < tracers.size(); ) {
            tracers[i].life += dt;
            if (tracers[i].life >= tracers[i].maxLife) {
                tracers[i] = tracers.back();
                tracers.pop_back();
            } else {
                ++i;
            }
        }

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

    void clear() {
        popups.clear();
        tracers.clear();
        killFeed.clear();
        hitmarkerTimer = 0.0f;
    }
};

} // namespace Combat

#endif // COMBAT_DAMAGEPOPUPSYSTEM_HPP
