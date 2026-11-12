# BÁO CÁO THỰC HÀNH LAB 01: THUẬT TOÁN BRESENHAM VÀ MIDPOINT

**Môn học:** Kỹ thuật Đồ họa / Đồ họa Máy tính  
**Công nghệ:** C++ (C++17/20), OpenGL 3.3 Core Profile, GLFW, GLAD  

---

## 1. MỤC TIÊU VÀ YÊU CẦU ĐỀ BÀI
Sử dụng thuật toán **Bresenham** và **Midpoint** để cài đặt các thuật toán vẽ các đường sau:
1. **1.1. Thuật toán vẽ đoạn thẳng** (xét đầy đủ 4 trường hợp của hệ số góc $m$).
2. **1.2. Thuật toán vẽ đường tròn** (Bresenham và Midpoint).
3. **1.3. Thuật toán vẽ Ellipse** (Midpoint).
4. **1.4. Vẽ các đường cong khác**: Parabol, Hyperbol, $\sin(x)$, $\cos(x)$.
5. **Yêu cầu tương tác:** Sử dụng sự kiện **click chuột** để xác định đoạn thẳng, đường tròn, ellipse,... cần vẽ kèm hiển thị xem trước (Live Preview).

---

## 2. CƠ SỞ LÝ THUYẾT VÀ GIẢI THUẬT CHI TIẾT

### 2.1. Thuật toán vẽ đoạn thẳng (Line Drawing Algorithm)
Cho 2 điểm đầu mút $P_1(x_1, y_1)$ và $P_2(x_2, y_2)$. Đặt $\Delta x = x_2 - x_1$, $\Delta y = y_2 - y_1$. Hệ số góc $m = \frac{\Delta y}{\Delta x}$.

#### Xét đầy đủ 4 trường hợp hệ số góc:
| Trường hợp | Khoảng giá trị $m$ | Trục định hướng (Driving Axis) | Hướng bước tiếp theo | Biến quyết định ban đầu | Cập nhật biến quyết định |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Trường hợp 1** | $0 < m \le 1$ | Trục $X$ ($x_{k+1} = x_k + 1$) | $y$ tăng hoặc giữ nguyên | $p_0 = 2\Delta y - \Delta x$ | $p < 0: p \mathrel{+}= 2\Delta y$<br>$p \ge 0: y++, p \mathrel{+}= 2(\Delta y - \Delta x)$ |
| **Trường hợp 2** | $m > 1$ | Trục $Y$ ($y_{k+1} = y_k + 1$) | $x$ tăng hoặc giữ nguyên | $p_0 = 2\Delta x - \Delta y$ | $p < 0: p \mathrel{+}= 2\Delta x$<br>$p \ge 0: x++, p \mathrel{+}= 2(\Delta x - \Delta y)$ |
| **Trường hợp 3** | $-1 \le m < 0$ | Trục $X$ ($x_{k+1} = x_k + 1$) | $y$ giảm hoặc giữ nguyên | $p_0 = 2\|\Delta y\| - \Delta x$ | $p < 0: p \mathrel{+}= 2\|\Delta y\|$<br>$p \ge 0: y--, p \mathrel{+}= 2(\|\Delta y\| - \Delta x)$ |
| **Trường hợp 4** | $m < -1$ | Trục $Y$ ($y_{k+1} = y_k - 1$) | $x$ tăng hoặc giữ nguyên | $p_0 = 2\Delta x - \|\Delta y\|$ | $p < 0: p \mathrel{+}= 2\Delta x$<br>$p \ge 0: x++, p \mathrel{+}= 2(\Delta x - \|\Delta y\|)$ |

- **Trường hợp đặc biệt:**
  - $m = 0$ ($\Delta y = 0$): Đoạn thẳng nằm ngang, chỉ cần tăng dần $x$.
  - $m = \pm \infty$ ($\Delta x = 0$): Đoạn thẳng thẳng đứng, chỉ cần tăng dần $y$.
- **Thuật toán Midpoint cho đoạn thẳng:** Dựa trên hàm ẩn $F(x, y) = ax + by + c = 0$ với $a = \Delta y, b = -\Delta x$. Tại mỗi bước, xét dấu của $F(M)$ tại trung điểm $M$ của hai pixel ứng viên. Giá trị số nguyên tương đương với thuật toán Bresenham nhưng có cơ sở đại số giải tích tường minh.

---

### 2.2. Thuật toán vẽ đường tròn (Circle Drawing Algorithm)
Phương trình đường tròn tâm $(x_c, y_c)$, bán kính $R$:
$$(x - x_c)^2 + (y - y_c)^2 = R^2$$

#### Tính chất đối xứng 8 phần (8-way symmetry):
Nếu xác định được một điểm $(x, y)$ thuộc cung ở góc phần tám thứ nhất ($0 \le x \le y$), ta suy ra ngay 7 điểm còn lại:
$$(x_c \pm x, y_c \pm y) \quad \text{và} \quad (x_c \pm y, y_c \pm x)$$

