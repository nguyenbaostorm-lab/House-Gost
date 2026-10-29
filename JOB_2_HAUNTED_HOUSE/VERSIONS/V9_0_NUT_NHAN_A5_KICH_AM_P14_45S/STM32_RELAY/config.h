#pragma once
#include <Arduino.h>

// ==============================================================================
// 1. CẤU HÌNH GIAO TIẾP I2C (OPEN-DRAIN) ĐIỀU KHIỂN MODULE PCF8575
// ==============================================================================
#define PIN_I2C_SCL PB12  // Chân xung clock I2C: PB12
#define PIN_I2C_SDA PB13  // Chân dữ liệu data I2C: PB13
#define PCF8575_ADDR 0x27 // Địa chỉ I2C chuẩn xác của chip PCF8575: 0x27

// ==============================================================================
// 2. SỐ LƯỢNG KÊNH ĐIỀU KHIỂN XUẤT TÍN HIỆU TRÊN PCF8575
// ==============================================================================
#define TOTAL_RELAYS 15 // Sử dụng 15 kênh rơ-le: P0 -> P14

// ==============================================================================
// 3. CỔNG GIAO TIẾP UART (USART2 PHẦN CỨNG) VỚI MẠCH ÂM THANH
// ==============================================================================
#define PIN_UART_TX PA2    // Chân TX gửi lệnh sang STM32 Sound
#define PIN_UART_RX PA3    // Chân RX nhận dữ liệu từ STM32 Sound
#define UART_BAUDRATE 9600 // Tốc độ truyền thông nối tiếp 9600 baud

// ==============================================================================
// 4. ĐÈN BÁO TRẠNG THÁI (HEARTBEAT) TRÊN BO STM32 BLUE PILL (PC13)
// ==============================================================================
#define PIN_LED_STATUS PC13      // Đèn LED xanh tích hợp trên bo PC13
#define TOC_DO_NHAY_NHANH_MS 80  // Vừa bắt đầu: nhấp nháy nhanh (80ms)
#define TOC_DO_NHAY_CHAM_MS 1000 // Khi ổn định: nhấp nháy chậm (1000ms)

// ==============================================================================
// 5. CẤU HÌNH THỜI GIAN & KÊNH RELAY KỊCH BẢN (CHỈ CẦN SỬA TẠI ĐÂY!)
// ==============================================================================
#define THOI_GIAN_KHOI_DONG_ON_DINH_MS                                         \
  3000 // Ổn định toàn bộ trong vòng 3s ban đầu
#define THOI_GIAN_LED_SANG_3S_MS                                               \
  3000 // Khi có lệnh S: đèn PC13 sáng lâu trong 3s

//  ⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐
//  🔥 [CÀI ĐẶT CHUYỂN SANG RELAY P0 VÀ P1 - TẤT CẢ SỬA TẠI ĐÂY] 🔥
//  -----------------------------------------------------------------------------
//  👉 1. THỜI GIAN CHỜ CHUYỂN PHA (Hỗ trợ cả PHÚT và GIÂY):
//        - Muốn 10 giây:  PHÚT = 0, GIÂY = 10 (Hiện tại đang chạy)
//        - Muốn 1 phút 30s: PHÚT = 1, GIÂY = 30
//        - Muốn 2 phút:   PHÚT = 2, GIÂY = 0
//  👉 2. CÁC KÊNH RELAY ĐƯỢC CHUYỂN SAU THỜI GIAN NÀY:
//        - Giữ bật P0, Bật thêm P1, Tắt P3
//  ⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐

#ifndef CHE_DO_THOI_GIAN
#define CHE_DO_THOI_GIAN 1 // 1: Giờ mới (Đúng giờ: 3p0s) | 2: Giờ cũ (Sai giờ: 10s)
#endif

