# BÁO CÁO THỰC HÀNH LAB 04: QUAN SÁT VẬT THỂ 3D VÀ CÁC PHÉP BIẾN ĐỔI HỆ QUAN SÁT

**Môn học:** Kỹ thuật Đồ họa / Đồ họa Máy tính (Computer Graphics)  
**Công nghệ:** C++ (C++17), OpenGL 3.3 Core Profile, GLFW, GLAD, GLSL  
**Học viên thực hiện:** Đồ họa máy tính Lab 04  

---

## 1. MỤC TIÊU VÀ YÊU CẦU ĐỀ BÀI

Viết chương trình cho phép quan sát vật thể 3D đáp ứng các yêu cầu sau:
1. **4.1. Vật thể được biểu diễn theo mô hình khung kết nối (Wireframe model):**
   - Biểu diễn danh sách các đỉnh (Vertices) và các cạnh nối (Edges).
   - Vẽ khung dây bằng các đoạn thẳng liên kết và hiển thị các nút đỉnh rõ ràng.
2. **4.2. Vật thể được biểu diễn theo mô hình mặt đa giác (Polygon / Surface model):**
   - Biểu diễn bằng các mặt phẳng đa giác (Faces/Polygons).
   - Tô màu phân biệt từng mặt đa giác, tính toán vector pháp tuyến mặt, áp dụng mô hình chiếu sáng (Blinn-Phong Lighting), khử mặt khuất bằng Z-buffer Depth Test và Back-Face Culling.
3. **Yêu cầu tương tác hệ quan sát (Viewing Transformations):**
   - **Thay đổi phép chiếu:** Chuyển đổi linh hoạt giữa **Phép chiếu song song (Parallel / Orthographic)** và **Phép chiếu phối cảnh (Perspective)**.
   - **Thay đổi góc quan sát $\theta$ (Theta) và $\Phi$ (Phi):** Xoay camera trên mặt cầu quanh vật thể theo góc phương vị và góc nâng.
   - **Thay đổi vị trí quan sát (Điểm nhìn):** Di chuyển vị trí camera (Eye) và tâm quan sát (Target) trong không gian (Camera Panning).
   - **Phóng to, thu nhỏ vật thể:** Thu phóng khoảng cách quan sát (Camera Zoom) và phóng to/thu nhỏ kích thước vật thể qua ma trận tỉ lệ (Model Scale Matrix).

---

## 2. CƠ SỞ LÝ THUYẾT VÀ CÔNG THỨC TOÁN HỌC

### 2.1. Hệ tọa độ quan sát (Viewing Coordinate System - VCS) và Ma trận LookAt

Trong không gian 3 chiều, vị trí của người quan sát (Camera) được xác định bởi:
- **Điểm nhìn (Eye position):** $E = (x_e, y_e, z_e)$
- **Điểm ngắm / Tâm quan sát (Target / Reference point):** $T = (x_t, y_t, z_t)$
- **Vector hướng lên (Up vector):** $V_{up} = (0, 1, 0)$

#### Xây dựng hệ trục trực chuẩn của Camera $(\vec{s}, \vec{u}, \vec{f})$:
1. Vector hướng nhìn chuẩn hóa:
   $$\vec{f} = \frac{T - E}{\|T - E\|}$$
2. Vector trục $X$ của camera (hướng sang phải - Right vector):
   $$\vec{s} = \frac{\vec{f} \times V_{up}}{\|\vec{f} \times V_{up}\|}$$
3. Vector trục $Y$ của camera (hướng lên thực - True Up vector):
   $$\vec{u} = \vec{s} \times \vec{f}$$
4. Vector trục $Z$ của camera (hướng nhìn ngược theo quy ước OpenGL nhìn về $-Z$):
   $$\vec{z}_c = -\vec{f}$$

#### Ma trận biến đổi quan sát (Viewing Matrix $M_{view}$):
Chuyển đổi một điểm từ Hệ tọa độ thế giới (WCS) sang Hệ tọa độ quan sát (VCS):
$$M_{view} = R_{view} \cdot T_{view} = \begin{bmatrix} s_x & s_y & s_z & -\vec{s} \cdot E \\ u_x & u_y & u_z & -\vec{u} \cdot E \\ -f_x & -f_y & -f_z & \vec{f} \cdot E \\ 0 & 0 & 0 & 1 \end{bmatrix}$$

