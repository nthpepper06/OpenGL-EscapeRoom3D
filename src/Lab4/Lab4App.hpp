#ifndef LAB4_APP_HPP
#define LAB4_APP_HPP

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
#include "Lab4Transformations.hpp"
#include "Lab4Models.hpp"

namespace Lab4 {

// Cấu trúc nút bấm giao diện UI (HUD Button)
struct UIButton {
    std::string label;
    int id = 0;
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
    Vec4 bgColor;
    Vec4 textColor;
    bool isSelected = false;
};

class Application {
public:
    GLFWwindow* window = nullptr;
    int windowWidth = 1280;
    int windowHeight = 720;

    // ------------------------------------------------------------------------
    // CÁC THAM SỐ CỦA HỆ QUAN SÁT (VIEWING SYSTEM PARAMETERS)
    // ------------------------------------------------------------------------
    // 1. Phép chiếu (Yêu cầu: song song hoặc phối cảnh)
    ProjectionMode projMode = PROJECTION_PERSPECTIVE;
    float fovY = 50.0f;          // Góc mở FOV phối cảnh (độ)
    float orthoSize = 2.8f;      // Bán kính vùng nhìn trực giao (song song)
    float zNear = 0.1f;
    float zFar = 100.0f;

    // 2. Góc quan sát θ (Theta) và Φ (Phi) để xoay vật thể
    float thetaDeg = 35.0f;      // Góc phương vị (Azimuth / Yaw quanh trục Y)
    float phiDeg = 25.0f;        // Góc nâng (Elevation / Pitch quanh trục XZ)
    float cameraRadius = 5.5f;   // Khoảng cách R từ điểm nhìn tới tâm ngắm
    bool isAutoRotating = false; // Tự động xoay quanh vật thể
    float autoRotateSpeed = 25.0f; // Độ/giây

    // 3. Vị trí quan sát (Điểm nhìn Eye & Điểm ngắm Target)
    Vec3 targetPos = Vec3(0.0f, 0.0f, 0.0f);
    Vec3 eyePos = Vec3(0.0f, 0.0f, 5.5f);
    Vec3 upVector = Vec3(0.0f, 1.0f, 0.0f);

    // 4. Phóng to, thu nhỏ vật thể (Scale)
    float modelScale = 1.0f;     // Hệ số tỉ lệ vật thể (0.2x đến 5.0x)

    // ------------------------------------------------------------------------
    // MÔ HÌNH VẬT THỂ & CHẾ ĐỘ HIỂN THỊ
    // ------------------------------------------------------------------------
    DisplayModelMode displayMode = MODEL_POLYGON_SOLID;
    int currentModelIndex = 0;   // 0..8
    std::vector<Model3D> modelList;

    // Tùy chọn render
    bool showGrid = true;
    bool showAxes = true;
    bool enableLighting = true;
    bool enableCulling = false;
    bool showMatrixHUD = true;   // Hiển thị ma trận 4x4 trên màn hình

    // Các ma trận hiện tại
    Matrix4x4 viewMatrix;
    Matrix4x4 projMatrix;
    Matrix4x4 modelMatrix;

    // ------------------------------------------------------------------------
    // TƯƠNG TÁC CHUỘT & BÀN PHÍM
    // ------------------------------------------------------------------------
    bool isLeftMouseDown = false;
    bool isRightMouseDown = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    // UI Buttons
    std::vector<UIButton> projButtons;
    std::vector<UIButton> modelTypeButtons;
    std::vector<UIButton> shapeButtons;
    std::vector<UIButton> actionButtons;

    // ------------------------------------------------------------------------
    // SHADERS & BUFFERS
    // ------------------------------------------------------------------------
    GLuint shader3D = 0;         // Shader vẽ vật thể 3D, trục tọa độ và lưới
    GLuint shader2D = 0;         // Shader vẽ giao diện HUD và chữ 5x7

    GLuint vao2D = 0;
    GLuint vbo2D = 0;

    // Trục tọa độ 3D (Ox, Oy, Oz)
    GLuint vaoAxes = 0;
    GLuint vboAxes = 0;

    // Lưới mặt phẳng tọa độ (Grid plane)
    GLuint vaoGrid = 0;
    GLuint vboGrid = 0;
    size_t gridVertexCount = 0;

    struct Vertex2D {
        float x, y;
        float r, g, b, a;
    };
    std::vector<Vertex2D> batch2D;

    static Application* instance;

    Application() {
        instance = this;
    }

    ~Application() {
        for (auto& m : modelList) {
            m.cleanup();
        }
        if (vaoAxes) glDeleteVertexArrays(1, &vaoAxes);
        if (vboAxes) glDeleteBuffers(1, &vboAxes);
        if (vaoGrid) glDeleteVertexArrays(1, &vaoGrid);
        if (vboGrid) glDeleteBuffers(1, &vboGrid);
        if (vao2D) glDeleteVertexArrays(1, &vao2D);
        if (vbo2D) glDeleteBuffers(1, &vbo2D);
        if (shader3D) glDeleteProgram(shader3D);
        if (shader2D) glDeleteProgram(shader2D);
        if (window) glfwDestroyWindow(window);
        glfwTerminate();
    }

    // ========================================================================
    // KHỞI TẠO ỨNG DỤNG (INITIALIZATION)
    // ========================================================================
    bool init(int w = 1280, int h = 720, const char* title = "LAB 04 - 3D Viewing Transformations & Projection") {
        windowWidth = w;
        windowHeight = h;

        if (!glfwInit()) {
            std::cerr << "Loi khoi tao GLFW Lab 4!\n";
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);

        window = glfwCreateWindow(windowWidth, windowHeight, title, nullptr, nullptr);
        if (!window) {
            std::cerr << "Loi tao cua so GLFW Lab 4!\n";
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Loi khoi tao GLAD Lab 4!\n";
            return false;
        }

        // Bật Depth Test và MSAA
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPointSize(7.0f);
        glLineWidth(2.0f);

        // Khởi tạo Shaders
        initShaders();

        // Khởi tạo Lưới tọa độ và Trục tọa độ
        initCoordinateAxes();
        initGridPlane(12.0f, 24);

        // Khởi tạo Buffer 2D UI
        glGenVertexArrays(1, &vao2D);
        glGenBuffers(1, &vbo2D);
        glBindVertexArray(vao2D);
        glBindBuffer(GL_ARRAY_BUFFER, vbo2D);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)(2 * sizeof(float)));
        glBindVertexArray(0);

        // Nạp danh sách 9 mô hình 3D mẫu
        loadAllModels();

