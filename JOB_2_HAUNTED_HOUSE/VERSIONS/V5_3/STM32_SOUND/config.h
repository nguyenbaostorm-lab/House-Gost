#pragma once
#include "stm32f1xx_hal.h"

// ==============================================================================
// 1. CẤU HÌNH 7 CHÂN RELAY TRÊN BO STM32_SOUND (PA0 ĐẾN PA6)
// ==============================================================================
#define TOTAL_RELAYS 7 // Tổng cộng 7 Relay sử dụng

#define PIN_RELAY_1_PORT GPIOA
#define PIN_RELAY_1_PIN GPIO_PIN_0 // Relay 1 -> Chân PA0 (A0): ÂM THANH 1 THEME

#define PIN_RELAY_2_PORT GPIOA
#define PIN_RELAY_2_PIN                                                        \
  GPIO_PIN_1 // Relay 2 -> Chân PA1 (A1): ÂM THANH 2 TRÒ CHUYỆN BẢO VẬT

#define PIN_RELAY_3_PORT GPIOA
#define PIN_RELAY_3_PIN                                                        \
  GPIO_PIN_2 // Relay 3 -> Chân PA2 (A2): ÂM THANH 3 TIẾNG KẼO KẸT

#define PIN_RELAY_4_PORT GPIOA
#define PIN_RELAY_4_PIN GPIO_PIN_3 // Relay 4 -> Chân PA3 (A3): ÂM THANH HÙ RÉ

#define PIN_RELAY_5_PORT GPIOA
#define PIN_RELAY_5_PIN GPIO_PIN_4 // Relay 5 -> Chân PA4

#define PIN_RELAY_6_PORT GPIOA
#define PIN_RELAY_6_PIN GPIO_PIN_5 // Relay 6 -> Chân PA5

#define PIN_RELAY_7_PORT GPIOA
#define PIN_RELAY_7_PIN GPIO_PIN_6 // Relay 7 -> Chân PA6

// Mức kích hoạt Relay (Chuẩn Active-HIGH theo thực tế phần cứng)
#define RELAY_ON GPIO_PIN_SET    // Mức HIGH (3.3V): Đóng rơ-le (Bật)
#define RELAY_OFF GPIO_PIN_RESET // Mức LOW (0V): Ngắt rơ-le (Tắt)

// ==============================================================================
// 2. CẤU HÌNH 6 CHÂN NÚT BẤM CƠ (NỐI MASS GND, KÉO LÊN NỘI BỘ PULL-UP)
// ==============================================================================
#define PIN_BTN_PORT GPIOB
#define PIN_BTN_1_PIN GPIO_PIN_0 // Nút nhấn 1: Chân PB0
#define PIN_BTN_2_PIN GPIO_PIN_5 // Nút nhấn 2: Chân PB5
#define PIN_BTN_3_PIN GPIO_PIN_6 // Nút nhấn 3: Chân PB6
#define PIN_BTN_4_PIN                                                          \
  GPIO_PIN_1 // Nút nhấn 4: Chân PB1 (Thay cho PB7, nằm ngay sát cạnh PB0)
#define PIN_BTN_5_PIN                                                          \
  GPIO_PIN_8 // Nút nhấn 5: Chân PB8 (NÚT KHÔI PHỤC KHI CÚP ĐIỆN)
#define PIN_BTN_6_PIN                                                          \
  GPIO_PIN_9 // Chân PB9: Nút nhấn 6 / Nhận tín hiệu kênh R1 mạch RF433 (Dừng
             // khẩn cấp)
#define PIN_RF_R1_PIN PIN_BTN_6_PIN // Kênh R1 của mạch RF433 nối vào PB9

// Tên gọi bí danh thân thiện
#define PIN_NUT_NHAN_1 PIN_BTN_1_PIN
#define PIN_NUT_NHAN_2 PIN_BTN_2_PIN
#define PIN_NUT_NHAN_3 PIN_BTN_3_PIN
#define PIN_NUT_NHAN_4 PIN_BTN_4_PIN
#define PIN_NUT_NHAN_5 PIN_BTN_5_PIN
#define PIN_NUT_NHAN_6 PIN_BTN_6_PIN
#define PIN_BTN_KHOI_PHUC_PIN                                                  \
  PIN_BTN_5_PIN // Chân PB8: Nút nhấn khôi phục kịch bản khi có điện lại
#define PIN_BTN_KHOI_PHUC PB8 // Chân PB8 chuẩn Arduino

