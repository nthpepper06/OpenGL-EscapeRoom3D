#ifndef LAB1_APP_HPP
#define LAB1_APP_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

#include "../Math3D.hpp"
#include "Lab1Algorithms.hpp"

namespace Lab1 {

// Các công cụ vẽ trong Lab 1
enum ToolType {
    TOOL_LINE_BRESENHAM = 0,    // 1.1. Đoạn thẳng Bresenham (4 trường hợp m)
    TOOL_LINE_MIDPOINT = 1,     // 1.1. Đoạn thẳng Midpoint (4 trường hợp m)
    TOOL_CIRCLE_BRESENHAM = 2,  // 1.2. Đường tròn Bresenham
    TOOL_CIRCLE_MIDPOINT = 3,   // 1.2. Đường tròn Midpoint
    TOOL_ELLIPSE_MIDPOINT = 4,  // 1.3. Ellipse Midpoint
    TOOL_PARABOLA_MIDPOINT = 5, // 1.4. Parabol Midpoint
    TOOL_HYPERBOLA_MIDPOINT = 6,// 1.4. Hyperbol Midpoint
    TOOL_SIN_WAVE = 7,          // 1.4. Sóng Sin(x)
    TOOL_COS_WAVE = 8           // 1.4. Sóng Cos(x)
};

inline std::string getToolName(ToolType t) {
    switch (t) {
        case TOOL_LINE_BRESENHAM: return "1.1. Doan thang (Bresenham)";
        case TOOL_LINE_MIDPOINT: return "1.1. Doan thang (Midpoint)";
        case TOOL_CIRCLE_BRESENHAM: return "1.2. Duong tron (Bresenham)";
        case TOOL_CIRCLE_MIDPOINT: return "1.2. Duong tron (Midpoint)";
        case TOOL_ELLIPSE_MIDPOINT: return "1.3. Ellipse (Midpoint)";
        case TOOL_PARABOLA_MIDPOINT: return "1.4. Parabol (Midpoint)";
        case TOOL_HYPERBOLA_MIDPOINT: return "1.4. Hyperbol (Midpoint)";
        case TOOL_SIN_WAVE: return "1.4. Song Sin(x)";
        case TOOL_COS_WAVE: return "1.4. Song Cos(x)";
        default: return "Khong xac dinh";
    }
}

// Cấu trúc một hình đã vẽ trên Canvas
struct DrawnShape {
    ToolType tool;
    Vec4 color;
    std::vector<PixelPoint> pixels;
    PixelPoint p1;
    PixelPoint p2;
    LineSlopeCase slopeCase = CASE_HORIZONTAL;
    float slope = 0.0f;
    std::string info;
};

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

    // Quản lý Camera 2D & Tọa độ
    float zoom = 1.0f;          // Tỷ lệ phóng to (hỗ trợ zoom xem từng pixel)
    float panX = 0.0f;          // Độ dịch chuyển X
    float panY = 0.0f;          // Độ dịch chuyển Y
    bool showGrid = true;       // Bật/tắt lưới pixel
    bool showAxes = true;       // Bật/tắt trục tọa độ Oxy

    // Trạng thái vẽ & Tương tác chuột
    ToolType currentTool = TOOL_LINE_BRESENHAM;
    int currentColorIdx = 0;
    std::vector<Vec4> palette = {
        Vec4(0.20f, 0.95f, 0.40f, 1.0f), // Xanh lá sáng (Neon Green)
        Vec4(0.95f, 0.25f, 0.25f, 1.0f), // Đỏ tươi
        Vec4(0.25f, 0.65f, 0.95f, 1.0f), // Xanh dương
        Vec4(0.95f, 0.90f, 0.20f, 1.0f), // Vàng chanh
        Vec4(0.20f, 0.95f, 0.95f, 1.0f), // Cyan
        Vec4(0.95f, 0.40f, 0.95f, 1.0f), // Magenta
        Vec4(0.98f, 0.60f, 0.15f, 1.0f), // Cam
        Vec4(0.95f, 0.95f, 0.95f, 1.0f)  // Trắng
    };

    bool isMouseDown = false;
    bool isPanning = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    // Tọa độ click chuột logic trên canvas (hệ tọa độ pixel thế giới)
    PixelPoint startPoint;
    PixelPoint currentPoint;
    bool hasStartPoint = false;

    // Hình vẽ xem trước (Live Preview / Ghost Shape)
    std::vector<PixelPoint> previewPixels;
    LineSlopeCase previewSlopeCase = CASE_HORIZONTAL;
    float previewSlope = 0.0f;
    std::string previewInfo;

    // Lịch sử các hình đã vẽ
    std::vector<DrawnShape> shapes;

    // UI Buttons
    std::vector<UIButton> toolButtons;
    std::vector<UIButton> actionButtons;
    std::vector<UIButton> colorButtons;

