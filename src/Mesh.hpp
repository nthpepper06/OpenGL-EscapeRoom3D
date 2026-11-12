#ifndef MESH_HPP
#define MESH_HPP

#include <glad/glad.h>
#include <vector>
#include <cmath>
#include "Math3D.hpp"
#include "World/IslandLayout.hpp"

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 texCoord;
    Vec4 color;

    Vertex() : position(), normal(0, 1, 0), texCoord(), color(1, 1, 1, 1) {}
    Vertex(const Vec3& p, const Vec3& n, const Vec2& uv, const Vec4& c = Vec4(1,1,1,1))
        : position(p), normal(n), texCoord(uv), color(c) {}
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint vao = 0, vbo = 0, ebo = 0;

    Mesh() = default;

    Mesh(const std::vector<Vertex>& verts, const std::vector<unsigned int>& inds)
        : vertices(verts), indices(inds) {
        setup();
    }

    void setup() {
        if (vao != 0) {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
            glDeleteBuffers(1, &ebo);
        }

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

        glBindVertexArray(0);
    }

    void draw(GLenum mode = GL_TRIANGLES) const {
        if (vao == 0) return;
        glBindVertexArray(vao);
        glDrawElements(mode, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

namespace MeshGen {

inline Mesh createTerrainMesh(float width = 1000.0f, float depth = 1000.0f, int gridX = 100, int gridZ = 100) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float halfW = width * 0.5f;
    float halfD = depth * 0.5f;
    float eps = 1.0f;

    for (int z = 0; z <= gridZ; ++z) {
        float fracZ = (float)z / gridZ;
        float posZ = -halfD + fracZ * depth;
        for (int x = 0; x <= gridX; ++x) {
            float fracX = (float)x / gridX;
            float posX = -halfW + fracX * width;
            float posY = World::getTerrainHeight(posX, posZ);

            float hL = World::getTerrainHeight(posX - eps, posZ);
            float hR = World::getTerrainHeight(posX + eps, posZ);
            float hD = World::getTerrainHeight(posX, posZ - eps);
            float hU = World::getTerrainHeight(posX, posZ + eps);

            Vec3 normal(-(hR - hL) / (2.0f * eps), 1.0f, -(hU - hD) / (2.0f * eps));
            normal = normal.normalized();

            Vec4 col;
            if (posY <= 0.6f) {
                // Sandy beach
                col = Vec4(0.82f, 0.78f, 0.55f, 1.0f);
            } else if (posY <= 15.0f) {
                // Grassland
                float gMix = (posY - 0.6f) / 14.4f;
                col = Vec4(0.26f + gMix * 0.05f, 0.55f - gMix * 0.06f, 0.22f + gMix * 0.04f, 1.0f);
            } else if (posY <= 22.0f) {
                // Mountain rock slope
                col = Vec4(0.52f, 0.48f, 0.44f, 1.0f);
            } else {
                // Peak summit granite
                col = Vec4(0.72f, 0.72f, 0.76f, 1.0f);
            }

            vertices.push_back(Vertex(
                Vec3(posX, posY, posZ),
                normal,
                Vec2(fracX * 24.0f, fracZ * 24.0f),
                col
            ));
        }
    }

    for (int z = 0; z < gridZ; ++z) {
        for (int x = 0; x < gridX; ++x) {
            int row1 = z * (gridX + 1);
            int row2 = (z + 1) * (gridX + 1);
            indices.push_back(row1 + x);
            indices.push_back(row2 + x);
            indices.push_back(row1 + x + 1);

            indices.push_back(row1 + x + 1);
            indices.push_back(row2 + x);
            indices.push_back(row2 + x + 1);
        }
    }

    return Mesh(vertices, indices);
}

inline Mesh createPlane(float width, float depth, int gridX = 1, int gridZ = 1, Vec4 color = Vec4(1,1,1,1)) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float halfW = width * 0.5f;
    float halfD = depth * 0.5f;

    for (int z = 0; z <= gridZ; ++z) {
        float fracZ = (float)z / gridZ;
        float posZ = -halfD + fracZ * depth;
        for (int x = 0; x <= gridX; ++x) {
            float fracX = (float)x / gridX;
            float posX = -halfW + fracX * width;
            vertices.push_back(Vertex(
                Vec3(posX, 0.0f, posZ),
                Vec3(0.0f, 1.0f, 0.0f),
                Vec2(fracX * 16.0f, fracZ * 16.0f),
                color
            ));
        }
    }

    for (int z = 0; z < gridZ; ++z) {
        for (int x = 0; x < gridX; ++x) {
            int row1 = z * (gridX + 1);
            int row2 = (z + 1) * (gridX + 1);
            indices.push_back(row1 + x);
            indices.push_back(row2 + x);
            indices.push_back(row1 + x + 1);

            indices.push_back(row1 + x + 1);
            indices.push_back(row2 + x);
            indices.push_back(row2 + x + 1);
        }
    }

    return Mesh(vertices, indices);
}

inline Mesh createCube(float width, float height, float depth, Vec4 color = Vec4(1,1,1,1)) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float hw = width * 0.5f, hh = height * 0.5f, hd = depth * 0.5f;

    struct Face {
        Vec3 n;
        Vec3 v[4];
    } faces[6] = {
        { Vec3(0,0,1),  {Vec3(-hw,-hh, hd), Vec3( hw,-hh, hd), Vec3( hw, hh, hd), Vec3(-hw, hh, hd)} },
        { Vec3(0,0,-1), {Vec3( hw,-hh,-hd), Vec3(-hw,-hh,-hd), Vec3(-hw, hh,-hd), Vec3( hw, hh,-hd)} },
        { Vec3(0,1,0),  {Vec3(-hw, hh, hd), Vec3( hw, hh, hd), Vec3( hw, hh,-hd), Vec3(-hw, hh,-hd)} },
        { Vec3(0,-1,0), {Vec3(-hw,-hh,-hd), Vec3( hw,-hh,-hd), Vec3( hw,-hh, hd), Vec3(-hw, hh, hd)} },
        { Vec3(1,0,0),  {Vec3( hw,-hh, hd), Vec3( hw,-hh,-hd), Vec3( hw, hh,-hd), Vec3( hw, hh, hd)} },
        { Vec3(-1,0,0), {Vec3(-hw,-hh,-hd), Vec3(-hw,-hh, hd), Vec3(-hw, hh, hd), Vec3(-hw, hh,-hd)} },
    };

    for (int f = 0; f < 6; ++f) {
        unsigned int base = (unsigned int)vertices.size();
        vertices.push_back(Vertex(faces[f].v[0], faces[f].n, Vec2(0,0), color));
        vertices.push_back(Vertex(faces[f].v[1], faces[f].n, Vec2(1,0), color));
        vertices.push_back(Vertex(faces[f].v[2], faces[f].n, Vec2(1,1), color));
        vertices.push_back(Vertex(faces[f].v[3], faces[f].n, Vec2(0,1), color));

        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    return Mesh(vertices, indices);
}

inline Mesh createCylinder(float radiusTop, float radiusBottom, float height, int segments = 16, Vec4 color = Vec4(1,1,1,1)) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float halfH = height * 0.5f;

    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i / segments * 2.0f * PI;
        float c = std::cos(angle);
        float s = std::sin(angle);

        Vec3 n(c, 0, s);
        vertices.push_back(Vertex(Vec3(c * radiusTop, halfH, s * radiusTop), n, Vec2((float)i / segments, 1.0f), color));
        vertices.push_back(Vertex(Vec3(c * radiusBottom, -halfH, s * radiusBottom), n, Vec2((float)i / segments, 0.0f), color));
    }

    for (int i = 0; i < segments; ++i) {
        int idx = i * 2;
        indices.push_back(idx);
        indices.push_back(idx + 1);
        indices.push_back(idx + 2);

        indices.push_back(idx + 1);
        indices.push_back(idx + 3);
        indices.push_back(idx + 2);
    }

    return Mesh(vertices, indices);
}

