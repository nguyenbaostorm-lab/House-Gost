# 📖 README_RELAY: KẾT NỐI MẠCH MỞ RỘNG I2C PCF8575

---

## 1. SƠ ĐỒ ĐẤU DÂY I2C PHẦN MỀM VỚI MODULE PCF8575

| Chân STM32 | Chân PCF8575 | Chức Năng |
| :---: | :---: | :--- |
| **`PB12`** | **`SCL`** | Chân xung nhịp clock I2C (mô phỏng phần mềm) |
| **`PB13`** | **`SDA`** | Chân dữ liệu data I2C (mô phỏng phần mềm) |
| **`3.3V / 5V`** | **`VCC`** | Cấp nguồn cho module PCF8575 |
| **`GND`** | **`GND`** | Nối chung mass GND |

---

## 2. PHÂN BỔ 17 CHÂN MODULE PCF8575
 
* **Các chân đang sử dụng (`P0` ➔ `P14`):** Tổng cộng **15 chân** dùng điều khiển 15 Relay.
* **Các chân dự trù:**
  * **`P15`:** 1 chân I/O còn lại dự trù để mở rộng thêm Relay hoặc thiết bị tải sau này.
  * **`INT`:** Chân tín hiệu ngắt (Interrupt) dự trù.

---

## 3. CỔNG GIAO TIẾP UART (USART2 PHẦN CỨNG)

| Chân STM32 | Chức Năng | Kết Nối Ngoại Vi |
| :---: | :---: | :--- |
| **`PA2`** | **TX** (USART2_TX) | Truyền lệnh điều khiển sang bo STM32 Sound |
| **`PA3`** | **RX** (USART2_RX) | Nhận dữ liệu trạng thái phản hồi từ bo STM32 Sound |
| **`GND`** | **Mass** | Nối chung mass GND toàn hệ thống |
