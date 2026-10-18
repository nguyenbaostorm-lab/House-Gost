# 📖 README_RELAY: KẾT NỐI MẠCH MỞ RỘNG I2C PCF8575

---

## 1. SƠ ĐỒ ĐẤU DÂY I2C PHẦN MỀM VỚI MODULE PCF8575

|       Chân STM32       |   Chân PCF8575   | Chức Năng                                        |
| :---------------------: | :---------------: | :------------------------------------------------- |
|   **`PB12`**   | **`SCL`** | Chân xung nhịp clock I2C (mô phỏng phần mềm) |
|   **`PB13`**   | **`SDA`** | Chân dữ liệu data I2C (mô phỏng phần mềm)   |
| **`3.3V / 5V`** | **`VCC`** | Cấp nguồn cho module PCF8575                     |
|    **`GND`**    | **`GND`** | Nối chung mass GND                                |

---

## 2. PHÂN BỔ CHI TIẾT CÁC CHÂN TRÊN MODULE MỞ RỘNG PCF8575

### 📌 A. Bảng 15 kênh rơ-le điều khiển đầu ra (Digital Output: P0 ➔ P14)

| Ký Hiệu In Trên Bo | Bit Trong Code | Kênh Rơ-le | Kiểu Chân (I/O) | Thiết Bị / Cơ Cấu Tải Kết Nối | Trạng Thái / Kịch Bản Hoạt Động | Ghi Chú / Thông Số | Trạng Thái Xác Nhận |
| :---: | :---: | :---: | :---: | :--- | :--- | :--- | :---: |
| **`P0`** (P00) | Bit 0 | Relay 0 | Output | 🔊 **LOA TỔNG** | Bật khi nhận 'S' (trong 30s quan sát), hết 30s tắt để chuyển sang P1 | Kênh công suất loa tổng | ✅ **Đã xác nhận đúng** |
| **`P1`** (P01) | Bit 1 | Relay 1 | Output | 🗣️ **LOA TRÊN GÁC** | Bật sau 30s từ lệnh S (khi gửi D), tiếp tục bật khi nhận F | Kênh loa trên gác | ✅ **Đã xác nhận đúng** |
| **`P2`** (P02) | Bit 2 | Relay 2 | Output | 🎶 **LOA HIỆU ỨNG** | Kênh loa hiệu ứng | Kênh loa hiệu ứng | ✅ **Đã xác nhận đúng** |
| **`P3`** (P03) | Bit 3 | Relay 3 | Output | 💡 **RELAY_LED4** | Bật khi nhận 'S' (30s quan sát), BẬT THÊM khi nhận 'F', và BẬT LẠI sau 50s từ lệnh 'H' | LED quan sát | ✅ **Đã xác nhận đúng** |
| **`P4`** (P04) | Bit 4 | Relay 4 | Output | 📺 **RELAY_4: ĐÈN TV** | Kênh điều khiển hiệu ứng đèn TV | Đèn TV | ⏳ **Chưa xác nhận** |
| **`P5`** (P05) | Bit 5 | Relay 5 | Output | 🧸 **LED ĐỒ CHƠI** | Bật khi nhận lệnh 'F', tắt khi chân A6 nhận tín hiệu (gửi 'G') | Relay 5 (Hiệu ứng LED đồ chơi) | ✅ **Đã xác nhận đúng** |
| **`P6`** (P06) | Bit 6 | Relay 6 | Output | 🚪 **RELAY_6: CỬA TỪ 1** | Bật khi chân A6 nhận tín hiệu -> **KHÔNG BAO GIỜ TẮT LẠI!** | Cửa từ 1 (Chốt luôn mở) | ✅ **Đã xác nhận đúng** |
| **`P7`** (P07) | Bit 7 | Relay 7 | Output | 🖼️ **RELAY_7: ĐÈN CHIẾU BỨC TRANH** | Bật sau 5s kể từ khi nhận lệnh 'H' | Đèn chiếu bức tranh | ⏳ **Chưa xác nhận** |
| **`P10`** | Bit 8 | Relay 8 / 10 | Output | 🐉 **RELAY_10: ĐÈN RỒNG PHƯỢNG** | Bật sau 45s kể từ khi bật Relay 7 (50s từ lệnh 'H') | Kênh Đèn Rồng Phượng | ⏳ **Chưa xác nhận** |
| **`P11`** | Bit 9 | Relay 11 | Output | 🪆 **RELAY_11: BÚP BÊ RƠI XUỐNG** | Kênh điều khiển cơ cấu nhả búp bê rơi xuống | Cơ cấu búp bê rơi | ✅ **Đã xác nhận đúng** |
| **`P12`** | Bit 10 | Relay 12 | Output | 💡 **RELAY_12: ĐÈN LED CHỖ CUỐN SỔ** | Kênh điều khiển đèn LED chiếu chỗ cuốn sổ | Đèn LED cuốn sổ | ⏳ **Chưa xác nhận** |
| **`P13`** | Bit 11 | Relay 13 | Output | 🔴 **RELAY_13: ĐÈN ĐỎ** | Bật khi nhấn giữ remote R4, nhả ra thì tắt / Bật khi nhận lệnh K (sau 30s tắt) | Kênh RF R4 (Đèn đỏ) | ⏳ **Chưa xác nhận** |
| **`P14`** | Bit 12 | Relay 14 | Output | 🔨 **RELAY_14: BỘ GÕ** | Kênh điều khiển relay tác động bộ gõ | Bộ gõ | ⏳ **Chưa xác nhận** |

