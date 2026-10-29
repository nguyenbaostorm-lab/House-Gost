# PHIÊN BẢN V9.0 - NÚT NHẤN PA5 KÍCH ÂM & BỘ GÕ P14 KÍCH ON 45 GIÂY

Phiên bản **V9.0** được nâng cấp trực tiếp từ **V8.0** theo yêu cầu thay đổi cơ cấu kích hoạt ngoại vi: **Thay thế cảm biến NPN 5V nhấc vật bằng Nút nhấn cơ kích âm GND**.

---

## 1. NGUYÊN TẮC THIẾT KẾ VÀ ĐIỂM THAY ĐỔI TRONG V9.0

* **Chân kích hoạt:** Vẫn là chân **`PA5` (A5)** trên bo STM32 Blue Pill và chân nối mass **`GND`**.
* **Cơ chế kích hoạt cũ (V8.0 / V1.0):** Cảm biến NPN 5V (đặt vật mức LOW $\ge 150\text{ms}$ -> nhấc vật mất tín hiệu chuyển sang mức HIGH $\ge 150\text{ms}$ mới kích hoạt).
* **Cơ chế kích hoạt mới (V9.0):** **NÚT NHẤN TIẾP ĐIỂM CƠ NỐI MASS GND (KÍCH ÂM ACTIVE-LOW)**:
  * Chân `PA5` được kéo lên `INPUT_PULLUP` nội bộ (bình thường mức HIGH 3.3V).
  * Khi bấm nút: Tiếp điểm đóng nối chân `PA5` xuống `GND` (mức LOW).
  * Bộ lọc chống dội (Debounce) quét liên tục 5 mẫu qua 50ms để lọc sạch hoàn toàn xung nhiễu tia lửa điện và rung dội tiếp điểm.
  * Sau khi bấm nút thành công: **Khóa chốt vĩnh viễn (One-shot Lock)**, không bao giờ nhận lại lần 2 trong suốt ván chơi.

* **Cơ chế Bộ gõ P14 (kế thừa từ V8.0):**
  * Kích hoạt tiếp điểm **Relay 14 (P14: Bộ gõ) BẬT ON LIÊN TỤC TRONG 45 GIÂY**.
  * Sau 45s: TẮT dứt điểm bộ gõ P14.
  * Sau đó 10s (tổng 55s): TẮT toàn bộ rơ-le, BẬT loa P2 và gửi lệnh UART `'J'` sang bo **STM32_SOUND**.

---

## 2. BẢNG ĐẤU NỐI DÂY NGOẠI VI CHO CHÂN PA5 (V9.0)

| Chân STM32 Blue Pill | Thiết Bị Ngoại Vi | Cách Đấu Dây | Trạng Thái Hoạt Động |
| :---: | :---: | :---: | :--- |
| **`PA5` (A5)** | Cọc 1 của Nút Nhấn Cơ | Nối trực tiếp vào 1 chân tiếp điểm nút nhấn | Bình thường mức CAO (3.3V do Pull-Up nội) |
| **`GND`** | Cọc 2 của Nút Nhấn Cơ | Nối trực tiếp vào chân tiếp điểm còn lại | Khi bấm nút: Chân PA5 được kéo xuống 0V (Kích âm) |

---

## 3. MÃ NGUỒN CỐT LÕI (RELAY)

```cpp
inline bool kiemTraNutNhanPA5() {
  if (g_trangThaiLocA5 == A5_DA_KHOA_VINH_VIEN) return false;

  bool pinMucThap = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET) || 
                    (digitalRead(PIN_BUTTON_BO_GO_A5) == LOW);

  if (pinMucThap) {
    for (uint8_t i = 0; i < 5; i++) {
      delay(10);
      bool vanMucThap = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET) || 
                        (digitalRead(PIN_BUTTON_BO_GO_A5) == LOW);
      if (!vanMucThap) return false;
    }
    g_trangThaiLocA5 = A5_DA_KHOA_VINH_VIEN; // Chốt khóa 1 lần duy nhất
    return true; // Kích hoạt bộ gõ ON 45s
  }
  return false;
}
```

---

## 4. DANH SÁCH FILE BIN ĐÃ BIÊN DỊCH (2 LOẠI THỜI GIAN)

Thư mục lưu trữ: `JOB_2_HAUNTED_HOUSE/VERSIONS/V9_0_NUT_NHAN_A5_KICH_AM_P14_45S/BIN/` và `BIN_CODE/V9_0_NUT_NHAN_A5_KICH_AM_P14_45S/`

### ⏰ Loại 1: BẢN GIỜ MỚI - ĐÚNG GIỜ (Chạy thực tế)
* **`STM32_RELAY_V9_0_GIO_MOI_3P.bin`** (và bản mặc định `STM32_RELAY_V9_0.bin`):
  * Chuyển pha: **3 phút 0 giây** (180s) sau lệnh 'S'.
  * Bộ gõ P14: Kích ON 45 giây.
* **`STM32_SOUND_V9_0_GIO_MOI_3P43S.bin`** (và bản mặc định `STM32_SOUND_V9_0.bin`):
  * Thoại A1: **3 phút 43 giây** (223s) sau lệnh 'D' rồi gửi 'F'.

### ⚡ Loại 2: BẢN GIỜ CŨ - SAI GIỜ (Test nhanh)
* **`STM32_RELAY_V9_0_GIO_CU_10S.bin`**:
  * Chuyển pha: **10 giây** sau lệnh 'S'.
  * Bộ gõ P14: Kích ON 45 giây.
* **`STM32_SOUND_V9_0_GIO_CU_15S.bin`**:
  * Thoại A1: **15 giây** sau lệnh 'D' rồi gửi 'F'.

