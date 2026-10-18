# 📖 README_SOUND: BO STM32 PHỤ (7 RELAY & 6 NÚT BẤM)

---

## 1. 7 CHÂN RELAY TRÊN BO (PA0 ➔ PA6)
Chuẩn kích **Active-HIGH** (Mức `HIGH` = Bật / Đóng rơ-le; Mức `LOW` = Tắt / Ngắt rơ-le):

| Kênh Relay | Chân STM32 | Mức Kích Hoạt | Chức Năng Âm Thanh / Thiết Bị |
| :---: | :---: | :---: | :--- |
| **Relay 1** | **`PA0` (A0)** | `HIGH` = Bật / `LOW` = Tắt | 🎵 **ÂM THANH 1:** Theme (Nhạc nền hoạt cảnh) |
| **Relay 2** | **`PA1` (A1)** | `HIGH` = Bật / `LOW` = Tắt | 🗣️ **ÂM THANH 2:** Trò chuyện bảo vật |
| **Relay 3** | **`PA2` (A2)** | `HIGH` = Bật / `LOW` = Tắt | 🚪 **ÂM THANH 3:** Tiếng kẽo kẹt |
| **Relay 4** | **`PA3` (A3)** | `HIGH` = Bật / `LOW` = Tắt | 😱 **ÂM THANH HÙ RÉ / ĐỪNG BỎ EM 1 MÌNH (CHÂN A3)** (Kích hoạt bởi RF433 R1 trong 10s, hoặc lệnh 'J' trong 1 phút) |
| **Relay 5** | **`PA4` (A4)** | `HIGH` = Bật / `LOW` = Tắt | 👶 **ÂM THANH 5 / CHÂN A4** (Bật 30s sau khi chờ 5s từ lệnh 'J') |
| **Relay 6** | **`PA5` (A5)** | `HIGH` = Bật / `LOW` = Tắt | 🌌 **ÂM THANH 6:** SCRA_NIGHT (Đêm kinh dị) |
| **Relay 7** | **`PA6` (A6)** | `HIGH` = Bật / `LOW` = Tắt | 🏆 **ÂM THANH 7:** WIN CHIẾN THẮNG |

---

## 2. 6 CHÂN TÍN HIỆU ĐẦU VÀO & NÚT BẤM (PB0, PB5, PB6, PB7, PB8, PB9)
Cấu hình chế độ kéo lên nội bộ **`INPUT_PULLUP`** (Một đầu nối chân STM32, đầu còn lại nối mass **`GND`** hoặc ngõ ra module thu RF433):

| Tín Hiệu Đầu Vào | Chân STM32 | Kết Nối Ngoại Vi | Chức Năng Kích Hoạt |
| :---: | :---: | :--- | :--- |
| **Nút 1 (BTN1)** | **`PB0`** | Nút bấm cơ (nối GND) | **START / RESET TOÀN BỘ 2 BO STM32 (CHUẨN V5.3):** Lọc chống dội FSM 30ms phi chặn (Non-blocking). Bấm 1 lần ăn ngay 100%! Bấm lần 1: START kịch bản (Bật A0, gửi 'S'). Bấm lần 2: RESET CỨNG TOÀN DIỆN CẢ 2 CON STM32 (Tắt sạch 7 relay âm thanh, xóa Flash checkpoint, gửi '0' sang Relay, cả 2 con cùng thực hiện NVIC_SystemReset() reboot về trạng thái ban đầu sạch sẽ 100%). |
| **Nút 2 (BTN2)** | **`PB5`** | Nút bấm cơ (nối GND) | 🎵 **Trong mọi trường hợp: Bấm 1 lần BẬT chân A5 (Nhạc scralight), kích 1 lần nữa là TẮT** |
| **Nút 3 (RB6)** | **`PB6`** | Nút bấm cơ (nối GND) | 🔘 **Bất kỳ khi nào được bật: BẬT Relay ở chân A5 (Relay 6) ON và GỬI LỆNH 'L' SANG BO STM32_RELAY** |
| **Nút 4 (BTN4)** | **`PB1`** *(Chân B1)* | Cảm biến / Nút nối GND | 🏆 **NHẠC CHIẾN THẮNG:** Nhận tín hiệu kích âm GND (Mức LOW). Lọc chống nhiễu 5 mẫu liên tiếp 50ms (chuẩn chân `PA6` của bo `STM32_RELAY`). Bấm lần 1: BẬT Relay A6 (PA6), TẮT toàn bộ A0..A5, GỬI lệnh 'V' đến bo Relay. Bấm lần 2: TẮT Relay A6. Bấm bất cứ lúc nào! *(Chân B1 nằm ngay sát cạnh chân nút Start B0)* |
| **Nút 5 (BTN5)** | **`PB8`** | Nút bấm cơ (nối GND) | (Dự phòng - Đã gỡ bỏ tính năng khôi phục Flash khi cúp điện) |
| **Tín hiệu RF433 (R1)** | **`PB9`** | **Nối chân `R1` module RF433** | 😱 **Bật âm thanh Hù Ré ở chân A3 (Relay 4) trong vòng 10s** (Luôn luôn hoạt động bất kể thời gian nào) |