    // OpenGL Shaders & Meshes
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
    }

    bool init(int w = 1280, int h = 720, const char* title = "LAB 01 - Computer Graphics Rasterization (Bresenham & Midpoint)") {
        windowWidth = w;
        windowHeight = h;

        if (!glfwInit()) {
            std::cerr << "Loi khoi tao GLFW!\n";
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);

        window = glfwCreateWindow(windowWidth, windowHeight, title, nullptr, nullptr);
        if (!window) {
            std::cerr << "Loi tao cua so GLFW!\n";
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // VSync

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Loi khoi tao GLAD!\n";
            return false;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        // Khởi tạo Shader 2D cơ bản
        initShaders();

        // Khởi tạo Buffers
        glGenVertexArrays(1, &vaoBatch);
        glGenBuffers(1, &vboBatch);
        glBindVertexArray(vaoBatch);
        glBindBuffer(GL_ARRAY_BUFFER, vboBatch);
        glEnableVertexAttribArray(0); // Pos (vec2)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), (void*)0);
        glEnableVertexAttribArray(1); // Color (vec4)
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), (void*)(2 * sizeof(float)));
        glBindVertexArray(0);

        // Đặt góc nhìn ban đầu (tâm màn hình tại (0, 0))
        panX = windowWidth * 0.5f;
        panY = windowHeight * 0.5f;
        zoom = 1.0f;

        // Đăng ký Callback GLFW
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, cursorPosCallback);
        glfwSetScrollCallback(window, scrollCallback);
        glfwSetKeyCallback(window, keyCallback);

        // Khởi tạo Toolbar Buttons
        buildUIButtons();

        // Vẽ sẵn một số hình mẫu minh họa ban đầu cho sinh động
        initSampleShapes();

        std::cout << "\n===============================================================\n";
        std::cout << "  LAB 01: THUAT TOAN BRESENHAM & MIDPOINT VE DUONG CONG 2D    \n";
        std::cout << "===============================================================\n";
        std::cout << " - Dung chuot trai: Click & keo de ve cac hinh tren canvas\n";
        std::cout << " - Chuot phai / Giu SPACE: Keo de di chuyen canvas (Pan)\n";
        std::cout << " - Cuon chuot: Phong to / Thu nho (Zoom in / Zoom out xem pixel)\n";
        std::cout << " - Phim [1..9]: Chon nhanh cong cu ve\n";
        std::cout << " - Phim [Z]: Hoan tac (Undo), [C]: Xoa sach (Clear)\n";
        std::cout << " - Phim [G]: Bat/tat Luoi pixel, [X]: Bat/tat Truc Oxy\n";
        std::cout << " - Phim [R]: Dat lai goc nhin ban dau\n";
        std::cout << "===============================================================\n\n";

        return true;
    }

    void buildUIButtons() {
        toolButtons.clear();
        actionButtons.clear();
        colorButtons.clear();

        float btnH = 28.0f;
        float btnY = 10.0f;

        // 9 nút công cụ vẽ hàng trên
        std::vector<std::string> toolLabels = {
            "1. Line Bres", "2. Line Mid", "3. Cir Bres", "4. Cir Mid",
            "5. Ellipse", "6. Parabol", "7. Hyperbol", "8. Sin(x)", "9. Cos(x)"
        };
        float startX = 15.0f;
        for (size_t i = 0; i < toolLabels.size(); ++i) {
            float btnW = 95.0f;
            UIButton btn;
            btn.id = static_cast<int>(i);
            btn.label = toolLabels[i];
            btn.x = startX;
            btn.y = btnY;
            btn.w = btnW;
            btn.h = btnH;
            btn.bgColor = (static_cast<int>(i) == currentTool) ? Vec4(0.2f, 0.5f, 0.9f, 0.9f) : Vec4(0.18f, 0.20f, 0.25f, 0.85f);
            btn.textColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
            btn.isSelected = (static_cast<int>(i) == currentTool);
            toolButtons.push_back(btn);
            startX += btnW + 6.0f;
        }

        // Các nút thao tác (Clear, Undo, Grid, Axes, Reset View)
        std::vector<std::string> actionLabels = {"Undo (Z)", "Clear (C)", "Grid (G)", "Axes (X)", "Reset (R)"};
        startX += 15.0f;
        for (size_t i = 0; i < actionLabels.size(); ++i) {
            float btnW = 80.0f;
            UIButton btn;
            btn.id = 100 + static_cast<int>(i);
            btn.label = actionLabels[i];
            btn.x = startX;
            btn.y = btnY;
            btn.w = btnW;
            btn.h = btnH;
            btn.bgColor = Vec4(0.25f, 0.28f, 0.35f, 0.85f);
            btn.textColor = Vec4(0.9f, 0.9f, 0.9f, 1.0f);
            actionButtons.push_back(btn);
            startX += btnW + 6.0f;
        }

        // Bảng chọn 8 màu vẽ ở góc trên bên phải hoặc hàng 2
        float colX = windowWidth - 250.0f;
        float colY = 46.0f;
        float colSize = 22.0f;
        for (size_t i = 0; i < palette.size(); ++i) {
            UIButton btn;
            btn.id = 200 + static_cast<int>(i);
            btn.label = "";
            btn.x = colX + i * (colSize + 6.0f);
            btn.y = colY;
            btn.w = colSize;
            btn.h = colSize;
            btn.bgColor = palette[i];
            btn.textColor = Vec4(0, 0, 0, 1);
            btn.isSelected = (static_cast<int>(i) == currentColorIdx);
            colorButtons.push_back(btn);
        }
    }

    void initSampleShapes() {
        // Tạo sẵn các hình mẫu đại diện cho các bài tập trong Lab 1
        shapes.clear();

        // 1.1. Bốn trường hợp hệ số góc của đoạn thẳng
        // Case 1: 0 < m <= 1 (dx=120, dy=50 => m = 0.42)
        {
            DrawnShape s;
            s.tool = TOOL_LINE_BRESENHAM;
            s.color = Vec4(0.2f, 0.95f, 0.4f, 1.0f);
            s.p1 = PixelPoint(-280, -180);
            s.p2 = PixelPoint(-160, -130);
            s.pixels = drawLineBresenham(s.p1.x, s.p1.y, s.p2.x, s.p2.y, s.slopeCase, s.slope);
            s.info = "Line Bresenham | " + getLineSlopeCaseName(s.slopeCase);
            shapes.push_back(s);
        }
        // Case 2: m > 1 (dx=40, dy=110 => m = 2.75)
        {
            DrawnShape s;
            s.tool = TOOL_LINE_BRESENHAM;
            s.color = Vec4(0.95f, 0.4f, 0.4f, 1.0f);
            s.p1 = PixelPoint(-120, -180);
            s.p2 = PixelPoint(-80, -70);
            s.pixels = drawLineBresenham(s.p1.x, s.p1.y, s.p2.x, s.p2.y, s.slopeCase, s.slope);
            s.info = "Line Bresenham | " + getLineSlopeCaseName(s.slopeCase);
            shapes.push_back(s);
        }
        // Case 3: -1 <= m < 0 (dx=120, dy=-50 => m = -0.42)
        {
            DrawnShape s;
            s.tool = TOOL_LINE_MIDPOINT;
            s.color = Vec4(0.3f, 0.7f, 1.0f, 1.0f);
            s.p1 = PixelPoint(-280, -50);
            s.p2 = PixelPoint(-160, -100);
            s.pixels = drawLineMidpoint(s.p1.x, s.p1.y, s.p2.x, s.p2.y, s.slopeCase, s.slope);
            s.info = "Line Midpoint | " + getLineSlopeCaseName(s.slopeCase);
            shapes.push_back(s);
        }
        // Case 4: m < -1 (dx=40, dy=-110 => m = -2.75)
        {
            DrawnShape s;
            s.tool = TOOL_LINE_MIDPOINT;
            s.color = Vec4(0.95f, 0.85f, 0.2f, 1.0f);
            s.p1 = PixelPoint(-120, -70);
            s.p2 = PixelPoint(-80, -180);
            s.pixels = drawLineMidpoint(s.p1.x, s.p1.y, s.p2.x, s.p2.y, s.slopeCase, s.slope);
            s.info = "Line Midpoint | " + getLineSlopeCaseName(s.slopeCase);
            shapes.push_back(s);
        }

        // 1.2. Đường tròn Bresenham & Midpoint
        {
            DrawnShape s;
            s.tool = TOOL_CIRCLE_BRESENHAM;
            s.color = Vec4(0.2f, 0.95f, 0.95f, 1.0f);
            s.p1 = PixelPoint(60, -120);
            s.p2 = PixelPoint(130, -120);
            s.pixels = drawCircleBresenham(s.p1.x, s.p1.y, 70);
            s.info = "Circle Bresenham (R = 70, 8-way symmetry)";
            shapes.push_back(s);
        }
        {
            DrawnShape s;
            s.tool = TOOL_CIRCLE_MIDPOINT;
            s.color = Vec4(0.95f, 0.4f, 0.95f, 1.0f);
            s.p1 = PixelPoint(60, -120);
            s.p2 = PixelPoint(105, -120);
            s.pixels = drawCircleMidpoint(s.p1.x, s.p1.y, 45);
            s.info = "Circle Midpoint (R = 45, 8-way symmetry)";
            shapes.push_back(s);
        }

        // 1.3. Ellipse Midpoint
        {
            DrawnShape s;
            s.tool = TOOL_ELLIPSE_MIDPOINT;
            s.color = Vec4(0.98f, 0.6f, 0.15f, 1.0f);
            s.p1 = PixelPoint(260, -120);
            s.p2 = PixelPoint(350, -60);
            s.pixels = drawEllipseMidpoint(s.p1.x, s.p1.y, 90, 60);
            s.info = "Ellipse Midpoint (rx = 90, ry = 60, 2 regions)";
            shapes.push_back(s);
        }

        // 1.4. Parabol Midpoint
        {
            DrawnShape s;
            s.tool = TOOL_PARABOLA_MIDPOINT;
            s.color = Vec4(0.4f, 0.9f, 0.5f, 1.0f);
            s.p1 = PixelPoint(-200, 150);
            s.p2 = PixelPoint(-140, 100);
            s.pixels = drawParabolaMidpoint(s.p1.x, s.p1.y, 18.0f, 110, 140, -1);
            s.info = "Parabol Midpoint (p = 18.0, 2 regions, opening up)";
            shapes.push_back(s);
        }

        // 1.4. Hyperbol Midpoint
        {
            DrawnShape s;
            s.tool = TOOL_HYPERBOLA_MIDPOINT;
            s.color = Vec4(0.95f, 0.35f, 0.6f, 1.0f);
            s.p1 = PixelPoint(30, 140);
            s.p2 = PixelPoint(75, 175);
            s.pixels = drawHyperbolaMidpoint(s.p1.x, s.p1.y, 45, 35, 110, 110);
            s.info = "Hyperbol Midpoint (a = 45, b = 35, 4 branches)";
            shapes.push_back(s);
        }

        // 1.4. Sóng Sin & Cos
        {
            DrawnShape s;
            s.tool = TOOL_SIN_WAVE;
            s.color = Vec4(0.25f, 0.85f, 1.0f, 1.0f);
            s.p1 = PixelPoint(260, 140);
            s.p2 = PixelPoint(310, 95);
            s.pixels = drawSinWave(s.p1.x, s.p1.y, 45.0f, 120.0f, 130);
            s.info = "Sin Wave: y = A*sin(2pi*x/lambda) [A=45, lambda=120]";
            shapes.push_back(s);
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

    // Chuyển đổi tọa độ cửa sổ chuột sang tọa độ pixel thế giới trên canvas
    PixelPoint screenToWorld(double mouseX, double mouseY) {
        float worldX = (static_cast<float>(mouseX) - panX) / zoom;
        float worldY = (static_cast<float>(mouseY) - panY) / zoom;
        return PixelPoint(static_cast<int>(std::round(worldX)), static_cast<int>(std::round(worldY)));
    }

    // Cập nhật đường vẽ xem trước (Live Preview) khi người dùng đang giữ và kéo chuột
    void updateLivePreview() {
        if (!hasStartPoint) {
            previewPixels.clear();
            return;
        }

        previewPixels.clear();
        int x1 = startPoint.x;
        int y1 = startPoint.y;
        int x2 = currentPoint.x;
        int y2 = currentPoint.y;

        switch (currentTool) {
            case TOOL_LINE_BRESENHAM: {
                previewPixels = drawLineBresenham(x1, y1, x2, y2, previewSlopeCase, previewSlope);
                std::stringstream ss;
                ss << "Line Bresenham: P1(" << x1 << "," << y1 << ") -> P2(" << x2 << "," << y2 << ") | ";
                ss << "dx=" << (x2 - x1) << ", dy=" << (y2 - y1) << " | ";
                ss << std::fixed << std::setprecision(2) << "m=" << previewSlope << " | ";
                ss << getLineSlopeCaseName(previewSlopeCase) << " (" << previewPixels.size() << " px)";
                previewInfo = ss.str();
                break;
            }
            case TOOL_LINE_MIDPOINT: {
                previewPixels = drawLineMidpoint(x1, y1, x2, y2, previewSlopeCase, previewSlope);
                std::stringstream ss;
                ss << "Line Midpoint: P1(" << x1 << "," << y1 << ") -> P2(" << x2 << "," << y2 << ") | ";
                ss << "dx=" << (x2 - x1) << ", dy=" << (y2 - y1) << " | ";
                ss << std::fixed << std::setprecision(2) << "m=" << previewSlope << " | ";
                ss << getLineSlopeCaseName(previewSlopeCase) << " (" << previewPixels.size() << " px)";
                previewInfo = ss.str();
                break;
            }
            case TOOL_CIRCLE_BRESENHAM: {
                int r = static_cast<int>(std::round(std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1))));
                previewPixels = drawCircleBresenham(x1, y1, r);
                std::stringstream ss;
                ss << "Circle Bresenham: Tam C(" << x1 << "," << y1 << "), R = " << r << " (" << previewPixels.size() << " px)";
                previewInfo = ss.str();
                break;
            }
            case TOOL_CIRCLE_MIDPOINT: {
                int r = static_cast<int>(std::round(std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1))));
                previewPixels = drawCircleMidpoint(x1, y1, r);
                std::stringstream ss;
                ss << "Circle Midpoint: Tam C(" << x1 << "," << y1 << "), R = " << r << " (" << previewPixels.size() << " px)";
                previewInfo = ss.str();
                break;
            }
            case TOOL_ELLIPSE_MIDPOINT: {
                int rx = std::abs(x2 - x1);
                int ry = std::abs(y2 - y1);
                previewPixels = drawEllipseMidpoint(x1, y1, rx, ry);
                std::stringstream ss;
                ss << "Ellipse Midpoint: Tam C(" << x1 << "," << y1 << "), rx = " << rx << ", ry = " << ry << " (" << previewPixels.size() << " px)";
                previewInfo = ss.str();
                break;
            }
            case TOOL_PARABOLA_MIDPOINT: {
                float p = std::max(5.0f, static_cast<float>(std::abs(x2 - x1)) * 0.4f);
                int dir = (y2 >= y1) ? 1 : -1;
                int spanX = std::max(80, std::abs(x2 - x1) * 2);
                int spanY = std::max(80, std::abs(y2 - y1) * 2);
                previewPixels = drawParabolaMidpoint(x1, y1, p, spanX, spanY, dir);
                std::stringstream ss;
                ss << "Parabol Midpoint: Dinh (" << x1 << "," << y1 << "), p = " << std::fixed << std::setprecision(1) << p << " (" << previewPixels.size() << " px)";
                previewInfo = ss.str();
                break;
            }
            case TOOL_HYPERBOLA_MIDPOINT: {
                int a = std::max(15, std::abs(x2 - x1));
                int b = std::max(15, std::abs(y2 - y1));
                previewPixels = drawHyperbolaMidpoint(x1, y1, a, b, a + 120, b + 120);
                std::stringstream ss;
                ss << "Hyperbol Midpoint: Tam C(" << x1 << "," << y1 << "), a = " << a << ", b = " << b << " (" << previewPixels.size() << " px)";
                previewInfo = ss.str();
                break;
            }
            case TOOL_SIN_WAVE: {
                float amp = static_cast<float>(std::abs(y2 - y1));
                if (amp < 10.0f) amp = 40.0f;
                float wl = std::max(30.0f, static_cast<float>(std::abs(x2 - x1)) * 2.0f);
                previewPixels = drawSinWave(x1, y1, amp, wl, 200);
                std::stringstream ss;
                ss << "Sin Wave: Goc (" << x1 << "," << y1 << "), A = " << amp << ", lambda = " << wl << " (" << previewPixels.size() << " px)";
                previewInfo = ss.str();
                break;
            }
            case TOOL_COS_WAVE: {
                float amp = static_cast<float>(std::abs(y2 - y1));
                if (amp < 10.0f) amp = 40.0f;
                float wl = std::max(30.0f, static_cast<float>(std::abs(x2 - x1)) * 2.0f);
                previewPixels = drawCosWave(x1, y1, amp, wl, 200);
                std::stringstream ss;
                ss << "Cos Wave: Goc (" << x1 << "," << y1 << "), A = " << amp << ", lambda = " << wl << " (" << previewPixels.size() << " px)";
                previewInfo = ss.str();
                break;
            }
        }
    }

    // Chốt hình vẽ khi người dùng click lần 2 hoặc thả chuột
    void commitShape() {
        if (!hasStartPoint || previewPixels.empty()) {
            hasStartPoint = false;
            previewPixels.clear();
            return;
        }

        DrawnShape s;
        s.tool = currentTool;
        s.color = palette[currentColorIdx];
        s.p1 = startPoint;
        s.p2 = currentPoint;
        s.pixels = previewPixels;
        s.slopeCase = previewSlopeCase;
        s.slope = previewSlope;
        s.info = previewInfo;

        shapes.push_back(s);

        hasStartPoint = false;
        previewPixels.clear();
    }

    void undo() {
        if (!shapes.empty()) {
            shapes.pop_back();
        }
    }

    void clearCanvas() {
        shapes.clear();
        hasStartPoint = false;
        previewPixels.clear();
    }

    void resetView() {
        panX = windowWidth * 0.5f;
        panY = windowHeight * 0.5f;
        zoom = 1.0f;
    }

    // ========================================================================
    // CÁC HÀM VẼ GIAO DIỆN & PIXEL BATCHING
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
        if (len < 0.0001f) return;

        float nx = -dy / len * (thickness * 0.5f);
        float ny =  dx / len * (thickness * 0.5f);

        batchVertices.push_back({x1 + nx, y1 + ny, color.x, color.y, color.z, color.w});
        batchVertices.push_back({x2 + nx, y2 + ny, color.x, color.y, color.z, color.w});
        batchVertices.push_back({x2 - nx, y2 - ny, color.x, color.y, color.z, color.w});

        batchVertices.push_back({x1 + nx, y1 + ny, color.x, color.y, color.z, color.w});
        batchVertices.push_back({x2 - nx, y2 - ny, color.x, color.y, color.z, color.w});
        batchVertices.push_back({x1 - nx, y1 - ny, color.x, color.y, color.z, color.w});
    }

    // Vẽ một pixel hình vuông kích thước pixel thế giới
    void addPixel(int wx, int wy, const Vec4& color) {
        float sx = panX + wx * zoom;
        float sy = panY + wy * zoom;
        float pSize = std::max(1.0f, zoom);
        // Khi zoom lớn, tạo khoảng hở 1 pixel viền để nhìn rõ rãnh pixel
        float gap = (zoom >= 8.0f) ? 1.0f : 0.0f;
        addQuad(sx, sy, pSize - gap, pSize - gap, color);
    }

    // Bitmap Font 5x7 tích hợp sẵn không cần tải file font ngoài
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
    // VÒNG LẶP RENDER CHÍNH (RENDER LOOP)
    // ========================================================================
    void render() {
        // Nền tối dễ quan sát pixel đồ họa
        glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        batchVertices.clear();

        // 1. Vẽ Lưới Pixel (Pixel Grid) khi phóng to hoặc bật Grid
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

            // Chỉ vẽ lưới khi số đường không quá dày
            if (maxX - minX < 400) {
                for (int x = minX; x <= maxX; ++x) {
                    float sx = panX + x * zoom;
                    addLine(sx, 0, sx, windowHeight, 1.0f, gridColor);
                }
                for (int y = minY; y <= maxY; ++y) {
                    float sy = panY + y * zoom;
                    addLine(0, sy, windowWidth, sy, 1.0f, gridColor);
                }
            }
        }

        // 2. Vẽ Trục Tọa độ Oxy thế giới
        if (showAxes) {
            Vec4 axisColorX(0.9f, 0.3f, 0.3f, 0.75f); // Trục X đỏ
            Vec4 axisColorY(0.3f, 0.85f, 0.4f, 0.75f); // Trục Y xanh lá
            addLine(0, panY, windowWidth, panY, 2.0f, axisColorX);
            addLine(panX, 0, panX, windowHeight, 2.0f, axisColorY);

            // Đánh dấu gốc O (0, 0)
            addQuad(panX - 4.0f, panY - 4.0f, 8.0f, 8.0f, Vec4(1.0f, 1.0f, 0.2f, 1.0f));
            drawText("O(0,0)", panX + 8.0f, panY + 8.0f, 10.0f, Vec4(1.0f, 1.0f, 0.3f, 0.9f));
        }

        // 3. Vẽ tất cả các hình trong lịch sử (Shapes History)
        for (const auto& shape : shapes) {
            for (const auto& pt : shape.pixels) {
                addPixel(pt.x, pt.y, shape.color);
            }
        }

        // 4. Vẽ đường xem trước (Live Preview / Ghost Shape) với màu phát sáng
        if (hasStartPoint && !previewPixels.empty()) {
            Vec4 previewCol = palette[currentColorIdx];
            previewCol.w = 0.65f; // Bán trong suốt cho live preview
            for (const auto& pt : previewPixels) {
                addPixel(pt.x, pt.y, previewCol);
            }

            // Điểm xuất phát (Start Anchor Marker)
            float sAnchorX = panX + startPoint.x * zoom;
            float sAnchorY = panY + startPoint.y * zoom;
            addQuad(sAnchorX - 4, sAnchorY - 4, 8, 8, Vec4(1.0f, 0.2f, 0.2f, 1.0f));

            // Điểm hiện tại (Current Anchor Marker)
            float cAnchorX = panX + currentPoint.x * zoom;
            float cAnchorY = panY + currentPoint.y * zoom;
            addQuad(cAnchorX - 4, cAnchorY - 4, 8, 8, Vec4(0.2f, 0.8f, 1.0f, 1.0f));
        }

        // 5. Vẽ Giao diện người dùng (UI Toolbar & HUD)
        // Thanh nền Topbar
        addQuad(0, 0, windowWidth, 80.0f, Vec4(0.11f, 0.12f, 0.16f, 0.95f));
        addLine(0, 80.0f, windowWidth, 80.0f, 1.5f, Vec4(0.25f, 0.28f, 0.36f, 1.0f));

        // Nút công cụ vẽ
        for (auto& btn : toolButtons) {
            btn.isSelected = (btn.id == static_cast<int>(currentTool));
            Vec4 bg = btn.isSelected ? Vec4(0.20f, 0.55f, 0.95f, 1.0f) : Vec4(0.18f, 0.21f, 0.27f, 0.9f);
            addQuad(btn.x, btn.y, btn.w, btn.h, bg);
            if (btn.isSelected) {
                // Viền sáng cho nút đang chọn
                addLine(btn.x, btn.y, btn.x + btn.w, btn.y, 2.0f, Vec4(0.6f, 0.85f, 1.0f, 1.0f));
            }
            drawText(btn.label, btn.x + 6.0f, btn.y + 8.0f, 10.0f, Vec4(1, 1, 1, 1));
        }

        // Nút thao tác
        for (const auto& btn : actionButtons) {
            addQuad(btn.x, btn.y, btn.w, btn.h, btn.bgColor);
            drawText(btn.label, btn.x + 6.0f, btn.y + 8.0f, 10.0f, btn.textColor);
        }

        // Bảng chọn màu
        drawText("COLOR:", windowWidth - 305.0f, 52.0f, 10.0f, Vec4(0.8f, 0.8f, 0.8f, 1.0f));
        for (size_t i = 0; i < colorButtons.size(); ++i) {
            const auto& btn = colorButtons[i];
            addQuad(btn.x, btn.y, btn.w, btn.h, btn.bgColor);
            if (static_cast<int>(i) == currentColorIdx) {
                // Khung viền trắng quanh màu được chọn
                addLine(btn.x - 2, btn.y - 2, btn.x + btn.w + 2, btn.y - 2, 2.0f, Vec4(1, 1, 1, 1));
                addLine(btn.x + btn.w + 2, btn.y - 2, btn.x + btn.w + 2, btn.y + btn.h + 2, 2.0f, Vec4(1, 1, 1, 1));
                addLine(btn.x + btn.w + 2, btn.y + btn.h + 2, btn.x - 2, btn.y + btn.h + 2, 2.0f, Vec4(1, 1, 1, 1));
                addLine(btn.x - 2, btn.y + btn.h + 2, btn.x - 2, btn.y - 2, 2.0f, Vec4(1, 1, 1, 1));
            }
        }

        // Thanh trạng thái & Hướng dẫn (Status Banner) ở hàng 2 của Topbar
        std::string modeTitle = "DANG CHON: " + getToolName(currentTool);
        drawText(modeTitle, 18.0f, 52.0f, 11.0f, Vec4(0.35f, 0.95f, 0.65f, 1.0f));

        // Thanh HUD thông tin toán học phía dưới màn hình (Bottom Info Panel)
        addQuad(0, windowHeight - 42.0f, windowWidth, 42.0f, Vec4(0.10f, 0.11f, 0.14f, 0.95f));
        addLine(0, windowHeight - 42.0f, windowWidth, windowHeight - 42.0f, 1.5f, Vec4(0.22f, 0.25f, 0.32f, 1.0f));

        std::string bottomText = previewInfo.empty() ?
            "HUONG DAN: Nhan giu chuot trai va keo de xac dinh toa do hinh ve | Cuon chuot: Zoom (" + std::to_string(static_cast<int>(zoom * 100)) + "%)" :
            "THONG SO TOAN HOC: " + previewInfo;

        drawText(bottomText, 20.0f, windowHeight - 27.0f, 11.0f, Vec4(1.0f, 0.95f, 0.4f, 1.0f));

        // Tọa độ chuột ở góc phải dưới
        std::stringstream mousePosSS;
        mousePosSS << "Mouse: (" << currentPoint.x << ", " << currentPoint.y << ") | Shapes: " << shapes.size();
        drawText(mousePosSS.str(), windowWidth - 280.0f, windowHeight - 27.0f, 10.0f, Vec4(0.7f, 0.8f, 0.9f, 1.0f));

        // Upload dữ liệu Vertex Batch lên GPU và Render
        if (!batchVertices.empty()) {
            glUseProgram(shaderProgram);

            // Ma trận chiếu trực giao từ (0, 0) góc trên bên trái đến (width, height) góc dưới bên phải
            Mat4 orthoProj = Mat4::ortho(0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight), 0.0f, -1.0f, 1.0f);
            GLint loc = glGetUniformLocation(shaderProgram, "uProjection");
            if (loc != -1) {
                glUniformMatrix4fv(loc, 1, GL_FALSE, orthoProj.data());
            }

            glBindVertexArray(vaoBatch);
            glBindBuffer(GL_ARRAY_BUFFER, vboBatch);
            glBufferData(GL_ARRAY_BUFFER, batchVertices.size() * sizeof(BatchVertex), batchVertices.data(), GL_STREAM_DRAW);
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(batchVertices.size()));
            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    void run() {
        while (!glfwWindowShouldClose(window)) {
            render();
        }
    }

    // Chạy kiểm thử tự động
    bool runAutoTest(int numFrames = 30) {
        std::cout << "[Lab1App] Khoi dong kiem thu tu dong...\n";
        bool algoSuccess = runAlgorithmTests();

        // Chạy một số frame render để đảm bảo OpenGL không bị crash
        for (int i = 0; i < numFrames && !glfwWindowShouldClose(window); ++i) {
            render();
        }

        std::cout << "[Lab1App] Hoan thanh " << numFrames << " frame render kiem thu.\n";
        return algoSuccess;
    }

    // ========================================================================
    // XỬ LÝ SỰ KIỆN CHUỘT VÀ BÀN PHÍM
    // ========================================================================
    static void mouseButtonCallback(GLFWwindow* w, int button, int action, int mods) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (!app) return;

        double mx, my;
        glfwGetCursorPos(w, &mx, &my);

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                // 1. Kiểm tra xem có click trúng nút Toolbar không
                if (my <= 80.0) {
                    for (const auto& btn : app->toolButtons) {
                        if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
                            app->currentTool = static_cast<ToolType>(btn.id);
                            app->hasStartPoint = false;
                            app->previewPixels.clear();
                            return;
                        }
                    }
                    for (const auto& btn : app->actionButtons) {
                        if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
                            if (btn.id == 100) app->undo();
                            else if (btn.id == 101) app->clearCanvas();
                            else if (btn.id == 102) app->showGrid = !app->showGrid;
                            else if (btn.id == 103) app->showAxes = !app->showAxes;
                            else if (btn.id == 104) app->resetView();
                            return;
                        }
                    }
                    for (size_t i = 0; i < app->colorButtons.size(); ++i) {
                        const auto& btn = app->colorButtons[i];
                        if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
                            app->currentColorIdx = static_cast<int>(i);
                            return;
                        }
                    }
                    return;
                }

                // 2. Click trên vùng Canvas vẽ hình
                app->isMouseDown = true;
                app->startPoint = app->screenToWorld(mx, my);
                app->currentPoint = app->startPoint;
                app->hasStartPoint = true;
                app->updateLivePreview();
            } else if (action == GLFW_RELEASE) {
                if (app->isMouseDown && app->hasStartPoint) {
                    app->currentPoint = app->screenToWorld(mx, my);
                    app->commitShape();
                }
                app->isMouseDown = false;
            }
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                app->isPanning = true;
                app->lastMouseX = mx;
                app->lastMouseY = my;
            } else if (action == GLFW_RELEASE) {
                app->isPanning = false;
            }
        }
    }

    static void cursorPosCallback(GLFWwindow* w, double xpos, double ypos) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (!app) return;

        if (app->isPanning) {
            float dx = static_cast<float>(xpos - app->lastMouseX);
            float dy = static_cast<float>(ypos - app->lastMouseY);
            app->panX += dx;
            app->panY += dy;
            app->lastMouseX = xpos;
            app->lastMouseY = ypos;
        }

        app->currentPoint = app->screenToWorld(xpos, ypos);

        if (app->isMouseDown && app->hasStartPoint) {
            app->updateLivePreview();
        }
    }

    static void scrollCallback(GLFWwindow* w, double xoffset, double yoffset) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (!app) return;

        double mx, my;
        glfwGetCursorPos(w, &mx, &my);

        float oldZoom = app->zoom;
        float factor = (yoffset > 0) ? 1.18f : (1.0f / 1.18f);
        app->zoom *= factor;

        // Giới hạn tỷ lệ phóng to từ 0.1x đến 64.0x
        if (app->zoom < 0.15f) app->zoom = 0.15f;
        if (app->zoom > 64.0f) app->zoom = 64.0f;

        // Phóng to tập trung tại vị trí trỏ chuột (Zoom toward mouse)
        app->panX = static_cast<float>(mx) - (static_cast<float>(mx) - app->panX) * (app->zoom / oldZoom);
        app->panY = static_cast<float>(my) - (static_cast<float>(my) - app->panY) * (app->zoom / oldZoom);
    }

    static void keyCallback(GLFWwindow* w, int key, int scancode, int action, int mods) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (!app) return;

        if (action == GLFW_PRESS) {
            if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
                int toolIdx = key - GLFW_KEY_1;
                app->currentTool = static_cast<ToolType>(toolIdx);
                app->hasStartPoint = false;
                app->previewPixels.clear();
                std::cout << "[Tool] Chuyen sang cong cu: " << getToolName(app->currentTool) << "\n";
            } else if (key == GLFW_KEY_Z || key == GLFW_KEY_U) {
                app->undo();
            } else if (key == GLFW_KEY_C || key == GLFW_KEY_DELETE || key == GLFW_KEY_BACKSPACE) {
                app->clearCanvas();
            } else if (key == GLFW_KEY_G) {
                app->showGrid = !app->showGrid;
            } else if (key == GLFW_KEY_X) {
                app->showAxes = !app->showAxes;
            } else if (key == GLFW_KEY_R) {
                app->resetView();
            } else if (key == GLFW_KEY_TAB) {
                app->currentColorIdx = (app->currentColorIdx + 1) % app->palette.size();
            } else if (key == GLFW_KEY_ESCAPE) {
                glfwSetWindowShouldClose(w, GLFW_TRUE);
            }
        }
    }

    static void framebufferSizeCallback(GLFWwindow* w, int width, int height) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (app) {
            app->windowWidth = width;
            app->windowHeight = height;
            glViewport(0, 0, width, height);
            app->buildUIButtons();
        }
    }

    void destroy() {
        if (vaoBatch) { glDeleteVertexArrays(1, &vaoBatch); vaoBatch = 0; }
        if (vboBatch) { glDeleteBuffers(1, &vboBatch); vboBatch = 0; }
        if (shaderProgram) { glDeleteProgram(shaderProgram); shaderProgram = 0; }
        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }

    ~Application() {
        destroy();
    }
};

inline Application* Application::instance = nullptr;

} // namespace Lab1

#endif // LAB1_APP_HPP
