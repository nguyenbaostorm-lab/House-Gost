# HƯỚNG DẪN TEST NẠP CODE STM32F103C8T6 (ĐÈN LED PC13)

Chương trình này dùng để kiểm tra quá trình nạp code và giao tiếp Serial:
- Khi gửi ký tự `'1'` từ Serial Monitor ➔ **Đèn LED PC13 BẬT SÁNG**.
- Khi gửi ký tự `'0'` từ Serial Monitor ➔ **Đèn LED PC13 TẮT**.

---

## 1. NẠP CODE VÀO STM32 BẰNG ST-LINK V2
Đấu 4 dây từ ST-Link V2 vào cổng nạp 4 chân trên đuôi bo STM32 Blue Pill:

| ST-Link V2 | Chân STM32 |
| :--- | :--- |
| **GND** | **GND** |
| **3.3V** | **3.3V** |
| **SWDIO** | **DIO (PA13)** |
| **SWCLK** | **CLK (PA14)** |

---

## 2. KẾT NỐI VỚI MÁY TÍNH ĐỂ GỬI LỆNH (MẠCH USB-TO-UART CH340 / CP2102)
Để mở Serial Monitor trên máy tính gửi số `'1'` và `'0'`:

| Mạch USB-UART (CH340/CP2102) | Chân trên STM32 Blue Pill |
| :--- | :--- |
| **TX** | **PA10 (RX)** |
| **RX** | **PA9 (TX)** |
| **GND** | **GND** (Nối chung mass) |

* Cài đặt Serial Monitor (trên VS Code / Arduino IDE / Tera Term / PuTTY):
  * **Baudrate:** `9600`
  * **Gõ phím `1` rồi nhấn Enter:** Đèn PC13 sáng rực.
  * **Gõ phím `0` rồi nhấn Enter:** Đèn PC13 tắt.

---

## 3. NGUYÊN LÝ ĐÈN PC13 TRÊN STM32 BLUE PILL
* Đèn LED PC13 trên bo STM32 hoạt động theo chuẩn **Active-LOW**:
  * `digitalWrite(PC13, LOW)`  ➔ **LED SÁNG**.
  * `digitalWrite(PC13, HIGH)` ➔ **LED TẮT**.