#### Thuật toán Bresenham cho đường tròn:
- Điểm xuất phát: $(0, R)$.
- Biến quyết định ban đầu: $p_0 = 3 - 2R$.
- Lặp khi $x < y$:
  - $x \leftarrow x + 1$
  - Nếu $p < 0$: $p \leftarrow p + 4x + 6$
  - Nếu $p \ge 0$: $y \leftarrow y - 1$, $p \leftarrow p + 4(x - y) + 10$.

#### Thuật toán Midpoint cho đường tròn:
- Hàm khoảng cách: $f(x, y) = x^2 + y^2 - R^2$.
- Điểm xuất phát: $(0, R)$, trung điểm ứng viên tiếp theo: $M(1, R - 0.5)$.
- Biến quyết định ban đầu: $p_0 = \frac{5}{4} - R \approx 1 - R$ (với số nguyên).
- Lặp khi $x < y$:
  - $x \leftarrow x + 1$
  - Nếu $p < 0$: $p \leftarrow p + 2x + 1$
  - Nếu $p \ge 0$: $y \leftarrow y - 1$, $p \leftarrow p + 2(x - y) + 1$.

---

### 2.3. Thuật toán vẽ Ellipse (Ellipse Drawing Algorithm)
Phương trình chính tắc Ellipse tâm $(x_c, y_c)$ bán trục $(r_x, r_y)$:
$$f(x, y) = r_y^2 x^2 + r_x^2 y^2 - r_x^2 r_y^2 = 0$$

#### Tính chất đối xứng 4 phần (4-way symmetry):
$$(x_c \pm x, y_c \pm y)$$

#### Chia 2 miền tiếp tuyến (Two Regions):
Độ dốc tiếp tuyến: $\frac{dy}{dx} = -\frac{2 r_y^2 x}{2 r_x^2 y}$.
1. **Miền 1 (Region 1):** $\left|\frac{dy}{dx}\right| < 1 \iff 2 r_y^2 x < 2 r_x^2 y$.
   - Bắt đầu tại $(0, r_y)$. Trục lái là $X$: bước $x \leftarrow x + 1$.
   - Biến quyết định ban đầu: $p_{1_0} = r_y^2 - r_x^2 r_y + \frac{1}{4} r_x^2$.
   - Nếu $p_1 < 0$: $p_1 \leftarrow p_1 + 2 r_y^2 x + r_y^2$.
   - Nếu $p_1 \ge 0$: $y \leftarrow y - 1$, $p_1 \leftarrow p_1 + 2 r_y^2 x - 2 r_x^2 y + r_y^2$.
2. **Miền 2 (Region 2):** $\left|\frac{dy}{dx}\right| \ge 1 \iff 2 r_y^2 x \ge 2 r_x^2 y$.
   - Bắt đầu tại điểm kết thúc của Miền 1. Trục lái là $Y$: bước $y \leftarrow y - 1$.
   - Biến quyết định ban đầu: $p_{2_0} = r_y^2 \left(x + \frac{1}{2}\right)^2 + r_x^2 (y - 1)^2 - r_x^2 r_y^2$.
   - Nếu $p_2 > 0$: $p_2 \leftarrow p_2 - 2 r_x^2 y + r_x^2$.
   - Nếu $p_2 \le 0$: $x \leftarrow x + 1$, $p_2 \leftarrow p_2 + 2 r_y^2 x - 2 r_x^2 y + r_x^2$.

---

### 2.4. Thuật toán vẽ các đường cong khác (Other Curves)

#### A. Parabol (Midpoint Algorithm)
- Phương trình chính tắc: $x^2 = 2py \iff y = \frac{x^2}{2p}$.
- Đỉnh tại $(x_c, y_c)$, đối xứng 2 nhánh qua trục thẳng đứng: $(x_c + x, y_c + y)$ và $(x_c - x, y_c + y)$.
- Độ dốc tiếp tuyến: $\frac{dy}{dx} = \frac{x}{p}$.
  - **Miền 1 ($x < p$):** Tiếp tuyến dốc $< 1$, bước theo trục $X$ ($x \leftarrow x + 1$).
  - **Miền 2 ($x \ge p$):** Tiếp tuyến dốc $\ge 1$, bước theo trục $Y$ ($y \leftarrow y + 1$).

#### B. Hyperbol (Midpoint Algorithm)
- Phương trình chính tắc: $\frac{x^2}{a^2} - \frac{y^2}{b^2} = 1 \iff b^2 x^2 - a^2 y^2 - a^2 b^2 = 0$.
- Đối xứng 4 nhánh qua tâm: $(x_c \pm x, y_c \pm y)$.
- Xuất phát từ đỉnh $(a, 0)$, tiếp tuyến tại đỉnh thẳng đứng ($\frac{dy}{dx} > 1$).
  - **Miền 1:** Bước theo trục $Y$ ($y \leftarrow y + 1$) đến khi độ dốc $\le 1$.
  - **Miền 2:** Bước theo trục $X$ ($x \leftarrow x + 1$) khi độ dốc $\le 1$.

