#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "renderer/Texture.hpp"
#include <algorithm>
#include <cmath>

#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

void Texture::applyAnisotropicFiltering() {
    static float maxAniso = 0.0f;
    static bool checked = false;
    if (!checked) {
        checked = true;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        if (glGetError() != GL_NO_ERROR) {
            maxAniso = 0.0f;
        }
    }
    if (maxAniso > 1.0f) {
        float val = std::min(maxAniso, 16.0f);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, val);
    }
}

unsigned int Texture::loadFromFile(const std::string& path, bool isSRGB) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RGB;
        GLenum internalFormat = GL_RGB8;
        if (nrComponents == 1) {
            format = GL_RED;
            internalFormat = GL_R8;
        } else if (nrComponents == 3) {
            format = GL_RGB;
            internalFormat = isSRGB ? GL_SRGB8 : GL_RGB8;
        } else if (nrComponents == 4) {
            format = GL_RGBA;
            internalFormat = isSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        applyAnisotropicFiltering();

        stbi_image_free(data);
        return textureID;
    } else {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
        return 0;
    }
}

unsigned int Texture::createSolidColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    unsigned char data[4] = { r, g, b, a };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return textureID;
}

// ----------------------------------------------------------------------------
// Safe Default 1x1 Fallbacks (Ensures no black screens or missing textures)
// ----------------------------------------------------------------------------
unsigned int Texture::getWhiteTexture() {
    static unsigned int tex = 0;
    if (tex == 0) tex = createSolidColor(255, 255, 255, 255);
    return tex;
}

unsigned int Texture::getFlatNormalTexture() {
    static unsigned int tex = 0;
    if (tex == 0) tex = createSolidColor(128, 128, 255, 255); // Tangent-space Z+ (0, 0, 1)
    return tex;
}

unsigned int Texture::getDefaultRoughnessTexture() {
    static unsigned int tex = 0;
    if (tex == 0) tex = createSolidColor(178, 178, 178, 255); // 0.7 roughness
    return tex;
}

unsigned int Texture::getDefaultMetallicTexture() {
    static unsigned int tex = 0;
    if (tex == 0) tex = createSolidColor(0, 0, 0, 255);       // 0.0 non-metallic
    return tex;
}

unsigned int Texture::getDefaultAoTexture() {
    static unsigned int tex = 0;
    if (tex == 0) tex = createSolidColor(255, 255, 255, 255); // 1.0 unoccluded
    return tex;
}

unsigned int Texture::getBlackTexture() {
    static unsigned int tex = 0;
    if (tex == 0) tex = createSolidColor(0, 0, 0, 255);       // 0.0 emissive
    return tex;
}

// ----------------------------------------------------------------------------
// Internal Upload Helpers for PBR Procedural Generators
// ----------------------------------------------------------------------------
static unsigned int uploadRgb(const std::vector<unsigned char>& data, int width, int height, bool isSRGB = false, bool clampEdge = false) {
    unsigned int texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    GLenum internalFormat = isSRGB ? GL_SRGB8 : GL_RGB8;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clampEdge ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clampEdge ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    Texture::applyAnisotropicFiltering();
    return texID;
}

static unsigned int uploadSingleChannel(const std::vector<unsigned char>& data, int width, int height, bool clampEdge = false) {
    unsigned int texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clampEdge ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clampEdge ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    Texture::applyAnisotropicFiltering();
    return texID;
}

static std::vector<unsigned char> generateNormalFromHeight(const std::vector<float>& heightMap, int width, int height, float strength = 2.5f) {
    std::vector<unsigned char> normalData(width * height * 3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int xPrev = (x - 1 + width) % width;
            int xNext = (x + 1) % width;
            int yPrev = (y - 1 + height) % height;
            int yNext = (y + 1) % height;

            float hL = heightMap[y * width + xPrev];
            float hR = heightMap[y * width + xNext];
            float hD = heightMap[yPrev * width + x];
            float hU = heightMap[yNext * width + x];

            float dx = (hR - hL) * strength;
            float dy = (hU - hD) * strength;

            glm::vec3 n = glm::normalize(glm::vec3(-dx, -dy, 1.0f));

            int idx = (y * width + x) * 3;
            normalData[idx + 0] = static_cast<unsigned char>((n.x * 0.5f + 0.5f) * 255.0f);
            normalData[idx + 1] = static_cast<unsigned char>((n.y * 0.5f + 0.5f) * 255.0f);
            normalData[idx + 2] = static_cast<unsigned char>((n.z * 0.5f + 0.5f) * 255.0f);
        }
    }
    return normalData;
}

