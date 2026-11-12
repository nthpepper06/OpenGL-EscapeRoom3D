#pragma once

#include "core/Common.hpp"

struct Material {
    // PBR Core Properties
    glm::vec3 albedoColor = glm::vec3(0.8f, 0.8f, 0.8f);
    float metallic = 0.0f;
    float roughness = 0.65f;
    float ao = 1.0f;
    glm::vec3 emissiveColor = glm::vec3(0.0f);
    float emissiveIntensity = 1.0f;

    // UV Fluorescent Blacklight Response
    bool isFluorescent = false;
    glm::vec3 fluorescentColor = glm::vec3(0.2f, 0.95f, 1.0f);
    bool uIsUVReactive = false;

    // PBR Texture Maps (0 = none)
    unsigned int albedoMap = 0;
    unsigned int normalMap = 0;
    unsigned int metallicMap = 0;
    unsigned int roughnessMap = 0;
    unsigned int aoMap = 0;
    unsigned int emissiveMap = 0;

    // Texture toggles
    bool useAlbedoMap = false;
    bool useNormalMap = false;
    bool useMetallicMap = false;
    bool useRoughnessMap = false;
    bool useAoMap = false;
    bool useEmissiveMap = false;

    // Legacy Blinn-Phong backwards compatibility
    glm::vec3 diffuseColor = glm::vec3(0.8f, 0.8f, 0.8f);
    glm::vec3 specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
    float shininess = 32.0f;
    bool useTexture = false;
    unsigned int textureId = 0;

    glm::vec3 getEffectiveAlbedo() const {
        if (albedoColor != glm::vec3(0.8f)) return albedoColor;
        return diffuseColor;
    }

    float getEffectiveMetallic() const {
        if (metallic > 0.0f) return metallic;
        if (specularColor.r > 0.7f && specularColor.g > 0.6f && shininess >= 64.0f) return 0.85f;
        return 0.0f;
    }

    float getEffectiveRoughness() const {
        if (roughness != 0.65f) return roughness;
        if (shininess > 0.0f) {
            float r = std::sqrt(2.0f / (shininess + 2.0f));
            return glm::clamp(r, 0.08f, 1.0f);
        }
        return 0.65f;
    }
};