#if (CHE_DO_THOI_GIAN == 2)
// --- [BẢN GIỜ CŨ / SAI GIỜ] - DÙNG TEST NHANH (10 GIÂY) ---
#define THOI_GIAN_CHUYEN_PHA_PHUT 0 // ⏳ ĐIỀN SỐ PHÚT (0 phút)
#define THOI_GIAN_CHUYEN_PHA_GIAY 10 // ⏳ ĐIỀN SỐ GIÂY (10 giây)
#else
// --- [BẢN GIỜ MỚI / ĐÚNG GIỜ] - CHẠY THỰC TẾ (3 PHÚT 0 GIÂY = 180s) ---
#define THOI_GIAN_CHUYEN_PHA_PHUT 3 // ⏳ ĐIỀN SỐ PHÚT (3 phút)
#define THOI_GIAN_CHUYEN_PHA_GIAY 0 // ⏳ ĐIỀN SỐ GIÂY (0 giây)
#endif

// Tự động tính ra mili-giây (Không cần sửa dòng này):
#define THOI_GIAN_CHUYEN_PHA_10S_MS                                            \
  (((uint32_t)(THOI_GIAN_CHUYEN_PHA_PHUT) * 60UL +                             \
    (uint32_t)(THOI_GIAN_CHUYEN_PHA_GIAY)) *                                   \
   1000UL)

// --- CÀI ĐẶT CHÂN RELAY SẼ BẬT / TẮT KHI CHUYỂN PHA D (SAU 10S TỪ LỆNH S) ---
#define RELAY_CHUYEN_PHA_BAT_P1 1 // 🎯 Chân Relay BẬT (Chân P1: LOA TRÊN GÁC)
#define RELAY_CHUYEN_PHA_TAT_P0 0 // 🎯 Chân Relay TẮT đi (Chân P0)
#define RELAY_CHUYEN_PHA_TAT_P3 3 // 🎯 Chân Relay TẮT đi (Chân P3: RELAY_LED4)

// ==============================================================================
// 6. RELAY P5 (LED ĐỒ CHƠI) & TÍN HIỆU CHÂN PA6 (A6 NỐI ÂM GND)
// ==============================================================================
#define RELAY_LED_DO_CHOI_P5                                                   \
  5 // 🧸 Chân P5: LED ĐỒ CHƠI (Bật khi nhận 'F', tắt khi chân A6 nhận tín hiệu)
#define PIN_BUTTON_OFF_R5                                                      \
  PA6 // Chân nhận tín hiệu nối âm: PA6 (A6), kéo lên PULLUP, kích mức LOW (GND)

// ==============================================================================
// 7. CẤU HÌNH TÍN HIỆU LOA (P0, P1, P2) ĐIỀU KHIỂN THEO KỊCH BẢN
// ==============================================================================
// - Lệnh S: Bật P0 (kèm P3) trong 10s
// - Sau 10s gửi D: Bật P1 (loa trên gác), tắt P0 & P3
// - Nhận F: Bật P5 (đèn đồ chơi), tắt đèn P3, và bật cả P0 và P1
#define RELAY_LOA_P0 0 // 🔊 P0: Loa tổng (Pha S)
#define RELAY_LOA_P1 1 // 🗣️ P1: Loa trên gác (Pha D & Pha F)
#define RELAY_LOA_P2 2 // 🎶 P2: Loa hiệu ứng

// ==============================================================================
// 8. CÁC KÊNH RELAY CƠ CẤU TẢI / NGOẠI VI KHÁC
// ==============================================================================
#define RELAY_DEN_TV_P4 4        // 📺 Chân P4 (Relay 4): ĐÈN TV
#define RELAY_CUA_TU_1_P6 6      // 🚪 Chân P6 (Relay 6): CỬA TỪ 1
#define RELAY_DEN_BUC_TRANH_P7 7 // 🖼️ Chân P7 (Relay 7): ĐÈN CHIẾU BỨC TRANH
#define RELAY_DEN_RONG_PHUONG_P10                                              \
  8 // 🐉 Chân P10 (Bit 8): ĐÈN RỒNG PHƯỢNG (Bật sau 45s kể từ khi bật Relay 7)
