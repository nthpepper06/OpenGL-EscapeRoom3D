#ifndef LAB2_ALGORITHMS_HPP
#define LAB2_ALGORITHMS_HPP

#include <vector>
#include <string>
#include <cmath>
#include <stack>
#include <queue>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace Lab2 {

// ============================================================================
// CẤU TRÚC DỮ LIỆU CƠ BẢN
// ============================================================================

struct Point2D {
    float x = 0.0f;
    float y = 0.0f;

    Point2D() = default;
    Point2D(float _x, float _y) : x(_x), y(_y) {}

    bool operator==(const Point2D& o) const {
        return std::abs(x - o.x) < 1e-4f && std::abs(y - o.y) < 1e-4f;
    }
};

struct PixelPoint {
    int x = 0;
    int y = 0;

    PixelPoint() = default;
    PixelPoint(int _x, int _y) : x(_x), y(_y) {}

    bool operator==(const PixelPoint& o) const {
        return x == o.x && y == o.y;
    }
    bool operator<(const PixelPoint& o) const {
        if (y != o.y) return y < o.y;
        return x < o.x;
    }
};

// Hình chữ nhật xén (Clipping Window)
struct ClipRect {
    float xMin = -150.0f;
    float yMin = -100.0f;
    float xMax = 150.0f;
    float yMax = 100.0f;

    ClipRect() = default;
    ClipRect(float _xMin, float _yMin, float _xMax, float _yMax) {
        set(_xMin, _yMin, _xMax, _yMax);
    }

    void set(float _xMin, float _yMin, float _xMax, float _yMax) {
        xMin = std::min(_xMin, _xMax);
        xMax = std::max(_xMin, _xMax);
        yMin = std::min(_yMin, _yMax);
        yMax = std::max(_yMin, _yMax);
    }
};

// ============================================================================
// 2.1. THUẬT TOÁN XÉN HÌNH (CLIPPING)
// ============================================================================

// Định nghĩa mã vùng 4-bit (Outcode)
namespace OutCode {
    constexpr int INSIDE = 0; // 0000: Nằm hoàn toàn trong cửa sổ
    constexpr int LEFT   = 1; // 0001: Nằm bên trái (x < xMin)
    constexpr int RIGHT  = 2; // 0010: Nằm bên phải (x > xMax)
    constexpr int BOTTOM = 4; // 0100: Nằm bên dưới (y < yMin)
    constexpr int TOP    = 8; // 1000: Nằm bên trên (y > yMax)
}

// Tính mã vùng 4-bit cho điểm P(x, y)
inline int computeOutCode(float x, float y, const ClipRect& r) {
    int code = OutCode::INSIDE;
    if (x < r.xMin)      code |= OutCode::LEFT;
    else if (x > r.xMax) code |= OutCode::RIGHT;

    if (y < r.yMin)      code |= OutCode::BOTTOM;
    else if (y > r.yMax) code |= OutCode::TOP;

    return code;
}

inline std::string outCodeToString(int code) {
    std::string s = "0000";
    if (code & OutCode::TOP)    s[0] = '1';
    if (code & OutCode::BOTTOM) s[1] = '1';
    if (code & OutCode::RIGHT)  s[2] = '1';
    if (code & OutCode::LEFT)   s[3] = '1';
    return s;
}

// Kết quả xén hình đoạn thẳng
struct LineClipResult {
    bool accepted = false;
    Point2D p1Orig;
    Point2D p2Orig;
    Point2D p1Clipped;
    Point2D p2Clipped;
    int code1Orig = 0;
    int code2Orig = 0;
    int iterations = 0;
    std::string method;
    std::vector<std::string> stepLogs;
};

/**
 * 2.1.A. Thuật toán Cohen-Sutherland Line Clipping
 * Cắt đoạn thẳng theo 4 cạnh của cửa sổ hình chữ nhật bằng mã 4-bit.
 */