// ----------------------------------------------------------------------------
// Procedural PBR Texture Sets
// ----------------------------------------------------------------------------
PbrTextureSet Texture::createStoneBrickPbr(int width, int height) {
    PbrTextureSet set;
    std::vector<unsigned char> albedo(width * height * 3);
    std::vector<float> heightMap(width * height);
    std::vector<unsigned char> roughness(width * height);
    std::vector<unsigned char> metallic(width * height, 0);
    std::vector<unsigned char> ao(width * height);

    int brickH = 32;
    int brickW = 64;

    for (int y = 0; y < height; ++y) {
        int row = y / brickH;
        int shift = (row % 2 == 1) ? (brickW / 2) : 0;
        int yInBrick = y % brickH;
        bool isHorizMortar = (yInBrick <= 2);

        for (int x = 0; x < width; ++x) {
            int shiftedX = (x + shift) % width;
            int xInBrick = shiftedX % brickW;
            bool isVertMortar = (xInBrick <= 2);

            int idx3 = (y * width + x) * 3;
            int idx1 = y * width + x;

            if (isHorizMortar || isVertMortar) {
                // Damp dark crypt mortar
                albedo[idx3 + 0] = 38;
                albedo[idx3 + 1] = 38;
                albedo[idx3 + 2] = 42;
                heightMap[idx1] = 0.05f;
                roughness[idx1] = 230; // 0.90
                ao[idx1] = 70;         // 0.27
            } else {
                int noise = ((x * 37 + y * 59 + row * 101) % 43);
                int baseVal = 70 + noise;
                albedo[idx3 + 0] = static_cast<unsigned char>(baseVal);
                albedo[idx3 + 1] = static_cast<unsigned char>(baseVal - 3);
                albedo[idx3 + 2] = static_cast<unsigned char>(baseVal + 2);

                // Slight bevel curvature towards mortar
                float edgeDistX = static_cast<float>(std::min(xInBrick - 2, brickW - xInBrick));
                float edgeDistY = static_cast<float>(std::min(yInBrick - 2, brickH - yInBrick));
                float edgeFactor = std::min(1.0f, std::min(edgeDistX, edgeDistY) / 4.0f);

                heightMap[idx1] = 0.5f + 0.35f * edgeFactor + (noise / 200.0f);
                roughness[idx1] = static_cast<unsigned char>(160 + (noise % 40)); // 0.63 - 0.78
                ao[idx1] = static_cast<unsigned char>(160 + 95 * edgeFactor);
            }
        }
    }

    auto normalData = generateNormalFromHeight(heightMap, width, height, 3.0f);

    set.albedo = uploadRgb(albedo, width, height, true);
    set.normal = uploadRgb(normalData, width, height, false);
    set.roughness = uploadSingleChannel(roughness, width, height);
    set.metallic = uploadSingleChannel(metallic, width, height);
    set.ao = uploadSingleChannel(ao, width, height);
    set.emissive = getBlackTexture();

    return set;
}

PbrTextureSet Texture::createWoodPbr(int width, int height) {
    PbrTextureSet set;
    std::vector<unsigned char> albedo(width * height * 3);
    std::vector<float> heightMap(width * height);
    std::vector<unsigned char> roughness(width * height);
    std::vector<unsigned char> metallic(width * height, 0);
    std::vector<unsigned char> ao(width * height, 240);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float grain = std::sin((x * 0.15f) + std::sin(y * 0.05f) * 4.0f) * 0.5f + 0.5f;
            float fineGrain = std::sin(x * 0.6f + y * 0.02f) * 0.5f + 0.5f;

            unsigned char r = static_cast<unsigned char>(135 + grain * 55 + fineGrain * 10);
            unsigned char g = static_cast<unsigned char>(80 + grain * 40 + fineGrain * 8);
            unsigned char b = static_cast<unsigned char>(40 + grain * 25 + fineGrain * 5);

            int idx3 = (y * width + x) * 3;
            int idx1 = y * width + x;

            albedo[idx3 + 0] = r;
            albedo[idx3 + 1] = g;
            albedo[idx3 + 2] = b;

            heightMap[idx1] = grain * 0.5f + fineGrain * 0.2f;
            roughness[idx1] = static_cast<unsigned char>(70 + grain * 50); // Polished mahogany 0.27 - 0.47
            ao[idx1] = static_cast<unsigned char>(220 + fineGrain * 35);
        }
    }

    auto normalData = generateNormalFromHeight(heightMap, width, height, 1.8f);

    set.albedo = uploadRgb(albedo, width, height, true);
    set.normal = uploadRgb(normalData, width, height, false);
    set.roughness = uploadSingleChannel(roughness, width, height);
    set.metallic = uploadSingleChannel(metallic, width, height);
    set.ao = uploadSingleChannel(ao, width, height);
    set.emissive = getBlackTexture();

    return set;
}