---

### 📌 B. Bảng các chân nhận tín hiệu đầu vào (Digital Input: P15 ➔ P17 & INT)

| Chân PCF8575 | Kiểu Chân (I/O) | Kết Nối Ngoại Vi | Chức Năng Kích Hoạt | Ghi Chú / Kịch Bản |
| :---: | :---: | :--- | :--- | :--- |
| **`P15`** | Input | Chân **`R2`** module thu RF433 | 🛑 **TẮT TẤT CẢ RELAY TỪ P4 ĐẾN P14 SẼ OFF TOÀN BỘ** | Bất kỳ trường hợp nào cũng hoạt động (P6 nếu đã bật vẫn giữ) |
| **`P16`** | Input | Chân **`R3`** module thu RF433 | ⚡ **CHỚP TẮT RELAY 13, RELAY 7 VÀ RELAY 10** | Chớp tắt liên tục (150ms) trong lúc nhận tín hiệu R3 |
| **`P17`** | Input | Chân **`R4`** module thu RF433 | 🔘 **RELAY 13 BẬT KHI NHẤN, KHÔNG NHẤN NỮA THÌ TẮT** | Giữ nút bật Relay 13 (P13), nhả nút tự ngắt (Momentary) |
| **`INT`** | Open-Drain | Chân ngắt dự trù | Tín hiệu ngắt báo thay đổi trạng thái Input | |

---

## 3. CỔNG GIAO TIẾP UART (USART2 PHẦN CỨNG)

|    Chân STM32    |       Chức Năng       | Kết Nối Ngoại Vi                                        |
| :---------------: | :----------------------: | :--------------------------------------------------------- |
| **`PA2`** | **TX** (USART2_TX) | Truyền lệnh điều khiển sang bo STM32 Sound            |
| **`PA3`** | **RX** (USART2_RX) | Nhận dữ liệu trạng thái phản hồi từ bo STM32 Sound |
| **`GND`** |      **Mass**      | Nối chung mass GND toàn hệ thống                       |

---

## 4. LUỒNG XỬ LÝ RIÊNG BIỆT CHÂN PA6 (A6 NỐI ÂM GND)
|      Chân STM32      |      Đầu Nối Ngoại Vi      |         Kiểu Chân (I/O)         | Chức Năng                                               |
| :--------------------: | :------------------------: | :--------------------------------: | :-------------------------------------------------------- |
| **`PA6` (A6)** | Cảm biến / Nút nối GND | Input Pull-Up (kéo lên nội bộ) | Nhận tín hiệu kích âm GND (Mức LOW)                 |
|   **`GND`**   | Nối chung mass GND |                Mass                | Cực âm kích chân PA6 về mức LOW (0V) |