        // Callbacks tương tác
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, cursorPosCallback);
        glfwSetScrollCallback(window, scrollCallback);
        glfwSetKeyCallback(window, keyCallback);

        // Tạo nút UI và cập nhật ma trận
        buildUIButtons();
        updateMatrices();

        // In hướng dẫn ra console
        printHelpToConsole();

        return true;
    }

    void loadAllModels() {
        modelList.clear();
        modelList.push_back(Model3DGenerator::createCube(1.8f));                 // 0: Cube
        modelList.push_back(Model3DGenerator::createPyramid(2.0f, 2.2f));        // 1: Pyramid
        modelList.push_back(Model3DGenerator::createTriangularPrism(1.4f, 2.2f));// 2: Prism 3
        modelList.push_back(Model3DGenerator::createHexagonalPrism(1.2f, 2.0f)); // 3: Prism 6
        modelList.push_back(Model3DGenerator::createUVSphere(1.2f, 16, 24));     // 4: Sphere
        modelList.push_back(Model3DGenerator::createCone(1.3f, 2.2f, 24));       // 5: Cone
        modelList.push_back(Model3DGenerator::createCylinder(1.1f, 2.0f, 24));   // 6: Cylinder
        modelList.push_back(Model3DGenerator::createTorus(1.2f, 0.45f, 24, 16)); // 7: Torus
        modelList.push_back(Model3DGenerator::create3DHouse(1.8f, 2.2f, 1.2f, 0.9f)); // 8: 3D House
    }

    void printHelpToConsole() const {
        std::cout << "\n====================================================================\n";
        std::cout << "  LAB 04: CHUONG TRINH QUAN SAT VAT THE 3D (3D VIEWING TRANSFORMATIONS)\n";
        std::cout << "====================================================================\n";
        std::cout << " [1] DIEU KHIEN PHEP CHIEU (PROJECTION):\n";
        std::cout << "   - Phim [P]         : Chuyen doi PHOI CANH (Perspective) <-> SONG SONG (Orthographic)\n";
        std::cout << "   - Phim [PageUp/Dn] : Thay doi goc mo FOV hoac kich thuoc Ortho\n\n";
        std::cout << " [2] DIEU KHIEN GOC QUAN SAT THETA (θ) VA PHI (Φ):\n";
        std::cout << "   - Keo Chuot Trai   : Xoay tu do quanh vat the (Orbit Camera)\n";
        std::cout << "   - Phim Mui Ten     : [Trai]/[Phai] thay doi Theta (θ), [Len]/[Xuong] thay doi Phi (Φ)\n";
        std::cout << "   - Phim [SPACE]     : Bat / Tat tu dong xoay (Auto Orbit Rotation)\n";
        std::cout << "   - Phim [1]..[4]    : Dat goc nhin nhanh (1: Front, 2: Top, 3: Right, 4: Isometric)\n\n";
        std::cout << " [3] DIEU KHIEN DIEM NHIN & VI TRI QUAN SAT (EYE & TARGET):\n";
        std::cout << "   - Keo Chuot Phai   : Di chuyen tam quan sat (Camera Panning)\n";
        std::cout << "   - Phim [W, A, S, D]: Di chuyen diem quan sat Len / Xuong / Trai / Phai\n";
        std::cout << "   - Phim [R]         : Dat lai he quan sat ve mac dinh (Reset View)\n\n";
        std::cout << " [4] PHONG TO, THU NHO VAT THE (ZOOM & SCALE):\n";
        std::cout << "   - Cuon Chuot       : Phong to / Thu nho khoang cach quan sat R (Camera Zoom)\n";
        std::cout << "   - Phim [+] hoac [-]: Phong to / Thu nho vat the qua ma tran Model Scale (0.2x - 5.0x)\n\n";
        std::cout << " [5] MO HINH BIEU DIEN VAT THE (4.1 & 4.2):\n";
        std::cout << "   - Phim [M]         : Chuyen doi 4 che do:\n";
        std::cout << "                        + 4.1. Khung ket noi (Wireframe Model)\n";
        std::cout << "                        + 4.2. Mat da giac (Polygon / Surface Model)\n";
        std::cout << "                        + Ket hop (Mat da giac + Khung vien canh)\n";
        std::cout << "                        + Tap cac diem dinh (Points)\n";
        std::cout << "   - Phim [Tab] / [O] : Chuyen doi qua 9 vat the 3D khac nhau\n";
        std::cout << "   - Phim [L]         : Bat / Tat Chieu sang (Lighting)\n";
        std::cout << "   - Phim [G] / [X]   : Bat / Tat Luoi (Grid) / Truc toa do (Axes)\n";
        std::cout << "   - Phim [K] / [ENT] : In ma tran View & Projection chi tiet ra Console\n";
        std::cout << "   - Phim [H]         : Bat / Tat Bang thong so Ma tran tren man hinh\n";
        std::cout << "====================================================================\n\n";
    }

    // ========================================================================
    // CẬP NHẬT MA TRẬN BIẾN ĐỔI (MATRIX CALCULATIONS)
    // ========================================================================
    void updateMatrices() {
        // 1. Tính toán vị trí quan sát Eye từ tọa độ cầu (θ, Φ, R) và Target
        eyePos = Matrix4x4::calculateEyeFromSpherical(targetPos, cameraRadius, thetaDeg, phiDeg);

        // 2. Ma trận biến đổi hệ quan sát (Viewing / LookAt Matrix)
        viewMatrix = Matrix4x4::makeLookAt(eyePos, targetPos, upVector);

        // 3. Ma trận phép chiếu (Projection Matrix)
        float aspect = (windowHeight > 0) ? (float)windowWidth / (float)windowHeight : 1.0f;
        if (projMode == PROJECTION_PERSPECTIVE) {
            projMatrix = Matrix4x4::makePerspective(fovY, aspect, zNear, zFar);
        } else {
            projMatrix = Matrix4x4::makeOrthographicSymmetric(orthoSize, aspect, zNear, zFar);
        }

        // 4. Ma trận biến đổi vật thể (Model Matrix = Scale)
        modelMatrix = Matrix4x4::makeScale(modelScale);
    }

    void dumpMatricesToConsole() const {
        std::cout << "\n=======================================================\n";
        std::cout << "   THONG SO MA TRAN HE QUAN SAT 3D - LAB 04            \n";
        std::cout << "=======================================================\n";
        std::cout << " - Vat the: " << modelList[currentModelIndex].name << "\n";
        std::cout << " - Mo hinh bieu dien: " 
                  << (displayMode == MODEL_WIREFRAME ? "4.1. Khung ket noi (Wireframe)" :
                     (displayMode == MODEL_POLYGON_SOLID ? "4.2. Mat da giac (Polygon Surface)" :
                     (displayMode == MODEL_SOLID_AND_WIRE ? "Mat da giac + Khung vien canh" : "Tap dinh (Points)"))) << "\n";
        std::cout << " - Phep chieu: " << (projMode == PROJECTION_PERSPECTIVE ? "PHOI CANH (Perspective)" : "SONG SONG (Orthographic)") << "\n";
        std::cout << " - Goc quan sat Theta (θ): " << std::fixed << std::setprecision(1) << thetaDeg << " do\n";
        std::cout << " - Goc quan sat Phi (Φ)  : " << phiDeg << " do\n";
        std::cout << " - Khoang cach quan sat R: " << cameraRadius << "\n";
        std::cout << " - Diem nhin Eye   : (" << eyePos.x << ", " << eyePos.y << ", " << eyePos.z << ")\n";
        std::cout << " - Diem ngam Target: (" << targetPos.x << ", " << targetPos.y << ", " << targetPos.z << ")\n";
        std::cout << " - He so phong to Scale : " << modelScale << "x\n\n";

        std::cout << viewMatrix.format("MA TRAN QUAN SAT (VIEW MATRIX - 4x4)") << "\n";
        std::cout << projMatrix.format("MA TRAN PHEP CHIEU (PROJECTION MATRIX - 4x4)") << "\n";
        std::cout << modelMatrix.format("MA TRAN TI LE VAT THE (MODEL SCALE MATRIX - 4x4)") << "\n";
        std::cout << "=======================================================\n\n";
    }

    // ========================================================================
    // SHADERS & BUFFER SETUP
    // ========================================================================
    void initShaders() {
        // Shader 3D (Đỉnh, Khung kết nối, Mặt đa giác và Chiếu sáng)
        const char* vert3D = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aNormal;
            layout (location = 2) in vec4 aColor;

            out vec3 FragPos;
            out vec3 Normal;
            out vec4 VertexColor;

            uniform mat4 uModel;
            uniform mat4 uView;
            uniform mat4 uProjection;

            void main() {
                vec4 worldPos = uModel * vec4(aPos, 1.0);
                FragPos = worldPos.xyz;
                Normal = mat3(transpose(inverse(uModel))) * aNormal;
                VertexColor = aColor;
                gl_Position = uProjection * uView * worldPos;
            }
        )";

        const char* frag3D = R"(
            #version 330 core
            in vec3 FragPos;
            in vec3 Normal;
            in vec4 VertexColor;

            out vec4 FragColor;

            uniform vec3 uViewPos;
            uniform vec3 uLightPos;
            uniform vec3 uLightColor;
            uniform vec4 uColorOverride; // Nếu .a >= 0 thì ghi đè màu
            uniform int uLightingEnabled;

            void main() {
                vec4 baseColor = (uColorOverride.a >= 0.0) ? uColorOverride : VertexColor;

                // Nếu không bật chiếu sáng (dùng cho Khung dây, Điểm đỉnh, Trục Oxy)
                if (uLightingEnabled == 0) {
                    FragColor = baseColor;
                    return;
                }

                // Chiếu sáng Blinn-Phong Shading cho Mô hình Mặt Đa Giác (4.2)
                vec3 norm = normalize(Normal);
                vec3 lightDir = normalize(uLightPos - FragPos);
                vec3 viewDir = normalize(uViewPos - FragPos);

                // 1. Thành phần Môi trường (Ambient)
                float ambientStrength = 0.35;
                vec3 ambient = ambientStrength * uLightColor * baseColor.rgb;

                // 2. Thành phần Khuếch tán (Diffuse - Lambertian)
                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = diff * uLightColor * baseColor.rgb * 0.85;

                // 3. Thành phần Phản xạ gương (Specular - Blinn-Phong)
                vec3 halfwayDir = normalize(lightDir + viewDir);
                float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
                vec3 specular = 0.35 * spec * uLightColor;

                // Ánh sáng 2 mặt (Two-sided lighting) giúp quan sát mặt trong đa giác
                float backDiff = max(dot(-norm, lightDir), 0.0);
                if (diff < 0.1 && backDiff > 0.1) {
                    diffuse += backDiff * uLightColor * baseColor.rgb * 0.5;
                }

                vec3 finalRgb = ambient + diffuse + specular;
                FragColor = vec4(finalRgb, baseColor.a);
            }
        )";

        shader3D = compileProgram(vert3D, frag3D);

        // Shader 2D (Vẽ HUD & Text)
        const char* vert2D = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec4 aColor;
            out vec4 vColor;
            uniform mat4 uOrtho2D;
            void main() {
                vColor = aColor;
                gl_Position = uOrtho2D * vec4(aPos, 0.0, 1.0);
            }
        )";

        const char* frag2D = R"(
            #version 330 core
            in vec4 vColor;
            out vec4 FragColor;
            void main() {
                FragColor = vColor;
            }
        )";

        shader2D = compileProgram(vert2D, frag2D);
    }

    GLuint compileProgram(const char* vertSrc, const char* fragSrc) {
        GLuint vert = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert, 1, &vertSrc, nullptr);
        glCompileShader(vert);

        GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag, 1, &fragSrc, nullptr);
        glCompileShader(frag);

        GLuint prog = glCreateProgram();
        glAttachShader(prog, vert);
        glAttachShader(prog, frag);
        glLinkProgram(prog);

        glDeleteShader(vert);
        glDeleteShader(frag);
        return prog;
    }

    void initCoordinateAxes() {
        // 3 Trục tọa độ: Ox (Đỏ), Oy (Xanh lá), Oz (Xanh dương)
        float len = 3.5f;
        std::vector<Vertex3D> axisVerts = {
            // Ox (Đỏ)
            Vertex3D(Vec3(0, 0, 0), Vec3(0, 1, 0), Vec4(0.95f, 0.2f, 0.2f, 1.0f)),
            Vertex3D(Vec3(len, 0, 0), Vec3(0, 1, 0), Vec4(0.95f, 0.2f, 0.2f, 1.0f)),
            // Mũi tên Ox
            Vertex3D(Vec3(len, 0, 0), Vec3(0, 1, 0), Vec4(0.95f, 0.2f, 0.2f, 1.0f)),
            Vertex3D(Vec3(len - 0.25f, 0.12f, 0), Vec3(0, 1, 0), Vec4(0.95f, 0.2f, 0.2f, 1.0f)),
            Vertex3D(Vec3(len, 0, 0), Vec3(0, 1, 0), Vec4(0.95f, 0.2f, 0.2f, 1.0f)),
            Vertex3D(Vec3(len - 0.25f, -0.12f, 0), Vec3(0, 1, 0), Vec4(0.95f, 0.2f, 0.2f, 1.0f)),

            // Oy (Xanh lá)
            Vertex3D(Vec3(0, 0, 0), Vec3(1, 0, 0), Vec4(0.2f, 0.95f, 0.3f, 1.0f)),
            Vertex3D(Vec3(0, len, 0), Vec3(1, 0, 0), Vec4(0.2f, 0.95f, 0.3f, 1.0f)),
            // Mũi tên Oy
            Vertex3D(Vec3(0, len, 0), Vec3(1, 0, 0), Vec4(0.2f, 0.95f, 0.3f, 1.0f)),
            Vertex3D(Vec3(0.12f, len - 0.25f, 0), Vec3(1, 0, 0), Vec4(0.2f, 0.95f, 0.3f, 1.0f)),
            Vertex3D(Vec3(0, len, 0), Vec3(1, 0, 0), Vec4(0.2f, 0.95f, 0.3f, 1.0f)),
            Vertex3D(Vec3(-0.12f, len - 0.25f, 0), Vec3(1, 0, 0), Vec4(0.2f, 0.95f, 0.3f, 1.0f)),

            // Oz (Xanh dương)
            Vertex3D(Vec3(0, 0, 0), Vec3(0, 1, 0), Vec4(0.2f, 0.5f, 0.98f, 1.0f)),
            Vertex3D(Vec3(0, 0, len), Vec3(0, 1, 0), Vec4(0.2f, 0.5f, 0.98f, 1.0f)),
            // Mũi tên Oz
            Vertex3D(Vec3(0, 0, len), Vec3(0, 1, 0), Vec4(0.2f, 0.5f, 0.98f, 1.0f)),
            Vertex3D(Vec3(0, 0.12f, len - 0.25f), Vec3(0, 1, 0), Vec4(0.2f, 0.5f, 0.98f, 1.0f)),
            Vertex3D(Vec3(0, 0, len), Vec3(0, 1, 0), Vec4(0.2f, 0.5f, 0.98f, 1.0f)),
            Vertex3D(Vec3(0, -0.12f, len - 0.25f), Vec3(0, 1, 0), Vec4(0.2f, 0.5f, 0.98f, 1.0f))
        };

        glGenVertexArrays(1, &vaoAxes);
        glGenBuffers(1, &vboAxes);
        glBindVertexArray(vaoAxes);
        glBindBuffer(GL_ARRAY_BUFFER, vboAxes);
        glBufferData(GL_ARRAY_BUFFER, axisVerts.size() * sizeof(Vertex3D), axisVerts.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, pos));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, color));
        glBindVertexArray(0);
    }

    void initGridPlane(float size = 12.0f, int divisions = 24) {
        std::vector<Vertex3D> gridVerts;
        float step = size / divisions;
        float half = size * 0.5f;
        float y = -1.2f; // Nằm phía dưới vật thể một chút

        Vec4 minorColor(0.25f, 0.28f, 0.35f, 0.4f);
        Vec4 majorColor(0.40f, 0.45f, 0.55f, 0.7f);

        for (int i = 0; i <= divisions; ++i) {
            float coord = -half + i * step;
            Vec4 col = (i == divisions / 2) ? majorColor : minorColor;

            // Đường dọc song song trục Oz
            gridVerts.push_back(Vertex3D(Vec3(coord, y, -half), Vec3(0, 1, 0), col));
            gridVerts.push_back(Vertex3D(Vec3(coord, y,  half), Vec3(0, 1, 0), col));

            // Đường ngang song song trục Ox
            gridVerts.push_back(Vertex3D(Vec3(-half, y, coord), Vec3(0, 1, 0), col));
            gridVerts.push_back(Vertex3D(Vec3( half, y, coord), Vec3(0, 1, 0), col));
        }

        gridVertexCount = gridVerts.size();

        glGenVertexArrays(1, &vaoGrid);
        glGenBuffers(1, &vboGrid);
        glBindVertexArray(vaoGrid);
        glBindBuffer(GL_ARRAY_BUFFER, vboGrid);
        glBufferData(GL_ARRAY_BUFFER, gridVerts.size() * sizeof(Vertex3D), gridVerts.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, pos));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, color));
        glBindVertexArray(0);
    }

    // ========================================================================
    // GIAO DIỆN NGƯỜI DÙNG HUD (2D BATCH & BITMAP FONT)
    // ========================================================================
    void buildUIButtons() {
        projButtons.clear();
        modelTypeButtons.clear();
        shapeButtons.clear();
        actionButtons.clear();

        float btnH = 26.0f;
        float yRow1 = 8.0f;
        float yRow2 = 38.0f;

        // --- HÀNG 1 (BÊN TRÁI): CHỌN PHÉP CHIẾU & MÔ HÌNH BIỂU DIỄN ---
        float curX = 12.0f;

        // Phép chiếu: Song song vs Phối cảnh
        UIButton btnPersp;
        btnPersp.id = 1;
        btnPersp.label = "Phep chieu: PHOI CANH";
        btnPersp.x = curX; btnPersp.y = yRow1; btnPersp.w = 175.0f; btnPersp.h = btnH;
        btnPersp.isSelected = (projMode == PROJECTION_PERSPECTIVE);
        btnPersp.bgColor = btnPersp.isSelected ? Vec4(0.15f, 0.55f, 0.95f, 0.95f) : Vec4(0.18f, 0.20f, 0.26f, 0.85f);
        btnPersp.textColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
        projButtons.push_back(btnPersp);
        curX += 181.0f;

        UIButton btnOrtho;
        btnOrtho.id = 2;
        btnOrtho.label = "Phep chieu: SONG SONG";
        btnOrtho.x = curX; btnOrtho.y = yRow1; btnOrtho.w = 175.0f; btnOrtho.h = btnH;
        btnOrtho.isSelected = (projMode == PROJECTION_PARALLEL);
        btnOrtho.bgColor = btnOrtho.isSelected ? Vec4(0.15f, 0.55f, 0.95f, 0.95f) : Vec4(0.18f, 0.20f, 0.26f, 0.85f);
        btnOrtho.textColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
        projButtons.push_back(btnOrtho);
        curX += 190.0f;

        // Mô hình biểu diễn: 4.1 Khung kết nối / 4.2 Mặt đa giác
        std::vector<std::string> modelLabels = {
            "4.1. Khung ket noi", "4.2. Mat da giac", "Mat + Khung vien", "Tap Dinh"
        };
        for (size_t i = 0; i < modelLabels.size(); ++i) {
            UIButton btn;
            btn.id = 10 + static_cast<int>(i);
            btn.label = modelLabels[i];
            btn.x = curX; btn.y = yRow1; btn.w = 145.0f; btn.h = btnH;
            btn.isSelected = (displayMode == static_cast<DisplayModelMode>(i));
            btn.bgColor = btn.isSelected ? Vec4(0.20f, 0.75f, 0.45f, 0.95f) : Vec4(0.20f, 0.25f, 0.28f, 0.85f);
            btn.textColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
            modelTypeButtons.push_back(btn);
            curX += 151.0f;
        }

        // Các nút thao tác nhanh
        std::vector<std::string> actLabels = {"Auto-Rot [SPACE]", "Reset View [R]", "In Ma Tran [K]", "Matr HUD [H]"};
        curX += 15.0f;
        for (size_t i = 0; i < actLabels.size(); ++i) {
            UIButton btn;
            btn.id = 50 + static_cast<int>(i);
            btn.label = actLabels[i];
            btn.x = curX; btn.y = yRow1; btn.w = 125.0f; btn.h = btnH;
            btn.isSelected = (i == 0 && isAutoRotating) || (i == 3 && showMatrixHUD);
            btn.bgColor = btn.isSelected ? Vec4(0.95f, 0.55f, 0.15f, 0.95f) : Vec4(0.25f, 0.28f, 0.35f, 0.85f);
            btn.textColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
            actionButtons.push_back(btn);
            curX += 131.0f;
        }

        // --- HÀNG 2: CHỌN 9 VẬT THỂ 3D ---
        curX = 12.0f;
        std::vector<std::string> shapeLabels = {
            "1.Lap Phuong", "2.Kim Tu Thap", "3.L.Tru Tam Giac", "4.L.Tru Luc Giac",
            "5.Hinh Cau", "6.Hinh Non", "7.Hinh Tru", "8.Hinh Xuyen", "9.Ngoi Nha 3D"
        };
        for (size_t i = 0; i < shapeLabels.size(); ++i) {
            UIButton btn;
            btn.id = 100 + static_cast<int>(i);
            btn.label = shapeLabels[i];
            btn.x = curX; btn.y = yRow2; btn.w = 132.0f; btn.h = btnH;
            btn.isSelected = (currentModelIndex == static_cast<int>(i));
            btn.bgColor = btn.isSelected ? Vec4(0.85f, 0.35f, 0.55f, 0.95f) : Vec4(0.22f, 0.20f, 0.25f, 0.85f);
            btn.textColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
            shapeButtons.push_back(btn);
            curX += 138.0f;
        }
    }

    void addQuad(float x, float y, float w, float h, const Vec4& color) {
        batch2D.push_back({x,     y,     color.x, color.y, color.z, color.w});
        batch2D.push_back({x + w, y,     color.x, color.y, color.z, color.w});
        batch2D.push_back({x + w, y + h, color.x, color.y, color.z, color.w});

        batch2D.push_back({x,     y,     color.x, color.y, color.z, color.w});
        batch2D.push_back({x + w, y + h, color.x, color.y, color.z, color.w});
        batch2D.push_back({x,     y + h, color.x, color.y, color.z, color.w});
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

    void renderHUD() {
        glDisable(GL_DEPTH_TEST);
        glUseProgram(shader2D);

        // Ma trận trực giao 2D cho HUD: [0, width] x [height, 0]
        Mat4 ortho2D = Mat4::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f, -1.0f, 1.0f);
        GLint loc = glGetUniformLocation(shader2D, "uOrtho2D");
        glUniformMatrix4fv(loc, 1, GL_FALSE, ortho2D.data());

        batch2D.clear();

        // 1. Vẽ thanh Menu phía trên
        addQuad(0.0f, 0.0f, (float)windowWidth, 72.0f, Vec4(0.09f, 0.10f, 0.14f, 0.95f));
        addQuad(0.0f, 71.0f, (float)windowWidth, 1.5f, Vec4(0.25f, 0.30f, 0.40f, 0.8f));

        // 2. Vẽ các nút bấm
        auto drawButtons = [this](const std::vector<UIButton>& btns) {
            for (const auto& btn : btns) {
                addQuad(btn.x, btn.y, btn.w, btn.h, btn.bgColor);
                // Viền nút
                Vec4 borderCol = btn.isSelected ? Vec4(1.0f, 1.0f, 1.0f, 0.9f) : Vec4(0.35f, 0.40f, 0.50f, 0.6f);
                addQuad(btn.x, btn.y, btn.w, 1.0f, borderCol);
                addQuad(btn.x, btn.y + btn.h - 1.0f, btn.w, 1.0f, borderCol);
                addQuad(btn.x, btn.y, 1.0f, btn.h, borderCol);
                addQuad(btn.x + btn.w - 1.0f, btn.y, 1.0f, btn.h, borderCol);

                // Chữ trong nút
                float textW = btn.label.size() * (9.0f * 1.25f);
                float tx = btn.x + (btn.w - textW) * 0.5f;
                if (tx < btn.x + 4.0f) tx = btn.x + 4.0f;
                drawText(btn.label, tx, btn.y + 6.0f, 9.0f, btn.textColor);
            }
        };

        drawButtons(projButtons);
        drawButtons(modelTypeButtons);
        drawButtons(actionButtons);
        drawButtons(shapeButtons);

        // 3. Vẽ Bảng thông số Hệ quan sát & Ma trận (HUD Panel góc trái dưới)
        if (showMatrixHUD) {
            float panelW = 420.0f;
            float panelH = 265.0f;
            float px = 14.0f;
            float py = (float)windowHeight - panelH - 14.0f;

            // Khung nền
            addQuad(px, py, panelW, panelH, Vec4(0.08f, 0.10f, 0.14f, 0.90f));
            addQuad(px, py, panelW, 22.0f, Vec4(0.14f, 0.22f, 0.35f, 0.95f));
            addQuad(px, py, panelW, 1.5f, Vec4(0.3f, 0.5f, 0.8f, 0.8f));
            addQuad(px, py + panelH - 1.5f, panelW, 1.5f, Vec4(0.3f, 0.5f, 0.8f, 0.8f));
            addQuad(px, py, 1.5f, panelH, Vec4(0.3f, 0.5f, 0.8f, 0.8f));
            addQuad(px + panelW - 1.5f, py, 1.5f, panelH, Vec4(0.3f, 0.5f, 0.8f, 0.8f));

            drawText("HE QUAN SAT 3D & MA TRAN BIEN DOI", px + 10.0f, py + 5.0f, 10.0f, Vec4(1.0f, 1.0f, 0.4f, 1.0f));

            float curY = py + 28.0f;
            float lineStep = 15.0f;
            Vec4 lblCol(0.7f, 0.8f, 0.9f, 1.0f);
            Vec4 valCol(0.2f, 0.9f, 0.9f, 1.0f);

            // Vat the
            drawText("VAT THE 3D:", px + 10.0f, curY, 9.0f, lblCol);
            drawText(modelList[currentModelIndex].name, px + 130.0f, curY, 9.0f, Vec4(1.0f, 0.7f, 0.3f, 1.0f));
            curY += lineStep;

            // Mo hinh
            drawText("MO HINH:", px + 10.0f, curY, 9.0f, lblCol);
            std::string mStr = (displayMode == MODEL_WIREFRAME ? "4.1. KHUNG KET NOI (WIREFRAME)" :
                               (displayMode == MODEL_POLYGON_SOLID ? "4.2. MAT DA GIAC (POLYGON SOLID)" :
                               (displayMode == MODEL_SOLID_AND_WIRE ? "MAT DA GIAC + KHUNG VIEN" : "TAP DINH (POINTS)")));
            drawText(mStr, px + 130.0f, curY, 9.0f, Vec4(0.4f, 1.0f, 0.5f, 1.0f));
            curY += lineStep;

            // Phep chieu
            drawText("PHEP CHIEU:", px + 10.0f, curY, 9.0f, lblCol);
            drawText(projMode == PROJECTION_PERSPECTIVE ? "PHOI CANH (PERSPECTIVE)" : "SONG SONG (ORTHOGRAPHIC)",
                     px + 130.0f, curY, 9.0f, Vec4(0.3f, 0.8f, 1.0f, 1.0f));
            curY += lineStep;

            // Goc Theta, Phi, R
            std::stringstream ssGoc;
            ssGoc << "THETA: " << (int)thetaDeg << "  PHI: " << (int)phiDeg << "  R: " << std::fixed << std::setprecision(1) << cameraRadius;
            drawText(ssGoc.str(), px + 10.0f, curY, 9.0f, valCol);
            curY += lineStep;

            // Diem nhin Eye & Target
            std::stringstream ssEye, ssTarget;
            ssEye << "EYE: (" << std::fixed << std::setprecision(1) << eyePos.x << ", " << eyePos.y << ", " << eyePos.z << ")";
            ssTarget << "TARGET: (" << targetPos.x << ", " << targetPos.y << ", " << targetPos.z << ") SCALE: " << modelScale << "X";
            drawText(ssEye.str(), px + 10.0f, curY, 9.0f, Vec4(0.9f, 0.8f, 0.2f, 1.0f));
            curY += lineStep;
            drawText(ssTarget.str(), px + 10.0f, curY, 9.0f, Vec4(0.9f, 0.8f, 0.2f, 1.0f));
            curY += lineStep + 3.0f;

            // In 4 dong cua Ma tran Quan sat View Matrix
            drawText("--- MA TRAN QUAN SAT VIEW (4x4) ---", px + 10.0f, curY, 9.0f, Vec4(1.0f, 0.9f, 0.5f, 1.0f));
            curY += 13.0f;
            for (int r = 0; r < 4; ++r) {
                std::stringstream ssRow;
                ssRow << "[";
                for (int c = 0; c < 4; ++c) {
                    ssRow << (viewMatrix.m[r][c] >= 0 ? " " : "")
                          << std::fixed << std::setprecision(2) << viewMatrix.m[r][c] << (c < 3 ? " " : "");
                }
                ssRow << "]";
                drawText(ssRow.str(), px + 10.0f, curY, 8.5f, Vec4(0.8f, 0.9f, 1.0f, 1.0f));
                curY += 12.0f;
            }

            // In 4 dong cua Ma tran Phep chieu Projection Matrix
            curY += 4.0f;
            drawText("--- MA TRAN PHEP CHIEU PROJECTION (4x4) ---", px + 10.0f, curY, 9.0f, Vec4(0.5f, 0.9f, 1.0f, 1.0f));
            curY += 13.0f;
            for (int r = 0; r < 4; ++r) {
                std::stringstream ssRow;
                ssRow << "[";
                for (int c = 0; c < 4; ++c) {
                    ssRow << (projMatrix.m[r][c] >= 0 ? " " : "")
                          << std::fixed << std::setprecision(2) << projMatrix.m[r][c] << (c < 3 ? " " : "");
                }
                ssRow << "]";
                drawText(ssRow.str(), px + 10.0f, curY, 8.5f, Vec4(0.7f, 1.0f, 0.8f, 1.0f));
                curY += 12.0f;
            }
        }

        // 4. Vẽ Bảng Phím tắt (Góc phải dưới)
        {
            float panelW = 320.0f;
            float panelH = 155.0f;
            float px = (float)windowWidth - panelW - 14.0f;
            float py = (float)windowHeight - panelH - 14.0f;

            addQuad(px, py, panelW, panelH, Vec4(0.08f, 0.10f, 0.14f, 0.85f));
            addQuad(px, py, panelW, 20.0f, Vec4(0.18f, 0.22f, 0.30f, 0.95f));
            addQuad(px, py, panelW, 1.0f, Vec4(0.4f, 0.5f, 0.6f, 0.7f));
            addQuad(px, py + panelH - 1.0f, panelW, 1.0f, Vec4(0.4f, 0.5f, 0.6f, 0.7f));
            addQuad(px, py, 1.0f, panelH, Vec4(0.4f, 0.5f, 0.6f, 0.7f));
            addQuad(px + panelW - 1.0f, py, 1.0f, panelH, Vec4(0.4f, 0.5f, 0.6f, 0.7f));

            drawText("HUONG DAN PHIM TAT NHANH", px + 10.0f, py + 4.0f, 9.5f, Vec4(1.0f, 0.9f, 0.3f, 1.0f));

            float curY = py + 25.0f;
            float step = 14.0f;
            drawText("P: Chuyen Phoi Canh / Song Song", px + 10.0f, curY, 8.5f, Vec4(0.9f, 0.9f, 0.9f, 1.0f)); curY += step;
            drawText("M: Chuyen Khung Day / Mat Da Giac", px + 10.0f, curY, 8.5f, Vec4(0.9f, 0.9f, 0.9f, 1.0f)); curY += step;
            drawText("Keo Chuot Trai: Xoay goc quan sat", px + 10.0f, curY, 8.5f, Vec4(0.9f, 0.9f, 0.9f, 1.0f)); curY += step;
            drawText("Keo Chuot Phai/WASD: Pan diem nhin", px + 10.0f, curY, 8.5f, Vec4(0.9f, 0.9f, 0.9f, 1.0f)); curY += step;
            drawText("Cuon Chuot: Zoom khoang cach R", px + 10.0f, curY, 8.5f, Vec4(0.9f, 0.9f, 0.9f, 1.0f)); curY += step;
            drawText("+ / -: Phong to / Thu nho vat the", px + 10.0f, curY, 8.5f, Vec4(0.9f, 0.9f, 0.9f, 1.0f)); curY += step;
            drawText("Tab: Doi vat the  R: Reset nhin", px + 10.0f, curY, 8.5f, Vec4(0.9f, 0.9f, 0.9f, 1.0f)); curY += step;
            drawText("SPACE: Tu dong xoay  K: In ma tran", px + 10.0f, curY, 8.5f, Vec4(0.9f, 0.9f, 0.9f, 1.0f)); curY += step;
        }

        // Tải dữ liệu 2D lên GPU và vẽ
        glBindVertexArray(vao2D);
        glBindBuffer(GL_ARRAY_BUFFER, vbo2D);
        glBufferData(GL_ARRAY_BUFFER, batch2D.size() * sizeof(Vertex2D), batch2D.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(batch2D.size()));
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);
    }

    // ========================================================================
    // VÒNG LẶP RENDER CHÍNH (RENDER LOOP)
    // ========================================================================
    void render() {
        // Tự động xoay quanh vật thể nếu bật SPACE
        if (isAutoRotating) {
            thetaDeg += 0.5f;
            if (thetaDeg >= 360.0f) thetaDeg -= 360.0f;
            updateMatrices();
        }

        glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Khử mặt khuất nếu bật
        if (enableCulling) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        } else {
            glDisable(GL_CULL_FACE);
        }

        // Kích hoạt Shader 3D
        glUseProgram(shader3D);

        // Thiết lập Uniforms Ma trận 4x4
        GLint locModel = glGetUniformLocation(shader3D, "uModel");
        GLint locView = glGetUniformLocation(shader3D, "uView");
        GLint locProj = glGetUniformLocation(shader3D, "uProjection");
        GLint locViewPos = glGetUniformLocation(shader3D, "uViewPos");
        GLint locLightPos = glGetUniformLocation(shader3D, "uLightPos");
        GLint locLightColor = glGetUniformLocation(shader3D, "uLightColor");
        GLint locColorOver = glGetUniformLocation(shader3D, "uColorOverride");
        GLint locLighting = glGetUniformLocation(shader3D, "uLightingEnabled");

        Mat4 matView = viewMatrix.toMat4();
        Mat4 matProj = projMatrix.toMat4();
        Mat4 matModel = modelMatrix.toMat4();

        glUniformMatrix4fv(locView, 1, GL_FALSE, matView.data());
        glUniformMatrix4fv(locProj, 1, GL_FALSE, matProj.data());
        glUniform3f(locViewPos, eyePos.x, eyePos.y, eyePos.z);

        // Nguồn sáng đặt chếch phía trên bên phải
        Vec3 lightPos(4.0f, 6.0f, 5.0f);
        glUniform3f(locLightPos, lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(locLightColor, 1.0f, 0.98f, 0.95f);

        // --------------------------------------------------------------------
        // 1. VẼ LƯỚI TỌA ĐỘ (GRID PLANE)
        // --------------------------------------------------------------------
        if (showGrid && vaoGrid) {
            Mat4 ident = Mat4::identity();
            glUniformMatrix4fv(locModel, 1, GL_FALSE, ident.data());
            glUniform4f(locColorOver, -1.0f, -1.0f, -1.0f, -1.0f);
            glUniform1i(locLighting, 0); // Không chiếu sáng lưới

            glBindVertexArray(vaoGrid);
            glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(gridVertexCount));
            glBindVertexArray(0);
        }

        // --------------------------------------------------------------------
        // 2. VẼ HỆ TRỤC TỌA ĐỘ 3D (OX, OY, OZ)
        // --------------------------------------------------------------------
        if (showAxes && vaoAxes) {
            Mat4 ident = Mat4::identity();
            glUniformMatrix4fv(locModel, 1, GL_FALSE, ident.data());
            glUniform4f(locColorOver, -1.0f, -1.0f, -1.0f, -1.0f);
            glUniform1i(locLighting, 0);

            glLineWidth(2.5f);
            glBindVertexArray(vaoAxes);
            glDrawArrays(GL_LINES, 0, 18);
            glBindVertexArray(0);
            glLineWidth(2.0f);
        }

        // --------------------------------------------------------------------
        // 3. VẼ VẬT THỂ 3D THEO MÔ HÌNH YÊU CẦU (4.1 & 4.2)
        // --------------------------------------------------------------------
        if (currentModelIndex >= 0 && currentModelIndex < static_cast<int>(modelList.size())) {
            const auto& currentModel = modelList[currentModelIndex];
            glUniformMatrix4fv(locModel, 1, GL_FALSE, matModel.data());

            // 4.2. MÔ HÌNH MẶT ĐA GIÁC (POLYGON / SURFACE MODEL)
            if (displayMode == MODEL_POLYGON_SOLID || displayMode == MODEL_SOLID_AND_WIRE) {
                glUniform4f(locColorOver, -1.0f, -1.0f, -1.0f, -1.0f); // Dùng màu từng mặt
                glUniform1i(locLighting, enableLighting ? 1 : 0);

                // Nếu có vẽ đè khung dây, dùng polygon offset để chống Z-fighting
                if (displayMode == MODEL_SOLID_AND_WIRE) {
                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(1.0f, 1.0f);
                }

                currentModel.renderSolid();

                if (displayMode == MODEL_SOLID_AND_WIRE) {
                    glDisable(GL_POLYGON_OFFSET_FILL);
                }
            }

            // 4.1. MÔ HÌNH KHUNG KẾT NỐI (WIREFRAME MODEL)
            if (displayMode == MODEL_WIREFRAME || displayMode == MODEL_SOLID_AND_WIRE) {
                glUniform1i(locLighting, 0); // Khung dây không cần bóng đổ

                if (displayMode == MODEL_WIREFRAME) {
                    // Màu xanh ngọc Neon sáng rực rỡ cho khung dây thuần túy
                    glUniform4f(locColorOver, 0.0f, 0.95f, 0.85f, 1.0f);
                    glLineWidth(2.2f);
                    currentModel.renderWireframe();

                    // Vẽ thêm các điểm đỉnh (Vertices Points) làm nổi bật các mối liên kết
                    glUniform4f(locColorOver, 1.0f, 0.85f, 0.2f, 1.0f);
                    glPointSize(8.0f);
                    currentModel.renderPoints();
                } else {
                    // Khung viền đè lên mặt đa giác: màu đen / xanh đậm sắc nét
                    glUniform4f(locColorOver, 0.1f, 0.12f, 0.15f, 1.0f);
                    glLineWidth(1.8f);
                    currentModel.renderWireframe();
                }
            }

            // MÔ HÌNH CHỈ HIỂN THỊ ĐỈNH (POINTS)
            if (displayMode == MODEL_POINTS) {
                glUniform1i(locLighting, 0);
                glUniform4f(locColorOver, 1.0f, 0.85f, 0.2f, 1.0f);
                glPointSize(9.0f);
                currentModel.renderPoints();
            }
        }

        // --------------------------------------------------------------------
        // 4. VẼ GIAO DIỆN NGƯỜI DÙNG HUD
        // --------------------------------------------------------------------
        renderHUD();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    void run() {
        while (!glfwWindowShouldClose(window)) {
            render();
        }
    }

    // ========================================================================
    // XỬ LÝ SỰ KIỆN TƯƠNG TÁC (INPUT CALLBACKS)
    // ========================================================================
    static void framebufferSizeCallback(GLFWwindow* w, int width, int height) {
        if (instance) {
            instance->windowWidth = width;
            instance->windowHeight = height;
            glViewport(0, 0, width, height);
            instance->buildUIButtons();
            instance->updateMatrices();
        }
    }

    static void mouseButtonCallback(GLFWwindow* w, int button, int action, int mods) {
        if (!instance) return;

        double mx, my;
        glfwGetCursorPos(w, &mx, &my);

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                instance->isLeftMouseDown = true;
                instance->lastMouseX = mx;
                instance->lastMouseY = my;

                // Kiểm tra click vào nút bấm trên thanh Menu HUD (Y <= 72)
                if (my <= 72.0) {
                    instance->handleUIClick((float)mx, (float)my);
                }
            } else if (action == GLFW_RELEASE) {
                instance->isLeftMouseDown = false;
            }
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                instance->isRightMouseDown = true;
                instance->lastMouseX = mx;
                instance->lastMouseY = my;
            } else if (action == GLFW_RELEASE) {
                instance->isRightMouseDown = false;
            }
        }
    }

    void handleUIClick(float mx, float my) {
        // 1. Phép chiếu
        for (const auto& btn : projButtons) {
            if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
                if (btn.id == 1) projMode = PROJECTION_PERSPECTIVE;
                else if (btn.id == 2) projMode = PROJECTION_PARALLEL;
                buildUIButtons();
                updateMatrices();
                return;
            }
        }

        // 2. Chế độ mô hình (4.1 Khung kết nối, 4.2 Mặt đa giác...)
        for (const auto& btn : modelTypeButtons) {
            if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
                displayMode = static_cast<DisplayModelMode>(btn.id - 10);
                buildUIButtons();
                return;
            }
        }

        // 3. Nút chức năng
        for (const auto& btn : actionButtons) {
            if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
                if (btn.id == 50) isAutoRotating = !isAutoRotating;
                else if (btn.id == 51) resetView();
                else if (btn.id == 52) dumpMatricesToConsole();
                else if (btn.id == 53) showMatrixHUD = !showMatrixHUD;
                buildUIButtons();
                return;
            }
        }

        // 4. Chọn vật thể 3D
        for (const auto& btn : shapeButtons) {
            if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
                currentModelIndex = btn.id - 100;
                buildUIButtons();
                return;
            }
        }
    }

    static void cursorPosCallback(GLFWwindow* w, double xpos, double ypos) {
        if (!instance) return;

        double dx = xpos - instance->lastMouseX;
        double dy = ypos - instance->lastMouseY;
        instance->lastMouseX = xpos;
        instance->lastMouseY = ypos;

        // Chuột trái: Thay đổi góc quan sát θ (Theta) và Φ (Phi) để xoay vật thể
        if (instance->isLeftMouseDown && instance->lastMouseY > 72.0) {
            instance->thetaDeg += static_cast<float>(dx) * 0.45f;
            instance->phiDeg += static_cast<float>(dy) * 0.45f;

            // Khóa góc ngẩng Phi [-89°, +89°] để tránh lật camera
            if (instance->phiDeg > 89.0f) instance->phiDeg = 89.0f;
            if (instance->phiDeg < -89.0f) instance->phiDeg = -89.0f;

            while (instance->thetaDeg >= 360.0f) instance->thetaDeg -= 360.0f;
            while (instance->thetaDeg < 0.0f) instance->thetaDeg += 360.0f;

            instance->updateMatrices();
        }

        // Chuột phải: Thay đổi vị trí quan sát (Panning điểm nhìn & điểm ngắm)
        if (instance->isRightMouseDown) {
            float panFactor = 0.005f * instance->cameraRadius;
            // Vector sang phải và lên của camera
            Vec3 f = (instance->targetPos - instance->eyePos).normalized();
            Vec3 s = cross(f, instance->upVector).normalized();
            Vec3 u = cross(s, f);

            Vec3 delta = s * (-static_cast<float>(dx) * panFactor) + u * (static_cast<float>(dy) * panFactor);
            instance->targetPos += delta;
            instance->updateMatrices();
        }
    }

    static void scrollCallback(GLFWwindow* w, double xoffset, double yoffset) {
        if (!instance) return;
        // Phóng to, thu nhỏ khoảng cách quan sát (Camera Zoom)
        instance->cameraRadius -= static_cast<float>(yoffset) * 0.4f;
        if (instance->cameraRadius < 1.0f) instance->cameraRadius = 1.0f;
        if (instance->cameraRadius > 25.0f) instance->cameraRadius = 25.0f;

        // Nếu ở chế độ song song, thu phóng kích thước ortho
        if (instance->projMode == PROJECTION_PARALLEL) {
            instance->orthoSize -= static_cast<float>(yoffset) * 0.2f;
            if (instance->orthoSize < 0.5f) instance->orthoSize = 0.5f;
            if (instance->orthoSize > 15.0f) instance->orthoSize = 15.0f;
        }

        instance->updateMatrices();
    }

    void resetView() {
        thetaDeg = 35.0f;
        phiDeg = 25.0f;
        cameraRadius = 5.5f;
        targetPos = Vec3(0.0f, 0.0f, 0.0f);
        modelScale = 1.0f;
        fovY = 50.0f;
        orthoSize = 2.8f;
        isAutoRotating = false;
        updateMatrices();
        buildUIButtons();
    }

    static void keyCallback(GLFWwindow* w, int key, int scancode, int action, int mods) {
        if (!instance || (action != GLFW_PRESS && action != GLFW_REPEAT)) return;

        switch (key) {
            // [P]: Chuyển đổi phép chiếu (Song song <-> Phối cảnh)
            case GLFW_KEY_P:
                instance->projMode = (instance->projMode == PROJECTION_PERSPECTIVE) ? PROJECTION_PARALLEL : PROJECTION_PERSPECTIVE;
                instance->buildUIButtons();
                instance->updateMatrices();
                std::cout << "[Phep Chieu] Da chuyen sang: "
                          << (instance->projMode == PROJECTION_PERSPECTIVE ? "PHOI CANH (Perspective)" : "SONG SONG (Orthographic)") << "\n";
                break;

            // [M]: Chuyển đổi mô hình (Khung kết nối 4.1 <-> Mặt đa giác 4.2)
            case GLFW_KEY_M:
                instance->displayMode = static_cast<DisplayModelMode>((instance->displayMode + 1) % 4);
                instance->buildUIButtons();
                std::cout << "[Mo Hinh] Da chuyen sang: "
                          << (instance->displayMode == MODEL_WIREFRAME ? "4.1. Khung ket noi (Wireframe)" :
                             (instance->displayMode == MODEL_POLYGON_SOLID ? "4.2. Mat da giac (Solid)" :
                             (instance->displayMode == MODEL_SOLID_AND_WIRE ? "Mat da giac + Khung vien" : "Tap dinh (Points)"))) << "\n";
                break;

            // [Tab] hoặc [O]: Đổi vật thể 3D
            case GLFW_KEY_TAB:
            case GLFW_KEY_O:
                instance->currentModelIndex = (instance->currentModelIndex + 1) % instance->modelList.size();
                instance->buildUIButtons();
                std::cout << "[Vat The 3D] " << instance->modelList[instance->currentModelIndex].name << "\n";
                break;

            // Phím mũi tên: Thay đổi góc quan sát θ (Theta) và Φ (Phi)
            case GLFW_KEY_LEFT:
                instance->thetaDeg -= 5.0f;
                if (instance->thetaDeg < 0.0f) instance->thetaDeg += 360.0f;
                instance->updateMatrices();
                break;
            case GLFW_KEY_RIGHT:
                instance->thetaDeg += 5.0f;
                if (instance->thetaDeg >= 360.0f) instance->thetaDeg -= 360.0f;
                instance->updateMatrices();
                break;
            case GLFW_KEY_UP:
                instance->phiDeg += 5.0f;
                if (instance->phiDeg > 89.0f) instance->phiDeg = 89.0f;
                instance->updateMatrices();
                break;
            case GLFW_KEY_DOWN:
                instance->phiDeg -= 5.0f;
                if (instance->phiDeg < -89.0f) instance->phiDeg = -89.0f;
                instance->updateMatrices();
                break;

            // Phím [W, S, A, D]: Thay đổi vị trí quan sát (Điểm nhìn / Điểm ngắm)
            case GLFW_KEY_W:
                instance->targetPos.y += 0.2f;
                instance->updateMatrices();
                break;
            case GLFW_KEY_S:
                instance->targetPos.y -= 0.2f;
                instance->updateMatrices();
                break;
            case GLFW_KEY_A:
                instance->targetPos.x -= 0.2f;
                instance->updateMatrices();
                break;
            case GLFW_KEY_D:
                instance->targetPos.x += 0.2f;
                instance->updateMatrices();
                break;

            // Phóng to, thu nhỏ vật thể bằng [+] và [-] (Model Scaling)
            case GLFW_KEY_EQUAL:
            case GLFW_KEY_KP_ADD:
                instance->modelScale = std::min(5.0f, instance->modelScale + 0.1f);
                instance->updateMatrices();
                std::cout << "[Thu Phong] He so Scale: " << instance->modelScale << "x\n";
                break;
            case GLFW_KEY_MINUS:
            case GLFW_KEY_KP_SUBTRACT:
                instance->modelScale = std::max(0.2f, instance->modelScale - 0.1f);
                instance->updateMatrices();
                std::cout << "[Thu Phong] He so Scale: " << instance->modelScale << "x\n";
                break;

            // Phóng to/thu nhỏ góc mở FOV hoặc kích thước trực giao
            case GLFW_KEY_PAGE_UP:
                if (instance->projMode == PROJECTION_PERSPECTIVE) {
                    instance->fovY = std::max(15.0f, instance->fovY - 5.0f);
                } else {
                    instance->orthoSize = std::max(0.5f, instance->orthoSize - 0.3f);
                }
                instance->updateMatrices();
                break;
            case GLFW_KEY_PAGE_DOWN:
                if (instance->projMode == PROJECTION_PERSPECTIVE) {
                    instance->fovY = std::min(110.0f, instance->fovY + 5.0f);
                } else {
                    instance->orthoSize = std::min(15.0f, instance->orthoSize + 0.3f);
                }
                instance->updateMatrices();
                break;

            // Phím [SPACE]: Tự động quay
            case GLFW_KEY_SPACE:
                instance->isAutoRotating = !instance->isAutoRotating;
                instance->buildUIButtons();
                break;

            // Phím [R]: Đặt lại góc nhìn mặc định
            case GLFW_KEY_R:
                instance->resetView();
                std::cout << "[Reset] Da dat lai he quan sat ve ban dau!\n";
                break;

            // Phím [1]..[4]: Góc nhìn Preset nhanh
            case GLFW_KEY_1:
                // Front View
                instance->thetaDeg = 0.0f; instance->phiDeg = 0.0f;
                instance->updateMatrices();
                std::cout << "[Goc nhin] TRUOC (Front View: Theta=0, Phi=0)\n";
                break;
            case GLFW_KEY_2:
                // Top View
                instance->thetaDeg = 0.0f; instance->phiDeg = 89.0f;
                instance->updateMatrices();
                std::cout << "[Goc nhin] TREN (Top View: Theta=0, Phi=89)\n";
                break;
            case GLFW_KEY_3:
                // Right View
                instance->thetaDeg = 90.0f; instance->phiDeg = 0.0f;
                instance->updateMatrices();
                std::cout << "[Goc nhin] PHAI (Right View: Theta=90, Phi=0)\n";
                break;
            case GLFW_KEY_4:
                // Isometric View
                instance->thetaDeg = 45.0f; instance->phiDeg = 35.264f;
                instance->updateMatrices();
                std::cout << "[Goc nhin] TRUC DO (Isometric View: Theta=45, Phi=35.26)\n";
                break;

            // Phím [L]: Bật/tắt chiếu sáng
            case GLFW_KEY_L:
                instance->enableLighting = !instance->enableLighting;
                std::cout << "[Chieu Sang] " << (instance->enableLighting ? "BAT [ON]" : "TAT [OFF]") << "\n";
                break;

            // Phím [G]: Bật/tắt lưới tọa độ
            case GLFW_KEY_G:
                instance->showGrid = !instance->showGrid;
                break;

            // Phím [X]: Bật/tắt trục tọa độ
            case GLFW_KEY_X:
                instance->showAxes = !instance->showAxes;
                break;

            // Phím [C]: Bật/tắt Back-Face Culling
            case GLFW_KEY_C:
                instance->enableCulling = !instance->enableCulling;
                std::cout << "[Khu Mat Khuat Culling] " << (instance->enableCulling ? "BAT [ON]" : "TAT [OFF]") << "\n";
                break;

            // Phím [K] hoặc [ENTER]: Xuất ma trận chi tiết ra Console
            case GLFW_KEY_K:
            case GLFW_KEY_ENTER:
                instance->dumpMatricesToConsole();
                break;

            // Phím [H]: Ẩn/hiện HUD ma trận
            case GLFW_KEY_H:
                instance->showMatrixHUD = !instance->showMatrixHUD;
                instance->buildUIButtons();
                break;

            // [ESC]: Thoát
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(w, GLFW_TRUE);
                break;
        }
    }
};

inline Application* Application::instance = nullptr;

} // namespace Lab4

#endif // LAB4_APP_HPP
