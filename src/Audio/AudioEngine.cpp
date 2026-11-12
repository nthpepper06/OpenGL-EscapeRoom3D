#include "audio/AudioEngine.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmsystem.h>
#endif

static constexpr int SAMPLE_RATE = 44100;
static constexpr int BUFFER_FRAMES = 2048;
static constexpr float PI = 3.14159265358979323846f;

#ifdef _WIN32
struct WinAudioData {
    HWAVEOUT hWaveOut = nullptr;
    WAVEHDR waveHeaders[2];
    short audioBuffers[2][BUFFER_FRAMES * 2]; // 16-bit stereo
};
static WinAudioData g_winAudio;
#endif

AudioEngine::AudioEngine() {
}

AudioEngine::~AudioEngine() {
    shutdown();
}

bool AudioEngine::init() {
#ifdef _WIN32
    WAVEFORMATEX wfx;
    ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2; // Stereo
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    MMRESULT result = waveOutOpen(&g_winAudio.hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    if (result != MMSYSERR_NOERROR) {
        std::cerr << "[AudioEngine] Warning: Could not initialize Windows WaveOut audio device (code " 
                  << result << "). Audio will run in silent fallback mode.\n";
        enabled = false;
        return false;
    }

    ZeroMemory(g_winAudio.audioBuffers, sizeof(g_winAudio.audioBuffers));
    for (int i = 0; i < 2; ++i) {
        ZeroMemory(&g_winAudio.waveHeaders[i], sizeof(WAVEHDR));
        g_winAudio.waveHeaders[i].lpData = reinterpret_cast<LPSTR>(g_winAudio.audioBuffers[i]);
        g_winAudio.waveHeaders[i].dwBufferLength = BUFFER_FRAMES * 2 * sizeof(short);
        waveOutPrepareHeader(g_winAudio.hWaveOut, &g_winAudio.waveHeaders[i], sizeof(WAVEHDR));
    }

    isRunning = true;
    audioThread = std::thread(&AudioEngine::audioLoop, this);
    std::cout << "[AudioEngine] Procedural 3D Spatial Audio Engine initialized (44100Hz Stereo).\n";
    return true;
#else
    enabled = false;
    return false;
#endif
}

void AudioEngine::shutdown() {
    if (!isRunning) return;
    isRunning = false;

    if (audioThread.joinable()) {
        audioThread.join();
    }

#ifdef _WIN32
    if (g_winAudio.hWaveOut) {
        waveOutReset(g_winAudio.hWaveOut);
        for (int i = 0; i < 2; ++i) {
            waveOutUnprepareHeader(g_winAudio.hWaveOut, &g_winAudio.waveHeaders[i], sizeof(WAVEHDR));
        }
        waveOutClose(g_winAudio.hWaveOut);
        g_winAudio.hWaveOut = nullptr;
    }
#endif
}

void AudioEngine::playFootstep(const glm::vec3& position) {
    if (!enabled || muted) return;
    std::lock_guard<std::mutex> lock(soundMutex);
    ActiveSound snd;
    snd.type = SoundType::Footstep;
    snd.position = position;
    snd.isPositional = true;
    snd.playbackPos = 0.0f;
    snd.duration = 0.16f;
    snd.volume = 0.55f;
    activeSounds.push_back(snd);
}

void AudioEngine::playKeypadBeep(bool success) {
    if (!enabled || muted) return;
    std::lock_guard<std::mutex> lock(soundMutex);
    ActiveSound snd;
    snd.type = success ? SoundType::KeypadBeep : SoundType::KeypadError;
    snd.isPositional = false;
    snd.playbackPos = 0.0f;
    snd.duration = success ? 0.09f : 0.22f;
    snd.volume = 0.6f;
    activeSounds.push_back(snd);
}

void AudioEngine::playSafeUnlock(const glm::vec3& position) {
    if (!enabled || muted) return;
    std::lock_guard<std::mutex> lock(soundMutex);
    ActiveSound snd;
    snd.type = SoundType::SafeUnlock;
    snd.position = position;
    snd.isPositional = true;
    snd.playbackPos = 0.0f;
    snd.duration = 0.45f;
    snd.volume = 0.9f;
    activeSounds.push_back(snd);
}

void AudioEngine::playDrawerSlide(const glm::vec3& position) {
    if (!enabled || muted) return;
    std::lock_guard<std::mutex> lock(soundMutex);
    ActiveSound snd;
    snd.type = SoundType::DrawerSlide;
    snd.position = position;
    snd.isPositional = true;
    snd.playbackPos = 0.0f;
    snd.duration = 0.38f;
    snd.volume = 0.7f;
    activeSounds.push_back(snd);
}

void AudioEngine::playDoorCreak(const glm::vec3& position) {
    if (!enabled || muted) return;
    std::lock_guard<std::mutex> lock(soundMutex);
    ActiveSound snd;
    snd.type = SoundType::DoorCreak;
    snd.position = position;
    snd.isPositional = true;
    snd.playbackPos = 0.0f;
    snd.duration = 0.65f;
    snd.volume = 0.85f;
    activeSounds.push_back(snd);
}

void AudioEngine::playItemPickup() {
    if (!enabled || muted) return;
    std::lock_guard<std::mutex> lock(soundMutex);
    ActiveSound snd;
    snd.type = SoundType::ItemPickup;
    snd.isPositional = false;
    snd.playbackPos = 0.0f;
    snd.duration = 0.85f;
    snd.volume = 0.8f;
    activeSounds.push_back(snd);
}

void AudioEngine::updateListener(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up) {
    std::lock_guard<std::mutex> lock(soundMutex);
    listenerPos = pos;
    listenerForward = glm::normalize(forward);
    listenerUp = glm::normalize(up);
    listenerRight = glm::normalize(glm::cross(listenerForward, listenerUp));
}

void AudioEngine::updateAtmosphere(float fearFactor, const glm::vec3& lightPos, bool lightFlickering) {
    std::lock_guard<std::mutex> lock(soundMutex);
    currentFear = std::clamp(fearFactor, 0.0f, 1.0f);
    ceilingLightPos = lightPos;
    lightFlicker = lightFlickering;
}

void AudioEngine::audioLoop() {
#ifdef _WIN32
    int currentBuf = 0;

    // Prefill first buffer
    renderAudioBlock(g_winAudio.audioBuffers[0], BUFFER_FRAMES);
    waveOutWrite(g_winAudio.hWaveOut, &g_winAudio.waveHeaders[0], sizeof(WAVEHDR));

    renderAudioBlock(g_winAudio.audioBuffers[1], BUFFER_FRAMES);
    waveOutWrite(g_winAudio.hWaveOut, &g_winAudio.waveHeaders[1], sizeof(WAVEHDR));

    while (isRunning) {
        // Wait for buffer to finish
        if (g_winAudio.waveHeaders[currentBuf].dwFlags & WHDR_DONE) {
            renderAudioBlock(g_winAudio.audioBuffers[currentBuf], BUFFER_FRAMES);
            waveOutWrite(g_winAudio.hWaveOut, &g_winAudio.waveHeaders[currentBuf], sizeof(WAVEHDR));
            currentBuf = (currentBuf + 1) % 2;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
#endif
}

void AudioEngine::renderAudioBlock(short* outputBuffer, int numFrames) {
    std::lock_guard<std::mutex> lock(soundMutex);

    if (muted || !enabled) {
        std::fill_n(outputBuffer, numFrames * 2, static_cast<short>(0));
        return;
    }

    const float dt = 1.0f / static_cast<float>(SAMPLE_RATE);
    static std::mt19937 rng(1337);
    static std::uniform_real_distribution<float> whiteDist(-1.0f, 1.0f);

    // Compute 3D light spatial attenuation
    float lightDist = glm::length(ceilingLightPos - listenerPos);
    float lightAtten = 1.0f / (1.0f + 0.5f * lightDist + 0.1f * lightDist * lightDist);
    glm::vec3 lightDir = (lightDist > 0.001f) ? (ceilingLightPos - listenerPos) / lightDist : glm::vec3(0.0f);
    float lightPan = std::clamp(glm::dot(lightDir, listenerRight), -1.0f, 1.0f);
    float lightL = std::sqrt(0.5f * (1.0f - lightPan)) * lightAtten;
    float lightR = std::sqrt(0.5f * (1.0f + lightPan)) * lightAtten;

    // Heartbeat BPM calculation
    float heartRateBPM = 60.0f + currentFear * 70.0f; // 60 to 130 BPM
    float heartPeriod = 60.0f / heartRateBPM;

    for (int i = 0; i < numFrames; ++i) {
        float mixL = 0.0f;
        float mixR = 0.0f;

        // 1. Eerie Low-frequency Horror Drone (55Hz and 58Hz beating)
        dronePhase += 2.0f * PI * 55.0f * dt;
        if (dronePhase > 2.0f * PI) dronePhase -= 2.0f * PI;
        float droneSample = std::sin(dronePhase) * 0.6f + std::sin(dronePhase * 1.0545f) * 0.4f;
        float droneAmp = ambientVolume * (0.12f + 0.25f * currentFear);
        mixL += droneSample * droneAmp;
        mixR += droneSample * droneAmp;

        // 2. Electrical Ceiling Light Arc Hum (60Hz + 120Hz harmonics)
        humPhase += 2.0f * PI * 60.0f * dt;
        if (humPhase > 2.0f * PI) humPhase -= 2.0f * PI;
        float humSample = (std::sin(humPhase) * 0.7f + std::sin(humPhase * 2.0f) * 0.3f);
        if (lightFlicker) {
            humSample += whiteDist(rng) * 0.4f;
        }
        float humAmp = ambientVolume * (lightFlicker ? 0.35f : 0.08f);
        mixL += humSample * humAmp * lightL;
        mixR += humSample * humAmp * lightR;

        // 3. Panic Heartbeat (active when fear > 0.3)
        if (currentFear > 0.3f) {
            heartbeatTimer += dt;
            if (heartbeatTimer >= heartPeriod) {
                heartbeatTimer -= heartPeriod;
                heartbeatPhase = 0.0f;
            }
            if (heartbeatTimer < 0.25f) {
                // First thump (lub)
                float t = heartbeatTimer;
                float env = std::sin(t / 0.25f * PI);
                float hb = std::sin(2.0f * PI * (65.0f - t * 60.0f) * t) * env;
                float hbAmp = masterVolume * (currentFear - 0.25f) * 0.45f;
                mixL += hb * hbAmp;
                mixR += hb * hbAmp;
            } else if (heartbeatTimer > 0.12f && heartbeatTimer < 0.35f) {
                // Second thump (dub)
                float t = heartbeatTimer - 0.12f;
                float env = std::sin(t / 0.23f * PI);
                float hb = std::sin(2.0f * PI * (55.0f - t * 40.0f) * t) * env;
                float hbAmp = masterVolume * (currentFear - 0.25f) * 0.35f;
                mixL += hb * hbAmp;
                mixR += hb * hbAmp;
            }
        }

        // 4. Synthesize Active Sound Effects
        for (auto& snd : activeSounds) {
            if (snd.playbackPos >= snd.duration) continue;

            float sndL = 1.0f;
            float sndR = 1.0f;
            if (snd.isPositional) {
                float dist = glm::length(snd.position - listenerPos);
                float atten = 1.0f / (1.0f + 0.45f * dist + 0.08f * dist * dist);
                glm::vec3 dir = (dist > 0.001f) ? (snd.position - listenerPos) / dist : glm::vec3(0.0f);
                float pan = std::clamp(glm::dot(dir, listenerRight), -1.0f, 1.0f);
                sndL = std::sqrt(0.5f * (1.0f - pan)) * atten;
                sndR = std::sqrt(0.5f * (1.0f + pan)) * atten;
            }

            float sample = 0.0f;
            float t = snd.playbackPos;
            float progress = t / snd.duration;

            switch (snd.type) {
                case SoundType::Footstep: {
                    // Muffled low-pass footstep impact
                    float env = std::exp(-t * 28.0f);
                    float noise = whiteDist(rng);
                    float thump = std::sin(2.0f * PI * 85.0f * t);
                    sample = (thump * 0.7f + noise * 0.3f) * env * snd.volume;
                    break;
                }
                case SoundType::KeypadBeep: {
                    // Crisp electronic beep (1200Hz)
                    float env = std::sin(progress * PI);
                    sample = std::sin(2.0f * PI * 1200.0f * t) * env * snd.volume;
                    break;
                }
                case SoundType::KeypadError: {
                    // Low buzz rejection tone (240Hz)
                    float env = std::sin(progress * PI);
                    sample = std::sin(2.0f * PI * 240.0f * t) * env * snd.volume;
                    break;
                }
                case SoundType::SafeUnlock: {
                    // Heavy steel ratchet clunk
                    float env = std::exp(-t * 12.0f);
                    float clunk = std::sin(2.0f * PI * 160.0f * t) * 0.8f + std::sin(2.0f * PI * 340.0f * t) * 0.3f;
                    sample = clunk * env * snd.volume;
                    break;
                }
                case SoundType::DrawerSlide: {
                    // Wooden / steel slide friction
                    float env = std::sin(progress * PI);
                    float scrape = whiteDist(rng) * 0.6f + std::sin(2.0f * PI * 120.0f * t) * 0.4f;
                    sample = scrape * env * snd.volume;
                    break;
                }
                case SoundType::DoorCreak: {
                    // Rusty iron hinge squeal
                    float freq = 550.0f - progress * 260.0f + std::sin(2.0f * PI * 40.0f * t) * 50.0f;
                    float env = std::sin(progress * PI);
                    sample = std::sin(2.0f * PI * freq * t) * env * snd.volume;
                    break;
                }
                case SoundType::ItemPickup: {
                    // Mystical chime (C-major triad: 523, 659, 784 Hz)
                    float env = std::exp(-t * 4.5f);
                    sample = (std::sin(2.0f * PI * 523.25f * t) * 0.4f +
                              std::sin(2.0f * PI * 659.25f * t) * 0.3f +
                              std::sin(2.0f * PI * 783.99f * t) * 0.3f) * env * snd.volume;
                    break;
                }
            }

            mixL += sample * sndL * sfxVolume;
            mixR += sample * sndR * sfxVolume;

            snd.playbackPos += dt;
        }

        // Master Volume & Soft Saturation Limiter (tanh)
        mixL *= masterVolume;
        mixR *= masterVolume;
        mixL = std::tanh(mixL);
        mixR = std::tanh(mixR);

        // Convert to 16-bit PCM integer
        outputBuffer[i * 2 + 0] = static_cast<short>(std::clamp(mixL * 32767.0f, -32768.0f, 32767.0f));
        outputBuffer[i * 2 + 1] = static_cast<short>(std::clamp(mixR * 32767.0f, -32768.0f, 32767.0f));
    }

    // Erase finished sounds
    activeSounds.erase(
        std::remove_if(activeSounds.begin(), activeSounds.end(),
                       [](const ActiveSound& s) { return s.playbackPos >= s.duration; }),
        activeSounds.end());
}