PbrTextureSet Texture::createMetalPbr(int width, int height) {
    PbrTextureSet set;
    std::vector<unsigned char> albedo(width * height * 3);
    std::vector<float> heightMap(width * height);
    std::vector<unsigned char> roughness(width * height);
    std::vector<unsigned char> metallic(width * height, 255); // Full metal
    std::vector<unsigned char> ao(width * height, 255);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float brushNoise = ((x * 19 + y * 7) % 31) / 31.0f;
            float microScratch = ((y * 47) % 23) / 23.0f;

            unsigned char v = static_cast<unsigned char>(180 + brushNoise * 35 + microScratch * 10);
            int idx3 = (y * width + x) * 3;
            int idx1 = y * width + x;

            albedo[idx3 + 0] = v;
            albedo[idx3 + 1] = v;
            albedo[idx3 + 2] = v + 3;

            heightMap[idx1] = microScratch * 0.3f + brushNoise * 0.2f;
            roughness[idx1] = static_cast<unsigned char>(75 + brushNoise * 20); // 0.29 - 0.37 brushed
        }
    }

    auto normalData = generateNormalFromHeight(heightMap, width, height, 1.2f);

    set.albedo = uploadRgb(albedo, width, height, true);
    set.normal = uploadRgb(normalData, width, height, false);
    set.roughness = uploadSingleChannel(roughness, width, height);
    set.metallic = uploadSingleChannel(metallic, width, height);
    set.ao = uploadSingleChannel(ao, width, height);
    set.emissive = getBlackTexture();

    return set;
}

PbrTextureSet Texture::createGoldPbr(int width, int height) {
    PbrTextureSet set;
    std::vector<unsigned char> albedo(width * height * 3);
    std::vector<float> heightMap(width * height);
    std::vector<unsigned char> roughness(width * height);
    std::vector<unsigned char> metallic(width * height, 255); // 1.0 pure metal
    std::vector<unsigned char> ao(width * height, 255);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float noise = ((x * 13 + y * 29) % 19) / 19.0f;
            int idx3 = (y * width + x) * 3;
            int idx1 = y * width + x;

            // Radiant Ornate Gold: RGB (255, 215, 80)
            albedo[idx3 + 0] = static_cast<unsigned char>(245 + noise * 10);
            albedo[idx3 + 1] = static_cast<unsigned char>(205 + noise * 15);
            albedo[idx3 + 2] = static_cast<unsigned char>(75 + noise * 15);

            heightMap[idx1] = noise * 0.15f;
            roughness[idx1] = static_cast<unsigned char>(40 + noise * 15); // 0.16 - 0.22 smooth lustrous
        }
    }

    auto normalData = generateNormalFromHeight(heightMap, width, height, 1.0f);

    set.albedo = uploadRgb(albedo, width, height, true);
    set.normal = uploadRgb(normalData, width, height, false);
    set.roughness = uploadSingleChannel(roughness, width, height);
    set.metallic = uploadSingleChannel(metallic, width, height);
    set.ao = uploadSingleChannel(ao, width, height);
    set.emissive = getBlackTexture();

    return set;
}

