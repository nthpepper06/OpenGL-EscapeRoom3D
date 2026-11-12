#ifndef LAB4_MODELS_HPP
#define LAB4_MODELS_HPP

#include <glad/glad.h>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../Math3D.hpp"
#include "Lab4Transformations.hpp"

namespace Lab4 {

// Đỉnh 3D đầy đủ thông tin: vị trí, pháp tuyến, màu sắc
struct Vertex3D {
    Vec3 pos;
    Vec3 normal;
    Vec4 color;

    Vertex3D() = default;
    Vertex3D(const Vec3& p, const Vec3& n, const Vec4& c)
        : pos(p), normal(n), color(c) {}
};

// Cạnh trong mô hình khung kết nối (Yêu cầu 4.1)
struct WireEdge {
    int v1 = 0;
    int v2 = 0;
    WireEdge() = default;
    WireEdge(int _v1, int _v2) : v1(_v1), v2(_v2) {}
};

// Mặt đa giác trong mô hình mặt đa giác (Yêu cầu 4.2)
struct PolygonFace {
    std::vector<int> vertexIndices;
    Vec3 normal;
    Vec4 color;
};

// Lớp biểu diễn Vật thể 3D hoàn chỉnh hỗ trợ cả 2 mô hình (Khung kết nối & Mặt đa giác)
class Model3D {
public:
    std::string name;

    // 1. Dữ liệu hình học gốc
    std::vector<Vec3> originalVertices;
    std::vector<WireEdge> edges;
    std::vector<PolygonFace> faces;

    // 2. GPU Buffers cho Mô hình Mặt đa giác (GL_TRIANGLES)
    GLuint vaoSolid = 0;
    GLuint vboSolid = 0;
    GLuint eboSolid = 0;
    size_t solidIndexCount = 0;

    // 3. GPU Buffers cho Mô hình Khung kết nối (GL_LINES)
    GLuint vaoWire = 0;
    GLuint vboWire = 0;
    GLuint eboWire = 0;
    size_t wireIndexCount = 0;

    // 4. GPU Buffers cho Mô hình Điểm đỉnh (GL_POINTS)
    GLuint vaoPoints = 0;
    GLuint vboPoints = 0;
    size_t vertexCount = 0;

    Model3D() = default;

    void cleanup() {
        if (eboSolid) { glDeleteBuffers(1, &eboSolid); eboSolid = 0; }
        if (vboSolid) { glDeleteBuffers(1, &vboSolid); vboSolid = 0; }
        if (vaoSolid) { glDeleteVertexArrays(1, &vaoSolid); vaoSolid = 0; }

        if (eboWire) { glDeleteBuffers(1, &eboWire); eboWire = 0; }
        if (vboWire) { glDeleteBuffers(1, &vboWire); vboWire = 0; }
        if (vaoWire) { glDeleteVertexArrays(1, &vaoWire); vaoWire = 0; }

        if (vboPoints) { glDeleteBuffers(1, &vboPoints); vboPoints = 0; }
        if (vaoPoints) { glDeleteVertexArrays(1, &vaoPoints); vaoPoints = 0; }
    }

    ~Model3D() {
        // GPU resources freed explicitly
    }

