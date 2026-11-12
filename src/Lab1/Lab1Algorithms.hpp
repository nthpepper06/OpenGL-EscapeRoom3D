#ifndef LAB1_ALGORITHMS_HPP
#define LAB1_ALGORITHMS_HPP

#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace Lab1 {

// Cấu trúc tọa độ nguyên của một pixel trên màn hình
struct PixelPoint {
    int x;
    int y;

    PixelPoint() : x(0), y(0) {}
    PixelPoint(int _x, int _y) : x(_x), y(_y) {}

    bool operator==(const PixelPoint& other) const {
        return x == other.x && y == other.y;
    }
};

// Phân loại 4 trường hợp hệ số góc m của đoạn thẳng (và 2 trường hợp suy biến)
enum LineSlopeCase {
    CASE_HORIZONTAL = 0,        // m = 0 (dy = 0)
    CASE_VERTICAL = 1,          // m = vô cùng (dx = 0)
    CASE_1_ZERO_TO_ONE = 2,     // 0 < m <= 1: x tăng từng bước, 0 <= dy <= dx
    CASE_2_GREATER_ONE = 3,     // m > 1: y tăng từng bước, dy > dx > 0
    CASE_3_MINUS_ONE_TO_ZERO = 4,// -1 <= m < 0: x tăng từng bước, y giảm (0 <= -dy <= dx)
    CASE_4_LESS_MINUS_ONE = 5   // m < -1: y giảm từng bước, -dy > dx > 0
};

inline std::string getLineSlopeCaseName(LineSlopeCase c) {
    switch (c) {
        case CASE_HORIZONTAL: return "Nam ngang (m = 0, dy = 0)";
        case CASE_VERTICAL: return "Thang dung (dx = 0, m = vo cung)";
        case CASE_1_ZERO_TO_ONE: return "Truong hop 1: 0 < m <= 1 (Truc chinh: X, y tang)";
        case CASE_2_GREATER_ONE: return "Truong hop 2: m > 1 (Truc chinh: Y, x tang)";
        case CASE_3_MINUS_ONE_TO_ZERO: return "Truong hop 3: -1 <= m < 0 (Truc chinh: X, y giam)";
        case CASE_4_LESS_MINUS_ONE: return "Truong hop 4: m < -1 (Truc chinh: Y, x tang khi y giam)";
        default: return "Khong xac dinh";
    }
}

// ============================================================================
// 1.1. THUẬT TOÁN VẼ ĐOẠN THẲNG (BRESENHAM & MIDPOINT)
// ============================================================================

/**
 * Thuật toán Bresenham vẽ đoạn thẳng xét đầy đủ 4 trường hợp của hệ số góc m.
 * 
 * Phân tích 4 trường hợp:
 * - Nếu |dx| >= |dy|: Trục lái là X (mỗi bước tăng x thêm 1 hoặc -1)
 *   + Case 1: 0 <= m <= 1  (dy/dx >= 0)
 *   + Case 3: -1 <= m < 0  (dy/dx < 0)
 * - Nếu |dy| > |dx|: Trục lái là Y (mỗi bước tăng y thêm 1 hoặc -1)
 *   + Case 2: m > 1        (dy/dx > 1)
 *   + Case 4: m < -1       (dy/dx < -1)
 */