inline LineClipResult clipCohenSutherland(Point2D p1, Point2D p2, const ClipRect& rect) {
    LineClipResult res;
    res.method = "Cohen-Sutherland";
    res.p1Orig = p1;
    res.p2Orig = p2;

    float x1 = p1.x;
    float y1 = p1.y;
    float x2 = p2.x;
    float y2 = p2.y;

    int code1 = computeOutCode(x1, y1, rect);
    int code2 = computeOutCode(x2, y2, rect);
    res.code1Orig = code1;
    res.code2Orig = code2;

    std::stringstream ss;
    ss << "P1(" << (int)x1 << "," << (int)y1 << ") code=" << outCodeToString(code1)
       << " | P2(" << (int)x2 << "," << (int)y2 << ") code=" << outCodeToString(code2);
    res.stepLogs.push_back(ss.str());

    int iter = 0;
    while (iter++ < 20) {
        // Trường hợp 1: Chấp nhận hoàn toàn (Trivially Accept)
        if ((code1 | code2) == 0) {
            res.accepted = true;
            res.p1Clipped = Point2D(x1, y1);
            res.p2Clipped = Point2D(x2, y2);
            res.stepLogs.push_back("-> Chap nhan hoan toan (Nam ben trong cua so).");
            break;
        }

        // Trường hợp 2: Loại bỏ hoàn toàn (Trivially Reject)
        if ((code1 & code2) != 0) {
            res.accepted = false;
            res.stepLogs.push_back("-> Loai bo hoan toan (Cung nam ngoai mot phia bien).");
            break;
        }

        // Trường hợp 3: Cắt đoạn thẳng tại biên
        // Chọn điểm nằm bên ngoài cửa sổ
        int outCodeSelected = code1 ? code1 : code2;
        float x = 0.0f, y = 0.0f;

        if (outCodeSelected & OutCode::TOP) {
            // Điểm nằm phía trên yMax
            x = x1 + (x2 - x1) * (rect.yMax - y1) / (y2 - y1);
            y = rect.yMax;
            std::stringstream sstep;
            sstep << "Cat bien TOP (y=" << rect.yMax << "): giao diem (" << (int)x << "," << (int)y << ")";
            res.stepLogs.push_back(sstep.str());
        } else if (outCodeSelected & OutCode::BOTTOM) {
            // Điểm nằm phía dưới yMin
            x = x1 + (x2 - x1) * (rect.yMin - y1) / (y2 - y1);
            y = rect.yMin;
            std::stringstream sstep;
            sstep << "Cat bien BOTTOM (y=" << rect.yMin << "): giao diem (" << (int)x << "," << (int)y << ")";
            res.stepLogs.push_back(sstep.str());
        } else if (outCodeSelected & OutCode::RIGHT) {
            // Điểm nằm phía bên phải xMax
            y = y1 + (y2 - y1) * (rect.xMax - x1) / (x2 - x1);
            x = rect.xMax;
            std::stringstream sstep;
            sstep << "Cat bien RIGHT (x=" << rect.xMax << "): giao diem (" << (int)x << "," << (int)y << ")";
            res.stepLogs.push_back(sstep.str());
        } else if (outCodeSelected & OutCode::LEFT) {
            // Điểm nằm phía bên trái xMin
            y = y1 + (y2 - y1) * (rect.xMin - x1) / (x2 - x1);
            x = rect.xMin;
            std::stringstream sstep;
            sstep << "Cat bien LEFT (x=" << rect.xMin << "): giao diem (" << (int)x << "," << (int)y << ")";
            res.stepLogs.push_back(sstep.str());
        }

        // Cập nhật điểm ngoài tương ứng
        if (outCodeSelected == code1) {
            x1 = x;
            y1 = y;
            code1 = computeOutCode(x1, y1, rect);
        } else {
            x2 = x;
            y2 = y;
            code2 = computeOutCode(x2, y2, rect);
        }
    }

    res.iterations = iter;
    return res;
}

