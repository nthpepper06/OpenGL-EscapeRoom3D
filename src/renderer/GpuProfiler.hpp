#pragma once

#include "core/Common.hpp"
#include <glad/glad.h>
#include <string>
#include <vector>
#include <array>

enum class GpuPass : size_t {
    SpotShadow = 0,
    PointShadow,
    SSAO,
    PlanarReflection,
    PbrScene,
    PostProcess,
    Count
};

enum class QualityLevel {
    Low = 0,
    Medium = 1,
    High = 2,
    Ultra = 3
};

struct GraphicsSettings {
    QualityLevel level = QualityLevel::Medium;
    std::string presetName = "Medium (Target UHD)";
    int shadowResolution = 1024;
    int pcfSamples = 8;
    bool ssaoEnabled = true;
    int ssaoSamples = 16;
    bool planarReflectionEnabled = true;
    int bloomMips = 5;
    int particleCount = 350;
    float anisotropy = 4.0f;

    void applyPreset(QualityLevel newLevel);
};

class GpuProfiler {
public:
    GpuProfiler();
    ~GpuProfiler();

    bool init();
    void shutdown();

    void beginFrame();
    void endFrame();

    void beginPass(GpuPass pass);
    void endPass(GpuPass pass);

    float getPassTimeMs(GpuPass pass) const;
    float getTotalGpuTimeMs() const;
    static const char* getPassName(GpuPass pass);

private:
    static constexpr size_t NUM_PASSES = static_cast<size_t>(GpuPass::Count);
    GLuint queries[2][NUM_PASSES] = {{0}};
    bool passActive[NUM_PASSES] = {false};
    float passTimesMs[NUM_PASSES] = {0.0f};
    int currentFrameIndex = 0;
    bool initialized = false;
    bool hasValidResults = false;
};
