#pragma once

#include "renderer/Mesh.hpp"

class PrimitiveGenerator {
public:
    static std::shared_ptr<Mesh> createCube();
    static std::shared_ptr<Mesh> createBeveledCube(float width = 1.0f, float height = 1.0f, float depth = 1.0f, float bevelRadius = 0.05f, int bevelSegments = 2);
    static std::shared_ptr<Mesh> createCylinder(int segments = 32);
    static std::shared_ptr<Mesh> createPlane();
    static std::shared_ptr<Mesh> createSphere(int sectorCount = 24, int stackCount = 16);
};
