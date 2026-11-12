#ifndef LAB3_APP_HPP
#define LAB3_APP_HPP

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
#include "Lab3Transformations.hpp"

namespace Lab3 {

enum TransformMode {
    TRANS_TRANSLATE = 0,    // 1. Tịnh tiến
    TRANS_SCALE = 1,        // 2. Tỉ lệ với tâm tỉ lệ (x0, y0)
    TRANS_ROTATE = 2,       // 3. Quay với tâm quay (x0, y0)
    TRANS_REFLECT = 3       // 4. Đối xứng qua đường thẳng y = ax + b
};

enum SampleShapeType {
    SHAPE_LETTER_F = 0,
    SHAPE_SPACESHIP = 1,
    SHAPE_STAR = 2,
    SHAPE_TRIANGLE = 3
};

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

    // Camera & Pan / Zoom
    float zoom = 1.0f;
    float panX = 0.0f;
    float panY = 0.0f;
    bool showGrid = true;
    bool showAxes = true;

    // Chế độ & Hình học
    TransformMode currentMode = TRANS_ROTATE;
    SampleShapeType currentShapeType = SHAPE_SPACESHIP;
    std::vector<Point2D> originalShape;
    std::vector<Point2D> transformedShape;

    // Tham số biến đổi
    // 1. Tịnh tiến
    float tx = 80.0f;
    float ty = 40.0f;

    // 2. Tỉ lệ
    float sx = 1.5f;
    float sy = 1.5f;
    Point2D scalePivot = Point2D(0.0f, 0.0f);

    // 3. Quay
    float rotateAngle = 45.0f;
    Point2D rotatePivot = Point2D(0.0f, 0.0f);
    bool isAutoRotating = false;

    // 4. Đối xứng qua y = ax + b
    float lineA = 1.0f;      // Hệ số góc a (đường y = x ban đầu)
    float lineB = 0.0f;      // Tung độ gốc b
    bool isVerticalLine = false; // Đường thẳng đứng x = c
    float verticalC = 0.0f;
    Point2D lineP1 = Point2D(-200.0f, -200.0f);
    Point2D lineP2 = Point2D(200.0f, 200.0f);

    // Ma trận biến đổi tổng hợp 3x3 hiện tại
    Matrix3x3 currentMatrix;

    // Tương tác chuột
    bool isMouseDown = false;
    bool isPanning = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    Point2D mouseWorld;
    bool settingLineSecondPoint = false;

