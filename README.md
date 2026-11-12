# 3D Escape Room - Bài Tập Lớn Đồ Họa Máy Tính (Computer Graphics)

Dự án mô phỏng trò chơi **3D Escape Room (Phòng Thoát Hiểm 3D)** sử dụng **Modern OpenGL (Core Profile 3.3)**, C++17, và kiến trúc đồ họa hướng module chuyên nghiệp.

Dự án được cấu hình tự động 100% bằng **CMake** và **FetchContent**, tự động tải và liên kết các thư viện tiêu chuẩn công nghiệp:
- **GLFW (3.4)**: Quản lý Cửa sổ và Input đa nền tảng
- **GLAD**: OpenGL 3.3 Core Profile function loader
- **GLM (1.0.1)**: Thư viện toán học đồ họa 3D (Vector, Matrix, Transforms)
- **stb_image**: Tải và xử lý kết cấu hình ảnh (Textures)
- **Dear ImGui (v1.91.5)**: Giao diện đồ họa người dùng thời gian thực (kèm backend `glfw` + `opengl3`)
- **nlohmann/json (v3.11.3)**: Đọc, ghi và tuần tự hóa cấu hình phòng (`room_scene.json`)

---

## 🏛️ Cấu Trúc Thư Mục Dự Án

```
OpenGL_Project/
├── CMakeLists.txt                # Cấu hình FetchContent & biên dịch toàn diện
├── room_scene.json               # File lưu/tải toàn bộ cấu hình phòng
├── assets/
│   ├── shaders/
│   │   ├── blinn_phong.vert      # Vertex Shader: World space & Normal Matrix
│   │   └── blinn_phong.frag      # Fragment Shader: Blinn-Phong, Point & Spotlight, Highlight
│   ├── scenes/
│   │   └── room_scene.json       # Bản sao lưu cấu hình phòng mặc định
│   └── textures/                 # Thư mục chứa hình ảnh texture
└── src/
    ├── main.cpp                  # Điểm khởi chạy chương trình (Entry Point)
    ├── core/
    │   ├── Common.hpp            # Định nghĩa enum, kiểu dữ liệu và thư viện dùng chung
    │   ├── Application.hpp       # Quản lý Window, OpenGL Context, Vòng lặp render
    │   └── Application.cpp
    ├── camera/
    │   ├── Camera.hpp            # FPS Camera: WASD + Mouse Look, Khóa góc Pitch [-89°, +89°]
    │   └── Camera.cpp
    ├── renderer/
    │   ├── Shader.hpp / .cpp     # Trình biên dịch và thiết lập Uniforms Shader
    │   ├── Mesh.hpp / .cpp       # Quản lý VAO, VBO, EBO và AABB của mô hình
    │   ├── PrimitiveGenerator.hpp/.cpp # Tạo hình học chuẩn: Cube, Cylinder, Plane, Sphere
    │   ├── Material.hpp          # Thuộc tính vật liệu: Diffuse, Specular, Shininess, Texture
    │   ├── Light.hpp             # Định nghĩa Point Light, SpotLight (Đèn pin), Ambient Light
    │   └── Texture.hpp / .cpp    # Đọc ảnh bằng STB và sinh Procedural Textures (gỗ, kim loại, gạch)
    ├── scene/
    │   ├── SceneObject.hpp/.cpp  # Hệ thống phân cấp Parent-Child, Local/World Matrix, Animation
    │   ├── EscapeRoomBuilder.hpp/.cpp # Khởi tạo phòng kín, bàn làm việc, két sắt, chìa khóa, cửa
    │   └── SceneSerializer.hpp/.cpp   # Lưu và khôi phục SceneGraph qua file JSON
    ├── interaction/
    │   ├── Raycaster.hpp / .cpp  # Thuật toán Ray-AABB Slab intersection và Object Picking
    │   └── InteractionManager.hpp/.cpp # Logic trò chơi: Mật mã két sắt, nhặt chìa khóa, mở cửa
    └── ui/
        ├── GuiLayer.hpp / .cpp   # ImGui Inspector, Hierarchy Tree, Bàn phím số Keypad, HUD Crosshair
```

---

## 🎮 Hướng Dẫn Điều Khiển & Gameplay

### 1. Phím tắt thao tác:
| Phím | Chức năng | Chế độ |
|---|---|---|
| **W, A, S, D** | Di chuyển người chơi (góc nhìn thứ nhất FPS) | Play Mode |
| **Chuột (Mouse Look)** | Xoay góc nhìn camera (Pitch khóa tối đa 89° tránh lật màn hình) | Play Mode |
| **Tab** | **Chuyển đổi giữa Play Mode và Edit Mode** | Toàn bộ |
| **E** | **Tương tác (Interact)** với vật thể đang ngắm trúng tâm ngắm Crosshair | Play Mode |
| **F** | **Bật / Tắt Đèn pin (Flashlight Spotlight)** gắn theo Camera | Toàn bộ |
| **Chuột trái (Click)** | Chọn trực tiếp SceneObject trong không gian 3D để Inspect | Edit Mode |
| **ESC** | Đóng popup Keypad hoặc chuyển nhanh về Edit Mode | Toàn bộ |

### 2. Hai chế độ hoạt động:
- **Play Mode (Chế độ chơi)**:
  - Con trỏ chuột bị ẩn và khóa vào giữa màn hình.
  - Hiện tâm ngắm **Crosshair** `(+)` ở chính giữa.
  - Khi ngắm vào vật thể có thể tương tác (khoảng cách <= 3.5m), vật thể sẽ **tự động sáng highlight vàng/kim loại**, tâm ngắm đổi màu và xuất hiện gợi ý thao tác: `[E] Mở ngăn kéo`, `[E] Bật/tắt đèn trần`, `[E] Nhập mã két sắt`, `[E] Nhặt chìa khóa`, `[E] Mở cửa thoát hiểm`.