inline Mesh createSphere(float radius, int rings = 14, int sectors = 20, Vec4 color = Vec4(1,1,1,1)) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float const R = 1.0f / (float)(rings - 1);
    float const S = 1.0f / (float)(sectors - 1);

    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < sectors; ++s) {
            float const y = std::sin(-PI / 2.0f + PI * r * R);
            float const x = std::cos(2.0f * PI * s * S) * std::sin(PI * r * R);
            float const z = std::sin(2.0f * PI * s * S) * std::sin(PI * r * R);

            Vec3 norm(x, y, z);
            Vec3 pos = norm * radius;
            Vec2 uv(s * S, r * R);

            vertices.push_back(Vertex(pos, norm.normalized(), uv, color));
        }
    }

    for (int r = 0; r < rings - 1; ++r) {
        for (int s = 0; s < sectors - 1; ++s) {
            indices.push_back(r * sectors + s);
            indices.push_back((r + 1) * sectors + s);
            indices.push_back((r + 1) * sectors + (s + 1));

            indices.push_back(r * sectors + s);
            indices.push_back((r + 1) * sectors + (s + 1));
            indices.push_back(r * sectors + (s + 1));
        }
    }

    return Mesh(vertices, indices);
}

// 3D Thick Solid Gloo Wall (Solid Ice Bunker)
inline Mesh createGlooWall(float radius = 3.6f, float height = 3.0f, float thickness = 0.55f, float arcAngle = 110.0f * DEG2RAD, int segments = 16) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float halfArc = arcAngle * 0.5f;
    float rOut = radius;
    float rIn = radius - thickness;

    auto addQuad = [&](Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, Vec3 n, Vec4 col) {
        unsigned int b = (unsigned int)vertices.size();
        vertices.push_back(Vertex(p0, n, Vec2(0,0), col));
        vertices.push_back(Vertex(p1, n, Vec2(1,0), col));
        vertices.push_back(Vertex(p2, n, Vec2(1,1), col));
        vertices.push_back(Vertex(p3, n, Vec2(0,1), col));
        indices.push_back(b); indices.push_back(b+1); indices.push_back(b+2);
        indices.push_back(b); indices.push_back(b+2); indices.push_back(b+3);
    };

    Vec4 iceCol(0.45f, 0.85f, 1.0f, 0.95f);

    for (int i = 0; i < segments; ++i) {
        float a0 = -halfArc + (float)i / segments * arcAngle;
        float a1 = -halfArc + (float)(i + 1) / segments * arcAngle;

        float sin0 = std::sin(a0), cos0 = std::cos(a0);
        float sin1 = std::sin(a1), cos1 = std::cos(a1);

        // Outer arc vertices (Front convex face, facing -Z toward enemy)
        Vec3 o0_bot(sin0 * rOut, 0, -cos0 * rOut + rOut);
        Vec3 o0_top(sin0 * rOut, height, -cos0 * rOut + rOut);
        Vec3 o1_bot(sin1 * rOut, 0, -cos1 * rOut + rOut);
        Vec3 o1_top(sin1 * rOut, height, -cos1 * rOut + rOut);

        // Inner arc vertices (Back concave face, facing +Z toward player)
        Vec3 i0_bot(sin0 * rIn, 0, -cos0 * rIn + rOut);
        Vec3 i0_top(sin0 * rIn, height, -cos0 * rIn + rOut);
        Vec3 i1_bot(sin1 * rIn, 0, -cos1 * rIn + rOut);
        Vec3 i1_top(sin1 * rIn, height, -cos1 * rIn + rOut);

        // Front Face
        Vec3 nOut = Vec3((sin0 + sin1) * 0.5f, 0, -(cos0 + cos1) * 0.5f).normalized();
        addQuad(o0_bot, o1_bot, o1_top, o0_top, nOut, iceCol);

        // Back Face
        Vec3 nIn = -nOut;
        addQuad(i1_bot, i0_bot, i0_top, i1_top, nIn, iceCol);

        // Top Cap
        addQuad(o0_top, o1_top, i1_top, i0_top, Vec3(0, 1, 0), iceCol);

        // Bottom Cap
        addQuad(i0_bot, i1_bot, o1_bot, o0_bot, Vec3(0, -1, 0), iceCol);
    }

    // Left End Cap
    float sSin = std::sin(-halfArc), sCos = std::cos(-halfArc);
    Vec3 oS_bot(sSin * rOut, 0, -sCos * rOut + rOut);
    Vec3 oS_top(sSin * rOut, height, -sCos * rOut + rOut);
    Vec3 iS_bot(sSin * rIn, 0, -sCos * rIn + rOut);
    Vec3 iS_top(sSin * rIn, height, -sCos * rIn + rOut);
    addQuad(oS_bot, oS_top, iS_top, iS_bot, Vec3(-sCos, 0, -sSin), iceCol);

    // Right End Cap
    float eSin = std::sin(halfArc), eCos = std::cos(halfArc);
    Vec3 oE_bot(eSin * rOut, 0, -eCos * rOut + rOut);
    Vec3 oE_top(eSin * rOut, height, -eCos * rOut + rOut);
    Vec3 iE_bot(eSin * rIn, 0, -eCos * rIn + rOut);
    Vec3 iE_top(eSin * rIn, height, -eCos * rIn + rOut);
    addQuad(iE_bot, iE_top, oE_top, oE_bot, Vec3(eCos, 0, eSin), iceCol);

    return Mesh(vertices, indices);
}

inline Mesh createSkyDome(float radius = 280.0f, int rings = 12, int sectors = 24) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int r = 0; r <= rings; ++r) {
        float phi = (float)r / rings * (PI * 0.5f);
        float y = std::cos(phi) * radius;
        float ringR = std::sin(phi) * radius;

        for (int s = 0; s <= sectors; ++s) {
            float theta = (float)s / sectors * 2.0f * PI;
            float x = std::cos(theta) * ringR;
            float z = std::sin(theta) * ringR;

            Vec3 pos(x, y - 8.0f, z);
            Vec3 norm = -pos.normalized();
            vertices.push_back(Vertex(pos, norm, Vec2((float)s/sectors, (float)r/rings), Vec4(1,1,1,1)));
        }
    }

    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < sectors; ++s) {
            int i0 = r * (sectors + 1) + s;
            int i1 = (r + 1) * (sectors + 1) + s;
            int i2 = (r + 1) * (sectors + 1) + (s + 1);
            int i3 = r * (sectors + 1) + (s + 1);

            indices.push_back(i0); indices.push_back(i2); indices.push_back(i1);
            indices.push_back(i0); indices.push_back(i3); indices.push_back(i2);
        }
    }
    return Mesh(vertices, indices);
}

inline Mesh createDropShadow(float radius = 0.8f) {
    return createCube(radius * 2.0f, 0.02f, radius * 2.0f, Vec4(0.02f, 0.04f, 0.02f, 0.55f));
}

// 3D Military Barracks Building with Solid Walls & Clear Doorway
inline Mesh createBuilding(float width = 12.0f, float height = 4.5f, float depth = 10.0f) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 size, Vec4 col) {
        Mesh c = createCube(size.x, size.y, size.z, col);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : c.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, col));
        }
        for (auto i : c.indices) inds.push_back(base + i);
    };

    Vec4 wallCol(0.68f, 0.65f, 0.58f, 1.0f); // Military sand concrete
    Vec4 trimCol(0.28f, 0.42f, 0.32f, 1.0f); // Olive camo trim
    Vec4 roofCol(0.42f, 0.22f, 0.18f, 1.0f); // Tin roof

    float wallT = 0.5f;
    float hw = width * 0.5f, hh = height * 0.5f, hd = depth * 0.5f;

    // Solid Back wall
    addBox(Vec3(0, hh, -hd), Vec3(width, height, wallT), wallCol);
    // Solid Left wall
    addBox(Vec3(-hw, hh, 0), Vec3(wallT, height, depth), wallCol);
    // Solid Right wall
    addBox(Vec3(hw, hh, 0), Vec3(wallT, height, depth), wallCol);

    // Front wall with doorway (Door width: 2.6m)
    float doorW = 2.6f;
    float sideW = (width - doorW) * 0.5f;
    addBox(Vec3(-hw + sideW*0.5f, hh, hd), Vec3(sideW, height, wallT), wallCol);
    addBox(Vec3(hw - sideW*0.5f, hh, hd), Vec3(sideW, height, wallT), wallCol);
    addBox(Vec3(0, height - 0.5f, hd), Vec3(doorW, 1.0f, wallT), trimCol);

    // Floor
    addBox(Vec3(0, 0.1f, 0), Vec3(width, 0.2f, depth), Vec4(0.38f, 0.36f, 0.32f, 1.0f));

    // Overhanging Roof
    addBox(Vec3(0, height + 0.2f, 0), Vec3(width + 1.2f, 0.4f, depth + 1.2f), roofCol);

    return Mesh(verts, inds);
}

