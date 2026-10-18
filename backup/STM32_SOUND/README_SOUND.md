# 📖 README_SOUND: BO STM32 PHỤ (7 RELAY & 6 NÚT BẤM)

---

## 1. 7 CHÂN RELAY TRÊN BO (PA0 ➔ PA6)
Chuẩn kích **Active-HIGH** (Mức `HIGH` = Bật / Đóng rơ-le; Mức `LOW` = Tắt / Ngắt rơ-le):

| Kênh Relay | Chân STM32 | Mức Kích Hoạt | Trạng Thái |
| :---: | :---: | :---: | :--- |
| **Relay 1** | **`PA0`** | `HIGH` = Bật / `LOW` = Tắt | Tiếp điểm điều khiển thiết bị 1 |
| **Relay 2** | **`PA1`** | `HIGH` = Bật / `LOW` = Tắt | Tiếp điểm điều khiển thiết bị 2 |
| **Relay 3** | **`PA2`** | `HIGH` = Bật / `LOW` = Tắt | Tiếp điểm điều khiển thiết bị 3 |
| **Relay 4** | **`PA3`** | `HIGH` = Bật / `LOW` = Tắt | Tiếp điểm điều khiển thiết bị 4 |
| **Relay 5** | **`PA4`** | `HIGH` = Bật / `LOW` = Tắt | Tiếp điểm điều khiển thiết bị 5 |
| **Relay 6** | **`PA5`** | `HIGH` = Bật / `LOW` = Tắt | Tiếp điểm điều khiển thiết bị 6 |
| **Relay 7** | **`PA6`** | `HIGH` = Bật / `LOW` = Tắt | Tiếp điểm điều khiển thiết bị 7 |

---

## 2. 6 CHÂN NÚT BẤM CƠ (PB0, PB5, PB6, PB7, PB8, PB9)
Cấu hình chế độ kéo lên nội bộ **`INPUT_PULLUP`** (Một đầu nối chân STM32, đầu còn lại nối thẳng vào mass **`GND`**):

| Nút Bấm | Chân STM32 | Nối Mass | Chức Năng Kích Hoạt |
| :---: | :---: | :---: | :--- |
| **Nút 1 (BTN1)** | **`PB0`** | **`GND`** | Kích hoạt kịch bản 1 / Bật Relay 1 / Báo lệnh sang bo Relay |
| **Nút 2 (BTN2)** | **`PB5`** | **`GND`** | Kích hoạt kịch bản 2 / Bật Relay 2 / Báo lệnh sang bo Relay |
| **Nút 3 (BTN3)** | **`PB6`** | **`GND`** | Kích hoạt kịch bản 3 / Bật Relay 3 / Báo lệnh sang bo Relay |
| **Nút 4 (BTN4)** | **`PB7`** | **`GND`** | Kích hoạt kịch bản 4 / Bật Relay 4 / Báo lệnh sang bo Relay |
| **Nút 5 (BTN5)** | **`PB8`** | **`GND`** | Kích hoạt kịch bản 5 / Bật Relay 5 / Báo lệnh sang bo Relay |
| **Nút 6 (BTN6)** | **`PB9`** | **`GND`** | Kích hoạt ngắt toàn bộ / Tắt hết Relay |

---

## 3. CỔNG UART GIAO TIẾP VỚI BO STM32 RELAY
Giao tiếp truyền thông nối tiếp 2 chiều tốc độ **9600 baud**:

| Chân trên Bo này | Chức Năng UART | Kết Nối Với Bo STM32_RELAY |
| :---: | :---: | :--- |
| **`12` (`PB12`)** | **TX** | Nối sang chân **`PA3` (RX)** của bo `STM32_RELAY` |
| **`13` (`PB13`)** | **RX** | Nối sang chân **`PA2` (TX)** của bo `STM32_RELAY` |
| **`GND`** | **Mass** | **Nối chung mass GND toàn hệ thống** |

*(Ghi chú: Trên bo này, cổng giao tiếp sử dụng chân TX là 12 (`PB12`) và RX là 13 (`PB13`)).*
