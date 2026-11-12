#ifndef LAB3_TRANSFORMATIONS_HPP
#define LAB3_TRANSFORMATIONS_HPP

#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace Lab3 {

constexpr float PI = 3.14159265358979323846f;
constexpr float DEG2RAD = PI / 180.0f;
constexpr float RAD2DEG = 180.0f / PI;

struct Point2D {
    float x = 0.0f;
    float y = 0.0f;

    Point2D() = default;
    Point2D(float _x, float _y) : x(_x), y(_y) {}

    bool operator==(const Point2D& o) const {
        return std::abs(x - o.x) < 1e-3f && std::abs(y - o.y) < 1e-3f;
    }
};

// ============================================================================
// MA TRẬN ĐỒNG NHẤT 3x3 (HOMOGENEOUS 2D TRANSFORMATION MATRIX)
// ============================================================================
/**
 * Biểu diễn ma trận 3x3 theo thứ tự dòng và cột:
 * [ m[0][0]  m[0][1]  m[0][2] ]   [ x ]   [ x' ]
 * [ m[1][0]  m[1][1]  m[1][2] ] * [ y ] = [ y' ]
 * [ m[2][0]  m[2][1]  m[2][2] ]   [ 1 ]   [ 1  ]
 */
struct Matrix3x3 {
    float m[3][3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    };

    static Matrix3x3 identity() {
        Matrix3x3 res;
        return res;
    }

    // Nhân hai ma trận 3x3: C = A * B
    Matrix3x3 operator*(const Matrix3x3& b) const {
        Matrix3x3 res;
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                res.m[r][c] = m[r][0] * b.m[0][c] +
                              m[r][1] * b.m[1][c] +
                              m[r][2] * b.m[2][c];
            }
        }
        return res;
    }

    // Nhân ma trận với điểm 2D đồng nhất [x, y, 1]^T
    Point2D transformPoint(const Point2D& p) const {
        float nx = m[0][0] * p.x + m[0][1] * p.y + m[0][2];
        float ny = m[1][0] * p.x + m[1][1] * p.y + m[1][2];
        float nw = m[2][0] * p.x + m[2][1] * p.y + m[2][2];
        if (std::abs(nw) > 1e-6f && std::abs(nw - 1.0f) > 1e-5f) {
            nx /= nw;
            ny /= nw;
        }
        return Point2D(nx, ny);
    }

    std::vector<Point2D> transformPolygon(const std::vector<Point2D>& poly) const {
        std::vector<Point2D> result;
        result.reserve(poly.size());
        for (const auto& pt : poly) {
            result.push_back(transformPoint(pt));
        }
        return result;
    }

    // ========================================================================
    // CÁC HÀM TẠO MA TRẬN BIẾN ĐỔI 2D CHUẨN
    // ========================================================================

    /**
     * 1. Phép tịnh tiến (Translation) theo vector (tx, ty)
     * [ 1  0  tx ]
     * [ 0  1  ty ]
     * [ 0  0  1  ]
     */
    static Matrix3x3 makeTranslation(float tx, float ty) {
        Matrix3x3 res;
        res.m[0][2] = tx;
        res.m[1][2] = ty;
        return res;
    }

    /**
     * 2. Phép tỉ lệ với tâm tỉ lệ (x0, y0) và hệ số (sx, sy)
     * Ma trận kết hợp: T(x0, y0) * S(sx, sy) * T(-x0, -y0)
     * [ sx   0    x0*(1 - sx) ]
     * [ 0    sy   y0*(1 - sy) ]
     * [ 0    0    1           ]
     */
    static Matrix3x3 makeScaling(float sx, float sy, float x0 = 0.0f, float y0 = 0.0f) {
        Matrix3x3 res;
        res.m[0][0] = sx;
        res.m[0][2] = x0 * (1.0f - sx);
        res.m[1][1] = sy;
        res.m[1][2] = y0 * (1.0f - sy);
        return res;
    }

    /**
     * 3. Phép quay quanh tâm quay (x0, y0) góc alpha (độ)
     * Ma trận kết hợp: T(x0, y0) * R(theta) * T(-x0, -y0)
     * [ cos(a)  -sin(a)   x0*(1 - cos(a)) + y0*sin(a) ]
     * [ sin(a)   cos(a)   y0*(1 - cos(a)) - x0*sin(a) ]
     * [ 0        0        1                           ]
     */
    static Matrix3x3 makeRotation(float angleDegrees, float x0 = 0.0f, float y0 = 0.0f) {
        float rad = angleDegrees * DEG2RAD;
        float c = std::cos(rad);
        float s = std::sin(rad);

        Matrix3x3 res;
        res.m[0][0] = c;
        res.m[0][1] = -s;
        res.m[0][2] = x0 * (1.0f - c) + y0 * s;

        res.m[1][0] = s;
        res.m[1][1] = c;
        res.m[1][2] = y0 * (1.0f - c) - x0 * s;

        return res;
    }

    /**
     * 4. Phép đối xứng qua đường thẳng y = ax + b (Reflection across line y = ax + b)
     * 
     * Chứng minh qua chuỗi 5 phép biến đổi cơ bản:
     * 1. T(0, -b): Tịnh tiến giao điểm (0, b) về gốc tọa độ O
     * 2. R(-theta): Quay góc -theta đưa đường thẳng trùng trục Ox (với tan(theta) = a)
     * 3. Ref_x: Lấy đối xứng qua trục Ox [1, 0, 0; 0, -1, 0; 0, 0, 1]
     * 4. R(theta): Quay ngược lại góc +theta
     * 5. T(0, b): Tịnh tiến ngược lại (0, b)
     * 
     * Kết quả tổng hợp:
     * [ (1 - a^2)/(1 + a^2)     (2a)/(1 + a^2)      (-2ab)/(1 + a^2) ]
     * [ (2a)/(1 + a^2)          (a^2 - 1)/(1 + a^2) (2b)/(1 + a^2)   ]
     * [ 0                       0                   1                ]
     */
    static Matrix3x3 makeReflectionLine(float a, float b) {
        float a2 = a * a;
        float denom = 1.0f + a2;

        Matrix3x3 res;
        res.m[0][0] = (1.0f - a2) / denom;
        res.m[0][1] = (2.0f * a) / denom;
        res.m[0][2] = (-2.0f * a * b) / denom;

        res.m[1][0] = (2.0f * a) / denom;
        res.m[1][1] = (a2 - 1.0f) / denom;
        res.m[1][2] = (2.0f * b) / denom;

        res.m[2][0] = 0.0f;
        res.m[2][1] = 0.0f;
        res.m[2][2] = 1.0f;

        return res;
    }

    /**
     * Trường hợp đường thẳng thẳng đứng x = c (tương ứng a = vô cùng)
     * x' = 2c - x, y' = y
     */
    static Matrix3x3 makeReflectionVertical(float c) {
        Matrix3x3 res;
        res.m[0][0] = -1.0f;
        res.m[0][2] = 2.0f * c;
        res.m[1][1] = 1.0f;
        return res;
    }

    // In ma trận dạng chuỗi 3 dòng đẹp mắt để hiển thị lên HUD
    std::string toString() const {
        std::stringstream ss;
        for (int r = 0; r < 3; ++r) {
            ss << "[ ";
            for (int c = 0; c < 3; ++c) {
                ss << std::fixed << std::setprecision(2) << std::setw(6) << m[r][c] << " ";
            }
            ss << "]\n";
        }
        return ss.str();
    }
};

