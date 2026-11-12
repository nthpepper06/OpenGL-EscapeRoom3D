#ifndef LAB4_TRANSFORMATIONS_HPP
#define LAB4_TRANSFORMATIONS_HPP

#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "../Math3D.hpp"

namespace Lab4 {

// ============================================================================
// HẰNG SỐ TOÁN HỌC DÙNG CHUNG
// ============================================================================
#ifndef PI_CONST_DEFINED
#define PI_CONST_DEFINED
constexpr float LAB4_PI = 3.14159265358979323846f;
constexpr float LAB4_DEG2RAD = LAB4_PI / 180.0f;
constexpr float LAB4_RAD2DEG = 180.0f / LAB4_PI;
#endif

// ============================================================================
// MA TRẬN BIẾN ĐỔI 4x4 (HOMOGENEOUS 4x4 TRANSFORMATION MATRIX)
// ============================================================================
/**
 * Biểu diễn ma trận 4x4 theo thứ tự dòng và cột:
 * [ m[0][0]  m[0][1]  m[0][2]  m[0][3] ]   [ x ]   [ x' ]
 * [ m[1][0]  m[1][1]  m[1][2]  m[1][3] ] * [ y ] = [ y' ]
 * [ m[2][0]  m[2][1]  m[2][2]  m[2][3] ]   [ z ]   [ z' ]
 * [ m[3][0]  m[3][1]  m[3][2]  m[3][3] ]   [ w ]   [ w' ]
 */
struct Matrix4x4 {
    float m[4][4] = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    };