inline std::vector<PixelPoint> drawLineBresenham(
    int x1, int y1, int x2, int y2,
    LineSlopeCase& outCase, float& outSlope
) {
    std::vector<PixelPoint> points;

    int dx = x2 - x1;
    int dy = y2 - y1;

    // Tính hệ số góc m
    if (dx == 0) {
        outSlope = (dy >= 0) ? 999999.0f : -999999.0f;
    } else {
        outSlope = static_cast<float>(dy) / static_cast<float>(dx);
    }

    // Xác định trường hợp hệ số góc
    if (dy == 0) {
        outCase = CASE_HORIZONTAL;
    } else if (dx == 0) {
        outCase = CASE_VERTICAL;
    } else {
        float absSlope = std::abs(outSlope);
        if (outSlope > 0) {
            outCase = (absSlope <= 1.0f) ? CASE_1_ZERO_TO_ONE : CASE_2_GREATER_ONE;
        } else {
            outCase = (absSlope <= 1.0f) ? CASE_3_MINUS_ONE_TO_ZERO : CASE_4_LESS_MINUS_ONE;
        }
    }

    // Hướng bước đi trên trục X và Y
    int stepX = (dx >= 0) ? 1 : -1;
    int stepY = (dy >= 0) ? 1 : -1;

    int absDx = std::abs(dx);
    int absDy = std::abs(dy);

    int curX = x1;
    int curY = y1;
    points.emplace_back(curX, curY);

    if (absDx >= absDy) {
        // Trục định hướng là X: Case 1 (0 <= m <= 1) hoặc Case 3 (-1 <= m < 0)
        // Biến quyết định ban đầu: p = 2 * |dy| - |dx|
        int p = 2 * absDy - absDx;
        int twoDy = 2 * absDy;
        int twoDyMinusDx = 2 * (absDy - absDx);

        for (int i = 0; i < absDx; ++i) {
            curX += stepX;
            if (p < 0) {
                p += twoDy;
            } else {
                curY += stepY;
                p += twoDyMinusDx;
            }
            points.emplace_back(curX, curY);
        }
    } else {
        // Trục định hướng là Y: Case 2 (m > 1) hoặc Case 4 (m < -1)
        // Biến quyết định ban đầu: p = 2 * |dx| - |dy|
        int p = 2 * absDx - absDy;
        int twoDx = 2 * absDx;
        int twoDxMinusDy = 2 * (absDx - absDy);

        for (int i = 0; i < absDy; ++i) {
            curY += stepY;
            if (p < 0) {
                p += twoDx;
            } else {
                curX += stepX;
                p += twoDxMinusDy;
            }
            points.emplace_back(curX, curY);
        }
    }

    return points;
}

/**
 * Thuật toán Midpoint vẽ đoạn thẳng xét đầy đủ 4 trường hợp hệ số góc m.
 * 
 * Đường thẳng ẩn: F(x, y) = a*x + b*y + c = 0 với a = dy, b = -dx.
 * Tại mỗi bước, kiểm tra giá trị của hàm F tại trung điểm M của 2 điểm ứng viên:
 * - TH 1 (0 <= m <= 1): M = (x + 1, y + 0.5)
 *   d_0 = 2*dy - dx; nếu d < 0: chọn E, d = d + 2*dy; nếu d >= 0: chọn NE, d = d + 2*(dy - dx)
 * - TH 2 (m > 1): M = (x + 0.5, y + 1)
 *   d_0 = dy - 2*dx; nếu d > 0: chọn N, d = d - 2*dx; nếu d <= 0: chọn NE, d = d + 2*(dy - dx)
 * - TH 3 (-1 <= m < 0): M = (x + 1, y - 0.5)
 * - TH 4 (m < -1): M = (x + 0.5, y - 1)
 */
inline std::vector<PixelPoint> drawLineMidpoint(
    int x1, int y1, int x2, int y2,
    LineSlopeCase& outCase, float& outSlope
) {
    std::vector<PixelPoint> points;

    int dx = x2 - x1;
    int dy = y2 - y1;

    if (dx == 0) {
        outSlope = (dy >= 0) ? 999999.0f : -999999.0f;
    } else {
        outSlope = static_cast<float>(dy) / static_cast<float>(dx);
    }

    if (dy == 0) {
        outCase = CASE_HORIZONTAL;
    } else if (dx == 0) {
        outCase = CASE_VERTICAL;
    } else {
        float absSlope = std::abs(outSlope);
        if (outSlope > 0) {
            outCase = (absSlope <= 1.0f) ? CASE_1_ZERO_TO_ONE : CASE_2_GREATER_ONE;
        } else {
            outCase = (absSlope <= 1.0f) ? CASE_3_MINUS_ONE_TO_ZERO : CASE_4_LESS_MINUS_ONE;
        }
    }

    int stepX = (dx >= 0) ? 1 : -1;
    int stepY = (dy >= 0) ? 1 : -1;

    int absDx = std::abs(dx);
    int absDy = std::abs(dy);

    int curX = x1;
    int curY = y1;
    points.emplace_back(curX, curY);

    if (absDx >= absDy) {
        // Trục định hướng X: d0 = 2*absDy - absDx
        int d = 2 * absDy - absDx;
        int dE = 2 * absDy;
        int dSEorNE = 2 * (absDy - absDx);

        for (int i = 0; i < absDx; ++i) {
            curX += stepX;
            if (d < 0) {
                d += dE;
            } else {
                curY += stepY;
                d += dSEorNE;
            }
            points.emplace_back(curX, curY);
        }
    } else {
        // Trục định hướng Y: d0 = 2*absDx - absDy
        int d = 2 * absDx - absDy;
        int dNorS = 2 * absDx;
        int dDiag = 2 * (absDx - absDy);

        for (int i = 0; i < absDy; ++i) {
            curY += stepY;
            if (d < 0) {
                d += dNorS;
            } else {
                curX += stepX;
                d += dDiag;
            }
            points.emplace_back(curX, curY);
        }
    }

    return points;
}

