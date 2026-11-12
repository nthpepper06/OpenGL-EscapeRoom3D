#pragma once

#include "core/Common.hpp"
#include "scene/SceneObject.hpp"
#include "renderer/Mesh.hpp"
#include <vector>

class MeshBatcher {
public:
    // Combines multiple static SceneObjects into a single SceneObject with a unified Mesh
    static std::shared_ptr<SceneObject> batchObjects(
        const std::string& batchId,
        const std::string& batchName,
        const std::vector<std::shared_ptr<SceneObject>>& objects,
        const Material& sharedMaterial
    );
};