PbrTextureSet Texture::createAsylumTilePbr(int width, int height) {
    PbrTextureSet set;
    std::vector<unsigned char> albedo(width * height * 3);
    std::vector<float> heightMap(width * height);
    std::vector<unsigned char> roughness(width * height);
    std::vector<unsigned char> metallic(width * height, 0);
    std::vector<unsigned char> ao(width * height);

    int tileSize = 32;

    for (int y = 0; y < height; ++y) {
        int yInTile = y % tileSize;
        bool isHorizGrout = (yInTile <= 1);

        for (int x = 0; x < width; ++x) {
            int xInTile = x % tileSize;
            bool isVertGrout = (xInTile <= 1);

            int idx3 = (y * width + x) * 3;
            int idx1 = y * width + x;

            if (isHorizGrout || isVertGrout) {
                // Moldy dark grout
                albedo[idx3 + 0] = 32;
                albedo[idx3 + 1] = 36;
                albedo[idx3 + 2] = 30;
                heightMap[idx1] = 0.05f;
                roughness[idx1] = 240; // 0.94
                ao[idx1] = 75;         // 0.29
            } else {
                int noise = ((x * 13 + y * 29) % 31);
                bool isStain = ((x * 7 + y * 11) % 89 < 15);

                if (isStain) {
                    albedo[idx3 + 0] = static_cast<unsigned char>(75 + noise);
                    albedo[idx3 + 1] = static_cast<unsigned char>(70 + noise);
                    albedo[idx3 + 2] = static_cast<unsigned char>(55 + noise);
                    roughness[idx1] = 135; // 0.53
                } else {
                    albedo[idx3 + 0] = static_cast<unsigned char>(140 + noise);
                    albedo[idx3 + 1] = static_cast<unsigned char>(165 + noise);
                    albedo[idx3 + 2] = static_cast<unsigned char>(150 + noise);
                    roughness[idx1] = 48;  // 0.19 glazed shiny tile
                }

                float edgeDistX = static_cast<float>(std::min(xInTile - 1, tileSize - xInTile));
                float edgeDistY = static_cast<float>(std::min(yInTile - 1, tileSize - yInTile));
                float edgeFactor = std::min(1.0f, std::min(edgeDistX, edgeDistY) / 3.0f);

                heightMap[idx1] = 0.6f + 0.35f * edgeFactor;
                ao[idx1] = static_cast<unsigned char>(180 + 75 * edgeFactor);
            }
        }
    }

    auto normalData = generateNormalFromHeight(heightMap, width, height, 3.2f);

    set.albedo = uploadRgb(albedo, width, height, true);
    set.normal = uploadRgb(normalData, width, height, false);
    set.roughness = uploadSingleChannel(roughness, width, height);
    set.metallic = uploadSingleChannel(metallic, width, height);
    set.ao = uploadSingleChannel(ao, width, height);
    set.emissive = getBlackTexture();

    return set;
}

PbrTextureSet Texture::createRitualCirclePbr(int width, int height) {
    PbrTextureSet set;
    std::vector<unsigned char> albedo(width * height * 3);
    std::vector<float> heightMap(width * height);
    std::vector<unsigned char> roughness(width * height);
    std::vector<unsigned char> metallic(width * height, 0);
    std::vector<unsigned char> ao(width * height, 220);
    std::vector<unsigned char> emissive(width * height * 3, 0);

    float cx = width * 0.5f;
    float cy = height * 0.5f;
    float maxR = width * 0.45f;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float dx = x - cx;
            float dy = y - cy;
            float r = std::sqrt(dx * dx + dy * dy);
            float angle = std::atan2(dy, dx);

            int idx3 = (y * width + x) * 3;
            int idx1 = y * width + x;

            int bgNoise = ((x * 19 + y * 23) % 17);
            albedo[idx3 + 0] = static_cast<unsigned char>(32 + bgNoise);
            albedo[idx3 + 1] = static_cast<unsigned char>(26 + bgNoise / 2);
            albedo[idx3 + 2] = static_cast<unsigned char>(28 + bgNoise / 2);

            heightMap[idx1] = 0.4f + bgNoise * 0.01f;
            roughness[idx1] = 205; // 0.80 matte dark altar stone

            bool isRing1 = std::abs(r - maxR) < 3.5f;
            bool isRing2 = std::abs(r - (maxR * 0.75f)) < 2.5f;
            bool isRing3 = std::abs(r - (maxR * 0.35f)) < 2.0f;

            bool isStar = false;
            for (int k = 0; k < 5; ++k) {
                float theta1 = k * (2.0f * 3.14159265f / 5.0f) - 3.14159265f * 0.5f;
                float theta2 = (k + 2) * (2.0f * 3.14159265f / 5.0f) - 3.14159265f * 0.5f;
                float p1x = cx + maxR * 0.74f * std::cos(theta1);
                float p1y = cy + maxR * 0.74f * std::sin(theta1);
                float p2x = cx + maxR * 0.74f * std::cos(theta2);
                float p2y = cy + maxR * 0.74f * std::sin(theta2);

                float l2 = (p2x - p1x) * (p2x - p1x) + (p2y - p1y) * (p2y - p1y);
                float t = std::max(0.0f, std::min(1.0f, ((x - p1x) * (p2x - p1x) + (y - p1y) * (p2y - p1y)) / l2));
                float projX = p1x + t * (p2x - p1x);
                float projY = p1y + t * (p2y - p1y);
                float distToSegment = std::sqrt((x - projX) * (x - projX) + (y - projY) * (y - projY));
                if (distToSegment < 2.2f) {
                    isStar = true;
                    break;
                }
            }

            bool isRays = (r < maxR && r > maxR * 0.75f) && (std::fmod(std::abs(angle * 10.0f), 3.14159f) < 0.15f);

            if (isRing1 || isRing2 || isRing3 || isStar || isRays) {
                // Coagulated crimson blood
                albedo[idx3 + 0] = 205;
                albedo[idx3 + 1] = 22;
                albedo[idx3 + 2] = 28;

                heightMap[idx1] = 0.65f; // Raised crust
                roughness[idx1] = 70;    // 0.27 wet glistening blood

                // Eldritch smoldering emissive pulse
                emissive[idx3 + 0] = 120;
                emissive[idx3 + 1] = 8;
                emissive[idx3 + 2] = 12;
            }
        }
    }

    auto normalData = generateNormalFromHeight(heightMap, width, height, 2.5f);

    set.albedo = uploadRgb(albedo, width, height, true, true);
    set.normal = uploadRgb(normalData, width, height, false, true);
    set.roughness = uploadSingleChannel(roughness, width, height, true);
    set.metallic = uploadSingleChannel(metallic, width, height, true);
    set.ao = uploadSingleChannel(ao, width, height, true);
    set.emissive = uploadRgb(emissive, width, height, true, true);

    return set;
}