// ==============================================================================
// 3. CỔNG GIAO TIẾP UART VỚI BO STM32 RELAY (PB12 TX & PB13 RX)
// ==============================================================================
#define PIN_UART_PORT GPIOB
#define PIN_UART_TX_PIN                                                        \
  GPIO_PIN_12 // Chân 12 (PB12 - TX): Truyền lệnh sang bo Relay
#define PIN_UART_RX_PIN                                                        \
  GPIO_PIN_13 // Chân 13 (PB13 - RX): Nhận dữ liệu từ bo Relay
#define UART_BAUDRATE                                                          \
  9600 // Tốc độ truyền thông nối tiếp 9600 baud (104us / bit)

// ==============================================================================
// 3B. CỔNG GIAO TIẾP UART VỚI BO STM32 JUMPSCARE (PB10 TX & PB11 RX)
// ==============================================================================
#define PIN_UART_JUMP_PORT GPIOB
#define PIN_UART_JUMP_TX_PIN                                                   \
  GPIO_PIN_10 // Chân B10 (TX bo Sound): Nối sang chân B6 bo Jumpscare
#define PIN_UART_JUMP_RX_PIN                                                   \
  GPIO_PIN_11 // Chân B11 (RX bo Sound): Nối từ chân B5 bo Jumpscare

// ==============================================================================
// 4. ĐÈN BÁO TRẠNG THÁI (HEARTBEAT) TRÊN BO STM32 BLUE PILL
// ==============================================================================
#define PIN_LED_PORT GPIOC
#define PIN_LED_PIN GPIO_PIN_13 // Đèn LED xanh tích hợp trên bo PC13
#define LED_ON GPIO_PIN_RESET
#define LED_OFF GPIO_PIN_SET

// ==============================================================================
// 5. CẤU HÌNH THỜI GIAN KHỞI ĐỘNG ỔN ĐỊNH
// ==============================================================================
#define THOI_GIAN_KHOI_DONG_ON_DINH_MS                                         \
  3000 // Khởi động ổn định nguồn trong vòng 3s

// ==============================================================================
// 6. CẤU HÌNH TỐC ĐỘ NHẤP NHÁY LED PC13
// ==============================================================================
#define TOC_DO_NHAY_SOUND_CHAM_MS 500 // Nháy chậm khi chờ (500ms)
#define TOC_DO_NHAY_SOUND_NHANH_MS 80 // Nháy nhanh sau khi gửi lệnh S (80ms)
#define THOI_GIAN_LED_NHAY_KHI_NHAN_D_MS                                       \
  3000 // Nháy trong vòng 3s khi nhận lệnh D

// ==============================================================================
// 7. CẤU HÌNH THỜI GIAN BẬT CHÂN A1 / RELAY 2 (TRÒ CHUYỆN TRÊN GÁC) & GỬI LỆNH
// 'F'
// ==============================================================================
// Khi nhận 'D': Bật A1 cuộc trò chuyện chạy trong vòng 3 phút 43 giây rồi TẮT
// HẲN, sau đó gửi lệnh 'F'
#ifndef CHE_DO_THOI_GIAN
#define CHE_DO_THOI_GIAN 1 // 1: Giờ mới (Đúng giờ: 3p43s) | 2: Giờ cũ (Sai giờ: 15s)
#endif

#if (CHE_DO_THOI_GIAN == 2)
// --- [BẢN GIỜ CŨ / SAI GIỜ] - DÙNG TEST NHANH (15 GIÂY) ---
#define THOI_GIAN_A1_SO_PHUT 0 // ⏳ Điền SỐ PHÚT (0 phút)
#define THOI_GIAN_A1_SO_GIAY 15 // ⏳ Điền SỐ GIÂY (15 giây)
#else
// --- [BẢN GIỜ MỚI / ĐÚNG GIỜ] - CHẠY THỰC TẾ (3 PHÚT 43 GIÂY) ---
#define THOI_GIAN_A1_SO_PHUT 3 // ⏳ Điền SỐ PHÚT (3 phút)
#define THOI_GIAN_A1_SO_GIAY 43 // ⏳ Điền SỐ GIÂY (43 giây)
#endif

#define THOI_GIAN_BAT_A1_MS                                                    \
  (((uint32_t)(THOI_GIAN_A1_SO_PHUT) * 60UL +                                  \
    (uint32_t)(THOI_GIAN_A1_SO_GIAY)) *                                        \
   1000UL)

#define THOI_GIAN_GUI_LENH_F_SAU_D_MS THOI_GIAN_BAT_A1_MS // Alias tương thích

