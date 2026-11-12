#include <iostream>
#include <string>
#include <limits>
#include "Lab1/Lab1App.hpp"
#include "Lab2/Lab2App.hpp"
#include "Lab3/Lab3App.hpp"
#include "Lab4/Lab4App.hpp"
#include "Lab4/Lab4Transformations.hpp"
#include "Lab2D3D/LabApp.hpp"

int main(int argc, char* argv[]) {
    std::cout << "====================================================================\n";
    std::cout << "   CHUONG TRINH DO HOA MAY TINH OPENGL - LAB 01, 02, 03, 04         \n";
    std::cout << "====================================================================\n";

    int selectedLab = 0; // 0: Menu, 1: Lab 1, 2: Lab 2, 3: Lab 3, 4: Lab 4, 5: Lab 3D, 9: Tests
    bool isTest = false;

    // Phan tich tham so dong lenh
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--test" || arg == "-t") {
            isTest = true;
            selectedLab = 9;
        } else if (arg == "--lab1" || arg == "-1") {
            selectedLab = 1;
        } else if (arg == "--lab2" || arg == "-2") {
            selectedLab = 2;
        } else if (arg == "--lab3" || arg == "-3") {
            selectedLab = 3;
        } else if (arg == "--lab4" || arg == "-4") {
            selectedLab = 4;
        } else if (arg == "--lab2d3d" || arg == "--3d" || arg == "-5") {
            selectedLab = 5;
        }
    }

    // Neu khong co tham so dong lenh, hien thi menu de nguoi dung lua chon
    if (selectedLab == 0) {
        std::cout << "\nCHON CHE DO KHOI CHAY:\n";
        std::cout << "  [1] LAB 01: Bresenham & Midpoint (Line, Circle, Ellipse, Parabol, Hyperbol, Sin/Cos)\n";
        std::cout << "  [2] LAB 02: Xen hinh & To mau (Cohen-Sutherland, Chia nhi phan, Boundary Fill, Scanline)\n";
        std::cout << "  [3] LAB 03: Bien doi 2D (Tinh tien, Ti le voi tam, Quay voi tam, Doi xung y=ax+b)\n";
        std::cout << "  [4] LAB 04: Quan sat vat the 3D (Khung ket noi, Mat da giac, Phep chieu, Theta/Phi, Scale) [MOI]\n";
        std::cout << "  [5] Lab 2D & 3D Phong chieu sang (Gallery)\n";
        std::cout << "  [6] Kiem thu tu dong (Automated Unit Tests cho ca 4 Lab 01 - 04)\n";
        std::cout << "  [0] Thoat chuong trinh\n";
        std::cout << "Nhap lua chon cua ban (1-6, mac dinh [4] cho Lab 4): ";

        std::string input;
        if (std::getline(std::cin, input) && !input.empty()) {
            if (input == "1") selectedLab = 1;
            else if (input == "2") selectedLab = 2;
            else if (input == "3") selectedLab = 3;
            else if (input == "4") selectedLab = 4;
            else if (input == "5") selectedLab = 5;
            else if (input == "6") { selectedLab = 9; isTest = true; }
            else if (input == "0") return 0;
            else selectedLab = 4; // Mặc định chạy Lab 4
        } else {
            selectedLab = 4; // Mặc định chạy Lab 4
        }
    }

    // =========================================================================
    // CHẠY TEST TỰ ĐỘNG
    // =========================================================================
    if (selectedLab == 9 || isTest) {
        std::cout << "\n[TEST] DANG CHAY KIEM THU TU DONG TAT CA THUAT TOAN...\n";
        bool lab1Ok = Lab1::runAlgorithmTests();
        bool lab2Ok = Lab2::runLab2Tests();
        bool lab3Ok = Lab3::runLab3Tests();
        bool lab4Ok = Lab4::runLab4Tests();

        bool allOk = lab1Ok && lab2Ok && lab3Ok && lab4Ok;
        std::cout << "\n=======================================================\n";
        std::cout << " TONG KET KIEM THU TOAN DIEN:\n";
        std::cout << "  - LAB 01 (Bresenham & Midpoint)  : " << (lab1Ok ? "PASSED [OK]" : "FAILED") << "\n";
        std::cout << "  - LAB 02 (Clipping & Polygon Fill): " << (lab2Ok ? "PASSED [OK]" : "FAILED") << "\n";
        std::cout << "  - LAB 03 (2D Transformations)    : " << (lab3Ok ? "PASSED [OK]" : "FAILED") << "\n";
        std::cout << "  - LAB 04 (3D Viewing & Projection): " << (lab4Ok ? "PASSED [OK]" : "FAILED") << "\n";
        std::cout << "=======================================================\n";
        std::cout << " KET QUA CHUNG CUOC: " << (allOk ? "TAT CA 100% THANH CONG!" : "CO LOI XAY RA!") << "\n\n";
        return allOk ? 0 : 1;
    }

    // =========================================================================
    // LAB 04: QUAN SÁT VẬT THỂ 3D (KHUNG KẾT NỐI & MẶT ĐA GIÁC)
    // =========================================================================
    if (selectedLab == 4) {
        std::cout << "[Che do] Khoi chay LAB 04 (Phep quan sat vat the 3D - Khung ket noi & Mat da giac)...\n";
        Lab4::Application app;
        if (!app.init(1280, 720, "LAB 04 - 3D Viewing Transformations & Projection (Wireframe & Polygon)")) {
            std::cerr << "Loi: Khong the khoi tao OpenGL Lab 4!\n";
            return -1;
        }
        app.run();
        std::cout << "Chuong trinh Lab 4 ket thuc thanh cong.\n";
        return 0;
    }

    // =========================================================================
    // LAB 03: CÁC PHÉP BIẾN ĐỔI 2D
    // =========================================================================
    if (selectedLab == 3) {
        std::cout << "[Che do] Khoi chay LAB 03 (Cac phep bien doi 2D)...\n";
        Lab3::Application app;
        if (!app.init(1280, 720, "LAB 03 - 2D Geometric Transformations (Translation, Scale, Rotate, Reflect)")) {
            std::cerr << "Loi: Khong the khoi tao OpenGL Lab 3!\n";
            return -1;
        }
        app.run();
        std::cout << "Chuong trinh Lab 3 ket thuc thanh cong.\n";
        return 0;
    }

    // =========================================================================
    // LAB 02: XÉN HÌNH & TÔ MÀU ĐA GIÁC
    // =========================================================================
    if (selectedLab == 2) {
        std::cout << "[Che do] Khoi chay LAB 02 (Xen hinh & To mau da giac)...\n";
        Lab2::Application app;
        if (!app.init(1280, 720, "LAB 02 - Clipping & Polygon Filling Algorithms")) {
            std::cerr << "Loi: Khong the khoi tao OpenGL Lab 2!\n";
            return -1;
        }
        app.run();
        std::cout << "Chuong trinh Lab 2 ket thuc thanh cong.\n";
        return 0;
    }

    // =========================================================================
    // LAB 01: VẼ ĐOẠN THẲNG & ĐƯỜNG CONG
    // =========================================================================
    if (selectedLab == 1) {
        std::cout << "[Che do] Khoi chay LAB 01 (Bresenham & Midpoint Drawing Tool)...\n";
        Lab1::Application app;
        if (!app.init(1280, 720, "LAB 01 - Rasterization Drawing (Bresenham & Midpoint)")) {
            std::cerr << "Loi: Khong the khoi tao OpenGL Lab 1!\n";
            return -1;
        }
        app.run();
        std::cout << "Chuong trinh Lab 1 ket thuc thanh cong.\n";
        return 0;
    }

    // =========================================================================
    // LAB 2D & 3D GALLERY
    // =========================================================================
    if (selectedLab == 5) {
        std::cout << "[Che do] Khoi chay Lab 2D & 3D...\n";
        LabApp::Application app;
        if (!app.init(1280, 720, "OpenGL 2D & 3D Computer Graphics Lab")) {
            std::cerr << "Loi: Khong the khoi tao ung dung OpenGL!\n";
            return -1;
        }
        app.run();
        return 0;
    }

    return 0;
}