/**
 * 2.1.B. Thuật toán xén hình Chia nhị phân (Binary / Midpoint Subdivision Line Clipping)
 * Phương pháp chia đôi liên tiếp đoạn thẳng để hội tụ về giao điểm với biên cửa sổ.
 */
inline LineClipResult clipBinarySubdivision(Point2D p1, Point2D p2, const ClipRect& rect, float tolerance = 0.5f) {
    LineClipResult res;
    res.method = "Chia nhi phan (Midpoint Subdivision)";
    res.p1Orig = p1;
    res.p2Orig = p2;

    int code1 = computeOutCode(p1.x, p1.y, rect);
    int code2 = computeOutCode(p2.x, p2.y, rect);
    res.code1Orig = code1;
    res.code2Orig = code2;

    // Kiểm tra nhanh
    if ((code1 | code2) == 0) {
        res.accepted = true;
        res.p1Clipped = p1;
        res.p2Clipped = p2;
        res.stepLogs.push_back("-> Chap nhan hoan toan ngay tu dau.");
        return res;
    }

    if ((code1 & code2) != 0) {
        res.accepted = false;
        res.stepLogs.push_back("-> Loai bo hoan toan (Cung phia ngoai).");
        return res;
    }

    // Hàm đệ quy/lặp tìm điểm cắt biên gần điểm ngoài bằng phương pháp chia đôi
    auto findBoundaryPoint = [&](Point2D outsidePt, Point2D otherPt) -> Point2D {
        Point2D pOut = outsidePt;
        Point2D pIn = otherPt;

        // Nếu cả 2 đều ngoài nhưng đoạn cắt qua cửa sổ, ta tìm một điểm trung gian trong cửa sổ trước
        int codeIn = computeOutCode(pIn.x, pIn.y, rect);
        if (codeIn != 0) {
            // Tìm điểm trong
            Point2D curA = pOut, curB = pIn;
            for (int k = 0; k < 25; ++k) {
                Point2D mid((curA.x + curB.x) * 0.5f, (curA.y + curB.y) * 0.5f);
                int cMid = computeOutCode(mid.x, mid.y, rect);
                if (cMid == 0) {
                    pIn = mid;
                    break;
                }
                if ((computeOutCode(curA.x, curA.y, rect) & cMid) != 0) {
                    curA = mid;
                } else {
                    curB = mid;
                }
            }
        }

        // Tiến hành chia nhị phân giữa pOut (ngoài) và pIn (trong hoặc gần trong)
        int step = 0;
        while (step++ < 30) {
            float dist = std::hypot(pOut.x - pIn.x, pOut.y - pIn.y);
            if (dist <= tolerance) {
                break;
            }

            Point2D mid((pOut.x + pIn.x) * 0.5f, (pOut.y + pIn.y) * 0.5f);
            int codeMid = computeOutCode(mid.x, mid.y, rect);

            if ((computeOutCode(pOut.x, pOut.y, rect) & codeMid) != 0) {
                // mid và pOut cùng ngoài một phía -> dời pOut tới mid
                pOut = mid;
            } else if (codeMid != 0) {
                // mid ngoài nhưng khác phía pOut
                pOut = mid;
            } else {
                // mid nằm trong cửa sổ -> dời pIn tới mid
                pIn = mid;
            }
        }
        return pIn;
    };

    Point2D c1 = p1;
    Point2D c2 = p2;

    if (code1 != 0) {
        c1 = findBoundaryPoint(p1, p2);
        std::stringstream ss;
        ss << "Chia nhi phan tim duoc giao diem P1': (" << (int)c1.x << "," << (int)c1.y << ")";
        res.stepLogs.push_back(ss.str());
    }
    if (code2 != 0) {
        c2 = findBoundaryPoint(p2, c1);
        std::stringstream ss;
        ss << "Chia nhi phan tim duoc giao diem P2': (" << (int)c2.x << "," << (int)c2.y << ")";
        res.stepLogs.push_back(ss.str());
    }

    // Kiểm tra lại đoạn kết quả
    int check1 = computeOutCode(c1.x, c1.y, rect);
    int check2 = computeOutCode(c2.x, c2.y, rect);

    if ((check1 | check2) == 0 || (std::abs(c1.x - c2.x) >= 0.5f || std::abs(c1.y - c2.y) >= 0.5f)) {
        res.accepted = true;
        res.p1Clipped = c1;
        res.p2Clipped = c2;
    } else {
        res.accepted = false;
    }

    return res;
}