#### C. Sóng $\sin(x)$ và $\cos(x)$ liên tục (8-Connected Continuous Rasterization)
- Phương trình: $y = y_c - \operatorname{round}\left(A \sin\left(\frac{2\pi (x - x_c)}{\lambda}\right)\right)$.
- Để đường cong lượng giác hiển thị liên tục, không bị đứt đoạn pixel ở các đoạn dốc lớn: giải thuật tự động lấp đầy khoảng trống pixel theo cột giữa 2 điểm liên tiếp bằng đoạn thẳng ngắn Bresenham, bảo đảm đường cong đạt chuẩn 8-connected hoàn hảo.

---

## 3. CƠ CHẾ TƯƠNG TÁC CHUỘT THỜI GIAN THỰC
- **Click chuột trái (Press):** Đặt điểm gốc / tâm hình vẽ.
- **Kéo chuột (Drag):** Kích hoạt cơ chế **Live Preview (Rubber-banding)** hiển thị ngay hình chiếu mờ của đường cong trên màn hình trước khi chốt điểm.
- **Thả chuột (Release):** Chốt điểm kết thúc / tham số hình vẽ, lưu vào danh sách hình vẽ trên Canvas.
- **Thao tác phụ trợ:**
  - **Cuộn chuột (Scroll Wheel):** Phóng to / thu nhỏ canvas (từ 0.15x đến 64.0x). Khi phóng to từ 4x trở lên, lưới **Pixel Grid** tự động xuất hiện để người dùng quan sát từng pixel rasterization rõ nét.
  - **Chuột phải (Right Drag):** Di chuyển góc nhìn (Pan Canvas).
  - **Thanh công cụ Toolbar trên đỉnh màn hình:** Cho phép click chuột trực tiếp để chọn 9 công cụ vẽ, bảng 8 màu sắc, nút Undo, Clear, Grid, Axes, Reset View.
  - **Thanh HUD dưới đáy màn hình:** Hiển thị tức thời thông số toán học: tọa độ chuột, $\Delta x, \Delta y$, hệ số góc $m$, trường hợp Case 1..4 của đoạn thẳng, bán kính $R$, số lượng pixel đã sinh ra.

---

## 4. KẾT QUẢ KIỂM THỬ VÀ ĐÁNH GIÁ
Chương trình tích hợp bộ kiểm thử tự động `.\main.exe --test` với kết quả đạt 100% các ca kiểm thử:
```text
===============================================================
        CHAY KIEM THU TU DONG CAC THUAT TOAN LAB 01            
===============================================================
 [PASS] Case 1: 0 < m <= 1 (m = 0.5) -> Bresenham: 41 pts, Midpoint: 41 pts
 [PASS] Case 2: m > 1 (m = 2.5) -> Bresenham: 51 pts, Midpoint: 51 pts
 [PASS] Case 3: -1 <= m < 0 (m = -0.5) -> Bresenham: 41 pts, Midpoint: 41 pts
 [PASS] Case 4: m < -1 (m = -5.0) -> Bresenham: 51 pts, Midpoint: 51 pts
 [PASS] Case Horizontal (m = 0) -> Bresenham: 41 pts, Midpoint: 41 pts
 [PASS] Case Vertical (dx = 0) -> Bresenham: 41 pts, Midpoint: 41 pts
 [PASS] Circle R=25 -> Bresenham: 144 pts, Midpoint: 152 pts
 [PASS] Ellipse rx=40, ry=20 -> Midpoint: 184 pts
 [PASS] Parabola -> 143 pts
 [PASS] Hyperbola -> 240 pts
 [PASS] Sin Wave -> 541 pts
 [PASS] Cos Wave -> 541 pts
---------------------------------------------------------------
KET QUA: TAT CA THUAT TOAN LAB 1 HOAT DONG CHINH XAC!
===============================================================
```

---

## 5. KẾT LUẬN
Toàn bộ các yêu cầu của **Lab 01** đã được hoàn thành trọn vẹn:
1. Đã cài đặt đầy đủ thuật toán Bresenham và Midpoint vẽ đoạn thẳng với 4 trường hợp hệ số góc.
2. Đã cài đặt đường tròn và ellipse với tính chất đối xứng 8 phần và 4 phần.
3. Đã cài đặt parabol, hyperbol, sin, cos theo phương pháp Midpoint và rasterization liên tục.
4. Đã xây dựng giao diện tương tác chuột trực quan, mượt mà, hỗ trợ phóng to lưới pixel và bảng thông số thời gian thực.