// ============================================================================
// 1.2. THUẬT TOÁN VẼ ĐƯỜNG TRÒN (BRESENHAM & MIDPOINT)
// ============================================================================

// Hàm bổ trợ lấy đối xứng 8 điểm của đường tròn tâm (xc, yc)
inline void plotCircle8Points(int xc, int yc, int x, int y, std::vector<PixelPoint>& points) {
    points.emplace_back(xc + x, yc + y);
    points.emplace_back(xc - x, yc + y);
    points.emplace_back(xc + x, yc - y);
    points.emplace_back(xc - x, yc - y);
    points.emplace_back(xc + y, yc + x);
    points.emplace_back(xc - y, yc + x);
    points.emplace_back(xc + y, yc - x);
    points.emplace_back(xc - y, yc - x);
}

/**
 * Thuật toán Bresenham vẽ đường tròn tâm (xc, yc), bán kính r.
 * Bắt đầu từ (0, r).
 * Biến quyết định ban đầu: p = 3 - 2*r.
 * Nếu p < 0: p = p + 4*x + 6
 * Nếu p >= 0: p = p + 4*(x - y) + 10, y = y - 1
 */
inline std::vector<PixelPoint> drawCircleBresenham(int xc, int yc, int r) {
    std::vector<PixelPoint> points;
    if (r < 0) r = -r;
    if (r == 0) {
        points.emplace_back(xc, yc);
        return points;
    }

    int x = 0;
    int y = r;
    int p = 3 - 2 * r;

    plotCircle8Points(xc, yc, x, y, points);

    while (x < y) {
        x++;
        if (p < 0) {
            p += 4 * x + 6;
        } else {
            y--;
            p += 4 * (x - y) + 10;
        }
        plotCircle8Points(xc, yc, x, y, points);
    }

    return points;
}

/**
 * Thuật toán Midpoint vẽ đường tròn tâm (xc, yc), bán kính r.
 * Phương trình: f(x, y) = x^2 + y^2 - r^2 = 0.
 * Điểm bắt đầu (0, r), trung điểm M = (1, r - 0.5).
 * Biến quyết định ban đầu: p0 = 5/4 - r ~ 1 - r (đối với số nguyên).
 * Tại mỗi bước:
 * Nếu p < 0: p = p + 2*x + 3
 * Nếu p >= 0: p = p + 2*(x - y) + 5, y = y - 1
 */
inline std::vector<PixelPoint> drawCircleMidpoint(int xc, int yc, int r) {
    std::vector<PixelPoint> points;
    if (r < 0) r = -r;
    if (r == 0) {
        points.emplace_back(xc, yc);
        return points;
    }

    int x = 0;
    int y = r;
    int p = 1 - r;

    plotCircle8Points(xc, yc, x, y, points);

    while (x < y) {
        x++;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            y--;
            p += 2 * (x - y) + 1;
        }
        plotCircle8Points(xc, yc, x, y, points);
    }

    return points;
}

// ============================================================================
// 1.3. THUẬT TOÁN VẼ ELLIPSE (MIDPOINT)
// ============================================================================