    // Tải dữ liệu hình học lên OpenGL GPU
    void uploadToGPU() {
        cleanup();
        vertexCount = originalVertices.size();

        // --------------------------------------------------------------------
        // A. TẠO BUFFER CHO MẶT ĐA GIÁC (POLYGON / SURFACE MODEL)
        // --------------------------------------------------------------------
        std::vector<Vertex3D> solidVertices;
        std::vector<unsigned int> solidIndices;

        for (const auto& face : faces) {
            if (face.vertexIndices.size() < 3) continue;

            // Tính pháp tuyến nếu chưa có
            Vec3 norm = face.normal;
            if (norm.lengthSq() < 1e-4f) {
                Vec3 p0 = originalVertices[face.vertexIndices[0]];
                Vec3 p1 = originalVertices[face.vertexIndices[1]];
                Vec3 p2 = originalVertices[face.vertexIndices[2]];
                norm = cross(p1 - p0, p2 - p0).normalized();
            }

            // Tam giác hóa mặt đa giác (Fan Triangulation cho n-giác lồi)
            unsigned int baseIdx = static_cast<unsigned int>(solidVertices.size());
            for (size_t i = 0; i < face.vertexIndices.size(); ++i) {
                int vIdx = face.vertexIndices[i];
                solidVertices.push_back(Vertex3D(originalVertices[vIdx], norm, face.color));
            }

            for (size_t i = 1; i + 1 < face.vertexIndices.size(); ++i) {
                solidIndices.push_back(baseIdx);
                solidIndices.push_back(baseIdx + static_cast<unsigned int>(i));
                solidIndices.push_back(baseIdx + static_cast<unsigned int>(i + 1));
            }
        }

        solidIndexCount = solidIndices.size();

        if (!solidVertices.empty()) {
            glGenVertexArrays(1, &vaoSolid);
            glGenBuffers(1, &vboSolid);
            glGenBuffers(1, &eboSolid);

            glBindVertexArray(vaoSolid);
            glBindBuffer(GL_ARRAY_BUFFER, vboSolid);
            glBufferData(GL_ARRAY_BUFFER, solidVertices.size() * sizeof(Vertex3D), solidVertices.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboSolid);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, solidIndices.size() * sizeof(unsigned int), solidIndices.data(), GL_STATIC_DRAW);

            // Layout 0: pos (3 floats)
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, pos));