---

### 2.2. Biến đổi góc quan sát $\theta$ và $\Phi$ theo tọa độ cầu (Spherical Orbit Coordinates)

Để người dùng có thể xoay quanh vật thể tự nhiên, vị trí điểm nhìn $E$ được tính theo tọa độ cầu có tâm tại điểm ngắm $T(x_t, y_t, z_t)$ với bán kính quan sát $R$:
- **Góc phương vị $\theta$ (Theta - Yaw / Azimuth):** Góc quay quanh trục thẳng đứng $Y$, phạm vi $[0^\circ, 360^\circ)$.
- **Góc nâng $\Phi$ (Phi - Pitch / Elevation):** Góc nâng so với mặt phẳng ngang $XZ$, phạm vi $[-89^\circ, +89^\circ]$ (khóa tránh hiện tượng suy biến gimbal lock khi trùng trục $Y$).

#### Công thức chuyển đổi:
$$\begin{aligned}
x_e &= x_t + R \cos(\Phi) \sin(\theta) \\
y_e &= y_t + R \sin(\Phi) \\
z_e &= z_t + R \cos(\Phi) \cos(\theta)
\end{aligned}$$

Khi $\theta = 0^\circ, \Phi = 0^\circ$, ta có $E = (x_t, y_t, z_t + R)$: Điểm nhìn đặt đối diện trực tiếp vật thể theo chiều dương trục $Z$.

---

### 2.3. Phép chiếu song song (Parallel / Orthographic Projection)

Phép chiếu song song đưa hộp thể tích quan sát $[left, right] \times [bottom, top] \times [near, far]$ về khối lập phương chuẩn hóa chuẩn NDC $[-1, 1]^3$:

$$P_{ortho} = \begin{bmatrix}
\frac{2}{right - left} & 0 & 0 & -\frac{right + left}{right - left} \\
0 & \frac{2}{top - bottom} & 0 & -\frac{top + bottom}{top - bottom} \\
0 & 0 & -\frac{2}{far - near} & -\frac{far + near}{far - near} \\
0 & 0 & 0 & 1
\end{bmatrix}$$

- **Đặc điểm:** Các đường thẳng song song trong không gian vẫn giữ tính chất song song sau khi chiếu; tỉ lệ kích thước vật thể không phụ thuộc vào khoảng cách từ camera đến vật thể.

---

### 2.4. Phép chiếu phối cảnh (Perspective Projection)

Phép chiếu phối cảnh mô phỏng cơ chế nhìn của mắt người hoặc ống kính máy ảnh, các tia chiếu hội tụ tại tâm chiếu (Camera).

Thể tích nhìn là hình chóp cụt (Frustum) xác định bởi góc mở theo chiều đứng $fov_y$, tỉ lệ khung hình $aspect = \frac{w}{h}$, khoảng cách mặt phẳng cắt gần $z_{near}$ và xa $z_{far}$:

$$P_{persp} = \begin{bmatrix}
\frac{1}{aspect \cdot \tan(fov_y / 2)} & 0 & 0 & 0 \\
0 & \frac{1}{\tan(fov_y / 2)} & 0 & 0 \\
0 & 0 & -\frac{far + near}{far - near} & -\frac{2 \cdot far \cdot near}{far - near} \\
0 & 0 & -1 & 0
\end{bmatrix}$$

- **Phép chia phối cảnh (Perspective Division):** Tọa độ đồng nhất $[x_c, y_c, z_c, w_c]^T$ với $w_c = -z_{view}$. Chia $x_c, y_c, z_c$ cho $w_c$ tạo nên hiện tượng: vật càng ở xa thì $|z_{view}|$ càng lớn $\implies$ tọa độ chiếu trên màn hình càng nhỏ lại.

---

### 2.5. Phép phóng to, thu nhỏ vật thể (Zoom & Scale)

Chương trình hỗ trợ 3 cơ chế phóng to, thu nhỏ toàn diện:
1. **Thu phóng khoảng cách quan sát (Camera Distance Zoom):**
   Thay đổi bán kính $R \leftarrow R + \Delta R$. Khi $R$ giảm, camera tiến sát vật thể làm vật thể chiếm góc nhìn lớn hơn.
