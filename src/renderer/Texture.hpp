#pragma once

#include "core/Common.hpp"

struct PbrTextureSet {
    unsigned int albedo = 0;
    unsigned int normal = 0;
    unsigned int metallic = 0;
    unsigned int roughness = 0;
    unsigned int ao = 0;
    unsigned int emissive = 0;
};

class Texture {
public:
    static unsigned int loadFromFile(const std::string& path, bool isSRGB = false);
    static void applyAnisotropicFiltering();

    // Default safe fallbacks
    static unsigned int getWhiteTexture();
    static unsigned int getFlatNormalTexture();
    static unsigned int getDefaultRoughnessTexture();
    static unsigned int getDefaultMetallicTexture();
    static unsigned int getDefaultAoTexture();
    static unsigned int getBlackTexture();

    // Procedural PBR Texture Sets
    static PbrTextureSet createStoneBrickPbr(int width = 512, int height = 512);
    static PbrTextureSet createWoodPbr(int width = 512, int height = 512);
    static PbrTextureSet createMetalPbr(int width = 512, int height = 512);
    static PbrTextureSet createGoldPbr(int width = 512, int height = 512);
    static PbrTextureSet createAsylumTilePbr(int width = 512, int height = 512);
    static PbrTextureSet createRitualCirclePbr(int width = 512, int height = 512);
    static PbrTextureSet createMirrorPbr(int width = 512, int height = 512);

    // Existing Legacy helpers for backward compatibility
    static unsigned int createSolidColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
    static unsigned int createCheckerboard(int width = 256, int height = 256, int checkSize = 32,
                                           glm::u8vec3 color1 = {220, 220, 220},
                                           glm::u8vec3 color2 = {140, 140, 140});
    static unsigned int createWoodPattern(int width = 256, int height = 256);
    static unsigned int createMetalPattern(int width = 256, int height = 256);
    static unsigned int createStoneBrickPattern(int width = 256, int height = 256);
    static unsigned int createRitualCirclePattern(int width = 256, int height = 256);
    static unsigned int createAsylumTilePattern(int width = 256, int height = 256);
    static unsigned int createMirrorPattern(int width = 256, int height = 256);
    static unsigned int createNoteTexture(const std::string& title, const std::string& pin, int width = 256, int height = 256);
    static unsigned int createNoteTexture(int width = 256, int height = 256);
};