    // UI Buttons
    std::vector<UIButton> modeButtons;
    std::vector<UIButton> shapeButtons;
    std::vector<UIButton> actionButtons;

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
    }

    bool init(int w = 1280, int h = 720, const char* title = "LAB 03 - 2D Geometric Transformations (Translation, Scale, Rotate, Reflect)") {
        windowWidth = w;
        windowHeight = h;

        if (!glfwInit()) {
            std::cerr << "Loi khoi tao GLFW Lab 3!\n";
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);

        window = glfwCreateWindow(windowWidth, windowHeight, title, nullptr, nullptr);
        if (!window) {
            std::cerr << "Loi tao cua so GLFW Lab 3!\n";
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Loi khoi tao GLAD Lab 3!\n";
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
        setShape(SHAPE_SPACESHIP);
        updateTransform();

        std::cout << "\n===============================================================\n";
        std::cout << "  LAB 03: CAC PHEP BIEN DOI 2D (2D AFFINE TRANSFORMATIONS)     \n";
        std::cout << "===============================================================\n";
        std::cout << " - Phim [1]: Tinh tien (Translation) - Dung phim mui ten hoac keo chuot\n";
        std::cout << " - Phim [2]: Ti le voi tam (x0, y0) - Click chuot dat tam ti le\n";
        std::cout << " - Phim [3]: Quay voi tam (x0, y0) - Click dat tam quay, [SPACE] quay tu dong\n";
        std::cout << " - Phim [4]: Doi xung qua y = ax + b - Click 2 diem de dat duong thang\n";
        std::cout << " - Phim [F]: Chon hinh mau (Chu F, Phi thuyen, Ngoi sao, Tam giac)\n";
        std::cout << " - Phim [R]: Reset thong so ve ban dau\n";
        std::cout << "===============================================================\n\n";

        return true;
    }

    void setShape(SampleShapeType type) {
        currentShapeType = type;
        if (type == SHAPE_LETTER_F) originalShape = createLetterFShape();
        else if (type == SHAPE_SPACESHIP) originalShape = createSpaceshipShape();
        else if (type == SHAPE_STAR) originalShape = createStarShape();
        else if (type == SHAPE_TRIANGLE) originalShape = createTriangleShape();
        updateTransform();
    }

    void updateTransform() {
        if (currentMode == TRANS_TRANSLATE) {
            currentMatrix = Matrix3x3::makeTranslation(tx, ty);
        } else if (currentMode == TRANS_SCALE) {
            currentMatrix = Matrix3x3::makeScaling(sx, sy, scalePivot.x, scalePivot.y);
        } else if (currentMode == TRANS_ROTATE) {
            currentMatrix = Matrix3x3::makeRotation(rotateAngle, rotatePivot.x, rotatePivot.y);
        } else if (currentMode == TRANS_REFLECT) {
            if (isVerticalLine) {
                currentMatrix = Matrix3x3::makeReflectionVertical(verticalC);
            } else {
                currentMatrix = Matrix3x3::makeReflectionLine(lineA, lineB);
            }
        }
        transformedShape = currentMatrix.transformPolygon(originalShape);
    }

    void buildUIButtons() {
        modeButtons.clear();
        shapeButtons.clear();
        actionButtons.clear();

        float btnH = 26.0f;
        float btnY1 = 8.0f;
        float btnY2 = 38.0f;

        // Hàng 1: 4 Phép biến đổi
        std::vector<std::string> modeLabels = {
            "1. Tinh tien", "2. Ti le (x0,y0)", "3. Quay (x0,y0)", "4. Doi xung y=ax+b"
        };
        float curX = 12.0f;
        for (size_t i = 0; i < modeLabels.size(); ++i) {
            float w = 135.0f;
            UIButton btn;
            btn.id = static_cast<int>(i);
            btn.label = modeLabels[i];
            btn.x = curX;
            btn.y = btnY1;
            btn.w = w;
            btn.h = btnH;
            btn.isSelected = (btn.id == currentMode);
            btn.bgColor = btn.isSelected ? Vec4(0.2f, 0.55f, 0.95f, 0.9f) : Vec4(0.18f, 0.20f, 0.25f, 0.85f);
            btn.textColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
            modeButtons.push_back(btn);
            curX += w + 6.0f;
        }

        // Hàng 1 (phía phải): Nút chức năng
        std::vector<std::string> actLabels = {"Auto-Rot (SPACE)", "Reset (R)", "Grid (G)", "Axes (X)", "Reset View"};
        curX += 16.0f;
        for (size_t i = 0; i < actLabels.size(); ++i) {
            float w = 100.0f;
            UIButton btn;
            btn.id = 100 + static_cast<int>(i);
            btn.label = actLabels[i];
            btn.x = curX;
            btn.y = btnY1;
            btn.w = w;
            btn.h = btnH;
            btn.isSelected = (i == 0 && isAutoRotating);
            btn.bgColor = btn.isSelected ? Vec4(0.9f, 0.5f, 0.2f, 0.9f) : Vec4(0.25f, 0.28f, 0.35f, 0.85f);
            btn.textColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
            actionButtons.push_back(btn);
            curX += w + 6.0f;
        }

        // Hàng 2: Chọn hình mẫu
        std::vector<std::string> shapeLabels = {
            "Hinh: Phi thuyen", "Hinh: Chu F (Classic)", "Hinh: Ngoi sao", "Hinh: Tam giac"
        };
        curX = 12.0f;
        for (size_t i = 0; i < shapeLabels.size(); ++i) {
            float w = 135.0f;
            UIButton btn;
            btn.id = 200 + static_cast<int>(i);
            btn.label = shapeLabels[i];
            btn.x = curX;
            btn.y = btnY2;
            btn.w = w;
            btn.h = btnH;
            int shapeIdx = (i == 0) ? SHAPE_SPACESHIP : (i == 1 ? SHAPE_LETTER_F : (i == 2 ? SHAPE_STAR : SHAPE_TRIANGLE));
            btn.isSelected = (shapeIdx == currentShapeType);
            btn.bgColor = btn.isSelected ? Vec4(0.15f, 0.60f, 0.45f, 0.9f) : Vec4(0.18f, 0.25f, 0.28f, 0.85f);
            btn.textColor = Vec4(0.9f, 1.0f, 0.95f, 1.0f);
            shapeButtons.push_back(btn);
            curX += w + 6.0f;
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
    // BATCH VERTEX & PRIMITIVES
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

    void addDashedWorldLine(float wx1, float wy1, float wx2, float wy2, float thickness, const Vec4& color, float dashLen = 8.0f) {
        float dx = wx2 - wx1;
        float dy = wy2 - wy1;
        float totalLen = std::hypot(dx, dy);
        if (totalLen < 1e-4f) return;

        float uX = dx / totalLen;
        float uY = dy / totalLen;
        float cur = 0.0f;
        bool draw = true;

        while (cur < totalLen) {
            float next = std::min(totalLen, cur + dashLen);
            if (draw) {
                addWorldLine(wx1 + uX * cur, wy1 + uY * cur, wx1 + uX * next, wy1 + uY * next, thickness, color);
            }
            cur = next;
            draw = !draw;
        }
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
        if (isAutoRotating && currentMode == TRANS_ROTATE) {
            rotateAngle += 1.0f;
            if (rotateAngle >= 360.0f) rotateAngle -= 360.0f;
            updateTransform();
        }

        glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        batchVertices.clear();

        // 1. Grid & Axes
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

        // 2. Vẽ HÌNH GỐC (Original Shape - Nét đứt màu xám lam mờ)
        if (!originalShape.empty()) {
            Vec4 origCol(0.45f, 0.55f, 0.70f, 0.65f);
            size_t n = originalShape.size();
            for (size_t i = 0; i < n; ++i) {
                size_t next = (i + 1) % n;
                addDashedWorldLine(originalShape[i].x, originalShape[i].y,
                                   originalShape[next].x, originalShape[next].y, 1.5f, origCol);
            }
        }

        // 3. Vẽ HÌNH BIẾN ĐỔI (Transformed Shape - Màu sáng nổi bật)
        if (!transformedShape.empty()) {
            Vec4 transCol(0.20f, 0.95f, 0.40f, 1.0f); // Xanh lá sáng
            if (currentMode == TRANS_SCALE) transCol = Vec4(0.25f, 0.75f, 1.0f, 1.0f);
            else if (currentMode == TRANS_ROTATE) transCol = Vec4(1.0f, 0.85f, 0.2f, 1.0f);
            else if (currentMode == TRANS_REFLECT) transCol = Vec4(0.95f, 0.35f, 0.85f, 1.0f);

            size_t n = transformedShape.size();
            for (size_t i = 0; i < n; ++i) {
                size_t next = (i + 1) % n;
                addWorldLine(transformedShape[i].x, transformedShape[i].y,
                             transformedShape[next].x, transformedShape[next].y, 2.5f, transCol);

                // Điểm đỉnh hình biến đổi
                float sx = panX + transformedShape[i].x * zoom;
                float sy = panY + transformedShape[i].y * zoom;
                addQuad(sx - 3, sy - 3, 6, 6, Vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }

            // Nối các đường hỗ trợ quan sát trực quan biến đổi:
            if (currentMode == TRANS_REFLECT) {
                // Nối các đỉnh gốc với đỉnh đối xứng qua đường thẳng
                for (size_t i = 0; i < n; i += (n > 6 ? 2 : 1)) {
                    addDashedWorldLine(originalShape[i].x, originalShape[i].y,
                                       transformedShape[i].x, transformedShape[i].y, 1.0f, Vec4(0.9f, 0.4f, 0.4f, 0.4f));
                }
            } else if (currentMode == TRANS_SCALE) {
                // Nối tia từ tâm tỉ lệ qua đỉnh
                for (size_t i = 0; i < n; i += (n > 6 ? 2 : 1)) {
                    addDashedWorldLine(scalePivot.x, scalePivot.y,
                                       transformedShape[i].x, transformedShape[i].y, 1.0f, Vec4(0.3f, 0.6f, 0.9f, 0.3f));
                }
            }
        }

        // 4. Vẽ CÁC YẾU TỐ HÌNH HỌC ĐẶC BIỆT THEO CHẾ ĐỘ
        if (currentMode == TRANS_SCALE || currentMode == TRANS_ROTATE) {
            // Vẽ TÂM BIẾN ĐỔI (x0, y0)
            Point2D pivot = (currentMode == TRANS_SCALE) ? scalePivot : rotatePivot;
            float sx = panX + pivot.x * zoom;
            float sy = panY + pivot.y * zoom;

            // Dấu chữ thập và vòng tròn tâm
            addLine(sx - 12, sy, sx + 12, sy, 2.0f, Vec4(1.0f, 0.2f, 0.2f, 1.0f));
            addLine(sx, sy - 12, sx, sy + 12, 2.0f, Vec4(1.0f, 0.2f, 0.2f, 1.0f));
            addQuad(sx - 4, sy - 4, 8, 8, Vec4(1.0f, 0.9f, 0.2f, 1.0f));

            std::stringstream ssP;
            ssP << "TAM (" << (int)pivot.x << "," << (int)pivot.y << ")";
            drawText(ssP.str(), sx + 14, sy - 8, 9.0f, Vec4(1.0f, 0.9f, 0.2f, 1.0f));
        } else if (currentMode == TRANS_REFLECT) {
            // Vẽ ĐƯỜNG THẲNG ĐỐI XỨNG y = ax + b
            Vec4 lineCol(0.98f, 0.60f, 0.15f, 0.95f); // Màu cam nổi bật

            if (isVerticalLine) {
                addWorldLine(verticalC, -1000.0f, verticalC, 1000.0f, 2.5f, lineCol);
                std::stringstream ssEq;
                ssEq << "Duong doi xung: x = " << (int)verticalC;
                float sx = panX + verticalC * zoom;
                drawText(ssEq.str(), sx + 8, 90.0f, 10.0f, lineCol);
            } else {
                // Tính 2 điểm xa ở mép màn hình
                float xL = -2000.0f;
                float yL = lineA * xL + lineB;
                float xR = 2000.0f;
                float yR = lineA * xR + lineB;
                addWorldLine(xL, yL, xR, yR, 2.5f, lineCol);

                std::stringstream ssEq;
                ssEq << "Duong doi xung: y = " << std::fixed << std::setprecision(2) << lineA << "*x "
                     << (lineB >= 0 ? "+ " : "- ") << std::abs(lineB);
                float midY = lineA * 0.0f + lineB;
                float sx = panX + 15.0f;
                float sy = panY + midY * zoom;
                drawText(ssEq.str(), sx, std::clamp(sy, 85.0f, (float)windowHeight - 110.0f), 10.0f, lineCol);
            }
        } else if (currentMode == TRANS_TRANSLATE) {
            // Vẽ VECTOR TỊNH TIẾN (Mũi tên chỉ hướng)
            addWorldLine(0.0f, 0.0f, tx, ty, 2.5f, Vec4(0.2f, 0.9f, 0.4f, 0.9f));
            float endSx = panX + tx * zoom;
            float endSy = panY + ty * zoom;
            addQuad(endSx - 4, endSy - 4, 8, 8, Vec4(0.2f, 0.9f, 0.4f, 1.0f));
            std::stringstream ssVec;
            ssVec << "v = (" << (int)tx << ", " << (int)ty << ")";
            drawText(ssVec.str(), endSx + 10, endSy - 6, 9.0f, Vec4(0.2f, 0.9f, 0.4f, 1.0f));
        }

        // 5. Thanh công cụ Toolbar UI
        addQuad(0, 0, windowWidth, 72.0f, Vec4(0.10f, 0.12f, 0.16f, 0.95f));
        addLine(0, 72.0f, windowWidth, 72.0f, 1.5f, Vec4(0.25f, 0.28f, 0.35f, 0.8f));

        for (const auto& btn : modeButtons) {
            addQuad(btn.x, btn.y, btn.w, btn.h, btn.bgColor);
            if (btn.isSelected) {
                addLine(btn.x, btn.y + btn.h - 2, btn.x + btn.w, btn.y + btn.h - 2, 2.0f, Vec4(1.0f, 0.9f, 0.3f, 1.0f));
            }
            drawText(btn.label, btn.x + 8.0f, btn.y + 7.0f, 9.0f, btn.textColor);
        }

        for (const auto& btn : shapeButtons) {
            addQuad(btn.x, btn.y, btn.w, btn.h, btn.bgColor);
            drawText(btn.label, btn.x + 8.0f, btn.y + 7.0f, 9.0f, btn.textColor);
        }

        for (const auto& btn : actionButtons) {
            addQuad(btn.x, btn.y, btn.w, btn.h, btn.bgColor);
            drawText(btn.label, btn.x + 8.0f, btn.y + 7.0f, 9.0f, btn.textColor);
        }

        // 6. BẢNG HUD HIỂN THỊ THÔNG TIN VÀ MA TRẬN 3X3 (Góc dưới màn hình)
        addQuad(12.0f, windowHeight - 115.0f, windowWidth - 24.0f, 105.0f, Vec4(0.10f, 0.12f, 0.16f, 0.94f));
        addLine(12.0f, windowHeight - 115.0f, windowWidth - 12.0f, windowHeight - 115.0f, 1.0f, Vec4(0.25f, 0.3f, 0.4f, 0.8f));

        // Cột trái: Tên phép biến đổi & Giá trị tham số
        std::string modeTitle = "PHEP BIEN DOI 2D: ";
        if (currentMode == TRANS_TRANSLATE) modeTitle += "TINH TIEN (TRANSLATION)";
        else if (currentMode == TRANS_SCALE) modeTitle += "TI LE VOI TAM (SCALING AROUND PIVOT)";
        else if (currentMode == TRANS_ROTATE) modeTitle += "QUAY VOI TAM (ROTATION AROUND PIVOT)";
        else if (currentMode == TRANS_REFLECT) modeTitle += "DOI XUNG QUA DUONG THANG y = ax + b";

        drawText(modeTitle, 24.0f, windowHeight - 105.0f, 11.0f, Vec4(1.0f, 0.9f, 0.3f, 1.0f));

        std::stringstream ssParam;
        if (currentMode == TRANS_TRANSLATE) {
            ssParam << "Tham so: Vector tinh tien (tx=" << (int)tx << ", ty=" << (int)ty
                    << ") | Phim mui ten: Thay doi tx, ty | Keo chuot de doi vector";
        } else if (currentMode == TRANS_SCALE) {
            ssParam << "Tham so: Tam ti le P0=(" << (int)scalePivot.x << "," << (int)scalePivot.y
                    << "), He so (sx=" << std::fixed << std::setprecision(2) << sx << ", sy=" << sy
                    << ") | Click chuot dat tam P0 | Phim Up/Down: doi ti le";
        } else if (currentMode == TRANS_ROTATE) {
            ssParam << "Tham so: Tam quay P0=(" << (int)rotatePivot.x << "," << (int)rotatePivot.y
                    << "), Goc quay = " << std::fixed << std::setprecision(1) << rotateAngle << " do"
                    << " | Click chuot dat tam P0 | Phim Left/Right: doi goc quay | [SPACE]: Auto-Rot";
        } else if (currentMode == TRANS_REFLECT) {
            if (isVerticalLine) {
                ssParam << "Duong doi xung: x = " << (int)verticalC << " | Click 2 diem dat duong thang bat ky";
            } else {
                ssParam << "Duong doi xung: y = " << std::fixed << std::setprecision(2) << lineA << "*x "
                        << (lineB >= 0 ? "+ " : "- ") << std::abs(lineB)
                        << " (a=" << lineA << ", b=" << lineB << ") | Click 2 diem dat duong thang";
            }
        }
        drawText(ssParam.str(), 24.0f, windowHeight - 85.0f, 9.0f, Vec4(0.85f, 0.9f, 1.0f, 0.9f));

        // Cột phải: HIỂN THỊ MA TRẬN 3X3 THEO THỜI GIAN THỰC
        float matBoxX = windowWidth - 480.0f;
        drawText("MA TRAN THUAN NHAT 3x3 [M]:", matBoxX, windowHeight - 105.0f, 10.0f, Vec4(0.4f, 0.9f, 0.95f, 1.0f));

        for (int r = 0; r < 3; ++r) {
            std::stringstream ssR;
            ssR << "[ ";
            for (int c = 0; c < 3; ++c) {
                ssR << std::fixed << std::setprecision(2) << std::setw(6) << currentMatrix.m[r][c] << " ";
            }
            ssR << "]";
            drawText(ssR.str(), matBoxX, windowHeight - 85.0f + r * 18.0f, 10.0f, Vec4(0.95f, 0.95f, 0.95f, 1.0f));
        }

        std::string tipStr = "Chuot: Click/Keo de tuong tac truc quan | Chuot phai: Pan | Cuon: Zoom | [1..4]: Che do | [F]: Doi hinh | [R]: Reset";
        drawText(tipStr, 24.0f, windowHeight - 40.0f, 9.0f, Vec4(0.5f, 0.85f, 0.7f, 1.0f));

        // Flush Batch to GPU
        if (!batchVertices.empty()) {
            glUseProgram(shaderProgram);

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
    // CALLBACKS CHUỘT & PHÍM
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
                // Click trên toolbar
                if (my <= 72.0) {
                    for (auto& btn : instance->modeButtons) {
                        if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
                            instance->currentMode = static_cast<TransformMode>(btn.id);
                            for (auto& b : instance->modeButtons) b.isSelected = (b.id == instance->currentMode);
                            instance->buildUIButtons();
                            instance->updateTransform();
                            return;
                        }
                    }
                    for (const auto& btn : instance->actionButtons) {
                        if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
                            if (btn.id == 100) instance->isAutoRotating = !instance->isAutoRotating;
                            else if (btn.id == 101) instance->resetTransform();
                            else if (btn.id == 102) instance->showGrid = !instance->showGrid;
                            else if (btn.id == 103) instance->showAxes = !instance->showAxes;
                            else if (btn.id == 104) instance->resetView();
                            instance->buildUIButtons();
                            return;
                        }
                    }
                    for (const auto& btn : instance->shapeButtons) {
                        if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
                            int sId = btn.id - 200;
                            SampleShapeType t = (sId == 0) ? SHAPE_SPACESHIP : (sId == 1 ? SHAPE_LETTER_F : (sId == 2 ? SHAPE_STAR : SHAPE_TRIANGLE));
                            instance->setShape(t);
                            instance->buildUIButtons();
                            return;
                        }
                    }
                    return;
                }

                // Click trên canvas
                instance->isMouseDown = true;
                Point2D wpt = instance->screenToWorld(mx, my);

                if (instance->currentMode == TRANS_SCALE) {
                    // Đặt tâm tỉ lệ
                    instance->scalePivot = wpt;
                    instance->updateTransform();
                } else if (instance->currentMode == TRANS_ROTATE) {
                    // Đặt tâm quay
                    instance->rotatePivot = wpt;
                    instance->updateTransform();
                } else if (instance->currentMode == TRANS_REFLECT) {
                    // Đặt điểm 1 hoặc điểm 2 của đường thẳng đối xứng
                    if (!instance->settingLineSecondPoint) {
                        instance->lineP1 = wpt;
                        instance->settingLineSecondPoint = true;
                    } else {
                        instance->lineP2 = wpt;
                        instance->settingLineSecondPoint = false;
                        instance->computeReflectionLine();
                        instance->updateTransform();
                    }
                }
            } else if (action == GLFW_RELEASE) {
                instance->isMouseDown = false;
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

    void computeReflectionLine() {
        float dx = lineP2.x - lineP1.x;
        float dy = lineP2.y - lineP1.y;

        if (std::abs(dx) < 1e-3f) {
            // Đường thẳng đứng x = c
            isVerticalLine = true;
            verticalC = lineP1.x;
        } else {
            isVerticalLine = false;
            lineA = dy / dx;
            lineB = lineP1.y - lineA * lineP1.x;
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
            Point2D wpt = instance->screenToWorld(mx, my);

            if (instance->currentMode == TRANS_TRANSLATE) {
                instance->tx = wpt.x;
                instance->ty = wpt.y;
                instance->updateTransform();
            } else if (instance->currentMode == TRANS_ROTATE && !instance->isAutoRotating) {
                float dx = wpt.x - instance->rotatePivot.x;
                float dy = wpt.y - instance->rotatePivot.y;
                instance->rotateAngle = std::atan2(dy, dx) * RAD2DEG;
                instance->updateTransform();
            }
        }
    }

    static void scrollCallback(GLFWwindow* win, double xoff, double yoff) {
        if (!instance) return;
        double mx, my;
        glfwGetCursorPos(win, &mx, &my);

        if (instance->currentMode == TRANS_SCALE && glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS) {
            float factor = (yoff > 0) ? 1.1f : 0.9f;
            instance->sx = std::clamp(instance->sx * factor, 0.1f, 10.0f);
            instance->sy = std::clamp(instance->sy * factor, 0.1f, 10.0f);
            instance->updateTransform();
            return;
        }

        float oldZoom = instance->zoom;
        float factor = (yoff > 0) ? 1.15f : 0.85f;
        instance->zoom = std::clamp(instance->zoom * factor, 0.15f, 64.0f);

        instance->panX = static_cast<float>(mx) - (static_cast<float>(mx) - instance->panX) * (instance->zoom / oldZoom);
        instance->panY = static_cast<float>(my) - (static_cast<float>(my) - instance->panY) * (instance->zoom / oldZoom);
    }

    static void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
        if (!instance || action != GLFW_PRESS) return;

        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_4) {
            instance->currentMode = static_cast<TransformMode>(key - GLFW_KEY_1);
            instance->buildUIButtons();
            instance->updateTransform();
        } else if (key == GLFW_KEY_SPACE) {
            instance->isAutoRotating = !instance->isAutoRotating;
            instance->buildUIButtons();
        } else if (key == GLFW_KEY_F) {
            int nextShape = (instance->currentShapeType + 1) % 4;
            instance->setShape(static_cast<SampleShapeType>(nextShape));
            instance->buildUIButtons();
        } else if (key == GLFW_KEY_R) {
            instance->resetTransform();
        } else if (key == GLFW_KEY_G) {
            instance->showGrid = !instance->showGrid;
        } else if (key == GLFW_KEY_X) {
            instance->showAxes = !instance->showAxes;
        } else if (key == GLFW_KEY_LEFT) {
            if (instance->currentMode == TRANS_TRANSLATE) instance->tx -= 10.0f;
            else if (instance->currentMode == TRANS_ROTATE) instance->rotateAngle -= 5.0f;
            else if (instance->currentMode == TRANS_REFLECT) instance->lineA -= 0.1f;
            instance->updateTransform();
        } else if (key == GLFW_KEY_RIGHT) {
            if (instance->currentMode == TRANS_TRANSLATE) instance->tx += 10.0f;
            else if (instance->currentMode == TRANS_ROTATE) instance->rotateAngle += 5.0f;
            else if (instance->currentMode == TRANS_REFLECT) instance->lineA += 0.1f;
            instance->updateTransform();
        } else if (key == GLFW_KEY_UP) {
            if (instance->currentMode == TRANS_TRANSLATE) instance->ty += 10.0f;
            else if (instance->currentMode == TRANS_SCALE) { instance->sx += 0.1f; instance->sy += 0.1f; }
            else if (instance->currentMode == TRANS_REFLECT) instance->lineB += 10.0f;
            instance->updateTransform();
        } else if (key == GLFW_KEY_DOWN) {
            if (instance->currentMode == TRANS_TRANSLATE) instance->ty -= 10.0f;
            else if (instance->currentMode == TRANS_SCALE) { instance->sx = std::max(0.1f, instance->sx - 0.1f); instance->sy = std::max(0.1f, instance->sy - 0.1f); }
            else if (instance->currentMode == TRANS_REFLECT) instance->lineB -= 10.0f;
            instance->updateTransform();
        } else if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(win, GLFW_TRUE);
        }
    }

    void resetTransform() {
        tx = 0.0f;
        ty = 0.0f;
        sx = 1.0f;
        sy = 1.0f;
        rotateAngle = 0.0f;
        scalePivot = Point2D(0.0f, 0.0f);
        rotatePivot = Point2D(0.0f, 0.0f);
        lineA = 1.0f;
        lineB = 0.0f;
        isVerticalLine = false;
        settingLineSecondPoint = false;
        isAutoRotating = false;
        updateTransform();
    }

    void resetView() {
        panX = windowWidth * 0.5f;
        panY = windowHeight * 0.5f;
        zoom = 1.0f;
    }

    bool runAutoTest(int frames = 60) {
        bool pass = runLab3Tests();
        for (int i = 0; i < frames; ++i) {
            glfwPollEvents();
            render();
            glfwSwapBuffers(window);
        }
        return pass;
    }
};

inline Application* Application::instance = nullptr;

} // namespace Lab3

#endif // LAB3_APP_HPP
