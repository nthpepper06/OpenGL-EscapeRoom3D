#ifndef PRIMITIVES_2D_HPP
#define PRIMITIVES_2D_HPP

#include <glad/glad.h>
#include <vector>
#include <cmath>
#include <string>
#include "../Math3D.hpp"

namespace Lab2D {

struct Vertex2D {
    Vec3 pos;       // Vị trí (x, y, z=0)
    Vec3 normal;    // Pháp tuyến (0, 0, 1)
    Vec2 texCoord;  // Tọa độ Texture (u, v)
    Vec4 color;     // Màu sắc RGBA (r, g, b, a)

    Vertex2D() = default;
    Vertex2D(const Vec3& p, const Vec4& c, const Vec3& n = Vec3(0, 0, 1), const Vec2& uv = Vec2(0, 0))
        : pos(p), normal(n), texCoord(uv), color(c) {}
    Vertex2D(float x, float y, const Vec4& c)
        : pos(x, y, 0.0f), normal(0, 0, 1), texCoord(0, 0), color(c) {}
};

struct Mesh2D {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLenum drawMode = GL_TRIANGLES;
    size_t indexCount = 0;
    size_t vertexCount = 0;
    std::string name;

    void upload(const std::vector<Vertex2D>& vertices, const std::vector<unsigned int>& indices, GLenum mode = GL_TRIANGLES, const std::string& meshName = "") {
        name = meshName;
        drawMode = mode;
        vertexCount = vertices.size();
        indexCount = indices.size();

        if (vao == 0) glGenVertexArrays(1, &vao);
        if (vbo == 0) glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex2D), vertices.data(), GL_STATIC_DRAW);

        if (!indices.empty()) {
            if (ebo == 0) glGenBuffers(1, &ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        }

        // Layout 0: Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, pos));

        // Layout 1: Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, normal));

        // Layout 2: TexCoord
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, texCoord));

        // Layout 3: Color
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, color));

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

class Shape2DGenerator {
public:
    // 1. Điểm & Các đường thẳng (Points & Lines)
    static Mesh2D createPointGrid(float width, float height, int countX, int countY, const Vec4& color) {
        std::vector<Vertex2D> verts;
        float stepX = width / (countX - 1);
        float stepY = height / (countY - 1);
        for (int i = 0; i < countX; ++i) {
            for (int j = 0; j < countY; ++j) {
                float x = -width * 0.5f + i * stepX;
                float y = -height * 0.5f + j * stepY;
                verts.emplace_back(x, y, color);
            }
        }
        Mesh2D mesh;
        mesh.upload(verts, {}, GL_POINTS, "Point Grid");
        return mesh;
    }

    static Mesh2D createCoordinateAxes(float length = 1.0f) {
        std::vector<Vertex2D> verts = {
            // Trục X (Đỏ)
            Vertex2D(Vec3(-length, 0, 0), Vec4(1.0f, 0.2f, 0.2f, 1.0f)),
            Vertex2D(Vec3( length, 0, 0), Vec4(1.0f, 0.2f, 0.2f, 1.0f)),
            // Trục Y (Xanh lá)
            Vertex2D(Vec3(0, -length, 0), Vec4(0.2f, 1.0f, 0.2f, 1.0f)),
            Vertex2D(Vec3(0,  length, 0), Vec4(0.2f, 1.0f, 0.2f, 1.0f))
        };
        Mesh2D mesh;
        mesh.upload(verts, {}, GL_LINES, "Coordinate Axes");
        return mesh;
    }

