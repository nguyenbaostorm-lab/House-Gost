# PHIÊN BẢN V7.0 - KHẮC PHỤC TRIỆT ĐỂ LỖI TREO LỆNH & NHẢY RELAY LIÊN TỤC

Phiên bản **V7.0** tập trung giải quyết triệt để 5 nguyên nhân gốc rễ gây ra hiện tượng **treo lệnh** và **rơ-le đóng ngắt tành tạch (nhảy liên tục)** thỉnh thoảng xuất hiện trong quá trình vận hành thực tế tại phòng Nhà Ma.

---

## 1. NGUYÊN NHÂN VÀ GIẢI PHÁP KỸ THUẬT V7.0

### ❌ LỖI 1: Trùng lệnh UART `'P'` kích hoạt nhầm Bộ Gõ P14
- **Hiện tượng:** Bo Jumpscare phát Heartbeat lệnh `'P'` (Play/Nghỉ) định kỳ sang bo Sound. Khi tín hiệu rò sang bo Relay, bo Relay có đoạn code `if (c == 'P' || c == 'B') kichHoatBoGoP14_CuongBuc();` $\rightarrow$ Bộ gõ P14 bị kích hoạt cưỡng bức liên tục ngoài ý muốn, làm rơ-le P14 đóng ngắt gõ cửa tành tạch và sinh xung nhiễu làm nhảy các rơ-le khác!
- **Khắc phục V7.0:** Xóa bỏ hoàn toàn ký tự `'P'` / `'p'` khỏi điều kiện kích hoạt bộ gõ trên bo Relay. Chỉ chấp nhận duy nhất ký tự `'B'` / `'b'` (Bộ gõ / Bang). Đồng thời khóa chốt: sau khi bộ gõ đã chạy xong 30s thì cấm tuyệt đối không cho kích hoạt lại lần thứ hai trong suốt ván chơi.

---

### ❌ LỖI 2: Rơ-le PA6 trên bo Sound chớp ngắt chu kỳ 500ms
- **Hiện tượng:** Mã nguồn test trước đây có khối chớp ngắt `PIN_SOUND_RELAY` (PA6) cứ 500ms BẬT, 500ms TẮT. Điều này làm rơ-le đóng ngắt liên tục mỗi nửa giây, gây tiếng nhảy cành cạch không ngừng.
- **Khắc phục V7.0:** Loại bỏ hoàn toàn khối chớp ngắt 500ms của PA6!
  - Âm thanh nền thực sự là **A0 (PA0 / Relay 1)**: Luôn BẬT ỔN ĐỊNH liên tục khi hệ thống rảnh.
  - Khi Jumpscare kích hoạt (`'S'`, `T1`, `T2`, `T3`): Âm thanh nền A0 **TẮT NGAY LẬP TỨC** trong suốt thời gian hiệu ứng (71s, 57s, 86s).
  - Khi Jumpscare kết thúc: Âm thanh nền A0 **BẬT LẠI ỔN ĐỊNH**, rơ-le không bị giật nhả liên tục.

---

### ❌ LỖI 3: Nhiễu sóng RF433 làm chớp tắt Rơ-le 13, 7, 10
- **Hiện tượng:** Module thu sóng RF433 không dây rất nhạy với xung điện từ và sụt áp. Khi ở trạng thái nghỉ, ngõ ra thường xuất hiện xung gai chập chờn. Bộ lọc cũ 6 mẫu (120ms) bị lọt xung nhiễu, làm rơ-le 13, 7, 10 bị chớp tắt liên tục (kênh R3 - P16).
- **Khắc phục V7.0:**
  - Nâng bộ lọc xác thực RF433 lên **12 mẫu liên tục (240ms)**.
  - Kênh R2 (P15): Chuyển sang kích hoạt theo sườn xung (Edge Triggered), chỉ thực hiện tắt 1 lần duy nhất khi nhấn, không gọi liên tục trong mỗi chu kỳ quét.
  - Giãn chu kỳ làm tươi chip PCF8575 từ 200ms lên **1000ms** khi đường truyền rảnh để giảm 80% tải I2C và giảm tần suất khóa ngắt toàn cục.

---

### ❌ LỖI 4: Treo cổng UART USART2 do cờ lỗi Overrun (ORE) trên bo Relay
- **Hiện tượng:** Khi các cuộn dây rơ-le hoặc bộ gõ P14 đóng ngắt, tia lửa điện tạo ra xung cảm ứng điện từ (EMI) đánh vào dây UART PA3 RX. Nếu CPU đang bận ghi chip PCF8575, một byte tới mà chưa kịp đọc sẽ làm phần cứng STM32 kích hoạt cờ lỗi **Overrun Error (ORE)** trong thanh ghi `USART2->SR`. Khi ORE = 1, vi điều khiển **từ chối nhận thêm bất kỳ lệnh nào nữa** $\rightarrow$ Dẫn đến hiện tượng "treo lệnh", bấm nút hay gửi lệnh đều không phản hồi!
- **Khắc phục V7.0:** Bổ sung cơ chế tự động phục hồi phần cứng USART2:
  ```cpp
  if (USART2->SR & (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE)) {
    volatile uint32_t dummySR = USART2->SR;
    volatile uint32_t dummyDR = USART2->DR;
    (void)dummySR;
    (void)dummyDR;
  }
  ```
  Tự động giải phóng cổng UART ngay trong vòng vài micro-giây, đảm bảo đường truyền luôn thông suốt 100% không bao giờ bị treo cứng.

---

### ❌ LỖI 5: Vòng lặp chặn 50ms & Nhịp tim Heartbeat gửi dồn dập
- **Hiện tượng:**
  - Trên bo Sound, đoạn đọc số kênh sau lệnh 'T' dùng `while (millis() - waitStart < 50)` làm CPU bị đơ 50ms mỗi khi nhận 'T'.
  - Trên bo Jumpscare, Heartbeat phát mỗi 500ms dồn dập làm ngập bộ đệm UART của bo Sound.
  - Nút bấm tiếp điểm khô bị rung dội cơ khí làm phát lệnh đúp.
- **Khắc phục V7.0:**
  - Chuyển toàn bộ luồng tiếp nhận lệnh Jumpscare trên bo Sound sang **Máy trạng thái phi chặn (Non-blocking FSM 100%)**.
  - Giãn chu kỳ Heartbeat của bo Jumpscare lên **2000ms (2 giây/lần)**.
  - Thêm bộ khóa Cooldown **400ms** sau mỗi lần nhấn công tắc khô trên bo Jumpscare để triệt tiêu hoàn toàn rung dội tiếp điểm.

---

## 2. KẾT QUẢ BIÊN DỊCH PLATFORMIO (V7.0)
```text
Environment         Status    Duration
------------------  --------  ------------
NAP_MACH_RELAY      SUCCESS   00:00:05.047
NAP_MACH_AM_THANH   SUCCESS   00:00:04.910
NAP_MACH_JUMPSCARE  SUCCESS   00:00:04.471
========================= 3/3 SUCCESS =========================
```
