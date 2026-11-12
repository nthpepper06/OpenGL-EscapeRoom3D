#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>
#include <string>
#include <iostream>
#include "Math3D.hpp"

class Shader {
public:
    GLuint id = 0;

    Shader() = default;

    bool compile(const char* vertexSrc, const char* fragmentSrc) {
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
        glCompileShader(vertexShader);
        if (!checkCompileErrors(vertexShader, "VERTEX")) return false;

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
        glCompileShader(fragmentShader);
        if (!checkCompileErrors(fragmentShader, "FRAGMENT")) return false;

        id = glCreateProgram();
        glAttachShader(id, vertexShader);
        glAttachShader(id, fragmentShader);
        glLinkProgram(id);
        if (!checkCompileErrors(id, "PROGRAM")) return false;

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return true;
    }

    void use() const {
        glUseProgram(id);
    }

    void setMat4(const char* name, const Mat4& mat) const {
        GLint loc = glGetUniformLocation(id, name);
        if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, mat.data());
    }

    void setVec3(const char* name, const Vec3& v) const {
        GLint loc = glGetUniformLocation(id, name);
        if (loc != -1) glUniform3f(loc, v.x, v.y, v.z);
    }

    void setVec4(const char* name, const Vec4& v) const {
        GLint loc = glGetUniformLocation(id, name);
        if (loc != -1) glUniform4f(loc, v.x, v.y, v.z, v.w);
    }

    void setFloat(const char* name, float val) const {
        GLint loc = glGetUniformLocation(id, name);
        if (loc != -1) glUniform1f(loc, val);
    }

    void setInt(const char* name, int val) const {
        GLint loc = glGetUniformLocation(id, name);
        if (loc != -1) glUniform1i(loc, val);
    }

    ~Shader() {
        if (id != 0) {
            glDeleteProgram(id);
            id = 0;
        }
    }

private:
    bool checkCompileErrors(GLuint shader, const std::string& type) {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
                std::cerr << "SHADER_COMPILATION_ERROR [" << type << "]:\n" << infoLog << "\n";
                return false;
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
                std::cerr << "PROGRAM_LINKING_ERROR:\n" << infoLog << "\n";
                return false;
            }
        }
        return true;
    }
};