// 3D Watchtower with Solid Pillars & Sniper Deck
inline Mesh createWatchtower(float height = 7.5f, float deckSize = 4.2f) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 size, Vec4 col) {
        Mesh c = createCube(size.x, size.y, size.z, col);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : c.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, col));
        }
        for (auto i : c.indices) inds.push_back(base + i);
    };

    Vec4 woodCol(0.45f, 0.30f, 0.18f, 1.0f);
    Vec4 roofCol(0.25f, 0.42f, 0.32f, 1.0f);

    float poleR = 0.35f;
    float hDeck = deckSize * 0.5f - 0.2f;

    // 4 Solid Thick Timber Legs
    addBox(Vec3(-hDeck, height*0.5f, -hDeck), Vec3(poleR, height, poleR), woodCol);
    addBox(Vec3( hDeck, height*0.5f, -hDeck), Vec3(poleR, height, poleR), woodCol);
    addBox(Vec3(-hDeck, height*0.5f,  hDeck), Vec3(poleR, height, poleR), woodCol);
    addBox(Vec3( hDeck, height*0.5f,  hDeck), Vec3(poleR, height, poleR), woodCol);

    // Platform
    addBox(Vec3(0, height, 0), Vec3(deckSize + 0.4f, 0.4f, deckSize + 0.4f), woodCol);

    // Safety Railings
    float railH = 1.15f;
    addBox(Vec3(0, height + railH*0.5f, -hDeck), Vec3(deckSize, railH, 0.15f), woodCol);
    addBox(Vec3(0, height + railH*0.5f,  hDeck), Vec3(deckSize, railH, 0.15f), woodCol);
    addBox(Vec3(-hDeck, height + railH*0.5f, 0), Vec3(0.15f, railH, deckSize), woodCol);
    addBox(Vec3( hDeck, height + railH*0.5f, 0), Vec3(0.15f, railH, deckSize), woodCol);

    // Canopy Roof
    addBox(Vec3(0, height + 3.2f, 0), Vec3(deckSize + 1.4f, 0.3f, deckSize + 1.4f), roofCol);

    return Mesh(verts, inds);
}

// 3D Tree
inline Mesh createTree(float height = 6.5f) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addPart = [&](Mesh& part, Mat4 m, Vec4 col) {
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : part.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, col));
        }
        for (auto i : part.indices) inds.push_back(base + i);
    };

    Mesh trunk = createCylinder(0.4f, 0.65f, height * 0.6f, 10, Vec4(0.42f, 0.28f, 0.14f, 1.0f));
    addPart(trunk, Mat4::translate(Vec3(0, height * 0.3f, 0)), Vec4(0.42f, 0.28f, 0.14f, 1.0f));

    Mesh leaf1 = createSphere(2.4f, 10, 12, Vec4(0.18f, 0.58f, 0.22f, 1.0f));
    addPart(leaf1, Mat4::translate(Vec3(0, height * 0.72f, 0)), Vec4(0.18f, 0.58f, 0.22f, 1.0f));

    Mesh leaf2 = createSphere(1.8f, 10, 12, Vec4(0.24f, 0.68f, 0.28f, 1.0f));
    addPart(leaf2, Mat4::translate(Vec3(0, height * 0.98f, 0)), Vec4(0.24f, 0.68f, 0.28f, 1.0f));

    return Mesh(verts, inds);
}

// 3D Parachute
inline Mesh createParachute(float radius = 4.2f, float height = 2.4f, int segments = 24) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    int rings = 10;
    for (int r = 0; r <= rings; ++r) {
        float phi = (float)r / rings * (PI * 0.48f);
        float y = std::cos(phi) * height;
        float ringR = std::sin(phi) * radius;

        for (int s = 0; s <= segments; ++s) {
            float theta = (float)s / segments * 2.0f * PI;
            float x = std::cos(theta) * ringR;
            float z = std::sin(theta) * ringR;

            Vec3 norm(x, y + 1.0f, z);
            Vec4 col;
            int slice = (s / 3) % 3;
            if (slice == 0) col = Vec4(0.95f, 0.85f, 0.15f, 1.0f);
            else if (slice == 1) col = Vec4(0.12f, 0.12f, 0.15f, 1.0f);
            else col = Vec4(0.95f, 0.2f, 0.2f, 1.0f);

            vertices.push_back(Vertex(Vec3(x, y + 3.2f, z), norm.normalized(), Vec2((float)s/segments, (float)r/rings), col));
        }
    }

    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < segments; ++s) {
            int i0 = r * (segments + 1) + s;
            int i1 = (r + 1) * (segments + 1) + s;
            int i2 = (r + 1) * (segments + 1) + (s + 1);
            int i3 = r * (segments + 1) + (s + 1);

            indices.push_back(i0); indices.push_back(i1); indices.push_back(i2);
            indices.push_back(i0); indices.push_back(i2); indices.push_back(i3);

            indices.push_back(i0); indices.push_back(i2); indices.push_back(i1);
            indices.push_back(i0); indices.push_back(i3); indices.push_back(i2);
        }
    }

    return Mesh(vertices, indices);
}

// 3D Detailed Weapon Mesh (Body, Barrel, Magazine, Scope)
inline Mesh createDetailedGun(int type) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 size, Vec4 col) {
        Mesh c = createCube(size.x, size.y, size.z, col);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : c.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, col));
        }
        for (auto i : c.indices) inds.push_back(base + i);
    };

    if (type == 1) {
        // AK-47 (Golden Flame Skin)
        Vec4 gold(0.95f, 0.75f, 0.15f, 1.0f);
        Vec4 wood(0.48f, 0.25f, 0.12f, 1.0f);
        Vec4 steel(0.18f, 0.18f, 0.22f, 1.0f);

        addBox(Vec3(0, 0, 0), Vec3(0.12f, 0.18f, 0.85f), gold); // Receiver
        addBox(Vec3(0, -0.16f, -0.25f), Vec3(0.1f, 0.22f, 0.14f), wood); // Grip
        addBox(Vec3(0, -0.04f, -0.55f), Vec3(0.1f, 0.22f, 0.45f), wood); // Stock
        addBox(Vec3(0, -0.15f, 0.12f), Vec3(0.08f, 0.32f, 0.18f), steel); // Curved Mag
        addBox(Vec3(0, 0.04f, 0.65f), Vec3(0.06f, 0.06f, 0.55f), steel); // Barrel
    } else if (type == 2) {
        // MP40 (Cyan Neon Skin)
        Vec4 neon(0.2f, 0.85f, 1.0f, 1.0f);
        Vec4 dark(0.12f, 0.12f, 0.15f, 1.0f);

        addBox(Vec3(0, 0, 0), Vec3(0.1f, 0.16f, 0.7f), neon);
        addBox(Vec3(0, -0.22f, 0.08f), Vec3(0.06f, 0.38f, 0.1f), dark); // Long Mag
        addBox(Vec3(0, -0.12f, -0.22f), Vec3(0.08f, 0.2f, 0.12f), dark); // Grip
        addBox(Vec3(0, 0.02f, 0.55f), Vec3(0.05f, 0.05f, 0.45f), neon); // Barrel
    } else if (type == 3) {
        // M1887 (Lava Shotgun Skin)
        Vec4 lava(0.95f, 0.35f, 0.1f, 1.0f);
        Vec4 wood(0.38f, 0.20f, 0.12f, 1.0f);

        addBox(Vec3(0, 0, 0), Vec3(0.14f, 0.18f, 0.65f), lava);
        addBox(Vec3(0, -0.08f, -0.32f), Vec3(0.1f, 0.18f, 0.35f), wood);
        addBox(Vec3(-0.04f, 0.04f, 0.55f), Vec3(0.06f, 0.06f, 0.55f), lava); // Barrel 1
        addBox(Vec3( 0.04f, 0.04f, 0.55f), Vec3(0.06f, 0.06f, 0.55f), lava); // Barrel 2
    } else {
        // AWM (Emerald Sniper Skin)
        Vec4 green(0.15f, 0.75f, 0.45f, 1.0f);
        Vec4 black(0.12f, 0.12f, 0.15f, 1.0f);

        addBox(Vec3(0, 0, 0), Vec3(0.14f, 0.20f, 1.1f), green); // Long chassis
        addBox(Vec3(0, -0.04f, -0.65f), Vec3(0.1f, 0.24f, 0.45f), black); // Sniper Stock
        addBox(Vec3(0, 0.06f, 0.95f), Vec3(0.07f, 0.07f, 0.95f), black); // Heavy Barrel
        addBox(Vec3(0, 0.18f, 0.15f), Vec3(0.1f, 0.12f, 0.45f), black); // Sniper Scope
    }

    return Mesh(verts, inds);
}

