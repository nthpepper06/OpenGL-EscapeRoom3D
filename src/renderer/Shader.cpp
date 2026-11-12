#include "renderer/Shader.hpp"
#include <fstream>
#include <sstream>

Shader::~Shader() {
    if (ID != 0) {
        glDeleteProgram(ID);
        ID = 0;
    }
}

bool Shader::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    // List of fallback directories to try
    std::vector<std::string> prefixes = {"", "../", "../../", "assets/shaders/", "../assets/shaders/"};

    bool vFound = false;
    bool fFound = false;

    for (const auto& pre : prefixes) {
        std::string tryV = pre + vertexPath;
        vShaderFile.open(tryV);
        if (vShaderFile.is_open()) {
            std::stringstream vStream;
            vStream << vShaderFile.rdbuf();
            vertexCode = vStream.str();
            vShaderFile.close();
            vFound = true;
            break;
        }
    }

    for (const auto& pre : prefixes) {
        std::string tryF = pre + fragmentPath;
        fShaderFile.open(tryF);
        if (fShaderFile.is_open()) {
            std::stringstream fStream;
            fStream << fShaderFile.rdbuf();
            fragmentCode = fStream.str();
            fShaderFile.close();
            fFound = true;
            break;
        }
    }

    if (!vFound || !fFound) {
        std::cerr << "Shader ERROR: Could not open shader files: " << vertexPath << " / " << fragmentPath << "\n";
        return false;
    }

    return loadFromStrings(vertexCode, fragmentCode);
}

bool Shader::loadFromStrings(const std::string& vertexCode, const std::string& fragmentCode) {
    if (ID != 0) {
        glDeleteProgram(ID);
        ID = 0;
    }
    uniformLocationCache.clear();
    warnedUniforms.clear();

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    if (!checkCompileErrors(vertex, "VERTEX")) {
        glDeleteShader(vertex);
        return false;
    }

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    if (!checkCompileErrors(fragment, "FRAGMENT")) {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return false;
    }

    // Shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    bool success = checkCompileErrors(ID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return success;
}

int Shader::getUniformLocation(const std::string& name) const {
    if (ID == 0) return -1;
    auto it = uniformLocationCache.find(name);
    if (it != uniformLocationCache.end()) {
        return it->second;
    }
    int loc = glGetUniformLocation(ID, name.c_str());
    uniformLocationCache[name] = loc;
    if (loc == -1) {
        if (warnedUniforms.find(name) == warnedUniforms.end()) {
            warnedUniforms.insert(name);
            std::cerr << "[Shader Warning] Uniform '" << name << "' (program " << ID
                      << ") does not exist or was optimized away (logged once).\n";
        }
    }
    return loc;
}

void Shader::use() const {
    if (ID != 0) {
        glUseProgram(ID);
    }
}

void Shader::setBool(const std::string& name, bool value) const {
    int loc = getUniformLocation(name);
    if (loc != -1) glUniform1i(loc, (int)value);
}

void Shader::setInt(const std::string& name, int value) const {
    int loc = getUniformLocation(name);
    if (loc != -1) glUniform1i(loc, value);
}

void Shader::setFloat(const std::string& name, float value) const {
    int loc = getUniformLocation(name);
    if (loc != -1) glUniform1f(loc, value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    int loc = getUniformLocation(name);
    if (loc != -1) glUniform2fv(loc, 1, &value[0]);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    int loc = getUniformLocation(name);
    if (loc != -1) glUniform3fv(loc, 1, &value[0]);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    int loc = getUniformLocation(name);
    if (loc != -1) glUniform4fv(loc, 1, &value[0]);
}

void Shader::setMat3(const std::string& name, const glm::mat3& mat) const {
    int loc = getUniformLocation(name);
    if (loc != -1) glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    int loc = getUniformLocation(name);
    if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

bool Shader::checkCompileErrors(unsigned int shader, const std::string& type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n";
            return false;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n";
            return false;
        }
    }
    return true;
}