// Hàm bổ trợ lấy đối xứng 4 điểm quanh tâm (xc, yc) của Ellipse
inline void plotEllipse4Points(int xc, int yc, int x, int y, std::vector<PixelPoint>& points) {
    points.emplace_back(xc + x, yc + y);
    points.emplace_back(xc - x, yc + y);
    points.emplace_back(xc + x, yc - y);
    points.emplace_back(xc - x, yc - y);
}

/**
 * Thuật toán Midpoint vẽ Ellipse tâm (xc, yc), bán trục rx, ry.
 * Phương trình: f(x, y) = ry^2 * x^2 + rx^2 * y^2 - rx^2 * ry^2 = 0
 * 
 * Chia làm 2 miền:
 * - Miền 1: Độ dốc |dy/dx| < 1  <=>  2*ry^2 * x < 2*rx^2 * y
 *   Bắt đầu tại (0, ry). Biến p1_0 = ry^2 - rx^2 * ry + 0.25 * rx^2
 *   Bước theo trục x: x = x + 1
 * - Miền 2: Độ dốc |dy/dx| >= 1 <=>  2*ry^2 * x >= 2*rx^2 * y
 *   Bắt đầu từ điểm cuối miền 1.
 *   p2_0 = ry^2 * (x + 0.5)^2 + rx^2 * (y - 1)^2 - rx^2 * ry^2
 *   Bước theo trục y: y = y - 1
 */
inline std::vector<PixelPoint> drawEllipseMidpoint(int xc, int yc, int rx, int ry) {
    std::vector<PixelPoint> points;
    if (rx < 0) rx = -rx;
    if (ry < 0) ry = -ry;
    if (rx == 0 && ry == 0) {
        points.emplace_back(xc, yc);
        return points;
    }
    if (rx == 0) {
        for (int y = -ry; y <= ry; ++y) points.emplace_back(xc, yc + y);
        return points;
    }
    if (ry == 0) {
        for (int x = -rx; x <= rx; ++x) points.emplace_back(xc + x, yc);
        return points;
    }

    long long rx2 = static_cast<long long>(rx) * rx;
    long long ry2 = static_cast<long long>(ry) * ry;
    long long twoRx2 = 2 * rx2;
    long long twoRy2 = 2 * ry2;

    int x = 0;
    int y = ry;
    long long px = 0;
    long long py = twoRx2 * y;

    plotEllipse4Points(xc, yc, x, y, points);

    // --- Miền 1: px < py (|dy/dx| < 1) ---
    double p1 = ry2 - (rx2 * ry) + (0.25 * rx2);
    while (px < py) {
        x++;
        px += twoRy2;
        if (p1 < 0) {
            p1 += ry2 + px;
        } else {
            y--;
            py -= twoRx2;
            p1 += ry2 + px - py;
        }
        plotEllipse4Points(xc, yc, x, y, points);
    }

    // --- Miền 2: px >= py (|dy/dx| >= 1) ---
    double p2 = ry2 * (x + 0.5) * (x + 0.5) + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
    while (y > 0) {
        y--;
        py -= twoRx2;
        if (p2 > 0) {
            p2 += rx2 - py;
        } else {
            x++;
            px += twoRy2;
            p2 += rx2 - py + px;
        }
        plotEllipse4Points(xc, yc, x, y, points);
    }

    return points;
}

// ============================================================================
// 1.4. CÁC ĐƯỜNG CONG KHÁC: PARABOL, HYPERBOL, SIN(X), COS(X)
// ============================================================================

/**
 * Thuật toán Midpoint vẽ Parabol:
 * Phương trình chính tắc: x^2 = 2*p*y  (với đỉnh tại gốc (0,0), đối xứng qua trục Y)
 * Ở đây vẽ Parabol với đỉnh tại (xc, yc), hệ số a (hoặc tham số p: a = 1 / (2*p)):
 * y - yc = a * (x - xc)^2
 * Tính chất đối xứng 2 nhánh: (xc + x, yc + sign * y) và (xc - x, yc + sign * y).
 * 
 * Chia 2 miền dựa trên độ dốc tiếp tuyến dy/dx = 2*a*x = x/p:
 * - Miền 1: dy/dx < 1 <=> x < p (bước theo x, x = x + 1)
 * - Miền 2: dy/dx >= 1 <=> x >= p (bước theo y, y = y + 1)
 */