// ============================================================================
// 2.2. THUẬT TOÁN TÔ MÀU (REGION & POLYGON FILLING)
// ============================================================================

// Cấu trúc Bitmap 2D lưu trữ pixel cho thuật toán Boundary Fill
struct PixelGridCanvas {
    int width = 0;
    int height = 0;
    int originX = 0; // Tọa độ tâm
    int originY = 0;
    std::vector<uint32_t> buffer; // Mỗi pixel lưu mã màu RGBA hoặc ID màu

    PixelGridCanvas() = default;
    PixelGridCanvas(int w, int h, int ox, int oy) : width(w), height(h), originX(ox), originY(oy) {
        buffer.assign(w * h, 0);
    }

    bool isValid(int wx, int wy) const {
        int gx = wx + originX;
        int gy = wy + originY;
        return (gx >= 0 && gx < width && gy >= 0 && gy < height);
    }

    uint32_t getPixel(int wx, int wy) const {
        int gx = wx + originX;
        int gy = wy + originY;
        if (gx < 0 || gx >= width || gy < 0 || gy >= height) return 0xFFFFFFFF; // Viền ngoài
        return buffer[gy * width + gx];
    }

    void setPixel(int wx, int wy, uint32_t color) {
        int gx = wx + originX;
        int gy = wy + originY;
        if (gx >= 0 && gx < width && gy >= 0 && gy < height) {
            buffer[gy * width + gx] = color;
        }
    }

    void clear(uint32_t color = 0) {
        std::fill(buffer.begin(), buffer.end(), color);
    }
};

/**
 * 2.2.A1. Tô màu theo đường biên ĐỆ QUY (Recursive Boundary Fill 4-connected)
 * Giới hạn độ sâu an toàn để không tràn stack hệ điều hành, có thể gọi nhiều lượt.
 */
inline void boundaryFillRecursiveHelper(
    PixelGridCanvas& canvas,
    int x, int y,
    uint32_t fillColor,
    uint32_t borderColor,
    int& filledCount,
    int depth,
    int maxDepth,
    std::vector<PixelPoint>& outFilledPixels
) {
    if (depth > maxDepth || !canvas.isValid(x, y)) return;

    uint32_t current = canvas.getPixel(x, y);
    if (current != borderColor && current != fillColor) {
        canvas.setPixel(x, y, fillColor);
        outFilledPixels.push_back(PixelPoint(x, y));
        filledCount++;

        boundaryFillRecursiveHelper(canvas, x + 1, y, fillColor, borderColor, filledCount, depth + 1, maxDepth, outFilledPixels);
        boundaryFillRecursiveHelper(canvas, x - 1, y, fillColor, borderColor, filledCount, depth + 1, maxDepth, outFilledPixels);
        boundaryFillRecursiveHelper(canvas, x, y + 1, fillColor, borderColor, filledCount, depth + 1, maxDepth, outFilledPixels);
        boundaryFillRecursiveHelper(canvas, x, y - 1, fillColor, borderColor, filledCount, depth + 1, maxDepth, outFilledPixels);
    }
}

