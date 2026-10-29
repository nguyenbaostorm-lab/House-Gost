# PHIÊN BẢN V10.0 - BỔ SUNG NÚT DỰ PHÒNG PA4 CHO BỘ GÕ P14 (KÍCH ON 45S)

Phiên bản **V10.0** nâng cấp trực tiếp từ **V9.0**, bổ sung thêm cơ chế **dự phòng cứu nguy phần cứng (Dual Hardware Trigger)** cho bộ gõ `P14`.

---

## 1. MỤC ĐÍCH & Ý NGHĨA KỸ THUẬT V10.0

* **Bài toán thực tế:** Trong trường hợp nút bấm chính ở chân **`PA5` (A5)** bị hỏng tiếp điểm, đứt dây nối dài hoặc kẹt cơ khí, người vận hành hoặc người chơi có thể bấm nút dự phòng ở chân **`PA4` (A4)** để kích hoạt tiến trình y hệt, không làm gián đoạn ván chơi nhà ma!
* **Nguyên lý hoạt động song song (OR Logic):**
  * Tín hiệu kích hoạt bộ gõ = `kiemTraNutNhanPA5()` || `kiemTraNutNhanPA4()`
  * Bấm nút **`PA5`** $\rightarrow$ Kích hoạt bộ gõ `P14` chạy ON 45 giây.
  * Hoặc bấm nút dự phòng **`PA4`** $\rightarrow$ Kích hoạt bộ gõ `P14` chạy ON 45 giây.
  * Cả 2 nút đều được chống rung dội tiếp điểm 50ms và dùng chung cờ khóa cứng một lần duy nhất (`A5_DA_KHOA_VINH_VIEN`). Khi 1 trong 2 nút đã bấm thành công, hệ thống khóa chốt an toàn và từ chối mọi lần bấm tiếp theo.

---

## 2. TRÌNH TỰ HOẠT ĐỘNG CHI TIẾT (MỐC BỘ GÕ P14)

1. **Kích hoạt:** Bấm nút **`PA5`** (hoặc nút cứu nguy **`PA4`**).
2. **Giai đoạn 1 (0 – 45 giây):**
   * Relay 14 (Chân `P14`: Bộ gõ) **KÍCH BẬT ON LIÊN TỤC**.
   * Tiếp điểm rơ-le đóng giữ ON liên tục trong **45 giây**.
3. **Giai đoạn 2 (Sau 45 giây):**
   * Bộ gõ `P14` TẮT dứt điểm hoàn toàn.
   * Bắt đầu đếm thời gian chờ **10 giây**.
4. **Giai đoạn 3 (Sau đó 10 giây - Tổng 55 giây kể từ khi bấm):**
   * **TẮT TẤT CẢ RELAY** từ `P0` đến `P14`.
   * **BẬT DUY NHẤT CHÂN `P2`** (Loa hiệu ứng P2).
   * **GỬI LỆNH `'J'` DUY NHẤT 1 LẦN** qua UART sang bo **STM32_SOUND**.
   * Bắt đầu đếm thời gian chờ **50 giây** để chuyển mốc tiếp theo.

---

## 3. SƠ ĐỒ ĐẤU NỐI DÂY CỤM NÚT BẤM (V10.0)

| Chân STM32 | Vai Trò | Đấu Nối Ngoại Vi |
| :---: | :---: | :--- |
| **`PA5` (A5)** | **Nút nhấn CHÍNH** | Nối vào 1 chân tiếp điểm nút bấm chính ngoài phòng |
| **`PA4` (A4)** | **Nút nhấn DỰ PHÒNG** | Nối vào 1 chân tiếp điểm nút bấm dự phòng tại tủ điện/cứu nguy |
| **`GND`** | **Mass chung** | Nối vào chân tiếp điểm còn lại của cả 2 nút nhấn PA5 và PA4 |