* **Kịch bản hoạt động:**
  - Luồng xử lý độc lập riêng biệt, **CHỈ NHẬN ĐÚNG 1 LẦN DUY NHẤT** (sau đó có kích lại hoặc giữ tiếp cũng bỏ qua không nhận nữa).
  - Khi chân `PA6` nhận tín hiệu nối âm GND (Mức LOW / 0V) (đặt đồ chơi vào vị trí cảm biến):
    1. **BẬT Relay `P6`** (Cửa từ 1 mở cửa) ➔ **CHỐT: P6 MỘT KHI ĐÃ BẬT THÌ KHÔNG BAO GIỜ TẮT LẠI!**
    2. **BẬT ĐÈN PHÒNG `P3`** (Đèn chiếu sáng chính) và **DUY TRÌ BẬT `P5`** (Đèn đồ chơi).
    3. **GỬI LỆNH `'G'`** sang bo **`STM32_SOUND`** (kịch bản flow tiếp theo vẫn chạy hoàn toàn bình thường).

---

## 5. KỊCH BẢN TỰ ĐỘNG KHI NHẬN LỆNH 'H' TỪ BO SOUND
* **Bước 1 (Khi nhận 'H'):** `P6` đã mở cửa từ trước và vẫn luôn giữ bật.
* **Bước 2 (Sau 5 giây):** BẬT thêm **Relay 7 (`P7` - Đèn chiếu bức tranh)**.
* **Bước 3 (Sau đó 45 giây - Tổng 50s từ lệnh 'H'):** BẬT thêm **Relay `P10` (Đèn Rồng Phượng)** và **Relay `P3` (Đèn LED4)**.

---

## 6. CÁC TÍNH NĂNG ĐIỀU KHIỂN TỪ REMOTE RF433 (P15, P16, P17)
*Hoạt động song song liên tục trong mọi trạng thái của hệ thống:*
* 🛑 **Chân `P15` (Kênh R2 RF433):** Tắt toàn bộ rơ-le từ **`P4` đến `P14`** (OFF toàn bộ tải/đèn). Ba kênh loa **`P0, P1, P2`** luôn giữ BẬT 100%. Hủy hẹn giờ lệnh H nếu đang chạy.
* ⚡ **Chân `P16` (Kênh R3 RF433):** **Chớp tắt đồng thời Relay 13 (`P13` - Đèn Đỏ), Relay 7 (`P7` - Đèn Bức Tranh) và Relay 10 (`P10` - Đèn Rồng Phượng)** theo chu kỳ 150ms trong lúc có tín hiệu. Khi thả nút thì ngắt tắt an toàn.
* 🔘 **Chân `P17` (Kênh R4 RF433):**
  - **BẤM GIỮ:** **BẬT ĐÈN ĐỎ (Relay 13 / `P13`)**, đồng thời **TẮT SẠCH TẤT CẢ ĐÈN** (`P3, P4, P5, P7, P10, P12, PA7`), **LOA P0, P1 VẪN GIỮ NGUYÊN!**
  - **NHẢ NÚT:** TẮT ngay Đèn Đỏ `P13` (Momentary).
  - **SAU 5 GIÂY KHÔNG NHẤN NỮA:** **TOÀN BỘ ĐÈN SẼ TỰ ĐỘNG BẬT LẠI ĐÚNG VỊ TRÍ MÀ LÚC ĐÓ ĐÃ BẤM!**

---

## 7. CẢM BIẾN NPN 5V CHÂN PA5 (A5) & KỊCH BẢN BỘ GÕ P14 (30S) -> SAU ĐÓ 10S TẮT TẤT CẢ ĐÈN & GỬI LỆNH 'J'

