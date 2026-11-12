#ifndef UI_UIMANAGER_HPP
#define UI_UIMANAGER_HPP

#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include "../Math3D.hpp"
#include "../Shader.hpp"
#include "../Mesh.hpp"
#include "../Match/MatchManager.hpp"
#include "../Player/PlayerController.hpp"
#include "../SafeZone/SafeZoneManager.hpp"
#include "../Combat/DamagePopupSystem.hpp"
#include "../Save/SaveSystem.hpp"
#include "../World/IslandLayout.hpp"
#include "../Vehicles/Vehicle.hpp"

namespace UI {

class UIManager {
public:
    Shader uiShader;
    Mesh meshQuad;
    bool showFullscreenMap = false;

    void toggleFullscreenMap() {
        showFullscreenMap = !showFullscreenMap;
    }

    void init() {
        uiShader.compile(Shaders::uiVertex, Shaders::uiFragment);
        meshQuad = MeshGen::createQuad2D(1.0f, 1.0f);
    }

    void drawRect(float x, float y, float w, float h, Vec4 color, Mat4 orthoProj, int shapeType = 0) {
        uiShader.use();
        uiShader.setInt("uShapeType", shapeType);
        uiShader.setVec4("uColor", color);

        Mat4 model = Mat4::translate(Vec3(x, y, 0)) * Mat4::scale(Vec3(w, h, 1));
        uiShader.setMat4("uProjection", orthoProj * model);
        meshQuad.draw();
    }