- **Edit Mode (Chế độ chỉnh sửa ImGui - bấm TAB)**:
  - Hiện con trỏ chuột tự do để thao tác trên giao diện Dear ImGui.
  - **Scene Hierarchy**: Cây phân cấp trực quan toàn bộ đồ vật trong phòng.
  - **Object Inspector**: Chỉnh sửa trực tiếp Position, Rotation, Scale, Màu sắc Diffuse, Specular, Độ bóng Shininess, và trạng thái tương tác.
  - **Lighting System**: Tinh chỉnh vị trí, màu sắc, cường độ đèn trần và đèn pin.
  - **File Menu**: Nút **Save Scene (room_scene.json)** và **Load Scene (room_scene.json)**.

### 3. Cốt truyện & Luồng giải đố (Escape Room Walkthrough):
1. Bạn tỉnh dậy trong một căn phòng kín với 4 bức tường, trần và sàn. Đèn trần có thể bật/tắt bằng công tắc trên tường cạnh cửa, và bạn có một chiếc đèn pin (bấm **F**).
2. Tiến lại gần chiếc **Bàn làm việc (Work Desk)**, ngắm vào ngăn kéo và bấm **E** để kéo ngăn kéo trượt ra ngoài.
3. Bên trong ngăn kéo có một **Mảnh giấy manh mối (Secret Clue Note)**: ngắm vào và bấm **E** để đọc mã PIN két sắt (`Mã PIN: 7492`).
4. Tiến sang chiếc **Két sắt bằng thép (Electronic Safe)**, ngắm vào bàn phím số điện tử trên cửa két sắt và bấm **E**.
5. Nhập mã PIN `7492` trên bàn phím ảo ImGui và bấm `OK`. Két sắt sẽ mở khóa và cánh cửa két xoay quanh bản lề 110 độ.
6. Bên trong két sắt là chiếc **Chìa khóa vàng (Golden Exit Key)**: ngắm vào và bấm **E** để nhặt chìa khóa vào hành trang.
7. Tiến lại **Cửa chính thoát hiểm (Exit Door)** dưới biển báo EXIT màu xanh phát sáng, ngắm vào ổ khóa và bấm **E**. Cánh cửa nặng nề sẽ xoay mở và màn hình hiện thông báo **MISSION ACCOMPLISHED! ESCAPED THE ROOM!**

---

## 🛠️ Hướng Dẫn Biên Dịch & Chạy Chương Trình

Dự án hỗ trợ trình biên dịch **GCC/MinGW-w64** (hoặc MSVC, Clang) và công cụ build **Ninja** hoặc **MinGW Makefiles**.

### Bước 1: Cấu hình với CMake
Mở Terminal / PowerShell tại thư mục dự án và chạy:
```powershell
cmake -B build -G "Ninja"
```
*(Nếu không dùng Ninja, bạn có thể dùng: `cmake -B build -G "MinGW Makefiles"`)*

CMake sẽ tự động tải các thư viện qua FetchContent và cấu hình các mục tiêu biên dịch.

### Bước 2: Biên dịch dự án
```powershell
cmake --build build
```
Sau khi hoàn thành, file thực thi `EscapeRoom.exe` và thư mục `assets` sẽ được tạo trong thư mục `build/`.

### Bước 3: Chạy chương trình
```powershell
./build/EscapeRoom.exe
```

---

## 💡 Điểm Nổi Bật Về Kỹ Thuật (Architecture Highlights)

1. **Modern OpenGL 3.3 Core Profile**:
   - Hoàn toàn không sử dụng immediate mode (`glBegin/glEnd`).
   - Sử dụng VAO, VBO, EBO với cấu trúc đỉnh chuẩn: `Position (vec3)`, `Normal (vec3)`, `TexCoords (vec2)`.
2. **Hệ thống chiếu sáng Blinn-Phong đa nguồn sáng**:
   - Sử dụng Halfway vector $H = \frac{L + V}{\|L + V\|}$ giúp specular highlight mượt mà và tự nhiên hơn Phong truyền thống.
   - 1 Point Light (đèn trần) với độ suy giảm theo khoảng cách: $\frac{1}{k_c + k_l d + k_q d^2}$.
   - 1 Spotlight (đèn pin) gắn theo vector nhìn của Camera với vùng chuyển mượt cone cut-off: $\text{clamp}\left(\frac{\cos\theta - \cos\theta_{out}}{\cos\theta_{in} - \cos\theta_{out}}, 0, 1\right)$.
3. **Mô hình phân cấp Scene Graph (Parent-Child)**:
   - Cho phép các chi tiết (như bản lề cửa, tay nắm, ngăn kéo, bảng mã) kế thừa ma trận biến đổi thế giới từ đối tượng cha:
     $$M_{\text{world}} = M_{\text{parent}} \times M_{\text{local}}$$
   - Hỗ trợ animation mượt mà (ngăn kéo trượt theo trục Z, cửa két và cửa chính xoay quanh trục bản lề Y).
4. **Hệ thống Raycasting & Bounding Box (AABB)**:
   - Sử dụng thuật toán Slab Method (Kay & Kajiya) để kiểm tra giao điểm tia nhìn từ tâm Camera với AABB của mọi vật thể trong phòng.
   - Hỗ trợ picking tương tác trong Play Mode và picking bằng con trỏ chuột trong Edit Mode.
5. **Tuần tự hóa JSON (Save & Load)**:
   - Dùng `nlohmann/json` để lưu trữ và nạp lại toàn bộ trạng thái vị trí, góc xoay, kích thước, màu sắc và thuộc tính tương tác ra file `room_scene.json`.
