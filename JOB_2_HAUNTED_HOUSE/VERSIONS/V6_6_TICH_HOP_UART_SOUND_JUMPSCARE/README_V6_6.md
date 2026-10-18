# PHIÊN BẢN V6.6 - TÍCH HỢP UART GIỮA STM32_SOUND VÀ STM32_JUMPSCARE

## 1. MỤC TIÊU NÂNG CẤP
Tích hợp trực tiếp thuật toán giao tiếp UART (`SoftwareSerial` 9600 baud) từ 2 mã nguồn test của người dùng vào hệ sinh thái 3 mạch STM32:
- **STM32_JUMPSCARE**: 3 kênh kích hoạt rơ-le độc lập, khóa chống kích đè, gửi tín hiệu ngắt âm thanh nền và tắt đèn.
- **STM32_SOUND**: Đón nhận tín hiệu điều khiển rơ-le âm thanh nền PA6 (chớp ngắt 500ms ON / 500ms OFF khi nghỉ, ngắt tuyệt đối khi có Jumpscare), đồng thời chuyển tiếp lệnh tắt đèn `T` sang `STM32_RELAY`.

---

## 2. SƠ ĐỒ ĐẤU NỐI UART GIỮA 2 MẠCH (VẬT LÝ)
Để 2 vi điều khiển bắt tay giao tiếp chuẩn xác 100%, phải đấu **CHÉO CHÂN TX/RX** và **NỐI CHUNG MASS (GND)**:

| Chân STM32_JUMPSCARE | Hướng tín hiệu | Chân STM32_SOUND | Chức năng |
| :---: | :---: | :---: | :--- |
| **PB5** (TX) | $\longrightarrow$ | **PB11** (RX) | Jumpscare phát lệnh `'S'`, `'P'`, `'T'` sang Sound |
| **PB6** (RX) | $\longleftarrow$ | **PB10** (TX) | Sound gửi lệnh điều khiển từ xa / Reset `'0'` sang Jumpscare |
| **GND** | $\longleftrightarrow$ | **GND** | Nối chung mass chuẩn điện áp tham chiếu 0V |

---

## 3. NGUYÊN LÝ HOẠT ĐỘNG TÍCH HỢP

### A. Phía bo STM32_JUMPSCARE:
- **3 Kênh công tắc khô (nối GND, INPUT_PULLUP):**
  - **SW1 (PA6)** $\rightarrow$ Kích Relay 1 (**PA3**) trong **71 giây** (1p11s).
  - **SW2 (PA7)** $\rightarrow$ Kích Relay 2 (**PA4**) trong **57 giây**.
  - **SW3 (PB12)** $\rightarrow$ Kích Relay 3 (**PB13**) trong **86 giây** (1p26s).
- **Khóa liên động (Interlock):**
  - Khi một kênh đang hoạt động, bấm nút kênh khác sẽ bị chặn hoàn toàn (chống kích đè).
  - Bấm lại chính nút của kênh đang chạy $\rightarrow$ **TẮT NGAY LẬP TỨC** (chế độ Toggle tắt khẩn cấp).
- **Phát tín hiệu UART sang bo Sound:**
  - Ngay khi kích hoạt relay $\rightarrow$ Bắn lệnh `'S'` (ngắt âm thanh nền) và lệnh phân loại kênh:
    + Kênh 1: Bắn lệnh **`T1`** (ứng với thời gian ngắt **71 giây**).
    + Kênh 2: Bắn lệnh **`T2`** (ứng với thời gian ngắt **57 giây**).
    + Kênh 3: Bắn lệnh **`T3`** (ứng với thời gian ngắt **86 giây**).
  - Khi tắt relay (hết giờ hoặc bấm tắt sớm) $\rightarrow$ Bắn lệnh `'P'` (báo bật lại âm thanh nền ngay).
  - Chu kỳ **Heartbeat 500ms**: Tự động phát liên tục `'S'` (nếu có kênh đang chạy) hoặc `'P'` (nếu nghỉ) để chống nhiễu hoặc mất gói tin.

### B. Phía bo STM32_SOUND:
- **Relay âm thanh nền (PA6) & Quản lý ngắt theo lệnh T1, T2, T3:**
  - Khi ở trạng thái rảnh (`soundEnabled == true`): Tự động chớp ngắt liên tục **500ms BẬT / 500ms TẮT**.
  - Khi nhận lệnh **`T1`**: Tắt ngay relay PA6 và định thời gian ngắt âm thanh nền đúng **71 giây**.
  - Khi nhận lệnh **`T2`**: Tắt ngay relay PA6 và định thời gian ngắt âm thanh nền đúng **57 giây**.
  - Khi nhận lệnh **`T3`**: Tắt ngay relay PA6 và định thời gian ngắt âm thanh nền đúng **86 giây**.
  - Sau khi hết thời gian tương ứng (hoặc nhận được lệnh `'P'` tắt sớm): Tự động mở lại relay PA6 chớp ngắt 500ms.
  - Khi nhận `T1`, `T2`, `T3`: Chuyển tiếp ngay lệnh `'T'` sang bo `STM32_RELAY` để tắt sạch hệ thống đèn phòng!
- **Đồng bộ Reset toàn hệ thống (Nút PB0 lần 2):**
  - Khi người chơi bấm nút Reset PB0 trên bo Sound: Bo Sound tự động bắn lệnh `'0'` sang cả `STM32_RELAY` lẫn `STM32_JUMPSCARE` $\rightarrow$ Toàn bộ 3 bo tắt sạch rơ-le đồng thời.

---

## 4. KẾT QUẢ BIÊN DỊCH PLATFORMIO
Cả 3 môi trường vi điều khiển đã biên dịch thành công 100%:
- `NAP_MACH_RELAY` $\rightarrow$ **SUCCESS** (RAM 7.1%, Flash 41.7%)
- `NAP_MACH_AM_THANH` $\rightarrow$ **SUCCESS** (RAM 6.5%, Flash 27.9%)
- `NAP_MACH_JUMPSCARE` $\rightarrow$ **SUCCESS** (RAM 6.3%, Flash 20.8%)
