#ifndef LAB2_APP_HPP
#define LAB2_APP_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <algorithm>

#include "../Math3D.hpp"
#include "Lab2Algorithms.hpp"

namespace Lab2 {

// Các chế độ công cụ trong Lab 2
enum Lab2Tool {
    TOOL_COHEN_SUTHERLAND = 0,    // 2.1. Xén hình Cohen-Sutherland
    TOOL_BINARY_SUBDIVISION = 1,  // 2.1. Xén hình Chia nhị phân
    TOOL_EDIT_CLIP_WINDOW = 2,    // Chỉnh sửa khung xén chữ nhật
    TOOL_BOUNDARY_REC = 3,        // 2.2. Tô biên đệ quy
    TOOL_BOUNDARY_STACK = 4,      // 2.2. Tô biên không đệ quy (Stack)
    TOOL_SCANLINE_FILL = 5,       // 2.2. Tô màu theo dòng quét
    TOOL_DRAW_CUSTOM_POLYGON = 6  // Click chuột vẽ đa giác tùy biến
};

inline std::string getToolTitle(Lab2Tool t) {
    switch (t) {
        case TOOL_COHEN_SUTHERLAND: return "2.1. Xen doan thang: COHEN-SUTHERLAND";
        case TOOL_BINARY_SUBDIVISION: return "2.1. Xen doan thang: CHIA NHI PHAN";
        case TOOL_EDIT_CLIP_WINDOW: return "Dieu chinh Cua so Xen (Clip Window)";
        case TOOL_BOUNDARY_REC: return "2.2. To duong bien: DE QUY (Recursive)";
        case TOOL_BOUNDARY_STACK: return "2.2. To duong bien: KHONG DE QUY (Stack)";
        case TOOL_SCANLINE_FILL: return "2.2. To mau da giac: DONG QUET (Scanline)";
        case TOOL_DRAW_CUSTOM_POLYGON: return "Ve da giac bang chuot (Click dinh, Enter de khep kin)";
        default: return "Lab 2";
    }
}

// Cấu trúc một nút bấm UI
struct UIButton {
    std::string label;
    int id;
    float x, y, w, h;
    Vec4 bgColor;
    Vec4 textColor;
    bool isSelected = false;
};

class Application {
public:
    GLFWwindow* window = nullptr;
    int windowWidth = 1280;
    int windowHeight = 720;

    // Viewport & Pan / Zoom
    float zoom = 1.0f;
    float panX = 0.0f;
    float panY = 0.0f;
    bool showGrid = true;
    bool showAxes = true;

    // Trạng thái công cụ
    Lab2Tool currentTool = TOOL_COHEN_SUTHERLAND;
    int currentColorIdx = 0;
    std::vector<Vec4> palette = {
        Vec4(0.20f, 0.95f, 0.40f, 1.0f), // Xanh lá sáng
        Vec4(0.95f, 0.25f, 0.25f, 1.0f), // Đỏ tươi
        Vec4(0.25f, 0.65f, 0.95f, 1.0f), // Xanh dương
        Vec4(0.95f, 0.90f, 0.20f, 1.0f), // Vàng chanh
        Vec4(0.20f, 0.95f, 0.95f, 1.0f), // Cyan
        Vec4(0.95f, 0.40f, 0.95f, 1.0f), // Magenta
        Vec4(0.98f, 0.60f, 0.15f, 1.0f), // Cam
        Vec4(0.95f, 0.95f, 0.95f, 1.0f)  // Trắng
    };

    // Dữ liệu xén hình (Clipping)
    ClipRect clipRect = ClipRect(-160.0f, -110.0f, 160.0f, 110.0f);
    std::vector<LineClipResult> clippedLines;
    LineClipResult previewClipResult;
    bool hasClipPreview = false;

    // Dữ liệu đa giác tô màu
    std::vector<Point2D> currentPolygon;
    std::vector<Point2D> customPolyBuilding;
    std::string currentPolyFileName = "polygon_star.txt";

    // Bitmap Canvas cho Boundary Fill
    PixelGridCanvas fillCanvas;
    std::vector<PixelPoint> filledPixels;
    Point2D lastSeedPoint = Point2D(0, 0);
    bool hasSeedPoint = false;
    std::string fillStatusMsg = "San sang. Click chuot trai de to mau.";

    // Chuột & Tương tác
    bool isMouseDown = false;
    bool isPanning = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    Point2D dragStartWorld;
    Point2D dragCurrentWorld;

    // UI Buttons
    std::vector<UIButton> toolButtons;
    std::vector<UIButton> actionButtons;
    std::vector<UIButton> fileButtons;

    // OpenGL Shaders & Buffers
    GLuint shaderProgram = 0;
    GLuint vaoBatch = 0;
    GLuint vboBatch = 0;

    struct BatchVertex {
        float x, y;
        float r, g, b, a;
    };
    std::vector<BatchVertex> batchVertices;

    static Application* instance;

    Application() {
        instance = this;
        fillCanvas = PixelGridCanvas(1000, 1000, 500, 500);
    }

    bool init(int w = 1280, int h = 720, const char* title = "LAB 02 - Clipping & Polygon Filling Algorithms") {
        windowWidth = w;
        windowHeight = h;

        if (!glfwInit()) {
            std::cerr << "Loi khoi tao GLFW Lab 2!\n";
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);

        window = glfwCreateWindow(windowWidth, windowHeight, title, nullptr, nullptr);
        if (!window) {
            std::cerr << "Loi tao cua so GLFW Lab 2!\n";
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Loi khoi tao GLAD Lab 2!\n";
            return false;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        initShaders();

        glGenVertexArrays(1, &vaoBatch);
        glGenBuffers(1, &vboBatch);
        glBindVertexArray(vaoBatch);
        glBindBuffer(GL_ARRAY_BUFFER, vboBatch);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), (void*)(2 * sizeof(float)));
        glBindVertexArray(0);

        panX = windowWidth * 0.5f;
        panY = windowHeight * 0.5f;
        zoom = 1.0f;

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, cursorPosCallback);
        glfwSetScrollCallback(window, scrollCallback);
        glfwSetKeyCallback(window, keyCallback);

        buildUIButtons();

        // Tải trước một file đa giác mẫu
        loadPolygonFromFile("data/polygon_star.txt");

        // Tạo sẵn 2 đoạn thẳng mẫu minh họa xén hình
        LineClipResult l1 = clipCohenSutherland(Point2D(-250, -150), Point2D(200, 160), clipRect);
        clippedLines.push_back(l1);
        LineClipResult l2 = clipCohenSutherland(Point2D(-220, 80), Point2D(220, 80), clipRect);
        clippedLines.push_back(l2);

