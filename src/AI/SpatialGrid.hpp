#ifndef AI_SPATIALGRID_HPP
#define AI_SPATIALGRID_HPP

#include <vector>
#include <unordered_map>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include "../Math3D.hpp"

namespace AI {

struct SpatialEntity {
    int id;
    Vec3 position;
    bool isPlayer;
    bool isAlive;
};

class SpatialGrid {
public:
    float cellSize;
    std::unordered_map<int64_t, std::vector<SpatialEntity>> grid;

    SpatialGrid(float cSize = 40.0f) : cellSize(cSize) {}

    void clear() {
        grid.clear();
    }

    int64_t getCellKey(int cx, int cz) const {
        return ((int64_t)cx << 32) | ((uint32_t)cz);
    }

    void insert(int id, const Vec3& pos, bool isPlayer, bool isAlive) {
        if (!isAlive) return;
        int cx = (int)std::floor(pos.x / cellSize);
        int cz = (int)std::floor(pos.z / cellSize);
        int64_t key = getCellKey(cx, cz);
        grid[key].push_back({ id, pos, isPlayer, isAlive });
    }

    std::vector<SpatialEntity> queryRadius(const Vec3& center, float radius) const {
        std::vector<SpatialEntity> results;
        int minCX = (int)std::floor((center.x - radius) / cellSize);
        int maxCX = (int)std::floor((center.x + radius) / cellSize);
        int minCZ = (int)std::floor((center.z - radius) / cellSize);
        int maxCZ = (int)std::floor((center.z + radius) / cellSize);

        float radiusSq = radius * radius;

        for (int cx = minCX; cx <= maxCX; ++cx) {
            for (int cz = minCZ; cz <= maxCZ; ++cz) {
                int64_t key = getCellKey(cx, cz);
                auto it = grid.find(key);
                if (it != grid.end()) {
                    for (const auto& ent : it->second) {
                        float dx = ent.position.x - center.x;
                        float dz = ent.position.z - center.z;
                        if (dx * dx + dz * dz <= radiusSq) {
                            results.push_back(ent);
                        }
                    }
                }
            }
        }
        return results;
    }
};

} // namespace AI

#endif // AI_SPATIALGRID_HPP
