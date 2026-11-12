#include "renderer/GpuProfiler.hpp"
#include <iostream>
#include <numeric>

void GraphicsSettings::applyPreset(QualityLevel newLevel) {
    level = newLevel;
    switch (newLevel) {
        case QualityLevel::Low:
            presetName = "Low (Max FPS)";
            shadowResolution = 512;
            pcfSamples = 4;
            ssaoEnabled = false;
            ssaoSamples = 8;
            planarReflectionEnabled = false;
            bloomMips = 3;
            particleCount = 150;
            anisotropy = 1.0f;
            break;
        case QualityLevel::Medium:
            presetName = "Medium (Target UHD >= 60 FPS)";
            shadowResolution = 1024;
            pcfSamples = 8;
            ssaoEnabled = true;
            ssaoSamples = 16;
            planarReflectionEnabled = true;
            bloomMips = 5;
            particleCount = 350;
            anisotropy = 4.0f;
            break;
        case QualityLevel::High:
            presetName = "High (Enhanced PBR)";
            shadowResolution = 2048;
            pcfSamples = 16;
            ssaoEnabled = true;
            ssaoSamples = 32;
            planarReflectionEnabled = true;
            bloomMips = 5;
            particleCount = 500;
            anisotropy = 8.0f;
            break;
        case QualityLevel::Ultra:
            presetName = "Ultra (Cinematic Fidelity)";
            shadowResolution = 2048;
            pcfSamples = 16;
            ssaoEnabled = true;
            ssaoSamples = 64;
            planarReflectionEnabled = true;
            bloomMips = 6;
            particleCount = 750;
            anisotropy = 16.0f;
            break;
    }
}

GpuProfiler::GpuProfiler() {
}

GpuProfiler::~GpuProfiler() {
    shutdown();
}

bool GpuProfiler::init() {
    for (int frame = 0; frame < 2; ++frame) {
        for (size_t p = 0; p < NUM_PASSES; ++p) {
            glGenQueries(1, &queries[frame][p]);
        }
    }
    initialized = true;
    hasValidResults = false;
    currentFrameIndex = 0;
    std::cout << "[GpuProfiler] Hardware GPU Timer Queries initialized (GL_TIME_ELAPSED).\n";
    return true;
}

void GpuProfiler::shutdown() {
    if (!initialized) return;
    for (int frame = 0; frame < 2; ++frame) {
        for (size_t p = 0; p < NUM_PASSES; ++p) {
            if (queries[frame][p] != 0) {
                glDeleteQueries(1, &queries[frame][p]);
                queries[frame][p] = 0;
            }
        }
    }
    initialized = false;
}

void GpuProfiler::beginFrame() {
    if (!initialized) return;

    if (hasValidResults) {
        int readFrame = 1 - currentFrameIndex;
        for (size_t p = 0; p < NUM_PASSES; ++p) {
            if (queries[readFrame][p] != 0) {
                GLint available = 0;
                glGetQueryObjectiv(queries[readFrame][p], GL_QUERY_RESULT_AVAILABLE, &available);
                if (available) {
                    GLuint64 timeElapsedNs = 0;
                    glGetQueryObjectui64v(queries[readFrame][p], GL_QUERY_RESULT, &timeElapsedNs);
                    passTimesMs[p] = static_cast<float>(timeElapsedNs) / 1000000.0f;
                }
            }
        }
    }
}

void GpuProfiler::endFrame() {
    if (!initialized) return;
    currentFrameIndex = 1 - currentFrameIndex;
    hasValidResults = true;
}

void GpuProfiler::beginPass(GpuPass pass) {
    if (!initialized) return;
    size_t idx = static_cast<size_t>(pass);
    if (idx < NUM_PASSES && queries[currentFrameIndex][idx] != 0) {
        glBeginQuery(GL_TIME_ELAPSED, queries[currentFrameIndex][idx]);
        passActive[idx] = true;
    }
}

void GpuProfiler::endPass(GpuPass pass) {
    if (!initialized) return;
    size_t idx = static_cast<size_t>(pass);
    if (idx < NUM_PASSES && passActive[idx]) {
        glEndQuery(GL_TIME_ELAPSED);
        passActive[idx] = false;
    }
}

float GpuProfiler::getPassTimeMs(GpuPass pass) const {
    size_t idx = static_cast<size_t>(pass);
    if (idx < NUM_PASSES) {
        return passTimesMs[idx];
    }
    return 0.0f;
}

float GpuProfiler::getTotalGpuTimeMs() const {
    float total = 0.0f;
    for (size_t i = 0; i < NUM_PASSES; ++i) {
        total += passTimesMs[i];
    }
    return total;
}

const char* GpuProfiler::getPassName(GpuPass pass) {
    switch (pass) {
        case GpuPass::SpotShadow:       return "Pass 0A: Spot Shadow Map";
        case GpuPass::PointShadow:      return "Pass 0B: Point Cubemap Depth";
        case GpuPass::SSAO:             return "Pass 0C: SSAO View-Space AO";
        case GpuPass::PlanarReflection: return "Pass 0D: Planar Mirror Reflection";
        case GpuPass::PbrScene:         return "Pass 1:  HDR PBR Scene Shading";
        case GpuPass::PostProcess:      return "Pass 2:  Post-FX & Bloom Upsample";
        default:                        return "Unknown Pass";
    }
}