// ============================================================================
// HÌNH HỌC MẪU CHO LAB 3
// ============================================================================
inline std::vector<Point2D> createLetterFShape() {
    // Chữ 'F' kinh điển trong sách giáo trình Computer Graphics
    return {
        Point2D(0, 0), Point2D(50, 0), Point2D(50, 15), Point2D(15, 15),
        Point2D(15, 35), Point2D(40, 35), Point2D(40, 50), Point2D(15, 50),
        Point2D(15, 70), Point2D(55, 70), Point2D(55, 85), Point2D(0, 85)
    };
}

inline std::vector<Point2D> createSpaceshipShape() {
    // Phi thuyền bất đối xứng, rất rõ chiều quay và đối xứng
    return {
        Point2D(0, 70),     // Mũi nhọn phía trên
        Point2D(18, 20),
        Point2D(55, -40),   // Cánh phải
        Point2D(20, -25),
        Point2D(12, -55),   // Đuôi phải
        Point2D(0, -40),    // Giữa đuôi
        Point2D(-12, -55),  // Đuôi trái
        Point2D(-20, -25),
        Point2D(-55, -40),  // Cánh trái
        Point2D(-18, 20)
    };
}

inline std::vector<Point2D> createTriangleShape() {
    return {
        Point2D(0, 60), Point2D(60, -40), Point2D(-60, -40)
    };
}

inline std::vector<Point2D> createStarShape() {
    std::vector<Point2D> pts;
    int points = 5;
    float rOuter = 65.0f;
    float rInner = 28.0f;
    for (int i = 0; i < points * 2; ++i) {
        float angle = (i * PI / points) - (PI * 0.5f);
        float r = (i % 2 == 0) ? rOuter : rInner;
        pts.push_back(Point2D(r * std::cos(angle), r * std::sin(angle)));
    }
    return pts;
}