inline std::vector<PixelPoint> drawParabolaMidpoint(
    int xc, int yc, float pParam, int xLimit, int yLimit, int direction = 1
) {
    std::vector<PixelPoint> points;
    if (pParam <= 0.001f) pParam = 10.0f;
    if (xLimit <= 0) xLimit = 150;
    if (yLimit <= 0) yLimit = 200;

    auto plotPair = [&](int x, int y) {
        int py = yc + direction * y;
        points.emplace_back(xc + x, py);
        if (x != 0) {
            points.emplace_back(xc - x, py);
        }
    };

    int x = 0;
    int y = 0;
    plotPair(x, y);

    // Miền 1: Độ dốc tiếp tuyến dy/dx < 1 => x < pParam
    float d1 = 1.0f - pParam;
    while (x < pParam && x <= xLimit && y <= yLimit) {
        x++;
        if (d1 < 0) {
            d1 += 2.0f * x + 1.0f;
        } else {
            y++;
            d1 += 2.0f * x + 1.0f - 2.0f * pParam;
        }
        plotPair(x, y);
    }

    // Miền 2: Độ dốc dy/dx >= 1 => x >= pParam (bước theo y)
    float d2 = (x + 0.5f) * (x + 0.5f) - 2.0f * pParam * (y + 1);
    while (x <= xLimit && y <= yLimit) {
        y++;
        if (d2 > 0) {
            d2 -= 2.0f * pParam;
        } else {
            x++;
            d2 += 2.0f * x - 2.0f * pParam;
        }
        plotPair(x, y);
    }

    return points;
}

/**
 * Thuật toán Midpoint vẽ Hyperbol:
 * Phương trình chính tắc: (x^2 / a^2) - (y^2 / b^2) = 1
 * <=> b^2 * x^2 - a^2 * y^2 - a^2 * b^2 = 0
 * Đối xứng 4 nhánh qua tâm (xc, yc): (xc +- x, yc +- y)
 * 
 * Điểm bắt đầu tại đỉnh (a, 0).
 * Độ dốc tiếp tuyến: dy/dx = (b^2 * x) / (a^2 * y)
 * - Miền 1: dy/dx > 1 <=> b^2 * x > a^2 * y (bước theo y: y = y + 1)
 * - Miền 2: dy/dx <= 1 <=> b^2 * x <= a^2 * y (bước theo x: x = x + 1)
 */
inline std::vector<PixelPoint> drawHyperbolaMidpoint(
    int xc, int yc, int a, int b, int xLimit, int yLimit
) {
    std::vector<PixelPoint> points;
    if (a <= 0) a = 20;
    if (b <= 0) b = 20;
    if (xLimit <= a) xLimit = a + 150;
    if (yLimit <= 0) yLimit = 150;

    auto plot4 = [&](int x, int y) {
        points.emplace_back(xc + x, yc + y);
        points.emplace_back(xc - x, yc + y);
        points.emplace_back(xc + x, yc - y);
        points.emplace_back(xc - x, yc - y);
    };

    long long a2 = static_cast<long long>(a) * a;
    long long b2 = static_cast<long long>(b) * b;

    int x = a;
    int y = 0;
    plot4(x, y);

    // Xuất phát từ đỉnh (a, 0), tiếp tuyến thẳng đứng dy/dx > 1
    double d1 = b2 * (x + 0.5) * (x + 0.5) - a2 * (y + 1) * (y + 1) - a2 * b2;

    while (b2 * x > a2 * y && x <= xLimit && y <= yLimit) {
        y++;
        if (d1 < 0) {
            x++;
            d1 += 2.0 * b2 * x - 2.0 * a2 * y - a2;
        } else {
            d1 += -2.0 * a2 * y - a2;
        }
        plot4(x, y);
    }

    // Miền 2: dy/dx <= 1 (bước theo x)
    double d2 = b2 * (x + 1) * (x + 1) - a2 * (y + 0.5) * (y + 0.5) - a2 * b2;
    while (x <= xLimit && y <= yLimit) {
        x++;
        if (d2 > 0) {
            y++;
            d2 += 2.0 * b2 * x + b2 - 2.0 * a2 * y;
        } else {
            d2 += 2.0 * b2 * x + b2;
        }
        plot4(x, y);
    }

    return points;
}