|      Chân STM32      |      Đầu Nối Ngoại Vi      |         Kiểu Chân (I/O)         | Chức Năng                                               |
| :--------------------: | :------------------------: | :--------------------------------: | :-------------------------------------------------------- |
| **`PA5` (A5)** | Dây tín hiệu Cảm biến NPN 5V | Input Pull-Up (kéo lên nội bộ) | Ban đầu mức CAO. Đặt vật: mức THẤP.<br>Khi MẤT TÍN HIỆU (nhấc vật -> mức CAO) ➔ Kích hoạt |
|   **`GND`**   | Dây âm (0V) Cảm biến NPN |                Mass                | Nối chung mass GND toàn hệ thống                       |

* **Kịch bản hoạt động:**
  - 🛡️ **CƠ CHẾ CHỐNG NHIỄU & CHỐNG SẬP HỆ THỐNG CAO CẤP (ANTI-CRASH ONE-SHOT LOCK):**
    - Sử dụng bộ lọc thời gian thực non-blocking (dựa trên `millis()`, hoàn toàn không dùng hàm `delay()` gây đơ vi điều khiển).
    - **Giai đoạn 1 (Đặt vật):** Chân `PA5` phải giữ mức THẤP (LOW) liên tục và ổn định $\ge 150\text{ms}$ mới xác nhận là đã đặt vật thành công (loại trừ các xung nhiễu chập chờn ban đầu).
    - **Giai đoạn 2 (Nhấc vật ra / Mất tín hiệu):** Chân `PA5` phải chuyển sang mức CAO (HIGH) liên tục và ổn định $\ge 150\text{ms}$ mới xác nhận là thực sự nhấc vật ra.
    - ⚡ **CHỐT KHÓA CỨNG VĨNH VIỄN (HARD LOCK):** Ngay khi nhấc vật thành công, bộ lọc chốt khóa vĩnh viễn (`A5_DA_KHOA_VINH_VIEN`). Dù người chơi có nhấc ra đặt vào bao nhiêu lần đi nữa, hệ thống cũng **KHÔNG BAO GIỜ nhận lại hay gửi lệnh lặp lại**, bảo vệ hệ thống tuyệt đối không bao giờ bị tràn bộ nhớ hay sập!
  - **Trình tự sau khi kích hoạt nhấc vật:**
    1. Kích hoạt chân **`P14` (Relay 14: Bộ gõ)** nhấp nháy ngẫu nhiên theo các chu kỳ:
       - **1s BẬT, 1.5s TẮT**
       - **2s BẬT, 2s TẮT**
       - **3s BẬT, 3s TẮT**
       - Tiến trình gõ ngẫu nhiên này duy trì liên tục trong vòng **30 giây**.
    2. **Sau 30 giây:**
       - Bộ gõ `P14` dừng hẳn (TẮT hoàn toàn).
       - Bắt đầu đếm thời gian chờ **10 giây**.
    3. **Sau đó 10 giây (Tổng 40 giây kể từ khi nhấc vật):**
       - **TẮT TẤT CẢ RELAY** (`P0..P14`), **CHỈ DUY NHẤT CHÂN P2 BẬT** (Loa hiệu ứng P2 phát bài "Đừng bỏ em 1 mình").
       - **GỬI LỆNH `'J'` DUY NHẤT 1 LẦN** qua UART sang bo **STM32_SOUND**.
       - Chờ bo Sound phát bài "Đừng bỏ em 1 mình".
    4. **Vừa hết bài "Đừng bỏ em 1 mình" (Nhận lệnh `'U'` từ STM32_SOUND hoặc timer 60s):**
       - **TẮT HẲN chân `P2`**.
       - **BẬT LẠI THÊM chân `P0` và `P1`** (Loa tổng P0 & Loa trên gác P1) ngay lập tức!
       - Chuyển pha nguyên khối (Atomic) qua I2C trong đúng 1 lệnh ghi duy nhất kèm cửa sổ khóa chống xung cuộn dây Relay (EMI Blanking 300ms), triệt tiêu hoàn toàn hiện tượng relay giật / nhảy loạn xạ / kích chéo các rơ-le khác.
       - Chờ nhận lệnh `'K'` từ STM32_SOUND.