    static Matrix4x4 identity() {
        Matrix4x4 res;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                res.m[r][c] = (r == c) ? 1.0f : 0.0f;
            }
        }
        return res;
    }

    // Nhân hai ma trận 4x4: C = A * B
    Matrix4x4 operator*(const Matrix4x4& b) const {
        Matrix4x4 res;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                res.m[r][c] = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    res.m[r][c] += m[r][k] * b.m[k][c];
                }
            }
        }
        return res;
    }

    // Nhân ma trận với vector 4D [x, y, z, w]^T
    Vec4 operator*(const Vec4& v) const {
        return Vec4(
            m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z + m[0][3]*v.w,
            m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z + m[1][3]*v.w,
            m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z + m[2][3]*v.w,
            m[3][0]*v.x + m[3][1]*v.y + m[3][2]*v.z + m[3][3]*v.w
        );
    }

    // Biến đổi điểm 3D (tự động chia phối cảnh perspective divide nếu w != 1)
    Vec3 transformPoint(const Vec3& p) const {
        Vec4 v4(p.x, p.y, p.z, 1.0f);
        Vec4 res = (*this) * v4;
        if (std::abs(res.w) > 1e-6f && std::abs(res.w - 1.0f) > 1e-5f) {
            return Vec3(res.x / res.w, res.y / res.w, res.z / res.w);
        }
        return Vec3(res.x, res.y, res.z);
    }

    // Chuyển sang mảng float 16 phần tử theo thứ tự cột (column-major) dùng cho OpenGL
    void toColumnMajor(float out[16]) const {
        for (int c = 0; c < 4; ++c) {
            for (int r = 0; r < 4; ++r) {
                out[c * 4 + r] = m[r][c];
            }
        }
    }

    Mat4 toMat4() const {
        Mat4 res;
        toColumnMajor(res.m);
        return res;
    }

    static Matrix4x4 fromMat4(const Mat4& mat) {
        Matrix4x4 res;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                res.m[r][c] = mat(r, c);
            }
        }
        return res;
    }

    // ========================================================================
    // 1. MA TRẬN BIẾN ĐỔI HỆ QUAN SÁT (VIEWING / LOOK-AT MATRIX)
    // ========================================================================
    /**
     * Lý thuyết Đồ họa máy tính:
     * Hệ tọa độ quan sát (VCS) được xác định bởi:
     * - Điểm nhìn (Eye / Camera Position): E(xe, ye, ze)
     * - Điểm ngắm (LookAt / Target): T(xt, yt, zt)
     * - Vector hướng lên (Up Vector): Vup
     *
     * Hệ trục trực chuẩn của Camera:
     * 1. Vector hướng nhìn chuẩn hóa: f = (T - E) / ||T - E||
     * 2. Trục X của camera (hướng sang phải): s = (f x Vup) / ||f x Vup||
     * 3. Trục Y của camera (hướng lên thực): u = s x f
     * 4. Trục Z của camera (hướng ngược lại): z_c = -f
     *
     * Ma trận biến đổi quan sát M_view đưa WCS về VCS:
     * [  s.x   s.y   s.z  -dot(s, E) ]
     * [  u.x   u.y   u.z  -dot(u, E) ]
     * [ -f.x  -f.y  -f.z   dot(f, E) ]
     * [  0     0     0         1     ]
     */
    static Matrix4x4 makeLookAt(const Vec3& eye, const Vec3& target, const Vec3& up = Vec3(0, 1, 0)) {
        Vec3 f = (target - eye).normalized();
        Vec3 s = cross(f, up).normalized();
        Vec3 u = cross(s, f);

        Matrix4x4 res = identity();
        res.m[0][0] = s.x;  res.m[0][1] = s.y;  res.m[0][2] = s.z;  res.m[0][3] = -dot(s, eye);
        res.m[1][0] = u.x;  res.m[1][1] = u.y;  res.m[1][2] = u.z;  res.m[1][3] = -dot(u, eye);
        res.m[2][0] = -f.x; res.m[2][1] = -f.y; res.m[2][2] = -f.z; res.m[2][3] = dot(f, eye);
        res.m[3][0] = 0.0f; res.m[3][1] = 0.0f; res.m[3][2] = 0.0f; res.m[3][3] = 1.0f;
        return res;
    }

    // ========================================================================
    // 2. BIẾN ĐỔI GÓC QUAN SÁT THETA (θ) VÀ PHI (Φ) THEO TỌA ĐỘ CẦU
    // ========================================================================
    /**
     * Vị trí quan sát (Eye) di chuyển trên mặt cầu tâm Target với bán kính R:
     * - Góc θ (Theta - Azimuth / Yaw): Góc quay quanh trục thẳng đứng Y [0°, 360°)
     * - Góc Φ (Phi - Elevation / Pitch): Góc ngẩng/cúi so với mặt phẳng XZ [-89°, +89°]
     * - Bán kính R: Khoảng cách từ điểm nhìn tới tâm vật thể
     *
     * Công thức tọa độ cầu sang Descartes:
     *   xe = xt + R * cos(Φ) * sin(θ)
     *   ye = yt + R * sin(Φ)
     *   ze = zt + R * cos(Φ) * cos(θ)
     */
    static Vec3 calculateEyeFromSpherical(const Vec3& target, float radius, float thetaDeg, float phiDeg) {
        float thetaRad = thetaDeg * LAB4_DEG2RAD;
        float phiRad = phiDeg * LAB4_DEG2RAD;
        float cosPhi = std::cos(phiRad);
        float sinPhi = std::sin(phiRad);
        float sinTheta = std::sin(thetaRad);
        float cosTheta = std::cos(thetaRad);

        float xe = target.x + radius * cosPhi * sinTheta;
        float ye = target.y + radius * sinPhi;
        float ze = target.z + radius * cosPhi * cosTheta;
        return Vec3(xe, ye, ze);
    }

    // ========================================================================
    // 3. PHÉP CHIẾU SONG SONG (ORTHOGRAPHIC / PARALLEL PROJECTION)
    // ========================================================================
    /**
     * Các tia chiếu song song nhau, vuông góc với mặt phẳng chiếu.
     * Thể tích nhìn là hình hộp chữ nhật [left, right] x [bottom, top] x [near, far].
     *
     * Ma trận chiếu trực giao:
     * [ 2/(r-l)     0           0       -(r+l)/(r-l) ]
     * [   0      2/(t-b)        0       -(t+b)/(t-b) ]
     * [   0         0       -2/(f-n)    -(f+n)/(f-n) ]
     * [   0         0           0             1      ]
     */
    static Matrix4x4 makeOrthographic(float left, float right, float bottom, float top, float zNear, float zFar) {
        Matrix4x4 res = identity();
        res.m[0][0] = 2.0f / (right - left);
        res.m[1][1] = 2.0f / (top - bottom);
        res.m[2][2] = -2.0f / (zFar - zNear);

        res.m[0][3] = -(right + left) / (right - left);
        res.m[1][3] = -(top + bottom) / (top - bottom);
        res.m[2][3] = -(zFar + zNear) / (zFar - zNear);
        return res;
    }

    static Matrix4x4 makeOrthographicSymmetric(float orthoSize, float aspect, float zNear = 0.1f, float zFar = 100.0f) {
        float halfH = orthoSize;
        float halfW = orthoSize * aspect;
        return makeOrthographic(-halfW, halfW, -halfH, halfH, zNear, zFar);
    }

    // ========================================================================
    // 4. PHÉP CHIẾU PHỐI CẢNH (PERSPECTIVE PROJECTION)
    // ========================================================================
    /**
     * Các tia chiếu hội tụ tại tâm chiếu (Camera).
     * Thể tích nhìn là hình chóp cụt (Frustum). Vật càng ở xa càng nhỏ.
     *
     * Ma trận chiếu phối cảnh:
     * [ 1/(aspect*tan(fov/2))       0                 0                       0             ]
     * [          0            1/tan(fov/2)            0                       0             ]
     * [          0                  0         -(f+n)/(f-n)         -(2*f*n)/(f-n)           ]
     * [          0                  0                -1                       0             ]
     */
    static Matrix4x4 makePerspective(float fovDeg, float aspect, float zNear = 0.1f, float zFar = 100.0f) {
        float fovRad = fovDeg * LAB4_DEG2RAD;
        float tanHalfFov = std::tan(fovRad * 0.5f);

        Matrix4x4 res;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                res.m[r][c] = 0.0f;
            }
        }

        res.m[0][0] = 1.0f / (aspect * tanHalfFov);
        res.m[1][1] = 1.0f / tanHalfFov;
        res.m[2][2] = -(zFar + zNear) / (zFar - zNear);
        res.m[2][3] = -(2.0f * zFar * zNear) / (zFar - zNear);
        res.m[3][2] = -1.0f;
        res.m[3][3] = 0.0f;
        return res;
    }

    // ========================================================================
    // 5. CÁC PHÉP BIẾN ĐỔI VẬT THỂ (MODEL TRANSFORMATIONS)
    // ========================================================================
    // Phép tỉ lệ / phóng to, thu nhỏ: S(sx, sy, sz)
    static Matrix4x4 makeScale(float sx, float sy, float sz) {
        Matrix4x4 res = identity();
        res.m[0][0] = sx;
        res.m[1][1] = sy;
        res.m[2][2] = sz;
        return res;
    }

    static Matrix4x4 makeScale(float s) {
        return makeScale(s, s, s);
    }

    // Phép tịnh tiến: T(tx, ty, tz)
    static Matrix4x4 makeTranslation(float tx, float ty, float tz) {
        Matrix4x4 res = identity();
        res.m[0][3] = tx;
        res.m[1][3] = ty;
        res.m[2][3] = tz;
        return res;
    }

    static Matrix4x4 makeTranslation(const Vec3& t) {
        return makeTranslation(t.x, t.y, t.z);
    }

    // Phép quay quanh các trục
    static Matrix4x4 makeRotationX(float angleDeg) {
        float rad = angleDeg * LAB4_DEG2RAD;
        float c = std::cos(rad);
        float s = std::sin(rad);
        Matrix4x4 res = identity();
        res.m[1][1] = c;  res.m[1][2] = -s;
        res.m[2][1] = s;  res.m[2][2] = c;
        return res;
    }

    static Matrix4x4 makeRotationY(float angleDeg) {
        float rad = angleDeg * LAB4_DEG2RAD;
        float c = std::cos(rad);
        float s = std::sin(rad);
        Matrix4x4 res = identity();
        res.m[0][0] = c;  res.m[0][2] = s;
        res.m[2][0] = -s; res.m[2][2] = c;
        return res;
    }

    static Matrix4x4 makeRotationZ(float angleDeg) {
        float rad = angleDeg * LAB4_DEG2RAD;
        float c = std::cos(rad);
        float s = std::sin(rad);
        Matrix4x4 res = identity();
        res.m[0][0] = c;  res.m[0][1] = -s;
        res.m[1][0] = s;  res.m[1][1] = c;
        return res;
    }

    // ========================================================================
    // ĐỊNH DẠNG VÀ XUẤT MA TRẬN ĐẸP RA CHUỖI
    // ========================================================================
    std::string format(const std::string& title = "Matrix 4x4") const {
        std::stringstream ss;
        ss << title << ":\n";
        for (int r = 0; r < 4; ++r) {
            ss << " [ ";
            for (int c = 0; c < 4; ++c) {
                ss << std::fixed << std::setprecision(3) << std::setw(7) << m[r][c] << " ";
            }
            ss << "]\n";
        }
        return ss.str();
    }
};

