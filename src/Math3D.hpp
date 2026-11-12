#ifndef MATH3D_HPP
#define MATH3D_HPP

#include <cmath>
#include <iostream>
#include <algorithm>

constexpr float PI = 3.14159265358979323846f;
constexpr float DEG2RAD = PI / 180.0f;
constexpr float RAD2DEG = 180.0f / PI;

struct Vec2 {
    float x = 0.0f, y = 0.0f;

    Vec2() = default;
    Vec2(float _x, float _y) : x(_x), y(_y) {}

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2 operator/(float s) const { return {x / s, y / s}; }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }

    float lengthSq() const { return x * x + y * y; }
    float length() const { return std::sqrt(lengthSq()); }

    Vec2 normalized() const {
        float l = length();
        if (l > 1e-6f) return {x / l, y / l};
        return {0.0f, 0.0f};
    }
};

struct Vec3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;

    Vec3() = default;
    Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    Vec3(float val) : x(val), y(val), z(val) {}

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator*(const Vec3& o) const { return {x * o.x, y * o.y, z * o.z}; }
    Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }
    Vec3 operator-() const { return {-x, -y, -z}; }

    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }

    float lengthSq() const { return x * x + y * y + z * z; }
    float length() const { return std::sqrt(lengthSq()); }

    Vec3 normalized() const {
        float l = length();
        if (l > 1e-6f) return {x / l, y / l, z / l};
        return {0.0f, 0.0f, 0.0f};
    }
};

inline float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

inline Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
    return a + (b - a) * t;
}

struct Vec4 {
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f;

    Vec4() = default;
    Vec4(float val) : x(val), y(val), z(val), w(val) {}
    Vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    Vec4(const Vec3& v, float _w = 1.0f) : x(v.x), y(v.y), z(v.z), w(_w) {}

    Vec4 operator*(float s) const { return {x * s, y * s, z * s, w}; }
    Vec4 operator*(const Vec4& o) const { return {x * o.x, y * o.y, z * o.z, w * o.w}; }
    Vec4 operator+(const Vec4& o) const { return {x + o.x, y + o.y, z + o.z, w + o.w}; }
    Vec4 operator-(const Vec4& o) const { return {x - o.x, y - o.y, z - o.z, w - o.w}; }
};

struct Mat4 {
    float m[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    Mat4() = default;

    static Mat4 identity() {
        return Mat4();
    }

    float& operator()(int row, int col) { return m[col * 4 + row]; }
    float operator()(int row, int col) const { return m[col * 4 + row]; }

    Mat4 operator*(const Mat4& o) const {
        Mat4 res;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                res(r, c) = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    res(r, c) += (*this)(r, k) * o(k, c);
                }
            }
        }
        return res;
    }

    Vec4 operator*(const Vec4& v) const {
        return {
            (*this)(0,0)*v.x + (*this)(0,1)*v.y + (*this)(0,2)*v.z + (*this)(0,3)*v.w,
            (*this)(1,0)*v.x + (*this)(1,1)*v.y + (*this)(1,2)*v.z + (*this)(1,3)*v.w,
            (*this)(2,0)*v.x + (*this)(2,1)*v.y + (*this)(2,2)*v.z + (*this)(2,3)*v.w,
            (*this)(3,0)*v.x + (*this)(3,1)*v.y + (*this)(3,2)*v.z + (*this)(3,3)*v.w
        };
    }

    static Mat4 translate(const Vec3& t) {
        Mat4 res = identity();
        res(0, 3) = t.x;
        res(1, 3) = t.y;
        res(2, 3) = t.z;
        return res;
    }

    static Mat4 scale(const Vec3& s) {
        Mat4 res = identity();
        res(0, 0) = s.x;
        res(1, 1) = s.y;
        res(2, 2) = s.z;
        return res;
    }

    static Mat4 rotateX(float angleRad) {
        Mat4 res = identity();
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        res(1, 1) = c; res(1, 2) = -s;
        res(2, 1) = s; res(2, 2) = c;
        return res;
    }

    static Mat4 rotateY(float angleRad) {
        Mat4 res = identity();
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        res(0, 0) = c;  res(0, 2) = s;
        res(2, 0) = -s; res(2, 2) = c;
        return res;
    }

    static Mat4 rotateZ(float angleRad) {
        Mat4 res = identity();
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        res(0, 0) = c; res(0, 1) = -s;
        res(1, 0) = s; res(1, 1) = c;
        return res;
    }

    static Mat4 rotateAxis(const Vec3& axis, float angleRad) {
        Vec3 a = axis.normalized();
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        float t = 1.0f - c;

        Mat4 res = identity();
        res(0, 0) = t * a.x * a.x + c;
        res(0, 1) = t * a.x * a.y - s * a.z;
        res(0, 2) = t * a.x * a.z + s * a.y;

        res(1, 0) = t * a.x * a.y + s * a.z;
        res(1, 1) = t * a.y * a.y + c;
        res(1, 2) = t * a.y * a.z - s * a.x;

        res(2, 0) = t * a.x * a.z - s * a.y;
        res(2, 1) = t * a.y * a.z + s * a.x;
        res(2, 2) = t * a.z * a.z + c;

        return res;
    }

    static Mat4 perspective(float fovRad, float aspect, float zNear, float zFar) {
        Mat4 res;
        for (int i = 0; i < 16; ++i) res.m[i] = 0.0f;
        float tanHalfFov = std::tan(fovRad / 2.0f);

        res(0, 0) = 1.0f / (aspect * tanHalfFov);
        res(1, 1) = 1.0f / tanHalfFov;
        res(2, 2) = -(zFar + zNear) / (zFar - zNear);
        res(2, 3) = -(2.0f * zFar * zNear) / (zFar - zNear);
        res(3, 2) = -1.0f;
        return res;
    }

    static Mat4 ortho(float left, float right, float bottom, float top, float zNear = -1.0f, float zFar = 1.0f) {
        Mat4 res = identity();
        res(0, 0) = 2.0f / (right - left);
        res(1, 1) = 2.0f / (top - bottom);
        res(2, 2) = -2.0f / (zFar - zNear);
        res(0, 3) = -(right + left) / (right - left);
        res(1, 3) = -(top + bottom) / (top - bottom);
        res(2, 3) = -(zFar + zNear) / (zFar - zNear);
        return res;
    }

    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 f = (target - eye).normalized();
        Vec3 s = cross(f, up).normalized();
        Vec3 u = cross(s, f);

        Mat4 res = identity();
        res(0, 0) = s.x;  res(0, 1) = s.y;  res(0, 2) = s.z;
        res(1, 0) = u.x;  res(1, 1) = u.y;  res(1, 2) = u.z;
        res(2, 0) = -f.x; res(2, 1) = -f.y; res(2, 2) = -f.z;

        res(0, 3) = -dot(s, eye);
        res(1, 3) = -dot(u, eye);
        res(2, 3) = dot(f, eye);
        return res;
    }

    const float* data() const { return m; }
};