inline Mesh createAirdropBox(float size = 1.8f) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 s, Vec4 col) {
        Mesh c = createCube(s.x, s.y, s.z, col);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : c.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, col));
        }
        for (auto i : c.indices) inds.push_back(base + i);
    };

    addBox(Vec3(0, size * 0.4f, 0), Vec3(size, size * 0.8f, size), Vec4(0.85f, 0.15f, 0.15f, 1.0f));
    addBox(Vec3(0, size * 0.85f, 0), Vec3(size * 1.04f, size * 0.22f, size * 1.04f), Vec4(0.15f, 0.45f, 0.9f, 1.0f));

    return Mesh(verts, inds);
}

inline Mesh createMedkit(float size = 0.85f) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 s, Vec4 col) {
        Mesh c = createCube(s.x, s.y, s.z, col);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : c.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, col));
        }
        for (auto i : c.indices) inds.push_back(base + i);
    };

    addBox(Vec3(0, size * 0.4f, 0), Vec3(size, size * 0.7f, size * 0.6f), Vec4(0.95f, 0.95f, 0.98f, 1.0f));
    addBox(Vec3(0, size * 0.76f, 0), Vec3(size * 0.5f, 0.04f, size * 0.18f), Vec4(0.1f, 0.85f, 0.25f, 1.0f));
    addBox(Vec3(0, size * 0.76f, 0), Vec3(size * 0.18f, 0.04f, size * 0.5f), Vec4(0.1f, 0.85f, 0.25f, 1.0f));

    return Mesh(verts, inds);
}

inline Mesh createAmmoBox(Vec4 col = Vec4(0.95f, 0.75f, 0.15f, 1.0f)) {
    return createCube(0.7f, 0.5f, 0.7f, col);
}

inline Mesh createGlooItem() {
    return createSphere(0.42f, 10, 12, Vec4(0.35f, 0.85f, 1.0f, 1.0f));
}

inline Mesh createVestItem(int level = 1) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 s, Vec4 c) {
        Mesh cb = createCube(s.x, s.y, s.z, c);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : cb.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, c));
        }
        for (auto i : cb.indices) inds.push_back(base + i);
    };

    Vec4 mainCol = (level == 1) ? Vec4(0.25f, 0.55f, 0.25f, 1.0f) : // Green Lv1
                   ((level == 2) ? Vec4(0.18f, 0.35f, 0.75f, 1.0f) : // Blue Lv2
                                   Vec4(0.95f, 0.75f, 0.15f, 1.0f)); // Gold Lv3

    Vec4 strapCol(0.15f, 0.18f, 0.22f, 1.0f);
    Vec4 plateCol = (level == 3) ? Vec4(0.15f, 0.15f, 0.18f, 1.0f) : mainCol;

    // Body plate
    addBox(Vec3(0, 0.35f, 0), Vec3(0.68f, 0.62f, 0.38f), mainCol);
    // Extra Ceramic Trauma Plate (Lv2 & Lv3)
    if (level >= 2) {
        addBox(Vec3(0, 0.35f, 0.21f), Vec3(0.48f, 0.45f, 0.08f), plateCol);
    }
    // Heavy Shoulder Guards (Lv3)
    if (level == 3) {
        addBox(Vec3(-0.38f, 0.65f, 0), Vec3(0.22f, 0.18f, 0.34f), mainCol);
        addBox(Vec3( 0.38f, 0.65f, 0), Vec3(0.22f, 0.18f, 0.34f), mainCol);
    }

    // Shoulder straps
    addBox(Vec3(-0.22f, 0.7f, 0), Vec3(0.16f, 0.16f, 0.34f), strapCol);
    addBox(Vec3( 0.22f, 0.7f, 0), Vec3(0.16f, 0.16f, 0.34f), strapCol);

    return Mesh(verts, inds);
}

inline Mesh createHelmetItem(int level = 1) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 s, Vec4 c) {
        Mesh cb = createCube(s.x, s.y, s.z, c);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : cb.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, c));
        }
        for (auto i : cb.indices) inds.push_back(base + i);
    };

    Vec4 helmCol = (level == 1) ? Vec4(0.28f, 0.45f, 0.25f, 1.0f) : // Green Lv1
                   ((level == 2) ? Vec4(0.18f, 0.22f, 0.32f, 1.0f) : // Dark Navy Lv2
                                   Vec4(0.12f, 0.14f, 0.18f, 1.0f)); // Spetsnaz Black Lv3

    // Helmet Dome
    Mesh dome = createSphere(0.42f, 12, 16, helmCol);
    unsigned int base = (unsigned int)verts.size();
    for (auto& v : dome.vertices) {
        verts.push_back(Vertex(v.position + Vec3(0, 0.2f, 0), v.normal, v.texCoord, helmCol));
    }
    for (auto i : dome.indices) inds.push_back(base + i);

    // Helmet Visor / Face Shield
    if (level == 1) {
        // Simple brim
        addBox(Vec3(0, 0.15f, 0.25f), Vec3(0.48f, 0.06f, 0.2f), helmCol);
    } else if (level == 2) {
        // Tactical Goggles & strap
        addBox(Vec3(0, 0.18f, 0.32f), Vec3(0.46f, 0.16f, 0.12f), Vec4(0.1f, 0.6f, 0.85f, 0.9f));
        addBox(Vec3(0, 0.18f, 0), Vec3(0.52f, 0.08f, 0.52f), Vec4(0.1f, 0.1f, 0.12f, 1.0f));
    } else {
        // Heavy Spetsnaz Titanium Face Shield with Golden Slit
        addBox(Vec3(0, 0.08f, 0.32f), Vec3(0.52f, 0.42f, 0.12f), Vec4(0.15f, 0.16f, 0.2f, 1.0f));
        addBox(Vec3(0, 0.15f, 0.38f), Vec3(0.38f, 0.08f, 0.05f), Vec4(0.95f, 0.75f, 0.15f, 1.0f));
    }

    return Mesh(verts, inds);
}

inline Mesh createBackpackItem(int tier = 1) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    Vec4 mainCol = (tier == 1) ? Vec4(0.28f, 0.42f, 0.22f, 1.0f) : // Green Lv1
                   ((tier == 2) ? Vec4(0.18f, 0.35f, 0.65f, 1.0f) : // Blue Lv2
                                  Vec4(0.95f, 0.75f, 0.15f, 1.0f)); // Gold Lv3
    Vec4 darkCol(0.12f, 0.14f, 0.16f, 1.0f);

    auto addBox = [&](Vec3 center, Vec3 s, Vec4 c) {
        Mesh cb = createCube(s.x, s.y, s.z, c);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : cb.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, c));
        }
        for (auto i : cb.indices) inds.push_back(base + i);
    };

    // Main pack
    addBox(Vec3(0, 0.3f, 0), Vec3(0.44f, 0.58f, 0.28f), mainCol);
    // Outer pocket
    addBox(Vec3(0, 0.24f, 0.18f), Vec3(0.36f, 0.38f, 0.12f), darkCol);
    // Top handle
    addBox(Vec3(0, 0.62f, 0), Vec3(0.18f, 0.06f, 0.08f), darkCol);

    return Mesh(verts, inds);
}

inline Mesh createGrenadeItem() {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 s, Vec4 c) {
        Mesh cb = createCube(s.x, s.y, s.z, c);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : cb.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, c));
        }
        for (auto i : cb.indices) inds.push_back(base + i);
    };

    Vec4 green(0.2f, 0.35f, 0.18f, 1.0f);
    Vec4 pin(0.85f, 0.85f, 0.9f, 1.0f);

    // Oval grenade body
    addBox(Vec3(0, 0.22f, 0), Vec3(0.32f, 0.42f, 0.32f), green);
    // Cap & Lever
    addBox(Vec3(0, 0.46f, 0), Vec3(0.14f, 0.12f, 0.14f), pin);
    addBox(Vec3(0.12f, 0.34f, 0), Vec3(0.04f, 0.28f, 0.08f), pin);

    return Mesh(verts, inds);
}

