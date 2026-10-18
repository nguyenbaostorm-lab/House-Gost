# THÔNG TIN PHIÊN BẢN V6.4 (HAUNTED HOUSE CONTROLLER)

## 🌟 TỔNG HỢP CÁC NÂNG CẤP & SỬA LỖI MỚI NHẤT TRÊN BẢN V6.4

### 1. 🔨 Thuật toán CƯỠNG BỨC KÍCH HOẠT (Force Override) cho Nút bấm thay thế Cảm biến tiệm cận:
- **Vấn đề trước đây:**
  - Khi cảm biến tiệm cận NPN (`PB0`) bị lỗi (chập chờn, nhiễu xung, kẹt tín hiệu LOW, hoặc kích ảo sớm làm khóa cờ hệ thống `daXuLyTinHieuA5 = true` hoặc `daKetThuc30sBoGo = true`).
  - Lúc này, kỹ thuật viên hoặc người chơi nhấn **nút bấm thay thế/cứu nguy** (ở chân `PA4`, `PB1` hoặc nút bấm cắm thay vào `PB0`) thì toàn bộ lệnh bị **treo cứng và không hoạt động được**, do các hàm kiểm tra nút bấm bị chặn đứng phía sau điều kiện của cảm biến tiệm cận!
- **Thuật toán mới trong V6.4:**
  - **Tách riêng luồng xử lý Nút bấm cứu nguy (`PA4` & `PB1`) lên mức ưu tiên cao nhất (Cưỡng bức - Force Override):**
    + Khi nút cứu nguy `PA4` hoặc `PB1` được bấm (nối mass GND $\ge$ 30ms): Hệ thống lập tức gọi `kichHoatBoGoP14_CuongBuc()`.
    + Hàm này tự động **xóa sạch mọi cờ lỗi, gạt bỏ trạng thái treo/kẹt của cảm biến cũ**, cưỡng bức khởi động chu trình bộ gõ P14 chạy ngay 100%!
  - **Hỗ trợ cắm Nút bấm cơ trực tiếp vào chân `PB0` thay cho Cảm biến:**
    + Cảm biến tiệm cận NPN: Đặt vật LOW $\rightarrow$ Nhấc vật lên HIGH $\ge$ 50ms $\rightarrow$ Kích hoạt.
    + Nút bấm cơ cắm vào PB0: Nhấn (LOW) rồi thả ra (HIGH) $\ge$ 50ms $\rightarrow$ Kích hoạt chuẩn xác.

---

### 2. ⏳ Fix dứt điểm lỗi mất thời gian bộ gõ P14 ("mấy giây các kiểu"):
- **Nguyên nhân gốc:** 
  - Trước đây khi hàm kích hoạt bộ gõ chạy, biến `thoiGianBatDauBoGo` lấy bằng `millis()`. Nhưng ở vòng lặp `loop()`, biến thời gian `now` được gán ở phía trên từ trước đó.
  - Do `now < thoiGianBatDauBoGo` $\rightarrow$ phép trừ `now - thoiGianBatDauBoGo` bị **tràn số nguyên không dấu (underflow) thành ~4.29 tỷ mili-giây**!
  - Hệ thống hiểu nhầm là đã vượt quá 40 giây $\rightarrow$ **Lập tức dập tắt bộ gõ P14 ngay tức khắc, tắt hết đèn và gửi lệnh 'J'**, khiến bộ gõ không kịp đập nhấp nháy theo các mốc giây đã định!
- **Khắc phục trong V6.4:**
  - Bộ tính thời gian được đo bằng `millis()` tươi mới thời gian thực (`nowBoGo = millis()`) kèm cơ chế khóa chống tràn số underflow tuyệt đối.
  - Đảm bảo Bộ gõ P14 chạy trọn vẹn và chuẩn xác 100% thời gian đã cài đặt.

---

