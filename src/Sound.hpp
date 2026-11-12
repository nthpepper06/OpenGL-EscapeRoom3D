#ifndef SOUND_HPP
#define SOUND_HPP

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <vector>
#include <cmath>
#include <cstdint>

class SoundManager {
public:
    static SoundManager& instance() {
        static SoundManager inst;
        return inst;
    }

    void init() {
        generateAK47Wav();
        generateMP40Wav();
        generateShotgunWav();
        generateAWMWav();
        generateGlooWallWav();
        generateHitmarkerWav();
        generateHeadshotWav();
        generateBooyahWav();
        generateReloadWav();
        generateAlarmWav();
        generateParachuteWav();
        generatePickupWav();
        generateMedkitWav();
        generateLandWav();
    }

    void playAK47() { playSoundMemory(ak47Wav); }
    void playMP40() { playSoundMemory(mp40Wav); }
    void playShotgun() { playSoundMemory(shotgunWav); }
    void playAWM() { playSoundMemory(awmWav); }
    void playGlooWall() { playSoundMemory(glooWallWav); }
    void playHitmarker() { playSoundMemory(hitmarkerWav); }
    void playHeadshot() { playSoundMemory(headshotWav); }
    void playBooyah() { playSoundMemory(booyahWav); }
    void playReload() { playSoundMemory(reloadWav); }
    void playSafeZoneAlarm() { playSoundMemory(alarmWav); }
    void playParachuteOpen() { playSoundMemory(parachuteWav); }
    void playPickup() { playSoundMemory(pickupWav); }
    void playMedkit() { playSoundMemory(medkitWav); }
    void playLand() { playSoundMemory(landWav); }

private:
    std::vector<uint8_t> ak47Wav;
    std::vector<uint8_t> mp40Wav;
    std::vector<uint8_t> shotgunWav;
    std::vector<uint8_t> awmWav;
    std::vector<uint8_t> glooWallWav;
    std::vector<uint8_t> hitmarkerWav;
    std::vector<uint8_t> headshotWav;
    std::vector<uint8_t> booyahWav;
    std::vector<uint8_t> reloadWav;
    std::vector<uint8_t> alarmWav;
    std::vector<uint8_t> parachuteWav;
    std::vector<uint8_t> pickupWav;
    std::vector<uint8_t> medkitWav;
    std::vector<uint8_t> landWav;

    SoundManager() {
        init();
    }

