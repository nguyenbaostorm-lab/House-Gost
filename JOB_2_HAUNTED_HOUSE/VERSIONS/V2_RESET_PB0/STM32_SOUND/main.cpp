#include "config.h"
#include "pc13_sound.h"
#include "sound_driver.h"
#include "flash_checkpoint.h"
#include <Arduino.h>

// Biến cờ đánh dấu: Vừa vào TẮT TOÀN BỘ cho đến khi nhận được tín hiệu ở PB0
static bool daKichHoatPB0 = false;
static bool daNhanD = false;       // Đã nhận được lệnh D từ bo Relay
static bool daBatA1 = false;       // Đang bật Relay A1 (trò chuyện trên gác)
static bool daTatA1 = false;       // Đã tắt hẳn Relay A1 sau 30s
static bool daGuiLenhF = false;    // Đã gửi lệnh F sang bo Relay
static uint32_t thoiGianNhanD = 0; // Mốc thời gian nhận được lệnh D

// Cờ ghi nhận trạng thái Relay ở chân A6 (PA6 / Relay 7: Nhạc chiến thắng) được kích hoạt bởi nút PB7
static bool cheDoChienThang = false;

// Cờ bảo vệ chân PB7 (tham khảo chuẩn PA6 bên bo STM32_RELAY):
// Sau 3s khởi động ổn định, chân PB7 phải ở mức nghỉ HIGH mới sẵn sàng kích hoạt
static bool pb7DaSanSang = false;

// ==============================================================================
// 1. SETUP: CHẠY ĐÚNG 1 LẦN DUY NHẤT KHI VỪA CẤP NGUỒN (KHỞI ĐỘNG)
// ==============================================================================
void setup() {
  khoiTaoHeThong(); // Khởi tạo 7 relay (PA0..PA6), 6 nút bấm (PB0,PB5..PB9),
                    // UART (PB12/PB13)
  pc13_sound_init(); // Khởi tạo LED PC13

  // 1. ÉP TẮT TUYỆT ĐỐI TOÀN BỘ 7 RELAY ngay khi vừa cấp nguồn
  tatTatCaRelay();

  // 2. KHỞI ĐỘNG ỔN ĐỊNH TRONG VÒNG 3S (3000ms)
  for (int i = 0; i < 30; i++) {
    pc13_sound_update(); // Nhấp nháy đèn báo hoạt động
    tatTatCaRelay(); // Giữ tất cả relay tắt hoàn toàn trong thời gian khởi động
    choMiliGiay(100); // 30 x 100ms = 3000ms = 3 giây
  }

  // 3. Đảm bảo toàn bộ 7 relay tắt sạch an toàn sẵn sàng chờ nút PB0
  tatTatCaRelay();
  pb7DaSanSang = false;
  pc13_sound_set_state(LED_SOUND_CHAM); // Trạng thái chờ: nháy chậm
}

// Biến đếm thời gian cho Relay 4 (PA3 - Âm thanh Hù Ré) khi nhận tín hiệu RF R1
static uint32_t thoiGianBatR4_RF = 0;
static bool r4DangBat = false;

// Biến đếm thời gian cho Relay 3 (PA2 - Âm thanh Tiếng kẽo kẹt) khi nhận lệnh
// 'G'
static uint32_t thoiGianBatA2 = 0;
static bool a2DangBat = false;

// Cờ ghi nhận trạng thái Relay ở chân A5 (PA5 / Relay 6) được kích hoạt bởi nút
// RB6
static bool relayA5DangBat = false;


// ==============================================================================
// CÁC TRẠNG THÁI TIẾN TRÌNH KHI NHẬN LỆNH 'J' TỪ BO RELAY:
// 1. Bật Relay ở chân A3 (PA3) trong 1 phút (60s), đồng thời TẮT chân A0
// 2. Tắt chân A3, BẬT LẠI chân A0, chờ trong vòng 5 giây
// 3. Bật âm thanh ở chân A4 (PA4) trong vòng 15 giây
// 4. Tắt hẳn chân A4, kết thúc chu kỳ
// ==============================================================================
enum TienTrinhLenhJ {
  TIEN_TRINH_J_IDLE = 0,    // Chưa nhận lệnh J
  TIEN_TRINH_J_BAT_A3 = 1,  // Đang BẬT chân A3 (trong 1 phút)
  TIEN_TRINH_J_CHO_5S = 2,  // Đã tắt A3, đang CHỜ 5 giây
  TIEN_TRINH_J_BAT_A4 = 3,  // Đang BẬT chân A4 (trong 15 giây)
  TIEN_TRINH_J_HOAN_TAT = 4 // Đã tắt hẳn chân A4, hoàn tất chu kỳ
};

