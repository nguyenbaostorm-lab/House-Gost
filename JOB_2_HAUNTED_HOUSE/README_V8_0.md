# PHIÊN BẢN V8.0 - KÍCH ON LIÊN TỤC BỘ GÕ P14 TRONG 45 GIÂY (CHUẨN TỪ V1_CHUAN)

Phiên bản **V8.0** được phát triển **chuẩn 100% từ nền tảng V1_CHUAN**, đáp ứng chính xác yêu cầu kỹ thuật đặc biệt cho cơ cấu bộ gõ thực tế tại phòng Nhà Ma.

---

## 1. NGUYÊN TẮC THIẾT KẾ & KHÁC BIỆT VỚI V1_CHUAN

* **Kế thừa 100% phần cứng chuẩn V1_CHUAN:**
  * Cảm biến NPN 5V: Chân **`PA5` (A5)** trên STM32 Blue Pill (không sử dụng chân phụ `PB0` của các bản thử nghiệm).
  * Bộ gõ: Relay 14 trên chip mở rộng **`PCF8575` (Chân `P14` / Bit 12)** (không chuyển sang chân direct `PA7`).
  * Tất cả các rơ-le, cảm biến A6, còi hú, đèn, loa và giao thức UART gửi nhận ('G', 'H', 'J', 'K', 'L', 'S', 'V') giữ nguyên vẹn 100% chuẩn gốc.

* **Điểm thay đổi duy nhất trong kịch bản:**
  * **Phiên bản cũ V1:** Sau khi nhấc vật khỏi cảm biến A5, chân `P14` nhấp nháy ngẫu nhiên theo 3 chu kỳ (1s BẬT, 1.5s TẮT / 2s BẬT, 2s TẮT / 3s BẬT, 3s TẮT) trong 30 giây.
  * **Phiên bản mới V8.0:** Bỏ hoàn toàn việc nhấp nháy ngẫu nhiên. Chân **`P14` (Relay 14: Bộ gõ) được KÍCH BẬT ON LIÊN TỤC TRONG VÒNG 45 GIÂY**.

---

## 2. TRÌNH TỰ HOẠT ĐỘNG CHI TIẾT TẠI MỐC BỘ GÕ (MỐC 5)

1. **Kích hoạt từ Cảm biến NPN PA5:**
   * Cảm biến đang giữ vật (mức THẤP $\ge 150\text{ms}$).
   * Người chơi nhấc vật ra (mức CAO $\ge 150\text{ms}$) $\rightarrow$ Hệ thống chốt khóa vĩnh viễn chống kích lặp lại.
2. **Giai đoạn 1 (0 – 45 giây):**
   * Chân **`P14` (Relay 14: Bộ gõ)** được kích **BẬT ON LIÊN TỤC**.
   * Tiếp điểm rơ-le 14 đóng liên tục trong suốt **45 giây (45000ms)**.
3. **Giai đoạn 2 (Đúng 45 giây):**
   * Hết 45s, bộ gõ `P14` được **TẮT HOÀN TOÀN**.
   * Bắt đầu đếm thời gian chờ **10 giây**.
4. **Giai đoạn 3 (Sau đó 10 giây - Tổng 55 giây kể từ khi nhấc vật):**
   * **TẮT TOÀN BỘ RƠ-LE** từ `P0` đến `P14`.
   * **BẬT DUY NHẤT CHÂN `P2`** (Loa hiệu ứng P2).
   * **GỬI LỆNH `'J'` DUY NHẤT 1 LẦN** qua UART USART2 sang bo **STM32_SOUND**.
   * Bắt đầu đếm thời gian chờ **50 giây** để chuyển sang mốc tiếp theo (nhận lệnh `'K'`).

---

## 3. THAY ĐỔI TRONG MÃ NGUỒN

### Trong [`config.h`](file:///d:/dowloands/House-Gost-main/House-Gost-main/JOB_2_HAUNTED_HOUSE/VERSIONS/V8_0_KICH_ON_BO_GO_P14_45S/STM32_RELAY/config.h):
```cpp
#define PIN_BUTTON_BO_GO_A5 PA5 // Chân cảm biến NPN 5V: PA5 (A5)
#define THOI_GIAN_BO_GO_MS \
  45000UL // ⚡ V8.0: Thời gian bộ gõ P14 KÍCH ON LIÊN TỤC: 45 giây (45000ms)
#define THOI_GIAN_SAU_BO_GO_TAT_DEN_MS \
  10000UL // Sau 45s gõ ON liên tục, chờ tiếp 10s rồi tắt tất cả đèn, bật P2 & gửi 'J'
```

### Trong [`main.cpp`](file:///d:/dowloands/House-Gost-main/House-Gost-main/JOB_2_HAUNTED_HOUSE/VERSIONS/V8_0_KICH_ON_BO_GO_P14_45S/STM32_RELAY/main.cpp):
```cpp
  // Kích hoạt khi mất tín hiệu A5:
  if (!daXuLyTinHieuA5 && !boGoDangChay && !daKetThucBoGo && !daTatDenSauBoGo && kiemTraNutNhanPA5()) {
    daXuLyTinHieuA5 = true;
    boGoDangChay = true;
    thoiGianBatDauBoGo = now;
    batChanPCF(RELAY_BO_GO_P14); // ⚡ V8.0: BẬT ON LIÊN TỤC P14
    daKetThucBoGo = false;
    daTatDenSauBoGo = false;
    daGuiLenhJ = false;
  }

  // Quản lý thời gian:
  if (boGoDangChay) {
    uint32_t thoiGianTroiQua = now - thoiGianBatDauBoGo;

    // Đủ 45s -> Tắt bộ gõ P14
    if (thoiGianTroiQua >= THOI_GIAN_BO_GO_MS && !daKetThucBoGo) {
      tatChanPCF(RELAY_BO_GO_P14);
      daKetThucBoGo = true;
    }

    // Sau đó 10s (tổng 55s) -> Tắt hết đèn, bật P2, gửi lệnh 'J'
    if (thoiGianTroiQua >= THOI_GIAN_BO_GO_MS + THOI_GIAN_SAU_BO_GO_TAT_DEN_MS) {
      if (!daTatDenSauBoGo) {
        tatTatCaRelay();
        batChanPCF(RELAY_LOA_P2);
        guiLenhSound('J');
        thoiGianGuiJ = now;
        ...
      }
    }
  }
```
