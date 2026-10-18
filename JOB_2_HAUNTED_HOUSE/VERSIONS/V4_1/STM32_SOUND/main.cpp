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
// CƠ CHẾ: KHI BẬT NHẠC JUMPSCARE THÌ NHẠC THEME CỦA A1 TẮT, HẾT THỜI GIAN THÌ BẬT LẠI
// ==============================================================================
static bool g_themeA1_TamTatDoJump = false;
static bool g_themeA0_TamTatDoJump = false;
static uint32_t g_thoiGianBatDauTamDungTheme = 0;

void batNhacJumpscareA3(void) {
  // 1. TẮT NHẠC THEME CỦA A1 (Relay 2 / PA1) nếu đang bật
  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == RELAY_ON || (daBatA1 && !daTatA1)) {
    g_themeA1_TamTatDoJump = true;
    tatRelay(2); // 👉 TẮT NGAY NHẠC THEME CỦA A1
  }
  // Đồng thời tắt nhạc Theme A0 (Relay 1 / PA0) nếu đang bật
  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == RELAY_ON) {
    g_themeA0_TamTatDoJump = true;
    tatRelay(1); // 👉 TẮT NGAY NHẠC THEME CỦA A0
  }
  g_thoiGianBatDauTamDungTheme = layThoiGianMs();

  // 2. BẬT Relay A3 (PA3 / Relay 4 - Âm thanh Jumpscare / Hù ré)
  batRelay(4);
}

void tatNhacJumpscareA3(void) {
  // 1. TẮT Relay A3 (Hết thời gian Jumpscare)
  tatRelay(4);

  // 2. KHÔI PHỤC LẠI NHẠC THEME:
  if (g_themeA1_TamTatDoJump) {
    batRelay(2); // 👉 BẬT LẠI NHẠC THEME CỦA A1!
    g_themeA1_TamTatDoJump = false;
    // Bù thêm thời gian bị ngắt quãng để A1 vẫn phát trọn vẹn đủ thời lượng kịch bản
    if (daBatA1 && !daTatA1) {
      thoiGianNhanD += (layThoiGianMs() - g_thoiGianBatDauTamDungTheme);
    }
  }
  if (g_themeA0_TamTatDoJump) {
    batRelay(1); // 👉 BẬT LẠI NHẠC THEME CỦA A0!
    g_themeA0_TamTatDoJump = false;
  }
}

