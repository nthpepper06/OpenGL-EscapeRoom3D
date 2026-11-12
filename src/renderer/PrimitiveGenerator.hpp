#pragma once

#include "renderer/Mesh.hpp"

class PrimitiveGenerator {
public:
    static std::shared_ptr<Mesh> createCube();
    static std::shared_ptr<Mesh> createCylinder(int segments = 32);
    static std::shared_ptr<Mesh> createPlane();
    static std::shared_ptr<Mesh> createSphere(int sectorCount = 24, int stackCount = 16);
};