2. **Ma trận biến đổi tỉ lệ vật thể (Model Scaling Transformation):**
   $$M_{scale}(s) = \begin{bmatrix} s & 0 & 0 & 0 \\ 0 & s & 0 & 0 \\ 0 & 0 & s & 0 \\ 0 & 0 & 0 & 1 \end{bmatrix}$$
   Biến đổi tọa độ hình học thực của vật thể từ $0.2\times$ đến $5.0\times$.
3. **Thu phóng góc mở FOV / Vùng nhìn trực giao Ortho Size:**
   Tăng/giảm tiêu cự ống kính mô phỏng hiện tượng Telephoto Zoom.

---

### 2.6. Mô hình khung kết nối (4.1) và Mô hình mặt đa giác (4.2)

| Đặc điểm so sánh | 4.1. Mô hình Khung kết nối (Wireframe) | 4.2. Mô hình Mặt đa giác (Polygon/Surface) |
| :--- | :--- | :--- |
| **Thành phần biểu diễn** | Tập đỉnh $V_i$ và tập cạnh $E_j = (V_a, V_b)$ | Tập mặt đa giác $F_k = (V_{k1}, V_{k2}, \dots)$ |
| **Phương thức render** | `GL_LINES` và `GL_POINTS` | `GL_TRIANGLES` |
| **Màu sắc** | Màu khung neon (Cyan/Vàng) | Mỗi mặt có bảng màu riêng biệt rực rỡ |
| **Khử mặt khuất** | Nhìn thấu xuyên suốt kết cấu khung xương | Kích hoạt Z-buffer `GL_DEPTH_TEST`, Back-face culling |
| **Chiếu sáng (Lighting)** | Không đổ bóng | Ánh sáng Blinn-Phong (Ambient + Diffuse + Specular) |

---

## 3. CÁC MÔ HÌNH VẬT THỂ 3D ĐÃ CÀI ĐẶT

Chương trình cung cấp 9 mô hình 3D đa dạng:
1. **Hình lập phương (Cube):** 8 đỉnh, 12 cạnh, 6 mặt tứ giác, 6 màu sắc tương phản.
2. **Hình chóp tứ giác / Kim tự tháp (Pyramid):** 5 đỉnh, 8 cạnh, 5 mặt (đáy vuông + 4 mặt bên).
3. **Hình lăng trụ tam giác (Triangular Prism):** 6 đỉnh, 9 cạnh, 5 mặt (2 đáy tam giác, 3 mặt bên).
4. **Hình lăng trụ lục giác (Hexagonal Prism):** 12 đỉnh, 18 cạnh, 8 mặt (2 đáy lục giác, 6 mặt bên).
5. **Hình cầu mặt cong (UV Sphere):** Lưới 16 vĩ tuyến $\times$ 24 kinh tuyến, thể hiện độ cong 3D mượt mà.
6. **Hình nón (Cone):** Đỉnh nhọn và đáy tròn 24 phân đoạn.
7. **Hình trụ (Cylinder):** 2 đáy tròn và mặt thân 24 phân đoạn.
8. **Hình xuyến (Torus / Bánh Donut):** Mặt cong bậc cao 3D với 16 vòng tiết diện $\times$ 24 vòng kinh tuyến.
9. **Mô hình Ngôi nhà 3D (3D House):** Gồm thân nhà, mái ngói dốc, ống khói tiêu chuẩn bài học đồ họa.

---

## 4. HƯỚNG DẪN BIÊN DỊCH VÀ ĐIỀU KHIỂN

### 4.1. Biên dịch chương trình
Sử dụng MinGW-w64 g++ trong PowerShell:
```powershell
g++ -O2 src/lab4_main.cpp src/glad.c -Iinclude -Llib -lglfw3 -lopengl32 -lgdi32 -lwinmm -o lab4.exe
```
Hoặc biên dịch bộ tổng hợp toàn bộ các Lab:
```powershell
g++ -O2 src/old_labs_main.cpp src/glad.c -Iinclude -Llib -lglfw3 -lopengl32 -lgdi32 -lwinmm -o main.exe
```

### 4.2. Bảng phím tắt và thao tác tương tác