static TienTrinhLenhJ tienTrinhJ = TIEN_TRINH_J_IDLE;
static uint32_t thoiGianMocJ = 0;

// ==============================================================================
// HÀM KHÔI PHỤC TRẠNG THÁI ÂM THANH THEO CHECKPOINT ĐÃ LƯU TRONG FLASH
// ==============================================================================
void thucHienKhoiPhucSound(uint8_t stage) {
  if (stage == 0) return;

  tatTatCaRelay(); // Ép tắt toàn bộ trước khi khôi phục chuẩn xác
  daKichHoatPB0 = true;
  cheDoChienThang = false;

  switch (stage) {
    case 1: // Pha mở đầu: Bật A0 (Theme), đang chờ chuyển D từ Relay
      batRelay(1);
      daNhanD = false;
      daBatA1 = false;
      daTatA1 = false;
      daGuiLenhF = false;
      break;

    case 2: // Pha gác: Bật A0 và A1
      batRelay(1);
      batRelay(2);
      daNhanD = true;
      daBatA1 = true;
      daTatA1 = false;
      daGuiLenhF = false;
      thoiGianNhanD = layThoiGianMs();
      break;

    case 3: // Pha chờ A6: Duy trì A0
      batRelay(1);
      daNhanD = true;
      daBatA1 = false;
      daTatA1 = true;
      daGuiLenhF = true;
      break;

    case 4: // Pha đã mở cửa A6: Bật A0 và A2 (kẽo kẹt)
      batRelay(1);
      batRelay(3);
      a2DangBat = true;
      thoiGianBatA2 = layThoiGianMs();
      daNhanD = true;
      daBatA1 = false;
      daTatA1 = true;
      daGuiLenhF = true;
      break;

    case 5: // Pha sau bộ gõ (J): Bật A3 (tiến trình J)
      batRelay(4);
      tatRelay(1);
      tienTrinhJ = TIEN_TRINH_J_BAT_A3;
      thoiGianMocJ = layThoiGianMs();
      daNhanD = true;
      daBatA1 = false;
      daTatA1 = true;
      break;

    case 6: // Pha chờ K / sau K: Bật A0
      batRelay(1);
      tienTrinhJ = TIEN_TRINH_J_HOAN_TAT;
      daNhanD = true;
      daBatA1 = false;
      daTatA1 = true;
      break;

    case 7: // Pha Chiến Thắng
      cheDoChienThang = true;
      batRelay(RELAY_A6_NUM);
      break;

    default:
      break;
  }

  pc13_sound_set_state(LED_SOUND_NHAY_3S_KHI_NHAN_D);
}

