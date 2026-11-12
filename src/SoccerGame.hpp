#ifndef SOCCER_GAME_HPP
#define SOCCER_GAME_HPP

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "Math3D.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Sound.hpp"

// Pitch bounds
constexpr float PITCH_LENGTH = 72.0f; // X: -36 to +36
constexpr float PITCH_WIDTH  = 48.0f; // Z: -24 to +24
constexpr float HALF_PITCH_L = PITCH_LENGTH * 0.5f;
constexpr float HALF_PITCH_W = PITCH_WIDTH * 0.5f;

constexpr float GOAL_WIDTH   = 8.4f;  // Z: -4.2 to +4.2
constexpr float GOAL_HEIGHT  = 3.2f;  // Y: 0 to 3.2
constexpr float GOAL_DEPTH   = 3.0f;  // X: 36 to 39
constexpr float BALL_RADIUS  = 0.42f;
constexpr float PLAYER_RADIUS = 0.65f;

enum class PlayerRole { GK, DF, MF, ST };
enum class PlayerState { IDLE, RUNNING, KICKING, TACKLING, STUNNED, CELEBRATING, DIVING };

struct Particle {
    Vec3 pos;
    Vec3 vel;
    Vec4 color;
    float life;
    float maxLife;
    float size;
};

struct Player {
    int id;
    int team; // 0 = Blue, 1 = Red
    PlayerRole role;
    PlayerState state = PlayerState::IDLE;

    Vec3 pos;
    Vec3 vel;
    Vec3 homePos;
    float facingAngle = 0.0f;
    float runAnim = 0.0f;
    float stateTimer = 0.0f;
    float kickCooldown = 0.0f;
    float tackleCooldown = 0.0f;

    float baseSpeed = 10.5f;
    float sprintSpeed = 14.5f;
    float kickPower = 28.0f;
    bool isControlled = false;

    // Dive animation for GK
    Vec3 diveDir = Vec3(0, 0, 0);

    Player(int _id, int _team, PlayerRole _role, const Vec3& _home)
        : id(_id), team(_team), role(_role), homePos(_home), pos(_home) {
        facingAngle = (team == 0) ? 0.0f : PI;
        if (role == PlayerRole::GK) {
            baseSpeed = 9.0f;
            sprintSpeed = 11.5f;
        } else if (role == PlayerRole::ST) {
            baseSpeed = 11.0f;
            sprintSpeed = 15.5f;
        }
    }
};

struct Ball {
    Vec3 pos = Vec3(0.0f, BALL_RADIUS, 0.0f);
    Vec3 vel = Vec3(0.0f, 0.0f, 0.0f);
    Mat4 rotation = Mat4::identity();
    int lastTouchedTeam = 0;
    int lastTouchedPlayer = -1;
    bool inNet = false;

    void reset(const Vec3& p = Vec3(0.0f, BALL_RADIUS, 0.0f)) {
        pos = p;
        vel = Vec3(0.0f, 0.0f, 0.0f);
        rotation = Mat4::identity();
        inNet = false;
    }
};

class SoccerGame {
public:
    // Game state
    int scoreBlue = 0;
    int scoreRed = 0;
    float matchTime = 0.0f; // Seconds elapsed
    bool isPaused = false;
    bool showHelp = false;
    int cameraMode = 0; // 0: Broadcast, 1: Chase/Follow, 2: Top-down

    // Match flow
    bool isGoalCelebration = false;
    float goalCelebrationTimer = 0.0f;
    int scoringTeam = 0;
    bool isKickoff = true;
    float kickoffTimer = 0.0f;

    // Entities
    Ball ball;
    std::vector<Player> players;
    int controlledPlayerBlue = 3; // Start controlling Striker
    int controlledPlayerRed = 7;  // For 2P mode or reference

    // Player inputs (P1 = Blue)
    Vec2 p1MoveInput;
    bool p1Sprint = false;
    bool p1ChargingShoot = false;
    float p1ShootCharge = 0.0f;
    bool p1PassPressed = false;
    bool p1TacklePressed = false;
    bool p1SwitchPressed = false;

    // 2-Player mode toggle (Local PvP vs Player vs AI)
    bool twoPlayerMode = false;
    Vec2 p2MoveInput;
    bool p2Sprint = false;
    bool p2ChargingShoot = false;
    float p2ShootCharge = 0.0f;
    bool p2PassPressed = false;
    bool p2TacklePressed = false;
    bool p2SwitchPressed = false;

    // Visuals & Meshes
    Mesh meshPitch;
    Mesh meshFieldLines;
    Mesh meshGoalFrame;
    Mesh meshGoalNet;
    Mesh meshBall;
    Mesh meshTorso;
    Mesh meshHead;
    Mesh meshLimb;
    Mesh meshStadium;
    Mesh meshLED;
    Mesh meshTower;
    Mesh meshShadow;
    Mesh meshRing;
    Mesh meshArrow;
    Mesh meshQuad;
    Mesh meshCenteredQuad;

    // Particles
    std::vector<Particle> particles;

    // Camera
    Vec3 camPos = Vec3(0, 22, 32);
    Vec3 camTarget = Vec3(0, 0, 0);

    // Shaders
    Shader worldShader;
    Shader uiShader;

    SoccerGame() {
        initSquads();
    }

    void initSquads() {
        players.clear();
        // Team Blue (Team 0: Attacks +X, Defends -X)
        players.emplace_back(0, 0, PlayerRole::GK, Vec3(-HALF_PITCH_L + 3.0f, 0, 0));
        players.emplace_back(1, 0, PlayerRole::DF, Vec3(-HALF_PITCH_L * 0.55f, 0, -6.0f));
        players.emplace_back(2, 0, PlayerRole::MF, Vec3(-HALF_PITCH_L * 0.25f, 0, 6.0f));
        players.emplace_back(3, 0, PlayerRole::ST, Vec3(-2.0f, 0, 0));

        // Team Red (Team 1: Attacks -X, Defends +X)
        players.emplace_back(4, 1, PlayerRole::GK, Vec3(HALF_PITCH_L - 3.0f, 0, 0));
        players.emplace_back(5, 1, PlayerRole::DF, Vec3(HALF_PITCH_L * 0.55f, 0, 6.0f));
        players.emplace_back(6, 1, PlayerRole::MF, Vec3(HALF_PITCH_L * 0.25f, 0, -6.0f));
        players.emplace_back(7, 1, PlayerRole::ST, Vec3(2.0f, 0, 0));

        controlledPlayerBlue = 3;
        controlledPlayerRed = 7;
        players[controlledPlayerBlue].isControlled = true;
        if (twoPlayerMode) {
            players[controlledPlayerRed].isControlled = true;
        }
    }

    void initGraphics() {
        // Compile Shaders
        worldShader.compile(Shaders::worldVertex, Shaders::worldFragment);
        uiShader.compile(Shaders::uiVertex, Shaders::uiFragment);

        // Generate Meshes
        meshPitch = MeshGen::createPlane(PITCH_LENGTH + 12.0f, PITCH_WIDTH + 10.0f, 32, 24);
        meshBall = MeshGen::createSphere(BALL_RADIUS, 18, 24);
        meshTorso = MeshGen::createCube(0.8f, 1.0f, 0.45f);
        meshHead = MeshGen::createSphere(0.35f, 12, 16);
        meshLimb = MeshGen::createCylinder(0.18f, 0.14f, 0.75f, 12);
        meshShadow = MeshGen::createPlane(1.0f, 1.0f, 1, 1);
        meshRing = MeshGen::createTorusRing(0.85f, 1.15f, 28);
        meshArrow = MeshGen::createCylinder(0.0f, 0.35f, 0.6f, 6);
        meshQuad = MeshGen::createQuad2D(1.0f, 1.0f);
        meshCenteredQuad = MeshGen::createCenteredQuad2D(1.0f, 1.0f);

        buildFieldLines();
        buildGoalMeshes();
        buildStadium();

        SoundManager::instance().playWhistle();
    }

