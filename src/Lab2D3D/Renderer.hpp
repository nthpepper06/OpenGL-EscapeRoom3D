#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <glad/glad.h>
#include <string>
#include <iostream>
#include "../Math3D.hpp"
#include "Primitives2D.hpp"
#include "Primitives3D.hpp"

namespace LabRenderer {

inline const char* labVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec4 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 VertexColor;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    Normal = mat3(uModel) * aNormal;
    TexCoords = aTexCoords;
    VertexColor = aColor;
    gl_Position = uProjection * uView * worldPos;
}
)";

inline const char* labFragmentShader = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 VertexColor;

out vec4 FragColor;

uniform vec3 uViewPos;
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec4 uColorOverride; // Nếu .a >= 0 thì ghi đè màu đỉnh
uniform int uShadingMode;    // 0: Blinn-Phong, 1: Unlit, 2: Normal Colors, 3: Flat

void main() {
    vec4 baseColor = (uColorOverride.a >= 0.0) ? uColorOverride : VertexColor;

    // Mode 1: Không chiếu sáng (Unlit - dùng cho 2D & Nguồn sáng)
    if (uShadingMode == 1) {
        FragColor = baseColor;
        return;
    }

    // Mode 2: Hiển thị màu theo Vector Pháp Tuyến (Normals)
    if (uShadingMode == 2) {
        vec3 nCol = normalize(Normal) * 0.5 + 0.5;
        FragColor = vec4(nCol, 1.0);
        return;
    }

    // Mode 0 & 3: Chiếu sáng Blinn-Phong Lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(uLightPos - FragPos);
    vec3 viewDir = normalize(uViewPos - FragPos);

    // 1. Ánh sáng môi trường (Ambient)
    float ambientStrength = 0.32;
    vec3 ambient = ambientStrength * uLightColor * baseColor.rgb;

    // 2. Ánh sáng khuếch tán (Diffuse - Lambertian)
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor * baseColor.rgb * 0.85;

    // 3. Ánh sáng phản xạ gương (Specular - Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = 0.45 * spec * uLightColor;

    // 4. Suy giảm ánh sáng theo khoảng cách (Distance Attenuation)
    float dist = length(uLightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.02 * dist + 0.003 * dist * dist);

    vec3 finalRgb = ambient + (diffuse + specular) * attenuation;
    FragColor = vec4(finalRgb, baseColor.a);
}
)";

class ShaderProgram {
public:
    GLuint id = 0;

    bool init(const char* vertSrc, const char* fragSrc) {
        GLuint vert = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert, 1, &vertSrc, nullptr);
        glCompileShader(vert);
        if (!checkShader(vert, "VERTEX")) return false;

        GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag, 1, &fragSrc, nullptr);
        glCompileShader(frag);
        if (!checkShader(frag, "FRAGMENT")) return false;

        id = glCreateProgram();
        glAttachShader(id, vert);
        glAttachShader(id, frag);
        glLinkProgram(id);
        if (!checkProgram(id)) return false;

        glDeleteShader(vert);
        glDeleteShader(frag);
        return true;
    }

    void use() const { glUseProgram(id); }

    void setMat4(const char* name, const Mat4& m) const {
        GLint loc = glGetUniformLocation(id, name);
        if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, m.data());
    }

    void setVec3(const char* name, const Vec3& v) const {
        GLint loc = glGetUniformLocation(id, name);
        if (loc != -1) glUniform3f(loc, v.x, v.y, v.z);
    }

    void setVec4(const char* name, const Vec4& v) const {
        GLint loc = glGetUniformLocation(id, name);
        if (loc != -1) glUniform4f(loc, v.x, v.y, v.z, v.w);
    }

    void setInt(const char* name, int val) const {
        GLint loc = glGetUniformLocation(id, name);
        if (loc != -1) glUniform1i(loc, val);
    }

    void setFloat(const char* name, float val) const {
        GLint loc = glGetUniformLocation(id, name);
        if (loc != -1) glUniform1f(loc, val);
    }

    void destroy() {
        if (id != 0) {
            glDeleteProgram(id);
            id = 0;
        }
    }

private:
    bool checkShader(GLuint s, const std::string& type) {
        GLint ok;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[1024];
            glGetShaderInfoLog(s, 1024, nullptr, log);
            std::cerr << "Shader compile error [" << type << "]: " << log << std::endl;
            return false;
        }
        return true;
    }

    bool checkProgram(GLuint p) {
        GLint ok;
        glGetProgramiv(p, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[1024];
            glGetProgramInfoLog(p, 1024, nullptr, log);
            std::cerr << "Program link error: " << log << std::endl;
            return false;
        }
        return true;
    }
};

class OrbitCamera {
public:
    Vec3 target = Vec3(0, 0, 0);
    float distance = 6.0f;
    float yaw = 45.0f;    // Độ (Yaw)
    float pitch = 25.0f;  // Độ (Pitch)
    float fov = 45.0f;
    float orthoSize = 3.5f;

    Vec3 getEyePosition() const {
        float radYaw = yaw * DEG2RAD;
        float radPitch = pitch * DEG2RAD;
        float x = target.x + distance * std::cos(radPitch) * std::sin(radYaw);
        float y = target.y + distance * std::sin(radPitch);
        float z = target.z + distance * std::cos(radPitch) * std::cos(radYaw);
        return Vec3(x, y, z);
    }

    Mat4 getViewMatrix() const {
        return Mat4::lookAt(getEyePosition(), target, Vec3(0, 1, 0));
    }

    Mat4 getProjectionMatrix(float aspect, bool isPerspective) const {
        if (isPerspective) {
            return Mat4::perspective(fov * DEG2RAD, aspect, 0.1f, 100.0f);
        } else {
            float hw = orthoSize * aspect;
            float hh = orthoSize;
            return Mat4::ortho(-hw, hw, -hh, hh, -50.0f, 50.0f);
        }
    }

    void rotate(float deltaYaw, float deltaPitch) {
        yaw += deltaYaw;
        pitch += deltaPitch;
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }

    void zoom(float deltaDistance) {
        distance += deltaDistance;
        if (distance < 1.0f) distance = 1.0f;
        if (distance > 30.0f) distance = 30.0f;
    }

    void reset(float dist = 6.0f, float y = 45.0f, float p = 25.0f) {
        distance = dist;
        yaw = y;
        pitch = p;
        target = Vec3(0, 0, 0);
    }
};

} // namespace LabRenderer

#endif // RENDERER_HPP