// ============================================================================
// CHẾ ĐỘ PHÉP CHIẾU & MÔ HÌNH QUAN SÁT
// ============================================================================
enum ProjectionMode {
    PROJECTION_PERSPECTIVE = 0, // Phép chiếu phối cảnh
    PROJECTION_PARALLEL = 1     // Phép chiếu song song (Trực giao)
};

enum DisplayModelMode {
    MODEL_WIREFRAME = 0,        // 4.1. Mô hình khung kết nối
    MODEL_POLYGON_SOLID = 1,    // 4.2. Mô hình mặt đa giác
    MODEL_SOLID_AND_WIRE = 2,   // Kết hợp: Mặt đa giác + Khung viền cạnh
    MODEL_POINTS = 3            // Chỉ hiển thị các đỉnh
};

// ============================================================================
// HỆ THỐNG KIỂM THỬ TỰ ĐỘNG (UNIT TESTS) CHO LAB 04
// ============================================================================
inline bool runLab4Tests() {
    std::cout << "\n=======================================================\n";
    std::cout << "  RUNNING AUTOMATED UNIT TESTS: LAB 04 3D VIEWING      \n";
    std::cout << "=======================================================\n";

    bool allPassed = true;

    // Test 1: Ma trận đơn vị 4x4
    {
        Matrix4x4 I = Matrix4x4::identity();
        Vec3 p(3.0f, -4.0f, 5.0f);
        Vec3 res = I.transformPoint(p);
        bool ok = (std::abs(res.x - p.x) < 1e-4f &&
                   std::abs(res.y - p.y) < 1e-4f &&
                   std::abs(res.z - p.z) < 1e-4f);
        std::cout << " [TEST 1] 4x4 Identity Matrix Transform: "
                  << (ok ? "PASSED" : "FAILED") << "\n";
        if (!ok) allPassed = false;
    }

    // Test 2: Tọa độ cầu sang điểm nhìn Eye (θ, Φ, R)
    {
        // theta = 0, phi = 0 -> Eye = (0, 0, R)
        Vec3 eye1 = Matrix4x4::calculateEyeFromSpherical(Vec3(0, 0, 0), 5.0f, 0.0f, 0.0f);
        bool ok1 = std::abs(eye1.x) < 1e-4f && std::abs(eye1.y) < 1e-4f && std::abs(eye1.z - 5.0f) < 1e-4f;

        // theta = 90, phi = 0 -> Eye = (R, 0, 0)
        Vec3 eye2 = Matrix4x4::calculateEyeFromSpherical(Vec3(0, 0, 0), 5.0f, 90.0f, 0.0f);
        bool ok2 = std::abs(eye2.x - 5.0f) < 1e-4f && std::abs(eye2.y) < 1e-4f && std::abs(eye2.z) < 1e-4f;

        // phi = 90 -> Eye = (0, R, 0)
        Vec3 eye3 = Matrix4x4::calculateEyeFromSpherical(Vec3(0, 0, 0), 5.0f, 0.0f, 90.0f);
        bool ok3 = std::abs(eye3.x) < 1e-4f && std::abs(eye3.y - 5.0f) < 1e-4f && std::abs(eye3.z) < 1e-4f;

        bool ok = ok1 && ok2 && ok3;
        std::cout << " [TEST 2] Spherical Orbit to Eye Position (Theta, Phi, R): "
                  << (ok ? "PASSED" : "FAILED") << "\n";
        if (!ok) allPassed = false;
    }

    // Test 3: Ma trận quan sát LookAt
    {
        Vec3 eye(0.0f, 0.0f, 5.0f);
        Vec3 target(0.0f, 0.0f, 0.0f);
        Matrix4x4 view = Matrix4x4::makeLookAt(eye, target, Vec3(0, 1, 0));

        // Điểm Eye sau khi qua View Matrix phải biến đổi về gốc tọa độ hệ camera (0, 0, 0)
        Vec3 eyeInVCS = view.transformPoint(eye);
        bool okEye = eyeInVCS.lengthSq() < 1e-4f;

        // Điểm Target phải nằm trên trục -Z ở khoảng cách 5 đơn vị: (0, 0, -5)
        Vec3 targetInVCS = view.transformPoint(target);
        bool okTarget = std::abs(targetInVCS.x) < 1e-4f &&
                        std::abs(targetInVCS.y) < 1e-4f &&
                        std::abs(targetInVCS.z - (-5.0f)) < 1e-4f;

        bool ok = okEye && okTarget;
        std::cout << " [TEST 3] Viewing Transformation Matrix (LookAt): "
                  << (ok ? "PASSED" : "FAILED") << "\n";
        if (!ok) allPassed = false;
    }

    // Test 4: Phép chiếu song song (Orthographic)
    {
        Matrix4x4 ortho = Matrix4x4::makeOrthographic(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 20.0f);
        Vec3 center(0.0f, 0.0f, -10.0f);
        Vec3 corner(10.0f, 10.0f, 0.0f);
        Vec3 cNDC = ortho.transformPoint(center);
        Vec3 corNDC = ortho.transformPoint(corner);

        bool ok = std::abs(cNDC.x) < 1e-4f && std::abs(cNDC.y) < 1e-4f &&
                  std::abs(corNDC.x - 1.0f) < 1e-4f && std::abs(corNDC.y - 1.0f) < 1e-4f;
        std::cout << " [TEST 4] Orthographic (Parallel) Projection Matrix: "
                  << (ok ? "PASSED" : "FAILED") << "\n";
        if (!ok) allPassed = false;
    }

    // Test 5: Phép chiếu phối cảnh (Perspective)
    {
        Matrix4x4 persp = Matrix4x4::makePerspective(60.0f, 1.0f, 1.0f, 100.0f);
        // Điểm trên trục Z âm z = -1 (near plane)
        Vec4 nearPt(0.0f, 0.0f, -1.0f, 1.0f);
        Vec4 resNear = persp * nearPt;
        float zNearNDC = resNear.z / resNear.w;

        // Điểm trên trục Z âm z = -100 (far plane)
        Vec4 farPt(0.0f, 0.0f, -100.0f, 1.0f);
        Vec4 resFar = persp * farPt;
        float zFarNDC = resFar.z / resFar.w;

        bool ok = (std::abs(zNearNDC - (-1.0f)) < 1e-3f) && (std::abs(zFarNDC - 1.0f) < 1e-3f);
        std::cout << " [TEST 5] Perspective Projection Matrix (Frustum Mapping): "
                  << (ok ? "PASSED" : "FAILED") << "\n";
        if (!ok) allPassed = false;
    }

    // Test 6: Ma trận phóng to / thu nhỏ vật thể (Scale)
    {
        Matrix4x4 scale = Matrix4x4::makeScale(2.5f);
        Vec3 p(2.0f, 3.0f, -4.0f);
        Vec3 scaledP = scale.transformPoint(p);
        bool ok = (std::abs(scaledP.x - 5.0f) < 1e-4f &&
                   std::abs(scaledP.y - 7.5f) < 1e-4f &&
                   std::abs(scaledP.z - (-10.0f)) < 1e-4f);
        std::cout << " [TEST 6] 3D Scaling Transformation Matrix (Zoom/Scale): "
                  << (ok ? "PASSED" : "FAILED") << "\n";
        if (!ok) allPassed = false;
    }

    std::cout << "=======================================================\n";
    std::cout << " LAB 04 TESTS RESULT: " << (allPassed ? "ALL PASSED (100%)" : "FAILED") << "\n";
    std::cout << "=======================================================\n\n";

    return allPassed;
}

} // namespace Lab4

#endif // LAB4_TRANSFORMATIONS_HPP
