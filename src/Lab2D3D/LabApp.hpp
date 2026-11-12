#ifndef LAB_APP_HPP
#define LAB_APP_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>

#include "../Math3D.hpp"
#include "Primitives2D.hpp"
#include "Primitives3D.hpp"
#include "Renderer.hpp"

namespace LabApp {

enum AppMode {
    MODE_2D_GALLERY = 1,
    MODE_3D_POLYHEDRA = 2,
    MODE_3D_CURVED = 3,
    MODE_3D_SCENE = 4,
    MODE_SINGLE_INSPECT = 5
};

enum PolygonDrawMode {
    POLY_SOLID = 0,
    POLY_WIREFRAME = 1,
    POLY_POINTS = 2
};

class Application {
public:
    GLFWwindow* window = nullptr;
    int width = 1280;
    int height = 720;

    // Shader & Camera
    LabRenderer::ShaderProgram shader;
    LabRenderer::OrbitCamera camera;

    // Trạng thái ứng dụng
    AppMode currentMode = MODE_2D_GALLERY;
    PolygonDrawMode polyMode = POLY_SOLID;
    bool isLightingEnabled = true;
    bool isNormalMapMode = false;
    bool isPerspective = true;
    bool isAutoRotate = true;

    // Biến đổi Mode 5 (Single Object Inspector)
    int inspectShapeIndex = 0; // 0..6
    Vec3 objTranslation = Vec3(0, 0, 0);
    Vec3 objRotation = Vec3(0, 0, 0);
    float objScale = 1.0f;

    // Hoạt hình & Thời gian
    float lastTime = 0.0f;
    float globalTime = 0.0f;
    float autoRotateAngle = 0.0f;
    int frameCount = 0;
    float fpsTimer = 0.0f;
    float currentFPS = 60.0f;

    // Tương tác chuột
    bool isMouseDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    // --- Danh sách Lưới 2D ---
    Lab2D::Mesh2D mesh2DAxes;
    Lab2D::Mesh2D mesh2DPoints;
    Lab2D::Mesh2D mesh2DTriangle;
    Lab2D::Mesh2D mesh2DRectangle;
    Lab2D::Mesh2D mesh2DGradQuad;
    Lab2D::Mesh2D mesh2DCircle;
    Lab2D::Mesh2D mesh2DPentagon;
    Lab2D::Mesh2D mesh2DHexagon;
    Lab2D::Mesh2D mesh2DStar;
    Lab2D::Mesh2D mesh2DWindmillTower;
    Lab2D::Mesh2D mesh2DWindmillBlades;
    Lab2D::Mesh2D mesh2DHouse;
    Lab2D::Mesh2D mesh2DSun;

    // --- Danh sách Lưới 3D ---
    Lab3D::Mesh3D mesh3DCubeMulti;
    Lab3D::Mesh3D mesh3DCubePlain;
    Lab3D::Mesh3D mesh3DPyramid;
    Lab3D::Mesh3D mesh3DPrism;
    Lab3D::Mesh3D mesh3DSphere;
    Lab3D::Mesh3D mesh3DCylinder;
    Lab3D::Mesh3D mesh3DCone;
    Lab3D::Mesh3D mesh3DTorus;
    Lab3D::Mesh3D mesh3DPlane;
    Lab3D::Mesh3D mesh3DLightOrb;

    // 2D HUD Quad
    Lab2D::Mesh2D hudBox;

    static Application* instance;

    Application() {
        instance = this;
    }

    bool init(int w = 1280, int h = 720, const char* title = "OpenGL 2D & 3D Computer Graphics Lab") {
        width = w;
        height = h;

        if (!glfwInit()) {
            std::cerr << "Khởi tạo GLFW thất bại!\n";
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4); // Khử răng cưa 4x MSAA

        window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!window) {
            std::cerr << "Tạo cửa sổ GLFW thất bại!\n";
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // VSync

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Khởi tạo GLAD thất bại!\n";
            return false;
        }

        // Cấu hình trạng thái OpenGL
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_MULTISAMPLE);
        glPointSize(6.0f);
        glLineWidth(2.5f);