// ==============================================================================
// 2. LOOP: CHẠY LẶP ĐI LẶP LẠI LIÊN TỤC
// ==============================================================================
void loop() {
  pc13_sound_update(); // Cập nhật nhấp nháy đèn xanh PC13 theo máy trạng thái

  // ============================================================================
  char lenhTuRelay = docLenhRelay();

  // ============================================================================
  // 👉 1B. ĐỌC LỆNH GỬI TỪ BO STM32_JUMPSCARE SANG (PB11 RX)
  // Khi công tắc trên bo Jumpscare kích -> gửi 'T' -> Bo Sound giữ nguyên âm thanh,
  // lập tức chuyển tiếp lệnh 'T' sang bo STM32_RELAY để tắt sạch đèn!
  // ============================================================================
  char lenhTuJump = docLenhJumpscare();
  if (lenhTuJump == 'T' || lenhTuJump == 't') {
    guiLenhRelay('T'); // Chuyển tiếp sang bo STM32_RELAY để tắt sạch đèn
  }

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
        if (!r4DangBat) {
          batNhacJumpscareA3(); // 👉 BẬT Relay A3 & TẮT NHẠC THEME CỦA A1 (và A0)
        }
        thoiGianBatR4_RF = layThoiGianMs();
        r4DangBat = true;
      }
    }
  }

  // Tự động ngắt Relay 4 (A3) sau đúng 10 giây nếu do RF R1 bật:
  if (r4DangBat &&
      (layThoiGianMs() - thoiGianBatR4_RF >= THOI_GIAN_R4_HU_RE_MS)) {
    if (tienTrinhJ != TIEN_TRINH_J_BAT_A3 && !cheDoChienThang) {
      tatNhacJumpscareA3(); // 👉 Hết 10s: TẮT A3 & BẬT LẠI NHẠC THEME CỦA A1 (và A0)!
    }
    r4DangBat = false;
  }

  // ============================================================================
  // 👉 QUÉT NÚT NHẤN Ở CHÂN RB6 (PB6): BẬT RELAY Ở CHÂN A5 (PA5 / RELAY 6)
  // VÀ GỬI LỆNH 'L' SANG BO STM32_RELAY
  // 🛡️ LỌC CHỐNG DỘI CHUẨN XÁC & CHỐNG GỬI LẶP UART
  // ⚡ BẤT KỲ KHI NÀO (KỂ CẢ TRƯỚC HAY SAU KHI BẤM PB0)
  // ============================================================================
  static bool daKichHoatRB6 = false;
  static uint32_t timerLocRB6 = 0;
  static bool dangKiemTraRB6 = false;
  bool rb6MucThap = (HAL_GPIO_ReadPin(PIN_BTN_PORT, PIN_BTN_RB6_PIN) == GPIO_PIN_RESET) || 
                    (digitalRead(PB6) == LOW);

  if (rb6MucThap) {
    if (!daKichHoatRB6) {
      if (!dangKiemTraRB6) {
        dangKiemTraRB6 = true;
        timerLocRB6 = layThoiGianMs();
      } else if (layThoiGianMs() - timerLocRB6 >= 40) {
        // Giữ mức LOW đủ 40ms -> Kích hoạt!
        batRelay(RELAY_A5_NUM); // 👉 BẬT Relay ở chân A5 (PA5 / Relay 6) LÊN ON
        relayA5DangBat = true;
        guiLenhRelay('L');    // 👉 GỬI LỆNH 'L' ĐẾN CON STM32_RELAY
        daKichHoatRB6 = true; // Khóa cờ chống gửi lặp tràn UART
        dangKiemTraRB6 = false;
        pc13_sound_set_state(LED_SOUND_NHAY_3S_KHI_NHAN_D);
      }
    }
  } else {
    dangKiemTraRB6 = false;
    daKichHoatRB6 = false; // Khi thả nút ra: Sẵn sàng cho lần nhấn tiếp theo
  }

  // ============================================================================
  // 👉 QUÉT NÚT NHẤN 2 (BTN2 - CHÂN PB5): BẬT / TẮT CHÂN A5 (NHẠC SCA NIGHT)
  // 🛡️ BỘ LỌC CHỐNG DỘI FSM 2 SƯỜN & KHÓA CHỐNG KÍCH ĐÚP (COOLDOWN 500MS)
  // ⚡ TRONG MỌI TRƯỜNG HỢP: Bấm 1 lần -> BẬT A5, bấm lần nữa -> TẮT A5
  // ============================================================================
  enum TrangThaiFsmPB5 {
    FSM_PB5_CHO_NHAN = 0,    // Trạng thái nghỉ: Nút ở mức HIGH (chờ bấm)
    FSM_PB5_LOC_NHAN = 1,    // Đang phát hiện LOW: đếm lọc sườn xuống (>= 40ms)
    FSM_PB5_CHO_NHA  = 2,    // Đã toggle: chờ người dùng nhả nút
    FSM_PB5_LOC_NHA  = 3     // Đang phát hiện HIGH: đếm lọc sườn lên (>= 60ms)
  };

  static TrangThaiFsmPB5 fsmPB5 = FSM_PB5_CHO_NHAN;
  static uint32_t timerLocPB5 = 0;
  static uint32_t thoiGianKhoaCooldownPB5 = 0; // Khóa chống kích đúp 500ms
  uint32_t nowPB5 = layThoiGianMs();

  bool pb5MucThap = (HAL_GPIO_ReadPin(PIN_BTN_PORT, PIN_BTN_PB5_PIN) == GPIO_PIN_RESET) || 
                    (digitalRead(PB5) == LOW);

  switch (fsmPB5) {
    case FSM_PB5_CHO_NHAN:
      if (pb5MucThap) {
        timerLocPB5 = nowPB5;
        fsmPB5 = FSM_PB5_LOC_NHAN;
      }
      break;

    case FSM_PB5_LOC_NHAN:
      if (pb5MucThap) {
        // Phải giữ mức LOW liên tục ổn định đủ 40ms mới công nhận là bấm thật
        if (nowPB5 - timerLocPB5 >= 40) {
          fsmPB5 = FSM_PB5_CHO_NHA; // Chuyển sang chờ nhả phím

          // Kiểm tra xem đã hết thời gian khóa cooldown (500ms) chưa
          if (nowPB5 >= thoiGianKhoaCooldownPB5) {
            thoiGianKhoaCooldownPB5 = nowPB5 + 500; // Khóa 500ms chống nảy phím

            relayA5DangBat = !relayA5DangBat; // Đảo trạng thái BẬT / TẮT (Toggle)
            if (relayA5DangBat) {
              batRelay(RELAY_A5_NUM); // 👉 BẬT Relay ở chân A5 (PA5 / Relay 6: Nhạc sca night)
            } else {
              tatRelay(RELAY_A5_NUM); // 👉 TẮT Relay ở chân A5 (Tắt nhạc sca night)
            }
            pc13_sound_set_state(LED_SOUND_NHAY_3S_KHI_NHAN_D);
          }
        }
      } else {
        // Xung nhiễu chập chờn dưới 40ms -> hủy
        fsmPB5 = FSM_PB5_CHO_NHAN;
      }
      break;

    case FSM_PB5_CHO_NHA:
      // Chờ người dùng thả nút ra (về mức HIGH)
      if (!pb5MucThap) {
        timerLocPB5 = nowPB5;
        fsmPB5 = FSM_PB5_LOC_NHA;
      }
      break;

    case FSM_PB5_LOC_NHA:
      if (!pb5MucThap) {
        // Phải giữ mức HIGH liên tục ổn định đủ 60ms mới xác nhận đã nhả phím hoàn toàn
        if (nowPB5 - timerLocPB5 >= 60) {
          fsmPB5 = FSM_PB5_CHO_NHAN; // Sẵn sàng cho lần bấm tiếp theo
        }
      } else {
        // Nút bị rung dội lại LOW trong lúc nhả -> quay lại chờ nhả
        fsmPB5 = FSM_PB5_CHO_NHA;
      }
      break;
  }

  // ============================================================================
  // 👉 1. LUỒNG XỬ LÝ CHUẨN CHO NÚT 4 (BTN4 - PB7): NHẠC CHIẾN THẮNG & LỆNH 'V' (VIN)
  // 🛡️ LỌC CHỐNG NHIỄU 5 MẪU LIÊN TIẾP 50MS (CHUẨN CHÂN PA6 BO RELAY) & KHÓA CHỐNG DỘI 800MS
  // ⚡ Bấm lần 1: BẬT A6 (Nhạc Vin) & Gửi lệnh 'V' sang bo Relay
  // ⚡ Bấm lần 2: TẮT A6 & Gửi lệnh 'V' lần 2 sang bo Relay để tắt toàn bộ hệ thống
  // ⚡ BẤM BẤT CỨ LÚC NÀO!
  // ============================================================================
  static bool daKichPB7 = false;
  static uint32_t thoiGianKhoaPB7 = 0;
  uint32_t nowSound = layThoiGianMs();

  if (nowSound >= THOI_GIAN_KHOI_DONG_ON_DINH_MS) {
    if (!daKichPB7 && (nowSound >= thoiGianKhoaPB7) && kiemTraNutNhanPB7()) {
      daKichPB7 = true;
      thoiGianKhoaPB7 = nowSound + 800; // Khóa 800ms triệt tiêu 100% rung dội sườn nhả phím

      // 🔄 ĐẢO TRẠNG THÁI (TOGGLE): Bấm 1 lần BẬT, bấm 1 lần nữa TẮT
      cheDoChienThang = !cheDoChienThang;

      if (cheDoChienThang) {
        // 👉 LẦN BẤM 1: BẬT NHẠC CHIẾN THẮNG & GỬI LỆNH 'V'
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

    // Khi người dùng đã nhả nút và đã qua 800ms -> sẵn sàng đón nhận lần bấm tiếp theo
    if (daKichPB7 && (nowSound >= thoiGianKhoaPB7)) {
      bool pb1DaTha = (HAL_GPIO_ReadPin(PIN_BTN_PORT, PIN_BTN_4_PIN) == GPIO_PIN_SET) && 
                      (digitalRead(PB1) == HIGH);
      if (pb1DaTha) {
        daKichPB7 = false;
      }
    }
  }

  // 🛡️ NẾU ĐANG Ở CHẾ ĐỘ CHIẾN THẮNG: LUÔN DUY TRÌ BẬT A6 VÀ ÉP TẮT SẠCH A0..A5 (TRỪ KHI BẬT SCA NIGHT)
  if (cheDoChienThang) {
    tatRelay(1);
    tatRelay(2);
    tatRelay(3);
    tatRelay(4);
    tatRelay(5);
    if (!relayA5DangBat) {
      tatRelay(6);
    }
    batRelay(RELAY_A6_NUM);
  }

  // ============================================================================
  // 👉 2. NÚT START / KHỞI ĐỘNG LẠI (BTN1 - PB0):
  // 🟢 NHẤN 1 LẦN: BẮT ĐẦU KỊCH BẢN (START) -> BẬT A0, GỬI 'S', PC13 NHÁY NHANH
  // 🛑 NHẤN 1 LẦN NỮA: KHỞI ĐỘNG LẠI (RESET) -> TẮT TOÀN BỘ 7 RELAY, GỬI '0', PC13 NHÁY CHẬM
  // ============================================================================
  static bool daKichPB0 = false;
  static uint32_t thoiGianKhoaPB0 = 0;

  if (nowSound >= THOI_GIAN_KHOI_DONG_ON_DINH_MS) {
    if (!daKichPB0 && (nowSound >= thoiGianKhoaPB0) && kiemTraNutNhanPB0()) {
      daKichPB0 = true;
      thoiGianKhoaPB0 = nowSound + 800; // Khóa 800ms chống nảy phím

      if (!daKichHoatPB0) {
        // ================================================================
        // 🟢 LẦN NHẤN 1: BẮT ĐẦU KỊCH BẢN (START)
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

        batRelay(1);             // 👉 Bật Relay A0 (PA0 - Âm thanh Theme) LÊN ON
        guiLenhRelay('S');       // 👉 Gửi lệnh 'S' sang bo STM32_RELAY
        ghiCheckpointFlashSound(1); // 💾 Lưu checkpoint Mốc 1 (Bắt đầu)
        pc13_sound_set_state(LED_SOUND_NHANH);
      } else {
        // ================================================================
        // 🛑 LẦN NHẤN NỮA: KHỞI ĐỘNG LẠI (RESET) TOÀN BỘ 2 BO MẠCH
        // ================================================================
        tatTatCaRelay();         // 1. Tắt toàn bộ 7 relay âm thanh trên Sound
        guiLenhRelay('0');       // 2. Gửi lệnh '0' sang bo STM32_RELAY
        resetCheckpointFlashSound(); // 3. Xóa sạch checkpoint Flash về 0

        daKichHoatPB0 = false;   // 4. Đưa về trạng thái chờ ban đầu
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

    if (daKichPB0 && (nowSound >= thoiGianKhoaPB0)) {
      bool pb0DaTha = (HAL_GPIO_ReadPin(PIN_BTN_PORT, PIN_NUT_NHAN_1) == GPIO_PIN_SET) && 
                      (digitalRead(PB0) == HIGH);
      if (pb0DaTha) {
        daKichPB0 = false;
      }
    }
  }

  // ============================================================================
  // GIAI ĐOẠN 1: KHI CHƯA CÓ TÍN HIỆU PB0 -> TẮT CÁC RELAY KỊCH BẢN, CHỜ PB0
  // ============================================================================
  if (!daKichHoatPB0) {
    if (!cheDoChienThang) {
      tatRelay(1);
      tatRelay(2);
      tatRelay(3);
      if (!r4DangBat && tienTrinhJ != TIEN_TRINH_J_BAT_A3) {
        tatRelay(4); // Chỉ tắt A3 nếu không đang trong chu kỳ 10s của RF và không
                     // chạy lệnh J
      }
      if (tienTrinhJ != TIEN_TRINH_J_BAT_A4) {
        tatRelay(5);
      }
      if (!relayA5DangBat) {
        tatRelay(6); // Chỉ tắt A5 nếu không đang được bật bởi nút RB6
      }
      tatRelay(7);
    }
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
      batNhacJumpscareA3(); // 👉 BẬT Relay A3 & TẮT NHẠC THEME CỦA A1 (và A0)
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
      g_themeA1_TamTatDoJump = false;
      g_themeA0_TamTatDoJump = false;
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
      tatNhacJumpscareA3(); // 👉 Hết 1 phút: TẮT chân A3 & BẬT LẠI NHẠC THEME CỦA A1 (và A0)!
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