inline int boundaryFillRecursive(
    PixelGridCanvas& canvas,
    int seedX, int seedY,
    uint32_t fillColor,
    uint32_t borderColor,
    std::vector<PixelPoint>& outFilledPixels,
    int maxDepth = 4000
) {
    int filledCount = 0;
    boundaryFillRecursiveHelper(canvas, seedX, seedY, fillColor, borderColor, filledCount, 0, maxDepth, outFilledPixels);
    return filledCount;
}

/**
 * 2.2.A2. Tô màu theo đường biên KHÔNG ĐỆ QUY (Non-Recursive Boundary Fill)
 * Sử dụng ngăn xếp std::stack tự quản lý (Stack-based Boundary Fill 4-connected).
 * Hoàn toàn an toàn, không bao giờ bị tràn call stack, hỗ trợ vùng đa giác lớn tùy ý.
 */
inline int boundaryFillNonRecursive(
    PixelGridCanvas& canvas,
    int seedX, int seedY,
    uint32_t fillColor,
    uint32_t borderColor,
    std::vector<PixelPoint>& outFilledPixels
) {
    if (!canvas.isValid(seedX, seedY)) return 0;
    uint32_t seedCol = canvas.getPixel(seedX, seedY);
    if (seedCol == borderColor || seedCol == fillColor) return 0;

    std::stack<PixelPoint> st;
    st.push(PixelPoint(seedX, seedY));
    canvas.setPixel(seedX, seedY, fillColor);
    outFilledPixels.push_back(PixelPoint(seedX, seedY));

    int filledCount = 1;

    // 4 hướng láng giềng: Phải, Trái, Trên, Dưới
    const int dx[4] = { 1, -1, 0, 0 };
    const int dy[4] = { 0, 0, 1, -1 };

    while (!st.empty()) {
        PixelPoint pt = st.top();
        st.pop();

        for (int i = 0; i < 4; ++i) {
            int nx = pt.x + dx[i];
            int ny = pt.y + dy[i];

            if (canvas.isValid(nx, ny)) {
                uint32_t col = canvas.getPixel(nx, ny);
                if (col != borderColor && col != fillColor) {
                    canvas.setPixel(nx, ny, fillColor);
                    outFilledPixels.push_back(PixelPoint(nx, ny));
                    filledCount++;
                    st.push(PixelPoint(nx, ny));
                }
            }
        }
    }

    return filledCount;
}

/**
 * 2.2.B. Thuật toán tô màu đa giác theo DÒNG QUÉT (Scanline Polygon Fill Algorithm)
 * Sử dụng Bảng cạnh (Edge Table - ET) và Bảng cạnh tích cực (Active Edge Table - AET).
 * Xử lý chính xác:
 * - Đa giác lồi, đa giác lõm (ngôi sao, chữ U), tự cắt
 * - Bỏ qua cạnh nằm ngang
 * - Xử lý đúng đỉnh cục bộ (cực đại/cực tiểu địa phương)
 */
struct ScanEdge {
    int yMax = 0;        // Tung độ lớn nhất của cạnh
    float x = 0.0f;      // Hoành độ hiện tại tại dòng quét y
    float invSlope = 0.0f; // 1/m = dx/dy
};