PbrTextureSet Texture::createMirrorPbr(int width, int height) {
    PbrTextureSet set;
    std::vector<unsigned char> albedo(width * height * 3);
    std::vector<float> heightMap(width * height);
    std::vector<unsigned char> roughness(width * height);
    std::vector<unsigned char> metallic(width * height);
    std::vector<unsigned char> ao(width * height);

    int border = 18;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx3 = (y * width + x) * 3;
            int idx1 = y * width + x;

            if (x < border || x >= width - border || y < border || y >= height - border) {
                // Antique filigree frame
                int fNoise = ((x * 11 + y * 19) % 23);
                albedo[idx3 + 0] = static_cast<unsigned char>(135 + fNoise);
                albedo[idx3 + 1] = static_cast<unsigned char>(120 + fNoise);
                albedo[idx3 + 2] = static_cast<unsigned char>(95 + fNoise);

                heightMap[idx1] = 0.7f + (fNoise / 50.0f);
                roughness[idx1] = 115; // 0.45
                metallic[idx1] = 215;  // 0.84 tarnished metal
                ao[idx1] = 110;        // 0.43 occluded filigree crevices
            } else {
                // Glass mirror center
                float diag = (float)(x + y) / (float)(width + height);
                int grad = static_cast<int>(190 + 35.0f * diag);
                albedo[idx3 + 0] = static_cast<unsigned char>(grad - 10);
                albedo[idx3 + 1] = static_cast<unsigned char>(grad + 5);
                albedo[idx3 + 2] = static_cast<unsigned char>(grad + 20);

                heightMap[idx1] = 0.2f;
                roughness[idx1] = 10;  // 0.04 (ultra-smooth mirror reflection!)
                metallic[idx1] = 250;  // 0.98 high metal reflection
                ao[idx1] = 255;
            }
        }
    }

    auto normalData = generateNormalFromHeight(heightMap, width, height, 1.0f);

    set.albedo = uploadRgb(albedo, width, height, true, true);
    set.normal = uploadRgb(normalData, width, height, false, true);
    set.roughness = uploadSingleChannel(roughness, width, height, true);
    set.metallic = uploadSingleChannel(metallic, width, height, true);
    set.ao = uploadSingleChannel(ao, width, height, true);
    set.emissive = getBlackTexture();

    return set;
}