#define RELAY_DEN_P10 RELAY_DEN_RONG_PHUONG_P10 // Alias tương thích cũ
#define RELAY_BUP_BE_ROI_P11                                                   \
  9 // 🪆 Chân P11 (Bit 9 / Relay 11): BÚP BÊ RƠI XUỐNG
#define RELAY_LED_CUON_SO_P12                                                  \
  10 // 💡 Chân P12 (Bit 10 / Relay 12): ĐÈN LED CHỖ CUỐN SỔ
#define RELAY_DEN_DO_P13                                                       \
  13 // 🔴 Chân P13 (Relay 13): ĐÈN ĐỎ (Điều khiển bởi remote RF R4, kịch bản
     // lệnh K)
#define RELAY_RF_P13 RELAY_DEN_DO_P13 // Alias tương thích cũ
#define RELAY_BO_GO_P14 14            // 🔨 Chân P14 (Relay 14): BỘ GÕ

// ==============================================================================
// 9. CẤU HÌNH THỜI GIAN KỊCH BẢN KHI NHẬN LỆNH 'H' TỪ STM32_SOUND
// ==============================================================================
// Khi nhận 'H': BẬT ngay Relay 6 (P6: Cửa từ 1)
#define THOI_GIAN_H_BAT_R7_MS                                                  \
  5000UL // Sau 5s: Bật Relay 7 (P7: Đèn chiếu bức tranh)
#define THOI_GIAN_SAU_R7_BAT_P10_P3_MS                                         \
  45000UL // Sau đó 45s: Bật Đèn Relay P10 và Relay P3
#define THOI_GIAN_SAU_R7_BAT_P13_P3_MS                                         \
  THOI_GIAN_SAU_R7_BAT_P10_P3_MS // Alias tương thích cũ

// ==============================================================================
// 10. CẤU HÌNH CÁC KÊNH TÍN HIỆU ĐẦU VÀO RF433 (P15, P16, P17)
// ==============================================================================
// - P15 (Kênh R2): Tắt toàn bộ relay từ P4 đến P14
// - P16 (Kênh R3): Chớp tắt Relay 13, Relay 7 và Relay 10
// - P17 (Kênh R4): Bật Relay 10 khi nhấn, nhả ra thì tắt
#define MASK_RF_INPUT_PINS                                                     \
  ((1 << 13) | (1 << 14) | (1 << 15)) // 0xE000: P15, P16, P17
#define RF_ACTIVE_LEVEL                                                        \
  0 // 0 = Active LOW (0V khi nhấn remote), 1 = Active HIGH
#define TOC_DO_CHOP_TAT_RF_MS 150 // Tốc độ nhấp nháy chớp tắt (150ms)

// ==============================================================================
// 11. CẤU HÌNH NÚT NHẤN CHÂN PA5 (A5) KÍCH ÂM & KỊCH BẢN BỘ GÕ P14 (V9.0: KÍCH ON 45S)
// ==============================================================================
// - Chân PA5 (A5), kéo lên INPUT_PULLUP nội bộ
// - Ban đầu là MỨC CAO (HIGH)
// - Khi nhấn nút kích âm: Nối mass GND (mức THẤP / LOW)
// - Bộ lọc chống dội 50ms xác nhận nút nhấn hợp lệ -> KÍCH ON LIÊN TỤC P14 trong 45s -> sau đó tắt đèn -> gửi J
#define PIN_BUTTON_BO_GO_A5 PA5 // Chân nút nhấn kích âm GND: PA5 (A5)
#define THOI_GIAN_BO_GO_MS                                                     \
  45000UL // ⚡ V9.0: Thời gian bộ gõ P14 KÍCH ON LIÊN TỤC: 45 giây (45000ms)
#define THOI_GIAN_SAU_BO_GO_TAT_DEN_MS                                         \
  10000UL // Sau 45s gõ ON liên tục, chờ tiếp 10s rồi tắt tất cả đèn, bật P2 & gửi 'J'