    // 2. Tam giác Đa sắc (Color Gradient Triangle)
    static Mesh2D createGradientTriangle(float size = 1.0f) {
        float h = size * std::sqrt(3.0f) * 0.5f;
        std::vector<Vertex2D> verts = {
            Vertex2D(Vec3( 0.0f,       h * 0.6f, 0.0f), Vec4(1.0f, 0.15f, 0.15f, 1.0f)), // Đỉnh trên: Đỏ
            Vertex2D(Vec3(-size * 0.5f, -h * 0.4f, 0.0f), Vec4(0.15f, 1.0f, 0.15f, 1.0f)), // Đáy trái: Xanh lá
            Vertex2D(Vec3( size * 0.5f, -h * 0.4f, 0.0f), Vec4(0.15f, 0.4f, 1.0f, 1.0f))  // Đáy phải: Xanh dương
        };
        std::vector<unsigned int> indices = {0, 1, 2};
        Mesh2D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "Gradient Triangle");
        return mesh;
    }

    // 3. Hình chữ nhật / Tứ giác (Rectangle / Quad)
    static Mesh2D createRectangle(float width, float height, const Vec4& color) {
        float hw = width * 0.5f;
        float hh = height * 0.5f;
        std::vector<Vertex2D> verts = {
            Vertex2D(Vec3(-hw, -hh, 0.0f), color, Vec3(0,0,1), Vec2(0, 0)),
            Vertex2D(Vec3( hw, -hh, 0.0f), color, Vec3(0,0,1), Vec2(1, 0)),
            Vertex2D(Vec3( hw,  hh, 0.0f), color, Vec3(0,0,1), Vec2(1, 1)),
            Vertex2D(Vec3(-hw,  hh, 0.0f), color, Vec3(0,0,1), Vec2(0, 1))
        };
        std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};
        Mesh2D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "Rectangle");
        return mesh;
    }

    // 4. Hình chữ nhật Gradient 4 góc
    static Mesh2D createGradientQuad(float width, float height, const Vec4& c1, const Vec4& c2, const Vec4& c3, const Vec4& c4) {
        float hw = width * 0.5f;
        float hh = height * 0.5f;
        std::vector<Vertex2D> verts = {
            Vertex2D(Vec3(-hw, -hh, 0.0f), c1),
            Vertex2D(Vec3( hw, -hh, 0.0f), c2),
            Vertex2D(Vec3( hw,  hh, 0.0f), c3),
            Vertex2D(Vec3(-hw,  hh, 0.0f), c4)
        };
        std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};
        Mesh2D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "Gradient Quad");
        return mesh;
    }

    // 5. Hình tròn 2D (Circle Fan / Triangles)
    static Mesh2D createCircle(float radius, int segments, const Vec4& centerColor, const Vec4& edgeColor) {
        std::vector<Vertex2D> verts;
        std::vector<unsigned int> indices;

        // Tâm hình tròn (đỉnh 0)
        verts.emplace_back(Vec3(0.0f, 0.0f, 0.0f), centerColor, Vec3(0,0,1), Vec2(0.5f, 0.5f));

        float angleStep = 2.0f * PI / segments;
        for (int i = 0; i <= segments; ++i) {
            float angle = i * angleStep;
            float x = radius * std::cos(angle);
            float y = radius * std::sin(angle);
            float u = (x / radius + 1.0f) * 0.5f;
            float v = (y / radius + 1.0f) * 0.5f;
            verts.emplace_back(Vec3(x, y, 0.0f), edgeColor, Vec3(0,0,1), Vec2(u, v));
        }

        for (int i = 1; i <= segments; ++i) {
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }

        Mesh2D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "2D Circle");
        return mesh;
    }

    // 6. Đa giác đều n-cạnh (Regular Polygon: Ngũ giác, Lục giác, Bát giác)
    static Mesh2D createRegularPolygon(int sides, float radius, const Vec4& color) {
        std::vector<Vertex2D> verts;
        std::vector<unsigned int> indices;

        verts.emplace_back(Vec3(0.0f, 0.0f, 0.0f), color); // Tâm

        float angleStep = 2.0f * PI / sides;
        for (int i = 0; i <= sides; ++i) {
            float angle = i * angleStep + PI * 0.5f; // Đỉnh hướng lên trên
            float x = radius * std::cos(angle);
            float y = radius * std::sin(angle);
            verts.emplace_back(Vec3(x, y, 0.0f), color);
        }

        for (int i = 1; i <= sides; ++i) {
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }

        Mesh2D mesh;
        std::string name = "Polygon (" + std::to_string(sides) + " sides)";
        mesh.upload(verts, indices, GL_TRIANGLES, name);
        return mesh;
    }

    // 7. Ngôi sao 5 cánh (5-pointed Star)
    static Mesh2D createStar(float outerRadius, float innerRadius, const Vec4& outerColor, const Vec4& innerColor) {
        std::vector<Vertex2D> verts;
        std::vector<unsigned int> indices;

        // Tâm ngôi sao
        verts.emplace_back(Vec3(0.0f, 0.0f, 0.0f), innerColor);

        int numPoints = 5;
        int totalVertices = numPoints * 2;
        float angleStep = PI / numPoints; // 36 độ mỗi bước

        for (int i = 0; i <= totalVertices; ++i) {
            float angle = i * angleStep + PI * 0.5f;
            float r = (i % 2 == 0) ? outerRadius : innerRadius;
            Vec4 c = (i % 2 == 0) ? outerColor : innerColor;
            float x = r * std::cos(angle);
            float y = r * std::sin(angle);
            verts.emplace_back(Vec3(x, y, 0.0f), c);
        }

        for (int i = 1; i <= totalVertices; ++i) {
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }

        Mesh2D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "5-Pointed Star");
        return mesh;
    }

    // 8. Cánh quạt cối xay gió 2D (4 Windmill Blades)
    static Mesh2D createWindmillBlades(float length = 0.8f, float width = 0.12f, const Vec4& color = Vec4(0.95f, 0.25f, 0.25f, 1.0f)) {
        std::vector<Vertex2D> verts;
        std::vector<unsigned int> indices;

        verts.emplace_back(Vec3(0.0f, 0.0f, 0.0f), Vec4(1.0f, 0.9f, 0.2f, 1.0f)); // Trục quay vàng

        // 4 cánh quạt cách nhau 90 độ
        for (int b = 0; b < 4; ++b) {
            float baseAngle = b * (PI * 0.5f);
            float cosA = std::cos(baseAngle);
            float sinA = std::sin(baseAngle);
            float perpCos = -sinA;
            float perpSin = cosA;

            Vec3 p0(0, 0, 0);
            Vec3 p1 = Vec3(cosA * length + perpCos * width * 0.5f, sinA * length + perpSin * width * 0.5f, 0.0f);
            Vec3 p2 = Vec3(cosA * length - perpCos * width * 0.5f, sinA * length - perpSin * width * 0.5f, 0.0f);

            unsigned int startIdx = verts.size();
            verts.emplace_back(p0, color);
            verts.emplace_back(p1, color);
            verts.emplace_back(p2, color);

            indices.push_back(startIdx);
            indices.push_back(startIdx + 1);
            indices.push_back(startIdx + 2);
        }

        Mesh2D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "Windmill Blades");
        return mesh;
    }

    // 9. Thân cối xay gió (Hình thang cân 2D)
    static Mesh2D createWindmillTower(float bottomWidth = 0.4f, float topWidth = 0.18f, float height = 1.0f, const Vec4& color = Vec4(0.85f, 0.85f, 0.88f, 1.0f)) {
        float hbw = bottomWidth * 0.5f;
        float htw = topWidth * 0.5f;
        std::vector<Vertex2D> verts = {
            Vertex2D(Vec3(-hbw, 0.0f, 0.0f), color),
            Vertex2D(Vec3( hbw, 0.0f, 0.0f), color),
            Vertex2D(Vec3( htw, height, 0.0f), color * 0.9f),
            Vertex2D(Vec3(-htw, height, 0.0f), color * 0.9f)
        };
        std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};
        Mesh2D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "Windmill Tower");
        return mesh;
    }

    // 10. Ngôi nhà 2D Phức hợp (2D Composite House)
    static Mesh2D create2DHouse() {
        std::vector<Vertex2D> verts;
        std::vector<unsigned int> indices;

        auto addQuad = [&](float x, float y, float w, float h, const Vec4& col) {
            unsigned int base = verts.size();
            verts.emplace_back(Vec3(x, y, 0.0f), col);
            verts.emplace_back(Vec3(x + w, y, 0.0f), col);
            verts.emplace_back(Vec3(x + w, y + h, 0.0f), col);
            verts.emplace_back(Vec3(x, y + h, 0.0f), col);
            indices.push_back(base);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        };

        auto addTriangle = [&](Vec3 p1, Vec3 p2, Vec3 p3, const Vec4& col) {
            unsigned int base = verts.size();
            verts.emplace_back(p1, col);
            verts.emplace_back(p2, col);
            verts.emplace_back(p3, col);
            indices.push_back(base);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
        };

        // 1. Ống khói (Xám)
        addQuad(0.18f, 0.35f, 0.12f, 0.35f, Vec4(0.45f, 0.45f, 0.48f, 1.0f));

        // 2. Thân nhà (Vàng cam ấm)
        addQuad(-0.5f, -0.5f, 1.0f, 0.65f, Vec4(0.96f, 0.76f, 0.38f, 1.0f));

        // 3. Mái nhà dốc (Đỏ tươi)
        addTriangle(Vec3(-0.62f, 0.15f, 0.0f), Vec3(0.62f, 0.15f, 0.0f), Vec3(0.0f, 0.65f, 0.0f), Vec4(0.88f, 0.22f, 0.18f, 1.0f));

        // 4. Cửa chính (Nâu gỗ)
        addQuad(-0.15f, -0.5f, 0.3f, 0.42f, Vec4(0.45f, 0.24f, 0.12f, 1.0f));

        // 5. Cửa sổ bên trái (Xanh dương phản chiếu kính)
        addQuad(-0.42f, -0.15f, 0.2f, 0.2f, Vec4(0.35f, 0.72f, 0.95f, 1.0f));

        // 6. Cửa sổ bên phải
        addQuad(0.22f, -0.15f, 0.2f, 0.2f, Vec4(0.35f, 0.72f, 0.95f, 1.0f));

        // 7. Khung cửa sổ chữ thập (Trắng)
        addQuad(-0.33f, -0.15f, 0.02f, 0.2f, Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        addQuad(-0.42f, -0.06f, 0.2f, 0.02f, Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        addQuad(0.31f, -0.15f, 0.02f, 0.2f, Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        addQuad(0.22f, -0.06f, 0.2f, 0.02f, Vec4(1.0f, 1.0f, 1.0f, 1.0f));

        Mesh2D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "2D House Model");
        return mesh;
    }

    // 11. Mặt trời 2D với các tia sáng (Sun with rays)
    static Mesh2D createSun(float centerRadius = 0.3f, float rayLength = 0.5f, int numRays = 12) {
        std::vector<Vertex2D> verts;
        std::vector<unsigned int> indices;

        // Đĩa tròn mặt trời
        verts.emplace_back(Vec3(0, 0, 0), Vec4(1.0f, 0.95f, 0.3f, 1.0f));
        int circleSegs = 32;
        float angleStep = 2.0f * PI / circleSegs;
        for (int i = 0; i <= circleSegs; ++i) {
            float a = i * angleStep;
            verts.emplace_back(Vec3(centerRadius * std::cos(a), centerRadius * std::sin(a), 0.0f), Vec4(1.0f, 0.6f, 0.05f, 1.0f));
        }
        for (int i = 1; i <= circleSegs; ++i) {
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }

        // Các tia nắng tam giác
        float rayStep = 2.0f * PI / numRays;
        float halfRayWidth = (2.0f * PI * centerRadius / numRays) * 0.35f;

        for (int r = 0; r < numRays; ++r) {
            float midAngle = r * rayStep;
            float cosM = std::cos(midAngle);
            float sinM = std::sin(midAngle);
            float perpCos = -sinM;
            float perpSin = cosM;

            Vec3 tip(cosM * rayLength, sinM * rayLength, 0.0f);
            Vec3 b1(cosM * centerRadius + perpCos * halfRayWidth, sinM * centerRadius + perpSin * halfRayWidth, 0.0f);
            Vec3 b2(cosM * centerRadius - perpCos * halfRayWidth, sinM * centerRadius - perpSin * halfRayWidth, 0.0f);

            unsigned int base = verts.size();
            verts.emplace_back(tip, Vec4(1.0f, 0.35f, 0.0f, 0.85f));
            verts.emplace_back(b1,  Vec4(1.0f, 0.85f, 0.1f, 1.0f));
            verts.emplace_back(b2,  Vec4(1.0f, 0.85f, 0.1f, 1.0f));

            indices.push_back(base);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
        }

        Mesh2D mesh;
        mesh.upload(verts, indices, GL_TRIANGLES, "2D Radiant Sun");
        return mesh;
    }
};

} // namespace Lab2D

#endif // PRIMITIVES_2D_HPP