---

## 8. KỊCH BẢN KHI NHẬN ĐƯỢC LỆNH 'K' TỪ STM32_SOUND

* **Khi nhận được lệnh `'K'` (hoặc `'k'`) từ cổng UART:**
  1. **Chuyển pha nguyên khối (Atomic Transition qua hàm `chuyenPha_NhanLenhK_BatP11_P13_P0_P1()`):**
     - **BẬT Relay ở chân `P11` ON** (`RELAY_BUP_BE_ROI_P11`: Rơi búp bê).
     - **BẬT chân `P13` ON** (Đèn đỏ).
     - **DUY TRÌ BẬT chân `P0` và `P1`** (Loa tổng & Loa trên gác).
     - Duy trì chân `P6` nếu đã mở chốt từ trước.
     - Toàn bộ được đóng gói và ghi đồng thời trong **đúng 1 lệnh I2C duy nhất** kèm **Ghi kép (Double-Write)** sau 5ms, triệt tiêu 100% hiện tượng xung cuộn dây gây kích chéo / giật nhảy các rơ-le khác.
  2. **Cửa sổ khóa chống nhiễu cuộn dây Relay & Tải cảm ứng (EMI Blanking Window 2000ms):**
     - Tạm ngưng toàn bộ việc đọc RF433 và lệnh refresh I2C trong 2 giây đầu để điện áp và tiếp điểm cơ khí ổn định hoàn toàn.
     - Bộ đệm UART được xóa sạch, chốt cờ `daNhanLenhK` chống kích đúp.
  3. Bắt đầu đếm thời gian **10 giây BẬT đèn P13** (`THOI_GIAN_K_BAT_P13_MS = 10000UL`).

* **Sau 10 giây (Kết thúc thời gian bật đèn P13, bắt đầu 20s tắt đèn):**
  1. **Chuyển pha nguyên khối (Atomic Transition qua hàm `chuyenPha_Sau10sK_TatP13()`):**
     - **TẮT chân `P13`** (Đèn đỏ tắt trong vòng 20 giây).
     - **DUY TRÌ BẬT** `P11` (Búp bê rơi), `P0` và `P1` (Loa), và `P6` (Cửa từ 1 nếu đã mở).
  2. Bắt đầu đếm thời gian **20 giây TẮT đèn P13** (`THOI_GIAN_K_TAT_P13_MS = 20000UL`).

* **Sau 20 giây tắt (Tổng 30 giây kể từ khi nhận lệnh K: `THOI_GIAN_K_TONG_MS = 30000UL`):**
  1. **Chuyển pha nguyên khối (Atomic Transition qua hàm `chuyenPha_Sau30sK_TatP13_BatP3()`):**
     - **TẮT chân `P13`** (Đèn đỏ).
     - **BẬT chân `P3` TRỞ LẠI** (Đèn quan sát).
     - Duy trì `P11`, `P0`, `P1` và `P6` (nếu đã mở).
  2. Khóa chống nhiễu cuộn dây 1000ms bảo vệ tiếp điểm rơ-le.

---

## 9. KỊCH BẢN KHI NHẬN ĐƯỢC LỆNH 'L' TỪ STM32_SOUND (BẤT KỲ THỜI ĐIỂM NÀO)

* **Khi nhận được lệnh `'L'` (hoặc `'l'`) từ cổng UART (PA3 RX):**
  1. **Chân `PA7` (A7) của STM32 xuất tín hiệu ON** (mức HIGH 3.3V).
  2. **Kích chân THÊM `P3` SÁNG** (`batChanPCF(3)` - giữ nguyên các relay khác đang chạy).
  3. Đèn LED PC13 sáng 3 giây phản hồi trạng thái nhận lệnh.