// ==============================================================================
// 2. LOOP: CHẠY LẶP ĐI LẶP LẠI LIÊN TỤC
// ==============================================================================
void loop() {
  pc13_sound_update(); // Cập nhật nhấp nháy đèn xanh PC13 theo máy trạng thái

  // ============================================================================
  // 👉 1. ĐỌC LỆNH GỬI TỪ BO STM32_RELAY SANG (PB13 RX)
  // Đọc ở đầu vòng lặp để luôn nhận được lệnh dù ở bất kỳ trạng thái nào!
  // ============================================================================
  char lenhTuRelay = docLenhRelay();

  // ============================================================================
  // 👉 2. NÚT BẤM KHÔI PHỤC KHI CÚP ĐIỆN (CHÂN PB8 / NÚT 5 TRÊN BO STM32_SOUND):
  // ⚡ KHI LỠ CÚP ĐIỆN VÀ CÓ ĐIỆN LẠI:
  // - Chỉ cần nhấn nút PB8 đúng 1 lần:
  //   1. Tự động đọc lại mốc đã lưu trong Flash
  //   2. Gửi lệnh 'R' sang bo STM32_RELAY để cả 2 bo cùng khôi phục đồng thời
  //   3. Lập tức chạy tiếp tục ngay khúc đó mà không cần chơi lại từ đầu!
  // ============================================================================
  static bool daNhanPB8 = false;
  if (!daNhanPB8) {
    if (kiemTraNutNhanPB8()) {
      daNhanPB8 = true;
      DuLieuCheckpointSound cp = docCheckpointFlashSound();
      if (cp.magic == FLASH_MAGIC_KEY && cp.stage > 0) {
        guiLenhRelay('R'); // Gửi lệnh 'R' sang bo Relay trước
        choMiliGiay(20);
        thucHienKhoiPhucSound(cp.stage);
      }
    }
  } else {
    if (HAL_GPIO_ReadPin(PIN_BTN_PORT, PIN_BTN_5_PIN) != GPIO_PIN_RESET && digitalRead(PB8) != LOW) {
      daNhanPB8 = false;
    }
  }

  // 👉 KHI NHẬN ĐƯỢC LỆNH 'R' TỪ BO RELAY (Trường hợp nhấn nút khôi phục ở bo Relay):
  if (lenhTuRelay == 'R' || lenhTuRelay == 'r') {
    DuLieuCheckpointSound cp = docCheckpointFlashSound();
    if (cp.magic == FLASH_MAGIC_KEY && cp.stage > 0) {
      thucHienKhoiPhucSound(cp.stage);
    }
  }

  // ============================================================================
  // 👉 QUÉT TÍN HIỆU RF433 R1 (CHÂN PB9): BẬT ÂM THANH HÙ RÉ CHÂN A3 TRONG 10S
  // ⚡ LUÔN LUÔN HOẠT ĐỘNG BẤT KỂ THỜI GIAN NÀO (KỂ CẢ TRƯỚC HAY SAU KHI BẤM
  // PB0)
  // ============================================================================
  static uint32_t thoiGianQuetRF_Sound = 0;
  if (layThoiGianMs() - thoiGianQuetRF_Sound >= 20) {
    thoiGianQuetRF_Sound = layThoiGianMs();
    if (HAL_GPIO_ReadPin(PIN_BTN_PORT, PIN_RF_R1_PIN) == GPIO_PIN_RESET) {
      if (!cheDoChienThang) {
        batRelay(4); // 👉 BẬT Relay ở chân A3 (PA3 - Âm thanh Hù Ré)
        thoiGianBatR4_RF = layThoiGianMs();
        r4DangBat = true;
      }
    }
  }

  // Tự động ngắt Relay 4 (A3) sau đúng 10 giây nếu do RF R1 bật:
  if (r4DangBat &&
      (layThoiGianMs() - thoiGianBatR4_RF >= THOI_GIAN_R4_HU_RE_MS)) {
    if (tienTrinhJ != TIEN_TRINH_J_BAT_A3 && !cheDoChienThang) {
      tatRelay(4); // Chỉ ngắt A3 nếu lệnh J không đang yêu cầu bật A3
    }
    r4DangBat = false;
  }

  // ============================================================================
  // 👉 QUÉT NÚT NHẤN Ở CHÂN RB6 (PB6): BẬT RELAY Ở CHÂN A5 (PA5 / RELAY 6)
  // VÀ GỬI LỆNH 'L' SANG BO STM32_RELAY
  // ⚡ BẤT KỲ KHI NÀO (KỂ CẢ TRƯỚC HAY SAU KHI BẤM PB0)
  // ============================================================================
  static bool daKichHoatRB6 = false;

  if (HAL_GPIO_ReadPin(PIN_BTN_PORT, PIN_BTN_RB6_PIN) == GPIO_PIN_RESET) {
    if (!daKichHoatRB6 && !cheDoChienThang) {
      batRelay(RELAY_A5_NUM); // 👉 BẬT Relay ở chân A5 (PA5 / Relay 6) LÊN ON
      relayA5DangBat = true;
      guiLenhRelay('L');    // 👉 GỬI LỆNH 'L' ĐẾN CON STM32_RELAY
      daKichHoatRB6 = true; // Khóa cờ chống gửi lặp tràn UART
      pc13_sound_set_state(LED_SOUND_NHAY_3S_KHI_NHAN_D);
    }
  } else {
    daKichHoatRB6 =
        false; // Khi thả nút ra: Reset cờ sẵn sàng cho lần nhấn tiếp theo
  }

  // ============================================================================
  // 👉 QUÉT NÚT NHẤN 2 (BTN2 - CHÂN PB5): BẬT / TẮT CHÂN A5 (NHẠC SCRALIGHT)
  // ⚡ TRONG MỌI TRƯỜNG HỢP: Bấm 1 lần -> BẬT A5, bấm lần nữa -> TẮT A5
  // ============================================================================
  static uint32_t thoiGianQuetPB5 = 0;
  static bool daNhanPB5 = false;

  if (layThoiGianMs() - thoiGianQuetPB5 >= 20) {
    thoiGianQuetPB5 = layThoiGianMs();
    if (HAL_GPIO_ReadPin(PIN_BTN_PORT, PIN_BTN_PB5_PIN) == GPIO_PIN_RESET) {
      if (!daNhanPB5 && !cheDoChienThang) {
        daNhanPB5 =
            true; // Khóa chống lặp khi giữ phím (chỉ nhận 1 lần mỗi lần bấm)
        relayA5DangBat = !relayA5DangBat; // Đảo trạng thái BẬT / TẮT (Toggle)
        if (relayA5DangBat) {
          batRelay(RELAY_A5_NUM); // 👉 BẬT Relay ở chân A5 (PA5 / Relay 6 -
                                  // Nhạc scralight)
        } else {
          tatRelay(RELAY_A5_NUM); // 👉 TẮT Relay ở chân A5 (Tắt nhạc scralight)
        }
        pc13_sound_set_state(LED_SOUND_NHAY_3S_KHI_NHAN_D);
      }
    } else {
      daNhanPB5 = false; // Khi nhả nút ra: Sẵn sàng cho lần bấm tiếp theo
    }
  }

  // ============================================================================
  // 👉 LUỒNG XỬ LÝ CHUẨN RIÊNG BIỆT CHO CHÂN NÚT 4 (BTN4 - PB7): NHẠC CHIẾN THẮNG
  // - Áp dụng cơ chế bảo vệ và lọc chống nhiễu giống hệt chân PA6 của bo STM32_RELAY:
  //   1. Chân STM32: PB7 | Đầu nối: Cảm biến / Nút nối GND | Kiểu: Input Pull-Up
  //   2. Nhận tín hiệu kích âm GND (Mức LOW)
  //   3. Bước 1: Sau 3s khởi động ổn định nguồn, xác nhận PB7 ở mức HIGH (nghỉ an toàn)
  //      thì cờ pb7DaSanSang mới được kích hoạt sẵn sàng đón tín hiệu.
  //   4. Bước 2: Khi đã sẵn sàng, lọc chống dội kỹ qua 5 lần đọc liên tiếp trong 50ms
  //      với hàm kiemTraNutNhanPB7() để loại bỏ hoàn toàn mọi xung gai điện từ / nhiễu nguồn.
  //   5. Kịch bản hoạt động (Hoạt động độc lập trong mọi trường hợp, bấm bất cứ lúc nào):
  //      - Nhấn lần 1 (BẬT): Gửi lệnh 'V' đến bo Relay, BẬT Relay A6 (PA6), TẮT toàn bộ A0..A5
  //      - Nhấn lần 2 (TẮT): TẮT Relay A6 (PA6)
  // ============================================================================
  // Bước 1: Sau 3s khởi động, xác nhận chân PB7 đang ở mức HIGH (trạng thái nghỉ an toàn)
  if (!pb7DaSanSang && (layThoiGianMs() >= THOI_GIAN_KHOI_DONG_ON_DINH_MS)) {
    if (HAL_GPIO_ReadPin(PIN_BTN_PORT, PIN_BTN_4_PIN) == GPIO_PIN_RESET || digitalRead(PB7) == LOW) {
      // Đang ở mức LOW -> chưa sẵn sàng (đang giữ nút hoặc cảm biến kích sớm từ trước)
    } else {
      pb7DaSanSang = true; // PB7 đã vào trạng thái nghỉ chuẩn (HIGH), sẵn sàng đón tín hiệu kích âm
    }
  }

  // Bước 2: Quét phát hiện tín hiệu nhấn nút / cảm biến kích âm GND qua kiemTraNutNhanPB7()
  static bool daNhanPB7 = false; // Khóa chống lặp khi giữ nút / cảm biến đang kích
  if (pb7DaSanSang) {
    if (!daNhanPB7) {
      if (kiemTraNutNhanPB7()) {
        daNhanPB7 = true; // Khóa chống lặp khi giữ phím

        // 🔄 ĐẢO TRẠNG THÁI (TOGGLE): Bấm 1 lần BẬT, bấm 1 lần nữa TẮT
        cheDoChienThang = !cheDoChienThang;

        if (cheDoChienThang) {
          // 👉 LẦN BẤM 1: BẬT NHẠC CHIẾN THẮNG
          // 1. TẮT TẤT CẢ CÁC RELAY CÒN LẠI (A0, A1, A2, A3, A4, A5)
          tatRelay(1); // Tắt A0 (PA0)
          tatRelay(2); // Tắt A1 (PA1)
          tatRelay(3); // Tắt A2 (PA2)
          tatRelay(4); // Tắt A3 (PA3)
          tatRelay(5); // Tắt A4 (PA4)
          tatRelay(6); // Tắt A5 (PA5)

          // 2. BẬT DUY NHẤT CHÂN RELAY A6 (PA6 / Relay 7: Nhạc chiến thắng)
          batRelay(RELAY_A6_NUM);

          // 3. GỬI LỆNH 'V' ĐẾN CON STM32_RELAY
          guiLenhRelay('V');
          ghiCheckpointFlashSound(7, 1); // 💾 Lưu checkpoint Mốc 7 (Chiến Thắng)

          // 4. Hủy các tiến trình âm thanh khác để nhạc chiến thắng phát độc quyền
          a2DangBat = false;
          r4DangBat = false;
          relayA5DangBat = false;
          tienTrinhJ = TIEN_TRINH_J_IDLE;
          daBatA1 = false;
          daTatA1 = true;

          pc13_sound_set_state(LED_SOUND_NHAY_3S_KHI_NHAN_D);
        } else {
          // 👉 LẦN BẤM 2: NHẤN 1 LẦN NỮA ĐỂ TẮT HỆ THỐNG VÀ ĐƯA VỀ BAN ĐẦU
          // 1. TẮT HẲN chân Relay A6 (Tắt nhạc chiến thắng) và toàn bộ relay âm thanh
          tatRelay(RELAY_A6_NUM);
          tatTatCaRelay();

          // 2. GỬI LỆNH 'V' LẦN 2 ĐẾN BO RELAY ĐỂ TẮT HỆ THỐNG VÀ ĐƯA VỀ BAN ĐẦU
          guiLenhRelay('V');
          resetCheckpointFlashSound(); // 💾 Xóa sạch checkpoint về 0 khi tắt hệ thống

          // 3. Đưa toàn bộ bo Sound về trạng thái ban đầu, sẵn sàng chờ bấm PB0 cho kịch bản mới
          daKichHoatPB0 = false;
          daNhanD = false;
          daBatA1 = false;
          daTatA1 = false;
          daGuiLenhF = false;
          a2DangBat = false;
          r4DangBat = false;
          relayA5DangBat = false;
          tienTrinhJ = TIEN_TRINH_J_IDLE;

          pc13_sound_set_state(LED_SOUND_CHAM);
        }
      }
    } else {
      // Khi nhả nút / nhả cảm biến về lại mức HIGH ổn định: mở khóa đón lần kích tiếp theo
      if (HAL_GPIO_ReadPin(PIN_BTN_PORT, PIN_BTN_4_PIN) != GPIO_PIN_RESET && digitalRead(PB7) != LOW) {
        daNhanPB7 = false;
      }
    }
  }

  // 🛡️ NẾU ĐANG Ở CHẾ ĐỘ CHIẾN THẮNG: LUÔN DUY TRÌ BẬT A6 VÀ ÉP TẮT SẠCH A0..A5
  if (cheDoChienThang) {
    tatRelay(1);
    tatRelay(2);
    tatRelay(3);
    tatRelay(4);
    tatRelay(5);
    tatRelay(6);
    batRelay(RELAY_A6_NUM);
  }

  // ============================================================================
  // 👉 THUẬT TOÁN FSM CHỐNG DỘI 2 SƯỜN & KHÓA CHỐNG NHÁY RELAY CHO NÚT START/RESET (PB0 / RB0)
  // 🛡️ 1. Lọc chống dội sườn xuống (Press Debounce): Đọc liên tục 50ms mức LOW ổn định
  // 🛡️ 2. Lọc chống dội sườn lên (Release Debounce): Đọc liên tục 80ms mức HIGH ổn định
  // 🛡️ 3. Cửa sổ khóa chống kích đúp liên tiếp (Interlock Cooldown Lockout): 1000ms (1 giây)
  //      Triệt tiêu 100% hiện tượng vừa Start lập tức bị Reset hoặc ngược lại gây nháy relay!
  // ============================================================================
  enum TrangThaiFsmPB0 {
    FSM_PB0_CHO_NHAN = 0,    // Trạng thái nghỉ: Nút ở mức HIGH (chờ bấm)
    FSM_PB0_LOC_NHAN = 1,    // Đang phát hiện LOW: đếm lọc sườn xuống (>= 50ms)
    FSM_PB0_CHO_NHA  = 2,    // Đã thực thi lệnh: chờ người dùng nhả nút
    FSM_PB0_LOC_NHA  = 3     // Đang phát hiện HIGH: đếm lọc sườn lên (>= 80ms)
  };

  static TrangThaiFsmPB0 fsmPB0 = FSM_PB0_CHO_NHAN;
  static uint32_t timerLocPB0 = 0;
  static uint32_t thoiGianKhoaCooldownPB0 = 0; // Cửa sổ khóa 1000ms sau mỗi lần bấm
  uint32_t nowSound = layThoiGianMs();

  bool pb0MucThap = (HAL_GPIO_ReadPin(PIN_BTN_PORT, PIN_NUT_NHAN_1) == GPIO_PIN_RESET) || 
                    (digitalRead(PB0) == LOW);

  switch (fsmPB0) {
    case FSM_PB0_CHO_NHAN:
      if (pb0MucThap) {
        timerLocPB0 = nowSound;
        fsmPB0 = FSM_PB0_LOC_NHAN;
      }
      break;

    case FSM_PB0_LOC_NHAN:
      if (pb0MucThap) {
        // Phải giữ LOW liên tục đủ 50ms mới công nhận là bấm thật
        if (nowSound - timerLocPB0 >= 50) {
          fsmPB0 = FSM_PB0_CHO_NHA; // Chuyển sang chờ nhả nút

          // Kiểm tra xem đã hết thời gian khóa chống kích đúp (1000ms) chưa
          if (nowSound >= thoiGianKhoaCooldownPB0) {
            thoiGianKhoaCooldownPB0 = nowSound + 1000; // Khóa 1000ms cho hành động tiếp theo

            if (!daKichHoatPB0) {
              // ================================================================
              // 🟢 LẦN NHẤN 1: KHỞI ĐỘNG KỊCH BẢN (START)
              // ================================================================
              daKichHoatPB0 = true;
              cheDoChienThang = false;
              tatRelay(RELAY_A6_NUM);
              daNhanD = false;
              daBatA1 = false;
              daTatA1 = false;
              daGuiLenhF = false;
              thoiGianNhanD = 0;
              a2DangBat = false;
              thoiGianBatA2 = 0;
              r4DangBat = false;
              relayA5DangBat = false;
              tienTrinhJ = TIEN_TRINH_J_IDLE;

              batRelay(1);             // 👉 Bật Relay A0 (PA0 - Theme) ON
              guiLenhRelay('S');       // 👉 Gửi lệnh 'S' sang bo STM32_RELAY
              ghiCheckpointFlashSound(1); // 💾 Lưu checkpoint Mốc 1
              pc13_sound_set_state(LED_SOUND_NHANH);
            } else {
              // ================================================================
              // 🛑 LẦN NHẤN NỮA: RESET VỀ BAN ĐẦU TOÀN BỘ CẢ 2 BO MẠCH!
              // ================================================================
              tatTatCaRelay();         // 1. Tắt toàn bộ 7 relay âm thanh trên Sound
              guiLenhRelay('0');       // 2. Gửi lệnh '0' sang bo STM32_RELAY
              resetCheckpointFlashSound(); // 3. Xóa sạch checkpoint Flash về 0

              daKichHoatPB0 = false;   // 4. Đưa về trạng thái ban đầu
              cheDoChienThang = false;
              daNhanD = false;
              daBatA1 = false;
              daTatA1 = false;
              daGuiLenhF = false;
              thoiGianNhanD = 0;
              a2DangBat = false;
              thoiGianBatA2 = 0;
              r4DangBat = false;
              relayA5DangBat = false;
              tienTrinhJ = TIEN_TRINH_J_IDLE;

              pc13_sound_set_state(LED_SOUND_CHAM); // 5. PC13 nháy chậm
            }
          }
        }
      } else {
        // Bị ngắt quãng trước 50ms -> là xung nhiễu / rung phím -> hủy
        fsmPB0 = FSM_PB0_CHO_NHAN;
      }
      break;

    case FSM_PB0_CHO_NHA:
      // Chờ người dùng bắt đầu thả nút (về mức HIGH)
      if (!pb0MucThap) {
        timerLocPB0 = nowSound;
        fsmPB0 = FSM_PB0_LOC_NHA;
      }
      break;

    case FSM_PB0_LOC_NHA:
      if (!pb0MucThap) {
        // Phải giữ mức HIGH liên tục ổn định đủ 80ms mới xác nhận đã nhả phím xong
        if (nowSound - timerLocPB0 >= 80) {
          fsmPB0 = FSM_PB0_CHO_NHAN; // Hoàn tất chu kỳ nhấn-nhả, sẵn sàng đón lần nhấn mới
        }
      } else {
        // Nút bị rung dội lại LOW trong lúc nhả -> reset bộ đếm nhả, quay lại chờ nhả
        fsmPB0 = FSM_PB0_CHO_NHA;
      }
      break;
  }

  // ============================================================================
  // GIAI ĐOẠN 1: KHI CHƯA CÓ TÍN HIỆU PB0 -> CHỜ NHẤN NÚT START ĐỂ BẮT ĐẦU
  // ============================================================================
  if (!daKichHoatPB0) {
    return; // Chưa có tín hiệu PB0 thì dừng tại đây, không xử lý kịch bản UART
  }

  // ============================================================================
  // GIAI ĐOẠN 2: ĐÃ BẤM PB0 -> RELAY A0 ĐANG BẬT, CHỜ NHẬN LỆNH TỪ BO RELAY
  // (Biến lenhTuRelay đã được đọc liên tục ở đầu hàm loop())
  // ============================================================================
  if (lenhTuRelay != 0) {
    // Nếu đang ở chế độ Chiến Thắng (A6) thì chỉ chấp nhận lệnh dừng '0' hoặc lệnh khởi động lại 'S'
    if (cheDoChienThang && lenhTuRelay != '0' && lenhTuRelay != 'S' && lenhTuRelay != 's') {
      return;
    }

    // 👉 KHI NHẬN ĐƯỢC LỆNH 'D':
    if (lenhTuRelay == 'D' || lenhTuRelay == 'd') {
      batRelay(1); // 👉 Duy trì Relay ở chân A0 (PA0 - Âm thanh 1 Theme) BẬT
      batRelay(
          2); // 👉 BẬT THÊM RELAY A1 (PA1 - Âm thanh 2 Trò chuyện trên gác)

      // 👉 ĐÈN PC13 NHÁY TRONG VÒNG 3S
      pc13_sound_set_state(LED_SOUND_NHAY_3S_KHI_NHAN_D);

      // 👉 Bắt đầu đếm thời gian 30s để bật A1, sau đó tắt hẳn A1 và gửi lệnh F
      daNhanD = true;
      daBatA1 = true;
      daTatA1 = false;
      daGuiLenhF = false;
      thoiGianNhanD = layThoiGianMs();
      ghiCheckpointFlashSound(2); // 💾 Lưu checkpoint Mốc 2 (Trò chuyện gác)
    }
    // 👉 KHI NHẬN ĐƯỢC LỆNH 'F' (HOẶC 'f') TỪ BO RELAY:
    else if (lenhTuRelay == 'F' || lenhTuRelay == 'f') {
      // ⚡ KHÔNG BẬT RELAY HAY GÌ CẢ: Chỉ nhận lệnh F thôi và chờ đến khi nhận
      // lệnh G
      pc13_sound_set_state(LED_SOUND_NHAY_3S_KHI_NHAN_D);
    }
    // 👉 KHI NHẬN ĐƯỢC LỆNH 'G' (HOẶC 'g') TỪ BO RELAY (BẤT CỨ LÚC NÀO):
    else if (lenhTuRelay == 'G' || lenhTuRelay == 'g') {
      batRelay(1); // 👉 Duy trì Relay ở chân A0 (PA0 - Âm thanh 1 Theme) BẬT
      batRelay(3); // 👉 BẬT THÊM Relay ở chân A2 (PA2 - Âm thanh 3 Tiếng kẽo
                   // kẹt trong vòng 5 giây)
      a2DangBat = true;
      thoiGianBatA2 = layThoiGianMs();
      guiLenhRelay('H'); // 👉 GỬI LỆNH 'H' CHO BO STM32_RELAY
      ghiCheckpointFlashSound(4, 1); // 💾 Lưu checkpoint Mốc 4 (Cửa từ P6 đã mở)
    }
    // 👉 KHI NHẬN ĐƯỢC LỆNH 'J' (HOẶC 'j') TỪ BO RELAY:
    // Bật Relay ở chân A3 (1 phút) và đồng thời tắt A0; sau khi tắt A3 thì bật
    // A0 lại, chờ 5s, bật âm thanh A4 (15s) và tắt hẳn ⚡ CHỈ KÍCH HOẠT KHI
    // TIẾN TRÌNH J ĐANG Ở TRẠNG THÁI RẢNH (IDLE): Tránh nhận lặp lệnh làm reset
    // tiến trình, treo kịch bản
    else if ((lenhTuRelay == 'J' || lenhTuRelay == 'j') &&
             (tienTrinhJ == TIEN_TRINH_J_IDLE)) {
      batRelay(4); // 👉 BẬT Relay ở chân A3 (PA3)
      tatRelay(1); // 👉 ĐỒNG THỜI TẮT chân A0 (PA0 - Âm thanh 1 Theme)
      tienTrinhJ = TIEN_TRINH_J_BAT_A3;
      thoiGianMocJ = layThoiGianMs();
      pc13_sound_set_state(LED_SOUND_NHAY_3S_KHI_NHAN_D);
      ghiCheckpointFlashSound(5, 1); // 💾 Lưu checkpoint Mốc 5 (Sau bộ gõ)
    }
    // Lệnh dừng '0'
    else if (lenhTuRelay == '0') {
      tatTatCaRelay();       // Nhận '0' -> Tắt toàn bộ relay
      resetCheckpointFlashSound(); // 💾 Xóa sạch checkpoint về 0
      daKichHoatPB0 = false; // Reset về trạng thái chờ bấm PB0
      cheDoChienThang = false; // Reset chế độ chiến thắng
      daNhanD = false;
      daBatA1 = false;
      daTatA1 = false;
      daGuiLenhF = false;
      r4DangBat = false;
      a2DangBat = false;
      relayA5DangBat = false;
      tienTrinhJ = TIEN_TRINH_J_IDLE; // Reset kịch bản lệnh J
      pc13_sound_set_state(LED_SOUND_CHAM);
    }
  }

  // ============================================================================
  // GIAI ĐOẠN 3: KHI NHẬN LỆNH D -> BẬT A1 TRONG 30S RỒI TẮT HẲN, SAU ĐÓ GỬI
  // LỆNH 'F'
  // ============================================================================
  if (daNhanD && daBatA1 && !daTatA1) {
    if (layThoiGianMs() - thoiGianNhanD >= THOI_GIAN_BAT_A1_MS) {
      tatRelay(2); // 👉 Hết 30 giây: TẮT HẲN Relay A1 (không bật nữa)
      daBatA1 = false;
      daTatA1 = true;
      guiLenhRelay('F'); // 👉 SAU ĐÓ LÀ GỬI LỆNH 'F' SANG BO STM32_RELAY
      daGuiLenhF = true;
      ghiCheckpointFlashSound(3); // 💾 Lưu checkpoint Mốc 3 (Xong gác, chờ A6)
    }
  }

  // ============================================================================
  // 👉 TỰ ĐỘNG TẮT RELAY A2 (TIẾNG KẼO KẸT) SAU ĐÚNG 5 GIÂY
  // (THOI_GIAN_A2_KEO_KET_MS)
  // ============================================================================
  if (a2DangBat &&
      (layThoiGianMs() - thoiGianBatA2 >= THOI_GIAN_A2_KEO_KET_MS)) {
    tatRelay(3); // 👉 Hết 5 giây: TẮT HẲN Relay A2 (tiếng kẽo kẹt)
    a2DangBat = false;
  }

  // ============================================================================
  // GIAI ĐOẠN 4: TIẾN TRÌNH TỰ ĐỘNG KHI NHẬN LỆNH 'J' TỪ BO RELAY
  // - Bước 1: Bật Relay ở chân A3 trong vòng 1 phút (60 giây)
  // - Bước 2: Tắt A3, chờ trong vòng 5 giây
  // - Bước 3: Bật âm thanh ở chân A4 trong vòng 30 giây
  // - Bước 4: Tắt hẳn chân A4, kết thúc chu kỳ
  // ============================================================================
  switch (tienTrinhJ) {
  case TIEN_TRINH_J_BAT_A3:
    // Bật chân A3 đủ 1 phút (THOI_GIAN_J_BAT_A3_MS = 60000ms):
    if (layThoiGianMs() - thoiGianMocJ >= THOI_GIAN_J_BAT_A3_MS) {
      tatRelay(4); // 👉 Hết 1 phút: TẮT chân A3
      batRelay(
          1); // 👉 SAU KHI TẮT A3: BẬT LẠI CHÂN A0 (PA0 - Âm thanh 1 Theme)
      tienTrinhJ = TIEN_TRINH_J_CHO_5S;
      thoiGianMocJ = layThoiGianMs();
    }
    break;

  case TIEN_TRINH_J_CHO_5S:
    // Chờ trong 5 giây (THOI_GIAN_J_CHO_A4_MS = 5000ms):
    if (layThoiGianMs() - thoiGianMocJ >= THOI_GIAN_J_CHO_A4_MS) {
      batRelay(5); // 👉 Sau 5s chờ: BẬT âm thanh chân A4 (PA4)
      tienTrinhJ = TIEN_TRINH_J_BAT_A4;
      thoiGianMocJ = layThoiGianMs();
    }
    break;

  case TIEN_TRINH_J_BAT_A4:
    // Bật chân A4 đủ 15 giây (THOI_GIAN_J_BAT_A4_MS = 15000ms):
    if (layThoiGianMs() - thoiGianMocJ >= THOI_GIAN_J_BAT_A4_MS) {
      tatRelay(5);       // 👉 Hết 15s: TẮT HẲN chân A4
      guiLenhRelay('K'); // 👉 GỬI LỆNH 'K' CHO BO STM32_RELAY
      tienTrinhJ = TIEN_TRINH_J_HOAN_TAT;
      ghiCheckpointFlashSound(6, 1); // 💾 Lưu checkpoint Mốc 6 (Búp bê rơi)
    }
    break;

  case TIEN_TRINH_J_HOAN_TAT:
  case TIEN_TRINH_J_IDLE:
  default:
    break;
  }
}

// ==============================================================================
// ĐIỂM VÀO CHÍNH (TƯƠNG THÍCH CẢ KEIL C / STM32CUBEIDE LẪN PLATFORMIO)
// ==============================================================================
#if !defined(ARDUINO)
int main(void) {
  HAL_Init();
  setup();
  while (1) {
    loop();
  }
}
#endif
