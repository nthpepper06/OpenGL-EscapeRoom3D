#ifndef AUDIO_AUDIOMANAGER_HPP
#define AUDIO_AUDIOMANAGER_HPP

#include <windows.h>
#include <mmsystem.h>
#include <vector>
#include <cmath>
#include <thread>
#include <atomic>
#include <iostream>

#pragma comment(lib, "winmm.lib")

namespace Audio {

class AudioManager {
private:
    AudioManager() = default;

    void playPcmBuffer(const std::vector<short>& pcm, int sampleRate = 22050) {
        std::thread([pcm, sampleRate]() {
            int numSamples = (int)pcm.size();
            int dataSize = numSamples * 2;
            int headerSize = 44;
            int totalSize = headerSize + dataSize;

            std::vector<char> wav(totalSize);

            memcpy(&wav[0], "RIFF", 4);
            int fileSizeMinus8 = totalSize - 8;
            memcpy(&wav[4], &fileSizeMinus8, 4);
            memcpy(&wav[8], "WAVE", 4);

            memcpy(&wav[12], "fmt ", 4);
            int subChunk1Size = 16;
            memcpy(&wav[16], &subChunk1Size, 4);
            short audioFormat = 1; // PCM
            memcpy(&wav[20], &audioFormat, 2);
            short numChannels = 1; // Mono
            memcpy(&wav[22], &numChannels, 2);
            memcpy(&wav[24], &sampleRate, 4);
            int byteRate = sampleRate * 2;
            memcpy(&wav[28], &byteRate, 4);
            short blockAlign = 2;
            memcpy(&wav[32], &blockAlign, 2);
            short bitsPerSample = 16;
            memcpy(&wav[34], &bitsPerSample, 2);

            memcpy(&wav[36], "data", 4);
            memcpy(&wav[40], &dataSize, 4);
            memcpy(&wav[44], pcm.data(), dataSize);

            PlaySoundA(wav.data(), NULL, SND_MEMORY | SND_SYNC);
        }).detach();
    }

public:
    static AudioManager& instance() {
        static AudioManager mgr;
        return mgr;
    }

