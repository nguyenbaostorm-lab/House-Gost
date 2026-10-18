# 📖 README_JUMPSCARE: BO STM32 PHỤ THỨ 3 (RELAY CONTROLLER WITH TIMER & TOGGLE)

---

## 1. SƠ ĐỒ CHÂN: 3 CÔNG TẮC & 3 RELAY

Cấu hình công tắc: **Tiếp điểm khô nối xuống GND** (Dùng điện trở kéo lên nội bộ `INPUT_PULLUP`).  
Mức kích Relay: **Active HIGH** (`HIGH` = Bật / Đóng tiếp điểm, `LOW` = Tắt).

| Kênh | Chân Công Tắc (Input Pullup) | Chân Relay (Output) | Thời Gian Tự Động Tắt (Timer) | Chức Năng |
| :---: | :---: | :---: | :---: | :--- |
| **Kênh 1** | **`PA6`** (A6 nối GND) | **`PA3`** (A3) | **1 phút 11 giây** (71s = 71000ms) | Bấm lần 1: BẬT & đếm 71s tự tắt. Bấm tiếp: TẮT NGAY LẬP TỨC |
| **Kênh 2** | **`PA7`** (A7 nối GND) | **`PA4`** (A4) | **57 giây** (57s = 57000ms) | Bấm lần 1: BẬT & đếm 57s tự tắt. Bấm tiếp: TẮT NGAY LẬP TỨC |
| **Kênh 3** | **`PB12`** (B12 nối GND) | **`PB13`** (B13) | **1 phút 26 giây** (86s = 86000ms) | Bấm lần 1: BẬT & đếm 86s tự tắt. Bấm tiếp: TẮT NGAY LẬP TỨC |

---

## 2. CỔNG GIAO TIẾP UART VỚI BO STM32_SOUND (B5 TX & B6 RX)
Tốc độ truyền thông: **9600 baud**:

| Chân trên BO JUMPSCARE | Hướng tín hiệu | Chân trên BO SOUND | Ý nghĩa |
| :---: | :---: | :---: | :--- |
| **`PB5`** (B5 - TX) | ➔ | **`PB11`** (B11 - RX) | Jumpscare gửi trạng thái sang Bo Sound ('1', '2', '3') |
| **`PB6`** (B6 - RX) | ⬅️ | **`PB10`** (B10 - TX) | Nhận lệnh kích từ Bo Sound ('1', '2', '3', 'J', '0') |
| **`GND`** | 🟰 | **`GND`** | **Nối chung Mass GND toàn hệ thống** |

---

## 3. ĐÈN BÁO TRẠNG THÁI (HEARTBEAT) TRÊN BO:
* **Chân `PC13`:** 
  * Nhấp nháy chậm **500ms**: Bo đang chạy bình thường ở chế độ chờ.
  * Nhấp nháy nhanh **100ms**: Đang có ít nhất 1 relay được bật hoạt động.
