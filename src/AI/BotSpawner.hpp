#ifndef AI_BOTSPAWNER_HPP
#define AI_BOTSPAWNER_HPP

#include <vector>
#include <string>
#include "BotController.hpp"

namespace AI {

class BotSpawner {
public:
    std::vector<BotController> bots;

    void spawnRoster(int count = 99) {
        bots.clear();
        std::vector<std::string> prefixes = {
            "Bong_Ma", "Bao_Den", "Xa_Thu", "Soi_Bang", "Dai_Bang", "Phong_Ba", "Hac_Bao", "Kestrel",
            "Viper", "Spectre", "Ronin", "Apex", "Lynx", "Havoc", "Echo", "Titan", "Phantom", "Zero",
            "Ghost", "Falcon", "Hawk", "Storm", "Blaze", "Frost", "Reaper", "Cobra", "Omega", "Alpha"
        };
        std::vector<std::string> suffixes = {
            "VN", "Pro", "King", "Sniper", "FF", "Zero", "God", "Solo", "99", "77", "88", "36", "Prime", "Top1"
        };

        for (int i = 0; i < count; ++i) {
            std::string name = prefixes[i % prefixes.size()] + "_" + suffixes[(i * 3 + 1) % suffixes.size()] + "_" + std::to_string(i + 1);

            // Scatter bots across the 1000m island (radius 40m - 420m)
            float angle = (float)i / count * 2.0f * PI + ((rand() % 100) / 400.0f);
            float dist = 35.0f + (rand() % 390);
            float spawnY = 135.0f + (rand() % 25);
            Vec3 spawnPos(std::cos(angle) * dist, spawnY, std::sin(angle) * dist);

            BotController bot(i + 1, name, spawnPos);
            bot.yaw = (float)(rand() % 360) * DEG2RAD;
            bot.state = BotState::AIRBORNE;
            bots.push_back(bot);
        }
    }
};

} // namespace AI

#endif // AI_BOTSPAWNER_HPP