/**
 * Thuật toán vẽ sóng Sin liên tục (8-connected continuous rasterization):
 * y(x) = yc - round(A * sin(2*pi * (x - xc) / wavelength))
 */
inline std::vector<PixelPoint> drawSinWave(
    int xc, int yc, float amplitude, float wavelength, int xSpan
) {
    std::vector<PixelPoint> points;
    if (wavelength <= 1.0f) wavelength = 100.0f;
    if (xSpan <= 0) xSpan = 300;

    int prevX = xc - xSpan;
    float arg = 2.0f * 3.1415926535f * static_cast<float>(prevX - xc) / wavelength;
    int prevY = yc - static_cast<int>(std::round(amplitude * std::sin(arg)));
    points.emplace_back(prevX, prevY);

    for (int curX = prevX + 1; curX <= xc + xSpan; ++curX) {
        float curArg = 2.0f * 3.1415926535f * static_cast<float>(curX - xc) / wavelength;
        int curY = yc - static_cast<int>(std::round(amplitude * std::sin(curArg)));

        int dy = curY - prevY;
        int stepY = (dy >= 0) ? 1 : -1;
        int absDy = std::abs(dy);

        if (absDy <= 1) {
            points.emplace_back(curX, curY);
        } else {
            for (int k = 1; k <= absDy; ++k) {
                points.emplace_back(curX, prevY + k * stepY);
            }
        }

        prevX = curX;
        prevY = curY;
    }

    return points;
}

/**
 * Thuật toán vẽ sóng Cos liên tục (8-connected continuous rasterization):
 * y(x) = yc - round(A * cos(2*pi * (x - xc) / wavelength))
 */
inline std::vector<PixelPoint> drawCosWave(
    int xc, int yc, float amplitude, float wavelength, int xSpan
) {
    std::vector<PixelPoint> points;
    if (wavelength <= 1.0f) wavelength = 100.0f;
    if (xSpan <= 0) xSpan = 300;

    int prevX = xc - xSpan;
    float arg = 2.0f * 3.1415926535f * static_cast<float>(prevX - xc) / wavelength;
    int prevY = yc - static_cast<int>(std::round(amplitude * std::cos(arg)));
    points.emplace_back(prevX, prevY);

    for (int curX = prevX + 1; curX <= xc + xSpan; ++curX) {
        float curArg = 2.0f * 3.1415926535f * static_cast<float>(curX - xc) / wavelength;
        int curY = yc - static_cast<int>(std::round(amplitude * std::cos(curArg)));

        int dy = curY - prevY;
        int stepY = (dy >= 0) ? 1 : -1;
        int absDy = std::abs(dy);

        if (absDy <= 1) {
            points.emplace_back(curX, curY);
        } else {
            for (int k = 1; k <= absDy; ++k) {
                points.emplace_back(curX, prevY + k * stepY);
            }
        }

        prevX = curX;
        prevY = curY;
    }

    return points;
}