    void drawCircle(float cx, float cy, float radius, Vec4 color, Mat4 orthoProj) {
        drawRect(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, color, orthoProj, 1);
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
            case '$': rows[0]=0x04; rows[1]=0x1E; rows[2]=0x14; rows[3]=0x0E; rows[4]=0x05; rows[5]=0x0F; rows[6]=0x04; break;
            case '%': rows[0]=0x19; rows[1]=0x19; rows[2]=0x02; rows[3]=0x04; rows[4]=0x08; rows[5]=0x13; rows[6]=0x13; break;
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

    // 1. Splash Screen
    void renderSplash(int sw, int sh, Mat4 orthoProj) {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float w = (float)sw, h = (float)sh;
        drawRect(0, 0, w, h, Vec4(0.04f, 0.05f, 0.08f, 0.75f), orthoProj);

        float pulse = 1.0f + 0.04f * std::sin((float)glfwGetTime() * 4.0f);
        drawText("VANGUARD", w * 0.5f - 180.0f * pulse, h * 0.5f + 40.0f, 56.0f * pulse, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        drawText("ZERO HORIZON", w * 0.5f - 140.0f, h * 0.5f - 15.0f, 26.0f, Vec4(0.2f, 0.85f, 1.0f, 1.0f), orthoProj);
        drawText("BERMUDA MILITARY ISLAND - 100 PLAYERS BR", w * 0.5f - 200.0f, h * 0.5f - 55.0f, 14.0f, Vec4(0.7f, 0.8f, 0.9f, 0.85f), orthoProj);

        float blink = (std::sin((float)glfwGetTime() * 6.0f) > 0.0f) ? 1.0f : 0.3f;
        drawText("[TAP ANYWHERE / PRESS SPACE TO ENTER]", w * 0.5f - 190.0f, 85.0f, 15.0f, Vec4(1.0f, 1.0f, 1.0f, blink), orthoProj);

        glEnable(GL_DEPTH_TEST);
    }

    // 2. Main Menu / Lobby Screen
    void renderLobby(int sw, int sh, Mat4 orthoProj, const Save::PlayerProfile& profile) {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float w = (float)sw, h = (float)sh;

        // Top Profile Banner
        drawRect(20.0f, h - 80.0f, 320.0f, 65.0f, Vec4(0.08f, 0.12f, 0.18f, 0.92f), orthoProj);
        drawRect(20.0f, h - 80.0f, 6.0f, 65.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        drawText(profile.playerName, 36.0f, h - 38.0f, 18.0f, Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj);
        drawText("LEVEL " + std::to_string(profile.level) + " | $" + std::to_string(profile.credits), 36.0f, h - 62.0f, 14.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);

        // Stats Summary (Top-Right)
        drawRect(w - 280.0f, h - 80.0f, 260.0f, 65.0f, Vec4(0.08f, 0.12f, 0.18f, 0.92f), orthoProj);
        drawText("MATCHES: " + std::to_string(profile.totalMatches) + " | WINS: " + std::to_string(profile.totalWins), w - 265.0f, h - 38.0f, 13.0f, Vec4(0.3f, 0.85f, 1.0f, 1.0f), orthoProj);
        drawText("TOTAL KILLS: " + std::to_string(profile.totalKills), w - 265.0f, h - 62.0f, 13.0f, Vec4(1.0f, 0.4f, 0.4f, 1.0f), orthoProj);

        // Center Game Mode Card (Bottom-Left)
        float cardW = 360.0f, cardH = 140.0f;
        float cardX = 20.0f, cardY = 100.0f;
        drawRect(cardX, cardY, cardW, cardH, Vec4(0.08f, 0.12f, 0.18f, 0.92f), orthoProj);
        drawRect(cardX, cardY + cardH - 4.0f, cardW, 4.0f, Vec4(0.2f, 0.85f, 1.0f, 1.0f), orthoProj);
        drawText("VANGUARD BR - 100 SOLO", cardX + 20.0f, cardY + 105.0f, 20.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        drawText("MAP: MILITARY ISLAND (1000M)", cardX + 20.0f, cardY + 75.0f, 13.0f, Vec4(0.85f, 0.9f, 0.95f, 0.9f), orthoProj);
        drawText("100 COMBATANTS - SURVIVE TO #1", cardX + 20.0f, cardY + 45.0f, 13.0f, Vec4(0.3f, 0.85f, 1.0f, 0.9f), orthoProj);

        // Big Gold START MATCH Button (Bottom Center)
        float btnW = 320.0f, btnH = 65.0f;
        float btnX = (w - btnW) * 0.5f, btnY = 55.0f;
        drawRect(btnX, btnY, btnW, btnH, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        drawRect(btnX + 4.0f, btnY + 4.0f, btnW - 8.0f, btnH - 8.0f, Vec4(0.08f, 0.1f, 0.14f, 0.95f), orthoProj);
        drawText("START MATCH", btnX + 75.0f, btnY + 22.0f, 24.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);

        // Bottom Controls Guide
        drawText("[CLICK / PRESS SPACE / F TO START]   [F3] CHEATS   [M] MAP", w * 0.5f - 240.0f, 20.0f, 13.0f, Vec4(0.6f, 0.7f, 0.8f, 0.8f), orthoProj);

        glEnable(GL_DEPTH_TEST);
    }

    // 3. Matchmaking Screen
    void renderMatchmaking(int sw, int sh, Mat4 orthoProj, float timer) {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float w = (float)sw, h = (float)sh;
        drawRect(0, 0, w, h, Vec4(0.04f, 0.06f, 0.1f, 0.88f), orthoProj);

        float mW = 460.0f, mH = 180.0f;
        float mX = (w - mW) * 0.5f, mY = (h - mH) * 0.5f;
        drawRect(mX, mY, mW, mH, Vec4(0.08f, 0.12f, 0.18f, 0.95f), orthoProj);
        drawRect(mX, mY + mH - 4.0f, mW, 4.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);

        drawText("MATCHMAKING 100 PLAYERS", mX + 65.0f, mY + 125.0f, 22.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        drawText("CONNECTING TO MILITARY ISLAND...", mX + 85.0f, mY + 85.0f, 14.0f, Vec4(0.3f, 0.85f, 1.0f, 1.0f), orthoProj);

        int dots = (int)(timer * 3.0f) % 4;
        std::string dotStr = "GATHERING 100 OPERATORS";
        for (int i = 0; i < dots; ++i) dotStr += ".";
        drawText(dotStr, mX + 105.0f, mY + 50.0f, 14.0f, Vec4(0.85f, 0.9f, 1.0f, 0.9f), orthoProj);

        glEnable(GL_DEPTH_TEST);
    }

    // 4. In-Game HUD
    void renderHUD(int sw, int sh, Mat4 orthoProj, const Player::PlayerController& player, const SafeZone::SafeZoneManager& zone, int aliveCount, int killCount, float zoneTimer, bool isZoneShrinking, int nearestLootIdx, const std::string& lootName, float hitmarkerTimer, bool hitmarkerHeadshot, int activeVehicleIdx = -1, const std::string& vehiclePrompt = "", float vehicleSpeedKmh = 0.0f, float vehicleHp = 0.0f, float vehicleMaxHp = 1.0f, int nearestMushroomIdx = -1, int mushroomTier = 1, bool isUAVActive = false, float uavTimeLeft = 0.0f, bool isAirdropIncoming = false, const std::vector<Vec3>& uavBlips = {}) {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float w = (float)sw, h = (float)sh;

        // A. Crosshair & Hitmarker
        if (player.state == Player::LocomotionState::GROUNDED && activeVehicleIdx < 0) {
            float cx = w * 0.5f;
            float cy = h * 0.5f;
            float chSpread = player.isAiming ? 3.5f : (player.velocity.lengthSq() > 1.0f ? 14.0f : 7.0f);

            drawRect(cx - 1.0f, cy + chSpread, 2.0f, 8.0f, Vec4(1, 1, 1, 0.9f), orthoProj);
            drawRect(cx - 1.0f, cy - chSpread - 8.0f, 2.0f, 8.0f, Vec4(1, 1, 1, 0.9f), orthoProj);
            drawRect(cx + chSpread, cy - 1.0f, 8.0f, 2.0f, Vec4(1, 1, 1, 0.9f), orthoProj);
            drawRect(cx - chSpread - 8.0f, cy - 1.0f, 8.0f, 2.0f, Vec4(1, 1, 1, 0.9f), orthoProj);
            drawRect(cx - 1.0f, cy - 1.0f, 2.0f, 2.0f, Vec4(1, 0.2f, 0.2f, 0.95f), orthoProj);

            if (hitmarkerTimer > 0.0f) {
                Vec4 hmCol = hitmarkerHeadshot ? Vec4(1.0f, 0.15f, 0.15f, 1.0f) : Vec4(1.0f, 0.95f, 0.2f, 1.0f);
                drawRect(cx - 8.0f, cy - 8.0f, 4.0f, 4.0f, hmCol, orthoProj);
                drawRect(cx + 4.0f, cy - 8.0f, 4.0f, 4.0f, hmCol, orthoProj);
                drawRect(cx - 8.0f, cy + 4.0f, 4.0f, 4.0f, hmCol, orthoProj);
                drawRect(cx + 4.0f, cy + 4.0f, 4.0f, 4.0f, hmCol, orthoProj);
            }
        }

        // B. Skydiving Altitude Prompt
        if (player.state != Player::LocomotionState::GROUNDED) {
            float promptW = 280.0f, promptX = (w - promptW) * 0.5f, promptY = 120.0f;
            drawRect(promptX, promptY, promptW, 44.0f, Vec4(0.06f, 0.08f, 0.12f, 0.92f), orthoProj);
            drawRect(promptX, promptY + 41.0f, promptW, 3.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
            drawText("ALTITUDE: " + std::to_string((int)player.position.y) + "M", promptX + 75.0f, promptY + 24.0f, 14.0f, Vec4(1.0f, 1.0f, 1.0f, 0.95f), orthoProj);
            if (player.state == Player::LocomotionState::FREEFALL) {
                drawText("[F / SPACE] DEPLOY CHUTE", promptX + 45.0f, promptY + 8.0f, 14.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
            } else {
                drawText("GLIDING TO DROP ZONE", promptX + 55.0f, promptY + 8.0f, 14.0f, Vec4(0.2f, 0.85f, 1.0f, 1.0f), orthoProj);
            }
        }

        // C. Vehicle Driving Dashboard OR Prompts
        if (activeVehicleIdx >= 0) {
            // DRIVING SPEEDOMETER & DASHBOARD
            float dashW = 420.0f, dashH = 72.0f;
            float dashX = (w - dashW) * 0.5f, dashY = 125.0f;
            drawRect(dashX, dashY, dashW, dashH, Vec4(0.06f, 0.08f, 0.12f, 0.94f), orthoProj);
            drawRect(dashX, dashY + dashH - 3.0f, dashW, 3.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);

            // Speedometer readout
            std::string speedStr = "SPEED: " + std::to_string((int)vehicleSpeedKmh) + " KM/H";
            drawText(speedStr, dashX + 20.0f, dashY + 42.0f, 20.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);

            // Vehicle Health bar
            float vHpFrac = std::clamp(vehicleHp / std::max(1.0f, vehicleMaxHp), 0.0f, 1.0f);
            drawRect(dashX + 220.0f, dashY + 44.0f, 180.0f, 14.0f, Vec4(0.1f, 0.1f, 0.14f, 0.9f), orthoProj);
            drawRect(dashX + 220.0f, dashY + 44.0f, 180.0f * vHpFrac, 14.0f, Vec4(0.2f, 0.85f, 0.35f, 1.0f), orthoProj);
            drawText("VEHICLE HP: " + std::to_string((int)vehicleHp), dashX + 230.0f, dashY + 46.0f, 10.0f, Vec4(0, 0, 0, 1.0f), orthoProj);

            // Control prompts
            drawText("[F / ENTER] EXIT   [W] GAS   [S] BRAKE   [A/D] STEER   [SHIFT] NITRO", dashX + 15.0f, dashY + 14.0f, 12.0f, Vec4(0.7f, 0.85f, 1.0f, 0.9f), orthoProj);
        } else if (!vehiclePrompt.empty() && player.state == Player::LocomotionState::GROUNDED) {
            float vPromptW = 360.0f, vPromptX = (w - vPromptW) * 0.5f, vPromptY = 175.0f;
            drawRect(vPromptX, vPromptY, vPromptW, 40.0f, Vec4(0.06f, 0.10f, 0.16f, 0.95f), orthoProj);
            drawRect(vPromptX, vPromptY, 6.0f, 40.0f, Vec4(0.95f, 0.65f, 0.15f, 1.0f), orthoProj);
            drawText("[F / ENTER] DRIVE " + vehiclePrompt, vPromptX + 18.0f, vPromptY + 12.0f, 14.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        }

        // C2. Energy Mushroom Eat Prompt
        if (nearestMushroomIdx >= 0 && activeVehicleIdx < 0 && player.state == Player::LocomotionState::GROUNDED) {
            float mushW = 340.0f, mushX = (w - mushW) * 0.5f, mushY = 178.0f;
            Vec4 mCol = (mushroomTier == 1) ? Vec4(0.95f, 0.52f, 0.12f, 1.0f) :
                        ((mushroomTier == 2) ? Vec4(0.98f, 0.85f, 0.15f, 1.0f) : Vec4(0.75f, 0.22f, 0.95f, 1.0f));
            std::string epAmt = (mushroomTier == 1) ? "+50 EP" : ((mushroomTier == 2) ? "+100 EP" : "+200 EP");
            drawRect(mushX, mushY, mushW, 42.0f, Vec4(0.05f, 0.08f, 0.14f, 0.95f), orthoProj);
            drawRect(mushX, mushY, 6.0f, 42.0f, mCol, orthoProj);
            drawText("[F] EAT MUSHROOM (" + epAmt + ")", mushX + 18.0f, mushY + 14.0f, 14.0f, mCol, orthoProj);
        }

        // C3. Nearest Loot Pickup Prompt
        if (nearestLootIdx >= 0 && activeVehicleIdx < 0 && player.state == Player::LocomotionState::GROUNDED) {
            float lootW = 340.0f, lootX = (w - lootW) * 0.5f, lootY = 130.0f;
            drawRect(lootX, lootY, lootW, 42.0f, Vec4(0.05f, 0.08f, 0.14f, 0.92f), orthoProj);
            drawRect(lootX, lootY, 6.0f, 42.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
            drawText("[F] LOOT: " + lootName, lootX + 18.0f, lootY + 14.0f, 14.0f, Vec4(1.0f, 0.9f, 0.2f, 1.0f), orthoProj);
        }

        // D. Health, EP, Shield & Armor Bars (Bottom Center)
        float barW = 320.0f, barH = 18.0f, barX = (w - barW) * 0.5f, barY = 32.0f;

        // Armor & Helmet & Backpack HUD Card (Left of Health Bar)
        drawRect(barX - 145.0f, barY - 2.0f, 138.0f, 66.0f, Vec4(0.06f, 0.08f, 0.12f, 0.94f), orthoProj);
        drawRect(barX - 145.0f, barY - 2.0f, 3.0f, 66.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);

        // Vest Row
        std::string vestStr = (player.inventory.vestLevel == 1) ? "VEST L1 (+30%)" :
                              ((player.inventory.vestLevel == 2) ? "VEST L2 (+45%)" :
                              ((player.inventory.vestLevel >= 3) ? "VEST L3 (+65%)" : "NO VEST"));
        Vec4 vestCol = (player.inventory.vestLevel == 1) ? Vec4(0.35f, 0.85f, 0.35f, 1.0f) :
                       ((player.inventory.vestLevel == 2) ? Vec4(0.3f, 0.75f, 1.0f, 1.0f) :
                       ((player.inventory.vestLevel >= 3) ? Vec4(0.95f, 0.75f, 0.15f, 1.0f) : Vec4(0.6f, 0.6f, 0.6f, 0.8f)));
        drawText(vestStr, barX - 138.0f, barY + 46.0f, 11.0f, vestCol, orthoProj);

        // Helmet Row
        std::string helmStr = (player.inventory.helmetLevel == 1) ? "HELM L1 (+25%)" :
                              ((player.inventory.helmetLevel == 2) ? "HELM L2 (+40%)" :
                              ((player.inventory.helmetLevel >= 3) ? "HELM L3 (+55%)" : "NO HELMET"));
        Vec4 helmCol = (player.inventory.helmetLevel == 1) ? Vec4(0.35f, 0.85f, 0.35f, 1.0f) :
                       ((player.inventory.helmetLevel == 2) ? Vec4(0.3f, 0.75f, 1.0f, 1.0f) :
                       ((player.inventory.helmetLevel >= 3) ? Vec4(0.95f, 0.75f, 0.15f, 1.0f) : Vec4(0.6f, 0.6f, 0.6f, 0.8f)));
        drawText(helmStr, barX - 138.0f, barY + 28.0f, 11.0f, helmCol, orthoProj);

        // Backpack Row
        std::string bagStr = (player.inventory.backpackLevel == 1) ? "BAG L1 (150)" :
                             ((player.inventory.backpackLevel == 2) ? "BAG L2 (250)" : "BAG L3 (400)");
        drawText(bagStr, barX - 138.0f, barY + 10.0f, 11.0f, Vec4(0.95f, 0.75f, 0.2f, 1.0f), orthoProj);

        // Shield Bar (Top of stack)
        drawRect(barX - 2.0f, barY + barH + 20.0f, barW + 4.0f, 10.0f, Vec4(0.05f, 0.05f, 0.08f, 0.9f), orthoProj);
        float shFrac = std::clamp(player.health.shield / player.health.maxShield, 0.0f, 1.0f);
        drawRect(barX, barY + barH + 21.0f, barW * shFrac, 8.0f, Vec4(0.2f, 0.8f, 1.0f, 1.0f), orthoProj);
        drawText("SHIELD " + std::to_string((int)player.health.shield) + "/" + std::to_string((int)player.health.maxShield), barX + 105.0f, barY + barH + 21.0f, 9.0f, Vec4(1.0f, 1.0f, 1.0f, 0.95f), orthoProj);

        // Yellow EP Bar (Energy Points - Free Fire style auto HP regen)
        drawRect(barX - 2.0f, barY + barH + 4.0f, barW + 4.0f, 12.0f, Vec4(0.05f, 0.05f, 0.08f, 0.9f), orthoProj);
        float epFrac = std::clamp(player.health.ep / player.health.maxEP, 0.0f, 1.0f);
        drawRect(barX, barY + barH + 5.0f, barW * epFrac, 10.0f, Vec4(0.95f, 0.82f, 0.15f, 1.0f), orthoProj);
        drawText("EP " + std::to_string((int)player.health.ep) + " / " + std::to_string((int)player.health.maxEP), barX + 115.0f, barY + barH + 6.0f, 10.0f, Vec4(0, 0, 0, 1.0f), orthoProj);

        // Health Bar Frame & Fill
        drawRect(barX - 2.0f, barY - 2.0f, barW + 4.0f, barH + 4.0f, Vec4(0.05f, 0.05f, 0.08f, 0.9f), orthoProj);
        float hpFrac = std::clamp(player.health.currentHealth / player.health.maxHealth, 0.0f, 1.0f);
        Vec4 hpColor = (hpFrac > 0.5f) ? Vec4(0.2f, 0.9f, 0.35f, 1.0f) : ((hpFrac > 0.25f) ? Vec4(0.95f, 0.8f, 0.15f, 1.0f) : Vec4(0.95f, 0.2f, 0.2f, 1.0f));
        drawRect(barX, barY, barW * hpFrac, barH, hpColor, orthoProj);
        drawText("HP " + std::to_string((int)player.health.currentHealth) + " / " + std::to_string((int)player.health.maxHealth), barX + 95.0f, barY + 3.0f, 13.0f, Vec4(0, 0, 0, 1.0f), orthoProj);

        // E. Weapon HUD Card (Bottom-Right)
        float wpHudX = w - 215.0f, wpHudY = 30.0f;
        drawRect(wpHudX, wpHudY, 195.0f, 65.0f, Vec4(0.06f, 0.08f, 0.12f, 0.92f), orthoProj);
        drawRect(wpHudX, wpHudY + 62.0f, 195.0f, 3.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);

        Weapons::WeaponSpec spec = player.weapons.getSpec();
        drawText(spec.displayName, wpHudX + 10.0f, wpHudY + 40.0f, 16.0f, Vec4(1.0f, 0.9f, 0.2f, 1.0f), orthoProj);

        std::string ammoStr = (player.weapons.currentWeaponId == Weapons::WeaponID::UNARMED) ? "NO WEAPON" :
                              (player.weapons.isReloading ? "RELOADING..." : (std::to_string(player.weapons.currentMag) + " / " + std::to_string(player.weapons.reserveAmmo)));
        drawText(ammoStr, wpHudX + 14.0f, wpHudY + 14.0f, 15.0f, Vec4(1.0f, 1.0f, 1.0f, 0.95f), orthoProj);

        // Tactical Throwables & Items
        // 1. Ice Barrier
        drawRect(wpHudX - 80.0f, wpHudY, 70.0f, 65.0f, Vec4(0.08f, 0.22f, 0.35f, 0.92f), orthoProj);
        drawText("[G] ICE", wpHudX - 75.0f, wpHudY + 40.0f, 13.0f, Vec4(0.4f, 0.85f, 1.0f, 1.0f), orthoProj);
        drawText("X" + std::to_string(player.inventory.barrierCount), wpHudX - 60.0f, wpHudY + 14.0f, 20.0f, Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj);

        // 2. Frag Grenade
        drawRect(wpHudX - 155.0f, wpHudY, 70.0f, 65.0f, Vec4(0.28f, 0.14f, 0.08f, 0.92f), orthoProj);
        drawText("[4] NADE", wpHudX - 152.0f, wpHudY + 40.0f, 11.0f, Vec4(0.95f, 0.55f, 0.2f, 1.0f), orthoProj);
        drawText("X" + std::to_string(player.inventory.grenadeCount), wpHudX - 138.0f, wpHudY + 14.0f, 20.0f, Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj);

        // 3. Medkit
        drawRect(wpHudX - 230.0f, wpHudY, 70.0f, 65.0f, Vec4(0.12f, 0.32f, 0.18f, 0.92f), orthoProj);
        drawText("[5] MED", wpHudX - 225.0f, wpHudY + 40.0f, 12.0f, Vec4(0.3f, 1.0f, 0.4f, 1.0f), orthoProj);
        drawText("X" + std::to_string(player.inventory.medkitCount), wpHudX - 212.0f, wpHudY + 14.0f, 20.0f, Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj);

        // F. Character Skill Active Card (Bottom-Right, above Weapon HUD)
        auto skillSpec = player.skills.getCurrentSpec();
        float skW = 215.0f, skH = 50.0f, skX = w - 235.0f, skY = 105.0f;
        drawRect(skX, skY, skW, skH, Vec4(0.06f, 0.08f, 0.12f, 0.92f), orthoProj);
        drawRect(skX, skY, 5.0f, skH, skillSpec.themeColor, orthoProj);

        if (player.skills.isActive()) {
            drawText("[ACTIVE] " + skillSpec.shortName, skX + 14.0f, skY + 28.0f, 14.0f, skillSpec.themeColor, orthoProj);
            drawText("TIME: " + std::to_string((int)player.skills.activeTimer + 1) + "S", skX + 14.0f, skY + 10.0f, 13.0f, Vec4(1, 1, 1, 1), orthoProj);
        } else if (player.skills.isReady()) {
            drawText("[E] " + skillSpec.shortName + " (READY)", skX + 14.0f, skY + 28.0f, 13.0f, skillSpec.themeColor, orthoProj);
            drawText("[K] CHANGE SKILL", skX + 14.0f, skY + 10.0f, 11.0f, Vec4(0.7f, 0.8f, 0.9f, 0.85f), orthoProj);
        } else {
            drawText(skillSpec.shortName + " CD: " + std::to_string((int)player.skills.cooldownTimer + 1) + "S", skX + 14.0f, skY + 28.0f, 13.0f, Vec4(0.6f, 0.6f, 0.6f, 0.85f), orthoProj);
            float cdProg = player.skills.getCooldownProgress();
            drawRect(skX + 14.0f, skY + 12.0f, 180.0f * cdProg, 6.0f, skillSpec.themeColor, orthoProj);
        }

        // G. Alive & Kill Counters (Top-Left)
        drawRect(20.0f, h - 65.0f, 160.0f, 45.0f, Vec4(0.06f, 0.08f, 0.12f, 0.92f), orthoProj);
        drawText("ALIVE: " + std::to_string(aliveCount) + " / 100", 28.0f, h - 42.0f, 14.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        drawText("KILLS: " + std::to_string(killCount), 28.0f, h - 58.0f, 13.0f, Vec4(1.0f, 0.3f, 0.3f, 1.0f), orthoProj);

        // H. Safe Zone / UAV / Airdrop Warning Banners (Top-Center)
        if (isUAVActive) {
            float uavW = 380.0f, uavX = (w - uavW) * 0.5f;
            drawRect(uavX, h - 55.0f, uavW, 30.0f, Vec4(0.12f, 0.18f, 0.32f, 0.95f), orthoProj);
            drawRect(uavX, h - 55.0f, uavW, 3.0f, Vec4(0.2f, 0.85f, 1.0f, 1.0f), orthoProj);
            drawText("UAV RECON SCANNING - " + std::to_string((int)uavTimeLeft) + "S", uavX + 45.0f, h - 46.0f, 14.0f, Vec4(0.2f, 0.85f, 1.0f, 1.0f), orthoProj);
        } else if (isAirdropIncoming) {
            float adW = 380.0f, adX = (w - adW) * 0.5f;
            drawRect(adX, h - 55.0f, adW, 30.0f, Vec4(0.32f, 0.18f, 0.08f, 0.95f), orthoProj);
            drawRect(adX, h - 55.0f, adW, 3.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
            drawText("C-130 AIRDROP INCOMING - GOLDEN CRATE", adX + 25.0f, h - 46.0f, 13.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        } else if (isZoneShrinking) {
            float warnW = 280.0f, warnX = (w - warnW) * 0.5f;
            drawRect(warnX, h - 55.0f, warnW, 30.0f, Vec4(0.9f, 0.15f, 0.15f, 0.92f), orthoProj);
            drawText("ZONE SHRINKING!", warnX + 60.0f, h - 46.0f, 15.0f, Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj);
        } else {
            std::string timeStr = "ZONE SHRINKS IN: " + std::to_string((int)zoneTimer) + "S";
            drawText(timeStr, w * 0.5f - 95.0f, h - 45.0f, 14.0f, Vec4(0.4f, 0.85f, 1.0f, 0.9f), orthoProj);
        }

        // I. Radar Minimap (Top-Left under counter, calibrated for 1000m island)
        renderMinimap(sw, sh, orthoProj, player.position, zone.currentCenter, zone.currentRadius, zone.nextCenter, zone.nextRadius, isUAVActive, uavBlips);

        // J. High Visibility Map Prompt Button
        drawRect(20.0f, h - 245.0f, 140.0f, 22.0f, Vec4(0.08f, 0.14f, 0.22f, 0.95f), orthoProj);
        drawRect(20.0f, h - 245.0f, 140.0f, 2.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        drawText("[M / TAB] MAP", 28.0f, h - 238.0f, 11.0f, Vec4(0.95f, 0.85f, 0.2f, 1.0f), orthoProj);

        // K. Mobile Virtual Touch Joystick Simulation Overlay
        drawCircle(player.mobileControls.moveStick.centerX, player.mobileControls.moveStick.centerY, player.mobileControls.moveStick.radius, Vec4(0.1f, 0.15f, 0.22f, 0.45f), orthoProj);
        drawCircle(player.mobileControls.moveStick.handleX, player.mobileControls.moveStick.handleY, 26.0f, Vec4(0.95f, 0.75f, 0.15f, 0.65f), orthoProj);

        glEnable(GL_DEPTH_TEST);
    }

    // Radar Minimap (Top-Left corner with UAV Sonar Enemy Blips)
    void renderMinimap(int sw, int sh, Mat4 orthoProj, const Vec3& pPos, const Vec3& zCenter, float zRadius, const Vec3& nzCenter, float nzRadius, bool isUAVActive = false, const std::vector<Vec3>& uavBlips = {}) {
        float rW = 140.0f, rH = 140.0f, rX = 20.0f, rY = (float)sh - 220.0f;
        float halfMap = 500.0f; // 1000m Island Extent

        drawRect(rX, rY, rW, rH, Vec4(0.12f, 0.20f, 0.16f, 0.88f), orthoProj);
        drawRect(rX, rY, rW, 2.0f, Vec4(0.9f, 0.9f, 0.9f, 0.8f), orthoProj);
        drawRect(rX, rY + rH - 2.0f, rW, 2.0f, Vec4(0.9f, 0.9f, 0.9f, 0.8f), orthoProj);
        drawRect(rX, rY, 2.0f, rH, Vec4(0.9f, 0.9f, 0.9f, 0.8f), orthoProj);
        drawRect(rX + rW - 2.0f, rY, 2.0f, rH, Vec4(0.9f, 0.9f, 0.9f, 0.8f), orthoProj);

        // Next Zone (White ring preview)
        float nzNormX = (nzCenter.x / halfMap) * 0.5f + 0.5f;
        float nzNormZ = (nzCenter.z / halfMap) * 0.5f + 0.5f;
        float nzRadFrac = (nzRadius / halfMap) * rW * 0.5f;
        drawRect(rX + nzNormX * rW - nzRadFrac, rY + (1.0f - nzNormZ) * rH - nzRadFrac, nzRadFrac * 2.0f, nzRadFrac * 2.0f, Vec4(1.0f, 1.0f, 1.0f, 0.25f), orthoProj, 1);

        // Current Safe Zone (Blue electric ring)
        float szNormX = (zCenter.x / halfMap) * 0.5f + 0.5f;
        float szNormZ = (zCenter.z / halfMap) * 0.5f + 0.5f;
        float szRadFrac = (zRadius / halfMap) * rW * 0.5f;
        drawRect(rX + szNormX * rW - szRadFrac, rY + (1.0f - szNormZ) * rH - szRadFrac, szRadFrac * 2.0f, szRadFrac * 2.0f, Vec4(0.2f, 0.6f, 1.0f, 0.35f), orthoProj, 1);

        // UAV Enemy Radar Blips (Red Sonar Dots)
        if (isUAVActive) {
            for (const auto& blip : uavBlips) {
                float bNormX = (blip.x / halfMap) * 0.5f + 0.5f;
                float bNormZ = (blip.z / halfMap) * 0.5f + 0.5f;
                drawRect(rX + bNormX * rW - 2.5f, rY + (1.0f - bNormZ) * rH - 2.5f, 5.0f, 5.0f, Vec4(1.0f, 0.15f, 0.15f, 1.0f), orthoProj, 1);
            }
        }

        // Player Position Dot
        float pNormX = (pPos.x / halfMap) * 0.5f + 0.5f;
        float pNormZ = (pPos.z / halfMap) * 0.5f + 0.5f;
        drawRect(rX + pNormX * rW - 3.5f, rY + (1.0f - pNormZ) * rH - 3.5f, 7.0f, 7.0f, Vec4(1.0f, 0.9f, 0.1f, 1.0f), orthoProj, 1);
    }

    // 5. Fullscreen Tactical Island Map Overlay [M / TAB]
    void renderFullscreenMap(int sw, int sh, Mat4 orthoProj, const Vec3& pPos, float pYaw, const SafeZone::SafeZoneManager& zone, const Vec3& airdropPos, const World::IslandLayout& layout, const std::vector<Vehicles::Vehicle>& vehicles = {}, bool isUAVActive = false, const std::vector<Vec3>& uavBlips = {}) {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float w = (float)sw, h = (float)sh;
        // Dark Dim Backdrop
        drawRect(0, 0, w, h, Vec4(0.02f, 0.04f, 0.07f, 0.88f), orthoProj);

        float mapSize = std::min(w * 0.75f, h * 0.85f);
        float mapX = (w - mapSize) * 0.5f;
        float mapY = (h - mapSize) * 0.5f;
        float halfMap = 500.0f; // 1000m Island Extent

        // Map Frame & Island Background
        drawRect(mapX, mapY, mapSize, mapSize, Vec4(0.08f, 0.16f, 0.12f, 0.98f), orthoProj);
        drawRect(mapX - 4.0f, mapY - 4.0f, mapSize + 8.0f, 4.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        drawRect(mapX - 4.0f, mapY + mapSize, mapSize + 8.0f, 4.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        drawRect(mapX - 4.0f, mapY - 4.0f, 4.0f, mapSize + 8.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        drawRect(mapX + mapSize, mapY - 4.0f, 4.0f, mapSize + 8.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);

        // Map Header & Footer
        drawText("MILITARY ISLAND - TACTICAL MAP [1000M]", mapX + 15.0f, mapY + mapSize + 12.0f, 16.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        drawText("[M / TAB / ESC] CLOSE MAP   [W,A,S,D] MOVE", mapX + 15.0f, mapY - 22.0f, 13.0f, Vec4(0.7f, 0.8f, 0.9f, 0.9f), orthoProj);

        // Road Network Lines (Connecting PEAK to other POIs)
        for (const auto& poi : layout.pois) {
            float px1 = mapX + 0.5f * mapSize;
            float py1 = mapY + 0.5f * mapSize;
            float px2 = mapX + (poi.center.x / halfMap * 0.5f + 0.5f) * mapSize;
            float py2 = mapY + (1.0f - (poi.center.z / halfMap * 0.5f + 0.5f)) * mapSize;

            float steps = 20.0f;
            for (float s = 0; s <= steps; s += 1.0f) {
                float t = s / steps;
                float rx = px1 + (px2 - px1) * t;
                float ry = py1 + (py2 - py1) * t;
                drawRect(rx - 1.5f, ry - 1.5f, 3.0f, 3.0f, Vec4(0.45f, 0.42f, 0.35f, 0.55f), orthoProj);
            }
        }

        // Grid Lines
        for (int g = 1; g < 6; ++g) {
            float frac = (float)g / 6.0f;
            drawRect(mapX + frac * mapSize, mapY, 1.0f, mapSize, Vec4(0.2f, 0.35f, 0.28f, 0.45f), orthoProj);
            drawRect(mapX, mapY + frac * mapSize, mapSize, 1.0f, Vec4(0.2f, 0.35f, 0.28f, 0.45f), orthoProj);
        }

        // 8 POI Badges & Names
        for (const auto& poi : layout.pois) {
            float px = mapX + (poi.center.x / halfMap * 0.5f + 0.5f) * mapSize;
            float py = mapY + (1.0f - (poi.center.z / halfMap * 0.5f + 0.5f)) * mapSize;

            drawRect(px - 22.0f, py - 12.0f, 44.0f, 24.0f, Vec4(0.06f, 0.08f, 0.12f, 0.92f), orthoProj);
            drawRect(px - 22.0f, py - 12.0f, 44.0f, 3.0f, poi.badgeColor, orthoProj);
            drawText(poi.code, px - 10.0f, py - 5.0f, 13.0f, poi.badgeColor, orthoProj);
            drawText(poi.name, px - (float)poi.name.length() * 4.2f, py - 26.0f, 12.0f, Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj);
        }

        // Vehicle Markers on Map
        for (const auto& v : vehicles) {
            if (!v.isAlive) continue;
            float vx = mapX + (v.position.x / halfMap * 0.5f + 0.5f) * mapSize;
            float vy = mapY + (1.0f - (v.position.z / halfMap * 0.5f + 0.5f)) * mapSize;
            drawRect(vx - 4.0f, vy - 4.0f, 8.0f, 8.0f, v.themeColor, orthoProj);
            drawText(v.type == Vehicles::VehicleType::MOTORBIKE ? "MOTO" : "JEEP", vx - 10.0f, vy + 5.0f, 8.0f, v.themeColor, orthoProj);
        }

        // Next Safe Zone (Yellow Preview Circle)
        float nzX = mapX + (zone.nextCenter.x / halfMap * 0.5f + 0.5f) * mapSize;
        float nzY = mapY + (1.0f - (zone.nextCenter.z / halfMap * 0.5f + 0.5f)) * mapSize;
        float nzR = (zone.nextRadius / halfMap) * mapSize * 0.5f;
        drawRect(nzX - nzR, nzY - nzR, nzR * 2.0f, nzR * 2.0f, Vec4(1.0f, 0.95f, 0.2f, 0.25f), orthoProj, 1);

        // Current Safe Zone (Cyan Electric Circle)
        float czX = mapX + (zone.currentCenter.x / halfMap * 0.5f + 0.5f) * mapSize;
        float czY = mapY + (1.0f - (zone.currentCenter.z / halfMap * 0.5f + 0.5f)) * mapSize;
        float czR = (zone.currentRadius / halfMap) * mapSize * 0.5f;
        drawRect(czX - czR, czY - czR, czR * 2.0f, czR * 2.0f, Vec4(0.2f, 0.8f, 1.0f, 0.35f), orthoProj, 1);

        // Airdrop Icon & Flare Beacon on Fullscreen Map
        float adX = mapX + (airdropPos.x / halfMap * 0.5f + 0.5f) * mapSize;
        float adY = mapY + (1.0f - (airdropPos.z / halfMap * 0.5f + 0.5f)) * mapSize;
        drawRect(adX - 7.0f, adY - 7.0f, 14.0f, 14.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        drawRect(adX - 4.0f, adY - 4.0f, 8.0f, 8.0f, Vec4(0.95f, 0.2f, 0.15f, 1.0f), orthoProj);
        drawText("AIRDROP [THINH]", adX - 35.0f, adY + 10.0f, 10.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);

        // UAV Enemy Radar Blips
        if (isUAVActive) {
            for (const auto& blip : uavBlips) {
                float bx = mapX + (blip.x / halfMap * 0.5f + 0.5f) * mapSize;
                float by = mapY + (1.0f - (blip.z / halfMap * 0.5f + 0.5f)) * mapSize;
                drawRect(bx - 4.5f, by - 4.5f, 9.0f, 9.0f, Vec4(1.0f, 0.15f, 0.15f, 1.0f), orthoProj, 1);
                drawText("ENEMY", bx - 14.0f, by + 6.0f, 8.0f, Vec4(1.0f, 0.2f, 0.2f, 1.0f), orthoProj);
            }
        }

        // Player Marker (Bright Gold Arrow)
        float plX = mapX + (pPos.x / halfMap * 0.5f + 0.5f) * mapSize;
        float plY = mapY + (1.0f - (pPos.z / halfMap * 0.5f + 0.5f)) * mapSize;
        drawRect(plX - 5.0f, plY - 5.0f, 10.0f, 10.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj, 1);
        float arrowX = plX + std::sin(pYaw) * 14.0f;
        float arrowY = plY - std::cos(pYaw) * 14.0f;
        drawRect(arrowX - 2.0f, arrowY - 2.0f, 4.0f, 4.0f, Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj);
        drawText("YOU", plX - 9.0f, plY - 20.0f, 11.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);

        glEnable(GL_DEPTH_TEST);
    }

    // 6. Match Results Screen
    void renderResults(int sw, int sh, Mat4 orthoProj, const Match::MatchManager& match, const Save::PlayerProfile& profile) {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float w = (float)sw, h = (float)sh;
        drawRect(0, 0, w, h, Vec4(0.04f, 0.06f, 0.1f, 0.96f), orthoProj);

        float resW = 560.0f, resH = 400.0f;
        float resX = (w - resW) * 0.5f, resY = (h - resH) * 0.5f;

        drawRect(resX, resY, resW, resH, Vec4(0.08f, 0.12f, 0.18f, 0.95f), orthoProj);
        bool isWin = (match.playerRank == 1);
        Vec4 topColor = isWin ? Vec4(0.95f, 0.75f, 0.15f, 1.0f) : Vec4(0.95f, 0.25f, 0.25f, 1.0f);
        drawRect(resX, resY + resH - 6.0f, resW, 6.0f, topColor, orthoProj);

        std::string title = isWin ? "BOOYAH! #1 VICTORY" : "MATCH COMPLETED";
        drawText(title, resX + (isWin ? 130.0f : 150.0f), resY + resH - 45.0f, 26.0f, topColor, orthoProj);

        float lineY = resY + resH - 95.0f;
        drawText("FINAL PLACEMENT: #" + std::to_string(match.playerRank) + " / 100", resX + 50.0f, lineY, 16.0f, Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj);
        lineY -= 32.0f;
        drawText("OPERATORS ELIMINATED: " + std::to_string(match.playerKills), resX + 50.0f, lineY, 15.0f, Vec4(1.0f, 0.4f, 0.4f, 1.0f), orthoProj);
        lineY -= 32.0f;
        drawText("TOTAL DAMAGE DEALT: " + std::to_string(match.totalDamageDealt), resX + 50.0f, lineY, 15.0f, Vec4(0.4f, 0.85f, 1.0f, 1.0f), orthoProj);
        lineY -= 40.0f;

        // Reward Breakdown Box
        drawRect(resX + 40.0f, lineY - 10.0f, resW - 80.0f, 65.0f, Vec4(0.05f, 0.08f, 0.12f, 0.9f), orthoProj);
        drawText("EARNED XP: +" + std::to_string(match.earnedXP) + " XP", resX + 60.0f, lineY + 24.0f, 16.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        drawText("EARNED CREDITS: +$" + std::to_string(match.earnedCredits), resX + 60.0f, lineY, 16.0f, Vec4(0.2f, 0.85f, 1.0f, 1.0f), orthoProj);

        // Return to Lobby Button
        float bW = 280.0f, bH = 50.0f;
        float bX = (w - bW) * 0.5f, bY = resY + 25.0f;
        drawRect(bX, bY, bW, bH, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);
        drawRect(bX + 3.0f, bY + 3.0f, bW - 6.0f, bH - 6.0f, Vec4(0.08f, 0.1f, 0.14f, 0.95f), orthoProj);
        drawText("RETURN TO LOBBY", bX + 45.0f, bY + 16.0f, 18.0f, Vec4(0.95f, 0.75f, 0.15f, 1.0f), orthoProj);

        glEnable(GL_DEPTH_TEST);
    }
};

} // namespace UI

#endif // UI_UIMANAGER_HPP