// ============================================================================
// BỘ KIỂM THỬ TỰ ĐỘNG LAB 3 (UNIT TESTS)
// ============================================================================
inline bool runLab3Tests() {
    std::cout << "\n=======================================================\n";
    std::cout << "  RUNNING AUTOMATED UNIT TESTS: LAB 03 2D TRANSFORMS   \n";
    std::cout << "=======================================================\n";

    bool allPassed = true;

    // Test 1: Tịnh tiến (Translation)
    {
        Matrix3x3 T = Matrix3x3::makeTranslation(50.0f, -30.0f);
        Point2D p(10.0f, 20.0f);
        Point2D res = T.transformPoint(p);
        bool ok = (std::abs(res.x - 60.0f) < 1e-3f && std::abs(res.y - (-10.0f)) < 1e-3f);
        std::cout << " [TEST 1] Translation (10,20) + (50,-30) -> (60,-10): " 
                  << (ok ? "PASSED" : "FAILED") << "\n";
        if (!ok) allPassed = false;
    }

    // Test 2: Tỉ lệ với tâm tỉ lệ (x0, y0)
    {
        Point2D pivot(20.0f, 30.0f);
        Matrix3x3 S = Matrix3x3::makeScaling(2.0f, 3.0f, pivot.x, pivot.y);
        // Tâm tỉ lệ phải là điểm bất biến: S * pivot = pivot
        Point2D pRes = S.transformPoint(pivot);
        bool okPivot = (pRes == pivot);

        // Điểm khác: P(30, 40) -> x' = 20 + (30-20)*2 = 40, y' = 30 + (40-30)*3 = 60
        Point2D q(30.0f, 40.0f);
        Point2D qRes = S.transformPoint(q);
        bool okPoint = (std::abs(qRes.x - 40.0f) < 1e-3f && std::abs(qRes.y - 60.0f) < 1e-3f);

        bool ok = okPivot && okPoint;
        std::cout << " [TEST 2] Scaling with Pivot (20,30): " 
                  << (ok ? "PASSED" : "FAILED") << "\n";
        if (!ok) allPassed = false;
    }

    // Test 3: Quay với tâm quay (x0, y0)
    {
        Point2D pivot(15.0f, 25.0f);
        Matrix3x3 R90 = Matrix3x3::makeRotation(90.0f, pivot.x, pivot.y);
        // Tâm quay phải bất biến
        Point2D pRes = R90.transformPoint(pivot);
        bool okPivot = (pRes == pivot);

        // Quay điểm (25, 25) 90 độ quanh (15, 25): dx = 10, dy = 0 -> dx' = 0, dy' = 10 -> (15, 35)
        Point2D q(25.0f, 25.0f);
        Point2D qRes = R90.transformPoint(q);
        bool okPoint = (std::abs(qRes.x - 15.0f) < 1e-3f && std::abs(qRes.y - 35.0f) < 1e-3f);

        // Quay 360 độ quanh tâm -> trở lại ban đầu
        Matrix3x3 R360 = Matrix3x3::makeRotation(360.0f, pivot.x, pivot.y);
        Point2D q360 = R360.transformPoint(q);
        bool ok360 = (q360 == q);

        bool ok = okPivot && okPoint && ok360;
        std::cout << " [TEST 3] Rotation with Pivot (15,25) 90 deg: " 
                  << (ok ? "PASSED" : "FAILED") << "\n";
        if (!ok) allPassed = false;
    }

    // Test 4: Đối xứng qua đường thẳng y = ax + b
    {
        // 4A: Đối xứng qua đường phân giác y = x (a = 1, b = 0): biến (x, y) -> (y, x)
        Matrix3x3 RefDiag = Matrix3x3::makeReflectionLine(1.0f, 0.0f);
        Point2D pA(40.0f, 15.0f);
        Point2D pARes = RefDiag.transformPoint(pA);
        bool okA = (std::abs(pARes.x - 15.0f) < 1e-3f && std::abs(pARes.y - 40.0f) < 1e-3f);

        // 4B: Điểm trên đường thẳng y = 2x + 10 phải giữ nguyên tọa độ
        float a = 2.0f, b = 10.0f;
        Matrix3x3 RefLine = Matrix3x3::makeReflectionLine(a, b);
        Point2D pOnLine(5.0f, 2.0f * 5.0f + 10.0f); // (5, 20)
        Point2D pOnRes = RefLine.transformPoint(pOnLine);
        bool okB = (pOnRes == pOnLine);

        // 4C: Đối xứng 2 lần liên tiếp phải quay về điểm ban đầu (M * M = I)
        Matrix3x3 RefTwice = RefLine * RefLine;
        Point2D pRand(73.0f, -42.0f);
        Point2D pTwiceRes = RefTwice.transformPoint(pRand);
        bool okC = (pTwiceRes == pRand);

        bool ok = okA && okB && okC;
        std::cout << " [TEST 4] Reflection across line y = ax + b: " 
                  << (ok ? "PASSED" : "FAILED") << "\n";
        if (!ok) allPassed = false;
    }

    std::cout << "=======================================================\n";
    std::cout << " LAB 03 TESTS RESULT: " << (allPassed ? "ALL PASSED (100%)" : "SOME FAILED") << "\n";
    std::cout << "=======================================================\n\n";

    return allPassed;
}

} // namespace Lab3

#endif // LAB3_TRANSFORMATIONS_HPP
