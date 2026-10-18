# THÔNG TIN PHIÊN BẢN: V6_5_FIX_THOI_GIAN_BO_GO_P14

## 🌟 TỔNG QUAN NÂNG CẤP & SỬA LỖI TRỌNG TÂM

### 1. 🔨 FIX TRIỆT ĐỂ LỖI MẤT THỜI GIAN BỘ GÕ P14 (V6.4 VẪN BỊ 0 GIÂY NHẢY QUA P2 VÀ J):
- **Phân tích nguyên nhân cốt lõi:**
  - Ở đầu hàm `loop()`, biến `now = millis()` được lấy mẫu sớm.
  - Khi lệnh kích hoạt bộ gõ chạy (`kichHoatBoGoP14()`), biến `thoiGianBatDauBoGo = millis()` được gán bằng thời gian tươi mới (sau `now` một vài micro-giây đến 1 mili-giây).
  - Kết quả: `now < thoiGianBatDauBoGo`!
  - Trong phép trừ số nguyên không dấu 32-bit (`uint32_t`):
    `now - thoiGianBatDauBoGo` bị **tràn số âm (integer underflow) thành 4,294,967,295 mili-giây**!
  - Con số 4.29 tỷ ms này ngay lập tức vượt qua cả 30s và 40s!
  - Hệ thống lập tức coi như đã kết thúc chu kỳ: **Tắt ngay bộ gõ P14 trong 0 giây, tắt sạch toàn bộ đèn, bật P2 và bắn lệnh 'J' sang bo Sound**! Bộ gõ hoàn toàn không có thời gian gõ nào cả!
- **Giải pháp tối ưu trong V6.5:**
  - Trong khối xử lý bộ gõ, lấy thời gian thực `nowBoGo = millis()`.
  - Thiết lập **chốt chặn toán học chống Underflow tuyệt đối**:
    ```cpp
    uint32_t thoiGianTroiQua = 0;
    if (nowBoGo >= thoiGianBatDauBoGo) {
      thoiGianTroiQua = nowBoGo - thoiGianBatDauBoGo;
    }
    ```
  - Nếu `nowBoGo < thoiGianBatDauBoGo`, `thoiGianTroiQua` được gán bằng `0` thay vì tràn thành 4.29 tỷ!
  - Đảm bảo Bộ gõ P14 chạy đủ 100% thời gian (mặc định 30 giây) theo các mẫu gõ nhấp nháy ngẫu nhiên.

---

### 2. ⚡ CƯỠNG BỨC KÍCH HOẠT (FORCE OVERRIDE) KHI CẢM BIẾN TIỆM CẬN PB0 BỊ LỖI:
- Kỹ thuật viên có thể bấm **Nút bấm cứu nguy ở chân PA4 hoặc PB1**:
  - Hệ thống tách riêng luồng nút bấm cứu nguy, đưa lên mức ưu tiên cao nhất.
  - Gọi hàm `kichHoatBoGoP14_CuongBuc()`: tự động xóa bỏ mọi cờ lỗi (`daKetThuc30sBoGo = false`, `daTatDenSauBoGo = false`), khóa chốt cảm biến PB0 chống xung nhiễu chèn vào, và kích hoạt ngay chu trình bộ gõ P14.
  - Bấm là ăn ngay 100%, không bị treo lệnh hay kẹt cờ của cảm biến tiệm cận.

---

### 3. ⏱️ DỄ DÀNG CÀI ĐẶT THỜI GIAN GÕ TRONG `config.h`:
Người dùng chỉ cần sửa số giây mong muốn tại `STM32_RELAY/config.h`:
```cpp
#define THOI_GIAN_BO_GO_GIAY 30UL       // ⏳ Số giây bộ gõ P14 đập nhấp nháy (Mặc định: 30s, test có thể sửa 5s, 10s...)
#define THOI_GIAN_SAU_BO_GO_TAT_DEN_GIAY 10UL // ⏳ Số giây sau khi gõ xong chờ rồi tắt hết đèn & gửi J (Mặc định: 10s)
```

---

### 4. 🗂️ ĐẶT TÊN PHIÊN BẢN KÈM TÊN LỖI ĐÃ FIX:
- Tên thư mục phiên bản: `V6_5_FIX_THOI_GIAN_BO_GO_P14`
- Tương thích 100% với file nạp `NAP_CODE.bat` (tự động nhận diện và liệt kê trong menu nạp code ST-Link).