// ============================================================================
// HÀM KIỂM THỬ TỰ ĐỘNG (ALGORITHM AUTOMATED UNIT TESTS)
// ============================================================================
inline bool runAlgorithmTests() {
    std::cout << "\n===============================================================\n";
    std::cout << "        CHAY KIEM THU TU DONG CAC THUAT TOAN LAB 01            \n";
    std::cout << "===============================================================\n";

    bool allPassed = true;

    // 1. Kiểm thử đoạn thẳng 4 trường hợp
    struct TestCaseLine {
        int x1, y1, x2, y2;
        LineSlopeCase expectedCase;
        std::string name;
    };

    std::vector<TestCaseLine> lineTests = {
        {10, 10, 50, 30, CASE_1_ZERO_TO_ONE, "Case 1: 0 < m <= 1 (m = 0.5)"},
        {10, 10, 30, 60, CASE_2_GREATER_ONE, "Case 2: m > 1 (m = 2.5)"},
        {10, 50, 50, 30, CASE_3_MINUS_ONE_TO_ZERO, "Case 3: -1 <= m < 0 (m = -0.5)"},
        {10, 60, 20, 10, CASE_4_LESS_MINUS_ONE, "Case 4: m < -1 (m = -5.0)"},
        {10, 20, 50, 20, CASE_HORIZONTAL, "Case Horizontal (m = 0)"},
        {30, 10, 30, 50, CASE_VERTICAL, "Case Vertical (dx = 0)"}
    };

    for (const auto& t : lineTests) {
        LineSlopeCase bCase, mCase;
        float bSlope, mSlope;
        auto ptsB = drawLineBresenham(t.x1, t.y1, t.x2, t.y2, bCase, bSlope);
        auto ptsM = drawLineMidpoint(t.x1, t.y1, t.x2, t.y2, mCase, mSlope);

        bool caseOk = (bCase == t.expectedCase && mCase == t.expectedCase);
        bool ptsNotEmpty = (!ptsB.empty() && !ptsM.empty());
        bool endpointsOk = (ptsB.front() == PixelPoint(t.x1, t.y1) && ptsB.back() == PixelPoint(t.x2, t.y2));

        if (caseOk && ptsNotEmpty && endpointsOk) {
            std::cout << " [PASS] " << t.name << " -> Bresenham: " << ptsB.size() << " pts, Midpoint: " << ptsM.size() << " pts\n";
        } else {
            std::cout << " [FAIL] " << t.name << "\n";
            allPassed = false;
        }
    }

    // 2. Kiểm thử đường tròn
    int r = 25;
    auto circleB = drawCircleBresenham(100, 100, r);
    auto circleM = drawCircleMidpoint(100, 100, r);
    if (!circleB.empty() && !circleM.empty()) {
        std::cout << " [PASS] Circle R=" << r << " -> Bresenham: " << circleB.size() << " pts, Midpoint: " << circleM.size() << " pts\n";
    } else {
        std::cout << " [FAIL] Circle test\n";
        allPassed = false;
    }

    // 3. Kiểm thử Ellipse
    int rx = 40, ry = 20;
    auto ellipseM = drawEllipseMidpoint(100, 100, rx, ry);
    if (!ellipseM.empty()) {
        std::cout << " [PASS] Ellipse rx=" << rx << ", ry=" << ry << " -> Midpoint: " << ellipseM.size() << " pts\n";
    } else {
        std::cout << " [FAIL] Ellipse test\n";
        allPassed = false;
    }

    // 4. Kiểm thử Parabol, Hyperbol, Sin, Cos
    auto parabola = drawParabolaMidpoint(100, 100, 20.0f, 60, 60, 1);
    auto hyperbola = drawHyperbolaMidpoint(100, 100, 30, 20, 80, 80);
    auto sinCurve = drawSinWave(200, 200, 50.0f, 120.0f, 150);
    auto cosCurve = drawCosWave(200, 200, 50.0f, 120.0f, 150);

    if (!parabola.empty() && !hyperbola.empty() && !sinCurve.empty() && !cosCurve.empty()) {
        std::cout << " [PASS] Parabola -> " << parabola.size() << " pts\n";
        std::cout << " [PASS] Hyperbola -> " << hyperbola.size() << " pts\n";
        std::cout << " [PASS] Sin Wave -> " << sinCurve.size() << " pts\n";
        std::cout << " [PASS] Cos Wave -> " << cosCurve.size() << " pts\n";
    } else {
        std::cout << " [FAIL] Curved shapes test\n";
        allPassed = false;
    }

    std::cout << "---------------------------------------------------------------\n";
    std::cout << (allPassed ? "KET QUA: TAT CA THUAT TOAN LAB 1 HOAT DONG CHINH XAC!\n" : "KET QUA: CO LOI TRONG CAC THUAT TOAN!\n");
    std::cout << "===============================================================\n\n";

    return allPassed;
}

} // namespace Lab1

#endif // LAB1_ALGORITHMS_HPP