inline std::vector<PixelPoint> scanlinePolygonFill(const std::vector<Point2D>& vertices) {
    std::vector<PixelPoint> filledPixels;
    int n = static_cast<int>(vertices.size());
    if (n < 3) return filledPixels;

    // Tìm yMin và yMax của toàn bộ đa giác
    int yMinAll = static_cast<int>(std::floor(vertices[0].y));
    int yMaxAll = static_cast<int>(std::ceil(vertices[0].y));
    for (int i = 1; i < n; ++i) {
        yMinAll = std::min(yMinAll, static_cast<int>(std::floor(vertices[i].y)));
        yMaxAll = std::max(yMaxAll, static_cast<int>(std::ceil(vertices[i].y)));
    }

    int rangeY = yMaxAll - yMinAll + 1;
    if (rangeY <= 0 || rangeY > 5000) return filledPixels;

    // Khởi tạo Edge Table (ET): Mỗi dòng y có danh sách các cạnh xuất phát từ y
    std::vector<std::vector<ScanEdge>> edgeTable(rangeY);

    for (int i = 0; i < n; ++i) {
        int next = (i + 1) % n;
        Point2D p1 = vertices[i];
        Point2D p2 = vertices[next];

        // Bỏ qua các cạnh nằm ngang hoàn toàn (dy = 0)
        int iy1 = static_cast<int>(std::round(p1.y));
        int iy2 = static_cast<int>(std::round(p2.y));
        if (iy1 == iy2) continue;

        Point2D lower = (p1.y < p2.y) ? p1 : p2;
        Point2D upper = (p1.y < p2.y) ? p2 : p1;

        int yStart = static_cast<int>(std::round(lower.y));
        int yEnd   = static_cast<int>(std::round(upper.y));

        float invM = (upper.x - lower.x) / (upper.y - lower.y);

        ScanEdge edge;
        edge.yMax = yEnd;
        edge.x = lower.x + invM * (yStart - lower.y);
        edge.invSlope = invM;

        int etIndex = yStart - yMinAll;
        if (etIndex >= 0 && etIndex < rangeY) {
            edgeTable[etIndex].push_back(edge);
        }
    }

    // Bảng cạnh tích cực AET
    std::vector<ScanEdge> activeEdgeTable;

    // Quét từng dòng từ yMinAll đến yMaxAll
    for (int y = yMinAll; y <= yMaxAll; ++y) {
        int etIndex = y - yMinAll;

        // 1. Chuyển các cạnh từ ET[y] vào AET
        if (etIndex >= 0 && etIndex < rangeY) {
            for (const auto& edge : edgeTable[etIndex]) {
                activeEdgeTable.push_back(edge);
            }
        }

        // 2. Loại bỏ khỏi AET các cạnh có yMax <= y
        activeEdgeTable.erase(
            std::remove_if(activeEdgeTable.begin(), activeEdgeTable.end(),
                [y](const ScanEdge& e) { return e.yMax <= y; }),
            activeEdgeTable.end()
        );

        // 3. Sắp xếp các cạnh trong AET theo hoành độ x tăng dần
        std::sort(activeEdgeTable.begin(), activeEdgeTable.end(),
            [](const ScanEdge& a, const ScanEdge& b) {
                return a.x < b.x;
            }
        );

        // 4. Ghép cặp các giao điểm (0, 1), (2, 3), ... và tô các pixel giữa các cặp
        for (size_t i = 0; i + 1 < activeEdgeTable.size(); i += 2) {
            int xStart = static_cast<int>(std::ceil(activeEdgeTable[i].x));
            int xEnd   = static_cast<int>(std::floor(activeEdgeTable[i + 1].x));

            for (int x = xStart; x <= xEnd; ++x) {
                filledPixels.push_back(PixelPoint(x, y));
            }
        }

        // 5. Cập nhật x = x + 1/m cho mỗi cạnh trong AET chuẩn bị cho dòng quét y + 1
        for (auto& edge : activeEdgeTable) {
            edge.x += edge.invSlope;
        }
    }

    return filledPixels;
}