---

## 10. KỊCH BẢN KHI NHẬN ĐƯỢC LỆNH 'V' (CHIẾN THẮNG / WIN) TỪ STM32_SOUND (CHẾ ĐỘ TOGGLE)

* **LẦN NHẬN 1 (KÍCH HOẠT CHIẾN THẮNG):**
  1. **BẬT Relay `P12`, `P0`, `P1`**:
     - `P0`: Loa tổng.
     - `P1`: Loa trên gác.
     - `P12`: Đèn LED chỗ cuốn sổ (`RELAY_LED_CUON_SO_P12` / Bit 10).
  2. **GIỮ NGUYÊN RELAY BẮT BUỘC:** Chân **`P6` (Cửa từ 1)** nếu đã mở chốt trước đó thì **GIỮ NGUYÊN BẬT 100%** (theo nguyên tắc an toàn không bao giờ đóng chốt lại).
  3. **TẮT TOÀN BỘ CÁC RELAY CÒN LẠI:** `P2, P3, P4, P5, P7, P10, P11, P13, P14` và chân `A7` đều bị TẮT hoàn toàn.
  4. Chuyển trạng thái nguyên khối (Atomic) ra chip PCF8575 trong đúng 1 lệnh ghi I2C duy nhất triệt tiêu hoàn toàn xung nhiễu chập chờn.
  5. Đèn PC13 sáng liên tục 3 giây xác nhận.
* **LẦN NHẬN 2 (TẮT HỆ THỐNG VÀ ĐƯA VỀ BAN ĐẦU):**
  1. **TẮT TOÀN BỘ RELAY** (trừ `P6` giữ nguyên nếu đã mở).
  2. Tắt chân `A7`.
  3. **Đưa toàn bộ hệ thống về trạng thái ban đầu**: Chuyển máy trạng thái về `TRANG_THAI_CHO_LENH_S`, reset toàn bộ các cờ kịch bản (sẵn sàng đón chu kỳ mới khi nhấn `PB0` trên bo Sound).
  4. Đèn PC13 chuyển về nhấp nháy chậm (1000ms).

---

## 11. TÍNH NĂNG KHÔI PHỤC KỊCH BẢN KHI CÚP ĐIỆN (POWER-LOSS CHECKPOINT RECOVERY)

* **Nguyên lý hoạt động:**
  - Dữ liệu mốc kịch bản (Mốc 1..7) và trạng thái Cửa từ 1 (`P6`) được tự động lưu vào Page 63 bộ nhớ Flash nội của STM32F103 (tồn tại vĩnh viễn không mất khi cúp điện).
  - Khi cúp điện rồi có điện trở lại: Toàn bộ rơ-le giữ TẮT an toàn.
  - **Khôi phục hoạt động ngay khúc dở dang bằng 1 trong 2 cách:**
    1. **Cách 1 (Chuẩn nhất):** Nhấn **Nút 5 (`PB8`) trên bo `STM32_SOUND`** đúng 1 lần ➔ Bo Sound gửi lệnh `'R'` sang cổng UART PA3 (RX) của bo Relay.
    2. **Cách 2 (Dự phòng):** Nhấn nút bấm ở chân **`PA4` (A4)** trên chính bo `STM32_RELAY` (kéo lên `INPUT_PULLUP`, một đầu nối PA4, đầu còn lại nối mass `GND`).
  - **Kết quả khôi phục:**
    - Nếu Cửa từ 1 (`P6`) đã mở trước khi cúp điện ➔ **LẬP TỨC MỞ LẠI CHỐT CỬA TỪ `P6` NGAY** để đảm bảo người chơi trong phòng không bị kẹt cửa!
    - Khôi phục lại đúng các rơ-le tương ứng với mốc kịch bản trước lúc mất điện (đèn, loa, cơ cấu) và tiếp tục đếm thời gian chạy tiếp mà không cần chơi lại từ đầu phòng!