// ==============================================================================
// 8. CẤU HÌNH THỜI GIAN ÂM THANH HÙ RÉ (RELAY 4 / PA3) KHI NHẬN TÍN HIỆU R1
// (PB9)
// ==============================================================================
#define THOI_GIAN_R4_HU_RE_MS                                                  \
  10000UL // ⏳ Thời gian bật âm thanh hù ré: 10 giây (10000ms)

// ==============================================================================
// 9. CẤU HÌNH THỜI GIAN KỊCH BẢN KHI NHẬN LỆNH 'J' TỪ BO RELAY
// ==============================================================================
// - Bật Relay ở chân A3 (PA3) trong 1 phút (60 giây)
// - Sau đó tắt A3, chờ 5 giây
// - Bật âm thanh ở chân A4 (PA4) trong 15 giây rồi tắt hẳn
#define THOI_GIAN_J_BAT_A3_MS                                                  \
  53000UL // ⏳ Thời gian bật Relay chân A3: 53 giây (giảm 7s từ 60s)
#define THOI_GIAN_J_CHO_A4_MS                                                  \
  5000UL // ⏳ Thời gian chờ trước khi bật A4: 5 giây (5000ms)
#define THOI_GIAN_J_BAT_A4_MS                                                  \
  15000UL // ⏳ Thời gian bật âm thanh chân A4: 15 giây (15000ms)

// ==============================================================================
// 10. CẤU HÌNH NÚT NHẤN RB6 (PB6), NÚT PB5 & RELAY A5 (PA5 / RELAY 6)
// ==============================================================================
// - Chân PB6 (RB6): Bật Relay A5 ON và gửi lệnh 'L' đến STM32_RELAY
// - Chân PB5 (Nút 2): Trong mọi trường hợp, bấm 1 lần bật chân A5, bấm lần nữa
// tắt chân A5
#define PIN_BTN_RB6_PIN PIN_BTN_3_PIN // Nút chân PB6 (RB6)
#define PIN_BTN_PB5_PIN                                                        \
  PIN_BTN_2_PIN        // Nút 2: chân PB5 (Bật / Tắt chân A5 nhạc scralight)
#define RELAY_A5_NUM 6 // Relay 6 tương ứng chân PA5 (A5)

// ==============================================================================
// 11. CẤU HÌNH THỜI GIAN ÂM THANH TIẾNG KẼO KẸT (RELAY 3 / CHÂN A2 - PA2)
// ==============================================================================
// Khi nhận lệnh 'G': Bật Relay A2 trong vòng 5 giây rồi tắt hẳn
#define THOI_GIAN_A2_KEO_KET_MS                                                \
  5000UL // ⏳ Thời gian bật âm thanh tiếng kẽo kẹt: 5 giây (5000ms)

// ==============================================================================
// 12. CẤU HÌNH CẢM BIẾN / NÚT 4 (BTN4 - CHÂN PB7): NHẠC CHIẾN THẮNG & LỆNH 'V'
// ==============================================================================
// - Chân STM32: PB7 (kéo lên nội bộ INPUT_PULLUP)
// - Đầu nối ngoại vi: Cảm biến / Nút nhấn nối GND (Nhận tín hiệu kích âm GND /
// Mức LOW)
// - Chuẩn bảo vệ & lọc nhiễu tham khảo từ chân PA6 của bo STM32_RELAY:
//   + Sau 3s khởi động ổn định nguồn, xác nhận PB7 ở mức HIGH (nghỉ an toàn)
//   mới sẵn sàng.
//   + Lọc chống dội kỹ qua 5 lần đọc liên tiếp trong 50ms
//   (kiemTraNutNhanPB7()).
// - Kịch bản hoạt động:
//   + Bấm lần 1 (BẬT): Gửi lệnh 'V' đến bo Relay, BẬT Relay A6 (PA6 / Relay 7:
//   Nhạc chiến thắng),
//     TẮT toàn bộ A0..A5.
//   + Bấm lần 2 (TẮT): TẮT Relay A6 (PA6).
//   + Có thể kích hoạt BẤT CỨ LÚC NÀO.
#define PIN_BTN_PB1_PIN                                                        \
  PIN_BTN_4_PIN         // Nút 4 / Cảm biến: chân PB1 (kéo lên INPUT_PULLUP)
#define PIN_BTN_PB1 PB1 // Chân PB1 chuẩn Arduino
#define PIN_BTN_PB7_PIN PIN_BTN_4_PIN // Alias tương thích cũ
#define PIN_BTN_PB7 PB1               // Alias tương thích cũ
#define RELAY_A6_NUM 7 // Relay 7 tương ứng chân PA6 (A6: Nhạc chiến thắng)