    void playARFire() {
        int sr = 22050;
        int n = (int)(sr * 0.12f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float env = std::exp(-t * 35.0f);
            float noise = ((rand() % 2000) / 1000.0f - 1.0f);
            float tone = std::sin(2.0f * 3.14159f * 145.0f * t);
            pcm[i] = (short)((tone * 0.45f + noise * 0.55f) * env * 24000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playSMGFire() {
        int sr = 22050;
        int n = (int)(sr * 0.08f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float env = std::exp(-t * 45.0f);
            float noise = ((rand() % 2000) / 1000.0f - 1.0f);
            pcm[i] = (short)(noise * env * 22000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playShotgunFire() {
        int sr = 22050;
        int n = (int)(sr * 0.28f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float env = std::exp(-t * 18.0f);
            float noise = ((rand() % 2000) / 1000.0f - 1.0f);
            float low = std::sin(2.0f * 3.14159f * 75.0f * t);
            pcm[i] = (short)((low * 0.5f + noise * 0.5f) * env * 29000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playSniperFire() {
        int sr = 22050;
        int n = (int)(sr * 0.45f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float env = std::exp(-t * 9.0f);
            float noise = ((rand() % 2000) / 1000.0f - 1.0f);
            float sub = std::sin(2.0f * 3.14159f * 60.0f * t);
            pcm[i] = (short)((sub * 0.6f + noise * 0.4f) * env * 32000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playHitmarker() {
        int sr = 22050;
        int n = (int)(sr * 0.06f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float tone = std::sin(2.0f * 3.14159f * 1800.0f * t);
            pcm[i] = (short)(tone * (1.0f - (float)i / n) * 16000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playHeadshot() {
        int sr = 22050;
        int n = (int)(sr * 0.15f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float bell = std::sin(2.0f * 3.14159f * 2400.0f * t) + 0.5f * std::sin(2.0f * 3.14159f * 4800.0f * t);
            pcm[i] = (short)(bell * std::exp(-t * 22.0f) * 22000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playReload() {
        int sr = 22050;
        int n = (int)(sr * 0.22f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float click = (std::sin(2.0f * 3.14159f * 950.0f * t) > 0.0f ? 1.0f : -1.0f);
            pcm[i] = (short)(click * std::exp(-t * 28.0f) * 15000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playBarrierDeploy() {
        int sr = 22050;
        int n = (int)(sr * 0.32f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float ice = std::sin(2.0f * 3.14159f * (450.0f + t * 900.0f) * t);
            pcm[i] = (short)(ice * std::exp(-t * 12.0f) * 24000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playPickup() {
        int sr = 22050;
        int n = (int)(sr * 0.12f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float tone = std::sin(2.0f * 3.14159f * (750.0f + t * 1200.0f) * t);
            pcm[i] = (short)(tone * (1.0f - (float)i / n) * 18000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playVictoryAnthem() {
        int sr = 22050;
        int n = (int)(sr * 1.6f);
        std::vector<short> pcm(n);
        float notes[4] = { 523.25f, 659.25f, 783.99f, 1046.50f }; // C5, E5, G5, C6
        int noteLen = n / 4;
        for (int i = 0; i < n; ++i) {
            int noteIdx = std::min(3, i / noteLen);
            float freq = notes[noteIdx];
            float t = (float)(i % noteLen) / sr;
            float chord = std::sin(2.0f * 3.14159f * freq * t) + 0.3f * std::sin(2.0f * 3.14159f * freq * 2.0f * t);
            pcm[i] = (short)(chord * std::exp(-t * 4.0f) * 26000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playButtonClick() {
        int sr = 22050;
        int n = (int)(sr * 0.04f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float tone = std::sin(2.0f * 3.14159f * 1200.0f * t);
            pcm[i] = (short)(tone * (1.0f - (float)i / n) * 14000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playExplosion() {
        int sr = 22050;
        int n = (int)(sr * 0.65f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float env = std::exp(-t * 6.5f);
            float noise = ((rand() % 2000) / 1000.0f - 1.0f);
            float sub = std::sin(2.0f * 3.14159f * (65.0f - t * 30.0f) * t);
            pcm[i] = (short)((sub * 0.65f + noise * 0.35f) * env * 32000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playIceWallHit() {
        int sr = 22050;
        int n = (int)(sr * 0.08f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float clink = std::sin(2.0f * 3.14159f * 2200.0f * t) + 0.4f * ((rand() % 2000) / 1000.0f - 1.0f);
            pcm[i] = (short)(clink * std::exp(-t * 35.0f) * 20000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playSkillSurge() {
        int sr = 22050;
        int n = (int)(sr * 0.45f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float sweep = std::sin(2.0f * 3.14159f * (350.0f + t * 1400.0f) * t);
            pcm[i] = (short)(sweep * std::exp(-t * 4.0f) * 26000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playSkillDash() {
        int sr = 22050;
        int n = (int)(sr * 0.22f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float swoosh = ((rand() % 2000) / 1000.0f - 1.0f) * std::sin(2.0f * 3.14159f * (800.0f - t * 600.0f) * t);
            pcm[i] = (short)(swoosh * std::exp(-t * 12.0f) * 26000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playSkillScan() {
        int sr = 22050;
        int n = (int)(sr * 0.5f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float ping = std::sin(2.0f * 3.14159f * 1760.0f * t) + 0.5f * std::sin(2.0f * 3.14159f * 880.0f * t);
            pcm[i] = (short)(ping * std::exp(-t * 6.0f) * 24000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playAirdropPlane() {
        int sr = 22050;
        int n = (int)(sr * 1.5f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float rumble = std::sin(2.0f * 3.14159f * 55.0f * t) * 0.6f + std::sin(2.0f * 3.14159f * 110.0f * t) * 0.4f;
            float noise = ((rand() % 2000) / 1000.0f - 1.0f) * 0.35f;
            float env = std::sin((float)i / n * 3.14159f); // Fade in & out
            pcm[i] = (short)((rumble + noise) * env * 22000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playAirdropLand() {
        int sr = 22050;
        int n = (int)(sr * 0.45f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float thud = std::sin(2.0f * 3.14159f * 80.0f * t) * std::exp(-t * 12.0f);
            float clatter = ((rand() % 2000) / 1000.0f - 1.0f) * std::exp(-t * 18.0f) * 0.5f;
            pcm[i] = (short)((thud + clatter) * 28000.0f);
        }
        playPcmBuffer(pcm, sr);
    }

    void playUAVSiren() {
        int sr = 22050;
        int n = (int)(sr * 1.2f);
        std::vector<short> pcm(n);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float freq = 850.0f + std::sin(t * 18.0f) * 350.0f;
            float siren = std::sin(2.0f * 3.14159f * freq * t);
            float env = (i < sr * 0.1f) ? (float)i / (sr * 0.1f) : ((n - i < sr * 0.2f) ? (float)(n - i) / (sr * 0.2f) : 1.0f);
            pcm[i] = (short)(siren * env * 18000.0f);
        }
        playPcmBuffer(pcm, sr);
    }
};

} // namespace Audio

#endif // AUDIO_AUDIOMANAGER_HPP
