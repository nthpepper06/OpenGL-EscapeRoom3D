#pragma once

#include "core/Common.hpp"
#include "scene/SceneObject.hpp"
#include "renderer/Light.hpp"

class SceneSerializer {
public:
    static bool saveToFile(const std::string& filepath,
                           const std::shared_ptr<SceneObject>& root,
                           const PointLight& pointLight,
                           const SpotLight& spotLight,
                           const AmbientLight& ambientLight);

    static bool loadFromFile(const std::string& filepath,
                             std::shared_ptr<SceneObject>& root,
                             PointLight& pointLight,
                             SpotLight& spotLight,
                             AmbientLight& ambientLight);
};
