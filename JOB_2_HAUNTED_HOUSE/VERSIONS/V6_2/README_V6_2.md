# THÔNG TIN PHIÊN BẢN V6.2 (HAUNTED HOUSE CONTROLLER)

## 🌟 TỔNG HỢP CÁC TÍNH NĂNG & THAY ĐỔI MỚI NHẤT

### 1. Trạng thái khởi động / Vừa bật hệ thống:
- **KHI BẬT HỆ THỐNG / CẤP NGUỒN:**
  - **CHỈ CÓ CHÂN P0 VÀ P1 BẬT (Loa tổng P0 và Loa trên gác P1)**.
  - **TẤT CẢ CÁC ĐÈN KHÁC KHÔNG BẬT (TẮT TUYỆT ĐỐI)**:
    - P3, P4, P5, P7, P10, P12, P13, PA7 đều TẮT.
    - P6 (Cửa từ 1), P11 (Búp bê), P14 (Bộ gõ) TẮT an toàn.
- **KHI NHẤN NÚT START (Chân PB0 trên bo Sound):**
  - Gửi lệnh `'S'` sang bo Relay $\rightarrow$ Hệ thống chuyển sang kịch bản hoạt động bình thường:
    - BẬT P0 và P3 trong 10s (bản test) hoặc 3 phút (bản chuẩn), P1 tắt.
    - Sau đó chuyển sang P1 (gửi D sang Sound, tắt P0 & P3).
    - Tiếp tục toàn bộ flowchart kịch bản tự động bình thường.

---

### 2. Các cách kích hoạt Bộ gõ P14 (Relay 14):
Hệ thống hỗ trợ song song 4 cách kích hoạt (nhận đúng 1 lần duy nhất trong suốt ván chơi):
1. **Chân `PA4` (A4 - NÚT BẤM CƠ KÍCH BỘ GÕ)**:
   - Cấu hình `INPUT_PULLUP` nội bộ (bình thường mức HIGH 3.3V).
   - Nối nút nhấn giữa **chân `PA4` và mass `GND`**.
   - **Bộ lọc phi chặn (Non-blocking)**: Nhấn mức LOW liên tục $\ge$ 30ms là kích hoạt ngay lập tức 100%! Hoạt động hoàn toàn độc lập, không bị ảnh hưởng hay phụ thuộc vào cảm biến PB0.
2. **Chân `PB1` (B1 - NÚT BẤM CƠ DỰ PHÒNG)**:
   - Kéo lên `INPUT_PULLUP` nội bộ, bấm nối mass GND $\ge$ 30ms $\rightarrow$ Kích hoạt bộ gõ ngay.
3. **Chân `PB0` (B0 - CẢM BIẾN NPN 5V NHẤC VẬT)**:
   - Đặt vật (kéo LOW $\ge$ 80ms) $\rightarrow$ ghi nhận đã có vật.
   - Nhấc vật ra (mất tín hiệu LOW $\rightarrow$ chuyển lên HIGH $\ge$ 80ms) $\rightarrow$ Kích hoạt bộ gõ.
   - Đã loại bỏ nhánh tự kích 350ms sai lầm để tránh tự kích và nghẽn hệ thống.
4. **Lệnh UART (`'P'` hoặc `'B'`)**:
   - Gửi ký tự `'P'`, `'p'`, `'B'`, hoặc `'b'` qua UART Serial $\rightarrow$ Kích hoạt ngay chu trình Bộ gõ P14.

---

### 3. Nút nhấn bật / tắt Đèn cuốn sổ P12 trên chân `PA5` (A5):
- **Chân `PA5`**: Kéo lên `INPUT_PULLUP` nội bộ. Nối nút nhấn giữa **`PA5` và mass `GND`**.
- **Chức năng:** **BẬT hoặc TẮT ĐÈN P12 (Relay 12 - Đèn cuốn sổ)**:
  - Bấm lần 1: **BẬT** sáng đèn P12.
  - Bấm lần 2: **TẮT** đèn P12.
  - Hoạt động độc lập 100%, có thể bấm bất kỳ lúc nào mà không can thiệp hay ảnh hưởng đến kịch bản chính.

---

### 4. Thuật toán tối ưu hóa triệt để: Chống Treo & Chống Nhảy lệnh khi P14 hoạt động:
- **🛡️ Lá chắn bảo vệ UART chống xung nhiễu cuộn cảm Solenoid P14**:
  - Khi cuộn solenoid/relay P14 đóng ngắt gõ cửa liên tục, xung cảm ứng điện từ (EMI) và hồ quang đánh vào dây UART PA3 RX thường sinh ra các ký tự rác (`'0'`, `'S'`, `'T'`, `'K'`, `'V'`).
  - Trong code cũ, ký tự rác `'0'` gây `NVIC_SystemReset()` reboot chip liên tục (**lỗi Treo máy**); ký tự rác `'S'`, `'T'`, `'K'`, `'V'` làm reset kịch bản hoặc nhảy cóc sang giai đoạn khác (**lỗi Nhảy lệnh**).
  - **Khắc phục dứt điểm:** Trong suốt 30s P14 đang chạy (`boGoDangChay == true`), hệ thống **cô lập và bỏ qua tuyệt đối các ký tự rác này**; đồng thời ghim chân `PA3 RX` lên `INPUT_PULLUP` 3.3V chống nhiễu bắt sóng.
- **🛡️ Phục hồi Bus I2C tự động (`i2c_bus_recover`)**:
  - Phát 9 nhịp clock SCL để giải phóng chân SDA nếu chip PCF8575 bị sụt áp kẹt bus.
- **🛡️ Tự động reset đầy đủ toàn bộ các cờ khi bắt đầu ván mới (`'S'`):**
  - Đã bổ sung reset `daKetThuc30sBoGo = false;`, `daTatDenSauBoGo = false;`, `daGuiLenhJ = false;` để các ván chơi sau bộ gõ luôn hoạt động trơn tru.

---

### 5. Nút bấm chiến thắng / Mở cửa từ 1 (PA6):
- Chân `PA6` nối GND kích hoạt mở chốt cửa từ P6 vĩnh viễn.

---
**Phiên bản:** V6.2 Tối ưu hóa (Tháng 10/2026).
