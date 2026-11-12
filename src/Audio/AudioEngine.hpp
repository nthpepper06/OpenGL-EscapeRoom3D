#pragma once

#include "core/Common.hpp"
#include <glm/glm.hpp>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <string>

enum class SoundType {
    Footstep,
    KeypadBeep,
    KeypadError,
    SafeUnlock,
    DrawerSlide,
    DoorCreak,
    ItemPickup
};

struct ActiveSound {
    SoundType type;
    glm::vec3 position{0.0f};
    bool isPositional = false;
    float playbackPos = 0.0f; // in seconds
    float duration = 0.5f;
    float volume = 1.0f;
};

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    bool init();
    void shutdown();

    // Sound triggers
    void playFootstep(const glm::vec3& position);
    void playKeypadBeep(bool success);
    void playSafeUnlock(const glm::vec3& position);
    void playDrawerSlide(const glm::vec3& position);
    void playDoorCreak(const glm::vec3& position);
    void playItemPickup();

    // Continuous dynamic ambient audio state
    void updateListener(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up);
    void updateAtmosphere(float fearFactor, const glm::vec3& lightPos, bool lightFlickering);

    // Audio controls for Dear ImGui
    bool enabled = true;
    float masterVolume = 0.75f;
    float sfxVolume = 0.85f;
    float ambientVolume = 0.65f;
    bool muted = false;

private:
    std::atomic<bool> isRunning{false};
    std::thread audioThread;
    std::mutex soundMutex;

    // Listener 3D State
    glm::vec3 listenerPos{0.0f, 1.7f, 2.5f};
    glm::vec3 listenerForward{0.0f, 0.0f, -1.0f};
    glm::vec3 listenerRight{1.0f, 0.0f, 0.0f};
    glm::vec3 listenerUp{0.0f, 1.0f, 0.0f};

    // Horror Ambient States
    float currentFear = 0.0f;
    glm::vec3 ceilingLightPos{0.0f, 3.4f, 0.0f};
    bool lightFlicker = false;

    // Sound effect instances
    std::vector<ActiveSound> activeSounds;

    // Procedural synthesis phases
    float dronePhase = 0.0f;
    float humPhase = 0.0f;
    float heartbeatPhase = 0.0f;
    float heartbeatTimer = 0.0f;

    void audioLoop();
    void renderAudioBlock(short* outputBuffer, int numFrames);
    float synthesizeSample(float dt, float& leftOut, float& rightOut);
};