| Phím / Thao tác | Chức năng thực hiện |
| :--- | :--- |
| **[P]** hoặc click nút UI | Chuyển đổi **Phối cảnh (Perspective)** $\longleftrightarrow$ **Song song (Orthographic)** |
| **[M]** hoặc click nút UI | Chuyển đổi giữa: Khung kết nối, Mặt đa giác, Mặt + Khung viền, Tập đỉnh |
| **[Tab]** hoặc **[O]** | Duyệt qua 9 mô hình vật thể 3D khác nhau |
| **Kéo chuột trái** | Xoay góc quan sát $\theta$ và $\Phi$ quanh vật thể (Orbit Camera) |
| **Phím Mũi tên** | Thay đổi góc $\theta$ (Trái/Phải) và $\Phi$ (Lên/Xuống) từng bước $5^\circ$ |
| **Kéo chuột phải / WASD** | Di chuyển điểm nhìn và điểm ngắm (Camera Panning) |
| **Cuộn chuột** | Phóng to / thu nhỏ khoảng cách quan sát $R$ (Camera Zoom) |
| **[+] / [-]** | Phóng to / thu nhỏ kích thước hình học vật thể qua ma trận Scale |
| **[PageUp] / [PageDown]** | Điều chỉnh góc mở FOV hoặc kích thước vùng nhìn trực giao |
| **[SPACE]** | Bật / tắt tự động xoay quanh vật thể (Auto Orbit) |
| **[1], [2], [3], [4]** | Đặt nhanh góc nhìn: Nhìn trước, Nhìn trên, Nhìn phải, Trục đo Isometric |
| **[R]** | Đặt lại toàn bộ hệ quan sát về trạng thái ban đầu (Reset View) |
| **[L]** | Bật / tắt chiếu sáng (Blinn-Phong Lighting) |
| **[G] / [X]** | Bật / tắt Lưới mặt phẳng tọa độ (Grid) và Hệ trục 3D (Ox, Oy, Oz) |
| **[C]** | Bật / tắt Khử mặt khuất (Back-Face Culling) |
| **[K] hoặc [ENTER]** | Xuất toàn bộ giá trị ma trận $4\times 4$ chi tiết ra màn hình Console |
| **[H]** | Ẩn / hiện Bảng ma trận $4\times 4$ HUD trực quan trên màn hình |

---

## 5. KẾT QUẢ KIỂM THỬ TỰ ĐỘNG (AUTOMATED UNIT TESTS)

Chạy lệnh kiểm thử:
```powershell
.\lab4.exe --test
```
Kết quả kiểm thử đạt **100% PASSED**:

```
=======================================================
  RUNNING AUTOMATED UNIT TESTS: LAB 04 3D VIEWING      
=======================================================
 [TEST 1] 4x4 Identity Matrix Transform: PASSED
 [TEST 2] Spherical Orbit to Eye Position (Theta, Phi, R): PASSED
 [TEST 3] Viewing Transformation Matrix (LookAt): PASSED
 [TEST 4] Orthographic (Parallel) Projection Matrix: PASSED
 [TEST 5] Perspective Projection Matrix (Frustum Mapping): PASSED
 [TEST 6] 3D Scaling Transformation Matrix (Zoom/Scale): PASSED
=======================================================
 LAB 04 TESTS RESULT: ALL PASSED (100%)
=======================================================
```

---

## 6. KẾT LUẬN

Chương trình đã giải quyết trọn vẹn và hoàn hảo tất cả các yêu cầu của **LAB 04**:
1. Đã cài đặt đầy đủ hai mô hình biểu diễn vật thể 3D: **4.1 Mô hình khung kết nối** (Wireframe với các đỉnh và cạnh) và **4.2 Mô hình mặt đa giác** (Polygon Solid với màu sắc từng mặt, chiếu sáng Blinn-Phong và kiểm tra chiều sâu Z-buffer).
2. Xây dựng ma trận biến đổi hệ quan sát (LookAt Matrix) chuẩn mực theo lý thuyết đồ họa máy tính, tính toán vị trí camera theo tọa độ cầu $(\theta, \Phi, R)$.
3. Hỗ trợ chuyển đổi mượt mà giữa phép chiếu song song và phép chiếu phối cảnh.
4. Cung cấp khả năng xoay góc nhìn, di chuyển điểm nhìn, và phóng to thu nhỏ đa chiều (cả qua khoảng cách camera và qua ma trận tỉ lệ vật thể).
5. Giao diện người dùng trực quan hiển thị ma trận 16 phần tử theo thời gian thực và hỗ trợ 9 vật thể 3D phong phú.