inline Mesh createAuraRing(float radius = 5.0f, float width = 0.4f, int segments = 32, Vec4 col = Vec4(0.2f, 0.85f, 1.0f, 0.85f)) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float rInner = radius - width * 0.5f;
    float rOuter = radius + width * 0.5f;

    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i / segments * 2.0f * PI;
        float c = std::cos(angle);
        float s = std::sin(angle);

        vertices.push_back(Vertex(Vec3(c * rInner, 0.05f, s * rInner), Vec3(0, 1, 0), Vec2((float)i/segments, 0), col));
        vertices.push_back(Vertex(Vec3(c * rOuter, 0.05f, s * rOuter), Vec3(0, 1, 0), Vec2((float)i/segments, 1), col));
    }

    for (int i = 0; i < segments; ++i) {
        int idx = i * 2;
        indices.push_back(idx);
        indices.push_back(idx + 1);
        indices.push_back(idx + 2);

        indices.push_back(idx + 1);
        indices.push_back(idx + 3);
        indices.push_back(idx + 2);
    }

    return Mesh(vertices, indices);
}

inline Mesh createFactoryHall(float width = 28.0f, float height = 9.0f, float depth = 22.0f) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 s, Vec4 c) {
        Mesh cb = createCube(s.x, s.y, s.z, c);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : cb.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, c));
        }
        for (auto i : cb.indices) inds.push_back(base + i);
    };

    Vec4 concrete(0.48f, 0.52f, 0.56f, 1.0f);
    Vec4 roof(0.25f, 0.32f, 0.38f, 1.0f);
    Vec4 steel(0.2f, 0.22f, 0.25f, 1.0f);
    Vec4 glass(0.6f, 0.85f, 0.95f, 0.7f);

    // Main hall walls
    addBox(Vec3(0, height * 0.5f, -depth * 0.5f), Vec3(width, height, 0.8f), concrete); // Back
    addBox(Vec3(-width * 0.5f, height * 0.5f, 0), Vec3(0.8f, height, depth), concrete); // Left
    addBox(Vec3( width * 0.5f, height * 0.5f, 0), Vec3(0.8f, height, depth), concrete); // Right

    // Front wall with large entrance
    float sideW = (width - 8.0f) * 0.5f;
    addBox(Vec3(-width * 0.5f + sideW * 0.5f, height * 0.5f, depth * 0.5f), Vec3(sideW, height, 0.8f), concrete);
    addBox(Vec3( width * 0.5f - sideW * 0.5f, height * 0.5f, depth * 0.5f), Vec3(sideW, height, 0.8f), concrete);
    addBox(Vec3(0, height * 0.8f, depth * 0.5f), Vec3(8.0f, height * 0.4f, 0.8f), steel);

    // Sloped Industrial Gabled Roof
    addBox(Vec3(0, height + 1.2f, 0), Vec3(width + 2.0f, 0.8f, depth + 2.0f), roof);

    // Industrial Chimney stack
    addBox(Vec3(width * 0.35f, height + 5.0f, -depth * 0.35f), Vec3(2.2f, 10.0f, 2.2f), steel);

    // Skylight windows
    addBox(Vec3(0, height + 1.65f, 0), Vec3(width * 0.6f, 0.2f, depth * 0.4f), glass);

    return Mesh(verts, inds);
}

inline Mesh createClockTower(float height = 24.0f) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 s, Vec4 c) {
        Mesh cb = createCube(s.x, s.y, s.z, c);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : cb.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, c));
        }
        for (auto i : cb.indices) inds.push_back(base + i);
    };

    Vec4 stone(0.65f, 0.58f, 0.52f, 1.0f);
    Vec4 darkStone(0.42f, 0.38f, 0.35f, 1.0f);
    Vec4 gold(0.95f, 0.82f, 0.25f, 1.0f);
    Vec4 roofCol(0.28f, 0.18f, 0.18f, 1.0f);

    // Base podium
    addBox(Vec3(0, 2.0f, 0), Vec3(12.0f, 4.0f, 12.0f), darkStone);
    // Tower body
    addBox(Vec3(0, height * 0.5f, 0), Vec3(7.2f, height, 7.2f), stone);
    // Top Balcony
    addBox(Vec3(0, height + 1.0f, 0), Vec3(9.2f, 2.0f, 9.2f), darkStone);
    // Clock Room
    addBox(Vec3(0, height + 4.5f, 0), Vec3(7.6f, 5.0f, 7.6f), stone);

    // 4 Clock Dials (Gold square faces)
    addBox(Vec3(0, height + 4.5f,  3.85f), Vec3(3.2f, 3.2f, 0.15f), gold);
    addBox(Vec3(0, height + 4.5f, -3.85f), Vec3(3.2f, 3.2f, 0.15f), gold);
    addBox(Vec3( 3.85f, height + 4.5f, 0), Vec3(0.15f, 3.2f, 3.2f), gold);
    addBox(Vec3(-3.85f, height + 4.5f, 0), Vec3(0.15f, 3.2f, 3.2f), gold);

    // Pyramidal Spire Roof
    addBox(Vec3(0, height + 9.0f, 0), Vec3(5.5f, 4.0f, 5.5f), roofCol);
    addBox(Vec3(0, height + 12.5f, 0), Vec3(2.5f, 3.0f, 2.5f), roofCol);
    addBox(Vec3(0, height + 15.0f, 0), Vec3(0.8f, 2.0f, 0.8f), gold);

    return Mesh(verts, inds);
}

inline Mesh createShippingContainer(float width = 6.0f, float height = 2.6f, float depth = 2.4f, Vec4 color = Vec4(0.85f, 0.25f, 0.2f, 1.0f)) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 s, Vec4 c) {
        Mesh cb = createCube(s.x, s.y, s.z, c);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : cb.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, c));
        }
        for (auto i : cb.indices) inds.push_back(base + i);
    };

    Vec4 frameCol(color.x * 0.75f, color.y * 0.75f, color.z * 0.75f, 1.0f);

    // Main box
    addBox(Vec3(0, height * 0.5f, 0), Vec3(width, height, depth), color);
    // Steel Frame edges
    addBox(Vec3(-width * 0.5f, height * 0.5f, -depth * 0.5f), Vec3(0.2f, height, 0.2f), frameCol);
    addBox(Vec3( width * 0.5f, height * 0.5f, -depth * 0.5f), Vec3(0.2f, height, 0.2f), frameCol);
    addBox(Vec3(-width * 0.5f, height * 0.5f,  depth * 0.5f), Vec3(0.2f, height, 0.2f), frameCol);
    addBox(Vec3( width * 0.5f, height * 0.5f,  depth * 0.5f), Vec3(0.2f, height, 0.2f), frameCol);

    return Mesh(verts, inds);
}

inline Mesh createHangar(float width = 32.0f, float height = 10.0f, float depth = 26.0f) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 s, Vec4 c) {
        Mesh cb = createCube(s.x, s.y, s.z, c);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : cb.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, c));
        }
        for (auto i : cb.indices) inds.push_back(base + i);
    };

    Vec4 corrugated(0.38f, 0.44f, 0.48f, 1.0f);
    Vec4 darkFrame(0.18f, 0.2f, 0.24f, 1.0f);

    // Arch walls & roof
    addBox(Vec3(0, height * 0.5f, -depth * 0.5f), Vec3(width, height, 0.6f), corrugated);
    addBox(Vec3(-width * 0.5f, height * 0.45f, 0), Vec3(0.6f, height * 0.9f, depth), corrugated);
    addBox(Vec3( width * 0.5f, height * 0.45f, 0), Vec3(0.6f, height * 0.9f, depth), corrugated);

    // Curved Barrel Roof
    addBox(Vec3(0, height + 0.6f, 0), Vec3(width * 0.95f, 1.2f, depth), darkFrame);
    addBox(Vec3(0, height + 2.0f, 0), Vec3(width * 0.75f, 1.4f, depth), corrugated);

    return Mesh(verts, inds);
}