// ----------------------------------------------------------------------------
// Legacy Procedural Patterns (Maintained for Backward Compatibility)
// ----------------------------------------------------------------------------
unsigned int Texture::createCheckerboard(int width, int height, int checkSize,
                                         glm::u8vec3 color1, glm::u8vec3 color2) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    std::vector<unsigned char> data(width * height * 3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int checkX = (x / checkSize) % 2;
            int checkY = (y / checkSize) % 2;
            bool isColor1 = (checkX ^ checkY) == 0;

            glm::u8vec3 c = isColor1 ? color1 : color2;
            int idx = (y * width + x) * 3;
            data[idx + 0] = c.r;
            data[idx + 1] = c.g;
            data[idx + 2] = c.b;
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    applyAnisotropicFiltering();

    return textureID;
}

unsigned int Texture::createWoodPattern(int width, int height) {
    PbrTextureSet pbr = createWoodPbr(width, height);
    return pbr.albedo;
}

unsigned int Texture::createMetalPattern(int width, int height) {
    PbrTextureSet pbr = createMetalPbr(width, height);
    return pbr.albedo;
}

unsigned int Texture::createStoneBrickPattern(int width, int height) {
    PbrTextureSet pbr = createStoneBrickPbr(width, height);
    return pbr.albedo;
}

unsigned int Texture::createRitualCirclePattern(int width, int height) {
    PbrTextureSet pbr = createRitualCirclePbr(width, height);
    return pbr.albedo;
}

unsigned int Texture::createAsylumTilePattern(int width, int height) {
    PbrTextureSet pbr = createAsylumTilePbr(width, height);
    return pbr.albedo;
}

unsigned int Texture::createMirrorPattern(int width, int height) {
    PbrTextureSet pbr = createMirrorPbr(width, height);
    return pbr.albedo;
}

unsigned int Texture::createNoteTexture(const std::string& title, const std::string& pin, int width, int height) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    std::vector<unsigned char> data(width * height * 3);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 3;
            if (x < 10 || x > width - 11 || y < 10 || y > height - 11) {
                data[idx + 0] = 175;
                data[idx + 1] = 145;
                data[idx + 2] = 105;
            } else if (y % 28 == 0) {
                data[idx + 0] = 210;
                data[idx + 1] = 220;
                data[idx + 2] = 240;
            } else {
                int noise = ((x * 17 + y * 31) % 11);
                data[idx + 0] = static_cast<unsigned char>(248 + noise / 3);
                data[idx + 1] = static_cast<unsigned char>(240 + noise / 3);
                data[idx + 2] = static_cast<unsigned char>(220 + noise / 3);
            }
        }
    }

    auto drawPixel = [&](int px, int py, int sz, unsigned char r, unsigned char g, unsigned char b) {
        for (int dy = 0; dy < sz; ++dy) {
            for (int dx = 0; dx < sz; ++dx) {
                int x = px + dx;
                int y = py + dy;
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    int idx = (y * width + x) * 3;
                    data[idx + 0] = r;
                    data[idx + 1] = g;
                    data[idx + 2] = b;
                }
            }
        }
    };

    auto getGlyph = [](char c) -> std::vector<std::string> {
        if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
        if (c == '0') return { "01110", "10001", "10011", "10101", "11001", "10001", "01110" };
        if (c == '1') return { "00100", "01100", "00100", "00100", "00100", "00100", "01110" };
        if (c == '2') return { "01110", "10001", "00001", "00110", "01000", "10000", "11111" };
        if (c == '3') return { "11110", "00001", "00001", "01110", "00001", "00001", "11110" };
        if (c == '4') return { "10001", "10001", "10001", "11111", "00001", "00001", "00001" };
        if (c == '5') return { "11111", "10000", "11110", "00001", "00001", "10001", "01110" };
        if (c == '6') return { "01110", "10000", "11110", "10001", "10001", "10001", "01110" };
        if (c == '7') return { "11111", "00001", "00010", "00100", "00100", "00100", "00100" };
        if (c == '8') return { "01110", "10001", "10001", "01110", "10001", "10001", "01110" };
        if (c == '9') return { "01110", "10001", "10001", "01111", "00001", "00001", "01110" };
        if (c == 'P') return { "11110", "10001", "11110", "10000", "10000", "10000", "10000" };
        if (c == 'I') return { "11111", "00100", "00100", "00100", "00100", "00100", "11111" };
        if (c == 'N') return { "10001", "11001", "10101", "10011", "10001", "10001", "10001" };
        if (c == ':') return { "00000", "00100", "00000", "00000", "00100", "00000", "00000" };
        if (c == '-') return { "00000", "00000", "00000", "11111", "00000", "00000", "00000" };
        if (c == 'S') return { "01111", "10000", "01110", "00001", "00001", "10001", "01110" };
        if (c == 'A') return { "01110", "10001", "10001", "11111", "10001", "10001", "10001" };
        if (c == 'F') return { "11111", "10000", "11110", "10000", "10000", "10000", "10000" };
        if (c == 'E') return { "11111", "10000", "11110", "10000", "10000", "10000", "11111" };
        if (c == 'C') return { "01110", "10001", "10000", "10000", "10000", "10001", "01110" };
        if (c == 'O') return { "01110", "10001", "10001", "10001", "10001", "10001", "01110" };
        if (c == 'D') return { "11110", "10001", "10001", "10001", "10001", "10001", "11110" };
        if (c == 'R') return { "11110", "10001", "11110", "10100", "10010", "10001", "10001" };
        if (c == 'T') return { "11111", "00100", "00100", "00100", "00100", "00100", "00100" };
        if (c == 'U') return { "10001", "10001", "10001", "10001", "10001", "10001", "01110" };
        if (c == 'L') return { "10000", "10000", "10000", "10000", "10000", "10000", "11111" };
        if (c == 'M') return { "10001", "11011", "10101", "10101", "10001", "10001", "10001" };
        return { "00000", "00000", "00000", "00000", "00000", "00000", "00000" };
    };

    auto drawText = [&](const std::string& text, int startX, int startY, int scale, unsigned char r, unsigned char g, unsigned char b) {
        int curX = startX;
        for (char c : text) {
            if (c == ' ') { curX += 4 * scale; continue; }
            auto glyph = getGlyph(c);
            for (int row = 0; row < 7; ++row) {
                for (int col = 0; col < 5; ++col) {
                    if (glyph[row][col] == '1') {
                        int py = startY + (6 - row) * scale;
                        int px = curX + col * scale;
                        drawPixel(px, py, scale, r, g, b);
                    }
                }
            }
            curX += 6 * scale;
        }
    };

    int titleW = static_cast<int>(title.size()) * 6 * 3;
    int startTitleX = std::max(16, (width - titleW) / 2);
    drawText(title, startTitleX, 160, 3, 175, 30, 30);

    int pinW = static_cast<int>(pin.size()) * 6 * 4;
    int startPinX = std::max(20, (width - pinW) / 2);
    drawText(pin, startPinX, 70, 4, 25, 25, 25);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    applyAnisotropicFiltering();

    return textureID;
}