struct Ray {
    Vec3 origin;
    Vec3 dir;

    Ray(const Vec3& o, const Vec3& d) : origin(o), dir(d.normalized()) {}
};

struct AABB {
    Vec3 min;
    Vec3 max;

    AABB() : min(), max() {}
    AABB(const Vec3& _min, const Vec3& _max) : min(_min), max(_max) {}

    bool intersectRay(const Ray& ray, float& tOut) const {
        float tmin = (min.x - ray.origin.x) / (std::abs(ray.dir.x) > 1e-6f ? ray.dir.x : 1e-6f);
        float tmax = (max.x - ray.origin.x) / (std::abs(ray.dir.x) > 1e-6f ? ray.dir.x : 1e-6f);
        if (tmin > tmax) std::swap(tmin, tmax);

        float tymin = (min.y - ray.origin.y) / (std::abs(ray.dir.y) > 1e-6f ? ray.dir.y : 1e-6f);
        float tymax = (max.y - ray.origin.y) / (std::abs(ray.dir.y) > 1e-6f ? ray.dir.y : 1e-6f);
        if (tymin > tymax) std::swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax)) return false;
        if (tymin > tmin) tmin = tymin;
        if (tymax < tmax) tmax = tymax;

        float tzmin = (min.z - ray.origin.z) / (std::abs(ray.dir.z) > 1e-6f ? ray.dir.z : 1e-6f);
        float tzmax = (max.z - ray.origin.z) / (std::abs(ray.dir.z) > 1e-6f ? ray.dir.z : 1e-6f);
        if (tzmin > tzmax) std::swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax)) return false;
        if (tzmin > tmin) tmin = tzmin;
        if (tzmax < tmax) tmax = tzmax;

        if (tmax < 0) return false;
        tOut = (tmin < 0) ? tmax : tmin;
        return true;
    }

    // Resolves 2D circle on (X, Z) against this AABB box
    bool resolveCircle(Vec3& pos, float radius) const {
        if (pos.y < min.y - 0.2f || pos.y > max.y + 0.2f) return false;

        float closestX = std::max(min.x, std::min(pos.x, max.x));
        float closestZ = std::max(min.z, std::min(pos.z, max.z));

        float dx = pos.x - closestX;
        float dz = pos.z - closestZ;
        float distSq = dx * dx + dz * dz;

        if (distSq < radius * radius && distSq > 1e-6f) {
            float dist = std::sqrt(distSq);
            float overlap = radius - dist;
            pos.x += (dx / dist) * overlap;
            pos.z += (dz / dist) * overlap;
            return true;
        } else if (distSq <= 1e-6f) {
            // Center is inside box - push to nearest face
            float dl = pos.x - min.x;
            float dr = max.x - pos.x;
            float dt = pos.z - min.z;
            float db = max.z - pos.z;

            float minD = std::min({dl, dr, dt, db});
            if (minD == dl) pos.x = min.x - radius;
            else if (minD == dr) pos.x = max.x + radius;
            else if (minD == dt) pos.z = min.z - radius;
            else pos.z = max.z + radius;
            return true;
        }
        return false;
    }
};

inline bool intersectSphere(const Ray& ray, const Vec3& center, float radius, float& tOut) {
    Vec3 oc = ray.origin - center;
    float b = dot(oc, ray.dir);
    float c = dot(oc, oc) - radius * radius;
    float disc = b * b - c;
    if (disc < 0) return false;
    float sqrtDisc = std::sqrt(disc);
    float t1 = -b - sqrtDisc;
    float t2 = -b + sqrtDisc;
    if (t1 > 0) { tOut = t1; return true; }
    if (t2 > 0) { tOut = t2; return true; }
    return false;
}

#endif // MATH3D_HPP