inline Mesh createRadarTower(float height = 36.0f) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 s, Vec4 c) {
        Mesh cb = createCube(s.x, s.y, s.z, c);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : cb.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, c));
        }
        for (auto i : cb.indices) inds.push_back(base + i);
    };

    Vec4 steel(0.35f, 0.38f, 0.42f, 1.0f);
    Vec4 dishCol(0.9f, 0.92f, 0.95f, 1.0f);
    Vec4 redWarning(0.95f, 0.2f, 0.15f, 1.0f);

    // 4 Lattice legs
    float d = 4.5f;
    addBox(Vec3(-d, height * 0.5f, -d), Vec3(0.7f, height, 0.7f), steel);
    addBox(Vec3( d, height * 0.5f, -d), Vec3(0.7f, height, 0.7f), steel);
    addBox(Vec3(-d, height * 0.5f,  d), Vec3(0.7f, height, 0.7f), steel);
    addBox(Vec3( d, height * 0.5f,  d), Vec3(0.7f, height, 0.7f), steel);

    // Cross braces
    for (int k = 1; k <= 4; ++k) {
        float y = (height / 5.0f) * k;
        addBox(Vec3(0, y, 0), Vec3(d * 2.1f, 0.4f, d * 2.1f), steel);
    }

    // Top Platform
    addBox(Vec3(0, height + 0.5f, 0), Vec3(11.0f, 1.0f, 11.0f), steel);
    // Rotating Big Radar Dish
    addBox(Vec3(0, height + 4.5f, 0), Vec3(8.5f, 6.5f, 1.2f), dishCol);
    // Beacon Light
    addBox(Vec3(0, height + 8.5f, 0), Vec3(1.2f, 1.8f, 1.2f), redWarning);

    return Mesh(verts, inds);
}

inline Mesh createMotorbike() {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 s, Vec4 c) {
        Mesh cb = createCube(s.x, s.y, s.z, c);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : cb.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, c));
        }
        for (auto i : cb.indices) inds.push_back(base + i);
    };

    Vec4 bodyCol(0.92f, 0.22f, 0.18f, 1.0f); // Sport Red
    Vec4 tireCol(0.12f, 0.12f, 0.14f, 1.0f); // Rubber Black
    Vec4 rimCol(0.75f, 0.8f, 0.85f, 1.0f);   // Chrome
    Vec4 seatCol(0.18f, 0.18f, 0.20f, 1.0f); // Black Leather
    Vec4 lightCol(1.0f, 0.95f, 0.6f, 1.0f);  // Bright LED

    // 1. Front Wheel & Rear Wheel
    addBox(Vec3(0, 0.38f, -0.95f), Vec3(0.18f, 0.76f, 0.76f), tireCol);
    addBox(Vec3(0, 0.38f, -0.95f), Vec3(0.20f, 0.44f, 0.44f), rimCol);

    addBox(Vec3(0, 0.38f,  0.85f), Vec3(0.24f, 0.76f, 0.76f), tireCol);
    addBox(Vec3(0, 0.38f,  0.85f), Vec3(0.26f, 0.44f, 0.44f), rimCol);

    // 2. Chassis & Engine Block
    addBox(Vec3(0, 0.45f, 0.0f), Vec3(0.42f, 0.45f, 0.95f), Vec4(0.22f, 0.24f, 0.28f, 1.0f));

    // 3. Sport Gas Tank & Fairings
    addBox(Vec3(0, 0.72f, -0.25f), Vec3(0.48f, 0.32f, 0.75f), bodyCol);
    addBox(Vec3(0, 0.82f, -0.65f), Vec3(0.38f, 0.28f, 0.45f), bodyCol);

    // 4. Seat
    addBox(Vec3(0, 0.68f, 0.25f), Vec3(0.36f, 0.16f, 0.65f), seatCol);

    // 5. Handlebars & Front Fork
    addBox(Vec3(0, 0.92f, -0.72f), Vec3(0.85f, 0.08f, 0.08f), rimCol);
    addBox(Vec3(-0.14f, 0.62f, -0.85f), Vec3(0.06f, 0.65f, 0.08f), rimCol);
    addBox(Vec3( 0.14f, 0.62f, -0.85f), Vec3(0.06f, 0.65f, 0.08f), rimCol);

    // 6. LED Headlight
    addBox(Vec3(0, 0.75f, -1.02f), Vec3(0.24f, 0.18f, 0.08f), lightCol);

    // 7. Dual Chrome Exhaust Pipes
    addBox(Vec3( 0.22f, 0.32f, 0.45f), Vec3(0.08f, 0.08f, 0.85f), rimCol);
    addBox(Vec3(-0.22f, 0.32f, 0.45f), Vec3(0.08f, 0.08f, 0.85f), rimCol);

    return Mesh(verts, inds);
}

inline Mesh createTacticalJeep() {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 s, Vec4 c) {
        Mesh cb = createCube(s.x, s.y, s.z, c);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : cb.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, c));
        }
        for (auto i : cb.indices) inds.push_back(base + i);
    };

    Vec4 bodyCol(0.28f, 0.42f, 0.24f, 1.0f); // Olive Drab Camo
    Vec4 darkFrame(0.12f, 0.14f, 0.16f, 1.0f); // Steel Chassis
    Vec4 tireCol(0.10f, 0.10f, 0.12f, 1.0f);   // Offroad Tires
    Vec4 rimCol(0.6f, 0.65f, 0.7f, 1.0f);      // Heavy Rims
    Vec4 glass(0.35f, 0.65f, 0.85f, 0.75f);    // Windshield
    Vec4 lightCol(1.0f, 0.95f, 0.6f, 1.0f);    // Halogen Headlights

    // 1. Four Heavy All-Terrain Tires
    float wx = 0.95f, wy = 0.48f, wz = 1.15f;
    addBox(Vec3(-wx, wy, -wz), Vec3(0.38f, 0.95f, 0.95f), tireCol);
    addBox(Vec3(-wx, wy, -wz), Vec3(0.40f, 0.55f, 0.55f), rimCol);

    addBox(Vec3( wx, wy, -wz), Vec3(0.38f, 0.95f, 0.95f), tireCol);
    addBox(Vec3( wx, wy, -wz), Vec3(0.40f, 0.55f, 0.55f), rimCol);

    addBox(Vec3(-wx, wy,  wz), Vec3(0.38f, 0.95f, 0.95f), tireCol);
    addBox(Vec3(-wx, wy,  wz), Vec3(0.40f, 0.55f, 0.55f), rimCol);

    addBox(Vec3( wx, wy,  wz), Vec3(0.38f, 0.95f, 0.95f), tireCol);
    addBox(Vec3( wx, wy,  wz), Vec3(0.40f, 0.55f, 0.55f), rimCol);

    // 2. Main Chassis & Floor
    addBox(Vec3(0, 0.65f, 0), Vec3(1.85f, 0.45f, 3.4f), bodyCol);

    // 3. Engine Hood & Front Grille
    addBox(Vec3(0, 0.95f, -1.05f), Vec3(1.65f, 0.45f, 1.25f), bodyCol);
    addBox(Vec3(0, 0.85f, -1.72f), Vec3(1.5f, 0.45f, 0.1f), darkFrame); // Radiator Grille
    addBox(Vec3(0, 0.45f, -1.8f), Vec3(1.95f, 0.25f, 0.25f), darkFrame); // Steel Front Bumper

    // 4. Twin Halogen Headlights
    addBox(Vec3(-0.6f, 0.92f, -1.72f), Vec3(0.24f, 0.24f, 0.08f), lightCol);
    addBox(Vec3( 0.6f, 0.92f, -1.72f), Vec3(0.24f, 0.24f, 0.08f), lightCol);

    // 5. Cabin & 2 Bucket Seats
    addBox(Vec3(-0.45f, 0.92f, 0.15f), Vec3(0.55f, 0.55f, 0.55f), darkFrame);
    addBox(Vec3( 0.45f, 0.92f, 0.15f), Vec3(0.55f, 0.55f, 0.55f), darkFrame);

    // 6. Windshield Glass & Frame
    addBox(Vec3(0, 1.35f, -0.42f), Vec3(1.65f, 0.65f, 0.08f), glass);
    addBox(Vec3(0, 1.7f, -0.42f), Vec3(1.72f, 0.12f, 0.12f), darkFrame);

    // 7. Heavy Roll Cage Bars
    addBox(Vec3(-0.8f, 1.45f, 0.4f), Vec3(0.1f, 1.1f, 0.1f), darkFrame);
    addBox(Vec3( 0.8f, 1.45f, 0.4f), Vec3(0.1f, 1.1f, 0.1f), darkFrame);
    addBox(Vec3(0, 1.95f, 0.4f), Vec3(1.7f, 0.1f, 0.1f), darkFrame);
    addBox(Vec3(0, 1.95f, -0.05f), Vec3(1.7f, 0.1f, 0.9f), darkFrame); // Overhead bars

    // 8. Spare Wheel on the Back
    addBox(Vec3(0, 1.05f, 1.76f), Vec3(0.85f, 0.85f, 0.32f), tireCol);

    return Mesh(verts, inds);
}