unsigned int Texture::createNoteTexture(int width, int height) {
    return createNoteTexture("SAFE CODE", "PIN: 7492", width, height);
}

unsigned int Texture::createRuneGlyphTexture(int glyphIndex, int width, int height) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    std::vector<unsigned char> data(width * height * 3);

    auto drawSegment = [&](float x1f, float y1f, float x2f, float y2f, float thickness,
                           unsigned char r, unsigned char g, unsigned char b) {
        float x1 = x1f * width;
        float y1 = y1f * height;
        float x2 = x2f * width;
        float y2 = y2f * height;
        float dx = x2 - x1;
        float dy = y2 - y1;
        float lenSq = dx * dx + dy * dy;
        if (lenSq < 0.0001f) return;

        int minX = std::max(0, static_cast<int>(std::min(x1, x2) - thickness - 2));
        int maxX = std::min(width - 1, static_cast<int>(std::max(x1, x2) + thickness + 2));
        int minY = std::max(0, static_cast<int>(std::min(y1, y2) - thickness - 2));
        int maxY = std::min(height - 1, static_cast<int>(std::max(y1, y2) + thickness + 2));

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                float t = std::max(0.0f, std::min(1.0f, ((x - x1) * dx + (y - y1) * dy) / lenSq));
                float px = x1 + t * dx;
                float py = y1 + t * dy;
                float dist = std::sqrt((x - px) * (x - px) + (y - py) * (y - py));

                if (dist <= thickness) {
                    float alpha = 1.0f;
                    if (dist > thickness - 1.5f) {
                        alpha = (thickness - dist) / 1.5f;
                    }
                    int idx = (y * width + x) * 3;
                    data[idx + 0] = static_cast<unsigned char>(data[idx + 0] * (1.0f - alpha) + r * alpha);
                    data[idx + 1] = static_cast<unsigned char>(data[idx + 1] * (1.0f - alpha) + g * alpha);
                    data[idx + 2] = static_cast<unsigned char>(data[idx + 2] * (1.0f - alpha) + b * alpha);
                }
            }
        }
    };

    // 1. Fill base dark ancient crypt stone background
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 3;
            // Stone texture noise with subtle edge vignette
            int noise = ((x * 17 + y * 31) % 19) + ((x * 7 - y * 13) % 11);
            float u = (float)x / width;
            float v = (float)y / height;
            float edgeDist = std::min(std::min(u, 1.0f - u), std::min(v, 1.0f - v)) * 2.0f;
            float vignette = std::min(1.0f, edgeDist * 3.5f);

            unsigned char baseR = static_cast<unsigned char>((22 + noise) * vignette);
            unsigned char baseG = static_cast<unsigned char>((18 + noise / 2) * vignette);
            unsigned char baseB = static_cast<unsigned char>((24 + noise / 2) * vignette);

            data[idx + 0] = baseR;
            data[idx + 1] = baseG;
            data[idx + 2] = baseB;
        }
    }

    // Outer stone tablet etched border
    drawSegment(0.08f, 0.08f, 0.92f, 0.08f, 2.5f, 70, 45, 45);
    drawSegment(0.92f, 0.08f, 0.92f, 0.92f, 2.5f, 70, 45, 45);
    drawSegment(0.92f, 0.92f, 0.08f, 0.92f, 2.5f, 70, 45, 45);
    drawSegment(0.08f, 0.92f, 0.08f, 0.08f, 2.5f, 70, 45, 45);

    // Glowing rune colors: vibrant crimson-amber glyph on dark stone
    unsigned char glowR = 255;
    unsigned char glowG = 135;
    unsigned char glowB = 55;
    float strokeThick = 4.5f;

    if (glyphIndex == 0) {
        // GLYPH 0 (Totem 1, Target: 90 deg / East): RIGHT-FACING ARROW WITH CROSSBAR
        // Horizontal main shaft pointing East (+X)
        drawSegment(0.20f, 0.50f, 0.80f, 0.50f, strokeThick, glowR, glowG, glowB);
        // Arrowhead barbs
        drawSegment(0.80f, 0.50f, 0.55f, 0.72f, strokeThick, glowR, glowG, glowB);
        drawSegment(0.80f, 0.50f, 0.55f, 0.28f, strokeThick, glowR, glowG, glowB);
        // Vertical crossbar
        drawSegment(0.35f, 0.32f, 0.35f, 0.68f, strokeThick, glowR, glowG, glowB);
    } else if (glyphIndex == 1) {
        // GLYPH 1 (Totem 2, Target: 180 deg / South / Up): HORNED TRIDENT / CRESCENT FORK
        // Central vertical shaft
        drawSegment(0.50f, 0.18f, 0.50f, 0.82f, strokeThick, glowR, glowG, glowB);
        // Central spearhead
        drawSegment(0.50f, 0.82f, 0.42f, 0.68f, strokeThick, glowR, glowG, glowB);
        drawSegment(0.50f, 0.82f, 0.58f, 0.68f, strokeThick, glowR, glowG, glowB);
        // Horizontal cross-brace
        drawSegment(0.24f, 0.42f, 0.76f, 0.42f, strokeThick, glowR, glowG, glowB);
        // Left and right upward-flaring horns
        drawSegment(0.24f, 0.42f, 0.24f, 0.76f, strokeThick, glowR, glowG, glowB);
        drawSegment(0.76f, 0.42f, 0.76f, 0.76f, strokeThick, glowR, glowG, glowB);
    } else {
        // GLYPH 2 (Totem 3, Target: 270 deg / West): LEFT-FACING ARROW WITH CROSSBAR
        // Horizontal main shaft pointing West (-X)
        drawSegment(0.80f, 0.50f, 0.20f, 0.50f, strokeThick, glowR, glowG, glowB);
        // Arrowhead barbs
        drawSegment(0.20f, 0.50f, 0.45f, 0.72f, strokeThick, glowR, glowG, glowB);
        drawSegment(0.20f, 0.50f, 0.45f, 0.28f, strokeThick, glowR, glowG, glowB);
        // Vertical crossbar
        drawSegment(0.65f, 0.32f, 0.65f, 0.68f, strokeThick, glowR, glowG, glowB);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    applyAnisotropicFiltering();

    return textureID;
}