        std::cout << "\n===============================================================\n";
        std::cout << "  LAB 02: THUAT TOAN XEN HINH VA TO MAU DA GIAC (CLIPPING & FILL)\n";
        std::cout << "===============================================================\n";
        std::cout << " - Phim [1]: Xen hinh Cohen-Sutherland (Click & keo doan thang)\n";
        std::cout << " - Phim [2]: Xen hinh Chia nhi phan (Midpoint Subdivision)\n";
        std::cout << " - Phim [3]: Dieu chinh khung cua so xen (Click & keo khung chu nhat)\n";
        std::cout << " - Phim [4]: To duong bien DE QUY (Click chon diem hat giong ben trong)\n";
        std::cout << " - Phim [5]: To duong bien KHONG DE QUY (Stack-based Boundary Fill)\n";
        std::cout << " - Phim [6]: To mau da giac DONG QUET (Scanline Polygon Fill)\n";
        std::cout << " - Phim [7]: Tu ve da giac bang chuot (Click dinh, Enter de khep kin)\n";
        std::cout << " - Phim [F1..F5]: Nap cac da giac mau tu file du lieu data/...\n";
        std::cout << " - Phim [TAB]: Chon mau to, [Z]: Undo, [C]: Clear, [G]: Grid, [R]: Reset\n";
        std::cout << "===============================================================\n\n";