// Walkable 2-Room Compound Villa (Front Archway 3.4m, Back Door 2.6m, Windows, Interior Doorway)
inline Mesh createCompoundHouse(float width = 16.0f, float height = 5.2f, float depth = 14.0f) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 size, Vec4 col) {
        Mesh c = createCube(size.x, size.y, size.z, col);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : c.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, col));
        }
        for (auto i : c.indices) inds.push_back(base + i);
    };

    Vec4 wallCol(0.85f, 0.82f, 0.74f, 1.0f); // Stucco Villa Beige
    Vec4 trimCol(0.35f, 0.22f, 0.16f, 1.0f); // Dark Wood Trim
    Vec4 roofCol(0.72f, 0.28f, 0.18f, 1.0f); // Terracotta Tile
    Vec4 floorCol(0.42f, 0.40f, 0.38f, 1.0f);

    float hw = width * 0.5f, hh = height * 0.5f, hd = depth * 0.5f;
    float wallT = 0.5f;

    // Solid Back Wall with Window / Back Door Gap
    addBox(Vec3(-hw * 0.55f, hh, -hd), Vec3(hw * 0.8f, height, wallT), wallCol);
    addBox(Vec3( hw * 0.55f, hh, -hd), Vec3(hw * 0.8f, height, wallT), wallCol);
    addBox(Vec3(0, height - 0.6f, -hd), Vec3(hw * 0.4f, 1.2f, wallT), trimCol);

    // Solid Left & Right Outer Walls
    addBox(Vec3(-hw, hh, 0), Vec3(wallT, height, depth), wallCol);
    addBox(Vec3( hw, hh, 0), Vec3(wallT, height, depth), wallCol);

    // Front Wall with Wide Open Entry Archway (3.4m wide)
    float doorW = 3.4f;
    float sideW = (width - doorW) * 0.5f;
    addBox(Vec3(-hw + sideW * 0.5f, hh, hd), Vec3(sideW, height, wallT), wallCol);
    addBox(Vec3( hw - sideW * 0.5f, hh, hd), Vec3(sideW, height, wallT), wallCol);
    addBox(Vec3(0, height - 0.5f, hd), Vec3(doorW, 1.0f, wallT), trimCol);

    // Interior Partition Wall with Open Passageway (2.6m wide)
    addBox(Vec3(-hw + sideW * 0.45f, hh, 0), Vec3(sideW * 0.9f, height, wallT), wallCol);
    addBox(Vec3(0, height - 0.5f, 0), Vec3(doorW, 1.0f, wallT), trimCol);

    // Floor Slab
    addBox(Vec3(0, 0.1f, 0), Vec3(width, 0.2f, depth), floorCol);

    // Terracotta Overhanging Sloped Roof
    addBox(Vec3(0, height + 0.3f, 0), Vec3(width + 1.6f, 0.6f, depth + 1.6f), roofCol);
    addBox(Vec3(0, height + 1.2f, 0), Vec3(width * 0.7f, 1.2f, depth * 0.85f), roofCol);

    return Mesh(verts, inds);
}

// Walkable Wilderness Cabin (Log Walls, Open Front Porch & Doorway)
inline Mesh createWildernessCabin(float width = 10.0f, float height = 4.2f, float depth = 8.5f) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 size, Vec4 col) {
        Mesh c = createCube(size.x, size.y, size.z, col);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : c.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, col));
        }
        for (auto i : c.indices) inds.push_back(base + i);
    };

    Vec4 woodLog(0.48f, 0.32f, 0.18f, 1.0f);
    Vec4 darkWood(0.32f, 0.20f, 0.12f, 1.0f);
    Vec4 roofCol(0.28f, 0.24f, 0.20f, 1.0f);

    float hw = width * 0.5f, hh = height * 0.5f, hd = depth * 0.5f;
    float wallT = 0.45f;

    // Back & Side walls
    addBox(Vec3(0, hh, -hd), Vec3(width, height, wallT), woodLog);
    addBox(Vec3(-hw, hh, 0), Vec3(wallT, height, depth), woodLog);
    addBox(Vec3( hw, hh, 0), Vec3(wallT, height, depth), woodLog);

    // Front wall with open door (2.6m wide)
    float doorW = 2.6f;
    float sideW = (width - doorW) * 0.5f;
    addBox(Vec3(-hw + sideW * 0.5f, hh, hd), Vec3(sideW, height, wallT), woodLog);
    addBox(Vec3( hw - sideW * 0.5f, hh, hd), Vec3(sideW, height, wallT), woodLog);
    addBox(Vec3(0, height - 0.4f, hd), Vec3(doorW, 0.8f, wallT), darkWood);

    // Porch Deck in Front
    addBox(Vec3(0, 0.15f, hd + 1.5f), Vec3(width + 0.8f, 0.3f, 3.0f), darkWood);
    // Porch Support Pillars
    addBox(Vec3(-hw + 0.3f, hh, hd + 2.8f), Vec3(0.3f, height, 0.3f), darkWood);
    addBox(Vec3( hw - 0.3f, hh, hd + 2.8f), Vec3(0.3f, height, 0.3f), darkWood);

    // Wooden Floor
    addBox(Vec3(0, 0.1f, 0), Vec3(width, 0.2f, depth), darkWood);

    // Sloped Cabin Roof
    addBox(Vec3(0, height + 0.4f, 0.8f), Vec3(width + 1.4f, 0.5f, depth + 3.8f), roofCol);
    addBox(Vec3(0, height + 1.1f, 0.8f), Vec3(width * 0.6f, 1.0f, depth + 3.8f), roofCol);

    return Mesh(verts, inds);
}

// Walkable Heavy Concrete Bunker (Fortified Entrance, Horizontal Firing Slits)
inline Mesh createConcreteBunker(float width = 11.0f, float height = 3.6f, float depth = 11.0f) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 size, Vec4 col) {
        Mesh c = createCube(size.x, size.y, size.z, col);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : c.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, col));
        }
        for (auto i : c.indices) inds.push_back(base + i);
    };

    Vec4 blastConcrete(0.42f, 0.45f, 0.44f, 1.0f);
    Vec4 steelTrim(0.20f, 0.22f, 0.24f, 1.0f);

    float hw = width * 0.5f, hh = height * 0.5f, hd = depth * 0.5f;
    float wallT = 0.8f;

    // Back & Side walls with firing slits
    addBox(Vec3(0, hh, -hd), Vec3(width, height, wallT), blastConcrete);
    addBox(Vec3(-hw, 0.8f, 0), Vec3(wallT, 1.6f, depth), blastConcrete);
    addBox(Vec3(-hw, height - 0.5f, 0), Vec3(wallT, 1.0f, depth), blastConcrete);
    addBox(Vec3( hw, 0.8f, 0), Vec3(wallT, 1.6f, depth), blastConcrete);
    addBox(Vec3( hw, height - 0.5f, 0), Vec3(wallT, 1.0f, depth), blastConcrete);

    // Front wall with 2.8m reinforced entrance
    float doorW = 2.8f;
    float sideW = (width - doorW) * 0.5f;
    addBox(Vec3(-hw + sideW * 0.5f, hh, hd), Vec3(sideW, height, wallT), blastConcrete);
    addBox(Vec3( hw - sideW * 0.5f, hh, hd), Vec3(sideW, height, wallT), blastConcrete);
    addBox(Vec3(0, height - 0.5f, hd), Vec3(doorW, 1.0f, wallT), steelTrim);

    // Floor & Thick Blast Slab Roof
    addBox(Vec3(0, 0.1f, 0), Vec3(width, 0.2f, depth), blastConcrete);
    addBox(Vec3(0, height + 0.4f, 0), Vec3(width + 1.2f, 0.8f, depth + 1.2f), steelTrim);

    return Mesh(verts, inds);
}

