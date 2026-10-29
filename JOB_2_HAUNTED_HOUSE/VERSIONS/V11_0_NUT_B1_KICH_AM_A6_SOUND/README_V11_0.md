# PHIÊN BẢN V11.0 - CHÂN B1 (PB1) STM32_SOUND KÍCH ÂM GND TẮT A0..A5 VÀ CHỈ BẬT A6

Phiên bản **V11.0** kế thừa toàn bộ tính năng ổn định của **V10.0**, bổ sung thêm tính năng điều khiển trực tiếp trên bo **STM32_SOUND**:

---

## 1. MỤC ĐÍCH & Ý NGHĨA KỸ THUẬT V11.0

* **Yêu cầu:** Khi chân **`PB1` (B1)** của bo **STM32_SOUND** nhận được tín hiệu kéo xuống **`GND`** (dùng GND để nhấn nút):
  * **TẮT HOÀN TOÀN** các chân âm thanh: **`A0`, `A1`, `A2`, `A3`, `A4`, `A5`**.
  * **CHỈ BẬT DUY NHẤT** chân: **`A6`** (Relay 7 - Âm thanh chiến thắng).
  * **Cơ chế chống nhiễu:** Để tránh kích nhầm do xung gai điện từ môi trường dây nối dài nhưng không gây cảm giác trễ quá lâu, tín hiệu GND được xác nhận liên tục trong **`150ms` (0.15 giây - 15 mẫu x 10ms)**.
  * **Bảo vệ khởi động:** Sau 3 giây cấp nguồn ổn định, chân `PB1` phải ở trạng thái nghỉ `HIGH` mới sẵn sàng nhận lệnh.
  * **Đảo trạng thái (Toggle an toàn):**
    * Nhấn lần 1: Tắt A0..A5, BẬT A6, gửi lệnh `'V'` sang STM32_RELAY.
    * Nhấn lần 2: TẮT A6 và đưa hệ thống về trạng thái ban đầu chờ khởi động mới.

---

## 2. SƠ ĐỒ ĐẤU NỐI CHÂN PB1

| Chân STM32_SOUND | Kiểu Cấu Hình | Đấu Nối Ngoại Vi | Chức Năng |
| :---: | :---: | :--- | :--- |
| **`PB1` (B1)** | `INPUT_PULLUP` (Kéo lên nội 3.3V) | Nối vào 1 đầu tiếp điểm nút nhấn / cảm biến | Nhận tín hiệu kích âm GND (Mức LOW) |
| **`GND`** | Mass chung | Nối vào đầu tiếp điểm còn lại của nút nhấn | Cực âm để nhấn kích chân PB1 về mức LOW |

---

## 3. CÁC FILE ĐƯỢC CẬP NHẬT TRONG V11.0
- `STM32_SOUND/config.h`: Khai báo chân `PIN_BTN_PB1` (`PB1`) và thời gian lọc chống nhiễu `THOI_GIAN_LOC_NHIEU_PB1_MS = 150`.
- `STM32_SOUND/sound_driver.h`: Cấu hình `pinMode(PB1, INPUT_PULLUP)` và hàm `kiemTraNutNhanPB1()` lọc liên tục 150ms.
- `STM32_SOUND/main.cpp`: Tích hợp cờ bảo vệ `pb1DaSanSang` và luồng xử lý ưu tiên: tắt A0..A5, chỉ bật A6, gửi đồng bộ UART `'V'` sang bo Relay.
- Bản code làm việc trực tiếp tại `JOB_2_HAUNTED_HOUSE/STM32_SOUND` cũng được đồng bộ tương ứng.

---

## 4. DANH SÁCH FILE BIN ĐÃ BIÊN DỊCH (2 LOẠI THỜI GIAN & JUMPSCARE)

Thư mục lưu trữ: `JOB_2_HAUNTED_HOUSE/VERSIONS/V11_0_NUT_B1_KICH_AM_A6_SOUND/BIN/` và `BIN_CODE/V11_0_NUT_B1_KICH_AM_A6_SOUND/`

### ⏰ Loại 1: BẢN GIỜ MỚI - ĐÚNG GIỜ (Chạy thực tế)
* **`STM32_RELAY_V11_0_GIO_MOI_3P.bin`** (và bản mặc định `STM32_RELAY_V11_0.bin`):
  * Chuyển pha: **3 phút 0 giây** (180s) sau lệnh 'S'.
  * Kế thừa đầy đủ tính năng dự phòng PA4 cho bộ gõ PA5.
* **`STM32_SOUND_V11_0_GIO_MOI_3P43S.bin`** (và bản mặc định `STM32_SOUND_V11_0.bin`):
  * Thoại A1: **3 phút 43 giây** (223s) sau lệnh 'D' rồi gửi 'F'.
  * Hỗ trợ nút PB1 kích âm GND: Tắt A0..A5, chỉ bật A6.

### ⚡ Loại 2: BẢN GIỜ CŨ - SAI GIỜ (Test nhanh)
* **`STM32_RELAY_V11_0_GIO_CU_10S.bin`**:
  * Chuyển pha: **10 giây** sau lệnh 'S'.
* **`STM32_SOUND_V11_0_GIO_CU_15S.bin`**:
  * Thoại A1: **15 giây** sau lệnh 'D' rồi gửi 'F'.
  * Hỗ trợ nút PB1 kích âm GND: Tắt A0..A5, chỉ bật A6.

### 👻 Mạch Jumpscare
* **`STM32_JUMPSCARE_V11_0.bin`**:
  * Biên dịch chuẩn cho bo phát âm thanh hù Jumpscare qua UART3 / DFPlayer Mini theo 3 chân kích Relay (PA1, PA2, PA3).