        return true;
    }

    void buildUIButtons() {
        toolButtons.clear();
        actionButtons.clear();
        fileButtons.clear();

        float btnH = 26.0f;
        float btnY1 = 8.0f;
        float btnY2 = 38.0f;

        // Hàng 1: Nút công cụ
        std::vector<std::string> labels = {
            "1. Cohen Clip", "2. Binary Clip", "3. Edit Box",
            "4. Bound Rec", "5. Bound Stack", "6. Scanline", "7. Draw Poly"
        };
        float curX = 12.0f;
        for (size_t i = 0; i < labels.size(); ++i) {
            float w = 100.0f;
            UIButton btn;
            btn.id = static_cast<int>(i);
            btn.label = labels[i];
            btn.x = curX;
            btn.y = btnY1;
            btn.w = w;
            btn.h = btnH;
            btn.isSelected = (btn.id == currentTool);
            btn.bgColor = btn.isSelected ? Vec4(0.2f, 0.55f, 0.95f, 0.9f) : Vec4(0.18f, 0.20f, 0.25f, 0.85f);
            btn.textColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
            toolButtons.push_back(btn);
            curX += w + 6.0f;
        }

        // Hàng 1 (phía phải): Nút chức năng tiện ích
        std::vector<std::string> actLabels = {"Undo (Z)", "Clear (C)", "Grid (G)", "Axes (X)", "Reset (R)"};
        curX += 16.0f;
        for (size_t i = 0; i < actLabels.size(); ++i) {
            float w = 82.0f;
            UIButton btn;
            btn.id = 100 + static_cast<int>(i);
            btn.label = actLabels[i];
            btn.x = curX;
            btn.y = btnY1;
            btn.w = w;
            btn.h = btnH;
            btn.isSelected = false;
            btn.bgColor = Vec4(0.25f, 0.28f, 0.35f, 0.85f);
            btn.textColor = Vec4(0.9f, 0.9f, 0.9f, 1.0f);
            actionButtons.push_back(btn);
            curX += w + 6.0f;
        }

        // Hàng 2: Nút nạp file đa giác
        std::vector<std::string> fileLabels = {
            "F1: Star.txt", "F2: House.txt", "F3: Concave.txt", "F4: Arrow.txt", "F5: Hexagon.txt"
        };
        curX = 12.0f;
        for (size_t i = 0; i < fileLabels.size(); ++i) {
            float w = 115.0f;
            UIButton btn;
            btn.id = 200 + static_cast<int>(i);
            btn.label = fileLabels[i];
            btn.x = curX;
            btn.y = btnY2;
            btn.w = w;
            btn.h = btnH;
            btn.isSelected = false;
            btn.bgColor = Vec4(0.15f, 0.40f, 0.35f, 0.85f);
            btn.textColor = Vec4(0.85f, 1.0f, 0.9f, 1.0f);
            fileButtons.push_back(btn);
            curX += w + 6.0f;
        }
    }

    void loadPolygonFromFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Khong the mo file: " << path << "\n";
            return;
        }

        currentPolygon.clear();
        std::string line;
        int numVertices = 0;

        // Đọc số đỉnh (bỏ qua comment '#')
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::stringstream ss(line);
            if (ss >> numVertices) break;
        }

        int count = 0;
        while (count < numVertices && std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::stringstream ss(line);
            float vx, vy;
            if (ss >> vx >> vy) {
                currentPolygon.push_back(Point2D(vx, vy));
                count++;
            }
        }

        // Đọc seed point nếu có
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::stringstream ss(line);
            float sx, sy;
            if (ss >> sx >> sy) {
                lastSeedPoint = Point2D(sx, sy);
                hasSeedPoint = true;
                break;
            }
        }

        file.close();
        currentPolyFileName = path;
        filledPixels.clear();

        // Chuẩn bị canvas biên cho Boundary Fill
        rasterizePolygonBoundaryToCanvas();

        std::stringstream ss;
        ss << "Da nap da giac tu " << path << " (" << currentPolygon.size() << " dinh)";
        fillStatusMsg = ss.str();
        std::cout << "[Lab 2] " << fillStatusMsg << "\n";
    }

    // Vẽ đường biên của đa giác lên `fillCanvas` bằng thuật toán Bresenham
    void rasterizePolygonBoundaryToCanvas() {
        fillCanvas.clear(0);
        if (currentPolygon.size() < 3) return;

        uint32_t BORDER_COLOR = 0xFFFFFFFF; // Màu viền trắng
        int n = static_cast<int>(currentPolygon.size());

        for (int i = 0; i < n; ++i) {
            int next = (i + 1) % n;
            int x1 = static_cast<int>(std::round(currentPolygon[i].x));
            int y1 = static_cast<int>(std::round(currentPolygon[i].y));
            int x2 = static_cast<int>(std::round(currentPolygon[next].x));
            int y2 = static_cast<int>(std::round(currentPolygon[next].y));

            // Vẽ đoạn thẳng Bresenham vào canvas
            int dx = std::abs(x2 - x1);
            int dy = std::abs(y2 - y1);
            int sx = (x1 < x2) ? 1 : -1;
            int sy = (y1 < y2) ? 1 : -1;
            int err = dx - dy;

            int cx = x1, cy = y1;
            while (true) {
                fillCanvas.setPixel(cx, cy, BORDER_COLOR);
                if (cx == x2 && cy == y2) break;
                int e2 = 2 * err;
                if (e2 > -dy) {
                    err -= dy;
                    cx += sx;
                }
                if (e2 < dx) {
                    err += dx;
                    cy += sy;
                }
            }
        }
    }

    void executeFillAt(Point2D pt) {
        lastSeedPoint = pt;
        hasSeedPoint = true;
        filledPixels.clear();

        int sx = static_cast<int>(std::round(pt.x));
        int sy = static_cast<int>(std::round(pt.y));

        uint32_t BORDER_COLOR = 0xFFFFFFFF;
        uint32_t FILL_COLOR = 0x00FF00FF; // Đánh dấu đã tô

        if (currentTool == TOOL_BOUNDARY_REC) {
            rasterizePolygonBoundaryToCanvas();
            int count = boundaryFillRecursive(fillCanvas, sx, sy, FILL_COLOR, BORDER_COLOR, filledPixels, 6000);
            std::stringstream ss;
            ss << "Boundary Fill DE QUY hoan tat: " << count << " pixels (Seed: " << sx << ", " << sy << ")";
            fillStatusMsg = ss.str();
        } else if (currentTool == TOOL_BOUNDARY_STACK) {
            rasterizePolygonBoundaryToCanvas();
            int count = boundaryFillNonRecursive(fillCanvas, sx, sy, FILL_COLOR, BORDER_COLOR, filledPixels);
            std::stringstream ss;
            ss << "Boundary Fill STACK hoan tat: " << count << " pixels (Seed: " << sx << ", " << sy << ")";
            fillStatusMsg = ss.str();
        } else if (currentTool == TOOL_SCANLINE_FILL) {
            filledPixels = scanlinePolygonFill(currentPolygon);
            std::stringstream ss;
            ss << "Scanline Fill hoan tat: " << filledPixels.size() << " pixels (AET/ET)";
            fillStatusMsg = ss.str();
        }
    }

    void initShaders() {
        const char* vertSrc = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec4 aColor;
            out vec4 vColor;
            uniform mat4 uProjection;
            void main() {
                vColor = aColor;
                gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
            }
        )";

        const char* fragSrc = R"(
            #version 330 core
            in vec4 vColor;
            out vec4 FragColor;
            void main() {
                FragColor = vColor;
            }
        )";

        GLuint vert = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert, 1, &vertSrc, nullptr);
        glCompileShader(vert);

        GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag, 1, &fragSrc, nullptr);
        glCompileShader(frag);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vert);
        glAttachShader(shaderProgram, frag);
        glLinkProgram(shaderProgram);

        glDeleteShader(vert);
        glDeleteShader(frag);
    }

    Point2D screenToWorld(double mx, double my) {
        float wx = (static_cast<float>(mx) - panX) / zoom;
        float wy = (static_cast<float>(my) - panY) / zoom;
        return Point2D(wx, wy);
    }

    // ========================================================================
    // RENDER MESH BATCHING & VẼ HÌNH
    // ========================================================================
    void addQuad(float x, float y, float w, float h, const Vec4& color) {
        batchVertices.push_back({x,     y,     color.x, color.y, color.z, color.w});
        batchVertices.push_back({x + w, y,     color.x, color.y, color.z, color.w});
        batchVertices.push_back({x + w, y + h, color.x, color.y, color.z, color.w});

        batchVertices.push_back({x,     y,     color.x, color.y, color.z, color.w});
        batchVertices.push_back({x + w, y + h, color.x, color.y, color.z, color.w});
        batchVertices.push_back({x,     y + h, color.x, color.y, color.z, color.w});
    }

    void addLine(float x1, float y1, float x2, float y2, float thickness, const Vec4& color) {
        float dx = x2 - x1;
        float dy = y2 - y1;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 1e-4f) return;

        float nx = -dy / len * (thickness * 0.5f);
        float ny =  dx / len * (thickness * 0.5f);

        batchVertices.push_back({x1 + nx, y1 + ny, color.x, color.y, color.z, color.w});
        batchVertices.push_back({x2 + nx, y2 + ny, color.x, color.y, color.z, color.w});
        batchVertices.push_back({x2 - nx, y2 - ny, color.x, color.y, color.z, color.w});

        batchVertices.push_back({x1 + nx, y1 + ny, color.x, color.y, color.z, color.w});
        batchVertices.push_back({x2 - nx, y2 - ny, color.x, color.y, color.z, color.w});
        batchVertices.push_back({x1 - nx, y1 - ny, color.x, color.y, color.z, color.w});
    }

    void addWorldLine(float wx1, float wy1, float wx2, float wy2, float thickness, const Vec4& color) {
        float sx1 = panX + wx1 * zoom;
        float sy1 = panY + wy1 * zoom;
        float sx2 = panX + wx2 * zoom;
        float sy2 = panY + wy2 * zoom;
        addLine(sx1, sy1, sx2, sy2, thickness, color);
    }

    void addWorldPixel(int wx, int wy, const Vec4& color) {
        float sx = panX + wx * zoom;
        float sy = panY + wy * zoom;
        float pSize = std::max(1.0f, zoom);
        float gap = (zoom >= 8.0f) ? 1.0f : 0.0f;
        addQuad(sx, sy, pSize - gap, pSize - gap, color);
    }

    void drawChar5x7(char c, float x, float y, float size, const Vec4& color) {
        if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';

        uint8_t rows[7] = {0};
        switch (c) {
            case 'A': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x1F; rows[4]=0x11; rows[5]=0x11; rows[6]=0x11; break;
            case 'B': rows[0]=0x1E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x1E; rows[4]=0x11; rows[5]=0x11; rows[6]=0x1E; break;
            case 'C': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x10; rows[3]=0x10; rows[4]=0x10; rows[5]=0x11; rows[6]=0x0E; break;
            case 'D': rows[0]=0x1C; rows[1]=0x12; rows[2]=0x11; rows[3]=0x11; rows[4]=0x11; rows[5]=0x12; rows[6]=0x1C; break;
            case 'E': rows[0]=0x1F; rows[1]=0x10; rows[2]=0x10; rows[3]=0x1E; rows[4]=0x10; rows[5]=0x10; rows[6]=0x1F; break;
            case 'F': rows[0]=0x1F; rows[1]=0x10; rows[2]=0x10; rows[3]=0x1E; rows[4]=0x10; rows[5]=0x10; rows[6]=0x10; break;
            case 'G': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x10; rows[3]=0x17; rows[4]=0x11; rows[5]=0x11; rows[6]=0x0F; break;
            case 'H': rows[0]=0x11; rows[1]=0x11; rows[2]=0x11; rows[3]=0x1F; rows[4]=0x11; rows[5]=0x11; rows[6]=0x11; break;
            case 'I': rows[0]=0x1F; rows[1]=0x04; rows[2]=0x04; rows[3]=0x04; rows[4]=0x04; rows[5]=0x04; rows[6]=0x1F; break;
            case 'J': rows[0]=0x07; rows[1]=0x02; rows[2]=0x02; rows[3]=0x02; rows[4]=0x12; rows[5]=0x12; rows[6]=0x0C; break;
            case 'K': rows[0]=0x11; rows[1]=0x12; rows[2]=0x14; rows[3]=0x18; rows[4]=0x14; rows[5]=0x12; rows[6]=0x11; break;
            case 'L': rows[0]=0x10; rows[1]=0x10; rows[2]=0x10; rows[3]=0x10; rows[4]=0x10; rows[5]=0x10; rows[6]=0x1F; break;
            case 'M': rows[0]=0x11; rows[1]=0x1B; rows[2]=0x15; rows[3]=0x11; rows[4]=0x11; rows[5]=0x11; rows[6]=0x11; break;
            case 'N': rows[0]=0x11; rows[1]=0x19; rows[2]=0x15; rows[3]=0x13; rows[4]=0x11; rows[5]=0x11; rows[6]=0x11; break;
            case 'O': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x11; rows[4]=0x11; rows[5]=0x11; rows[6]=0x0E; break;
            case 'P': rows[0]=0x1E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x1E; rows[4]=0x10; rows[5]=0x10; rows[6]=0x10; break;
            case 'Q': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x11; rows[4]=0x15; rows[5]=0x12; rows[6]=0x0D; break;
            case 'R': rows[0]=0x1E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x1E; rows[4]=0x14; rows[5]=0x12; rows[6]=0x11; break;
            case 'S': rows[0]=0x0F; rows[1]=0x10; rows[2]=0x10; rows[3]=0x0E; rows[4]=0x01; rows[5]=0x01; rows[6]=0x1E; break;
            case 'T': rows[0]=0x1F; rows[1]=0x04; rows[2]=0x04; rows[3]=0x04; rows[4]=0x04; rows[5]=0x04; rows[6]=0x04; break;
            case 'U': rows[0]=0x11; rows[1]=0x11; rows[2]=0x11; rows[3]=0x11; rows[4]=0x11; rows[5]=0x11; rows[6]=0x0E; break;
            case 'V': rows[0]=0x11; rows[1]=0x11; rows[2]=0x11; rows[3]=0x11; rows[4]=0x11; rows[5]=0x0A; rows[6]=0x04; break;
            case 'W': rows[0]=0x11; rows[1]=0x11; rows[2]=0x11; rows[3]=0x11; rows[4]=0x15; rows[5]=0x1B; rows[6]=0x11; break;
            case 'X': rows[0]=0x11; rows[1]=0x11; rows[2]=0x0A; rows[3]=0x04; rows[4]=0x0A; rows[5]=0x11; rows[6]=0x11; break;
            case 'Y': rows[0]=0x11; rows[1]=0x11; rows[2]=0x0A; rows[3]=0x04; rows[4]=0x04; rows[5]=0x04; rows[6]=0x04; break;
            case 'Z': rows[0]=0x1F; rows[1]=0x01; rows[2]=0x02; rows[3]=0x04; rows[4]=0x08; rows[5]=0x10; rows[6]=0x1F; break;
            case '0': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x13; rows[3]=0x15; rows[4]=0x19; rows[5]=0x11; rows[6]=0x0E; break;
            case '1': rows[0]=0x04; rows[1]=0x0C; rows[2]=0x04; rows[3]=0x04; rows[4]=0x04; rows[5]=0x04; rows[6]=0x0E; break;
            case '2': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x01; rows[3]=0x06; rows[4]=0x08; rows[5]=0x10; rows[6]=0x1F; break;
            case '3': rows[0]=0x1F; rows[1]=0x02; rows[2]=0x04; rows[3]=0x02; rows[4]=0x01; rows[5]=0x11; rows[6]=0x0E; break;
            case '4': rows[0]=0x02; rows[1]=0x06; rows[2]=0x0A; rows[3]=0x12; rows[4]=0x1F; rows[5]=0x02; rows[6]=0x02; break;
            case '5': rows[0]=0x1F; rows[1]=0x10; rows[2]=0x1E; rows[3]=0x01; rows[4]=0x01; rows[5]=0x11; rows[6]=0x0E; break;
            case '6': rows[0]=0x06; rows[1]=0x08; rows[2]=0x10; rows[3]=0x1E; rows[4]=0x11; rows[5]=0x11; rows[6]=0x0E; break;
            case '7': rows[0]=0x1F; rows[1]=0x01; rows[2]=0x02; rows[3]=0x04; rows[4]=0x08; rows[5]=0x08; rows[6]=0x08; break;
            case '8': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x0E; rows[4]=0x11; rows[5]=0x11; rows[6]=0x0E; break;
            case '9': rows[0]=0x0E; rows[1]=0x11; rows[2]=0x11; rows[3]=0x0F; rows[4]=0x01; rows[5]=0x02; rows[6]=0x0C; break;
            case '.': rows[0]=0x00; rows[1]=0x00; rows[2]=0x00; rows[3]=0x00; rows[4]=0x00; rows[5]=0x06; rows[6]=0x06; break;
            case ',': rows[0]=0x00; rows[1]=0x00; rows[2]=0x00; rows[3]=0x00; rows[4]=0x04; rows[5]=0x04; rows[6]=0x08; break;
            case ':': rows[0]=0x00; rows[1]=0x04; rows[2]=0x04; rows[3]=0x00; rows[4]=0x04; rows[5]=0x04; rows[6]=0x00; break;
            case '-': rows[0]=0x00; rows[1]=0x00; rows[2]=0x00; rows[3]=0x1F; rows[4]=0x00; rows[5]=0x00; rows[6]=0x00; break;
            case '+': rows[0]=0x00; rows[1]=0x04; rows[2]=0x04; rows[3]=0x1F; rows[4]=0x04; rows[5]=0x04; rows[6]=0x00; break;
            case '=': rows[0]=0x00; rows[1]=0x1F; rows[2]=0x00; rows[3]=0x1F; rows[4]=0x00; rows[5]=0x00; rows[6]=0x00; break;
            case '(': rows[0]=0x02; rows[1]=0x04; rows[2]=0x08; rows[3]=0x08; rows[4]=0x08; rows[5]=0x04; rows[6]=0x02; break;
            case ')': rows[0]=0x08; rows[1]=0x04; rows[2]=0x02; rows[3]=0x02; rows[4]=0x02; rows[5]=0x04; rows[6]=0x08; break;
            case '[': rows[0]=0x0E; rows[1]=0x08; rows[2]=0x08; rows[3]=0x08; rows[4]=0x08; rows[5]=0x08; rows[6]=0x0E; break;
            case ']': rows[0]=0x0E; rows[1]=0x02; rows[2]=0x02; rows[3]=0x02; rows[4]=0x02; rows[5]=0x02; rows[6]=0x0E; break;
            case '<': rows[0]=0x02; rows[1]=0x04; rows[2]=0x08; rows[3]=0x10; rows[4]=0x08; rows[5]=0x04; rows[6]=0x02; break;
            case '>': rows[0]=0x08; rows[1]=0x04; rows[2]=0x02; rows[3]=0x01; rows[4]=0x02; rows[5]=0x04; rows[6]=0x08; break;
            case '/': rows[0]=0x01; rows[1]=0x02; rows[2]=0x04; rows[3]=0x08; rows[4]=0x10; rows[5]=0x00; rows[6]=0x00; break;
            case '|': rows[0]=0x04; rows[1]=0x04; rows[2]=0x04; rows[3]=0x04; rows[4]=0x04; rows[5]=0x04; rows[6]=0x04; break;
            default: return;
        }

        float pW = size / 5.0f;
        float pH = size / 7.0f;
        for (int r = 0; r < 7; ++r) {
            float py = y + r * pH;
            for (int c_idx = 0; c_idx < 5; ++c_idx) {
                if (rows[r] & (1 << (4 - c_idx))) {
                    float px = x + c_idx * pW;
                    addQuad(px, py, pW, pH, color);
                }
            }
        }
    }

    void drawText(const std::string& text, float x, float y, float size, const Vec4& color) {
        float curX = x;
        float spacing = size * 0.25f;
        for (char c : text) {
            if (c == ' ') {
                curX += size * 0.6f;
                continue;
            }
            drawChar5x7(c, curX, y, size, color);
            curX += size + spacing;
        }
    }

    // ========================================================================
    // RENDER MAIN
    // ========================================================================
    void render() {
        glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        batchVertices.clear();

        // 1. Lưới Pixel & Trục tọa độ
        if (showGrid && zoom >= 4.0f) {
            Vec4 gridColor(0.18f, 0.20f, 0.26f, 0.6f);
            float startWorldX = (-panX) / zoom;
            float endWorldX = (windowWidth - panX) / zoom;
            float startWorldY = (-panY) / zoom;
            float endWorldY = (windowHeight - panY) / zoom;

            int minX = static_cast<int>(std::floor(startWorldX));
            int maxX = static_cast<int>(std::ceil(endWorldX));
            int minY = static_cast<int>(std::floor(startWorldY));
            int maxY = static_cast<int>(std::ceil(endWorldY));

            if (maxX - minX < 400) {
                for (int x = minX; x <= maxX; ++x) {
                    float sx = panX + x * zoom;
                    addLine(sx, 0, sx, windowHeight, 1.0f, gridColor);
                }
            }
            if (maxY - minY < 400) {
                for (int y = minY; y <= maxY; ++y) {
                    float sy = panY + y * zoom;
                    addLine(0, sy, windowWidth, sy, 1.0f, gridColor);
                }
            }
        }

        if (showAxes) {
            Vec4 axisColorX(0.9f, 0.3f, 0.3f, 0.5f);
            Vec4 axisColorY(0.3f, 0.9f, 0.3f, 0.5f);
            addLine(0, panY, windowWidth, panY, 1.5f, axisColorX);
            addLine(panX, 0, panX, windowHeight, 1.5f, axisColorY);
        }

        // 2. Vẽ Cửa sổ xén (Clipping Rectangle)
        {
            Vec4 rectCol(0.2f, 0.8f, 0.9f, 0.9f); // Màu Cyan
            float thickness = 2.0f;
            addWorldLine(clipRect.xMin, clipRect.yMin, clipRect.xMax, clipRect.yMin, thickness, rectCol);
            addWorldLine(clipRect.xMax, clipRect.yMin, clipRect.xMax, clipRect.yMax, thickness, rectCol);
            addWorldLine(clipRect.xMax, clipRect.yMax, clipRect.xMin, clipRect.yMax, thickness, rectCol);
            addWorldLine(clipRect.xMin, clipRect.yMax, clipRect.xMin, clipRect.yMin, thickness, rectCol);

            // Tô nền mờ nhẹ cho cửa sổ xén
            float sx = panX + clipRect.xMin * zoom;
            float sy = panY + clipRect.yMin * zoom;
            float sw = (clipRect.xMax - clipRect.xMin) * zoom;
            float sh = (clipRect.yMax - clipRect.yMin) * zoom;
            addQuad(sx, sy, sw, sh, Vec4(0.1f, 0.3f, 0.4f, 0.12f));

            // Nhãn kích thước cửa sổ
            std::stringstream ssW;
            ssW << "Clip Window: [" << (int)clipRect.xMin << "," << (int)clipRect.yMin 
                << "] to [" << (int)clipRect.xMax << "," << (int)clipRect.yMax << "]";
            drawText(ssW.str(), sx + 6.0f, sy + 6.0f, 9.0f, Vec4(0.5f, 0.9f, 1.0f, 0.8f));
        }

        // 3. Vẽ các đoạn thẳng đã xén
        for (const auto& cl : clippedLines) {
            // Đoạn thẳng gốc (màu mờ)
            addWorldLine(cl.p1Orig.x, cl.p1Orig.y, cl.p2Orig.x, cl.p2Orig.y, 1.2f, Vec4(0.4f, 0.45f, 0.55f, 0.4f));

            if (cl.accepted) {
                // Đoạn xén được giữ lại (màu xanh lá neon rực rỡ)
                addWorldLine(cl.p1Clipped.x, cl.p1Clipped.y, cl.p2Clipped.x, cl.p2Clipped.y, 3.0f, Vec4(0.1f, 0.95f, 0.4f, 1.0f));

                // Điểm mút giao điểm
                addWorldPixel((int)std::round(cl.p1Clipped.x), (int)std::round(cl.p1Clipped.y), Vec4(1.0f, 0.3f, 0.3f, 1.0f));
                addWorldPixel((int)std::round(cl.p2Clipped.x), (int)std::round(cl.p2Clipped.y), Vec4(1.0f, 0.3f, 0.3f, 1.0f));
            }
        }

        // 4. Xem trước đoạn thẳng đang kéo chuột (Live Preview Clipping)
        if (hasClipPreview) {
            // Đoạn gốc
            addWorldLine(previewClipResult.p1Orig.x, previewClipResult.p1Orig.y,
                         previewClipResult.p2Orig.x, previewClipResult.p2Orig.y, 1.5f, Vec4(0.9f, 0.8f, 0.3f, 0.6f));

            if (previewClipResult.accepted) {
                addWorldLine(previewClipResult.p1Clipped.x, previewClipResult.p1Clipped.y,
                             previewClipResult.p2Clipped.x, previewClipResult.p2Clipped.y, 3.5f, Vec4(0.2f, 1.0f, 0.3f, 1.0f));
            }

            // Nhãn mã Outcode tại hai đầu mút
            float sx1 = panX + previewClipResult.p1Orig.x * zoom;
            float sy1 = panY + previewClipResult.p1Orig.y * zoom;
            float sx2 = panX + previewClipResult.p2Orig.x * zoom;
            float sy2 = panY + previewClipResult.p2Orig.y * zoom;

            std::string codeStr1 = "P1: " + outCodeToString(previewClipResult.code1Orig);
            std::string codeStr2 = "P2: " + outCodeToString(previewClipResult.code2Orig);
            drawText(codeStr1, sx1 + 8.0f, sy1 - 12.0f, 9.0f, Vec4(1.0f, 0.8f, 0.2f, 1.0f));
            drawText(codeStr2, sx2 + 8.0f, sy2 - 12.0f, 9.0f, Vec4(1.0f, 0.8f, 0.2f, 1.0f));
        }

        // 5. Vẽ các Pixel đã tô màu (Filled Pixels)
        Vec4 fillCol = palette[currentColorIdx];
        for (const auto& p : filledPixels) {
            addWorldPixel(p.x, p.y, fillCol);
        }

        // 6. Vẽ đa giác hiện tại (Polygon Edges & Vertices)
        if (currentPolygon.size() >= 2) {
            Vec4 polyCol(1.0f, 1.0f, 1.0f, 0.9f);
            size_t n = currentPolygon.size();
            for (size_t i = 0; i < n; ++i) {
                size_t next = (i + 1) % n;
                addWorldLine(currentPolygon[i].x, currentPolygon[i].y,
                             currentPolygon[next].x, currentPolygon[next].y, 2.0f, polyCol);

                // Điểm đỉnh
                float sx = panX + currentPolygon[i].x * zoom;
                float sy = panY + currentPolygon[i].y * zoom;
                addQuad(sx - 3, sy - 3, 6, 6, Vec4(1.0f, 0.4f, 0.2f, 1.0f));
            }
        }

        // Đa giác đang vẽ dở (Custom polygon)
        if (!customPolyBuilding.empty()) {
            Vec4 buildingCol(0.9f, 0.8f, 0.3f, 0.8f);
            for (size_t i = 0; i + 1 < customPolyBuilding.size(); ++i) {
                addWorldLine(customPolyBuilding[i].x, customPolyBuilding[i].y,
                             customPolyBuilding[i + 1].x, customPolyBuilding[i + 1].y, 2.0f, buildingCol);
            }
            if (isMouseDown && !customPolyBuilding.empty()) {
                addWorldLine(customPolyBuilding.back().x, customPolyBuilding.back().y,
                             dragCurrentWorld.x, dragCurrentWorld.y, 1.5f, Vec4(0.9f, 0.5f, 0.3f, 0.6f));
            }
        }

        // Điểm hạt giống (Seed Point)
        if (hasSeedPoint && (currentTool == TOOL_BOUNDARY_REC || currentTool == TOOL_BOUNDARY_STACK)) {
            float sx = panX + lastSeedPoint.x * zoom;
            float sy = panY + lastSeedPoint.y * zoom;
            addLine(sx - 8, sy, sx + 8, sy, 2.0f, Vec4(1.0f, 0.2f, 0.2f, 1.0f));
            addLine(sx, sy - 8, sx, sy + 8, 2.0f, Vec4(1.0f, 0.2f, 0.2f, 1.0f));
            drawText("SEED", sx + 10, sy - 6, 8.0f, Vec4(1.0f, 0.4f, 0.4f, 1.0f));
        }

        // 7. Vẽ Thanh công cụ UI (Toolbar)
        addQuad(0, 0, windowWidth, 72.0f, Vec4(0.10f, 0.12f, 0.16f, 0.95f));
        addLine(0, 72.0f, windowWidth, 72.0f, 1.5f, Vec4(0.25f, 0.28f, 0.35f, 0.8f));

        for (const auto& btn : toolButtons) {
            addQuad(btn.x, btn.y, btn.w, btn.h, btn.bgColor);
            if (btn.isSelected) {
                addLine(btn.x, btn.y + btn.h - 2, btn.x + btn.w, btn.y + btn.h - 2, 2.0f, Vec4(1.0f, 0.9f, 0.3f, 1.0f));
            }
            drawText(btn.label, btn.x + 8.0f, btn.y + 7.0f, 9.0f, btn.textColor);
        }

        for (const auto& btn : actionButtons) {
            addQuad(btn.x, btn.y, btn.w, btn.h, btn.bgColor);
            drawText(btn.label, btn.x + 8.0f, btn.y + 7.0f, 9.0f, btn.textColor);
        }

        for (const auto& btn : fileButtons) {
            addQuad(btn.x, btn.y, btn.w, btn.h, btn.bgColor);
            drawText(btn.label, btn.x + 8.0f, btn.y + 7.0f, 9.0f, btn.textColor);
        }

        // 8. Bảng thông tin HUD góc dưới màn hình
        addQuad(12.0f, windowHeight - 95.0f, windowWidth - 24.0f, 85.0f, Vec4(0.10f, 0.12f, 0.16f, 0.92f));
        addLine(12.0f, windowHeight - 95.0f, windowWidth - 12.0f, windowHeight - 95.0f, 1.0f, Vec4(0.25f, 0.3f, 0.4f, 0.8f));

        std::string modeTitle = "CHE DO HIEN TAI: " + getToolTitle(currentTool);
        drawText(modeTitle, 24.0f, windowHeight - 85.0f, 11.0f, Vec4(1.0f, 0.9f, 0.3f, 1.0f));

        std::stringstream ssInfo;
        if (currentTool == TOOL_COHEN_SUTHERLAND || currentTool == TOOL_BINARY_SUBDIVISION) {
            ssInfo << "Thao tac: Nhan giu & keo chuot trai de ve doan thang can xen. | Da luu: "
                   << clippedLines.size() << " doan. | Zoom: " << std::fixed << std::setprecision(1) << zoom << "x";
        } else if (currentTool == TOOL_EDIT_CLIP_WINDOW) {
            ssInfo << "Thao tac: Keo chuot de doi kich thuoc Cua so Xen. Hien tai: ["
                   << (int)clipRect.xMin << "," << (int)clipRect.yMin << "] -> ["
                   << (int)clipRect.xMax << "," << (int)clipRect.yMax << "]";
        } else if (currentTool == TOOL_BOUNDARY_REC || currentTool == TOOL_BOUNDARY_STACK) {
            ssInfo << fillStatusMsg << " | So pixel to: " << filledPixels.size()
                   << " | File da giac: " << currentPolyFileName;
        } else if (currentTool == TOOL_SCANLINE_FILL) {
            ssInfo << fillStatusMsg << " | Click chuot trai de to mau lai bang Dong quet AET/ET.";
        } else if (currentTool == TOOL_DRAW_CUSTOM_POLYGON) {
            ssInfo << "Click chuot trai de them dinh (Da co " << customPolyBuilding.size()
                   << " dinh). Nhan ENTER de chot va khep kin da giac!";
        }
        drawText(ssInfo.str(), 24.0f, windowHeight - 65.0f, 9.0f, Vec4(0.85f, 0.9f, 1.0f, 0.9f));

        // Dòng trạng thái xén chi tiết nếu có
        std::stringstream ssDetail;
        if (hasClipPreview) {
            ssDetail << "Ket qua xen: " << (previewClipResult.accepted ? "ACCEPTED (Cat hoan tat)" : "REJECTED (Loai bo)")
                     << " | Iterations: " << previewClipResult.iterations
                     << " | P1=(" << (int)previewClipResult.p1Orig.x << "," << (int)previewClipResult.p1Orig.y << ") Outcode="
                     << outCodeToString(previewClipResult.code1Orig)
                     << " | P2=(" << (int)previewClipResult.p2Orig.x << "," << (int)previewClipResult.p2Orig.y << ") Outcode="
                     << outCodeToString(previewClipResult.code2Orig);
        } else {
            ssDetail << "Phim tat: [1..7] Chon cong cu | [F1..F5] Nap da giac tu file | [TAB] Doi mau to | [Z] Undo | [C] Clear | [R] Reset view";
        }
        drawText(ssDetail.str(), 24.0f, windowHeight - 42.0f, 9.0f, Vec4(0.4f, 0.85f, 0.7f, 1.0f));

        // Gửi dữ liệu đỉnh lên GPU
        if (!batchVertices.empty()) {
            glUseProgram(shaderProgram);

            // Ma trận chiếu trực giao 2D (Ortho Projection)
            float left = 0.0f, right = static_cast<float>(windowWidth);
            float top = 0.0f, bottom = static_cast<float>(windowHeight);
            float ortho[16] = {
                2.0f / (right - left), 0.0f, 0.0f, 0.0f,
                0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
                0.0f, 0.0f, -1.0f, 0.0f,
                -(right + left) / (right - left), -(top + bottom) / (top - bottom), 0.0f, 1.0f
            };

            GLint locProj = glGetUniformLocation(shaderProgram, "uProjection");
            glUniformMatrix4fv(locProj, 1, GL_FALSE, ortho);

            glBindVertexArray(vaoBatch);
            glBindBuffer(GL_ARRAY_BUFFER, vboBatch);
            glBufferData(GL_ARRAY_BUFFER, batchVertices.size() * sizeof(BatchVertex), batchVertices.data(), GL_STREAM_DRAW);

            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(batchVertices.size()));
            glBindVertexArray(0);
        }
    }

    void run() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            render();
            glfwSwapBuffers(window);
        }
    }

    // ========================================================================
    // XỬ LÝ SỰ KIỆN CHUỘT & BÀN PHÍM
    // ========================================================================
    static void framebufferSizeCallback(GLFWwindow* win, int w, int h) {
        if (instance) {
            instance->windowWidth = w;
            instance->windowHeight = h;
            glViewport(0, 0, w, h);
            instance->buildUIButtons();
        }
    }

    static void mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {
        if (!instance) return;
        double mx, my;
        glfwGetCursorPos(win, &mx, &my);

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                // Kiểm tra click vào nút UI trên toolbar
                if (my <= 72.0) {
                    for (auto& btn : instance->toolButtons) {
                        if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
                            instance->currentTool = static_cast<Lab2Tool>(btn.id);
                            for (auto& b : instance->toolButtons) b.isSelected = (b.id == instance->currentTool);
                            instance->buildUIButtons();
                            if (instance->currentTool == TOOL_SCANLINE_FILL) {
                                instance->executeFillAt(instance->lastSeedPoint);
                            }
                            return;
                        }
                    }
                    for (const auto& btn : instance->actionButtons) {
                        if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
                            if (btn.id == 100) instance->undo();
                            else if (btn.id == 101) instance->clearAll();
                            else if (btn.id == 102) instance->showGrid = !instance->showGrid;
                            else if (btn.id == 103) instance->showAxes = !instance->showAxes;
                            else if (btn.id == 104) instance->resetView();
                            return;
                        }
                    }
                    for (const auto& btn : instance->fileButtons) {
                        if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
                            int fId = btn.id - 200;
                            if (fId == 0) instance->loadPolygonFromFile("data/polygon_star.txt");
                            else if (fId == 1) instance->loadPolygonFromFile("data/polygon_house.txt");
                            else if (fId == 2) instance->loadPolygonFromFile("data/polygon_concave.txt");
                            else if (fId == 3) instance->loadPolygonFromFile("data/polygon_arrow.txt");
                            else if (fId == 4) instance->loadPolygonFromFile("data/polygon_hexagon.txt");
                            return;
                        }
                    }
                    return;
                }

                // Click trên Canvas
                instance->isMouseDown = true;
                instance->dragStartWorld = instance->screenToWorld(mx, my);
                instance->dragCurrentWorld = instance->dragStartWorld;

                if (instance->currentTool == TOOL_BOUNDARY_REC ||
                    instance->currentTool == TOOL_BOUNDARY_STACK ||
                    instance->currentTool == TOOL_SCANLINE_FILL) {
                    instance->executeFillAt(instance->dragStartWorld);
                } else if (instance->currentTool == TOOL_DRAW_CUSTOM_POLYGON) {
                    instance->customPolyBuilding.push_back(instance->dragStartWorld);
                }
            } else if (action == GLFW_RELEASE) {
                if (instance->isMouseDown) {
                    instance->isMouseDown = false;
                    instance->dragCurrentWorld = instance->screenToWorld(mx, my);

                    if (instance->currentTool == TOOL_COHEN_SUTHERLAND || instance->currentTool == TOOL_BINARY_SUBDIVISION) {
                        if (instance->hasClipPreview) {
                            instance->clippedLines.push_back(instance->previewClipResult);
                            instance->hasClipPreview = false;
                        }
                    } else if (instance->currentTool == TOOL_EDIT_CLIP_WINDOW) {
                        instance->clipRect.set(instance->dragStartWorld.x, instance->dragStartWorld.y,
                                              instance->dragCurrentWorld.x, instance->dragCurrentWorld.y);
                    }
                }
            }
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                instance->isPanning = true;
                instance->lastMouseX = mx;
                instance->lastMouseY = my;
            } else if (action == GLFW_RELEASE) {
                instance->isPanning = false;
            }
        }
    }

    static void cursorPosCallback(GLFWwindow* win, double mx, double my) {
        if (!instance) return;

        if (instance->isPanning) {
            double dx = mx - instance->lastMouseX;
            double dy = my - instance->lastMouseY;
            instance->panX += static_cast<float>(dx);
            instance->panY += static_cast<float>(dy);
            instance->lastMouseX = mx;
            instance->lastMouseY = my;
            return;
        }

        instance->lastMouseX = mx;
        instance->lastMouseY = my;

        if (instance->isMouseDown) {
            instance->dragCurrentWorld = instance->screenToWorld(mx, my);

            if (instance->currentTool == TOOL_COHEN_SUTHERLAND) {
                instance->previewClipResult = clipCohenSutherland(instance->dragStartWorld, instance->dragCurrentWorld, instance->clipRect);
                instance->hasClipPreview = true;
            } else if (instance->currentTool == TOOL_BINARY_SUBDIVISION) {
                instance->previewClipResult = clipBinarySubdivision(instance->dragStartWorld, instance->dragCurrentWorld, instance->clipRect);
                instance->hasClipPreview = true;
            } else if (instance->currentTool == TOOL_EDIT_CLIP_WINDOW) {
                instance->clipRect.set(instance->dragStartWorld.x, instance->dragStartWorld.y,
                                      instance->dragCurrentWorld.x, instance->dragCurrentWorld.y);
            }
        }
    }

    static void scrollCallback(GLFWwindow* win, double xoff, double yoff) {
        if (!instance) return;
        double mx, my;
        glfwGetCursorPos(win, &mx, &my);

        float oldZoom = instance->zoom;
        float factor = (yoff > 0) ? 1.15f : 0.85f;
        instance->zoom = std::clamp(instance->zoom * factor, 0.15f, 64.0f);

        instance->panX = static_cast<float>(mx) - (static_cast<float>(mx) - instance->panX) * (instance->zoom / oldZoom);
        instance->panY = static_cast<float>(my) - (static_cast<float>(my) - instance->panY) * (instance->zoom / oldZoom);
    }

    static void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
        if (!instance || action != GLFW_PRESS) return;

        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_7) {
            instance->currentTool = static_cast<Lab2Tool>(key - GLFW_KEY_1);
            instance->buildUIButtons();
            if (instance->currentTool == TOOL_SCANLINE_FILL) {
                instance->executeFillAt(instance->lastSeedPoint);
            }
        } else if (key == GLFW_KEY_F1) instance->loadPolygonFromFile("data/polygon_star.txt");
        else if (key == GLFW_KEY_F2) instance->loadPolygonFromFile("data/polygon_house.txt");
        else if (key == GLFW_KEY_F3) instance->loadPolygonFromFile("data/polygon_concave.txt");
        else if (key == GLFW_KEY_F4) instance->loadPolygonFromFile("data/polygon_arrow.txt");
        else if (key == GLFW_KEY_F5) instance->loadPolygonFromFile("data/polygon_hexagon.txt");
        else if (key == GLFW_KEY_TAB) {
            instance->currentColorIdx = (instance->currentColorIdx + 1) % instance->palette.size();
        } else if (key == GLFW_KEY_Z) instance->undo();
        else if (key == GLFW_KEY_C) instance->clearAll();
        else if (key == GLFW_KEY_G) instance->showGrid = !instance->showGrid;
        else if (key == GLFW_KEY_X) instance->showAxes = !instance->showAxes;
        else if (key == GLFW_KEY_R) instance->resetView();
        else if (key == GLFW_KEY_ENTER) {
            // Chốt đa giác tự vẽ
            if (instance->currentTool == TOOL_DRAW_CUSTOM_POLYGON && instance->customPolyBuilding.size() >= 3) {
                instance->currentPolygon = instance->customPolyBuilding;
                instance->customPolyBuilding.clear();
                instance->currentPolyFileName = "Custom_Drawn_Polygon";
                instance->rasterizePolygonBoundaryToCanvas();
                instance->currentTool = TOOL_SCANLINE_FILL;
                instance->executeFillAt(Point2D(0, 0));
                instance->buildUIButtons();
            }
        } else if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(win, GLFW_TRUE);
        }
    }

    void undo() {
        if (!clippedLines.empty()) {
            clippedLines.pop_back();
        }
        filledPixels.clear();
    }

    void clearAll() {
        clippedLines.clear();
        filledPixels.clear();
        hasClipPreview = false;
        hasSeedPoint = false;
        customPolyBuilding.clear();
    }

    void resetView() {
        panX = windowWidth * 0.5f;
        panY = windowHeight * 0.5f;
        zoom = 1.0f;
    }

    bool runAutoTest(int frames = 60) {
        bool algPass = runLab2Tests();
        for (int i = 0; i < frames; ++i) {
            glfwPollEvents();
            render();
            glfwSwapBuffers(window);
        }
        return algPass;
    }
};

inline Application* Application::instance = nullptr;

} // namespace Lab2

#endif // LAB2_APP_HPP