> 📡 **ĐẤU NỐI MẠCH RF433 (KÊNH R1):**
> * Sử dụng chân **`PB9`** trên vi điều khiển STM32 làm chân nhận tín hiệu **Digital Input** từ mạch **RF433**.
> * Nối trực tiếp ngõ ra **`R1`** của mạch RF433 vào chân **`PB9`**.
> * Khi nhận tín hiệu từ kênh R1 của remote RF433, hệ thống sẽ **BẬT ÂM THANH HÙ RÉ / ĐỪNG BỎ EM 1 MÌNH Ở CHÂN A3 TRONG VÒNG 10 GIÂY** (hoạt động độc lập liên tục bất kể thời gian nào).
> * **ĐẶC BIỆT (CHUẨN V5.2):** 
>   1. Khi bật đoạn hội thoại trò chuyện `A1`, **RELAY A0 VẪN TIẾP TỤC BẬT SONG SONG** (nhạc nền Theme du dương cùng lời thoại).
>   2. Nhạc Theme `A0` **CHỈ TẮT KHI NHẠC "ĐỪNG BỎ EM 1 MÌNH" (CHÂN A3) XẢY RA**, và **TỰ ĐỘNG BẬT LẠI NGAY LẬP TỨC SAU ĐÓ** khi bài A3 kết thúc!

---

## 3. CỔNG UART GIAO TIẾP VỚI BO STM32 RELAY
Giao tiếp truyền thông nối tiếp 2 chiều tốc độ **9600 baud**:

| Chân trên Bo này | Chức Năng UART | Kết Nối Với Bo STM32_RELAY |
| :---: | :---: | :--- |
| **`12` (`PB12`)** | **TX** | Nối sang chân **`PA3` (RX)** của bo `STM32_RELAY` |
| **`13` (`PB13`)** | **RX** | Nối sang chân **`PA2` (TX)** của bo `STM32_RELAY` |
| **`GND`** | **Mass** | **Nối chung mass GND toàn hệ thống** |

*(Ghi chú: Trên bo này, cổng giao tiếp sử dụng chân TX là 12 (`PB12`) và RX là 13 (`PB13`)).*

* **Kịch bản xử lý lệnh UART nhận từ bo STM32_RELAY:**
  * **Lệnh `'D'`:** Đang bật A0 (Nhạc nền) ➔ **BẬT THÊM Relay `A1` (`PA1` - Trò chuyện trên gác)** trong vòng **45 giây** (`THOI_GIAN_BAT_A1_MS = 45000ms`), sau đó **TẮT HẲN không bật nữa**.
    * **Sau khi tắt A1 (đủ 45s): Lập tức GỬI LỆNH `'F'`** sang bo Relay (`guiLenhRelay('F');`).
  * **Lệnh `'G'` (Nhận bất cứ lúc nào):** Bo Sound duy trì bật A0, **BẬT THÊM Relay `A2` (`PA2` - Tiếng kẽo kẹt vang lên)**, đồng thời gửi lệnh `'H'` sang bo `STM32_RELAY`.
  * **Lệnh `'J'`:** Kích hoạt chuỗi kịch bản:
    1. **BẬT Relay ở chân `A3`** (`PA3` - bài "Đừng bỏ em 1 mình") trong vòng **53 giây** (giảm 7s từ 60s), đồng thời tạm tắt A0.
    2. Hết bài (sau 53 giây): **TẮT Relay `A3`**, **GỬI LỆNH `'U'` SANG BO `STM32_RELAY`** (chuyển ngay lập tức từ loa P2 sang 2 loa P0 & P1), bật lại nhạc Theme A0, và bắt đầu chờ **5 giây**.
    3. Sau 5 giây chờ: **BẬT âm thanh ở chân `A4`** (`PA4`) trong vòng **15 giây**.
    4. Sau 15 giây: **TẮT HẲN** chân `A4` và **GỬI LỆNH `'K'` SANG BO `STM32_RELAY`**.
  * **Lệnh `'0'`:** Dừng khẩn cấp toàn bộ rơ-le.

