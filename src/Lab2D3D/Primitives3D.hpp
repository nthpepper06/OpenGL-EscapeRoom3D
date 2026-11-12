#ifndef PRIMITIVES_3D_HPP
#define PRIMITIVES_3D_HPP

#include <glad/glad.h>
#include <vector>
#include <cmath>
#include <string>
#include "../Math3D.hpp"

namespace Lab3D {

struct Vertex3D {
    Vec3 pos;
    Vec3 normal;
    Vec2 texCoord;
    Vec4 color;

    Vertex3D() = default;
    Vertex3D(const Vec3& p, const Vec3& n, const Vec2& uv, const Vec4& c)
        : pos(p), normal(n), texCoord(uv), color(c) {}
    Vertex3D(const Vec3& p, const Vec3& n, const Vec4& c)
        : pos(p), normal(n), texCoord(0.0f, 0.0f), color(c) {}
};

struct Mesh3D {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLenum drawMode = GL_TRIANGLES;
    size_t indexCount = 0;
    size_t vertexCount = 0;
    std::string name;

    void upload(const std::vector<Vertex3D>& vertices, const std::vector<unsigned int>& indices, GLenum mode = GL_TRIANGLES, const std::string& meshName = "") {
        name = meshName;
        drawMode = mode;
        vertexCount = vertices.size();
        indexCount = indices.size();

        if (vao == 0) glGenVertexArrays(1, &vao);
        if (vbo == 0) glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex3D), vertices.data(), GL_STATIC_DRAW);

        if (!indices.empty()) {
            if (ebo == 0) glGenBuffers(1, &ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        }

        // Layout 0: Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, pos));

        // Layout 1: Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, normal));

        // Layout 2: TexCoord
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, texCoord));

        // Layout 3: Color
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, color));

        glBindVertexArray(0);
    }

    void draw() const {
        if (vao == 0) return;
        glBindVertexArray(vao);
        if (indexCount > 0) {
            glDrawElements(drawMode, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(drawMode, 0, static_cast<GLsizei>(vertexCount));
        }
        glBindVertexArray(0);
    }

    void destroy() {
        if (ebo) { glDeleteBuffers(1, &ebo); ebo = 0; }
        if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
        if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
    }
};

