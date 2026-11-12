#pragma once

#include "core/Common.hpp"

class Shader {
public:
    unsigned int ID = 0;

    Shader() = default;
    ~Shader();

    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    bool loadFromStrings(const std::string& vertexCode, const std::string& fragmentCode);

    void use() const;

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    bool checkCompileErrors(unsigned int shader, const std::string& type);
};