---

## 4. TÍNH NĂNG NÚT NHẤN RB6 (PB6) & RELAY A5 (PA5 / RELAY 6)
* **Cấu hình chân:** Chân `PB6` (RB6) nối 1 đầu nút nhấn, đầu còn lại nối mass `GND` (chế độ `INPUT_PULLUP`).
* **Kịch bản hoạt động:**
  - Bất kỳ khi nào nút ở chân **`RB6` (PB6)** được bật (nhấn nối GND):
    1. **BẬT Relay ở chân `A5` (PA5 / Relay 6) LÊN ON**.
    2. **GỬI LỆNH `'L'` ĐẾN CON `STM32_RELAY`** qua chân UART PB12 (TX).
  - Hoạt động độc lập liên tục ở mọi thời điểm (kể cả trước hay sau khi nhấn PB0).

---

## 5. LUỒNG XỬ LÝ RIÊNG BIỆT CHÂN PB7 (BTN4 / CẢM BIẾN - NHẠC CHIẾN THẮNG & LỆNH 'V')
*(Chuẩn thiết kế và thuật toán lọc nhiễu tham khảo tương đương chân PA6 trên bo STM32_RELAY)*

| Chân STM32 | Đầu Nối Ngoại Vi | Kiểu Chân (I/O) | Chức Năng |
| :---: | :---: | :---: | :--- |
| **`PB7`** | Cảm biến / Nút nối GND | Input Pull-Up (kéo lên nội bộ) | Nhận tín hiệu kích âm GND (Mức LOW) |
| **`GND`** | Nối chung mass GND | Mass | Cực âm kích chân PB7 về mức LOW (0V) |

* **Quy trình bảo vệ và lọc nhiễu phần cứng/phần mềm:**
  1. **Bảo vệ khởi động an toàn (3 giây):** Sau 3000ms khởi động ổn định nguồn, hệ thống kiểm tra chân `PB7` phải đang ở mức nghỉ `HIGH` an toàn thì cờ `pb7DaSanSang` mới được bật sẵn sàng đón tín hiệu.
  2. **Lọc chống dội kỹ qua 5 mẫu liên tiếp 50ms (`kiemTraNutNhanPB7()`):** Đọc liên tục 5 lần cách nhau 10ms xác nhận mức `LOW` thực sự để loại bỏ hoàn toàn mọi xung gai điện từ / nhiễu nguồn.
* **Kịch bản hoạt động (Chế độ Toggle Bật/Tắt - Hoạt động độc lập, kích hoạt bất cứ lúc nào):**
  - **Kích lần 1 (BẬT):**
    1. **BẬT CHÂN RELAY `A6`** (`PA6` / Relay 7: Nhạc chiến thắng).
    2. **ĐỒNG THỜI TẮT TOÀN BỘ CÁC CHÂN RELAY CÒN LẠI:** `A0`, `A1`, `A2`, `A3`, `A4`, `A5`.
    3. **GỬI LỆNH `'V'` ĐẾN CON `STM32_RELAY`** qua chân UART PB12 (TX).
    4. Khóa chế độ phát nhạc chiến thắng độc quyền, loại trừ xung đột với các kịch bản âm thanh khác.
  - **Kích lần 2 (TẮT HỆ THỐNG VÀ ĐƯA VỀ BAN ĐẦU):**
    1. **TẮT HẲN chân Relay `A6`** (Dừng nhạc chiến thắng) và toàn bộ các relay âm thanh.
    2. **GỬI TIẾP LỆNH `'V'` SANG BO STM32_RELAY** để bo Relay cũng tắt toàn bộ hệ thống và đưa về trạng thái ban đầu chờ lệnh S.
    3. Đưa toàn bộ bo Sound về trạng thái ban đầu (sẵn sàng chờ nhấn `PB0` để bắt đầu lượt chơi mới).
    4. Đèn PC13 nháy chậm bình thường.

---

## 6. KHÔNG LƯU TRẠNG THÁI KHI CÚP ĐIỆN (ĐÃ GỠ BỎ FLASH CHECKPOINT)

* **Trạng thái khi có điện lại:**
  - Đã gỡ bỏ hoàn toàn module `flash_checkpoint.h` trên bo Sound.
  - Khi cúp điện hoặc có điện lại: Hệ thống khởi động mới sạch sẽ từ đầu, toàn bộ relay giữ tắt an toàn chờ nút Start `PB0` để bắt đầu lượt mới.
  - Không cần lưu và không lưu mốc khi cúp điện.