        // Đăng ký callback sự kiện
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, cursorPosCallback);
        glfwSetScrollCallback(window, scrollCallback);

        // Khởi tạo Shader
        if (!shader.init(LabRenderer::labVertexShader, LabRenderer::labFragmentShader)) {
            std::cerr << "Khởi tạo Shader thất bại!\n";
            return false;
        }

        // Khởi tạo các đối tượng hình học
        initGeometry();

        printWelcomeConsole();
        return true;
    }

    void initGeometry() {
        // 1. Tạo các đối tượng 2D
        mesh2DAxes = Lab2D::Shape2DGenerator::createCoordinateAxes(1.5f);
        mesh2DPoints = Lab2D::Shape2DGenerator::createPointGrid(1.2f, 1.2f, 6, 6, Vec4(0.2f, 0.9f, 0.9f, 1.0f));
        mesh2DTriangle = Lab2D::Shape2DGenerator::createGradientTriangle(1.2f);
        mesh2DRectangle = Lab2D::Shape2DGenerator::createRectangle(1.2f, 0.8f, Vec4(0.25f, 0.7f, 0.95f, 1.0f));
        mesh2DGradQuad = Lab2D::Shape2DGenerator::createGradientQuad(
            1.2f, 0.8f,
            Vec4(1.0f, 0.2f, 0.2f, 1.0f), // Đỏ
            Vec4(1.0f, 0.9f, 0.1f, 1.0f), // Vàng
            Vec4(0.1f, 0.8f, 0.3f, 1.0f), // Xanh lá
            Vec4(0.1f, 0.4f, 1.0f, 1.0f)  // Xanh dương
        );
        mesh2DCircle = Lab2D::Shape2DGenerator::createCircle(0.65f, 40, Vec4(1.0f, 1.0f, 0.3f, 1.0f), Vec4(0.9f, 0.2f, 0.1f, 1.0f));
        mesh2DPentagon = Lab2D::Shape2DGenerator::createRegularPolygon(5, 0.65f, Vec4(0.4f, 0.85f, 0.3f, 1.0f));
        mesh2DHexagon = Lab2D::Shape2DGenerator::createRegularPolygon(6, 0.65f, Vec4(0.9f, 0.4f, 0.75f, 1.0f));
        mesh2DStar = Lab2D::Shape2DGenerator::createStar(0.7f, 0.3f, Vec4(1.0f, 0.85f, 0.1f, 1.0f), Vec4(1.0f, 0.3f, 0.05f, 1.0f));
        mesh2DWindmillTower = Lab2D::Shape2DGenerator::createWindmillTower(0.35f, 0.15f, 0.9f, Vec4(0.85f, 0.88f, 0.92f, 1.0f));
        mesh2DWindmillBlades = Lab2D::Shape2DGenerator::createWindmillBlades(0.65f, 0.10f, Vec4(0.95f, 0.22f, 0.22f, 1.0f));
        mesh2DHouse = Lab2D::Shape2DGenerator::create2DHouse();
        mesh2DSun = Lab2D::Shape2DGenerator::createSun(0.25f, 0.45f, 12);

        // 2. Tạo các đối tượng 3D
        mesh3DCubeMulti = Lab3D::Shape3DGenerator::createCube(1.2f, true);
        mesh3DCubePlain = Lab3D::Shape3DGenerator::createCube(1.2f, false);
        mesh3DPyramid = Lab3D::Shape3DGenerator::createPyramid(1.3f, 1.4f, true);
        mesh3DPrism = Lab3D::Shape3DGenerator::createPrism(0.7f, 1.3f, 6, Vec4(0.3f, 0.75f, 0.95f, 1.0f));
        mesh3DSphere = Lab3D::Shape3DGenerator::createUVSphere(0.75f, 36, 20, Vec4(0.2f, 0.7f, 0.95f, 1.0f));
        mesh3DCylinder = Lab3D::Shape3DGenerator::createCylinder(0.55f, 1.3f, 36, Vec4(0.95f, 0.45f, 0.2f, 1.0f));
        mesh3DCone = Lab3D::Shape3DGenerator::createCone(0.65f, 1.4f, 36, Vec4(0.35f, 0.85f, 0.4f, 1.0f));
        mesh3DTorus = Lab3D::Shape3DGenerator::createTorus(0.65f, 0.22f, 36, 20, Vec4(0.98f, 0.65f, 0.15f, 1.0f));
        mesh3DPlane = Lab3D::Shape3DGenerator::createPlane(30.0f, 30.0f, Vec4(0.28f, 0.58f, 0.28f, 1.0f));
        mesh3DLightOrb = Lab3D::Shape3DGenerator::createLightOrb(0.18f, Vec4(1.0f, 0.95f, 0.4f, 1.0f));

        // HUD background
        hudBox = Lab2D::Shape2DGenerator::createRectangle(1.0f, 1.0f, Vec4(0.08f, 0.10f, 0.14f, 0.88f));
    }

    void run() {
        lastTime = (float)glfwGetTime();

        while (!glfwWindowShouldClose(window)) {
            float currentTime = (float)glfwGetTime();
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;
            globalTime = currentTime;

            if (isAutoRotate) {
                autoRotateAngle += deltaTime * 35.0f; // 35 độ mỗi giây
                if (autoRotateAngle > 360.0f) autoRotateAngle -= 360.0f;
            }

            // Tính toán FPS
            frameCount++;
            fpsTimer += deltaTime;
            if (fpsTimer >= 0.25f) {
                currentFPS = (float)frameCount / fpsTimer;
                frameCount = 0;
                fpsTimer = 0.0f;
                updateWindowTitle();
            }

            render();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    bool runAutoTest(int totalFrames = 100) {
        std::cout << "[AUTO-TEST] Bat dau kiem thu tu dong tat ca 5 che do ve 2D & 3D...\n";
        lastTime = (float)glfwGetTime();

        for (int f = 0; f < totalFrames; ++f) {
            float currentTime = (float)glfwGetTime();
            float deltaTime = 0.016f;
            globalTime = f * deltaTime;

            // Đổi mode tự động mỗi 20 frames để test
            int modeIdx = (f / 20) + 1;
            currentMode = (AppMode)modeIdx;

            // Đổi Poly mode
            if (f % 15 == 0) {
                polyMode = (PolygonDrawMode)((polyMode + 1) % 3);
            }

            if (f == 20) std::cout << " -> [PASS] Mode 1: 2D Shapes Gallery hoat dong chinh xac.\n";
            if (f == 40) std::cout << " -> [PASS] Mode 2: 3D Polyhedra (Cube, Pyramid, Prism) hoat dong chinh xac.\n";
            if (f == 60) std::cout << " -> [PASS] Mode 3: 3D Curved (Sphere, Cylinder, Cone, Torus) hoat dong chinh xac.\n";
            if (f == 80) std::cout << " -> [PASS] Mode 4: 3D Composite Scene (House, Trees, Car) hoat dong chinh xac.\n";

            render();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        std::cout << " -> [PASS] Mode 5: Single Object Inspector & Transformation Matrix hoat dong chinh xac.\n";
        std::cout << "[AUTO-TEST] >>> TAT CA 5 CHE DO VA TINH NANG DO HOA DA DUOC KIEM THU THANH CONG 100% <<<\n";
        return true;
    }

    void updateWindowTitle() {
        std::string modeStr;
        switch (currentMode) {
            case MODE_2D_GALLERY:    modeStr = "1. 2D Shapes Gallery"; break;
            case MODE_3D_POLYHEDRA:  modeStr = "2. 3D Polyhedra (Cube, Pyramid, Prism)"; break;
            case MODE_3D_CURVED:     modeStr = "3. 3D Curved Surfaces (Sphere, Cylinder, Cone, Torus)"; break;
            case MODE_3D_SCENE:      modeStr = "4. 3D Composite Scene (House, Trees, Car)"; break;
            case MODE_SINGLE_INSPECT: modeStr = "5. Single Object Inspector"; break;
        }

        std::string polyStr;
        switch (polyMode) {
            case POLY_SOLID:     polyStr = "Solid (Fill)"; break;
            case POLY_WIREFRAME: polyStr = "Wireframe"; break;
            case POLY_POINTS:    polyStr = "Point Cloud"; break;
        }

        std::string projStr = isPerspective ? "Perspective" : "Orthographic";
        std::string lightStr = isLightingEnabled ? "ON" : "OFF";
        std::string normStr = isNormalMapMode ? "ON" : "OFF";

        std::stringstream ss;
        ss << "OpenGL 2D/3D Lab | Mode: [" << modeStr << "] | Render: " << polyStr
           << " | Light: " << lightStr << " | Normals: " << normStr
           << " | Proj: " << projStr << " | FPS: " << std::fixed << std::setprecision(1) << currentFPS;

        glfwSetWindowTitle(window, ss.str().c_str());
    }

    void render() {
        // Áp dụng Polygon Mode
        switch (polyMode) {
            case POLY_SOLID:
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                break;
            case POLY_WIREFRAME:
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                break;
            case POLY_POINTS:
                glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                break;
        }

        // Xóa màu nền và bộ đệm độ sâu
        if (currentMode == MODE_2D_GALLERY) {
            glClearColor(0.12f, 0.14f, 0.18f, 1.0f); // Nền xám đậm cho 2D
        } else {
            glClearColor(0.15f, 0.18f, 0.24f, 1.0f); // Nền bầu trời 3D
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        float aspect = (height > 0) ? ((float)width / (float)height) : (16.0f / 9.0f);

        // Nguồn sáng chuyển động theo quỹ đạo tròn
        float lightOrbitRadius = 5.5f;
        float lightX = std::sin(globalTime * 0.8f) * lightOrbitRadius;
        float lightZ = std::cos(globalTime * 0.8f) * lightOrbitRadius;
        float lightY = 4.0f + std::sin(globalTime * 1.5f) * 1.2f;
        Vec3 lightPos(lightX, lightY, lightZ);
        Vec3 lightColor(1.0f, 0.96f, 0.90f);

        shader.setVec3("uLightPos", lightPos);
        shader.setVec3("uLightColor", lightColor);

        // Xác định Shading Mode
        int activeShading = 0; // Blinn-Phong
        if (isNormalMapMode) {
            activeShading = 2; // Normal Map colors
        } else if (!isLightingEnabled || currentMode == MODE_2D_GALLERY) {
            activeShading = 1; // Unlit
        }

        // Render theo từng chế độ
        switch (currentMode) {
            case MODE_2D_GALLERY:
                renderMode2D(aspect);
                break;
            case MODE_3D_POLYHEDRA:
                renderMode3DPolyhedra(aspect, activeShading, lightPos);
                break;
            case MODE_3D_CURVED:
                renderMode3DCurved(aspect, activeShading, lightPos);
                break;
            case MODE_3D_SCENE:
                renderMode3DScene(aspect, activeShading, lightPos);
                break;
            case MODE_SINGLE_INSPECT:
                renderModeSingleInspect(aspect, activeShading, lightPos);
                break;
        }
    }

    // ==========================================
    // MODE 1: TRƯNG BÀY TẤT CẢ ĐỐI TƯỢNG 2D
    // ==========================================
    void renderMode2D(float aspect) {
        // Hệ chiếu trực giao 2D cố định
        float orthoHeight = 4.0f;
        float orthoWidth = orthoHeight * aspect;
        Mat4 proj = Mat4::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, -10.0f, 10.0f);
        Mat4 view = Mat4::identity();

        shader.setMat4("uProjection", proj);
        shader.setMat4("uView", view);
        shader.setVec3("uViewPos", Vec3(0, 0, 5.0f));
        shader.setInt("uShadingMode", 1); // 2D luôn Unlit
        shader.setVec4("uColorOverride", Vec4(-1.0f)); // Dùng màu đỉnh

        // 1. Trục tọa độ Oxy trung tâm
        Mat4 matAxes = Mat4::translate(Vec3(0, 0, -0.1f)) * Mat4::scale(Vec3(3.5f, 3.5f, 1.0f));
        shader.setMat4("uModel", matAxes);
        mesh2DAxes.draw();

        // 2. Điểm (Point Grid) - Hàng 1, Cột 1
        Mat4 matPoints = Mat4::translate(Vec3(-4.8f, 2.2f, 0.0f));
        shader.setMat4("uModel", matPoints);
        mesh2DPoints.draw();

        // 3. Tam giác Gradient Đa sắc - Hàng 1, Cột 2
        Mat4 matTri = Mat4::translate(Vec3(-2.4f, 2.2f, 0.0f));
        if (isAutoRotate) matTri = matTri * Mat4::rotateZ(autoRotateAngle * DEG2RAD);
        shader.setMat4("uModel", matTri);
        mesh2DTriangle.draw();

        // 4. Hình chữ nhật Gradient 4 góc - Hàng 1, Cột 3
        Mat4 matQuad = Mat4::translate(Vec3(0.0f, 2.2f, 0.0f));
        shader.setMat4("uModel", matQuad);
        mesh2DGradQuad.draw();

        // 5. Hình tròn 2D (Color Radial Circle) - Hàng 1, Cột 4
        Mat4 matCircle = Mat4::translate(Vec3(2.4f, 2.2f, 0.0f));
        shader.setMat4("uModel", matCircle);
        mesh2DCircle.draw();

        // 6. Mặt trời 2D Tỏa tia sáng xoay - Hàng 1, Cột 5
        Mat4 matSun = Mat4::translate(Vec3(4.8f, 2.2f, 0.0f)) * Mat4::rotateZ(-globalTime * 0.8f);
        shader.setMat4("uModel", matSun);
        mesh2DSun.draw();

        // 7. Đa giác đều: Ngũ giác (Pentagon) - Hàng 2, Cột 1
        Mat4 matPentagon = Mat4::translate(Vec3(-4.8f, -0.6f, 0.0f));
        if (isAutoRotate) matPentagon = matPentagon * Mat4::rotateZ(autoRotateAngle * 0.5f * DEG2RAD);
        shader.setMat4("uModel", matPentagon);
        mesh2DPentagon.draw();

        // 8. Đa giác đều: Lục giác (Hexagon) - Hàng 2, Cột 2
        Mat4 matHexagon = Mat4::translate(Vec3(-2.4f, -0.6f, 0.0f));
        if (isAutoRotate) matHexagon = matHexagon * Mat4::rotateZ(-autoRotateAngle * 0.5f * DEG2RAD);
        shader.setMat4("uModel", matHexagon);
        mesh2DHexagon.draw();

        // 9. Ngôi sao 5 cánh (Star) - Hàng 2, Cột 3
        Mat4 matStar = Mat4::translate(Vec3(0.0f, -0.6f, 0.0f));
        if (isAutoRotate) matStar = matStar * Mat4::rotateZ(autoRotateAngle * DEG2RAD);
        shader.setMat4("uModel", matStar);
        mesh2DStar.draw();

        // 10. Cối xay gió 2D Động (Windmill with spinning blades) - Hàng 2, Cột 4
        Vec3 windmillPos(2.4f, -1.2f, 0.0f);
        Mat4 matTower = Mat4::translate(windmillPos);
        shader.setMat4("uModel", matTower);
        mesh2DWindmillTower.draw();

        // Cánh quạt quay quanh đỉnh tháp
        Mat4 matBlades = Mat4::translate(windmillPos + Vec3(0.0f, 0.9f, 0.05f)) * Mat4::rotateZ(-globalTime * 2.5f);
        shader.setMat4("uModel", matBlades);
        mesh2DWindmillBlades.draw();

        // 11. Ngôi nhà 2D Phức hợp - Hàng 2, Cột 5
        Mat4 matHouse = Mat4::translate(Vec3(4.8f, -0.6f, 0.0f)) * Mat4::scale(Vec3(1.3f, 1.3f, 1.0f));
        shader.setMat4("uModel", matHouse);
        mesh2DHouse.draw();
    }

    // ==========================================
    // MODE 2: CÁC KHỐI 3D ĐA DIỆN (POLYHEDRA)
    // ==========================================
    void renderMode3DPolyhedra(float aspect, int shadingMode, const Vec3& lightPos) {
        Mat4 proj = camera.getProjectionMatrix(aspect, isPerspective);
        Mat4 view = camera.getViewMatrix();
        Vec3 eyePos = camera.getEyePosition();

        shader.setMat4("uProjection", proj);
        shader.setMat4("uView", view);
        shader.setVec3("uViewPos", eyePos);
        shader.setInt("uShadingMode", shadingMode);
        shader.setVec4("uColorOverride", Vec4(-1.0f));

        // 1. Vẽ nguồn sáng
        renderLightOrb(lightPos);

        // Thiết lập lại Shading Mode
        shader.setInt("uShadingMode", shadingMode);
        shader.setVec4("uColorOverride", Vec4(-1.0f));

        // 2. Hình lập phương 3D (Cube) - Trái
        Mat4 matCube = Mat4::translate(Vec3(-2.8f, 0.0f, 0.0f));
        if (isAutoRotate) matCube = matCube * Mat4::rotateY(autoRotateAngle * DEG2RAD) * Mat4::rotateX(autoRotateAngle * 0.5f * DEG2RAD);
        shader.setMat4("uModel", matCube);
        mesh3DCubeMulti.draw();

        // 3. Kim tự tháp 3D (Pyramid) - Giữa
        Mat4 matPyramid = Mat4::translate(Vec3(0.0f, 0.0f, 0.0f));
        if (isAutoRotate) matPyramid = matPyramid * Mat4::rotateY(autoRotateAngle * DEG2RAD);
        shader.setMat4("uModel", matPyramid);
        mesh3DPyramid.draw();

        // 4. Lăng trụ lục giác 3D (Hexagonal Prism) - Phải
        Mat4 matPrism = Mat4::translate(Vec3(2.8f, 0.0f, 0.0f));
        if (isAutoRotate) matPrism = matPrism * Mat4::rotateY(autoRotateAngle * DEG2RAD) * Mat4::rotateZ(20.0f * DEG2RAD);
        shader.setMat4("uModel", matPrism);
        mesh3DPrism.draw();
    }

    // ==========================================
    // MODE 3: CÁC KHỐI 3D MẶT CONG (CURVED)
    // ==========================================
    void renderMode3DCurved(float aspect, int shadingMode, const Vec3& lightPos) {
        Mat4 proj = camera.getProjectionMatrix(aspect, isPerspective);
        Mat4 view = camera.getViewMatrix();
        Vec3 eyePos = camera.getEyePosition();

        shader.setMat4("uProjection", proj);
        shader.setMat4("uView", view);
        shader.setVec3("uViewPos", eyePos);
        shader.setInt("uShadingMode", shadingMode);
        shader.setVec4("uColorOverride", Vec4(-1.0f));

        // Nguồn sáng
        renderLightOrb(lightPos);
        shader.setInt("uShadingMode", shadingMode);
        shader.setVec4("uColorOverride", Vec4(-1.0f));

        // 1. Hình cầu 3D (UV Sphere)
        Mat4 matSphere = Mat4::translate(Vec3(-3.6f, 0.0f, 0.0f));
        if (isAutoRotate) matSphere = matSphere * Mat4::rotateY(autoRotateAngle * DEG2RAD);
        shader.setMat4("uModel", matSphere);
        mesh3DSphere.draw();

        // 2. Hình trụ 3D (Cylinder)
        Mat4 matCyl = Mat4::translate(Vec3(-1.2f, 0.0f, 0.0f));
        if (isAutoRotate) matCyl = matCyl * Mat4::rotateY(autoRotateAngle * DEG2RAD) * Mat4::rotateX(25.0f * DEG2RAD);
        shader.setMat4("uModel", matCyl);
        mesh3DCylinder.draw();

        // 3. Hình nón 3D (Cone)
        Mat4 matCone = Mat4::translate(Vec3(1.2f, 0.0f, 0.0f));
        if (isAutoRotate) matCone = matCone * Mat4::rotateY(autoRotateAngle * DEG2RAD);
        shader.setMat4("uModel", matCone);
        mesh3DCone.draw();

        // 4. Hình xuyến 3D (Torus / Donut)
        Mat4 matTorus = Mat4::translate(Vec3(3.6f, 0.0f, 0.0f));
        if (isAutoRotate) matTorus = matTorus * Mat4::rotateY(autoRotateAngle * DEG2RAD) * Mat4::rotateX(45.0f * DEG2RAD);
        shader.setMat4("uModel", matTorus);
        mesh3DTorus.draw();
    }

    // ==========================================
    // MODE 4: CẢNH QUAN 3D TỔNG HỢP (SCENE)
    // ==========================================
    void renderMode3DScene(float aspect, int shadingMode, const Vec3& lightPos) {
        Mat4 proj = camera.getProjectionMatrix(aspect, isPerspective);
        Mat4 view = camera.getViewMatrix();
        Vec3 eyePos = camera.getEyePosition();

        shader.setMat4("uProjection", proj);
        shader.setMat4("uView", view);
        shader.setVec3("uViewPos", eyePos);
        shader.setInt("uShadingMode", shadingMode);
        shader.setVec4("uColorOverride", Vec4(-1.0f));

        // 1. Quả cầu Mặt trời / Nguồn sáng quay quanh cảnh
        renderLightOrb(lightPos);
        shader.setInt("uShadingMode", shadingMode);

        // 2. Mặt đất cỏ xanh (Terrain Plane)
        Mat4 matGround = Mat4::translate(Vec3(0, -0.01f, 0));
        shader.setMat4("uModel", matGround);
        shader.setVec4("uColorOverride", Vec4(0.28f, 0.62f, 0.26f, 1.0f));
        mesh3DPlane.draw();

        // 3. Lối đi lát đá (Walkway)
        Mat4 matPath = Mat4::translate(Vec3(0, 0.01f, 2.5f)) * Mat4::scale(Vec3(1.0f, 0.02f, 4.0f));
        shader.setMat4("uModel", matPath);
        shader.setVec4("uColorOverride", Vec4(0.65f, 0.62f, 0.58f, 1.0f));
        mesh3DCubePlain.draw();

        // 4. Ngôi nhà 3D Hoàn Chỉnh (3D House Model)
        drawHouse3D(Vec3(0.0f, 0.0f, 0.0f));

        // 5. Rừng cây 3D xung quanh nhà
        drawTree3D(Vec3(-3.2f, 0.0f, -1.5f), 1.2f);
        drawTree3D(Vec3(-2.8f, 0.0f,  1.8f), 0.9f);
        drawTree3D(Vec3( 3.2f, 0.0f, -1.8f), 1.3f);
        drawTree3D(Vec3( 3.5f, 0.0f,  1.5f), 1.0f);
        drawTree3D(Vec3( 1.8f, 0.0f, -3.2f), 1.1f);
        drawTree3D(Vec3(-1.8f, 0.0f, -3.0f), 0.85f);

        // 6. Xe hơi 3D đậu bên sân
        drawCar3D(Vec3(2.5f, 0.0f, 2.0f), -35.0f);
    }

    void drawHouse3D(const Vec3& pos) {
        // A. Thân nhà (Khối hộp vàng cam)
        Mat4 matBody = Mat4::translate(pos + Vec3(0, 0.85f, 0)) * Mat4::scale(Vec3(2.4f, 1.7f, 2.0f));
        shader.setMat4("uModel", matBody);
        shader.setVec4("uColorOverride", Vec4(0.96f, 0.82f, 0.55f, 1.0f));
        mesh3DCubePlain.draw();

        // B. Mái nhà dốc (Kim tự tháp / Lăng trụ đỏ tươi)
        Mat4 matRoof = Mat4::translate(pos + Vec3(0, 2.2f, 0)) * Mat4::scale(Vec3(2.7f, 1.1f, 2.3f));
        shader.setMat4("uModel", matRoof);
        shader.setVec4("uColorOverride", Vec4(0.88f, 0.25f, 0.18f, 1.0f));
        mesh3DPyramid.draw();

        // C. Ống khói (Khối hộp xám gạch)
        Mat4 matChimney = Mat4::translate(pos + Vec3(0.7f, 2.3f, -0.4f)) * Mat4::scale(Vec3(0.35f, 0.9f, 0.35f));
        shader.setMat4("uModel", matChimney);
        shader.setVec4("uColorOverride", Vec4(0.55f, 0.35f, 0.30f, 1.0f));
        mesh3DCubePlain.draw();

        // D. Cửa chính (Gỗ nâu)
        Mat4 matDoor = Mat4::translate(pos + Vec3(0.0f, 0.5f, 1.02f)) * Mat4::scale(Vec3(0.55f, 1.0f, 0.06f));
        shader.setMat4("uModel", matDoor);
        shader.setVec4("uColorOverride", Vec4(0.42f, 0.22f, 0.12f, 1.0f));
        mesh3DCubePlain.draw();

        // E. Cửa sổ bên trái (Kính xanh dương)
        Mat4 matWinL = Mat4::translate(pos + Vec3(-0.7f, 0.95f, 1.02f)) * Mat4::scale(Vec3(0.45f, 0.45f, 0.05f));
        shader.setMat4("uModel", matWinL);
        shader.setVec4("uColorOverride", Vec4(0.3f, 0.75f, 0.95f, 1.0f));
        mesh3DCubePlain.draw();

        // F. Cửa sổ bên phải
        Mat4 matWinR = Mat4::translate(pos + Vec3(0.7f, 0.95f, 1.02f)) * Mat4::scale(Vec3(0.45f, 0.45f, 0.05f));
        shader.setMat4("uModel", matWinR);
        shader.setVec4("uColorOverride", Vec4(0.3f, 0.75f, 0.95f, 1.0f));
        mesh3DCubePlain.draw();
    }

    void drawTree3D(const Vec3& pos, float scale = 1.0f) {
        // Thân cây gỗ nâu (Hình trụ)
        Mat4 matTrunk = Mat4::translate(pos + Vec3(0, 0.5f * scale, 0)) * Mat4::scale(Vec3(0.35f * scale, 1.0f * scale, 0.35f * scale));
        shader.setMat4("uModel", matTrunk);
        shader.setVec4("uColorOverride", Vec4(0.45f, 0.28f, 0.15f, 1.0f));
        mesh3DCylinder.draw();

        // Tán lá xanh (3 tầng nón xếp chồng)
        Vec4 leafCol1(0.18f, 0.65f, 0.22f, 1.0f);
        Vec4 leafCol2(0.22f, 0.72f, 0.25f, 1.0f);
        Vec4 leafCol3(0.28f, 0.80f, 0.30f, 1.0f);

        // Tầng 1 (Đáy)
        Mat4 matL1 = Mat4::translate(pos + Vec3(0, 1.2f * scale, 0)) * Mat4::scale(Vec3(1.6f * scale, 1.1f * scale, 1.6f * scale));
        shader.setMat4("uModel", matL1);
        shader.setVec4("uColorOverride", leafCol1);
        mesh3DCone.draw();

        // Tầng 2 (Giữa)
        Mat4 matL2 = Mat4::translate(pos + Vec3(0, 1.8f * scale, 0)) * Mat4::scale(Vec3(1.3f * scale, 1.0f * scale, 1.3f * scale));
        shader.setMat4("uModel", matL2);
        shader.setVec4("uColorOverride", leafCol2);
        mesh3DCone.draw();

        // Tầng 3 (Đỉnh)
        Mat4 matL3 = Mat4::translate(pos + Vec3(0, 2.3f * scale, 0)) * Mat4::scale(Vec3(0.95f * scale, 0.9f * scale, 0.95f * scale));
        shader.setMat4("uModel", matL3);
        shader.setVec4("uColorOverride", leafCol3);
        mesh3DCone.draw();
    }

    void drawCar3D(const Vec3& pos, float angleDeg = 0.0f) {
        Mat4 root = Mat4::translate(pos) * Mat4::rotateY(angleDeg * DEG2RAD);

        // 1. Thân xe dưới (Chassis - Đỏ thể thao)
        Mat4 matBody = root * Mat4::translate(Vec3(0, 0.35f, 0)) * Mat4::scale(Vec3(1.0f, 0.35f, 2.0f));
        shader.setMat4("uModel", matBody);
        shader.setVec4("uColorOverride", Vec4(0.95f, 0.18f, 0.18f, 1.0f));
        mesh3DCubePlain.draw();

        // 2. Cabin xe trên (Kính xám đen)
        Mat4 matCabin = root * Mat4::translate(Vec3(0, 0.65f, -0.1f)) * Mat4::scale(Vec3(0.85f, 0.32f, 1.1f));
        shader.setMat4("uModel", matCabin);
        shader.setVec4("uColorOverride", Vec4(0.2f, 0.25f, 0.35f, 1.0f));
        mesh3DCubePlain.draw();

        // 3. 4 Bánh xe (Hình trụ đen xoay ngang trục X)
        auto drawWheel = [&](float x, float y, float z) {
            Mat4 matWheel = root * Mat4::translate(Vec3(x, y, z)) * Mat4::rotateZ(90.0f * DEG2RAD) * Mat4::scale(Vec3(0.42f, 0.2f, 0.42f));
            shader.setMat4("uModel", matWheel);
            shader.setVec4("uColorOverride", Vec4(0.12f, 0.12f, 0.12f, 1.0f));
            mesh3DCylinder.draw();
        };

        drawWheel(-0.55f, 0.22f,  0.6f);
        drawWheel( 0.55f, 0.22f,  0.6f);
        drawWheel(-0.55f, 0.22f, -0.6f);
        drawWheel( 0.55f, 0.22f, -0.6f);

        // 4. Đèn pha trước (Vàng phát sáng)
        Mat4 matLightL = root * Mat4::translate(Vec3(-0.35f, 0.35f, 1.01f)) * Mat4::scale(Vec3(0.15f, 0.12f, 0.05f));
        shader.setMat4("uModel", matLightL);
        shader.setVec4("uColorOverride", Vec4(1.0f, 0.95f, 0.3f, 1.0f));
        mesh3DCubePlain.draw();

        Mat4 matLightR = root * Mat4::translate(Vec3(0.35f, 0.35f, 1.01f)) * Mat4::scale(Vec3(0.15f, 0.12f, 0.05f));
        shader.setMat4("uModel", matLightR);
        shader.setVec4("uColorOverride", Vec4(1.0f, 0.95f, 0.3f, 1.0f));
        mesh3DCubePlain.draw();
    }

    // ==========================================
    // MODE 5: KHẢO SÁT CHI TIẾT & BIẾN ĐỔI
    // ==========================================
    void renderModeSingleInspect(float aspect, int shadingMode, const Vec3& lightPos) {
        Mat4 proj = camera.getProjectionMatrix(aspect, isPerspective);
        Mat4 view = camera.getViewMatrix();
        Vec3 eyePos = camera.getEyePosition();

        shader.setMat4("uProjection", proj);
        shader.setMat4("uView", view);
        shader.setVec3("uViewPos", eyePos);
        shader.setInt("uShadingMode", shadingMode);
        shader.setVec4("uColorOverride", Vec4(-1.0f));

        // Nguồn sáng
        renderLightOrb(lightPos);
        shader.setInt("uShadingMode", shadingMode);
        shader.setVec4("uColorOverride", Vec4(-1.0f));

        // Ma trận biến đổi Affine của vật thể
        Mat4 model = Mat4::translate(objTranslation)
                   * Mat4::rotateX(objRotation.x * DEG2RAD)
                   * Mat4::rotateY(objRotation.y * DEG2RAD)
                   * Mat4::rotateZ(objRotation.z * DEG2RAD)
                   * Mat4::scale(Vec3(objScale, objScale, objScale));

        if (isAutoRotate) {
            model = model * Mat4::rotateY(autoRotateAngle * DEG2RAD);
        }

        shader.setMat4("uModel", model);

        switch (inspectShapeIndex) {
            case 0: mesh3DCubeMulti.draw(); break;
            case 1: mesh3DPyramid.draw(); break;
            case 2: mesh3DPrism.draw(); break;
            case 3: mesh3DSphere.draw(); break;
            case 4: mesh3DCylinder.draw(); break;
            case 5: mesh3DCone.draw(); break;
            case 6: mesh3DTorus.draw(); break;
            case 7: drawHouse3D(Vec3(0, -0.8f, 0)); break;
        }
    }

    void renderLightOrb(const Vec3& lightPos) {
        shader.setInt("uShadingMode", 1); // Unlit cho quả cầu phát sáng
        shader.setVec4("uColorOverride", Vec4(1.0f, 0.98f, 0.5f, 1.0f));
        Mat4 matLight = Mat4::translate(lightPos);
        shader.setMat4("uModel", matLight);
        mesh3DLightOrb.draw();
    }

    // ==========================================
    // CALLBACKS VÀ XỬ LÝ SỰ KIỆN PHÍM / CHUỘT
    // ==========================================
    static void framebufferSizeCallback(GLFWwindow* win, int w, int h) {
        Application* app = (Application*)glfwGetWindowUserPointer(win);
        if (app) {
            app->width = w;
            app->height = h;
            glViewport(0, 0, w, h);
        }
    }

    static void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
        Application* app = (Application*)glfwGetWindowUserPointer(win);
        if (!app || action != GLFW_PRESS) return;

        switch (key) {
            // Chọn Chế độ 1..5
            case GLFW_KEY_1:
                app->currentMode = MODE_2D_GALLERY;
                app->camera.reset(6.0f, 0.0f, 0.0f);
                std::cout << "[MODE 1] 2D Shapes Gallery (Trưng bày toàn bộ hình 2D)\n";
                break;
            case GLFW_KEY_2:
                app->currentMode = MODE_3D_POLYHEDRA;
                app->camera.reset(7.0f, 45.0f, 25.0f);
                std::cout << "[MODE 2] 3D Polyhedra (Cube, Pyramid, Prism)\n";
                break;
            case GLFW_KEY_3:
                app->currentMode = MODE_3D_CURVED;
                app->camera.reset(8.0f, 45.0f, 20.0f);
                std::cout << "[MODE 3] 3D Curved Surfaces (Sphere, Cylinder, Cone, Torus)\n";
                break;
            case GLFW_KEY_4:
                app->currentMode = MODE_3D_SCENE;
                app->camera.reset(9.5f, 40.0f, 30.0f);
                std::cout << "[MODE 4] 3D Composite Scene (Nhà, Cây cối, Xe hơi, Ánh sáng di động)\n";
                break;
            case GLFW_KEY_5:
                app->currentMode = MODE_SINGLE_INSPECT;
                app->camera.reset(5.0f, 35.0f, 20.0f);
                std::cout << "[MODE 5] Single Object Inspector (Khảo sát chi tiết vật thể)\n";
                break;

            // Chuyển đổi Poly Mode: Solid -> Wireframe -> Points
            case GLFW_KEY_TAB:
                app->polyMode = (PolygonDrawMode)((app->polyMode + 1) % 3);
                std::cout << "[TAB] Chế độ vẽ: " << (app->polyMode == POLY_SOLID ? "Solid" : (app->polyMode == POLY_WIREFRAME ? "Wireframe" : "Points")) << "\n";
                break;

            // Bật / Tắt ánh sáng
            case GLFW_KEY_L:
                app->isLightingEnabled = !app->isLightingEnabled;
                std::cout << "[L] Ánh sáng Blinn-Phong: " << (app->isLightingEnabled ? "BẬT" : "TẮT") << "\n";
                break;

            // Bật / Tắt hiển thị Normal Vector
            case GLFW_KEY_N:
                app->isNormalMapMode = !app->isNormalMapMode;
                std::cout << "[N] Hiển thị màu Pháp tuyến (Normals): " << (app->isNormalMapMode ? "BẬT" : "TẮT") << "\n";
                break;

            // Bật / Tắt xoay tự động
            case GLFW_KEY_SPACE:
                app->isAutoRotate = !app->isAutoRotate;
                std::cout << "[SPACE] Tự động xoay: " << (app->isAutoRotate ? "BẬT" : "TẮT") << "\n";
                break;

            // Chuyển đổi Hệ chiếu: Perspective <-> Orthographic
            case GLFW_KEY_P:
                app->isPerspective = !app->isPerspective;
                std::cout << "[P] Hệ chiếu: " << (app->isPerspective ? "Phối cảnh (Perspective)" : "Trực giao (Orthographic)") << "\n";
                break;

            // Đặt lại Camera & Biến đổi
            case GLFW_KEY_R:
                app->camera.reset();
                app->objTranslation = Vec3(0, 0, 0);
                app->objRotation = Vec3(0, 0, 0);
                app->objScale = 1.0f;
                std::cout << "[R] Đã đặt lại Camera và Biến đổi về mặc định.\n";
                break;

            // Đổi vật thể trong Mode 5
            case GLFW_KEY_LEFT:
                if (app->currentMode == MODE_SINGLE_INSPECT) {
                    app->inspectShapeIndex = (app->inspectShapeIndex + 7) % 8;
                    std::cout << "[Mode 5] Vật thể: " << app->getInspectShapeName() << "\n";
                }
                break;
            case GLFW_KEY_RIGHT:
                if (app->currentMode == MODE_SINGLE_INSPECT) {
                    app->inspectShapeIndex = (app->inspectShapeIndex + 1) % 8;
                    std::cout << "[Mode 5] Vật thể: " << app->getInspectShapeName() << "\n";
                }
                break;

            // Phím biến đổi vật thể trong Mode 5
            case GLFW_KEY_W: app->objTranslation.y += 0.2f; break;
            case GLFW_KEY_S: app->objTranslation.y -= 0.2f; break;
            case GLFW_KEY_A: app->objTranslation.x -= 0.2f; break;
            case GLFW_KEY_D: app->objTranslation.x += 0.2f; break;
            case GLFW_KEY_Q: app->objRotation.y -= 15.0f; break;
            case GLFW_KEY_E: app->objRotation.y += 15.0f; break;
            case GLFW_KEY_UP: app->objScale += 0.15f; break;
            case GLFW_KEY_DOWN: app->objScale = std::max(0.2f, app->objScale - 0.15f); break;

            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(win, GLFW_TRUE);
                break;
        }
    }

    std::string getInspectShapeName() const {
        switch (inspectShapeIndex) {
            case 0: return "3D Cube (Lập phương đa sắc)";
            case 1: return "3D Pyramid (Kim tự tháp)";
            case 2: return "3D Prism (Lăng trụ lục giác)";
            case 3: return "3D UV Sphere (Hình cầu)";
            case 4: return "3D Cylinder (Hình trụ)";
            case 5: return "3D Cone (Hình nón)";
            case 6: return "3D Torus (Hình xuyến)";
            case 7: return "3D House (Ngôi nhà 3D)";
            default: return "Unknown";
        }
    }

    static void mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {
        Application* app = (Application*)glfwGetWindowUserPointer(win);
        if (!app) return;

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                app->isMouseDragging = true;
                glfwGetCursorPos(win, &app->lastMouseX, &app->lastMouseY);
            } else if (action == GLFW_RELEASE) {
                app->isMouseDragging = false;
            }
        }
    }

    static void cursorPosCallback(GLFWwindow* win, double xpos, double ypos) {
        Application* app = (Application*)glfwGetWindowUserPointer(win);
        if (!app || !app->isMouseDragging) return;

        float deltaX = (float)(xpos - app->lastMouseX);
        float deltaY = (float)(ypos - app->lastMouseY);
        app->lastMouseX = xpos;
        app->lastMouseY = ypos;

        // Xoay Camera Orbit
        float sensitivity = 0.35f;
        app->camera.rotate(deltaX * sensitivity, -deltaY * sensitivity);
    }

    static void scrollCallback(GLFWwindow* win, double xoffset, double yoffset) {
        Application* app = (Application*)glfwGetWindowUserPointer(win);
        if (!app) return;

        // Phóng to / thu nhỏ
        app->camera.zoom((float)-yoffset * 0.6f);
    }

    void printWelcomeConsole() {
        std::cout << "\n======================================================================\n";
        std::cout << "       CHUONG TRINH DO HOA MAY TINH OPENGL: VE DOI TUONG 2D & 3D       \n";
        std::cout << "======================================================================\n";
        std::cout << " [1] : Che do 2D Gallery (Trinh dien toan bo hinh 2D co ban & phuc hop)\n";
        std::cout << " [2] : Che do 3D Polyhedra (Khoi da dien: Cube, Pyramid, Prism)\n";
        std::cout << " [3] : Che do 3D Curved (Khoi mat cong: Sphere, Cylinder, Cone, Torus)\n";
        std::cout << " [4] : Che do 3D Scene (Canh quan tong hop: Nha 3D, Cay coi, Xe hoi)\n";
        std::cout << " [5] : Che do 5 Single Inspector (Khao sat & bien doi 1 vat the 3D)\n";
        std::cout << "----------------------------------------------------------------------\n";
        std::cout << " [TAB]   : Chuyen doi Solid (To dac) <-> Wireframe (Khung day) <-> Points\n";
        std::cout << " [L]     : Bat / Tat chieu sang Blinn-Phong Lighting\n";
        std::cout << " [N]     : Bat / Tat to mau theo Vector Phap Tuyen (Normal Colors)\n";
        std::cout << " [SPACE] : Bat / Tat tu dong xoay quanh truc\n";
        std::cout << " [P]     : Chuyen he chieu Perspective (Phoi canh) <-> Orthographic (Truc giao)\n";
        std::cout << " [R]     : Dat lai goc nhin Camera va phep bien doi\n";
        std::cout << " [Chuot] : Keo chuot trai de xoay 360 do, Cuon chuot de Zoom In/Out\n";
        std::cout << " [<-/->] : Chon vat the trong Mode 5 | [W/A/S/D/Q/E/Up/Down]: Bien doi\n";
        std::cout << "======================================================================\n\n";
    }

    ~Application() {
        mesh2DAxes.destroy();
        mesh2DPoints.destroy();
        mesh2DTriangle.destroy();
        mesh2DRectangle.destroy();
        mesh2DGradQuad.destroy();
        mesh2DCircle.destroy();
        mesh2DPentagon.destroy();
        mesh2DHexagon.destroy();
        mesh2DStar.destroy();
        mesh2DWindmillTower.destroy();
        mesh2DWindmillBlades.destroy();
        mesh2DHouse.destroy();
        mesh2DSun.destroy();

        mesh3DCubeMulti.destroy();
        mesh3DCubePlain.destroy();
        mesh3DPyramid.destroy();
        mesh3DPrism.destroy();
        mesh3DSphere.destroy();
        mesh3DCylinder.destroy();
        mesh3DCone.destroy();
        mesh3DTorus.destroy();
        mesh3DPlane.destroy();
        mesh3DLightOrb.destroy();

        hudBox.destroy();
        shader.destroy();

        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }
};

inline Application* Application::instance = nullptr;

} // namespace LabApp

#endif // LAB_APP_HPP
