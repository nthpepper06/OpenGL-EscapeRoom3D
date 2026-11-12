#pragma once

#include "core/Common.hpp"

struct PointLight {
    glm::vec3 position = glm::vec3(0.0f, 3.8f, 0.0f);
    glm::vec3 color = glm::vec3(1.0f, 0.95f, 0.85f); // Warm incandescent
    float intensity = 2.2f;
    float baseIntensity = 2.2f;
    float constant = 1.0f;
    float linear = 0.05f;
    float quadratic = 0.015f;
    bool enabled = true;

    // Horror flickering light simulation
    bool flickerEnabled = true;
    float flickerSpeed = 6.0f;
    float flickerNoiseDepth = 0.35f;
    float flickerTimer = 0.0f;

    void updateFlicker(float deltaTime) {
        if (!enabled || !flickerEnabled) {
            intensity = baseIntensity;
            return;
        }

        flickerTimer += deltaTime * flickerSpeed;

        // Combine multi-frequency sine waves with pseudo-random jitter
        float s1 = std::sin(flickerTimer);
        float s2 = std::sin(flickerTimer * 2.73f);
        float s3 = std::sin(flickerTimer * 6.81f);

        // Deterministic pseudo-random noise generator
        float noise = std::fmod(std::abs(std::sin(flickerTimer * 12.9898f)) * 43758.5453f, 1.0f);

        // Occasional sharp blackout / stutter dip
        float dip = (noise > 0.88f) ? 0.30f : 1.0f;

        float wave = 0.72f + 0.15f * s1 + 0.08f * s2 + 0.05f * s3 + (noise - 0.5f) * flickerNoiseDepth;
        intensity = baseIntensity * wave * dip;
        if (intensity < 0.10f) intensity = 0.10f;
    }
};

struct SpotLight {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 color = glm::vec3(0.95f, 0.98f, 1.0f); // Cool LED flashlight
    float intensity = 2.0f;
    float cutOff = glm::cos(glm::radians(14.0f));
    float outerCutOff = glm::cos(glm::radians(20.0f));
    float constant = 1.0f;
    float linear = 0.07f;
    float quadratic = 0.02f;
    bool enabled = true;
};

struct AmbientLight {
    glm::vec3 color = glm::vec3(0.3f, 0.3f, 0.35f);
    float intensity = 0.45f;
};