### 3. ⏱️ Khung cài đặt thời gian Bộ gõ P14 trực quan trong `config.h`:
Người dùng có thể dễ dàng điều chỉnh số giây hoạt động của bộ gõ ngay trong file `config.h`:

```cpp
// ⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐
// 🔨 [CÀI ĐẶT THỜI GIAN BỘ GÕ P14 (MẤY GIÂY CÁC KIỂU) - SỬA TẠI ĐÂY] 🔥
// -----------------------------------------------------------------------------
// 👉 1. THỜI GIAN CHẠY BỘ GÕ P14 (Tính bằng GIÂY):
//       - Mặc định kịch bản chuẩn: 30 GIÂY
//       - Muốn test nhanh: Có thể chỉnh 5 giây, 10 giây, 15 giây...
// 👉 2. THỜI GIAN SAU BỘ GÕ CHỜ RỒI TẮT HẾT ĐÈN & GỬI LỆNH 'J':
//       - Mặc định: 10 GIÂY
// 👉 3. CÁC MẪU GÕ NHẤP NHÁY NGẪU NHIÊN:
//       - Mẫu 1: 1s BẬT, 1.5s TẮT
//       - Mẫu 2: 2s BẬT, 2s TẮT
//       - Mẫu 3: 3s BẬT, 3s TẮT
// ⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐
#define THOI_GIAN_BO_GO_GIAY 30UL       // ⏳ Muốn test chỉnh số giây tại đây (VD: 5UL, 10UL, 30UL)
#define THOI_GIAN_BO_GO_MS (THOI_GIAN_BO_GO_GIAY * 1000UL)

#define THOI_GIAN_SAU_BO_GO_TAT_DEN_GIAY 10UL // ⏳ Sau khi gõ xong, chờ 10 giây rồi tắt hết đèn & gửi J
#define THOI_GIAN_SAU_BO_GO_TAT_DEN_MS (THOI_GIAN_SAU_BO_GO_TAT_DEN_GIAY * 1000UL)
```

---

### 4. 🎯 4 Phương thức kích hoạt Bộ gõ P14 song song:
1. **Nút nhấn cơ chân `PA4` (A4)**: Nhấn nối mass GND $\ge$ 30ms $\rightarrow$ Kích hoạt Cưỡng bức (Force Override) 100%, bỏ qua mọi lỗi của cảm biến tiệm cận.
2. **Nút nhấn cơ dự phòng chân `PB1` (B1)**: Nhấn nối mass GND $\ge$ 30ms $\rightarrow$ Kích hoạt Cưỡng bức (Force Override) 100%.
3. **Cảm biến NPN 5V hoặc nút cắm chân `PB0` (B0)**: Nhấc vật ra (hoặc nhấn-nhả nút) $\rightarrow$ Kích hoạt chuẩn xác.
4. **Lệnh UART Serial**: Gửi ký tự `'P'`, `'p'`, `'B'`, hoặc `'b'` để kích hoạt cưỡng bức từ xa.

---

### 5. 🛡️ Lá chắn bảo vệ UART chống xung nhiễu cuộn cảm:
- Ghim điện trở kéo lên nội bộ `pinMode(PA3, INPUT_PULLUP)` trên chân RX bo Relay.
- Trong thời gian 30s bộ gõ chạy, bỏ qua các ký tự rác `'0'`, `'S'`, `'T'`, `'K'`, `'V'` do tia lửa cuộn cảm P14 đánh vào đường truyền để triệt tiêu hoàn toàn lỗi Reset (Treo máy) và Nhảy lệnh.

---

### 6. 💡 Nút nhấn độc lập Bật/Tắt Đèn cuốn sổ P12 (PA5):
- Bấm 1 lần: **BẬT** sáng đèn P12.
- Bấm lần 2: **TẮT** đèn P12.
- Đọc đồng thời cả HAL GPIO lẫn Arduino chống trôi logic. Hoạt động độc lập 100% không ảnh hưởng kịch bản.

---
**Phiên bản:** V6.4 (Tháng 10/2026).
