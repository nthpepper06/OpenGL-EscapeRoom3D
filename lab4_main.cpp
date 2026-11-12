#include <iostream>
#include "Lab4/Lab4App.hpp"
#include "Lab4/Lab4Transformations.hpp"

int main(int argc, char* argv[]) {
    // Kiem tra tham so dong lenh
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--test" || arg == "-t") {
            bool ok = Lab4::runLab4Tests();
            return ok ? 0 : 1;
        }
    }

    std::cout << "====================================================================\n";
    std::cout << "   CHUONG TRINH DO HOA MAY TINH OPENGL - LAB 04                      \n";
    std::cout << "   PHEP QUAN SAT VAT THE 3D (3D VIEWING TRANSFORMATIONS & PROJECTION)\n";
    std::cout << "====================================================================\n";

    Lab4::Application app;
    if (!app.init(1280, 720, "LAB 04 - Quan Sat Vat The 3D (Khung Ket Noi & Mat Da Giac)")) {
        std::cerr << "Loi: Khong the khoi tao OpenGL Lab 4!\n";
        return -1;
    }

    app.run();
    std::cout << "Chuong trinh Lab 4 da dong thanh cong.\n";
    return 0;
}