    void playSoundMemory(const std::vector<uint8_t>& wavData) {
        if (!wavData.empty()) {
            PlaySoundA((LPCSTR)wavData.data(), NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
        }
    }

    std::vector<uint8_t> createWavHeader(int numSamples, int sampleRate = 22050) {
        std::vector<uint8_t> header(44);
        int dataSize = numSamples * 2;
        int totalSize = 36 + dataSize;

        header[0] = 'R'; header[1] = 'I'; header[2] = 'F'; header[3] = 'F';
        header[4] = (uint8_t)(totalSize & 0xFF);
        header[5] = (uint8_t)((totalSize >> 8) & 0xFF);
        header[6] = (uint8_t)((totalSize >> 16) & 0xFF);
        header[7] = (uint8_t)((totalSize >> 24) & 0xFF);
        header[8] = 'W'; header[9] = 'A'; header[10] = 'V'; header[11] = 'E';
        header[12] = 'f'; header[13] = 'm'; header[14] = 't'; header[15] = ' ';
        header[16] = 16; header[17] = 0; header[18] = 0; header[19] = 0;
        header[20] = 1; header[21] = 0;
        header[22] = 1; header[23] = 0;
        header[24] = (uint8_t)(sampleRate & 0xFF);
        header[25] = (uint8_t)((sampleRate >> 8) & 0xFF);
        header[26] = (uint8_t)((sampleRate >> 16) & 0xFF);
        header[27] = (uint8_t)((sampleRate >> 24) & 0xFF);
        int byteRate = sampleRate * 2;
        header[28] = (uint8_t)(byteRate & 0xFF);
        header[29] = (uint8_t)((byteRate >> 8) & 0xFF);
        header[30] = (uint8_t)((byteRate >> 16) & 0xFF);
        header[31] = (uint8_t)((byteRate >> 24) & 0xFF);
        header[32] = 2; header[33] = 0;
        header[34] = 16; header[35] = 0;
        header[36] = 'd'; header[37] = 'a'; header[38] = 't'; header[39] = 'a';
        header[40] = (uint8_t)(dataSize & 0xFF);
        header[41] = (uint8_t)((dataSize >> 8) & 0xFF);
        header[42] = (uint8_t)((dataSize >> 16) & 0xFF);
        header[43] = (uint8_t)((dataSize >> 24) & 0xFF);

        return header;
    }

    void generateAK47Wav() {
        int sampleRate = 22050;
        float duration = 0.22f;
        int numSamples = (int)(sampleRate * duration);
        ak47Wav = createWavHeader(numSamples, sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            float t = (float)i / sampleRate;
            float env = std::exp(-t * 26.0f);
            float noise = ((float)(rand() % 1000) / 500.0f - 1.0f);
            float punch = std::sin(2.0f * 3.14159f * (180.0f * std::exp(-t * 30.0f) + 45.0f) * t);
            float sample = (punch * 0.65f + noise * 0.55f) * env;
            sample = std::tanh(sample * 1.8f);

            int16_t s16 = (int16_t)(sample * 28000.0f);
            ak47Wav.push_back((uint8_t)(s16 & 0xFF));
            ak47Wav.push_back((uint8_t)((s16 >> 8) & 0xFF));
        }
    }

    void generateMP40Wav() {
        int sampleRate = 22050;
        float duration = 0.14f;
        int numSamples = (int)(sampleRate * duration);
        mp40Wav = createWavHeader(numSamples, sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            float t = (float)i / sampleRate;
            float env = std::exp(-t * 38.0f);
            float noise = ((float)(rand() % 1000) / 500.0f - 1.0f);
            float punch = std::sin(2.0f * 3.14159f * (280.0f * std::exp(-t * 40.0f) + 80.0f) * t);
            float sample = (punch * 0.5f + noise * 0.6f) * env;

            int16_t s16 = (int16_t)(sample * 26000.0f);
            mp40Wav.push_back((uint8_t)(s16 & 0xFF));
            mp40Wav.push_back((uint8_t)((s16 >> 8) & 0xFF));
        }
    }

    void generateShotgunWav() {
        int sampleRate = 22050;
        float duration = 0.38f;
        int numSamples = (int)(sampleRate * duration);
        shotgunWav = createWavHeader(numSamples, sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            float t = (float)i / sampleRate;
            float env = std::exp(-t * 14.0f);
            float noise = ((float)(rand() % 1000) / 500.0f - 1.0f);
            float sub = std::sin(2.0f * 3.14159f * (90.0f * std::exp(-t * 18.0f) + 30.0f) * t);
            float sample = (sub * 0.8f + noise * 0.7f) * env;
            sample = std::tanh(sample * 2.2f);

            int16_t s16 = (int16_t)(sample * 30000.0f);
            shotgunWav.push_back((uint8_t)(s16 & 0xFF));
            shotgunWav.push_back((uint8_t)((s16 >> 8) & 0xFF));
        }
    }

    void generateAWMWav() {
        int sampleRate = 22050;
        float duration = 0.55f;
        int numSamples = (int)(sampleRate * duration);
        awmWav = createWavHeader(numSamples, sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            float t = (float)i / sampleRate;
            float env = std::exp(-t * 8.0f);
            float noise = ((float)(rand() % 1000) / 500.0f - 1.0f);
            float crack = std::sin(2.0f * 3.14159f * (450.0f * std::exp(-t * 25.0f) + 60.0f) * t);
            float sample = (crack * 0.7f + noise * 0.8f) * env;
            sample = std::tanh(sample * 2.5f);

            int16_t s16 = (int16_t)(sample * 31000.0f);
            awmWav.push_back((uint8_t)(s16 & 0xFF));
            awmWav.push_back((uint8_t)((s16 >> 8) & 0xFF));
        }
    }

    void generateGlooWallWav() {
        int sampleRate = 22050;
        float duration = 0.45f;
        int numSamples = (int)(sampleRate * duration);
        glooWallWav = createWavHeader(numSamples, sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            float t = (float)i / sampleRate;
            float env = std::sin(t / duration * 3.14159f);
            float ice = std::sin(2.0f * 3.14159f * (1200.0f - t * 800.0f) * t) * 0.4f +
                        std::sin(2.0f * 3.14159f * (2400.0f + t * 400.0f) * t) * 0.3f;
            float thud = std::sin(2.0f * 3.14159f * 120.0f * t) * std::exp(-t * 12.0f) * 0.6f;
            float sample = (ice + thud) * env;

            int16_t s16 = (int16_t)(sample * 26000.0f);
            glooWallWav.push_back((uint8_t)(s16 & 0xFF));
            glooWallWav.push_back((uint8_t)((s16 >> 8) & 0xFF));
        }
    }

    void generateHitmarkerWav() {
        int sampleRate = 22050;
        float duration = 0.08f;
        int numSamples = (int)(sampleRate * duration);
        hitmarkerWav = createWavHeader(numSamples, sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            float t = (float)i / sampleRate;
            float env = std::exp(-t * 45.0f);
            float sample = std::sin(2.0f * 3.14159f * 1800.0f * t) * env;

            int16_t s16 = (int16_t)(sample * 22000.0f);
            hitmarkerWav.push_back((uint8_t)(s16 & 0xFF));
            hitmarkerWav.push_back((uint8_t)((s16 >> 8) & 0xFF));
        }
    }

    void generateHeadshotWav() {
        int sampleRate = 22050;
        float duration = 0.25f;
        int numSamples = (int)(sampleRate * duration);
        headshotWav = createWavHeader(numSamples, sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            float t = (float)i / sampleRate;
            float env = std::exp(-t * 16.0f);
            float chime1 = std::sin(2.0f * 3.14159f * 2200.0f * t);
            float chime2 = std::sin(2.0f * 3.14159f * 3300.0f * t);
            float sample = (chime1 + chime2) * 0.5f * env;

            int16_t s16 = (int16_t)(sample * 26000.0f);
            headshotWav.push_back((uint8_t)(s16 & 0xFF));
            headshotWav.push_back((uint8_t)((s16 >> 8) & 0xFF));
        }
    }

    void generateBooyahWav() {
        int sampleRate = 22050;
        float duration = 1.8f;
        int numSamples = (int)(sampleRate * duration);
        booyahWav = createWavHeader(numSamples, sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            float t = (float)i / sampleRate;
            float env = std::min(t * 4.0f, 1.0f) * std::exp(-t * 0.7f);

            float chord = std::sin(2.0f * 3.14159f * 523.25f * t) * 0.25f +
                          std::sin(2.0f * 3.14159f * 659.25f * t) * 0.25f +
                          std::sin(2.0f * 3.14159f * 783.99f * t) * 0.25f +
                          std::sin(2.0f * 3.14159f * 1046.50f * t) * 0.35f;

            float crowd = ((float)(rand() % 1000) / 500.0f - 1.0f) * 0.25f;
            float sample = (chord * 0.75f + crowd * 0.25f) * env;

            int16_t s16 = (int16_t)(sample * 26000.0f);
            booyahWav.push_back((uint8_t)(s16 & 0xFF));
            booyahWav.push_back((uint8_t)((s16 >> 8) & 0xFF));
        }
    }

    void generateReloadWav() {
        int sampleRate = 22050;
        float duration = 0.35f;
        int numSamples = (int)(sampleRate * duration);
        reloadWav = createWavHeader(numSamples, sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            float t = (float)i / sampleRate;
            float env = (t < 0.15f) ? std::exp(-t * 25.0f) : std::exp(-(t - 0.15f) * 25.0f);
            float sample = std::sin(2.0f * 3.14159f * 800.0f * t) * env * 0.6f;

            int16_t s16 = (int16_t)(sample * 20000.0f);
            reloadWav.push_back((uint8_t)(s16 & 0xFF));
            reloadWav.push_back((uint8_t)((s16 >> 8) & 0xFF));
        }
    }

    void generateAlarmWav() {
        int sampleRate = 22050;
        float duration = 0.5f;
        int numSamples = (int)(sampleRate * duration);
        alarmWav = createWavHeader(numSamples, sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            float t = (float)i / sampleRate;
            float freq = 600.0f + 250.0f * std::sin(2.0f * 3.14159f * 8.0f * t);
            float sample = std::sin(2.0f * 3.14159f * freq * t) * 0.45f;

            int16_t s16 = (int16_t)(sample * 18000.0f);
            alarmWav.push_back((uint8_t)(s16 & 0xFF));
            alarmWav.push_back((uint8_t)((s16 >> 8) & 0xFF));
        }
    }

    void generateParachuteWav() {
        int sampleRate = 22050;
        float duration = 0.65f;
        int numSamples = (int)(sampleRate * duration);
        parachuteWav = createWavHeader(numSamples, sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            float t = (float)i / sampleRate;
            float env = std::sin(t / duration * 3.14159f);
            float noise = ((float)(rand() % 1000) / 500.0f - 1.0f);
            float flap = std::sin(2.0f * 3.14159f * (80.0f + 40.0f * std::sin(t * 15.0f)) * t);
            float sample = (noise * 0.55f + flap * 0.45f) * env;

            int16_t s16 = (int16_t)(sample * 24000.0f);
            parachuteWav.push_back((uint8_t)(s16 & 0xFF));
            parachuteWav.push_back((uint8_t)((s16 >> 8) & 0xFF));
        }
    }

    void generatePickupWav() {
        int sampleRate = 22050;
        float duration = 0.16f;
        int numSamples = (int)(sampleRate * duration);
        pickupWav = createWavHeader(numSamples, sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            float t = (float)i / sampleRate;
            float env = std::exp(-t * 22.0f);
            float click1 = std::sin(2.0f * 3.14159f * (1200.0f + t * 1400.0f) * t);
            float sample = click1 * env;

            int16_t s16 = (int16_t)(sample * 25000.0f);
            pickupWav.push_back((uint8_t)(s16 & 0xFF));
            pickupWav.push_back((uint8_t)((s16 >> 8) & 0xFF));
        }
    }

    void generateMedkitWav() {
        int sampleRate = 22050;
        float duration = 0.45f;
        int numSamples = (int)(sampleRate * duration);
        medkitWav = createWavHeader(numSamples, sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            float t = (float)i / sampleRate;
            float env = std::sin(t / duration * 3.14159f);
            float tone = std::sin(2.0f * 3.14159f * (440.0f + t * 440.0f) * t);
            float sample = tone * env * 0.7f;

            int16_t s16 = (int16_t)(sample * 24000.0f);
            medkitWav.push_back((uint8_t)(s16 & 0xFF));
            medkitWav.push_back((uint8_t)((s16 >> 8) & 0xFF));
        }
    }

    void generateLandWav() {
        int sampleRate = 22050;
        float duration = 0.28f;
        int numSamples = (int)(sampleRate * duration);
        landWav = createWavHeader(numSamples, sampleRate);

        for (int i = 0; i < numSamples; ++i) {
            float t = (float)i / sampleRate;
            float env = std::exp(-t * 18.0f);
            float thud = std::sin(2.0f * 3.14159f * 75.0f * t);
            float noise = ((float)(rand() % 1000) / 500.0f - 1.0f) * 0.4f;
            float sample = (thud + noise) * env;

            int16_t s16 = (int16_t)(sample * 27000.0f);
            landWav.push_back((uint8_t)(s16 & 0xFF));
            landWav.push_back((uint8_t)((s16 >> 8) & 0xFF));
        }
    }
};

#else
class SoundManager {
public:
    static SoundManager& instance() { static SoundManager inst; return inst; }
    void playAK47() {}
    void playMP40() {}
    void playShotgun() {}
    void playAWM() {}
    void playGlooWall() {}
    void playHitmarker() {}
    void playHeadshot() {}
    void playBooyah() {}
    void playReload() {}
    void playSafeZoneAlarm() {}
    void playParachuteOpen() {}
    void playPickup() {}
    void playMedkit() {}
    void playLand() {}
};
#endif

#endif // SOUND_HPP