    void buildFieldLines() {
        std::vector<Vertex> verts;
        std::vector<unsigned int> inds;
        Vec4 white(0.98f, 0.98f, 0.98f, 1.0f);
        float lineW = 0.22f;
        float y = 0.02f; // slightly above pitch to prevent z-fighting

        auto addSegment = [&](Vec3 p1, Vec3 p2) {
            Vec3 dir = (p2 - p1).normalized();
            Vec3 perp = cross(dir, Vec3(0,1,0)).normalized() * (lineW * 0.5f);

            unsigned int b = (unsigned int)verts.size();
            verts.push_back(Vertex(p1 - perp + Vec3(0,y,0), Vec3(0,1,0), Vec2(0,0), white));
            verts.push_back(Vertex(p1 + perp + Vec3(0,y,0), Vec3(0,1,0), Vec2(1,0), white));
            verts.push_back(Vertex(p2 + perp + Vec3(0,y,0), Vec3(0,1,0), Vec2(1,1), white));
            verts.push_back(Vertex(p2 - perp + Vec3(0,y,0), Vec3(0,1,0), Vec2(0,1), white));

            inds.push_back(b); inds.push_back(b+1); inds.push_back(b+2);
            inds.push_back(b); inds.push_back(b+2); inds.push_back(b+3);
        };

        // Outer Touchlines & Goal lines
        addSegment(Vec3(-HALF_PITCH_L, y, -HALF_PITCH_W), Vec3(HALF_PITCH_L, y, -HALF_PITCH_W));
        addSegment(Vec3(-HALF_PITCH_L, y,  HALF_PITCH_W), Vec3(HALF_PITCH_L, y,  HALF_PITCH_W));
        addSegment(Vec3(-HALF_PITCH_L, y, -HALF_PITCH_W), Vec3(-HALF_PITCH_L, y,  HALF_PITCH_W));
        addSegment(Vec3( HALF_PITCH_L, y, -HALF_PITCH_W), Vec3( HALF_PITCH_L, y,  HALF_PITCH_W));

        // Halfway Line
        addSegment(Vec3(0, y, -HALF_PITCH_W), Vec3(0, y, HALF_PITCH_W));

        // Center Circle (Radius 6.5)
        int segs = 48;
        for (int i = 0; i < segs; ++i) {
            float a1 = (float)i / segs * 2.0f * PI;
            float a2 = (float)(i + 1) / segs * 2.0f * PI;
            addSegment(Vec3(std::cos(a1) * 6.5f, y, std::sin(a1) * 6.5f),
                       Vec3(std::cos(a2) * 6.5f, y, std::sin(a2) * 6.5f));
        }

        // Left Penalty Box (16m50 equivalent)
        float boxL = 13.0f, boxW = 18.0f;
        addSegment(Vec3(-HALF_PITCH_L, y, -boxW*0.5f), Vec3(-HALF_PITCH_L + boxL, y, -boxW*0.5f));
        addSegment(Vec3(-HALF_PITCH_L + boxL, y, -boxW*0.5f), Vec3(-HALF_PITCH_L + boxL, y, boxW*0.5f));
        addSegment(Vec3(-HALF_PITCH_L, y, boxW*0.5f), Vec3(-HALF_PITCH_L + boxL, y, boxW*0.5f));

        // Right Penalty Box
        addSegment(Vec3(HALF_PITCH_L, y, -boxW*0.5f), Vec3(HALF_PITCH_L - boxL, y, -boxW*0.5f));
        addSegment(Vec3(HALF_PITCH_L - boxL, y, -boxW*0.5f), Vec3(HALF_PITCH_L - boxL, y, boxW*0.5f));
        addSegment(Vec3(HALF_PITCH_L, y, boxW*0.5f), Vec3(HALF_PITCH_L - boxL, y, boxW*0.5f));

        // Goal Areas (5m50 equivalent)
        float gAreaL = 4.5f, gAreaW = 10.5f;
        addSegment(Vec3(-HALF_PITCH_L, y, -gAreaW*0.5f), Vec3(-HALF_PITCH_L + gAreaL, y, -gAreaW*0.5f));
        addSegment(Vec3(-HALF_PITCH_L + gAreaL, y, -gAreaW*0.5f), Vec3(-HALF_PITCH_L + gAreaL, y, gAreaW*0.5f));
        addSegment(Vec3(-HALF_PITCH_L, y, gAreaW*0.5f), Vec3(-HALF_PITCH_L + gAreaL, y, gAreaW*0.5f));

        addSegment(Vec3(HALF_PITCH_L, y, -gAreaW*0.5f), Vec3(HALF_PITCH_L - gAreaL, y, -gAreaW*0.5f));
        addSegment(Vec3(HALF_PITCH_L - gAreaL, y, -gAreaW*0.5f), Vec3(HALF_PITCH_L - gAreaL, y, gAreaW*0.5f));
        addSegment(Vec3(HALF_PITCH_L, y, gAreaW*0.5f), Vec3(HALF_PITCH_L - gAreaL, y, gAreaW*0.5f));

        meshFieldLines = Mesh(verts, inds);
    }