class Shape3DGenerator {
public:
    // 1. Hình lập phương 3D (Cube - 6 mặt đa màu sắc & pháp tuyến)
    static Mesh3D createCube(float size = 1.0f, bool multiColor = true) {
        float h = size * 0.5f;
        std::vector<Vertex3D> verts;
        std::vector<unsigned int> indices;

        // Bảng màu 6 mặt
        Vec4 colors[6] = {
            Vec4(0.95f, 0.25f, 0.25f, 1.0f), // Mặt trước (Front): Đỏ
            Vec4(0.20f, 0.85f, 0.35f, 1.0f), // Mặt sau (Back): Xanh lá
            Vec4(0.20f, 0.50f, 0.95f, 1.0f), // Mặt trái (Left): Xanh dương
            Vec4(0.95f, 0.85f, 0.20f, 1.0f), // Mặt phải (Right): Vàng
            Vec4(0.85f, 0.35f, 0.95f, 1.0f), // Mặt trên (Top): Tím
            Vec4(0.95f, 0.55f, 0.15f, 1.0f)  // Mặt dưới (Bottom): Cam
        };

        if (!multiColor) {
            Vec4 defaultCol(0.8f, 0.8f, 0.85f, 1.0f);
            for (int i = 0; i < 6; ++i) colors[i] = defaultCol;
        }

        auto addFace = [&](const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& n, const Vec4& c) {
            unsigned int base = verts.size();
            verts.emplace_back(p0, n, Vec2(0, 0), c);
            verts.emplace_back(p1, n, Vec2(1, 0), c);
            verts.emplace_back(p2, n, Vec2(1, 1), c);
            verts.emplace_back(p3, n, Vec2(0, 1), c);

            indices.push_back(base);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        };

        // Front (+Z)
        addFace(Vec3(-h, -h,  h), Vec3( h, -h,  h), Vec3( h,  h,  h), Vec3(-h,  h,  h), Vec3(0, 0, 1), colors[0]);
        // Back (-Z)
        addFace(Vec3( h, -h, -h), Vec3(-h, -h, -h), Vec3(-h,  h, -h), Vec3( h,  h, -h), Vec3(0, 0, -1), colors[1]);
        // Left (-X)
        addFace(Vec3(-h, -h, -h), Vec3(-h, -h,  h), Vec3(-h,  h,  h), Vec3(-h,  h, -h), Vec3(-1, 0, 0), colors[2]);
        // Right (+X)
        addFace(Vec3( h, -h,  h), Vec3( h, -h, -h), Vec3( h,  h, -h), Vec3( h,  h,  h), Vec3(1, 0, 0), colors[3]);
        // Top (+Y)
        addFace(Vec3(-h,  h,  h), Vec3( h,  h,  h), Vec3( h,  h, -h), Vec3(-h,  h, -h), Vec3(0, 1, 0), colors[4]);
        // Bottom (-Y)
        addFace(Vec3(-h, -h, -h), Vec3( h, -h, -h), Vec3( h, -h,  h), Vec3(-h, -h,  h), Vec3(0, -1, 0), colors[5]);

        Mesh3D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "3D Cube");
        return mesh;
    }

    // 2. Hình kim tự tháp / Chóp tứ giác (Pyramid)
    static Mesh3D createPyramid(float baseWidth = 1.0f, float height = 1.2f, bool multiColor = true) {
        float hw = baseWidth * 0.5f;
        std::vector<Vertex3D> verts;
        std::vector<unsigned int> indices;

        Vec3 apex(0.0f, height * 0.5f, 0.0f);
        Vec3 b0(-hw, -height * 0.5f,  hw); // Trước trái
        Vec3 b1( hw, -height * 0.5f,  hw); // Trước phải
        Vec3 b2( hw, -height * 0.5f, -hw); // Sau phải
        Vec3 b3(-hw, -height * 0.5f, -hw); // Sau trái

        Vec4 colors[5] = {
            Vec4(0.95f, 0.35f, 0.20f, 1.0f), // Mặt trước: Đỏ cam
            Vec4(0.20f, 0.80f, 0.40f, 1.0f), // Mặt phải: Xanh lá
            Vec4(0.25f, 0.55f, 0.95f, 1.0f), // Mặt sau: Xanh dương
            Vec4(0.95f, 0.80f, 0.20f, 1.0f), // Mặt trái: Vàng
            Vec4(0.50f, 0.50f, 0.55f, 1.0f)  // Đáy: Xám
        };

        if (!multiColor) {
            Vec4 defaultCol(0.85f, 0.75f, 0.45f, 1.0f); // Màu cát sa mạc
            for (int i = 0; i < 5; ++i) colors[i] = defaultCol;
        }

        auto addTri = [&](const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec4& c) {
            Vec3 edge1 = p1 - p0;
            Vec3 edge2 = p2 - p0;
            Vec3 n = cross(edge1, edge2).normalized();
            unsigned int base = verts.size();
            verts.emplace_back(p0, n, c);
            verts.emplace_back(p1, n, c);
            verts.emplace_back(p2, n, c);
            indices.push_back(base);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
        };

        // 4 mặt bên
        addTri(b0, b1, apex, colors[0]); // Front
        addTri(b1, b2, apex, colors[1]); // Right
        addTri(b2, b3, apex, colors[2]); // Back
        addTri(b3, b0, apex, colors[3]); // Left

        // Mặt đáy (2 tam giác)
        Vec3 nBottom(0, -1, 0);
        unsigned int base = verts.size();
        verts.emplace_back(b3, nBottom, colors[4]);
        verts.emplace_back(b2, nBottom, colors[4]);
        verts.emplace_back(b1, nBottom, colors[4]);
        verts.emplace_back(b0, nBottom, colors[4]);
        indices.push_back(base);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base);
        indices.push_back(base + 2);
        indices.push_back(base + 3);

        Mesh3D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "3D Pyramid");
        return mesh;
    }

    // 3. Hình lăng trụ đều n-cạnh (Prism: Triangular, Hexagonal)
    static Mesh3D createPrism(float radius = 0.6f, float height = 1.0f, int sides = 6, const Vec4& color = Vec4(0.4f, 0.8f, 0.85f, 1.0f)) {
        std::vector<Vertex3D> verts;
        std::vector<unsigned int> indices;
        float hh = height * 0.5f;

        float angleStep = 2.0f * PI / sides;

        // Các mặt bên (Quads)
        for (int i = 0; i < sides; ++i) {
            float a1 = i * angleStep;
            float a2 = (i + 1) * angleStep;

            float x1 = radius * std::cos(a1), z1 = radius * std::sin(a1);
            float x2 = radius * std::cos(a2), z2 = radius * std::sin(a2);

            Vec3 midNorm = Vec3((x1 + x2) * 0.5f, 0.0f, (z1 + z2) * 0.5f).normalized();

            Vec3 p0(x1, -hh, z1);
            Vec3 p1(x2, -hh, z2);
            Vec3 p2(x2,  hh, z2);
            Vec3 p3(x1,  hh, z1);

            unsigned int base = verts.size();
            verts.emplace_back(p0, midNorm, color);
            verts.emplace_back(p1, midNorm, color);
            verts.emplace_back(p2, midNorm, color);
            verts.emplace_back(p3, midNorm, color);

            indices.push_back(base);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }

        // Đáy trên (+Y)
        unsigned int topCenter = verts.size();
        verts.emplace_back(Vec3(0, hh, 0), Vec3(0, 1, 0), color * 1.15f);
        for (int i = 0; i <= sides; ++i) {
            float a = i * angleStep;
            verts.emplace_back(Vec3(radius * std::cos(a), hh, radius * std::sin(a)), Vec3(0, 1, 0), color * 1.15f);
        }
        for (int i = 1; i <= sides; ++i) {
            indices.push_back(topCenter);
            indices.push_back(topCenter + i);
            indices.push_back(topCenter + i + 1);
        }

        // Đáy dưới (-Y)
        unsigned int botCenter = verts.size();
        verts.emplace_back(Vec3(0, -hh, 0), Vec3(0, -1, 0), color * 0.85f);
        for (int i = 0; i <= sides; ++i) {
            float a = i * angleStep;
            verts.emplace_back(Vec3(radius * std::cos(a), -hh, radius * std::sin(a)), Vec3(0, -1, 0), color * 0.85f);
        }
        for (int i = 1; i <= sides; ++i) {
            indices.push_back(botCenter);
            indices.push_back(botCenter + i + 1);
            indices.push_back(botCenter + i);
        }

        Mesh3D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "3D Prism");
        return mesh;
    }

    // 4. Hình cầu 3D (UV Sphere - Tọa độ cầu & Pháp tuyến trơn)
    static Mesh3D createUVSphere(float radius = 0.7f, int sectors = 36, int stacks = 20, const Vec4& color = Vec4(0.2f, 0.7f, 0.95f, 1.0f)) {
        std::vector<Vertex3D> verts;
        std::vector<unsigned int> indices;

        for (int i = 0; i <= stacks; ++i) {
            float v = (float)i / stacks;
            float phi = PI * 0.5f - v * PI; // từ +PI/2 đến -PI/2
            float y = radius * std::sin(phi);
            float rCosPhi = radius * std::cos(phi);

            for (int j = 0; j <= sectors; ++j) {
                float u = (float)j / sectors;
                float theta = u * 2.0f * PI; // từ 0 đến 2*PI

                float x = rCosPhi * std::cos(theta);
                float z = rCosPhi * std::sin(theta);

                Vec3 pos(x, y, z);
                Vec3 norm = pos.normalized();
                verts.emplace_back(pos, norm, Vec2(u, v), color);
            }
        }

        for (int i = 0; i < stacks; ++i) {
            int k1 = i * (sectors + 1);
            int k2 = k1 + sectors + 1;

            for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
                if (i != 0) {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }
                if (i != (stacks - 1)) {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }

        Mesh3D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "3D UV Sphere");
        return mesh;
    }

    // 5. Hình trụ 3D (3D Cylinder - Thân trụ + 2 Nắp)
    static Mesh3D createCylinder(float radius = 0.5f, float height = 1.2f, int segments = 36, const Vec4& color = Vec4(0.95f, 0.45f, 0.2f, 1.0f)) {
        std::vector<Vertex3D> verts;
        std::vector<unsigned int> indices;
        float hh = height * 0.5f;
        float angleStep = 2.0f * PI / segments;

        // Thân trụ (Cylinder side surface)
        for (int i = 0; i <= segments; ++i) {
            float angle = i * angleStep;
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);

            float x = radius * cosA;
            float z = radius * sinA;
            Vec3 norm(cosA, 0.0f, sinA);

            float u = (float)i / segments;
            verts.emplace_back(Vec3(x,  hh, z), norm, Vec2(u, 1.0f), color);
            verts.emplace_back(Vec3(x, -hh, z), norm, Vec2(u, 0.0f), color);
        }

        for (int i = 0; i < segments; ++i) {
            int idx = i * 2;
            indices.push_back(idx);
            indices.push_back(idx + 1);
            indices.push_back(idx + 2);

            indices.push_back(idx + 2);
            indices.push_back(idx + 1);
            indices.push_back(idx + 3);
        }

        // Nắp trên (+Y)
        unsigned int topStart = verts.size();
        verts.emplace_back(Vec3(0, hh, 0), Vec3(0, 1, 0), Vec2(0.5f, 0.5f), color * 1.1f);
        for (int i = 0; i <= segments; ++i) {
            float a = i * angleStep;
            float x = radius * std::cos(a);
            float z = radius * std::sin(a);
            verts.emplace_back(Vec3(x, hh, z), Vec3(0, 1, 0), Vec2(x/radius*0.5f+0.5f, z/radius*0.5f+0.5f), color * 1.1f);
        }
        for (int i = 1; i <= segments; ++i) {
            indices.push_back(topStart);
            indices.push_back(topStart + i);
            indices.push_back(topStart + i + 1);
        }

        // Nắp dưới (-Y)
        unsigned int botStart = verts.size();
        verts.emplace_back(Vec3(0, -hh, 0), Vec3(0, -1, 0), Vec2(0.5f, 0.5f), color * 0.9f);
        for (int i = 0; i <= segments; ++i) {
            float a = i * angleStep;
            float x = radius * std::cos(a);
            float z = radius * std::sin(a);
            verts.emplace_back(Vec3(x, -hh, z), Vec3(0, -1, 0), Vec2(x/radius*0.5f+0.5f, z/radius*0.5f+0.5f), color * 0.9f);
        }
        for (int i = 1; i <= segments; ++i) {
            indices.push_back(botStart);
            indices.push_back(botStart + i + 1);
            indices.push_back(botStart + i);
        }

        Mesh3D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "3D Cylinder");
        return mesh;
    }

    // 6. Hình nón 3D (3D Cone)
    static Mesh3D createCone(float radius = 0.6f, float height = 1.3f, int segments = 36, const Vec4& color = Vec4(0.35f, 0.85f, 0.35f, 1.0f)) {
        std::vector<Vertex3D> verts;
        std::vector<unsigned int> indices;
        float hh = height * 0.5f;
        float angleStep = 2.0f * PI / segments;
        float slantLength = std::sqrt(radius * radius + height * height);

        // Thân nón dốc
        Vec3 apex(0.0f, hh, 0.0f);
        for (int i = 0; i < segments; ++i) {
            float a1 = i * angleStep;
            float a2 = (i + 1) * angleStep;

            float x1 = radius * std::cos(a1), z1 = radius * std::sin(a1);
            float x2 = radius * std::cos(a2), z2 = radius * std::sin(a2);

            Vec3 p0 = apex;
            Vec3 p1(x1, -hh, z1);
            Vec3 p2(x2, -hh, z2);

            // Vector pháp tuyến của tam giác mặt bên
            Vec3 n1(std::cos(a1) * (height / slantLength), radius / slantLength, std::sin(a1) * (height / slantLength));
            Vec3 n2(std::cos(a2) * (height / slantLength), radius / slantLength, std::sin(a2) * (height / slantLength));
            Vec3 nMid = (n1 + n2).normalized();

            unsigned int base = verts.size();
            verts.emplace_back(p0, nMid, color);
            verts.emplace_back(p1, n1, color);
            verts.emplace_back(p2, n2, color);

            indices.push_back(base);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
        }

        // Đáy tròn (-Y)
        unsigned int botStart = verts.size();
        verts.emplace_back(Vec3(0, -hh, 0), Vec3(0, -1, 0), color * 0.85f);
        for (int i = 0; i <= segments; ++i) {
            float a = i * angleStep;
            verts.emplace_back(Vec3(radius * std::cos(a), -hh, radius * std::sin(a)), Vec3(0, -1, 0), color * 0.85f);
        }
        for (int i = 1; i <= segments; ++i) {
            indices.push_back(botStart);
            indices.push_back(botStart + i + 1);
            indices.push_back(botStart + i);
        }

        Mesh3D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "3D Cone");
        return mesh;
    }

    // 7. Hình xuyến 3D (3D Torus / Donut)
    static Mesh3D createTorus(float mainRadius = 0.6f, float tubeRadius = 0.22f, int mainSegs = 36, int tubeSegs = 20, const Vec4& color = Vec4(0.98f, 0.65f, 0.15f, 1.0f)) {
        std::vector<Vertex3D> verts;
        std::vector<unsigned int> indices;

        for (int i = 0; i <= mainSegs; ++i) {
            float u = (float)i / mainSegs;
            float phi = u * 2.0f * PI;
            float cosPhi = std::cos(phi);
            float sinPhi = std::sin(phi);

            for (int j = 0; j <= tubeSegs; ++j) {
                float v = (float)j / tubeSegs;
                float theta = v * 2.0f * PI;
                float cosTheta = std::cos(theta);
                float sinTheta = std::sin(theta);

                // Tọa độ tham số Torus
                float x = (mainRadius + tubeRadius * cosTheta) * cosPhi;
                float y = tubeRadius * sinTheta;
                float z = (mainRadius + tubeRadius * cosTheta) * sinPhi;

                // Vector pháp tuyến chuẩn xác
                Vec3 norm(cosTheta * cosPhi, sinTheta, cosTheta * sinPhi);

                verts.emplace_back(Vec3(x, y, z), norm.normalized(), Vec2(u, v), color);
            }
        }

        for (int i = 0; i < mainSegs; ++i) {
            for (int j = 0; j < tubeSegs; ++j) {
                int first = (i * (tubeSegs + 1)) + j;
                int second = first + tubeSegs + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        Mesh3D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "3D Torus");
        return mesh;
    }

    // 8. Mặt phẳng 3D (3D Ground / Terrain Plane)
    static Mesh3D createPlane(float width = 20.0f, float depth = 20.0f, const Vec4& color = Vec4(0.35f, 0.65f, 0.35f, 1.0f)) {
        float hw = width * 0.5f;
        float hd = depth * 0.5f;
        std::vector<Vertex3D> verts = {
            Vertex3D(Vec3(-hw, 0.0f, -hd), Vec3(0, 1, 0), Vec2(0, 0), color),
            Vertex3D(Vec3( hw, 0.0f, -hd), Vec3(0, 1, 0), Vec2(10, 0), color),
            Vertex3D(Vec3( hw, 0.0f,  hd), Vec3(0, 1, 0), Vec2(10, 10), color),
            Vertex3D(Vec3(-hw, 0.0f,  hd), Vec3(0, 1, 0), Vec2(0, 10), color)
        };
        std::vector<unsigned int> indices = {0, 2, 1, 0, 3, 2};
        Mesh3D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "3D Ground Plane");
        return mesh;
    }

    // 9. Quả cầu Nguồn sáng phát quang (Light Bulb / Sun Orb)
    static Mesh3D createLightOrb(float radius = 0.15f, const Vec4& color = Vec4(1.0f, 0.95f, 0.4f, 1.0f)) {
        return createUVSphere(radius, 16, 12, color);
    }
};

} // namespace Lab3D

#endif // PRIMITIVES_3D_HPP
