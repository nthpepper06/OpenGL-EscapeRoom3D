#include "core/Application.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "========================================================\n";
    std::cout << "  3D ESCAPE ROOM - COMPUTER GRAPHICS ASSIGNMENT\n";
    std::cout << "  Modern OpenGL (Core Profile 3.3) + Dear ImGui\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << "  CONTROLS:\n";
    std::cout << "   - [W, A, S, D]  : Di chuyen goc nhin thu nhat (FPS)\n";
    std::cout << "   - [Chuot]       : Xoay goc nhin (Mouse Look, khoa Pitch)\n";
    std::cout << "   - [Tab]         : Chuyen doi Play Mode <-> Edit Mode\n";
    std::cout << "   - [F]           : Bat / Tat den pin (Spotlight)\n";
    std::cout << "   - [E]           : Tuong tac voi vat the (Ngam tam ngam)\n";
    std::cout << "========================================================\n";

    Application app;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--capture" || arg == "--capture-all" || arg == "--benchmark") {
            app.captureMode = true;
        }
        if (arg == "--test-bug" || arg == "--diagnose") {
            app.testBugMode = true;
        }
        if (arg == "--smoke-test") {
            app.smokeTestMode = true;
        }
    }

    if (!app.init(1280, 720, "3D Escape Room - Modern OpenGL 3.3 Core")) {
        std::cerr << "Application initialization failed!\n";
        return -1;
    }

    app.run();
    return 0;
}