    void buildGoalMeshes() {
        // Goal post frames (Cylinders)
        std::vector<Vertex> verts;
        std::vector<unsigned int> inds;
        Vec4 white(0.95f, 0.95f, 0.95f, 1.0f);
        float r = 0.12f;

        // Post helper
        auto addPost = [&](Vec3 p1, Vec3 p2) {
            Mesh cyl = MeshGen::createCylinder(r, r, (p2 - p1).length(), 10, white);
            // Transform cylinder to align with p1 -> p2
            Vec3 dir = (p2 - p1).normalized();
            Vec3 center = (p1 + p2) * 0.5f;
            float len = (p2 - p1).length();

            Mat4 model = Mat4::translate(center);
            if (std::abs(dir.y) > 0.99f) {
                if (dir.y < 0) model = model * Mat4::rotateX(PI);
            } else {
                Vec3 axis = cross(Vec3(0,1,0), dir).normalized();
                float angle = std::acos(std::max(-1.0f, std::min(1.0f, dir.y)));
                model = model * Mat4::rotateAxis(axis, angle);
            }

            unsigned int base = (unsigned int)verts.size();
            for (auto& v : cyl.vertices) {
                Vec4 tp = model * Vec4(v.position, 1.0f);
                Vec4 tn = model * Vec4(v.normal, 0.0f);
                verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), Vec3(tn.x, tn.y, tn.z).normalized(), v.texCoord, white));
            }
            for (auto i : cyl.indices) {
                inds.push_back(base + i);
            }
        };

        // Left Goal posts & frame
        float hgW = GOAL_WIDTH * 0.5f;
        addPost(Vec3(-HALF_PITCH_L, 0, -hgW), Vec3(-HALF_PITCH_L, GOAL_HEIGHT, -hgW));
        addPost(Vec3(-HALF_PITCH_L, 0,  hgW), Vec3(-HALF_PITCH_L, GOAL_HEIGHT,  hgW));
        addPost(Vec3(-HALF_PITCH_L, GOAL_HEIGHT, -hgW), Vec3(-HALF_PITCH_L, GOAL_HEIGHT, hgW));
        addPost(Vec3(-HALF_PITCH_L, GOAL_HEIGHT, -hgW), Vec3(-HALF_PITCH_L - GOAL_DEPTH, GOAL_HEIGHT*0.6f, -hgW));
        addPost(Vec3(-HALF_PITCH_L, GOAL_HEIGHT,  hgW), Vec3(-HALF_PITCH_L - GOAL_DEPTH, GOAL_HEIGHT*0.6f,  hgW));
        addPost(Vec3(-HALF_PITCH_L - GOAL_DEPTH, GOAL_HEIGHT*0.6f, -hgW), Vec3(-HALF_PITCH_L - GOAL_DEPTH, GOAL_HEIGHT*0.6f, hgW));
        addPost(Vec3(-HALF_PITCH_L - GOAL_DEPTH, 0, -hgW), Vec3(-HALF_PITCH_L - GOAL_DEPTH, GOAL_HEIGHT*0.6f, -hgW));
        addPost(Vec3(-HALF_PITCH_L - GOAL_DEPTH, 0,  hgW), Vec3(-HALF_PITCH_L - GOAL_DEPTH, GOAL_HEIGHT*0.6f,  hgW));

        // Right Goal posts & frame
        addPost(Vec3(HALF_PITCH_L, 0, -hgW), Vec3(HALF_PITCH_L, GOAL_HEIGHT, -hgW));
        addPost(Vec3(HALF_PITCH_L, 0,  hgW), Vec3(HALF_PITCH_L, GOAL_HEIGHT,  hgW));
        addPost(Vec3(HALF_PITCH_L, GOAL_HEIGHT, -hgW), Vec3(HALF_PITCH_L, GOAL_HEIGHT, hgW));
        addPost(Vec3(HALF_PITCH_L, GOAL_HEIGHT, -hgW), Vec3(HALF_PITCH_L + GOAL_DEPTH, GOAL_HEIGHT*0.6f, -hgW));
        addPost(Vec3(HALF_PITCH_L, GOAL_HEIGHT,  hgW), Vec3(HALF_PITCH_L + GOAL_DEPTH, GOAL_HEIGHT*0.6f,  hgW));
        addPost(Vec3(HALF_PITCH_L + GOAL_DEPTH, GOAL_HEIGHT*0.6f, -hgW), Vec3(HALF_PITCH_L + GOAL_DEPTH, GOAL_HEIGHT*0.6f, hgW));
        addPost(Vec3(HALF_PITCH_L + GOAL_DEPTH, 0, -hgW), Vec3(HALF_PITCH_L + GOAL_DEPTH, GOAL_HEIGHT*0.6f, -hgW));
        addPost(Vec3(HALF_PITCH_L + GOAL_DEPTH, 0,  hgW), Vec3(HALF_PITCH_L + GOAL_DEPTH, GOAL_HEIGHT*0.6f,  hgW));

        meshGoalFrame = Mesh(verts, inds);

        // Goal Net Mesh (Left & Right)
        std::vector<Vertex> netVerts;
        std::vector<unsigned int> netInds;

        auto addNetQuad = [&](Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3) {
            unsigned int b = (unsigned int)netVerts.size();
            Vec3 n = cross(p1 - p0, p2 - p0).normalized();
            netVerts.push_back(Vertex(p0, n, Vec2(0,0), Vec4(1,1,1,0.7f)));
            netVerts.push_back(Vertex(p1, n, Vec2(1,0), Vec4(1,1,1,0.7f)));
            netVerts.push_back(Vertex(p2, n, Vec2(1,1), Vec4(1,1,1,0.7f)));
            netVerts.push_back(Vertex(p3, n, Vec2(0,1), Vec4(1,1,1,0.7f)));

            netInds.push_back(b); netInds.push_back(b+1); netInds.push_back(b+2);
            netInds.push_back(b); netInds.push_back(b+2); netInds.push_back(b+3);
        };

        // Left Net
        addNetQuad(Vec3(-HALF_PITCH_L, GOAL_HEIGHT, -hgW), Vec3(-HALF_PITCH_L, GOAL_HEIGHT, hgW),
                   Vec3(-HALF_PITCH_L - GOAL_DEPTH, GOAL_HEIGHT*0.6f, hgW), Vec3(-HALF_PITCH_L - GOAL_DEPTH, GOAL_HEIGHT*0.6f, -hgW)); // Top
        addNetQuad(Vec3(-HALF_PITCH_L - GOAL_DEPTH, 0, -hgW), Vec3(-HALF_PITCH_L - GOAL_DEPTH, 0, hgW),
                   Vec3(-HALF_PITCH_L - GOAL_DEPTH, GOAL_HEIGHT*0.6f, hgW), Vec3(-HALF_PITCH_L - GOAL_DEPTH, GOAL_HEIGHT*0.6f, -hgW)); // Back
        addNetQuad(Vec3(-HALF_PITCH_L, 0, -hgW), Vec3(-HALF_PITCH_L - GOAL_DEPTH, 0, -hgW),
                   Vec3(-HALF_PITCH_L - GOAL_DEPTH, GOAL_HEIGHT*0.6f, -hgW), Vec3(-HALF_PITCH_L, GOAL_HEIGHT, -hgW)); // Side 1
        addNetQuad(Vec3(-HALF_PITCH_L, 0, hgW), Vec3(-HALF_PITCH_L - GOAL_DEPTH, 0, hgW),
                   Vec3(-HALF_PITCH_L - GOAL_DEPTH, GOAL_HEIGHT*0.6f, hgW), Vec3(-HALF_PITCH_L, GOAL_HEIGHT, hgW)); // Side 2

        // Right Net
        addNetQuad(Vec3(HALF_PITCH_L, GOAL_HEIGHT, -hgW), Vec3(HALF_PITCH_L, GOAL_HEIGHT, hgW),
                   Vec3(HALF_PITCH_L + GOAL_DEPTH, GOAL_HEIGHT*0.6f, hgW), Vec3(HALF_PITCH_L + GOAL_DEPTH, GOAL_HEIGHT*0.6f, -hgW));
        addNetQuad(Vec3(HALF_PITCH_L + GOAL_DEPTH, 0, -hgW), Vec3(HALF_PITCH_L + GOAL_DEPTH, 0, hgW),
                   Vec3(HALF_PITCH_L + GOAL_DEPTH, GOAL_HEIGHT*0.6f, hgW), Vec3(HALF_PITCH_L + GOAL_DEPTH, GOAL_HEIGHT*0.6f, -hgW));
        addNetQuad(Vec3(HALF_PITCH_L, 0, -hgW), Vec3(HALF_PITCH_L + GOAL_DEPTH, 0, -hgW),
                   Vec3(HALF_PITCH_L + GOAL_DEPTH, GOAL_HEIGHT*0.6f, -hgW), Vec3(HALF_PITCH_L, GOAL_HEIGHT, -hgW));
        addNetQuad(Vec3(HALF_PITCH_L, 0, hgW), Vec3(HALF_PITCH_L + GOAL_DEPTH, 0, hgW),
                   Vec3(HALF_PITCH_L + GOAL_DEPTH, GOAL_HEIGHT*0.6f, hgW), Vec3(HALF_PITCH_L, GOAL_HEIGHT, hgW));

        meshGoalNet = Mesh(netVerts, netInds);
    }

    void buildStadium() {
        // Stadium Stands (Tiered multi-level grandstands around pitch)
        std::vector<Vertex> sVerts;
        std::vector<unsigned int> sInds;

        auto addStandBlock = [&](Vec3 center, Vec3 size, Vec4 color) {
            Mesh cube = MeshGen::createCube(size.x, size.y, size.z, color);
            Mat4 model = Mat4::translate(center);
            unsigned int base = (unsigned int)sVerts.size();
            for (auto& v : cube.vertices) {
                Vec4 tp = model * Vec4(v.position, 1.0f);
                Vec4 tn = model * Vec4(v.normal, 0.0f);
                sVerts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), Vec3(tn.x, tn.y, tn.z), v.texCoord, color));
            }
            for (auto i : cube.indices) {
                sInds.push_back(base + i);
            }
        };

        // Long side North Stands (+Z)
        for (int tier = 0; tier < 5; ++tier) {
            float distZ = HALF_PITCH_W + 5.0f + tier * 3.5f;
            float heightY = 1.0f + tier * 2.2f;
            Vec4 seatColor = ((tier % 2 == 0) ? Vec4(0.15f, 0.35f, 0.75f, 1.0f) : Vec4(0.75f, 0.2f, 0.2f, 1.0f));
            addStandBlock(Vec3(0, heightY * 0.5f, distZ), Vec3(PITCH_LENGTH + 16.0f, heightY, 3.5f), seatColor);
        }

        // Long side South Stands (-Z)
        for (int tier = 0; tier < 5; ++tier) {
            float distZ = -HALF_PITCH_W - 5.0f - tier * 3.5f;
            float heightY = 1.0f + tier * 2.2f;
            Vec4 seatColor = ((tier % 2 == 0) ? Vec4(0.15f, 0.35f, 0.75f, 1.0f) : Vec4(0.75f, 0.2f, 0.2f, 1.0f));
            addStandBlock(Vec3(0, heightY * 0.5f, distZ), Vec3(PITCH_LENGTH + 16.0f, heightY, 3.5f), seatColor);
        }

        // Short side West Stands (-X)
        for (int tier = 0; tier < 4; ++tier) {
            float distX = -HALF_PITCH_L - 6.0f - tier * 3.5f;
            float heightY = 1.0f + tier * 2.2f;
            Vec4 seatColor = Vec4(0.18f, 0.45f, 0.85f, 1.0f);
            addStandBlock(Vec3(distX, heightY * 0.5f, 0), Vec3(3.5f, heightY, PITCH_WIDTH + 24.0f), seatColor);
        }

        // Short side East Stands (+X)
        for (int tier = 0; tier < 4; ++tier) {
            float distX = HALF_PITCH_L + 6.0f + tier * 3.5f;
            float heightY = 1.0f + tier * 2.2f;
            Vec4 seatColor = Vec4(0.85f, 0.22f, 0.22f, 1.0f);
            addStandBlock(Vec3(distX, heightY * 0.5f, 0), Vec3(3.5f, heightY, PITCH_WIDTH + 24.0f), seatColor);
        }

        meshStadium = Mesh(sVerts, sInds);

        // LED Ad Boards surrounding pitch
        meshLED = MeshGen::createCube(1.0f, 0.9f, 0.3f, Vec4(0.1f, 0.1f, 0.15f, 1.0f));

        // Floodlight Tower Pole
        meshTower = MeshGen::createCylinder(0.35f, 0.65f, 24.0f, 12, Vec4(0.7f, 0.72f, 0.75f, 1.0f));
    }

    void spawnKickParticles(const Vec3& pos, const Vec3& dir, float power) {
        int count = std::min(24, (int)(power * 0.8f));
        for (int i = 0; i < count; ++i) {
            Particle p;
            p.pos = pos;
            float spread = 0.6f;
            Vec3 randomSpread(
                ((rand() % 100) / 50.0f - 1.0f) * spread,
                ((rand() % 100) / 100.0f) * spread + 0.2f,
                ((rand() % 100) / 50.0f - 1.0f) * spread
            );
            p.vel = (dir.normalized() * (power * 0.4f) + randomSpread * 4.0f);
            p.color = Vec4(1.0f, 0.9f, 0.4f, 1.0f); // Bright spark
            p.life = 0.0f;
            p.maxLife = 0.25f + ((rand() % 100) / 500.0f);
            p.size = 0.18f;
            particles.push_back(p);
        }
    }

    void spawnGoalConfetti() {
        for (int i = 0; i < 180; ++i) {
            Particle p;
            p.pos = ball.pos + Vec3(
                ((rand() % 200) / 20.0f - 5.0f),
                ((rand() % 100) / 10.0f + 2.0f),
                ((rand() % 200) / 20.0f - 5.0f)
            );
            p.vel = Vec3(
                ((rand() % 100) / 10.0f - 5.0f),
                ((rand() % 100) / 15.0f + 3.0f),
                ((rand() % 100) / 10.0f - 5.0f)
            );
            // Festive colors: Gold, Cyan, Magenta, Lime, White
            int colIdx = rand() % 5;
            if (colIdx == 0) p.color = Vec4(1.0f, 0.85f, 0.1f, 1.0f);
            else if (colIdx == 1) p.color = Vec4(0.2f, 0.9f, 1.0f, 1.0f);
            else if (colIdx == 2) p.color = Vec4(1.0f, 0.2f, 0.8f, 1.0f);
            else if (colIdx == 3) p.color = Vec4(0.3f, 1.0f, 0.3f, 1.0f);
            else p.color = Vec4(1.0f, 1.0f, 1.0f, 1.0f);

            p.life = 0.0f;
            p.maxLife = 2.5f + ((rand() % 100) / 50.0f);
            p.size = 0.22f;
            particles.push_back(p);
        }
    }

    void update(float dt) {
        if (isPaused) return;

        matchTime += dt;

        // Update celebration or kickoff timers
        if (isGoalCelebration) {
            goalCelebrationTimer -= dt;
            if (goalCelebrationTimer <= 0.0f) {
                isGoalCelebration = false;
                resetKickoff(scoringTeam == 0 ? 1 : 0);
            }
        }

        if (isKickoff) {
            kickoffTimer -= dt;
            if (kickoffTimer <= 0.0f) {
                isKickoff = false;
            }
        }

        // 1. Process Player Controls
        updatePlayerControls(dt);

        // 2. Process AI logic for both teams
        updateAI(dt);

        // 3. Update Player Physics & Collisions
        updatePlayers(dt);

        // 4. Update Ball Physics & Goals
        updateBall(dt);

        // 5. Update Particles
        updateParticles(dt);

        // 6. Update Camera
        updateCamera(dt);
    }

    void updatePlayerControls(float dt) {
        // P1 (Blue Team)
        Player& p1 = players[controlledPlayerBlue];
        p1.isControlled = true;

        if (p1SwitchPressed) {
            p1SwitchPressed = false;
            switchControlledPlayer(0);
        }

        // Movement
        if (p1.state != PlayerState::TACKLING && p1.state != PlayerState::STUNNED && p1.state != PlayerState::CELEBRATING) {
            if (p1MoveInput.lengthSq() > 0.01f) {
                Vec2 moveNorm = p1MoveInput.normalized();
                float currentSpeed = p1Sprint ? p1.sprintSpeed : p1.baseSpeed;
                p1.vel = Vec3(moveNorm.x, 0.0f, moveNorm.y) * currentSpeed;

                // Face moving direction
                float targetAngle = std::atan2(-moveNorm.y, moveNorm.x);
                p1.facingAngle = targetAngle;
                p1.state = PlayerState::RUNNING;
            } else {
                p1.vel = Vec3(0, 0, 0);
                p1.state = PlayerState::IDLE;
            }

            // Shoot charge
            if (p1ChargingShoot) {
                p1ShootCharge = std::min(1.0f, p1ShootCharge + dt * 1.8f);
            } else if (p1ShootCharge > 0.05f) {
                // Release shoot!
                executeKick(p1, p1ShootCharge, true);
                p1ShootCharge = 0.0f;
            } else {
                p1ShootCharge = 0.0f;
            }

            // Pass
            if (p1PassPressed) {
                p1PassPressed = false;
                executePass(p1);
            }

            // Tackle
            if (p1TacklePressed) {
                p1TacklePressed = false;
                executeTackle(p1);
            }
        }

        // P2 (Red Team if 2-Player mode is enabled)
        if (twoPlayerMode) {
            Player& p2 = players[controlledPlayerRed];
            p2.isControlled = true;

            if (p2SwitchPressed) {
                p2SwitchPressed = false;
                switchControlledPlayer(1);
            }

            if (p2.state != PlayerState::TACKLING && p2.state != PlayerState::STUNNED && p2.state != PlayerState::CELEBRATING) {
                if (p2MoveInput.lengthSq() > 0.01f) {
                    Vec2 moveNorm = p2MoveInput.normalized();
                    float currentSpeed = p2Sprint ? p2.sprintSpeed : p2.baseSpeed;
                    p2.vel = Vec3(moveNorm.x, 0.0f, moveNorm.y) * currentSpeed;
                    p2.facingAngle = std::atan2(-moveNorm.y, moveNorm.x);
                    p2.state = PlayerState::RUNNING;
                } else {
                    p2.vel = Vec3(0, 0, 0);
                    p2.state = PlayerState::IDLE;
                }

                if (p2ChargingShoot) {
                    p2ShootCharge = std::min(1.0f, p2ShootCharge + dt * 1.8f);
                } else if (p2ShootCharge > 0.05f) {
                    executeKick(p2, p2ShootCharge, true);
                    p2ShootCharge = 0.0f;
                } else {
                    p2ShootCharge = 0.0f;
                }

                if (p2PassPressed) {
                    p2PassPressed = false;
                    executePass(p2);
                }

                if (p2TacklePressed) {
                    p2TacklePressed = false;
                    executeTackle(p2);
                }
            }
        }
    }

    void switchControlledPlayer(int team) {
        int bestId = -1;
        float minDist = 9999.0f;
        int startIdx = (team == 0) ? 1 : 5; // skip GK
        int endIdx = (team == 0) ? 4 : 8;

        for (int i = startIdx; i < endIdx; ++i) {
            if (team == 0 && i == controlledPlayerBlue) continue;
            if (team == 1 && i == controlledPlayerRed) continue;
            float d = (players[i].pos - ball.pos).lengthSq();
            if (d < minDist) {
                minDist = d;
                bestId = i;
            }
        }

        if (bestId != -1) {
            if (team == 0) {
                players[controlledPlayerBlue].isControlled = false;
                controlledPlayerBlue = bestId;
                players[controlledPlayerBlue].isControlled = true;
            } else {
                players[controlledPlayerRed].isControlled = false;
                controlledPlayerRed = bestId;
                players[controlledPlayerRed].isControlled = true;
            }
        }
    }

    void executeKick(Player& p, float charge, bool isGoalShot) {
        float distToBall = (p.pos - ball.pos).length();
        if (distToBall > 1.8f || ball.pos.y > 1.6f) return;

        Vec3 kickDir;
        if (isGoalShot) {
            // Target opponent goal
            Vec3 targetGoal = (p.team == 0) ? Vec3(HALF_PITCH_L, 1.2f, 0) : Vec3(-HALF_PITCH_L, 1.2f, 0);
            // Slight angle towards facing direction
            Vec3 facing(std::cos(p.facingAngle), 0, -std::sin(p.facingAngle));
            kickDir = (targetGoal - p.pos).normalized() * 0.7f + facing * 0.3f;
        } else {
            kickDir = Vec3(std::cos(p.facingAngle), 0, -std::sin(p.facingAngle));
        }
        kickDir = kickDir.normalized();

        float power = p.kickPower * (0.45f + charge * 0.85f);
        float loft = 0.15f + charge * 0.55f;

        ball.vel = Vec3(kickDir.x * power, power * loft, kickDir.z * power);
        ball.lastTouchedTeam = p.team;
        ball.lastTouchedPlayer = p.id;

        p.state = PlayerState::KICKING;
        p.stateTimer = 0.25f;
        p.kickCooldown = 0.4f;

        SoundManager::instance().playKick();
        spawnKickParticles(ball.pos, ball.vel, power);
    }

    void executePass(Player& p) {
        float distToBall = (p.pos - ball.pos).length();
        if (distToBall > 1.8f || ball.pos.y > 1.6f) return;

        // Find best teammate in front
        Vec3 forward(std::cos(p.facingAngle), 0, -std::sin(p.facingAngle));
        int bestTeammate = -1;
        float bestScore = -999.0f;

        for (auto& tm : players) {
            if (tm.team != p.team || tm.id == p.id) continue;
            Vec3 toTm = tm.pos - p.pos;
            float dist = toTm.length();
            if (dist < 2.0f) continue;
            float dotP = dot(forward, toTm.normalized());
            if (dotP > 0.2f) {
                float score = dotP * 10.0f - dist * 0.15f;
                if (score > bestScore) {
                    bestScore = score;
                    bestTeammate = tm.id;
                }
            }
        }

        Vec3 passDir = forward;
        float power = 18.0f;
        if (bestTeammate != -1) {
            Vec3 leadTarget = players[bestTeammate].pos + players[bestTeammate].vel * 0.4f;
            passDir = (leadTarget - p.pos).normalized();
            float dist = (leadTarget - p.pos).length();
            power = std::min(26.0f, std::max(12.0f, dist * 1.25f));
        }

        ball.vel = Vec3(passDir.x * power, 1.8f, passDir.z * power);
        ball.lastTouchedTeam = p.team;
        ball.lastTouchedPlayer = p.id;

        p.state = PlayerState::KICKING;
        p.stateTimer = 0.2f;

        SoundManager::instance().playKick();
        spawnKickParticles(ball.pos, ball.vel, power * 0.7f);
    }

    void executeTackle(Player& p) {
        if (p.tackleCooldown > 0.0f) return;

        p.state = PlayerState::TACKLING;
        p.stateTimer = 0.55f;
        p.tackleCooldown = 1.2f;

        Vec3 tackleDir(std::cos(p.facingAngle), 0, -std::sin(p.facingAngle));
        p.vel = tackleDir * 18.0f;

        SoundManager::instance().playTackle();

        // Check tackle hit against opponents
        for (auto& opp : players) {
            if (opp.team == p.team) continue;
            if ((opp.pos - p.pos).length() < 1.6f) {
                opp.state = PlayerState::STUNNED;
                opp.stateTimer = 0.8f;
                opp.vel = tackleDir * 4.0f;
                // Dislodge ball if opponent has it
                if ((opp.pos - ball.pos).length() < 1.8f) {
                    ball.vel = tackleDir * 15.0f + Vec3(0, 3.0f, 0);
                    ball.lastTouchedTeam = p.team;
                    ball.lastTouchedPlayer = p.id;
                }
            }
        }
    }

    void updateAI(float dt) {
        for (auto& p : players) {
            // If human controlled, skip AI
            if (p.isControlled && (p.team == 0 || (p.team == 1 && twoPlayerMode))) {
                continue;
            }

            if (p.state == PlayerState::TACKLING || p.state == PlayerState::STUNNED || p.state == PlayerState::CELEBRATING) {
                continue;
            }

            // Cooldowns
            if (p.kickCooldown > 0.0f) p.kickCooldown -= dt;
            if (p.tackleCooldown > 0.0f) p.tackleCooldown -= dt;

            // Target position calculation
            Vec3 target = p.homePos;
            float distToBall = (p.pos - ball.pos).length();
            bool teamHasBall = (ball.lastTouchedTeam == p.team && distToBall < 2.5f);
            Vec3 goalPos = (p.team == 0) ? Vec3(HALF_PITCH_L, 0, 0) : Vec3(-HALF_PITCH_L, 0, 0);
            Vec3 ownGoalPos = (p.team == 0) ? Vec3(-HALF_PITCH_L, 0, 0) : Vec3(HALF_PITCH_L, 0, 0);

            if (p.role == PlayerRole::GK) {
                // Goalkeeper AI: Guards goal line along bisector
                float gkX = (p.team == 0) ? (-HALF_PITCH_L + 2.0f) : (HALF_PITCH_L - 2.0f);
                float gkZ = std::max(-GOAL_WIDTH * 0.42f, std::min(GOAL_WIDTH * 0.42f, ball.pos.z * 0.35f));
                target = Vec3(gkX, 0, gkZ);

                // Dive / Block when shot is incoming
                if (distToBall < 3.2f && ball.pos.y < 2.5f && std::abs(ball.vel.x) > 8.0f) {
                    // Block shot!
                    ball.vel = Vec3(-ball.vel.x * 0.5f, 4.0f, (rand() % 100 / 50.0f - 1.0f) * 8.0f);
                    ball.lastTouchedTeam = p.team;
                    ball.lastTouchedPlayer = p.id;
                    p.state = PlayerState::KICKING;
                    p.stateTimer = 0.3f;
                    SoundManager::instance().playKick();
                }
            } else {
                // Outfield AI (DF, MF, ST)
                // Check if this player is the closest to ball in their team
                bool isClosestToBall = true;
                for (auto& other : players) {
                    if (other.team == p.team && other.id != p.id && other.role != PlayerRole::GK) {
                        if ((other.pos - ball.pos).lengthSq() < (p.pos - ball.pos).lengthSq()) {
                            isClosestToBall = false;
                            break;
                        }
                    }
                }

                if (isClosestToBall || distToBall < 6.0f) {
                    // Chase ball!
                    target = ball.pos + ball.vel * 0.15f;

                    // If close enough, control / shoot / pass
                    if (distToBall < 1.4f && ball.pos.y < 1.6f && p.kickCooldown <= 0.0f) {
                        float distToGoal = (p.pos - goalPos).length();

                        if (distToGoal < 28.0f && std::abs(p.pos.z) < 14.0f) {
                            // Sút cầu môn!
                            executeKick(p, 0.6f + (rand() % 100) / 300.0f, true);
                        } else if (p.role == PlayerRole::DF || (rand() % 100 < 35)) {
                            // Chuyền cho đồng đội
                            executePass(p);
                        } else {
                            // Rê bóng hướng về khung thành đối phương
                            Vec3 dribbleDir = (goalPos - p.pos).normalized();
                            ball.vel = dribbleDir * 12.0f + Vec3(0, 0.5f, 0);
                            ball.lastTouchedTeam = p.team;
                            ball.lastTouchedPlayer = p.id;
                            p.kickCooldown = 0.25f;
                            SoundManager::instance().playKick();
                        }
                    } else if (distToBall < 2.0f && ball.lastTouchedTeam != p.team && p.tackleCooldown <= 0.0f) {
                        // Cướp bóng / Xoạc bóng
                        if (rand() % 100 < 40) {
                            executeTackle(p);
                        }
                    }
                } else {
                    // Positional support & marking
                    float ballInfluenceX = (ball.pos.x - p.homePos.x) * 0.45f;
                    float ballInfluenceZ = (ball.pos.z - p.homePos.z) * 0.35f;
                    target = p.homePos + Vec3(ballInfluenceX, 0, ballInfluenceZ);

                    // Clamp to legal bounds
                    target.x = std::max(-HALF_PITCH_L + 3.0f, std::min(HALF_PITCH_L - 3.0f, target.x));
                    target.z = std::max(-HALF_PITCH_W + 2.0f, std::min(HALF_PITCH_W - 2.0f, target.z));
                }
            }

            // Move towards target
            Vec3 toTarget = target - p.pos;
            toTarget.y = 0;
            float dist = toTarget.length();

            if (dist > 0.4f) {
                Vec3 moveDir = toTarget.normalized();
                float spd = (distToBall < 8.0f) ? p.sprintSpeed : p.baseSpeed;
                p.vel = moveDir * spd;
                p.facingAngle = std::atan2(-moveDir.z, moveDir.x);
                p.state = PlayerState::RUNNING;
            } else {
                p.vel = Vec3(0, 0, 0);
                p.state = PlayerState::IDLE;
            }
        }
    }

    void updatePlayers(float dt) {
        for (auto& p : players) {
            // State timer
            if (p.stateTimer > 0.0f) {
                p.stateTimer -= dt;
                if (p.stateTimer <= 0.0f) {
                    if (p.state == PlayerState::KICKING || p.state == PlayerState::TACKLING || p.state == PlayerState::STUNNED) {
                        p.state = PlayerState::IDLE;
                    }
                }
            }

            // Cooldowns
            if (p.kickCooldown > 0.0f) p.kickCooldown -= dt;
            if (p.tackleCooldown > 0.0f) p.tackleCooldown -= dt;

            // Apply position
            p.pos += p.vel * dt;

            // Pitch boundary clamping
            p.pos.x = std::max(-HALF_PITCH_L, std::min(HALF_PITCH_L, p.pos.x));
            p.pos.z = std::max(-HALF_PITCH_W, std::min(HALF_PITCH_W, p.pos.z));
            p.pos.y = 0.0f;

            // Running animation cycle
            if (p.vel.lengthSq() > 0.5f) {
                p.runAnim += dt * p.vel.length() * 1.5f;
            } else {
                p.runAnim = 0.0f;
            }

            // Tackle deceleration
            if (p.state == PlayerState::TACKLING) {
                p.vel *= (1.0f - 4.0f * dt);
            }
        }

        // Player vs Player collision repulsion
        for (size_t i = 0; i < players.size(); ++i) {
            for (size_t j = i + 1; j < players.size(); ++j) {
                Vec3 diff = players[j].pos - players[i].pos;
                diff.y = 0;
                float dist = diff.length();
                float minDist = PLAYER_RADIUS * 2.0f;
                if (dist < minDist && dist > 0.001f) {
                    Vec3 push = diff.normalized() * ((minDist - dist) * 0.5f);
                    players[i].pos -= push;
                    players[j].pos += push;
                }
            }
        }
    }

    void updateBall(float dt) {
        // Gravity
        if (ball.pos.y > BALL_RADIUS) {
            ball.vel.y -= 22.0f * dt;
        }

        // Air drag
        ball.vel.x *= (1.0f - 0.25f * dt);
        ball.vel.z *= (1.0f - 0.25f * dt);

        // Move
        ball.pos += ball.vel * dt;

        // Ground collision & rolling friction
        if (ball.pos.y <= BALL_RADIUS) {
            ball.pos.y = BALL_RADIUS;
            if (std::abs(ball.vel.y) > 1.2f) {
                ball.vel.y = -ball.vel.y * 0.62f; // Bouncy ground
            } else {
                ball.vel.y = 0.0f;
            }
            // Rolling grass friction
            ball.vel.x *= (1.0f - 1.85f * dt);
            ball.vel.z *= (1.0f - 1.85f * dt);

            // Ball roll rotation matrix
            Vec3 horizVel(ball.vel.x, 0, ball.vel.z);
            float speed = horizVel.length();
            if (speed > 0.05f) {
                Vec3 rollAxis = cross(Vec3(0, 1, 0), horizVel).normalized();
                float rollAngle = (speed * dt) / BALL_RADIUS;
                ball.rotation = Mat4::rotateAxis(rollAxis, rollAngle) * ball.rotation;
            }
        }

        // Check Goal Post Collisions (Cylinders at post positions)
        checkGoalPostCollisions();

        // Check Goal Score
        if (!isGoalCelebration && !ball.inNet) {
            float hgW = GOAL_WIDTH * 0.5f - 0.2f;

            // Blue scores in Right Goal (+X)
            if (ball.pos.x >= HALF_PITCH_L && std::abs(ball.pos.z) < hgW && ball.pos.y < GOAL_HEIGHT) {
                scoreBlue++;
                triggerGoalCelebration(0);
            }
            // Red scores in Left Goal (-X)
            else if (ball.pos.x <= -HALF_PITCH_L && std::abs(ball.pos.z) < hgW && ball.pos.y < GOAL_HEIGHT) {
                scoreRed++;
                triggerGoalCelebration(1);
            }
        }

        // Goal Net Cushions & Bounds
        if (std::abs(ball.pos.x) > HALF_PITCH_L) {
            if (std::abs(ball.pos.z) < GOAL_WIDTH * 0.5f) {
                ball.inNet = true;
                ball.vel *= 0.88f; // Net friction
                // Back of net clamp
                if (ball.pos.x > HALF_PITCH_L + GOAL_DEPTH - BALL_RADIUS) {
                    ball.pos.x = HALF_PITCH_L + GOAL_DEPTH - BALL_RADIUS;
                    ball.vel.x = -ball.vel.x * 0.3f;
                } else if (ball.pos.x < -HALF_PITCH_L - GOAL_DEPTH + BALL_RADIUS) {
                    ball.pos.x = -HALF_PITCH_L - GOAL_DEPTH + BALL_RADIUS;
                    ball.vel.x = -ball.vel.x * 0.3f;
                }
            } else {
                // Outside goal line bounce
                if (ball.pos.x > HALF_PITCH_L + 1.5f) {
                    ball.pos.x = HALF_PITCH_L + 1.5f;
                    ball.vel.x = -ball.vel.x * 0.6f;
                } else if (ball.pos.x < -HALF_PITCH_L - 1.5f) {
                    ball.pos.x = -HALF_PITCH_L - 1.5f;
                    ball.vel.x = -ball.vel.x * 0.6f;
                }
            }
        }

        // Sideline Ad Board bounce
        if (std::abs(ball.pos.z) > HALF_PITCH_W + 1.2f) {
            ball.pos.z = (ball.pos.z > 0 ? 1 : -1) * (HALF_PITCH_W + 1.2f);
            ball.vel.z = -ball.vel.z * 0.65f;
        }

        // Ball vs Player feet collision (Dribble touch)
        for (auto& p : players) {
            Vec3 diff = ball.pos - p.pos;
            diff.y = 0;
            float dist = diff.length();
            float touchRadius = PLAYER_RADIUS + BALL_RADIUS + 0.15f;

            if (dist < touchRadius && ball.pos.y < 1.4f) {
                // Soft rebound / push in facing direction
                Vec3 push = diff.normalized() * (touchRadius - dist);
                ball.pos += push;

                Vec3 facing(std::cos(p.facingAngle), 0, -std::sin(p.facingAngle));
                if (p.vel.lengthSq() > 1.0f) {
                    // Dribbling momentum
                    ball.vel = p.vel * 1.08f + facing * 2.5f + Vec3(0, 0.4f, 0);
                } else {
                    ball.vel = push * 4.0f;
                }
                ball.lastTouchedTeam = p.team;
                ball.lastTouchedPlayer = p.id;
            }
        }
    }

    void checkGoalPostCollisions() {
        float hgW = GOAL_WIDTH * 0.5f;
        float postR = 0.25f;

        Vec3 posts[4] = {
            Vec3(-HALF_PITCH_L, 0, -hgW),
            Vec3(-HALF_PITCH_L, 0,  hgW),
            Vec3( HALF_PITCH_L, 0, -hgW),
            Vec3( HALF_PITCH_L, 0,  hgW)
        };

        for (int i = 0; i < 4; ++i) {
            Vec3 diff = ball.pos - posts[i];
            diff.y = 0;
            float d = diff.length();
            if (d < (BALL_RADIUS + postR) && ball.pos.y < GOAL_HEIGHT) {
                // Rebound off vertical post!
                Vec3 norm = diff.normalized();
                ball.pos = posts[i] + norm * (BALL_RADIUS + postR + 0.05f);
                ball.vel = norm * std::max(12.0f, ball.vel.length() * 0.85f) + Vec3(0, 2.0f, 0);
                SoundManager::instance().playPost();
                spawnKickParticles(ball.pos, ball.vel, 18.0f);
            }
        }

        // Crossbars
        if (ball.pos.y >= GOAL_HEIGHT - BALL_RADIUS && ball.pos.y <= GOAL_HEIGHT + BALL_RADIUS) {
            // Left Crossbar
            if (std::abs(ball.pos.x - (-HALF_PITCH_L)) < (BALL_RADIUS + postR) && std::abs(ball.pos.z) <= hgW) {
                ball.vel.y = -ball.vel.y * 0.75f;
                ball.vel.x = 8.0f;
                SoundManager::instance().playPost();
                spawnKickParticles(ball.pos, ball.vel, 20.0f);
            }
            // Right Crossbar
            if (std::abs(ball.pos.x - HALF_PITCH_L) < (BALL_RADIUS + postR) && std::abs(ball.pos.z) <= hgW) {
                ball.vel.y = -ball.vel.y * 0.75f;
                ball.vel.x = -8.0f;
                SoundManager::instance().playPost();
                spawnKickParticles(ball.pos, ball.vel, 20.0f);
            }
        }
    }

    void triggerGoalCelebration(int team) {
        isGoalCelebration = true;
        goalCelebrationTimer = 3.5f;
        scoringTeam = team;

        SoundManager::instance().playGoal();
        spawnGoalConfetti();

        for (auto& p : players) {
            if (p.team == team) {
                p.state = PlayerState::CELEBRATING;
                p.stateTimer = 3.5f;
            }
        }
    }

    void resetKickoff(int kickoffTeam) {
        ball.reset(Vec3(0.0f, BALL_RADIUS, 0.0f));
        isKickoff = true;
        kickoffTimer = 1.0f;

        for (auto& p : players) {
            p.pos = p.homePos;
            p.vel = Vec3(0, 0, 0);
            p.state = PlayerState::IDLE;
            p.facingAngle = (p.team == 0) ? 0.0f : PI;
        }

        // Move kickoff striker to center spot
        if (kickoffTeam == 0) {
            players[3].pos = Vec3(-0.8f, 0, 0);
        } else {
            players[7].pos = Vec3(0.8f, 0, 0);
        }

        SoundManager::instance().playWhistle();
    }

    void restartGame() {
        scoreBlue = 0;
        scoreRed = 0;
        matchTime = 0.0f;
        isGoalCelebration = false;
        initSquads();
        resetKickoff(0);
    }

    void updateParticles(float dt) {
        for (size_t i = 0; i < particles.size(); ) {
            particles[i].life += dt;
            if (particles[i].life >= particles[i].maxLife) {
                particles[i] = particles.back();
                particles.pop_back();
            } else {
                particles[i].pos += particles[i].vel * dt;
                particles[i].vel.y -= 9.8f * dt; // gravity
                particles[i].color.w = 1.0f - (particles[i].life / particles[i].maxLife);
                ++i;
            }
        }
    }

    void updateCamera(float dt) {
        Vec3 focusPoint = ball.pos * 0.65f + players[controlledPlayerBlue].pos * 0.35f;

        if (cameraMode == 0) {
            // Broadcast View (Sideline TV Cam)
            Vec3 targetCamPos = Vec3(focusPoint.x * 0.65f, 20.0f, 28.0f);
            camPos = lerp(camPos, targetCamPos, dt * 4.5f);
            camTarget = lerp(camTarget, focusPoint, dt * 6.0f);
        } else if (cameraMode == 1) {
            // Follow Player / Action View
            Player& cp = players[controlledPlayerBlue];
            Vec3 backDir = Vec3(-std::cos(cp.facingAngle), 0, std::sin(cp.facingAngle));
            Vec3 targetCamPos = cp.pos + backDir * 10.0f + Vec3(0, 7.0f, 0);
            camPos = lerp(camPos, targetCamPos, dt * 5.5f);
            camTarget = lerp(camTarget, cp.pos + Vec3(0, 1.2f, 0) + (ball.pos - cp.pos) * 0.4f, dt * 8.0f);
        } else if (cameraMode == 2) {
            // Top-down Tactical View
            Vec3 targetCamPos = Vec3(focusPoint.x * 0.5f, 38.0f, 1.0f);
            camPos = lerp(camPos, targetCamPos, dt * 4.0f);
            camTarget = lerp(camTarget, focusPoint, dt * 5.0f);
        }
    }

    void render(int screenW, int screenH) {
        glViewport(0, 0, screenW, screenH);
        glClearColor(0.08f, 0.12f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float aspect = (float)screenW / (float)screenH;
        Mat4 projection = Mat4::perspective(45.0f * DEG2RAD, aspect, 0.5f, 250.0f);
        Mat4 view = Mat4::lookAt(camPos, camTarget, Vec3(0, 1, 0));

        worldShader.use();
        worldShader.setMat4("uProjection", projection);
        worldShader.setMat4("uView", view);
        worldShader.setVec3("uViewPos", camPos);
        worldShader.setFloat("uTime", (float)glfwGetTime());

        // 1. Render Pitch (Grass)
        worldShader.setInt("uShadingType", 1);
        worldShader.setMat4("uModel", Mat4::translate(Vec3(0, 0, 0)));
        worldShader.setVec4("uColor", Vec4(1, 1, 1, 1));
        meshPitch.draw();

        // 2. Render Field Lines
        worldShader.setInt("uShadingType", 2);
        worldShader.setMat4("uModel", Mat4::identity());
        worldShader.setVec4("uColor", Vec4(1, 1, 1, 1));
        meshFieldLines.draw();

        // 3. Render Stadium Stands & Towers
        worldShader.setInt("uShadingType", 0);
        worldShader.setMat4("uModel", Mat4::identity());
        meshStadium.draw();

        // Floodlight Towers in 4 corners
        float towerX = HALF_PITCH_L + 6.0f;
        float towerZ = HALF_PITCH_W + 6.0f;
        Vec3 towerPositions[4] = {
            Vec3(-towerX, 12.0f, -towerZ),
            Vec3( towerX, 12.0f, -towerZ),
            Vec3(-towerX, 12.0f,  towerZ),
            Vec3( towerX, 12.0f,  towerZ)
        };
        for (int i = 0; i < 4; ++i) {
            worldShader.setMat4("uModel", Mat4::translate(towerPositions[i]));
            worldShader.setVec4("uColor", Vec4(0.75f, 0.78f, 0.82f, 1.0f));
            meshTower.draw();
        }

        // LED Boards surrounding pitch
        for (float x = -HALF_PITCH_L; x <= HALF_PITCH_L; x += 3.5f) {
            // North & South boards
            Mat4 mN = Mat4::translate(Vec3(x, 0.45f, HALF_PITCH_W + 1.2f)) * Mat4::scale(Vec3(3.2f, 1.0f, 1.0f));
            Mat4 mS = Mat4::translate(Vec3(x, 0.45f, -HALF_PITCH_W - 1.2f)) * Mat4::scale(Vec3(3.2f, 1.0f, 1.0f));
            // Pulsing LED colors
            float ledColorShift = std::sin(x * 0.2f + (float)glfwGetTime() * 2.0f);
            Vec4 ledCol = (ledColorShift > 0.0f) ? Vec4(0.1f, 0.5f, 0.9f, 1.0f) : Vec4(0.9f, 0.7f, 0.1f, 1.0f);

            worldShader.setInt("uShadingType", 2);
            worldShader.setVec4("uColor", ledCol);
            worldShader.setMat4("uModel", mN);
            meshLED.draw();
            worldShader.setMat4("uModel", mS);
            meshLED.draw();
        }

        // 4. Render Goal Frames & Nets
        worldShader.setInt("uShadingType", 0);
        worldShader.setMat4("uModel", Mat4::identity());
        worldShader.setVec4("uColor", Vec4(0.95f, 0.95f, 0.98f, 1.0f));
        meshGoalFrame.draw();

        worldShader.setInt("uShadingType", 4);
        meshGoalNet.draw();

        // 5. Render Player Shadows & Ball Shadow
        worldShader.setInt("uShadingType", 3);
        worldShader.setVec4("uColor", Vec4(0, 0, 0, 0.45f));
        for (auto& p : players) {
            Mat4 shadowMat = Mat4::translate(Vec3(p.pos.x, 0.03f, p.pos.z)) * Mat4::scale(Vec3(1.6f, 1.0f, 1.6f));
            worldShader.setMat4("uModel", shadowMat);
            meshShadow.draw();
        }
        // Ball Shadow (Scales with height)
        float bShadowScale = std::max(0.4f, 1.2f - ball.pos.y * 0.12f);
        float bShadowAlpha = std::max(0.15f, 0.6f - ball.pos.y * 0.08f);
        worldShader.setVec4("uColor", Vec4(0, 0, 0, bShadowAlpha));
        Mat4 ballShadowMat = Mat4::translate(Vec3(ball.pos.x, 0.035f, ball.pos.z)) * Mat4::scale(Vec3(bShadowScale, 1.0f, bShadowScale));
        worldShader.setMat4("uModel", ballShadowMat);
        meshShadow.draw();

        // 6. Render Players
        renderPlayers();

        // 7. Render Active Player Selection Halo Ring & Arrow
        renderIndicators();

        // 8. Render Ball
        worldShader.setInt("uShadingType", 5);
        Mat4 ballMat = Mat4::translate(ball.pos) * ball.rotation;
        worldShader.setMat4("uModel", ballMat);
        worldShader.setVec4("uColor", Vec4(1, 1, 1, 1));
        meshBall.draw();

        // 9. Render Particles
        renderParticles();

        // 10. Render 2D HUD / UI
        glDisable(GL_DEPTH_TEST);
        renderHUD(screenW, screenH);
    }

    void renderPlayers() {
        for (auto& p : players) {
            worldShader.setInt("uShadingType", 0);

            // Team Colors: Blue Team vs Red Team
            Vec4 jerseyColor = (p.team == 0) ? Vec4(0.12f, 0.38f, 0.95f, 1.0f) : Vec4(0.92f, 0.18f, 0.18f, 1.0f);
            Vec4 shortsColor = (p.team == 0) ? Vec4(0.95f, 0.95f, 0.98f, 1.0f) : Vec4(0.12f, 0.12f, 0.15f, 1.0f);
            Vec4 skinColor = Vec4(0.94f, 0.76f, 0.62f, 1.0f);
            Vec4 bootsColor = Vec4(0.1f, 0.1f, 0.1f, 1.0f);

            if (p.role == PlayerRole::GK) {
                jerseyColor = (p.team == 0) ? Vec4(0.1f, 0.85f, 0.3f, 1.0f) : Vec4(0.95f, 0.85f, 0.1f, 1.0f);
            }

            // Stunned flashing or celebratory jumping
            float jumpY = 0.0f;
            if (p.state == PlayerState::CELEBRATING) {
                jumpY = std::abs(std::sin(p.stateTimer * 8.0f)) * 0.8f;
            }

            Mat4 playerRoot = Mat4::translate(p.pos + Vec3(0, jumpY, 0)) * Mat4::rotateY(p.facingAngle);

            if (p.state == PlayerState::TACKLING) {
                playerRoot = playerRoot * Mat4::rotateZ(45.0f * DEG2RAD) * Mat4::translate(Vec3(0, -0.4f, 0));
            }

            // Torso (Jersey)
            Mat4 torsoMat = playerRoot * Mat4::translate(Vec3(0, 1.35f, 0));
            worldShader.setMat4("uModel", torsoMat);
            worldShader.setVec4("uColor", jerseyColor);
            meshTorso.draw();

            // Head
            Mat4 headMat = playerRoot * Mat4::translate(Vec3(0, 2.1f, 0));
            worldShader.setMat4("uModel", headMat);
            worldShader.setVec4("uColor", skinColor);
            meshHead.draw();

            // Hair
            Mat4 hairMat = playerRoot * Mat4::translate(Vec3(0, 2.22f, -0.05f)) * Mat4::scale(Vec3(0.85f, 0.45f, 0.85f));
            worldShader.setMat4("uModel", hairMat);
            worldShader.setVec4("uColor", Vec4(0.18f, 0.12f, 0.08f, 1.0f));
            meshHead.draw();

            // Legs swing animation
            float legSwing = std::sin(p.runAnim) * 0.55f;
            float armSwing = std::cos(p.runAnim) * 0.55f;

            // Left Leg (Thigh + Boot)
            Mat4 lLegMat = playerRoot * Mat4::translate(Vec3(0.24f, 0.85f, 0)) * Mat4::rotateX(legSwing) * Mat4::translate(Vec3(0, -0.38f, 0));
            worldShader.setMat4("uModel", lLegMat);
            worldShader.setVec4("uColor", shortsColor);
            meshLimb.draw();

            Mat4 lBootMat = lLegMat * Mat4::translate(Vec3(0, -0.4f, 0.08f)) * Mat4::scale(Vec3(1.1f, 0.4f, 1.5f));
            worldShader.setMat4("uModel", lBootMat);
            worldShader.setVec4("uColor", bootsColor);
            meshTorso.draw();

            // Right Leg
            Mat4 rLegMat = playerRoot * Mat4::translate(Vec3(-0.24f, 0.85f, 0)) * Mat4::rotateX(-legSwing) * Mat4::translate(Vec3(0, -0.38f, 0));
            worldShader.setMat4("uModel", rLegMat);
            worldShader.setVec4("uColor", shortsColor);
            meshLimb.draw();

            Mat4 rBootMat = rLegMat * Mat4::translate(Vec3(0, -0.4f, 0.08f)) * Mat4::scale(Vec3(1.1f, 0.4f, 1.5f));
            worldShader.setMat4("uModel", rBootMat);
            worldShader.setVec4("uColor", bootsColor);
            meshTorso.draw();

            // Arms
            Mat4 lArmMat = playerRoot * Mat4::translate(Vec3(0.55f, 1.45f, 0)) * Mat4::rotateX(-armSwing) * Mat4::translate(Vec3(0, -0.35f, 0));
            worldShader.setMat4("uModel", lArmMat);
            worldShader.setVec4("uColor", skinColor);
            meshLimb.draw();

            Mat4 rArmMat = playerRoot * Mat4::translate(Vec3(-0.55f, 1.45f, 0)) * Mat4::rotateX(armSwing) * Mat4::translate(Vec3(0, -0.35f, 0));
            worldShader.setMat4("uModel", rArmMat);
            worldShader.setVec4("uColor", skinColor);
            meshLimb.draw();
        }
    }

    void renderIndicators() {
        worldShader.setInt("uShadingType", 2);

        // P1 Halo Indicator Ring (Team Blue)
        Player& p1 = players[controlledPlayerBlue];
        float ringRot = (float)glfwGetTime() * 3.0f;
        Mat4 ringMat = Mat4::translate(p1.pos) * Mat4::rotateY(ringRot);
        worldShader.setMat4("uModel", ringMat);
        worldShader.setVec4("uColor", Vec4(0.2f, 0.9f, 1.0f, 0.9f));
        meshRing.draw();

        // 3D Floating Arrow above P1
        float arrowBob = std::sin((float)glfwGetTime() * 6.0f) * 0.15f;
        Mat4 arrowMat = Mat4::translate(p1.pos + Vec3(0, 3.1f + arrowBob, 0)) * Mat4::rotateX(PI);
        worldShader.setMat4("uModel", arrowMat);
        worldShader.setVec4("uColor", Vec4(0.2f, 0.9f, 1.0f, 1.0f));
        meshArrow.draw();

        // P2 Halo Ring if 2-Player mode
        if (twoPlayerMode) {
            Player& p2 = players[controlledPlayerRed];
            Mat4 rMat2 = Mat4::translate(p2.pos) * Mat4::rotateY(ringRot);
            worldShader.setMat4("uModel", rMat2);
            worldShader.setVec4("uColor", Vec4(1.0f, 0.3f, 0.2f, 0.9f));
            meshRing.draw();

            Mat4 arrowMat2 = Mat4::translate(p2.pos + Vec3(0, 3.1f + arrowBob, 0)) * Mat4::rotateX(PI);
            worldShader.setMat4("uModel", arrowMat2);
            worldShader.setVec4("uColor", Vec4(1.0f, 0.3f, 0.2f, 1.0f));
            meshArrow.draw();
        }
    }

    void renderParticles() {
        worldShader.setInt("uShadingType", 2);
        for (const auto& p : particles) {
            Mat4 pMat = Mat4::translate(p.pos) * Mat4::scale(Vec3(p.size, p.size, p.size));
            worldShader.setMat4("uModel", pMat);
            worldShader.setVec4("uColor", p.color);
            meshBall.draw();
        }
    }

    void drawRect(float x, float y, float w, float h, Vec4 color, Mat4 orthoProj, int shapeType = 0) {
        uiShader.setMat4("uProjection", orthoProj);
        uiShader.setInt("uShapeType", shapeType);
        uiShader.setVec4("uColor", color);

        Mat4 model = Mat4::translate(Vec3(x, y, 0)) * Mat4::scale(Vec3(w, h, 1));
        uiShader.setMat4("uProjection", orthoProj * model);
        meshQuad.draw();
    }

    void drawChar(char c, float x, float y, float size, Vec4 color, Mat4 orthoProj) {
        if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';

        // 5x7 Font Patterns (7 rows, 5 bits each)
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
            case ':': rows[0]=0x00; rows[1]=0x04; rows[2]=0x04; rows[3]=0x00; rows[4]=0x04; rows[5]=0x04; rows[6]=0x00; break;
            case '-': rows[0]=0x00; rows[1]=0x00; rows[2]=0x00; rows[3]=0x1F; rows[4]=0x00; rows[5]=0x00; rows[6]=0x00; break;
            case '/': rows[0]=0x01; rows[1]=0x02; rows[2]=0x04; rows[3]=0x08; rows[4]=0x10; rows[5]=0x00; rows[6]=0x00; break;
            case '[': rows[0]=0x0E; rows[1]=0x08; rows[2]=0x08; rows[3]=0x08; rows[4]=0x08; rows[5]=0x08; rows[6]=0x0E; break;
            case ']': rows[0]=0x0E; rows[1]=0x02; rows[2]=0x02; rows[3]=0x02; rows[4]=0x02; rows[5]=0x02; rows[6]=0x0E; break;
            case '(': rows[0]=0x02; rows[1]=0x04; rows[2]=0x08; rows[3]=0x08; rows[4]=0x08; rows[5]=0x04; rows[6]=0x02; break;
            case ')': rows[0]=0x08; rows[1]=0x04; rows[2]=0x02; rows[3]=0x02; rows[4]=0x02; rows[5]=0x04; rows[6]=0x08; break;
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
                drawChar(c, curX, y, size, color, orthoProj);
            }
            curX += spacing;
        }
    }

    void renderHUD(int screenW, int screenH) {
        uiShader.use();
        Mat4 orthoProj = Mat4::ortho(0.0f, (float)screenW, 0.0f, (float)screenH, -1.0f, 1.0f);

        // 1. Top Scoreboard Banner
        float boardW = 380.0f;
        float boardH = 68.0f;
        float boardX = (screenW - boardW) * 0.5f;
        float boardY = screenH - boardH - 16.0f;

        // Background card
        drawRect(boardX, boardY, boardW, boardH, Vec4(0.06f, 0.08f, 0.12f, 0.90f), orthoProj);
        // Team colored header stripes
        drawRect(boardX, boardY + boardH - 5.0f, boardW * 0.5f, 5.0f, Vec4(0.12f, 0.45f, 0.95f, 1.0f), orthoProj);
        drawRect(boardX + boardW * 0.5f, boardY + boardH - 5.0f, boardW * 0.5f, 5.0f, Vec4(0.95f, 0.2f, 0.2f, 1.0f), orthoProj);

        // Team Names
        drawText("BLUE", boardX + 22.0f, boardY + 44.0f, 16.0f, Vec4(0.3f, 0.8f, 1.0f, 1.0f), orthoProj);
        drawText("RED", boardX + boardW - 65.0f, boardY + 44.0f, 16.0f, Vec4(1.0f, 0.4f, 0.4f, 1.0f), orthoProj);

        // Scores
        drawText(std::to_string(scoreBlue), boardX + 110.0f, boardY + 16.0f, 34.0f, Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj);
        drawText("-", boardX + boardW * 0.5f - 8.0f, boardY + 20.0f, 28.0f, Vec4(0.8f, 0.8f, 0.85f, 1.0f), orthoProj);
        drawText(std::to_string(scoreRed), boardX + boardW - 130.0f, boardY + 16.0f, 34.0f, Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj);

        // Match Time (MM:SS)
        int minutes = (int)matchTime / 60;
        int seconds = (int)matchTime % 60;
        std::stringstream ssTime;
        ssTime << (minutes < 10 ? "0" : "") << minutes << ":" << (seconds < 10 ? "0" : "") << seconds;

        float timeW = 110.0f;
        float timeX = (screenW - timeW) * 0.5f;
        float timeY = boardY - 26.0f;
        drawRect(timeX, timeY, timeW, 22.0f, Vec4(0.04f, 0.05f, 0.08f, 0.80f), orthoProj);
        drawText(ssTime.str(), timeX + 20.0f, timeY + 3.0f, 16.0f, Vec4(1.0f, 0.88f, 0.2f, 1.0f), orthoProj);

        // Camera mode hint in top-left
        std::string camName = (cameraMode == 0) ? "CAM: BROADCAST TV" : (cameraMode == 1 ? "CAM: ACTION FOLLOW" : "CAM: TOP-DOWN");
        drawText(camName, 20.0f, screenH - 32.0f, 14.0f, Vec4(0.9f, 0.9f, 0.95f, 0.85f), orthoProj);
        drawText("[H] HUONG DAN  [V] DOI CAM  [R] RESTART", 20.0f, screenH - 52.0f, 12.0f, Vec4(0.7f, 0.75f, 0.85f, 0.75f), orthoProj);

        // 2. Shot Power Charge Bar (when charging)
        if (p1ShootCharge > 0.05f) {
            float barW = 180.0f;
            float barH = 16.0f;
            float barX = (screenW - barW) * 0.5f;
            float barY = 95.0f;

            drawRect(barX - 3.0f, barY - 3.0f, barW + 6.0f, barH + 6.0f, Vec4(0.05f, 0.05f, 0.08f, 0.9f), orthoProj);
            Vec4 powerCol = lerp(Vec3(0.2f, 0.95f, 0.3f), Vec3(1.0f, 0.25f, 0.1f), p1ShootCharge);
            drawRect(barX, barY, barW * p1ShootCharge, barH, powerCol, orthoProj);
            drawText("LUC SUT", barX + 50.0f, barY + barH + 6.0f, 14.0f, Vec4(1.0f, 0.9f, 0.2f, 1.0f), orthoProj);
        }

        // 3. Mini-Map Radar (Bottom-Right corner)
        renderRadar(screenW, screenH, orthoProj);

        // 4. "GOAL!" Celebration Banner
        if (isGoalCelebration) {
            float bannerW = 440.0f;
            float bannerH = 110.0f;
            float bannerX = (screenW - bannerW) * 0.5f;
            float bannerY = (screenH - bannerH) * 0.5f + 40.0f;

            float pulse = 1.0f + 0.08f * std::sin(goalCelebrationTimer * 14.0f);
            Vec4 bannerBg = (scoringTeam == 0) ? Vec4(0.1f, 0.45f, 0.95f, 0.94f) : Vec4(0.95f, 0.2f, 0.2f, 0.94f);

            drawRect(bannerX - (bannerW*(pulse-1.0f)*0.5f), bannerY - (bannerH*(pulse-1.0f)*0.5f),
                     bannerW * pulse, bannerH * pulse, bannerBg, orthoProj);

            drawRect(bannerX + 8.0f, bannerY + 8.0f, bannerW - 16.0f, bannerH - 16.0f,
                     Vec4(0.04f, 0.06f, 0.1f, 0.7f), orthoProj);

            drawText("GOAAALLL!", bannerX + 90.0f, bannerY + 46.0f, 44.0f, Vec4(1.0f, 0.9f, 0.1f, 1.0f), orthoProj);
            std::string teamScored = (scoringTeam == 0) ? "TEAM BLUE GHI BAN!" : "TEAM RED GHI BAN!";
            drawText(teamScored, bannerX + 115.0f, bannerY + 18.0f, 16.0f, Vec4(1.0f, 1.0f, 1.0f, 0.9f), orthoProj);
        }

        // 5. Help Overlay (when toggled with H)
        if (showHelp) {
            renderHelpMenu(screenW, screenH, orthoProj);
        }

        // 6. Pause Banner
        if (isPaused) {
            float pW = 260.0f;
            float pH = 60.0f;
            float pX = (screenW - pW) * 0.5f;
            float pY = (screenH - pH) * 0.5f;
            drawRect(pX, pY, pW, pH, Vec4(0.02f, 0.04f, 0.08f, 0.92f), orthoProj);
            drawText("TAM DUNG (PAUSED)", pX + 22.0f, pY + 22.0f, 18.0f, Vec4(1.0f, 0.9f, 0.2f, 1.0f), orthoProj);
        }
    }

    void renderRadar(int screenW, int screenH, Mat4 orthoProj) {
        float rW = 180.0f;
        float rH = 120.0f;
        float rX = screenW - rW - 20.0f;
        float rY = 20.0f;

        // Radar background
        drawRect(rX, rY, rW, rH, Vec4(0.08f, 0.22f, 0.1f, 0.85f), orthoProj);
        // Border
        drawRect(rX, rY, rW, 2.0f, Vec4(0.9f, 0.9f, 0.9f, 0.8f), orthoProj);
        drawRect(rX, rY + rH - 2.0f, rW, 2.0f, Vec4(0.9f, 0.9f, 0.9f, 0.8f), orthoProj);
        drawRect(rX, rY, 2.0f, rH, Vec4(0.9f, 0.9f, 0.9f, 0.8f), orthoProj);
        drawRect(rX + rW - 2.0f, rY, 2.0f, rH, Vec4(0.9f, 0.9f, 0.9f, 0.8f), orthoProj);
        // Halfway line
        drawRect(rX + rW * 0.5f - 1.0f, rY, 2.0f, rH, Vec4(0.9f, 0.9f, 0.9f, 0.5f), orthoProj);

        // Player dots
        for (const auto& p : players) {
            float normX = (p.pos.x / HALF_PITCH_L) * 0.5f + 0.5f;
            float normZ = (p.pos.z / HALF_PITCH_W) * 0.5f + 0.5f;

            float dotX = rX + normX * rW - 3.0f;
            float dotY = rY + (1.0f - normZ) * rH - 3.0f;

            Vec4 dotColor = (p.team == 0) ? Vec4(0.2f, 0.8f, 1.0f, 1.0f) : Vec4(1.0f, 0.3f, 0.3f, 1.0f);
            if (p.isControlled) dotColor = Vec4(1.0f, 1.0f, 0.2f, 1.0f);

            drawRect(dotX, dotY, 6.0f, 6.0f, dotColor, orthoProj, 1);
        }

        // Ball dot
        float bNormX = (ball.pos.x / HALF_PITCH_L) * 0.5f + 0.5f;
        float bNormZ = (ball.pos.z / HALF_PITCH_W) * 0.5f + 0.5f;
        float bDotX = rX + bNormX * rW - 3.5f;
        float bDotY = rY + (1.0f - bNormZ) * rH - 3.5f;
        drawRect(bDotX, bDotY, 7.0f, 7.0f, Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj, 1);
    }

    void renderHelpMenu(int screenW, int screenH, Mat4 orthoProj) {
        float menuW = 560.0f;
        float menuH = 400.0f;
        float menuX = (screenW - menuW) * 0.5f;
        float menuY = (screenH - menuH) * 0.5f;

        // Dark modal box
        drawRect(menuX, menuY, menuW, menuH, Vec4(0.04f, 0.06f, 0.1f, 0.95f), orthoProj);
        // Header
        drawRect(menuX, menuY + menuH - 38.0f, menuW, 38.0f, Vec4(0.12f, 0.4f, 0.85f, 1.0f), orthoProj);
        drawText("BANG HUONG DAN DIEU KHIEN", menuX + 115.0f, menuY + menuH - 26.0f, 18.0f, Vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProj);

        // Border
        drawRect(menuX, menuY, menuW, 2.0f, Vec4(0.3f, 0.6f, 1.0f, 0.8f), orthoProj);
        drawRect(menuX, menuY + menuH - 2.0f, menuW, 2.0f, Vec4(0.3f, 0.6f, 1.0f, 0.8f), orthoProj);
        drawRect(menuX, menuY, 2.0f, menuH, Vec4(0.3f, 0.6f, 1.0f, 0.8f), orthoProj);
        drawRect(menuX + menuW - 2.0f, menuY, 2.0f, menuH, Vec4(0.3f, 0.6f, 1.0f, 0.8f), orthoProj);

        float lineY = menuY + menuH - 72.0f;
        float step = 28.0f;
        Vec4 keyCol(1.0f, 0.88f, 0.25f, 1.0f);
        Vec4 descCol(0.9f, 0.95f, 1.0f, 0.9f);

        drawText("[W, A, S, D / MUI TEN]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("DI CHUYEN CAU THU", menuX + 270.0f, lineY, 14.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[GIU SHIFT]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("CHAY NHANH (SPRINT)", menuX + 270.0f, lineY, 14.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[SPACE / J]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("SUT BONG (GIU DE SAC LUC)", menuX + 270.0f, lineY, 14.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[K / X]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("CHUYEN BONG CHO DONG DOI", menuX + 270.0f, lineY, 14.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[L / C]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("XOAC BONG / TRANH CUOP", menuX + 270.0f, lineY, 14.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[TAB / Q]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("DOI CAU THU DIEU KHIEN", menuX + 270.0f, lineY, 14.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[V / C / F1-F3]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("DOI GOC CAMERA", menuX + 270.0f, lineY, 14.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[T]", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("BAT/TAT CHE DO 2 NGUOI (PVP)", menuX + 270.0f, lineY, 14.0f, descCol, orthoProj);
        lineY -= step;

        drawText("[R] RESTART", menuX + 28.0f, lineY, 14.0f, keyCol, orthoProj);
        drawText("[P / ESC] TAM DUNG  [H] DONG", menuX + 270.0f, lineY, 14.0f, descCol, orthoProj);
    }
};

#endif // SOCCER_GAME_HPP
