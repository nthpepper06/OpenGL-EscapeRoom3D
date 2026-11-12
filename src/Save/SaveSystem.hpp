#ifndef SAVE_SAVESYSTEM_HPP
#define SAVE_SAVESYSTEM_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

namespace Save {

struct PlayerProfile {
    std::string playerName = "OPERATOR_07";
    int level = 1;
    int currentXP = 0;
    int maxXP = 500;
    int credits = 1500;
    int totalMatches = 0;
    int totalWins = 0;
    int totalKills = 0;

    void addXP(int xp) {
        currentXP += xp;
        while (currentXP >= maxXP) {
            currentXP -= maxXP;
            level++;
            maxXP = (int)(maxXP * 1.35f);
            credits += 250;
        }
    }
};

struct GameSettings {
    float lookSensitivity = 1.0f;
    float adsSensitivity = 0.6f;
    float sfxVolume = 1.0f;
    float musicVolume = 0.7f;
    int graphicsQuality = 2; // 0: Low, 1: Medium, 2: Ultra
    bool enableShadows = true;
    bool enableBloom = true;
    bool mobileTouchControls = true; // Displays touch virtual controls overlay
};

class SaveSystem {
public:
    PlayerProfile profile;
    GameSettings settings;

    static SaveSystem& instance() {
        static SaveSystem sys;
        return sys;
    }

    void saveToFile(const std::string& filename = "save_profile.dat") {
        std::ofstream file(filename);
        if (!file.is_open()) return;

        file << profile.playerName << "\n";
        file << profile.level << "\n";
        file << profile.currentXP << "\n";
        file << profile.maxXP << "\n";
        file << profile.credits << "\n";
        file << profile.totalMatches << "\n";
        file << profile.totalWins << "\n";
        file << profile.totalKills << "\n";
        file << settings.lookSensitivity << "\n";
        file << settings.adsSensitivity << "\n";
        file << settings.sfxVolume << "\n";
        file << settings.musicVolume << "\n";
        file << settings.graphicsQuality << "\n";
        file << settings.enableShadows << "\n";
        file.close();
    }

    void loadFromFile(const std::string& filename = "save_profile.dat") {
        std::ifstream file(filename);
        if (!file.is_open()) return;

        std::string line;
        if (std::getline(file, line)) profile.playerName = line;
        if (std::getline(file, line)) profile.level = std::stoi(line);
        if (std::getline(file, line)) profile.currentXP = std::stoi(line);
        if (std::getline(file, line)) profile.maxXP = std::stoi(line);
        if (std::getline(file, line)) profile.credits = std::stoi(line);
        if (std::getline(file, line)) profile.totalMatches = std::stoi(line);
        if (std::getline(file, line)) profile.totalWins = std::stoi(line);
        if (std::getline(file, line)) profile.totalKills = std::stoi(line);
        if (std::getline(file, line)) settings.lookSensitivity = std::stof(line);
        if (std::getline(file, line)) settings.adsSensitivity = std::stof(line);
        if (std::getline(file, line)) settings.sfxVolume = std::stof(line);
        if (std::getline(file, line)) settings.musicVolume = std::stof(line);
        if (std::getline(file, line)) settings.graphicsQuality = std::stoi(line);
        if (std::getline(file, line)) settings.enableShadows = (std::stoi(line) != 0);
        file.close();
    }
};

} // namespace Save

#endif // SAVE_SAVESYSTEM_HPP