// ============================================================================
// BỘ KIỂM THỬ TỰ ĐỘNG LAB 2 (UNIT TESTS)
// ============================================================================
inline bool runLab2Tests() {
    std::cout << "\n=======================================================\n";
    std::cout << "  RUNNING AUTOMATED UNIT TESTS: LAB 02 ALGORITHMS      \n";
    std::cout << "=======================================================\n";

    bool allPassed = true;
    ClipRect rect(-100, -100, 100, 100);

    // Test 1: Cohen-Sutherland - Đoạn nằm trong hoàn toàn
    {
        auto res = clipCohenSutherland(Point2D(-50, -50), Point2D(50, 50), rect);
        bool ok = res.accepted && (res.p1Clipped == Point2D(-50, -50)) && (res.p2Clipped == Point2D(50, 50));
        std::cout << " [TEST 1] Cohen-Sutherland Inside Line: " << (ok ? "PASSED" : "FAILED") << "\n";
        if (!ok) allPassed = false;
    }

    // Test 2: Cohen-Sutherland - Đoạn nằm ngoài hoàn toàn
    {
        auto res = clipCohenSutherland(Point2D(120, 50), Point2D(150, 80), rect);
        bool ok = !res.accepted;
        std::cout << " [TEST 2] Cohen-Sutherland Outside Line: " << (ok ? "PASSED" : "FAILED") << "\n";
        if (!ok) allPassed = false;
    }

    // Test 3: Cohen-Sutherland - Đoạn cắt qua 2 biên
    {
        auto res = clipCohenSutherland(Point2D(-150, 0), Point2D(150, 0), rect);
        bool ok = res.accepted && std::abs(res.p1Clipped.x - (-100.0f)) < 1e-3f && std::abs(res.p2Clipped.x - 100.0f) < 1e-3f;
        std::cout << " [TEST 3] Cohen-Sutherland Crossing Line: " << (ok ? "PASSED" : "FAILED") << "\n";
        if (!ok) allPassed = false;
    }

    // Test 4: Chia nhị phân (Midpoint Subdivision)
    {
        auto res = clipBinarySubdivision(Point2D(-150, 0), Point2D(150, 0), rect);
        bool ok = res.accepted && std::abs(res.p1Clipped.x - (-100.0f)) <= 1.5f && std::abs(res.p2Clipped.x - 100.0f) <= 1.5f;
        std::cout << " [TEST 4] Binary Subdivision Crossing Line: " << (ok ? "PASSED" : "FAILED") << "\n";
        if (!ok) allPassed = false;
    }

    // Test 5: Boundary Fill Non-recursive trong ô chữ nhật 10x10
    {
        PixelGridCanvas canvas(100, 100, 50, 50);
        uint32_t BORDER = 0xFF;
        uint32_t FILL = 0xAA;
        // Vẽ khung viền từ x: -5..5, y: -5..5
        for (int x = -5; x <= 5; ++x) {
            canvas.setPixel(x, -5, BORDER);
            canvas.setPixel(x, 5, BORDER);
        }
        for (int y = -5; y <= 5; ++y) {
            canvas.setPixel(-5, y, BORDER);
            canvas.setPixel(5, y, BORDER);
        }

        std::vector<PixelPoint> filled;
        int count = boundaryFillNonRecursive(canvas, 0, 0, FILL, BORDER, filled);
        // Vùng bên trong là 9x9 = 81 pixel
        bool ok = (count == 81);
        std::cout << " [TEST 5] Non-Recursive Boundary Fill (81 px expected): " 
                  << (ok ? "PASSED" : "FAILED") << " (Got " << count << " px)\n";
        if (!ok) allPassed = false;
    }

    // Test 6: Scanline Polygon Fill hình tam giác
    {
        std::vector<Point2D> tri = { Point2D(0, 0), Point2D(10, 0), Point2D(0, 10) };
        auto filled = scanlinePolygonFill(tri);
        bool ok = !filled.empty();
        std::cout << " [TEST 6] Scanline Polygon Fill Triangle: " 
                  << (ok ? "PASSED" : "FAILED") << " (Got " << filled.size() << " px)\n";
        if (!ok) allPassed = false;
    }

    std::cout << "=======================================================\n";
    std::cout << " LAB 02 TESTS RESULT: " << (allPassed ? "ALL PASSED (100%)" : "SOME FAILED") << "\n";
    std::cout << "=======================================================\n\n";

    return allPassed;
}

} // namespace Lab2

#endif // LAB2_ALGORITHMS_HPP