#define THOI_GIAN_SAU_J_TAT_P2_BAT_P0_P1_MS                                    \
  50000UL // ⏳ Sau khi gửi J và bật P2: chờ 50 giây rồi tắt P2, bật lại P0 và
          // P1

// ==============================================================================
// 12. CẤU HÌNH THỜI GIAN KỊCH BẢN KHI NHẬN LỆNH 'K' TỪ STM32_SOUND
// ==============================================================================
// Khi nhận 'K': BẬT Relay ở chân P11 ON (Búp bê rơi), BẬT P0 và P1
// - BẬT đèn P13 ON (Đèn đỏ) trong vòng 10 giây (10s)
// - Sau đó TẮT đèn P13 trong vòng 20 giây (20s)
// - Hết 20s tắt (tổng 10s + 20s = 30s): BẬT chân P3 trở lại
#define THOI_GIAN_K_BAT_P13_MS                                                 \
  10000UL // ⏳ Thời gian BẬT đèn P13: 10 giây (10000ms)
#define THOI_GIAN_K_TAT_P13_MS                                                 \
  20000UL // ⏳ Thời gian TẮT đèn P13: 20 giây (20000ms)
#define THOI_GIAN_K_TONG_MS                                                    \
  (THOI_GIAN_K_BAT_P13_MS +                                                    \
   THOI_GIAN_K_TAT_P13_MS) // ⏳ Tổng thời gian: 30s (30000ms)
#define THOI_GIAN_K_TAT_P13_BAT_P3_MS                                          \
  THOI_GIAN_K_TONG_MS // Alias tương thích cũ

// ==============================================================================
// 13. CẤU HÌNH XUẤT TÍN HIỆU CHÂN A7 (PA7) KHI NHẬN LỆNH 'L'
// ==============================================================================
// Khi nhận lệnh 'L': chân A7 của STM32 xuất tín hiệu ON và kích chân thêm P3
// sáng
#define PIN_OUTPUT_A7 PA7 // Chân A7 (PA7) trên STM32 Blue Pill
#define MUC_A7_ON HIGH    // Mức ON: 3.3V (HIGH)
#define MUC_A7_OFF LOW    // Mức OFF: 0V (LOW)

// ==============================================================================
// 14. CẤU HÌNH KỊCH BẢN CHIẾN THẮNG KHI NHẬN LỆNH 'V' TỪ STM32_SOUND
// ==============================================================================
// - Lần nhận 1: BẬT Relay P12, P0, P1; giữ nguyên P6 (Cửa từ 1) nếu đã bật; TẮT
// các relay khác & A7.
// - Lần nhận 2: TẮT toàn bộ hệ thống (trừ P6 giữ nguyên nếu đã mở) và đưa về
// trạng thái ban đầu chờ lệnh S.
#define RELAY_CHIEN_THANG_P0 RELAY_LOA_P0 // Relay P0 (Bit 0): Loa tổng
#define RELAY_CHIEN_THANG_P1 RELAY_LOA_P1 // Relay P1 (Bit 1): Loa trên gác
#define RELAY_CHIEN_THANG_P12                                                  \
  RELAY_LED_CUON_SO_P12 // Relay P12 (Bit 10): Đèn LED cuốn sổ

// ==============================================================================
// 15. NÚT NHẤN KHÔI PHỤC KHI CÚP ĐIỆN DỰ PHÒNG TRÊN BO RELAY (CHÂN PA4)
// ==============================================================================
// - Chân PA4: Cấu hình INPUT_PULLUP, nối nút thường mở về mass GND
// - Khi nhấn: Khôi phục toàn bộ rơ-le theo checkpoint đã lưu và gửi 'R' sang bo
// Sound
#define PIN_BTN_KHOI_PHUC PA4 // Chân PA4 (A4) trên STM32 Blue Pill