            // Layout 1: normal (3 floats)
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, normal));

            // Layout 2: color (4 floats)
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, color));

            glBindVertexArray(0);
        }

        // --------------------------------------------------------------------
        // B. TẠO BUFFER CHO KHUNG KẾT NỐI (WIREFRAME MODEL)
        // --------------------------------------------------------------------
        std::vector<Vertex3D> wireVertices;
        std::vector<unsigned int> wireIndices;

        for (size_t i = 0; i < originalVertices.size(); ++i) {
            wireVertices.push_back(Vertex3D(originalVertices[i], Vec3(0, 1, 0), Vec4(0.0f, 0.95f, 0.85f, 1.0f)));
        }

        for (const auto& e : edges) {
            wireIndices.push_back(static_cast<unsigned int>(e.v1));
            wireIndices.push_back(static_cast<unsigned int>(e.v2));
        }

        wireIndexCount = wireIndices.size();

        if (!wireVertices.empty() && !wireIndices.empty()) {
            glGenVertexArrays(1, &vaoWire);
            glGenBuffers(1, &vboWire);
            glGenBuffers(1, &eboWire);

            glBindVertexArray(vaoWire);
            glBindBuffer(GL_ARRAY_BUFFER, vboWire);
            glBufferData(GL_ARRAY_BUFFER, wireVertices.size() * sizeof(Vertex3D), wireVertices.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboWire);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, wireIndices.size() * sizeof(unsigned int), wireIndices.data(), GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, pos));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, normal));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, color));

            glBindVertexArray(0);
        }

        // --------------------------------------------------------------------
        // C. TẠO BUFFER CHO CÁC ĐỈNH ĐỘC LẬP (POINTS)
        // --------------------------------------------------------------------
        if (!originalVertices.empty()) {
            std::vector<Vertex3D> ptVertices;
            for (const auto& v : originalVertices) {
                ptVertices.push_back(Vertex3D(v, Vec3(0, 1, 0), Vec4(1.0f, 0.85f, 0.2f, 1.0f)));
            }

            glGenVertexArrays(1, &vaoPoints);
            glGenBuffers(1, &vboPoints);

            glBindVertexArray(vaoPoints);
            glBindBuffer(GL_ARRAY_BUFFER, vboPoints);
            glBufferData(GL_ARRAY_BUFFER, ptVertices.size() * sizeof(Vertex3D), ptVertices.data(), GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, pos));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, normal));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, color));

            glBindVertexArray(0);
        }
    }

    // Vẽ mô hình mặt đa giác (Polygon / Solid)
    void renderSolid() const {
        if (vaoSolid == 0 || solidIndexCount == 0) return;
        glBindVertexArray(vaoSolid);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(solidIndexCount), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    // Vẽ mô hình khung kết nối (Wireframe)
    void renderWireframe() const {
        if (vaoWire == 0 || wireIndexCount == 0) return;
        glBindVertexArray(vaoWire);
        glDrawElements(GL_LINES, static_cast<GLsizei>(wireIndexCount), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    // Vẽ các đỉnh của vật thể (Points)
    void renderPoints() const {
        if (vaoPoints == 0 || vertexCount == 0) return;
        glBindVertexArray(vaoPoints);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(vertexCount));
        glBindVertexArray(0);
    }
};

// ============================================================================
// BỘ SINH CÁC MÔ HÌNH VẬT THỂ 3D ĐA DẠNG (3D MODEL GENERATORS)
// ============================================================================
class Model3DGenerator {
public:
    // Bảng màu rực rỡ phân biệt các mặt đa giác
    static inline const Vec4 Palette[12] = {
        Vec4(0.92f, 0.25f, 0.25f, 1.0f), // Đỏ
        Vec4(0.25f, 0.78f, 0.35f, 1.0f), // Xanh lá
        Vec4(0.20f, 0.55f, 0.95f, 1.0f), // Xanh dương
        Vec4(0.98f, 0.78f, 0.15f, 1.0f), // Vàng
        Vec4(0.95f, 0.45f, 0.15f, 1.0f), // Cam
        Vec4(0.75f, 0.30f, 0.88f, 1.0f), // Tím
        Vec4(0.15f, 0.85f, 0.85f, 1.0f), // Cyan
        Vec4(0.92f, 0.35f, 0.65f, 1.0f), // Hồng
        Vec4(0.40f, 0.80f, 0.60f, 1.0f), // Xanh ngọc
        Vec4(0.85f, 0.65f, 0.40f, 1.0f), // Nâu vàng
        Vec4(0.60f, 0.75f, 0.90f, 1.0f), // Xanh nhạt
        Vec4(0.80f, 0.80f, 0.85f, 1.0f)  // Bạc
    };

    // 1. Hình lập phương (Cube) - 8 đỉnh, 12 cạnh, 6 mặt tứ giác
    static Model3D createCube(float size = 1.6f) {
        Model3D m;
        m.name = "Hinh Lap Phuong (Cube)";
        float h = size * 0.5f;

        // 8 Đỉnh
        m.originalVertices = {
            Vec3(-h, -h,  h), // 0: Dưới Trước Trái
            Vec3( h, -h,  h), // 1: Dưới Trước Phải
            Vec3( h,  h,  h), // 2: Trên Trước Phải
            Vec3(-h,  h,  h), // 3: Trên Trước Trái
            Vec3(-h, -h, -h), // 4: Dưới Sau Trái
            Vec3( h, -h, -h), // 5: Dưới Sau Phải
            Vec3( h,  h, -h), // 6: Trên Sau Phải
            Vec3(-h,  h, -h)  // 7: Trên Sau Trái
        };

        // 12 Cạnh Khung kết nối
        m.edges = {
            // Mặt trước
            {0, 1}, {1, 2}, {2, 3}, {3, 0},
            // Mặt sau
            {4, 5}, {5, 6}, {6, 7}, {7, 4},
            // 4 Cạnh nối trước - sau
            {0, 4}, {1, 5}, {2, 6}, {3, 7}
        };

        // 6 Mặt đa giác (mỗi mặt 1 màu riêng biệt)
        m.faces = {
            // Trước (Z+)
            {{0, 1, 2, 3}, Vec3(0, 0, 1), Palette[0]},
            // Sau (Z-)
            {{5, 4, 7, 6}, Vec3(0, 0, -1), Palette[1]},
            // Trái (X-)
            {{4, 0, 3, 7}, Vec3(-1, 0, 0), Palette[2]},
            // Phải (X+)
            {{1, 5, 6, 2}, Vec3(1, 0, 0), Palette[3]},
            // Trên (Y+)
            {{3, 2, 6, 7}, Vec3(0, 1, 0), Palette[4]},
            // Dưới (Y-)
            {{4, 5, 1, 0}, Vec3(0, -1, 0), Palette[5]}
        };

        m.uploadToGPU();
        return m;
    }

    // 2. Hình chóp tứ giác / Kim tự tháp (Pyramid) - 5 đỉnh, 8 cạnh, 5 mặt
    static Model3D createPyramid(float baseSize = 1.8f, float height = 2.0f) {
        Model3D m;
        m.name = "Kim Tu Thap / Hinh Chop (Pyramid)";
        float h = baseSize * 0.5f;
        float yBottom = -height * 0.4f;
        float yTop = height * 0.6f;

        m.originalVertices = {
            Vec3(-h, yBottom,  h), // 0: Đáy Trước Trái
            Vec3( h, yBottom,  h), // 1: Đáy Trước Phải
            Vec3( h, yBottom, -h), // 2: Đáy Sau Phải
            Vec3(-h, yBottom, -h), // 3: Đáy Sau Trái
            Vec3(0.0f, yTop, 0.0f) // 4: Đỉnh Chóp
        };

        m.edges = {
            // 4 Cạnh đáy
            {0, 1}, {1, 2}, {2, 3}, {3, 0},
            // 4 Cạnh bên nối lên đỉnh
            {0, 4}, {1, 4}, {2, 4}, {3, 4}
        };

        m.faces = {
            // Mặt đáy (Tứ giác)
            {{3, 2, 1, 0}, Vec3(0, -1, 0), Palette[5]},
            // 4 Mặt bên tam giác
            {{0, 1, 4}, Vec3(0, 0.5f, 1.0f).normalized(), Palette[0]},
            {{1, 2, 4}, Vec3(1.0f, 0.5f, 0).normalized(), Palette[2]},
            {{2, 3, 4}, Vec3(0, 0.5f, -1.0f).normalized(), Palette[3]},
            {{3, 0, 4}, Vec3(-1.0f, 0.5f, 0).normalized(), Palette[4]}
        };

        m.uploadToGPU();
        return m;
    }

    // 3. Hình lăng trụ tam giác (Triangular Prism) - 6 đỉnh, 9 cạnh, 5 mặt
    static Model3D createTriangularPrism(float radius = 1.2f, float length = 2.0f) {
        Model3D m;
        m.name = "Lang Tru Tam Giac (Triangular Prism)";
        float hz = length * 0.5f;

        // Góc 3 đỉnh tam giác đều: 90°, 210°, 330°
        float a0 = 90.0f * LAB4_DEG2RAD;
        float a1 = 210.0f * LAB4_DEG2RAD;
        float a2 = 330.0f * LAB4_DEG2RAD;

        m.originalVertices = {
            // Đáy trước (+Z)
            Vec3(radius * std::cos(a0), radius * std::sin(a0),  hz), // 0
            Vec3(radius * std::cos(a1), radius * std::sin(a1),  hz), // 1
            Vec3(radius * std::cos(a2), radius * std::sin(a2),  hz), // 2
            // Đáy sau (-Z)
            Vec3(radius * std::cos(a0), radius * std::sin(a0), -hz), // 3
            Vec3(radius * std::cos(a1), radius * std::sin(a1), -hz), // 4
            Vec3(radius * std::cos(a2), radius * std::sin(a2), -hz)  // 5
        };

        m.edges = {
            // Đáy trước
            {0, 1}, {1, 2}, {2, 0},
            // Đáy sau
            {3, 4}, {4, 5}, {5, 3},
            // Cạnh bên
            {0, 3}, {1, 4}, {2, 5}
        };

        m.faces = {
            // 2 Đáy tam giác
            {{0, 1, 2}, Vec3(0, 0, 1), Palette[0]},
            {{5, 4, 3}, Vec3(0, 0, -1), Palette[1]},
            // 3 Mặt bên chữ nhật
            {{1, 4, 3, 0}, Vec3(-1.0f, 0.5f, 0).normalized(), Palette[2]},
            {{2, 5, 4, 1}, Vec3(0, -1.0f, 0).normalized(), Palette[3]},
            {{0, 3, 5, 2}, Vec3(1.0f, 0.5f, 0).normalized(), Palette[4]}
        };

        m.uploadToGPU();
        return m;
    }

    // 4. Hình lăng trụ lục giác (Hexagonal Prism) - 12 đỉnh, 18 cạnh, 8 mặt
    static Model3D createHexagonalPrism(float radius = 1.1f, float height = 1.8f) {
        Model3D m;
        m.name = "Lang Tru Luc Giac (Hexagonal Prism)";
        float hy = height * 0.5f;

        // 6 đỉnh đáy trên (Y+), 6 đỉnh đáy dưới (Y-)
        for (int i = 0; i < 6; ++i) {
            float angle = (i * 60.0f) * LAB4_DEG2RAD;
            float x = radius * std::cos(angle);
            float z = radius * std::sin(angle);
            m.originalVertices.push_back(Vec3(x,  hy, z)); // 0..5 (Top)
        }
        for (int i = 0; i < 6; ++i) {
            float angle = (i * 60.0f) * LAB4_DEG2RAD;
            float x = radius * std::cos(angle);
            float z = radius * std::sin(angle);
            m.originalVertices.push_back(Vec3(x, -hy, z)); // 6..11 (Bottom)
        }

        // Cạnh
        for (int i = 0; i < 6; ++i) {
            int next = (i + 1) % 6;
            m.edges.push_back({i, next});         // Vòng trên
            m.edges.push_back({i + 6, next + 6}); // Vòng dưới
            m.edges.push_back({i, i + 6});        // Cột đứng
        }

        // Mặt trên & dưới
        std::vector<int> topFace = {0, 1, 2, 3, 4, 5};
        std::vector<int> botFace = {11, 10, 9, 8, 7, 6};
        m.faces.push_back({topFace, Vec3(0, 1, 0), Palette[0]});
        m.faces.push_back({botFace, Vec3(0, -1, 0), Palette[1]});

        // 6 Mặt bên
        for (int i = 0; i < 6; ++i) {
            int next = (i + 1) % 6;
            float midAngle = (i * 60.0f + 30.0f) * LAB4_DEG2RAD;
            Vec3 norm(std::cos(midAngle), 0.0f, std::sin(midAngle));
            m.faces.push_back({{i, next, next + 6, i + 6}, norm, Palette[(i + 2) % 12]});
        }

        m.uploadToGPU();
        return m;
    }

    // 5. Hình cầu (UV Sphere) - Lưới vĩ tuyến & kinh tuyến thể hiện mặt cong 3D
    static Model3D createUVSphere(float radius = 1.1f, int rings = 14, int sectors = 20) {
        Model3D m;
        m.name = "Hinh Cau Mat Cong (UV Sphere)";

        // Sinh các đỉnh
        for (int r = 0; r <= rings; ++r) {
            float phi = LAB4_PI * (float)r / (float)rings; // 0 to PI
            float y = radius * std::cos(phi);
            float rSinPhi = radius * std::sin(phi);

            for (int s = 0; s < sectors; ++s) {
                float theta = 2.0f * LAB4_PI * (float)s / (float)sectors;
                float x = rSinPhi * std::cos(theta);
                float z = rSinPhi * std::sin(theta);
                m.originalVertices.push_back(Vec3(x, y, z));
            }
        }

        // Sinh cạnh khung kết nối (Vĩ tuyến và Kinh tuyến)
        for (int r = 0; r <= rings; ++r) {
            for (int s = 0; s < sectors; ++s) {
                int cur = r * sectors + s;
                int nextSector = r * sectors + (s + 1) % sectors;
                m.edges.push_back({cur, nextSector}); // Vĩ tuyến ngang

                if (r < rings) {
                    int nextRing = (r + 1) * sectors + s;
                    m.edges.push_back({cur, nextRing}); // Kinh tuyến dọc
                }
            }
        }

        // Sinh mặt đa giác
        for (int r = 0; r < rings; ++r) {
            for (int s = 0; s < sectors; ++s) {
                int cur = r * sectors + s;
                int nextS = r * sectors + (s + 1) % sectors;
                int nextR = (r + 1) * sectors + s;
                int nextRS = (r + 1) * sectors + (s + 1) % sectors;

                Vec3 center = (m.originalVertices[cur] + m.originalVertices[nextS] +
                               m.originalVertices[nextRS] + m.originalVertices[nextR]) * 0.25f;
                Vec3 norm = center.normalized();
                Vec4 color = Palette[(r + s) % 12];

                m.faces.push_back({{cur, nextS, nextRS, nextR}, norm, color});
            }
        }

        m.uploadToGPU();
        return m;
    }

    // 6. Hình nón (Cone) - Đỉnh nhọn và đáy tròn
    static Model3D createCone(float radius = 1.2f, float height = 2.0f, int sectors = 24) {
        Model3D m;
        m.name = "Hinh Non (Cone)";
        float yBot = -height * 0.5f;
        float yTop =  height * 0.5f;

        // Đỉnh 0: Chóp nhọn
        m.originalVertices.push_back(Vec3(0, yTop, 0));
        // Đỉnh 1: Tâm đáy
        m.originalVertices.push_back(Vec3(0, yBot, 0));

        // Các đỉnh viền đáy
        for (int i = 0; i < sectors; ++i) {
            float angle = 2.0f * LAB4_PI * (float)i / (float)sectors;
            m.originalVertices.push_back(Vec3(radius * std::cos(angle), yBot, radius * std::sin(angle)));
        }

        // Cạnh
        for (int i = 0; i < sectors; ++i) {
            int cur = 2 + i;
            int next = 2 + (i + 1) % sectors;
            m.edges.push_back({cur, next}); // Vòng đáy
            m.edges.push_back({0, cur});    // Nối lên chóp
            if (i % 4 == 0) m.edges.push_back({1, cur}); // Nan hoa đáy
        }

        // Mặt bên tam giác
        for (int i = 0; i < sectors; ++i) {
            int cur = 2 + i;
            int next = 2 + (i + 1) % sectors;
            Vec3 pMid = (m.originalVertices[cur] + m.originalVertices[next]) * 0.5f;
            Vec3 norm = Vec3(pMid.x, radius / height, pMid.z).normalized();
            m.faces.push_back({{0, cur, next}, norm, Palette[i % 12]});
        }

        // Mặt đáy
        for (int i = 0; i < sectors; ++i) {
            int cur = 2 + i;
            int next = 2 + (i + 1) % sectors;
            m.faces.push_back({{1, next, cur}, Vec3(0, -1, 0), Palette[6]});
        }

        m.uploadToGPU();
        return m;
    }

    // 7. Hình trụ (Cylinder) - 2 đáy tròn và mặt thân
    static Model3D createCylinder(float radius = 1.0f, float height = 1.8f, int sectors = 24) {
        Model3D m;
        m.name = "Hinh Tru (Cylinder)";
        float hy = height * 0.5f;

        // Tâm trên (0) và Tâm dưới (1)
        m.originalVertices.push_back(Vec3(0,  hy, 0)); // 0
        m.originalVertices.push_back(Vec3(0, -hy, 0)); // 1

        // Đáy trên: 2 .. 2+sectors-1
        for (int i = 0; i < sectors; ++i) {
            float angle = 2.0f * LAB4_PI * (float)i / (float)sectors;
            m.originalVertices.push_back(Vec3(radius * std::cos(angle), hy, radius * std::sin(angle)));
        }

        // Đáy dưới: 2+sectors .. 2+2*sectors-1
        int botStart = 2 + sectors;
        for (int i = 0; i < sectors; ++i) {
            float angle = 2.0f * LAB4_PI * (float)i / (float)sectors;
            m.originalVertices.push_back(Vec3(radius * std::cos(angle), -hy, radius * std::sin(angle)));
        }

        // Cạnh khung kết nối
        for (int i = 0; i < sectors; ++i) {
            int tCur = 2 + i;
            int tNext = 2 + (i + 1) % sectors;
            int bCur = botStart + i;
            int bNext = botStart + (i + 1) % sectors;

            m.edges.push_back({tCur, tNext}); // Vòng trên
            m.edges.push_back({bCur, bNext}); // Vòng dưới
            m.edges.push_back({tCur, bCur});  // Đường sinh dọc thân
        }

        // Mặt thân chữ nhật
        for (int i = 0; i < sectors; ++i) {
            int tCur = 2 + i;
            int tNext = 2 + (i + 1) % sectors;
            int bCur = botStart + i;
            int bNext = botStart + (i + 1) % sectors;

            float midAngle = 2.0f * LAB4_PI * ((float)i + 0.5f) / (float)sectors;
            Vec3 norm(std::cos(midAngle), 0, std::sin(midAngle));
            m.faces.push_back({{tCur, tNext, bNext, bCur}, norm, Palette[i % 12]});
        }

        // Mặt đáy trên & đáy dưới
        for (int i = 0; i < sectors; ++i) {
            int tCur = 2 + i;
            int tNext = 2 + (i + 1) % sectors;
            m.faces.push_back({{0, tCur, tNext}, Vec3(0, 1, 0), Palette[3]});

            int bCur = botStart + i;
            int bNext = botStart + (i + 1) % sectors;
            m.faces.push_back({{1, bNext, bCur}, Vec3(0, -1, 0), Palette[4]});
        }

        m.uploadToGPU();
        return m;
    }

    // 8. Hình xuyến (Torus / Bánh Donut) - Đỉnh cao mô phỏng mặt cong 3D
    static Model3D createTorus(float mainRadius = 1.1f, float tubeRadius = 0.45f, int mainSegs = 24, int tubeSegs = 16) {
        Model3D m;
        m.name = "Hinh Xuyen (Torus / Donut)";

        for (int i = 0; i < mainSegs; ++i) {
            float u = 2.0f * LAB4_PI * (float)i / (float)mainSegs;
            float cosU = std::cos(u);
            float sinU = std::sin(u);

            for (int j = 0; j < tubeSegs; ++j) {
                float v = 2.0f * LAB4_PI * (float)j / (float)tubeSegs;
                float cosV = std::cos(v);
                float sinV = std::sin(v);

                float x = (mainRadius + tubeRadius * cosV) * cosU;
                float y = tubeRadius * sinV;
                float z = (mainRadius + tubeRadius * cosV) * sinU;
                m.originalVertices.push_back(Vec3(x, y, z));
            }
        }

        // Cạnh khung kết nối
        for (int i = 0; i < mainSegs; ++i) {
            int nextI = (i + 1) % mainSegs;
            for (int j = 0; j < tubeSegs; ++j) {
                int nextJ = (j + 1) % tubeSegs;

                int cur = i * tubeSegs + j;
                int right = nextI * tubeSegs + j;
                int up = i * tubeSegs + nextJ;

                m.edges.push_back({cur, right});
                m.edges.push_back({cur, up});
            }
        }

        // Mặt đa giác
        for (int i = 0; i < mainSegs; ++i) {
            int nextI = (i + 1) % mainSegs;
            for (int j = 0; j < tubeSegs; ++j) {
                int nextJ = (j + 1) % tubeSegs;

                int v0 = i * tubeSegs + j;
                int v1 = nextI * tubeSegs + j;
                int v2 = nextI * tubeSegs + nextJ;
                int v3 = i * tubeSegs + nextJ;

                Vec3 center = (m.originalVertices[v0] + m.originalVertices[v1] +
                               m.originalVertices[v2] + m.originalVertices[v3]) * 0.25f;
                float u = 2.0f * LAB4_PI * ((float)i + 0.5f) / (float)mainSegs;
                Vec3 ringCenter(mainRadius * std::cos(u), 0.0f, mainRadius * std::sin(u));
                Vec3 norm = (center - ringCenter).normalized();

                m.faces.push_back({{v0, v1, v2, v3}, norm, Palette[(i + j) % 12]});
            }
        }

        m.uploadToGPU();
        return m;
    }

    // 9. Mô hình Ngôi nhà 3D (3D House Model) - Vật thể tiêu chuẩn bài tập Đồ họa máy tính
    static Model3D create3DHouse(float w = 1.6f, float d = 2.0f, float hWall = 1.2f, float hRoof = 0.8f) {
        Model3D m;
        m.name = "Mo Hinh Ngoi Nha 3D (3D House)";

        float hw = w * 0.5f;
        float hd = d * 0.5f;
        float yBot = -0.8f;
        float yWall = yBot + hWall;
        float yPeak = yWall + hRoof;

        // 10 Đỉnh chính của Ngôi nhà
        m.originalVertices = {
            // Thân nhà (Khối hộp chữ nhật dưới)
            Vec3(-hw, yBot,  hd), // 0: Trước Trái Dưới
            Vec3( hw, yBot,  hd), // 1: Trước Phải Dưới
            Vec3( hw, yWall, hd), // 2: Trước Phải Trên
            Vec3(-hw, yWall, hd), // 3: Trước Trái Trên

            Vec3(-hw, yBot, -hd), // 4: Sau Trái Dưới
            Vec3( hw, yBot, -hd), // 5: Sau Phải Dưới
            Vec3( hw, yWall,-hd), // 6: Sau Phải Trên
            Vec3(-hw, yWall,-hd), // 7: Sau Trái Trên

            // 2 Đỉnh chóp nóc nhà (Đỉnh mái dốc)
            Vec3(0.0f, yPeak,  hd), // 8: Đỉnh mái trước
            Vec3(0.0f, yPeak, -hd)  // 9: Đỉnh mái sau
        };

        // 17 Cạnh Khung kết nối
        m.edges = {
            // Đáy nhà
            {0, 1}, {1, 5}, {5, 4}, {4, 0},
            // Cột tường đứng
            {0, 3}, {1, 2}, {5, 6}, {4, 7},
            // Vành trên tường
            {3, 2}, {2, 6}, {6, 7}, {7, 3},
            // Mái dốc trước & sau
            {3, 8}, {2, 8},
            {7, 9}, {6, 9},
            // Đòn nóc nhà
            {8, 9}
        };

        // Các mặt đa giác
        m.faces = {
            // Đáy nhà
            {{4, 5, 1, 0}, Vec3(0, -1, 0), Palette[9]},
            // Tường trước (Đa giác 5 cạnh: hình chữ nhật + tam giác trán mái)
            {{0, 1, 2, 8, 3}, Vec3(0, 0, 1), Palette[0]},
            // Tường sau (Đa giác 5 cạnh)
            {{5, 4, 7, 9, 6}, Vec3(0, 0, -1), Palette[1]},
            // Tường trái
            {{4, 0, 3, 7}, Vec3(-1, 0, 0), Palette[2]},
            // Tường phải
            {{1, 5, 6, 2}, Vec3(1, 0, 0), Palette[3]},
            // Mái dốc bên phải
            {{2, 6, 9, 8}, Vec3(1.0f, 1.0f, 0).normalized(), Palette[4]},
            // Mái dốc bên trái
            {{7, 3, 8, 9}, Vec3(-1.0f, 1.0f, 0).normalized(), Palette[7]}
        };

        m.uploadToGPU();
        return m;
    }
};

} // namespace Lab4

#endif // LAB4_MODELS_HPP