namespace Shaders {

inline const char* worldVertex = R"(
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

inline const char* worldFragment = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 VertexColor;

out vec4 FragColor;

uniform vec3 uViewPos;
uniform vec4 uColor;
uniform int uShadingType; // 0: lit, 1: terrain, 2: unlit, 3: shadow, 4: safezone, 5: gloowall, 6: smoke, 7: sky
uniform float uTime;

void main() {
    // 7. Panoramic Sky Atmosphere
    if (uShadingType == 7) {
        float h = normalize(FragPos - uViewPos).y;
        vec3 skyZenith = vec3(0.12, 0.45, 0.88); // Deep vibrant blue
        vec3 skyHorizon = vec3(0.92, 0.82, 0.65); // Warm golden horizon
        vec3 col = mix(skyHorizon, skyZenith, clamp(h * 2.0, 0.0, 1.0));
        FragColor = vec4(col, 1.0);
        return;
    }

    // 3. Drop Shadow
    if (uShadingType == 3) {
        float d = length(TexCoords - vec2(0.5));
        if (d > 0.5) discard;
        float alpha = smoothstep(0.5, 0.05, d) * uColor.a;
        FragColor = vec4(0.02, 0.05, 0.03, alpha);
        return;
    }

    // 2. Unlit Glow
    if (uShadingType == 2) {
        FragColor = uColor * VertexColor;
        return;
    }

    // 4. Safe Zone Electric Cylinder (Vòng Bo)
    if (uShadingType == 4) {
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(uViewPos - FragPos);
        float rim = 1.0 - abs(dot(norm, viewDir));
        float wave = sin(FragPos.y * 1.8 - uTime * 4.0) * 0.5 + 0.5;
        float grid = abs(fract(TexCoords.y * 16.0 - uTime * 0.4) - 0.5);
        float alpha = 0.35 + 0.45 * pow(rim, 2.0) + 0.15 * wave;
        vec3 blueColor = mix(vec3(0.1, 0.65, 1.0), vec3(0.6, 0.95, 1.0), wave);
        FragColor = vec4(blueColor, alpha);
        return;
    }

    // 5. Bom Keo (Solid Ice Crystal Wall)
    if (uShadingType == 5) {
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(uViewPos - FragPos);
        float fresnel = pow(1.0 - max(dot(norm, viewDir), 0.0), 2.5);

        // Hexagonal frost lattice pattern
        vec2 p = FragPos.xz * 4.0;
        float frost = sin(p.x * 2.0) * cos(p.y * 2.0) + sin(FragPos.y * 6.0);
        vec3 iceCore = vec3(0.2, 0.75, 0.98);
        vec3 iceGlow = vec3(0.85, 0.98, 1.0);
        vec3 iceCol = mix(iceCore, iceGlow, fresnel * 0.75 + frost * 0.15);

        // Specular highlight
        vec3 lightDir = normalize(vec3(0.4, 0.85, 0.35));
        vec3 halfway = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfway), 0.0), 32.0);

        FragColor = vec4(iceCol + vec3(spec * 0.6), 0.92);
        return;
    }

    // 6. Airdrop Red Smoke Beacon
    if (uShadingType == 6) {
        float dist = length(TexCoords - vec2(0.5));
        if (dist > 0.5) discard;
        float alpha = (0.5 - dist) * 2.0 * uColor.a;
        FragColor = vec4(uColor.rgb, alpha);
        return;
    }

    vec4 baseColor = uColor * VertexColor;

    // 1. Bermuda Island Terrain
    if (uShadingType == 1) {
        // High quality procedural grass, pathways, and dirt
        float n1 = sin(FragPos.x * 0.2) * cos(FragPos.z * 0.2);
        vec3 grassLush = vec3(0.26, 0.62, 0.22); // Vibrant tropical green
        vec3 grassDry  = vec3(0.38, 0.55, 0.24);
        vec3 dirtPath  = vec3(0.55, 0.44, 0.28); // Warm dirt roads

        // Paths leading to buildings
        float roadX = abs(FragPos.x);
        float roadZ = abs(FragPos.z);
        float isRoad = step(roadX, 3.5) + step(roadZ, 3.5);

        vec3 ground = mix(grassLush, grassDry, n1 * 0.5 + 0.5);
        ground = mix(ground, dirtPath, clamp(isRoad, 0.0, 1.0));

        baseColor = vec4(ground, 1.0);
    }

    // --- Enhanced Blinn-Phong Lighting ---
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(0.45, 0.85, 0.35));
    vec3 lightCol = vec3(1.0, 0.97, 0.90);

    // Ambient
    vec3 ambient = vec3(0.38, 0.42, 0.48) * baseColor.rgb;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightCol * baseColor.rgb * 0.85;

    // Secondary Rim Light (Fresnel Glow on Character/Gun Edges)
    vec3 viewDir = normalize(uViewPos - FragPos);
    float rim = pow(1.0 - max(dot(norm, viewDir), 0.0), 3.0);
    vec3 rimLight = rim * vec3(0.3, 0.5, 0.7) * baseColor.rgb;

    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 24.0);
    vec3 specular = vec3(0.25) * spec * lightCol;

    // Soft Distance Fog
    float distToCam = length(FragPos - uViewPos);
    float fogFactor = clamp((distToCam - 40.0) / 180.0, 0.0, 0.45);
    vec3 fogColor = vec3(0.75, 0.82, 0.90);

    vec3 finalColor = ambient + diffuse + specular + rimLight;
    finalColor = mix(finalColor, fogColor, fogFactor);

    FragColor = vec4(finalColor, baseColor.a);
}
)";

inline const char* uiVertex = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec4 aColor;

out vec2 TexCoords;
out vec4 VertexColor;

uniform mat4 uProjection;

void main() {
    TexCoords = aTexCoords;
    VertexColor = aColor;
    gl_Position = uProjection * vec4(aPos, 1.0);
}
)";

inline const char* uiFragment = R"(
#version 330 core
in vec2 TexCoords;
in vec4 VertexColor;

out vec4 FragColor;

uniform vec4 uColor;
uniform int uShapeType;

void main() {
    if (uShapeType == 1) {
        float d = length(TexCoords - vec2(0.5));
        if (d > 0.5) discard;
        float alpha = smoothstep(0.5, 0.47, d);
        FragColor = uColor * VertexColor * vec4(1.0, 1.0, 1.0, alpha);
        return;
    }
    FragColor = uColor * VertexColor;
}
)";

} // namespace Shaders

#endif // SHADER_HPP