// 3D Glowing Energy Mushroom (Nấm EP Lv1/Lv2/Lv3)
inline Mesh createMushroomMesh(int tier = 1) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addPart = [&](Mesh& part, Mat4 m, Vec4 col) {
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : part.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, col));
        }
        for (auto i : part.indices) inds.push_back(base + i);
    };

    Vec4 capCol = (tier == 1) ? Vec4(0.95f, 0.52f, 0.12f, 1.0f) :  // Orange Lv1 (+50 EP)
                  ((tier == 2) ? Vec4(0.98f, 0.85f, 0.15f, 1.0f) :  // Yellow Lv2 (+100 EP)
                                 Vec4(0.75f, 0.22f, 0.95f, 1.0f));  // Purple Lv3 (+200 EP)

    Vec4 stemCol(0.92f, 0.88f, 0.80f, 1.0f);
    Vec4 spotCol(1.0f, 1.0f, 1.0f, 1.0f);

    float scale = (tier == 1) ? 0.7f : ((tier == 2) ? 0.85f : 1.05f);

    // 1. Pale Mushroom Stem
    Mesh stem = createCylinder(0.20f * scale, 0.32f * scale, 0.8f * scale, 12, stemCol);
    addPart(stem, Mat4::translate(Vec3(0, 0.4f * scale, 0)), stemCol);

    // 2. Wide Rounded Glowing Mushroom Cap
    Mesh cap = createSphere(0.75f * scale, 12, 16, capCol);
    addPart(cap, Mat4::translate(Vec3(0, 0.75f * scale, 0)) * Mat4::scale(Vec3(1.0f, 0.55f, 1.0f)), capCol);

    // 3. Glowing White Spots
    Mesh spot1 = createSphere(0.16f * scale, 8, 8, spotCol);
    addPart(spot1, Mat4::translate(Vec3(0, 1.05f * scale, 0)), spotCol);
    addPart(spot1, Mat4::translate(Vec3(-0.38f * scale, 0.88f * scale, 0.25f * scale)), spotCol);
    addPart(spot1, Mat4::translate(Vec3( 0.38f * scale, 0.88f * scale, -0.25f * scale)), spotCol);

    return Mesh(verts, inds);
}

// 3D Military C-130 Cargo Transport Aircraft (Airdrop Flyover)
inline Mesh createCargoPlane() {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 size, Vec4 col) {
        Mesh c = createCube(size.x, size.y, size.z, col);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : c.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, col));
        }
        for (auto i : c.indices) inds.push_back(base + i);
    };

    Vec4 camoGray(0.45f, 0.48f, 0.52f, 1.0f);
    Vec4 darkGray(0.22f, 0.24f, 0.28f, 1.0f);
    Vec4 glass(0.2f, 0.65f, 0.95f, 0.8f);
    Vec4 engineCol(0.18f, 0.18f, 0.22f, 1.0f);
    Vec4 propCol(0.95f, 0.75f, 0.15f, 1.0f);

    // 1. Fuselage
    addBox(Vec3(0, 0, 0), Vec3(4.8f, 4.8f, 28.0f), camoGray);
    // Nose Cone
    addBox(Vec3(0, -0.4f, -15.5f), Vec3(3.6f, 3.6f, 3.5f), camoGray);
    // Cockpit Glass
    addBox(Vec3(0, 1.4f, -12.5f), Vec3(3.8f, 1.8f, 3.5f), glass);

    // 2. High Giant Wings (38m Span)
    addBox(Vec3(0, 1.8f, -2.0f), Vec3(38.0f, 0.6f, 5.2f), camoGray);

    // 3. 4 Turboprop Engine Pods & Spinners
    float engX[4] = {-12.0f, -6.0f, 6.0f, 12.0f};
    for (int i = 0; i < 4; ++i) {
        addBox(Vec3(engX[i], 0.8f, -3.2f), Vec3(1.6f, 1.6f, 5.5f), engineCol);
        // Propeller hub
        addBox(Vec3(engX[i], 0.8f, -6.2f), Vec3(0.4f, 4.2f, 0.4f), propCol);
    }

    // 4. T-Tail Empennage
    addBox(Vec3(0, 3.8f, 12.5f), Vec3(0.6f, 6.5f, 4.8f), camoGray); // Vertical Fin
    addBox(Vec3(0, 6.8f, 13.0f), Vec3(13.0f, 0.4f, 3.4f), camoGray); // Horizontal Stabilizer

    return Mesh(verts, inds);
}

// 3D UAV Surveillance Recon Drone
inline Mesh createUAVDroneMesh() {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 size, Vec4 col) {
        Mesh c = createCube(size.x, size.y, size.z, col);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : c.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, col));
        }
        for (auto i : c.indices) inds.push_back(base + i);
    };

    Vec4 carbon(0.12f, 0.14f, 0.16f, 1.0f);
    Vec4 white(0.92f, 0.95f, 0.98f, 1.0f);
    Vec4 redLED(0.95f, 0.15f, 0.15f, 1.0f);
    Vec4 radarScan(0.2f, 0.85f, 1.0f, 1.0f);

    // Central Body
    addBox(Vec3(0, 0, 0), Vec3(1.4f, 0.45f, 2.2f), carbon);
    addBox(Vec3(0, 0.25f, 0), Vec3(0.9f, 0.35f, 1.5f), white);

    // 4 X-Rotor Arms
    float armD = 1.6f;
    addBox(Vec3(-armD*0.5f, 0.05f, -armD*0.5f), Vec3(armD, 0.12f, 0.18f), carbon);
    addBox(Vec3( armD*0.5f, 0.05f, -armD*0.5f), Vec3(armD, 0.12f, 0.18f), carbon);
    addBox(Vec3(-armD*0.5f, 0.05f,  armD*0.5f), Vec3(armD, 0.12f, 0.18f), carbon);
    addBox(Vec3( armD*0.5f, 0.05f,  armD*0.5f), Vec3(armD, 0.12f, 0.18f), carbon);

    // 4 Rotor Pods & Blades
    Vec3 pods[4] = { Vec3(-armD, 0.1f, -armD), Vec3(armD, 0.1f, -armD), Vec3(-armD, 0.1f, armD), Vec3(armD, 0.1f, armD) };
    for (int i = 0; i < 4; ++i) {
        addBox(pods[i], Vec3(0.35f, 0.35f, 0.35f), carbon);
        addBox(pods[i] + Vec3(0, 0.2f, 0), Vec3(1.2f, 0.04f, 0.12f), white);
        addBox(pods[i] + Vec3(0, 0.22f, 0), Vec3(0.12f, 0.08f, 0.12f), redLED);
    }

    // Belly Radar Sensor Dome
    addBox(Vec3(0, -0.32f, 0.2f), Vec3(0.6f, 0.45f, 0.6f), radarScan);

    return Mesh(verts, inds);
}

// 3D Red Beacon Smoke Column (Airdrop Flare)
inline Mesh createSmokeColumnMesh(float height = 45.0f, float radius = 0.6f) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    auto addBox = [&](Vec3 center, Vec3 size, Vec4 col) {
        Mesh c = createCube(size.x, size.y, size.z, col);
        Mat4 m = Mat4::translate(center);
        unsigned int base = (unsigned int)verts.size();
        for (auto& v : c.vertices) {
            Vec4 tp = m * Vec4(v.position, 1.0f);
            verts.push_back(Vertex(Vec3(tp.x, tp.y, tp.z), v.normal, v.texCoord, col));
        }
        for (auto i : c.indices) inds.push_back(base + i);
    };

    Vec4 redSmoke(0.95f, 0.20f, 0.15f, 0.75f);
    Vec4 brightCore(1.0f, 0.65f, 0.2f, 0.95f);

    // Base flare device
    addBox(Vec3(0, 0.2f, 0), Vec3(0.4f, 0.4f, 0.4f), Vec4(0.15f, 0.15f, 0.18f, 1.0f));
    addBox(Vec3(0, 0.45f, 0), Vec3(0.2f, 0.2f, 0.2f), brightCore);

    // Rising Smoke Column
    int segments = 15;
    for (int s = 0; s < segments; ++s) {
        float frac = (float)s / segments;
        float y = frac * height + 1.0f;
        float w = radius * (1.0f + frac * 2.2f);
        float alpha = 0.85f * (1.0f - frac * 0.55f);
        addBox(Vec3(0, y, 0), Vec3(w, height / segments, w), Vec4(redSmoke.x, redSmoke.y, redSmoke.z, alpha));
    }

    return Mesh(verts, inds);
}

inline Mesh createQuad2D(float w, float h, Vec4 color = Vec4(1,1,1,1)) {
    std::vector<Vertex> vertices = {
        Vertex(Vec3(0, 0, 0), Vec3(0,0,1), Vec2(0, 0), color),
        Vertex(Vec3(w, 0, 0), Vec3(0,0,1), Vec2(1, 0), color),
        Vertex(Vec3(w, h, 0), Vec3(0,0,1), Vec2(1, 1), color),
        Vertex(Vec3(0, h, 0), Vec3(0,0,1), Vec2(0, 1), color)
    };
    std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};
    return Mesh(vertices, indices);
}

} // namespace MeshGen

#endif // MESH_HPP
