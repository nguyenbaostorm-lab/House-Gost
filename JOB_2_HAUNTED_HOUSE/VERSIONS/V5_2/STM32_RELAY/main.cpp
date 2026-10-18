#include "config.h"
#include "pc13_relay.h"
#include "pcf8575_relay.h"
#include "flash_checkpoint.h"
#include <Arduino.h>

// ==============================================================================
// CÁC TRẠNG THÁI HOẠT ĐỘNG CỦA HỆ THỐNG STM32_RELAY (THEO FLOWCHART)
// ==============================================================================
enum TrangThaiHeThong {
  TRANG_THAI_KHOI_DONG = 0, // 1. Vừa bật nguồn: ổn định 3s, LED PC13 nháy nhanh
  TRANG_THAI_CHO_LENH_S = 1, // 2. Đã ổn định: chờ lệnh S, LED PC13 nháy chậm
  TRANG_THAI_KICH_P0_P3 =
      2, // 3. Đã nhận S: Bật P0 & P3, LED PC13 sáng 3s rồi tắt
  TRANG_THAI_CHUYEN_P1 = 3, // 4. Sau 10s: Tắt P0 & P3, Bật P1, LED PC13 nháy lại
  TRANG_THAI_CHO_50S_P2 = 4, // 5. Sau khi tắt đèn sau bộ gõ: Bật duy nhất P2 trong 50s
  TRANG_THAI_CHO_LENH_K = 5, // 6. Sau 50s: Tắt P2, Bật lại P0 và P1, chờ lệnh K từ STM32_SOUND
  TRANG_THAI_DA_NHAN_K = 6   // 7. Đã nhận lệnh K (P11 búp bê, P13 đèn đỏ)
};

static TrangThaiHeThong trangThai = TRANG_THAI_KHOI_DONG;
static uint32_t thoiGianNhanS = 0;
static uint32_t lanRefreshCuoi = 0;

// Các biến quản lý kịch bản hẹn giờ khi nhận lệnh 'H'
static uint32_t thoiGianNhanH = 0;
static bool daNhanLenhH = false;
static bool daBatR7_SauH = false;
static bool daBatP10_P3_SauH = false;

// Các biến quản lý kịch bản hẹn giờ khi nhận lệnh 'K'
static uint32_t thoiGianNhanK = 0;
static bool daNhanLenhK = false;
static bool daTatP13_Sau10sK = false; // Cờ đánh dấu đã tắt P13 sau 10s (bắt đầu 20s tắt)
static bool daXuLy30sSauK = false;    // Cờ đánh dấu đã qua 30s (10s bật + 20s tắt) và bật lại P3

// ==============================================================================
// CÁC MẪU NHẤP NHÁY NGẪU NHIÊN CHO BỘ GÕ P14 (THEO YÊU CẦU NGƯỜI DÙNG)
// 1. 1s BẬT, 1.5s TẮT
// 2. 2s BẬT, 2s TẮT
// 3. 3s BẬT, 3s TẮT
// ==============================================================================
struct MauBoGo {
  uint32_t thoiGianBatMs;
  uint32_t thoiGianTatMs;
};

static const MauBoGo CAC_MAU_BO_GO[] = {
    {1000UL, 1500UL}, // Mẫu 1: 1s BẬT, 1.5s TẮT
    {2000UL, 2000UL}, // Mẫu 2: 2s BẬT, 2s TẮT
    {3000UL, 3000UL}  // Mẫu 3: 3s BẬT, 3s TẮT
};
static const uint8_t SO_MAU_BO_GO =
    sizeof(CAC_MAU_BO_GO) / sizeof(CAC_MAU_BO_GO[0]);

// Các biến quản lý kịch bản bộ gõ P14 khi nhấn nút PA5
static bool boGoDangChay = false;
static uint32_t thoiGianBatDauBoGo = 0;
static uint32_t thoiGianChuyenTrangThaiBoGo = 0;
static uint32_t thoiGianGiuTrangThaiHienTai = 0;
static bool trangThaiBoGoHienTai = false;
static uint8_t chiSoMauHienTai = 0;
static bool daKetThuc30sBoGo = false;
static bool daTatDenSauBoGo = false;
static bool daGuiLenhJ = false;
static bool daXuLyTinHieuA5 = false; // Chốt chỉ nhận mất tín hiệu A5 đúng 1 lần duy nhất!
static bool daXuLyTinHieuA6 = false; // Chốt chỉ nhận kích âm A6 đúng 1 lần duy nhất -> KHÔNG BAO GIỜ TẮT CHO ĐẾN KHI NHẤN NÚT RESET!
static bool pa6DaSanSang = false;    // Cờ bảo vệ: PA6 phải vào mức nghỉ HIGH sau khởi động mới sẵn sàng kích hoạt

// Các biến quản lý kịch bản sau khi gửi 'J' (bật P2 trong 50s, sau đó tắt P2 bật lại P0 và P1)
static uint32_t thoiGianGuiJ = 0;
static bool daGuiJ_BatP2 = false;
static bool daTatP2_BatP0_P1 = false;

// Cờ quản lý chế độ Chiến Thắng khi nhận lệnh 'V' từ bo STM32_SOUND:
// - Lần 1: BẬT P12, P0, P1 (và giữ nguyên P6 nếu đã bật)
// - Lần 2: TẮT hệ thống và đưa về ban đầu
static bool cheDoV_DangBat = false;

// ==============================================================================
// HÀM KHÔI PHỤC TRẠNG THÁI RELAY THEO CHECKPOINT ĐÃ LƯU TRONG FLASH
// ==============================================================================
void thucHienKhoiPhucRelay(uint8_t stage, uint8_t p6Mo) {
  if (stage == 0) return;

  uint32_t now = millis();

  // 🔥 NẾU TRƯỚC ĐÓ CỬA TỪ 1 (P6) ĐÃ ĐƯỢC MỞ -> LẬP TỨC MỞ LẠI CHỐT CỬA TỪ P6 NGAY!
  if (p6Mo == 1) {
    g_p6DaBat = true;
    batChanPCF(RELAY_CUA_TU_1_P6);
  }

  switch (stage) {
    case 1: // Khởi động: Bật P0 và P3, đếm 10s chuyển sang P1
      tatTatCaRelay();
      batChanPCF(RELAY_LOA_P0);
      batChanPCF(3);
      thoiGianNhanS = now;
      trangThai = TRANG_THAI_KICH_P0_P3;
      break;

    case 2: // Chuyển P1: Tắt P0, P3; Bật P1 (Loa trên gác)
      tatChanPCF(RELAY_LOA_P0);
      tatChanPCF(3);
      batChanPCF(RELAY_LOA_P1);
      trangThai = TRANG_THAI_CHUYEN_P1;
      break;

    case 3: // Xong gác: Bật P5 (đồ chơi), P0, P1; tắt P3; chờ A6
      batChanPCF(RELAY_LED_DO_CHOI_P5);
      tatChanPCF(3);
      batChanPCF(RELAY_LOA_P0);
      batChanPCF(RELAY_LOA_P1);
      trangThai = TRANG_THAI_CHUYEN_P1;
      break;

    case 4: // A6 đã kích: P6 mở, P5 đồ chơi duy trì bật, P7 tranh bật, P10 & P3 bật, P0, P1 bật
      daXuLyTinHieuA6 = true;
      g_p6DaBat = true;
      batChanPCF(RELAY_CUA_TU_1_P6);
      batChanPCF(RELAY_LED_DO_CHOI_P5);
      batChanPCF(RELAY_DEN_BUC_TRANH_P7);
      batChanPCF(RELAY_DEN_RONG_PHUONG_P10);
      batChanPCF(3);
      batChanPCF(RELAY_LOA_P0);
      batChanPCF(RELAY_LOA_P1);
      daNhanLenhH = true;
      daBatR7_SauH = true;
      daBatP10_P3_SauH = true;
      trangThai = TRANG_THAI_CHUYEN_P1;
      break;

    case 5: // Sau bộ gõ: Bật duy nhất P2 trong 50s
      tatTatCaRelay();
      batChanPCF(RELAY_LOA_P2);
      thoiGianGuiJ = now;
      daGuiJ_BatP2 = true;
      daTatP2_BatP0_P1 = false;
      daTatDenSauBoGo = true;
      daGuiLenhJ = true;
      daXuLyTinHieuA5 = true;
      daKetThuc30sBoGo = true;
      boGoDangChay = false;
      trangThai = TRANG_THAI_CHO_50S_P2;
      break;

    case 6: // Búp bê rơi: Bật P11, P13 (10s), P0, P1
      chuyenPha_NhanLenhK_BatP11_P13_P0_P1();
      thoiGianNhanK = now;
      daNhanLenhK = true;
      daTatP13_Sau10sK = false;
      daXuLy30sSauK = false;
      trangThai = TRANG_THAI_DA_NHAN_K;
      break;

    case 7: // Chế độ Chiến Thắng: P12, P0, P1, P6
      cheDoV_DangBat = true;
      batCheDoChienThangV();
      break;

    default:
      break;
  }

  pc13_relay_set_state(LED_SANG_3S_ROI_TAT);
}

// ==============================================================================
// 1. SETUP: CHẠY ĐÚNG 1 LẦN DUY NHẤT KHI VỪA CẤP NGUỒN (KHỞI ĐỘNG)
// ==============================================================================
void setup() {
  // 🔥 KHỞI TẠO BAN ĐẦU: Lúc đầu tiên LUÔN LUÔN KHÔNG CÓ KÍCH TÍN HIỆU P6 (CỬA TỪ 1 TẮT TUYỆT ĐỐI)
  g_p6DaBat = false;
  daXuLyTinHieuA6 = false;
  pa6DaSanSang = false;
  daXuLyTinHieuA5 = false;
  daGuiJ_BatP2 = false;
  daTatP2_BatP0_P1 = false;
  cheDoV_DangBat = false;
  daTatP13_Sau10sK = false;
  daXuLy30sSauK = false;

  khoiTaoHeThong();  // Cấu hình I2C Open-Drain, UART USART2, GPIO PA6 & PA5
  resetBoLocA5();    // Đảm bảo bộ lọc cảm biến A5 sạch sẽ ngay từ đầu
  pc13_relay_init(); // Cấu hình LED PC13 theo module pc13_relay.h
  tatTatCaRelay();   // Ép tắt sạch toàn bộ 16 relay (0x0000)
  randomSeed(analogRead(PA0) + micros()); // Khởi tạo mầm ngẫu nhiên

  // 👉 1. VỪA BẮT ĐẦU: Ổn định trong vòng 3 giây, đèn PC13 nhấp nháy nhanh
  pc13_relay_set_state(LED_CHOP_NHANH);
  uint32_t thoiGianBatDau = millis();
  while (millis() - thoiGianBatDau < THOI_GIAN_KHOI_DONG_ON_DINH_MS) {
    pc13_relay_update(); // Cập nhật nhấp nháy nhanh (80ms)
    tatTatCaRelay();     // Duy trì ép tắt toàn bộ relay an toàn (P6 luôn TẮT tuyệt đối)
    delay(10);
  }

  // 👉 2. KHI ĐÃ ỔN ĐỊNH: Chuyển sang chờ lệnh 'S', đèn PC13 nhấp nháy chậm
  tatTatCaRelay();
  pc13_relay_set_state(LED_CHOP_CHAM);
  trangThai = TRANG_THAI_CHO_LENH_S;
}

// ==============================================================================
// 2. LOOP: VÒNG LẶP CHÍNH CỦA CHƯƠNG TRÌNH
// ==============================================================================
void loop() {
  // 👉 Luôn cập nhật trạng thái đèn PC13 tự động theo pc13_relay.h
  pc13_relay_update();

  // ==========================================================================
  // 👉 QUÉT & XỬ LÝ 3 KÊNH RF433 TỪ PCF8575 (P15, P16, P17)
  // 🛡️ BỘ LỌC CHỐNG NHIỄU SỐ ĐA MẪU (MULTI-SAMPLE FILTER) & KHÓA CHỐNG XUNG RELAY
  // BẤT KỲ TRƯỜNG HỢP / TRẠNG THÁI NÀO VẪN LUÔN HOẠT ĐỘNG CHUẨN XÁC 100%
  // ==========================================================================
  static uint32_t thoiGianDocRFCuoi = 0;
  static uint32_t thoiGianChopRF = 0;
  static bool trangThaiChopRF = false;
  static bool rfP16_DangKichHoat = false;
  static bool rfP17_DangKichHoat = false;
  static uint8_t demTichCucP15 = 0;
  static uint8_t demTichCucP16 = 0;
  static uint8_t demTichCucP17 = 0;
  static uint16_t g_snapshotDenPcfR4 = 0;
  static bool g_snapshotA7_R4 = false;
  static bool g_daLuuSnapshotR4 = false;
  static bool g_dangCho5sKhoiPhucR4 = false;
  static uint32_t g_timerNhaNutR4 = 0;

  // 🛡️ CHỈ QUÉT RF VÀ REFRESH I2C KHI NGOÀI CỬA SỔ KHÓA CHỐNG NHIỄU (EMI BLANKING WINDOW)
  // Khi rơ-le/cuộn cảm vừa đóng ngắt -> I2C hoàn toàn ở trạng thái nghỉ (Pull-up cao tĩnh),
  // tuyệt đối không gửi nhận dữ liệu để triệt tiêu 100% khả năng xung gai gây sai lệch bit!
  if (millis() >= g_thoiGianKhoaNhietRelay) {
    if (millis() - thoiGianDocRFCuoi >= 20) {
      thoiGianDocRFCuoi = millis();
      uint16_t rawPcf = readPCF8575();

      // 🛡️ LỌC NHIỄU SỐ ĐA TẦNG & XÁC THỰC MÃ RF433 CHUẨN XÁC:
      // 1. rawPcf == 0xFFFF (lỗi I2C/NACK) hoặc 0x0000 (sụt mass kéo toàn bộ chân LOW do nhiễu): BỎ QUA NGAY!
      // 2. Chế độ Active-LOW: Remote thực tế CHỈ CÓ THỂ BẤM DUY NHẤT 1 NÚT TẠI MỘT THỜI ĐIỂM!
      //    Nếu có >= 2 nút cùng nhận LOW -> 100% là xung gai nhiễu điện từ, lập tức loại bỏ & reset bộ lọc!
      if (rawPcf != 0xFFFF && rawPcf != 0x0000) {
        uint8_t rfActiveMask = 0;
        if (docTinHieuRF_P15(rawPcf)) rfActiveMask |= (1 << 0);
        if (docTinHieuRF_P16(rawPcf)) rfActiveMask |= (1 << 1);
        if (docTinHieuRF_P17(rawPcf)) rfActiveMask |= (1 << 2);

        // Duy nhất 1 phím được bấm hợp lệ (1, 2 hoặc 4):
        if (rfActiveMask == 1) { // Kênh R2 (P15)
          if (demTichCucP15 < 6) demTichCucP15++;
          demTichCucP16 = 0;
          demTichCucP17 = 0;
        } else if (rfActiveMask == 2) { // Kênh R3 (P16)
          if (demTichCucP16 < 6) demTichCucP16++;
          demTichCucP15 = 0;
          demTichCucP17 = 0;
        } else if (rfActiveMask == 4) { // Kênh R4 (P17)
          if (demTichCucP17 < 6) demTichCucP17++;
          demTichCucP15 = 0;
          demTichCucP16 = 0;
        } else {
          // Không bấm (rfActiveMask == 0) hoặc nhiễu đồng thời nhiều kênh -> xóa sạch bộ lọc
          demTichCucP15 = 0;
          demTichCucP16 = 0;
          demTichCucP17 = 0;
        }

        // 1. KÊNH R2 (P15): Xác nhận khi giữ liên tục đủ 6 mẫu (120ms)
        if (demTichCucP15 >= 6) {
          tatTatCaRelayP4DenP14();
          daNhanLenhH = false; // Hủy chuỗi hẹn giờ lệnh H nếu đang chạy
          daBatR7_SauH = false;
          daBatP10_P3_SauH = false;
          boGoDangChay = false; // Dừng chuỗi bộ gõ nếu đang chạy
          daNhanLenhK = false;  // Hủy chuỗi hẹn giờ lệnh K nếu đang chạy
          daXuLy30sSauK = false;
        }

        // 2. KÊNH R3 (P16): Xác nhận khi giữ liên tục đủ 6 mẫu (120ms)
        if (demTichCucP16 >= 6) {
          rfP16_DangKichHoat = true;
          if (millis() - thoiGianChopRF >= TOC_DO_CHOP_TAT_RF_MS) {
            thoiGianChopRF = millis();
            trangThaiChopRF = !trangThaiChopRF;
            if (trangThaiChopRF) {
              batRelayChopRF();
            } else {
              tatRelayChopRF();
            }
          }
        } else {
          if (rfP16_DangKichHoat) {
            rfP16_DangKichHoat = false;
            tatRelayChopRF(); // Dừng chớp và tắt khi nhả nút
          }
        }

        // 3. KÊNH R4 (P17): Xác nhận khi giữ liên tục đủ 6 mẫu (120ms)
        if (demTichCucP17 >= 6) {
          if (!rfP17_DangKichHoat) {
            rfP17_DangKichHoat = true;
            g_dangCho5sKhoiPhucR4 = false; // Hủy đếm 5s nếu bấm tiếp

            // Lưu snapshot vị trí các đèn trước khi bấm nếu chưa lưu
            if (!g_daLuuSnapshotR4) {
              g_snapshotDenPcfR4 = g_pcfData & MASK_TAT_CA_DEN;
              g_snapshotA7_R4 = (digitalRead(PIN_OUTPUT_A7) == MUC_A7_ON);
              g_daLuuSnapshotR4 = true;
            }

            // TẮT TẤT CẢ ĐÈN (P3, P4, P5, P7, P10, P12, PA7) - LOA P0, P1 VẪN GIỮ NGUYÊN!
            g_pcfData &= ~MASK_TAT_CA_DEN;
            tatChanA7();

            // BẬT Relay 13 (P13: ĐÈN ĐỎ)
            g_pcfData |= MASK_RELAY_DEN_DO;
            writePCF8575(g_pcfData);
          }
        } else {
          if (rfP17_DangKichHoat) {
            rfP17_DangKichHoat = false;
            // NHẢ NÚT -> TẮT RELAY 13 NGAY LẬP TỨC (Momentary)
            g_pcfData &= ~MASK_RELAY_DEN_DO;
            writePCF8575(g_pcfData);

            // BẮT ĐẦU ĐẾM THỜI GIAN 5 GIÂY ĐỂ KHÔI PHỤC VỊ TRÍ ĐÈN
            g_timerNhaNutR4 = millis();
            g_dangCho5sKhoiPhucR4 = true;
          }
        }
      }
    }

    // 👉 XỬ LÝ KHÔI PHỤC ĐÈN VỀ ĐÚNG VỊ TRÍ SAU 5 GIÂY KHÔNG NHẤN R4 NỮA
    if (g_dangCho5sKhoiPhucR4 && (millis() - g_timerNhaNutR4 >= 5000)) {
      g_dangCho5sKhoiPhucR4 = false;
      if (g_daLuuSnapshotR4) {
        g_pcfData |= g_snapshotDenPcfR4; // Khôi phục lại các đèn về đúng vị trí lúc bấm
        writePCF8575(g_pcfData);
        if (g_snapshotA7_R4) {
          batChanA7();
        }
        g_daLuuSnapshotR4 = false;
      }
    }

    // 👉 Duy trì lệnh ghi ra PCF8575 mỗi 200ms khi đường truyền rảnh rỗi để chống trôi trạng thái
    if (millis() - lanRefreshCuoi >= 200) {
      lanRefreshCuoi = millis();
      pcf8575_update();
    }
  } else {
    // Đang trong cửa sổ khóa chống nhiễu cuộn dây: Reset sạch các biến đếm RF
    demTichCucP15 = 0;
    demTichCucP16 = 0;
    demTichCucP17 = 0;
    if (rfP16_DangKichHoat) {
      rfP16_DangKichHoat = false;
      tatRelayChopRF();
    }
    if (rfP17_DangKichHoat) {
      rfP17_DangKichHoat = false;
      g_pcfData &= ~MASK_RELAY_DEN_DO;
      writePCF8575(g_pcfData);
    }
  }

  // 👉 Đọc lệnh gửi từ bo STM32_SOUND qua chân PA3 RX
  char lenhTuSound = docLenhSound();

  // ==========================================================================
  // 👉 1. XỬ LÝ LỆNH KHÔI PHỤC 'R' TỪ BO STM32_SOUND
  // ==========================================================================
  if (lenhTuSound == 'R' || lenhTuSound == 'r') {
    DuLieuCheckpointRelay cp = docCheckpointFlashRelay();
    if (cp.magic == FLASH_MAGIC_KEY && cp.stage > 0) {
      thucHienKhoiPhucRelay(cp.stage, cp.p6Opened);
    }
  }

  // ==========================================================================
  // 👉 2. NÚT BẤM KHÔI PHỤC KHI CÚP ĐIỆN DỰ PHÒNG TRÊN BO RELAY (CHÂN PA4):
  // - Khi nhấn nút nối mass GND ở PA4:
  //   1. Tự động đọc lại mốc đã lưu trong Flash
  //   2. Bật lại đúng rơ-le và gửi lệnh 'R' sang bo Sound cùng khôi phục
  // ==========================================================================
  static bool daNhanPA4 = false;
  if (!daNhanPA4) {
    if (kiemTraNutNhanPA4()) {
      daNhanPA4 = true;
      DuLieuCheckpointRelay cp = docCheckpointFlashRelay();
      if (cp.magic == FLASH_MAGIC_KEY && cp.stage > 0) {
        guiLenhSound('R'); // Gửi sang bo Sound để cùng khôi phục
        delay(20);
        thucHienKhoiPhucRelay(cp.stage, cp.p6Opened);
      }
    }
  } else {
    if (digitalRead(PIN_BTN_KHOI_PHUC) != LOW) {
      daNhanPA4 = false;
    }
  }

  // ==========================================================================
  // 👉 LUỒNG XỬ LÝ RIÊNG BIỆT CHO CHÂN A6 (PA6):
  // - LÚC ĐẦU TIÊN: Luôn luôn KHÔNG có kích tín hiệu, Relay P6 TẮT tuyệt đối.
  // - CHỈ ĐẾN KHI VÀ CHỈ KHI chân PA6 (A6 - kéo lên nội bộ Pull-Up) nhận tín hiệu
  //   kích âm GND (Mức LOW) sau thời gian ổn định 3s ban đầu:
  //   1. TẮT P5 (LED đồ chơi)
  //   2. BẬT P6 (Cửa từ 1 mở chốt) -> KHÔNG BAO GIỜ TẮT LẠI CHO ĐẾN KHI NHẤN NÚT RESET!
  //   3. Phản hồi đèn PC13 sáng
  //   4. GỬI LỆNH 'G' CHO STM32_SOUND
  // - Sau khi đã kích hoạt: CHỐT VĨNH VIỄN, không có bất kỳ lệnh hay sự kiện nào
  //   được phép tắt P6 hoặc reset lại cờ này (CHỈ TẮT KHI NHẤN NÚT RESET CỨNG BO STM32)!
  // ==========================================================================
  // Bước 1: Sau 3s khởi động, xác nhận chân PA6 đang ở mức HIGH (trạng thái nghỉ an toàn)
  if (!pa6DaSanSang && (millis() >= THOI_GIAN_KHOI_DONG_ON_DINH_MS)) {
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET || digitalRead(PIN_BUTTON_OFF_R5) == LOW) {
      // Đang ở mức LOW -> chưa sẵn sàng (có thể đang giữ nút hoặc cảm biến đang kích từ trước)
    } else {
      pa6DaSanSang = true; // PA6 đã vào trạng thái nghỉ chuẩn (HIGH), sẵn sàng đón tín hiệu kích âm
    }
  }

  // Bước 2: Quét phát hiện tín hiệu nhấn nút / cảm biến kích âm GND (Mức LOW) qua kiemTraNutNhanPA6()
  if (pa6DaSanSang && !daXuLyTinHieuA6 && kiemTraNutNhanPA6()) {
    daXuLyTinHieuA6 = true;           // ⚡ CHỐT CHẶN VĨNH VIỄN: Chỉ nhận đúng 1 lần duy nhất!
    g_p6DaBat = true;                 // Đánh dấu Cửa từ 1 (P6) đã mở chốt
    batChanPCF(RELAY_CUA_TU_1_P6);    // BẬT P6 (Mở chốt Cửa từ 1)
    batChanPCF(3);                    // BẬT đèn P3 (Đèn phòng / quan sát)
    batChanPCF(RELAY_LED_DO_CHOI_P5); // BẬT / Duy trì đèn đồ chơi P5
    pc13_relay_set_state(LED_SANG_3S_ROI_TAT);
    guiLenhSound('G');                // GỬI LỆNH 'G' CHO STM32_SOUND
    ghiCheckpointFlashRelay(4, 1);    // 💾 Lưu checkpoint Mốc 4 (Cửa từ 1 đã mở, đèn bật)
  }

  // Nhận lệnh '0' (dừng khẩn cấp): Tắt các relay kịch bản, quay lại chờ lệnh S (P6 nếu đã bật vẫn giữ nguyên)
  if (lenhTuSound == '0') {
    tatTatCaRelay();
    resetCheckpointFlashRelay(); // 💾 Xóa sạch checkpoint về 0
    cheDoV_DangBat = false;
    pc13_relay_set_state(LED_CHOP_CHAM);
    trangThai = TRANG_THAI_CHO_LENH_S;
    daNhanLenhH = false;
    daBatR7_SauH = false;
    daBatP10_P3_SauH = false;
    boGoDangChay = false;
    daNhanLenhK = false;
    daTatP13_Sau10sK = false;
    daXuLy30sSauK = false;
    daXuLyTinHieuA5 = false; // Reset cờ cảm biến A5
    resetBoLocA5();          // Reset bộ lọc cảm biến A5 cho chu kỳ mới
    daGuiJ_BatP2 = false;
    daTatP2_BatP0_P1 = false;
    g_daLuuSnapshotR4 = false;
    g_dangCho5sKhoiPhucR4 = false;
    rfP17_DangKichHoat = false;
    while (SoundSerial.available() > 0) {
      SoundSerial.read();
    }
  }

  uint32_t now = millis();

  // 👉 KHI NHẬN ĐƯỢC LỆNH 'K' TỪ BO SOUND (BƯỚC 22-23 THEO FLOWCHART):
  // ⚡ CHỈ NHẬN VÀ XỬ LÝ KHI ĐÃ QUA 50S (P2 ĐÃ TẮT) VÀ CHƯA TỪNG NHẬN K:
  // 1. Chuyển pha nguyên khối (Atomic): BẬT P11 (Búp bê rơi), BẬT P13 (Đèn đỏ), DUY TRÌ BẬT P0 và P1 (Loa)
  // 2. Ghi kép (Double-write) chốt cứng trạng thái + Khóa chống xung nhiễu cuộn dây 2000ms
  // 3. Bắt đầu đếm thời gian: Bật P13 10s, sau đó tắt đèn 20s, sau 30s bật lại P3
  if (!daNhanLenhK && (lenhTuSound == 'K' || lenhTuSound == 'k') && (trangThai == TRANG_THAI_CHO_LENH_K || daTatP2_BatP0_P1)) {
    chuyenPha_NhanLenhK_BatP11_P13_P0_P1(); // 🛡️ Ghi đồng thời tất cả các chân trong 1 lần duy nhất, triệt tiêu 100% nhảy relay
    thoiGianNhanK = now;
    daNhanLenhK = true;
    daTatP13_Sau10sK = false;
    daXuLy30sSauK = false;
    trangThai = TRANG_THAI_DA_NHAN_K;
    pc13_relay_set_state(LED_SANG_3S_ROI_TAT);
    ghiCheckpointFlashRelay(6, 1); // 💾 Lưu checkpoint Mốc 6 (Búp bê rơi)
    while (SoundSerial.available() > 0) {
      SoundSerial.read(); // Xóa sạch bộ đệm UART tránh trùng lặp lệnh
    }
  }

  // ==========================================================================
  // 👉 BẤT KỲ KHI NÀO NHẬN ĐƯỢC LỆNH 'U' TỪ STM32_SOUND:
  // Vừa hết bài "Đừng bỏ em 1 mình" (chân A3 trên bo Sound vừa kết thúc)
  // -> Chuyển ngay lập tức từ loa P2 sang 2 loa P0 và P1!
  // ==========================================================================
  if ((lenhTuSound == 'U' || lenhTuSound == 'u') && (trangThai == TRANG_THAI_CHO_50S_P2 || (daGuiJ_BatP2 && !daTatP2_BatP0_P1))) {
    chuyenPha_TatP2_BatP0P1(); // 🛡️ TẮT P2, BẬT LẠI P0 VÀ P1 NGUYÊN KHỐI
    trangThai = TRANG_THAI_CHO_LENH_K;
    daTatP2_BatP0_P1 = true;
    while (SoundSerial.available() > 0) {
      SoundSerial.read();
    }
  }

  // 👉 BẤT KỲ KHI NÀO NHẬN ĐƯỢC LỆNH 'L' TỪ STM32_SOUND:
  // 1. Chân A7 của STM32 xuất tín hiệu ON
  // 2. Kích chân THÊM P3 sáng
  if (lenhTuSound == 'L' || lenhTuSound == 'l') {
    batChanA7();   // 👉 Chân A7 của STM32 xuất tín hiệu ON (Mức HIGH 3.3V)
    batChanPCF(3); // 👉 Kích chân THÊM P3 sáng
    pc13_relay_set_state(LED_SANG_3S_ROI_TAT);
  }

  // ==========================================================================
  // 👉 BẤT KỲ KHI NÀO NHẬN ĐƯỢC LỆNH 'T' (TẮT SẠCH ĐÈN) TỪ BO JUMPSCARE / SOUND:
  // - Những gì đang bật (Loa P0, P1, P2, Cửa từ P6, Búp bê P11, Bộ gõ P14...) GIỮ NGUYÊN!
  // - CÒN ĐÈN THÌ TẮT SẠCH TOÀN BỘ:
  //   + P3 (Đèn P3)
  //   + P4 (Đèn TV)
  //   + P5 (Đèn đồ chơi)
  //   + P7 (Đèn chiếu tranh)
  //   + P10 (Đèn rồng phượng)
  //   + P12 (Đèn cuốn sổ)
  //   + P13 (Đèn đỏ)
  //   + PA7 (Tín hiệu đèn A7)
  // ==========================================================================
  if (lenhTuSound == 'T' || lenhTuSound == 't') {
    // 🛑 TẮT TẤT CẢ CÁC ĐÈN PHÒNG (CHỪA LẠI LOA P0, P1, P2):
    tatChanPCF(3);                         // Tắt đèn P3
    tatChanPCF(RELAY_DEN_TV_P4);           // Tắt đèn TV P4
    tatChanPCF(RELAY_LED_DO_CHOI_P5);      // Tắt đèn đồ chơi P5
    tatChanPCF(RELAY_DEN_BUC_TRANH_P7);    // Tắt đèn bức tranh P7
    tatChanPCF(RELAY_DEN_RONG_PHUONG_P10); // Tắt đèn rồng phượng P10
    tatChanPCF(RELAY_LED_CUON_SO_P12);     // Tắt đèn cuốn sổ P12
    tatChanA7();                           // Tắt tín hiệu đèn chân PA7
    // 👉 CHÚ Ý: CHỪA LẠI LOA P0, P1, P2 (GIỮ NGUYÊN BẬT)!
    // 👉 ĐÈN ĐỎ RELAY 13 (P13) HOÀN TOÀN CÓ THỂ SÁNG BÌNH THƯỜNG KHI BẤM NÚT R4 (P17)!
    pc13_relay_set_state(LED_SANG_3S_ROI_TAT); // Báo hiệu LED PC13
  }

  // ==========================================================================
  // 👉 BẤT KỲ KHI NÀO NHẬN ĐƯỢC LỆNH 'V' (CHIẾN THẮNG / WIN) TỪ STM32_SOUND:
  // - LẦN NHẬN 1: BẬT Relay P12, P0, P1; giữ nguyên P6 (Cửa từ 1) nếu đã bật; TẮT toàn bộ các relay khác & chân A7.
  // - LẦN NHẬN 2: TẮT TOÀN BỘ HỆ THỐNG VÀ ĐƯA VỀ TRẠNG THÁI BAN ĐẦU (chờ lệnh S mới).
  // ==========================================================================
  if (lenhTuSound == 'V' || lenhTuSound == 'v') {
    cheDoV_DangBat = !cheDoV_DangBat; // Đảo trạng thái toggle

    if (cheDoV_DangBat) {
      // 🏆 LẦN NHẬN 1: KÍCH HOẠT CHẾ ĐỘ CHIẾN THẮNG TRÊN BO RELAY
      // 1. Dừng ngay các tiến trình kịch bản đang chạy dở dang
      daNhanLenhH = false;
      daBatR7_SauH = false;
      daBatP10_P3_SauH = false;
      boGoDangChay = false;
      daKetThuc30sBoGo = true;
      daTatDenSauBoGo = true;
      daGuiJ_BatP2 = false;
      daTatP2_BatP0_P1 = false;
      daNhanLenhK = false;
      daTatP13_Sau10sK = true;
      daXuLy30sSauK = true;

      // 2. KÍCH RELAY P12, P0, P1 & GIỮ NGUYÊN P6 (NẾU ĐÃ BẬT); TẮT CÁC RELAY KHÁC & CHÂN A7
      batCheDoChienThangV();

      // 3. Đèn PC13 sáng 3 giây xác nhận
      pc13_relay_set_state(LED_SANG_3S_ROI_TAT);
      ghiCheckpointFlashRelay(7, 1); // 💾 Lưu checkpoint Mốc 7 (Chiến Thắng)
    } else {
      // 🛑 LẦN NHẬN 2: TẮT HỆ THỐNG VÀ ĐƯA TOÀN BỘ VỀ BAN ĐẦU
      tatTatCaRelay(); // Tắt toàn bộ relay (P6 nếu đã bật vẫn được giữ nguyên theo quy tắc an toàn)
      tatChanA7();
      resetCheckpointFlashRelay(); // 💾 Xóa sạch checkpoint về 0 khi tắt hệ thống

      // Đưa máy trạng thái và toàn bộ các cờ về ban đầu (chờ lệnh S mới)
      trangThai = TRANG_THAI_CHO_LENH_S;
      daNhanLenhH = false;
      daBatR7_SauH = false;
      daBatP10_P3_SauH = false;
      boGoDangChay = false;
      daKetThuc30sBoGo = false;
      daTatDenSauBoGo = false;
      daGuiLenhJ = false;
      daNhanLenhK = false;
      daTatP13_Sau10sK = false;
      daXuLy30sSauK = false;
      daXuLyTinHieuA5 = false;
      resetBoLocA5();
      daGuiJ_BatP2 = false;
      daTatP2_BatP0_P1 = false;

      pc13_relay_set_state(LED_CHOP_CHAM); // Trạng thái chờ: LED nháy chậm
    }
  }

  // 🛡️ NẾU ĐANG Ở CHẾ ĐỘ CHIẾN THẮNG 'V': DUY TRÌ TRẠNG THÁI P12, P0, P1 (VÀ P6 NẾU ĐÃ BẬT),
  // BỎ QUA CÁC TIẾN TRÌNH KỊCH BẢN TỰ ĐỘNG KHÁC CHO ĐẾN KHI NHẬN LỆNH TIẾP THEO
  if (cheDoV_DangBat) {
    uint16_t maskV = (1 << 0) | (1 << 1) | (1 << 10);
    if (g_p6DaBat) {
      maskV |= (1 << 6);
    }
    if ((g_pcfData & 0x1FFF) != maskV) {
      g_pcfData = maskV;
      writePCF8575(g_pcfData);
      tatChanA7();
    }
    return;
  }

  switch (trangThai) {
  // --------------------------------------------------------------------------
  // GIAI ĐOẠN 1: CHỜ LỆNH S TỪ STM32_SOUND
  // Đèn PC13 nhấp nháy chậm (1000ms), toàn bộ relay tắt
  // --------------------------------------------------------------------------
  case TRANG_THAI_CHO_LENH_S:
    if (lenhTuSound == 'S' || lenhTuSound == 's') {
      thoiGianNhanS = now;

      // 👉 KÍCH RELAY: Bật chân P0 và P3 lên (các P khác tắt)
      tatTatCaRelay();          // Tắt các rơ-le khác
      batChanPCF(RELAY_LOA_P0); // BẬT Relay chân P0 (Loa tổng)
      batChanPCF(3);            // BẬT Relay chân P3 (RELAY_LED4)

      // 👉 ĐÈN PC13: Sáng liên tục 3 giây rồi tự tắt
      pc13_relay_set_state(LED_SANG_3S_ROI_TAT);

      trangThai = TRANG_THAI_KICH_P0_P3;
      ghiCheckpointFlashRelay(1, g_p6DaBat ? 1 : 0); // 💾 Lưu checkpoint Mốc 1
    }
    break;

  // ==========================================================================
  // GIAI ĐOẠN 2: KHI ĐÃ CÓ LỆNH S (P0 VÀ P3 ĐANG BẬT) -> CHỜ ĐỦ 10 GIÂY
  // Sau 10s: GỬI LỆNH D, BẬT P1 (LOA TRÊN GÁC), CÒN LẠI CÁC P KHÁC TẮT
  // ==========================================================================
  case TRANG_THAI_KICH_P0_P3:
    // 👉 Đủ 10 giây (THOI_GIAN_CHUYEN_PHA_10S_MS) kể từ khi nhận lệnh S:
    if (now - thoiGianNhanS >= THOI_GIAN_CHUYEN_PHA_10S_MS) {
      // 🎯 [CHUYỂN TRẠNG THÁI RELAY]:
      // Chân P1 BẬT, còn lại các P khác TẮT (bật loa trên gác)
      tatChanPCF(RELAY_LOA_P0); // 👉 Tắt P0
      tatChanPCF(3);            // 👉 Tắt P3
      batChanPCF(RELAY_LOA_P1); // 👉 BẬT P1 (Loa trên gác)

      // 👉 ĐÈN PC13: Quay lại nhấp nháy chậm
      pc13_relay_set_state(LED_CHOP_CHAM);

      // 👉 Gửi lệnh 'D' sang bo STM32_SOUND để kích hoạt pha âm thanh tiếp theo
      guiLenhSound('D');

      // Chuyển máy trạng thái sang Giai đoạn 3 (P1 đang bật)
      trangThai = TRANG_THAI_CHUYEN_P1;
      ghiCheckpointFlashRelay(2, g_p6DaBat ? 1 : 0); // 💾 Lưu checkpoint Mốc 2
    }
    break;

  // --------------------------------------------------------------------------
  // GIAI ĐOẠN 3: SAU 10S (RELAY P1 ĐANG BẬT)
  // Chờ lệnh F hoặc lệnh S mới
  // --------------------------------------------------------------------------
  case TRANG_THAI_CHUYEN_P1:
    // 👉 KHI NHẬN ĐƯỢC LỆNH 'F': BẬT P5 (ĐÈN ĐỒ CHƠI), TẮT ĐÈN P3, VÀ BẬT CẢ P0 VÀ P1
    if (lenhTuSound == 'F' || lenhTuSound == 'f') {
      batChanPCF(RELAY_LED_DO_CHOI_P5); // BẬT P5 (LED đồ chơi)
      tatChanPCF(3);                    // TẮT đèn P3
      batChanPCF(RELAY_LOA_P0);         // BẬT P0 (Loa tổng)
      batChanPCF(RELAY_LOA_P1);         // BẬT P1 (Loa trên gác)
      ghiCheckpointFlashRelay(3, g_p6DaBat ? 1 : 0); // 💾 Lưu checkpoint Mốc 3
    }

    // 👉 KHI NHẬN ĐƯỢC LỆNH 'H':
    // P6 đã được bật ngay khi A6 có tín hiệu và không bao giờ tắt lại.
    // Kích hoạt đếm thời gian: sau 5s bật Relay 7, sau đó 45s bật P10 & P3
    if (lenhTuSound == 'H' || lenhTuSound == 'h') {
      thoiGianNhanH = now;
      daNhanLenhH = true;
      daBatR7_SauH = false;
      daBatP10_P3_SauH = false;
    }

    // Nếu nhận thêm lệnh 'S' mới thì khởi động lại chu kỳ mới
    if (lenhTuSound == 'S' || lenhTuSound == 's') {
      tatTatCaRelay(); // Tắt các rơ-le (P6 nếu đã bật vẫn được giữ nguyên)
      daNhanLenhH = false;
      daBatR7_SauH = false;
      daBatP10_P3_SauH = false;
      boGoDangChay = false;
      daNhanLenhK = false;
      daTatP13_Sau10sK = false;
      daXuLy30sSauK = false;
      daXuLyTinHieuA5 = false; // Reset cờ cảm biến A5 cho lượt chơi mới
      resetBoLocA5();          // Reset bộ lọc cảm biến A5
      daGuiJ_BatP2 = false;
      daTatP2_BatP0_P1 = false;
      thoiGianNhanS = now;
      batChanPCF(RELAY_LOA_P0); // Bật P0
      batChanPCF(3);            // Bật P3
      pc13_relay_set_state(LED_SANG_3S_ROI_TAT);
      trangThai = TRANG_THAI_KICH_P0_P3;
      ghiCheckpointFlashRelay(1, g_p6DaBat ? 1 : 0); // 💾 Lưu checkpoint Mốc 1
    }
    break;

  case TRANG_THAI_CHO_50S_P2:
  case TRANG_THAI_CHO_LENH_K:
  case TRANG_THAI_DA_NHAN_K:
    // Nếu nhận thêm lệnh 'S' mới thì khởi động lại chu kỳ mới
    if (lenhTuSound == 'S' || lenhTuSound == 's') {
      tatTatCaRelay(); // Tắt các rơ-le (P6 nếu đã bật vẫn được giữ nguyên)
      daNhanLenhH = false;
      daBatR7_SauH = false;
      daBatP10_P3_SauH = false;
      boGoDangChay = false;
      daNhanLenhK = false;
      daTatP13_Sau10sK = false;
      daXuLy30sSauK = false;
      daXuLyTinHieuA5 = false; // Reset cờ cảm biến A5 cho lượt chơi mới
      resetBoLocA5();          // Reset bộ lọc cảm biến A5
      daGuiJ_BatP2 = false;
      daTatP2_BatP0_P1 = false;
      thoiGianNhanS = now;
      batChanPCF(RELAY_LOA_P0); // Bật P0
      batChanPCF(3);            // Bật P3
      pc13_relay_set_state(LED_SANG_3S_ROI_TAT);
      trangThai = TRANG_THAI_KICH_P0_P3;
      ghiCheckpointFlashRelay(1, g_p6DaBat ? 1 : 0); // 💾 Lưu checkpoint Mốc 1
      while (SoundSerial.available() > 0) {
        SoundSerial.read();
      }
    }
    break;
  }

  // ==========================================================================
  // XỬ LÝ CHU TRÌNH TỰ ĐỘNG SAU KHI NHẬN LỆNH 'H' TỪ STM32_SOUND:
  // - Bước 1: Khi nhận H đã BẬT ngay Relay 6 (P6: Cửa từ 1)
  // - Bước 2: Sau 5s -> BẬT Relay 7 (P7: Đèn chiếu bức tranh)
  // - Bước 3: Sau đó 45s (5s + 45s = 50s) -> BẬT Relay P10 và P3
  // ==========================================================================
  if (daNhanLenhH) {
    // Bước 2: Sau 5s kể từ khi nhận H
    if (!daBatR7_SauH && (now - thoiGianNhanH >= THOI_GIAN_H_BAT_R7_MS)) {
      batChanPCF(
          RELAY_DEN_BUC_TRANH_P7); // BẬT Relay 7 (P7: Đèn chiếu bức tranh)
      daBatR7_SauH = true;
    }

    // Bước 3: Sau đó 45s (tức 5s + 45s = 50s kể từ khi nhận H)
    if (daBatR7_SauH && !daBatP10_P3_SauH &&
        (now - thoiGianNhanH >=
         THOI_GIAN_H_BAT_R7_MS + THOI_GIAN_SAU_R7_BAT_P10_P3_MS)) {
      batChanPCF(RELAY_DEN_RONG_PHUONG_P10); // BẬT Relay P10 (ĐÈN RỒNG PHƯỢNG)
      batChanPCF(3);                         // BẬT Relay P3
      daBatP10_P3_SauH = true;
    }
  }

  // ==========================================================================
  // XỬ LÝ CHU TRÌNH TỰ ĐỘNG SAU KHI NHẬN LỆNH 'K' TỪ STM32_SOUND:
  // - Khi nhận K: Đã BẬT chân P11 ON (Búp bê rơi), P13 ON (Đèn đỏ), giữ P0 và P1
  // - Bước 1: Sau 10s -> TẮT đèn P13 trong vòng 20s
  // - Bước 2: Sau 20s tắt (tổng 10s + 20s = 30s) -> BẬT chân P3 trở lại
  // ==========================================================================
  if (daNhanLenhK) {
    // Bước 1: Sau 10s bật đèn -> TẮT đèn P13 trong vòng 20s
    if (!daTatP13_Sau10sK && (now - thoiGianNhanK >= THOI_GIAN_K_BAT_P13_MS)) {
      chuyenPha_Sau10sK_TatP13(); // 🛡️ Tắt P13, giữ P11, P0, P1, P6
      daTatP13_Sau10sK = true;
    }

    // Bước 2: Sau 20s tắt đèn (tổng 30s kể từ khi nhận K) -> BẬT chân P3 trở lại
    if (daTatP13_Sau10sK && !daXuLy30sSauK &&
        (now - thoiGianNhanK >= THOI_GIAN_K_TONG_MS)) {
      chuyenPha_Sau30sK_TatP13_BatP3(); // 🛡️ Chuyển pha nguyên khối Atomic + Khóa chống nhiễu 1000ms
      daXuLy30sSauK = true;
    }
  }

  // ==========================================================================
  // 👉 CẢM BIẾN NPN 5V CHÂN PA5 (A5): KHI CẢM BIẾN BỊ MẤT TÍN HIỆU (NHẤC VẬT RA)
  // ⚡ CHỈ NHẬN ĐÚNG 1 LẦN DUY NHẤT (nếu lỡ có nhận nữa thì cũng không có tác dụng)
  // PHÁT HIỆN VÀ KÍCH HOẠT CHU TRÌNH BỘ GÕ P14 (30S) -> SAU ĐÓ 10S GỬI LỆNH 'J'
  // ==========================================================================
  if (!daXuLyTinHieuA5 && !boGoDangChay && !daKetThuc30sBoGo && !daTatDenSauBoGo && kiemTraNutNhanPA5()) {
    daXuLyTinHieuA5 = true; // Chốt chỉ nhận đúng 1 lần duy nhất!
    boGoDangChay = true;
    thoiGianBatDauBoGo = now;
    thoiGianChuyenTrangThaiBoGo = now;
    chiSoMauHienTai = random(0, SO_MAU_BO_GO);
    trangThaiBoGoHienTai = true; // Bắt đầu bằng BẬT P14
    thoiGianGiuTrangThaiHienTai = CAC_MAU_BO_GO[chiSoMauHienTai].thoiGianBatMs;
    batChanPCF(RELAY_BO_GO_P14); // BẬT Relay 14 (P14: Bộ gõ)
    daKetThuc30sBoGo = false;
    daTatDenSauBoGo = false;
    daGuiLenhJ = false;
  }

  // ==========================================================================
  // XỬ LÝ TIẾN TRÌNH BỘ GÕ P14 & TẮT TẤT CẢ ĐÈN SAU ĐÓ 10S:
  // - Trong 30s: Nhấp nháy ngẫu nhiên (1s BẬT 1.5s TẮT / 2s BẬT 2s TẮT / 3s BẬT
  // 3s TẮT)
  // - Sau 30s: Tắt bộ gõ P14
  // - Sau đó 10s (tổng 40s): Tắt tất cả đèn (P0..P2 loa giữ BẬT) & Gửi lệnh 'J'
  // ==========================================================================
  if (boGoDangChay) {
    uint32_t thoiGianTroiQua = now - thoiGianBatDauBoGo;

    // Giai đoạn 1: Trong vòng 30s đầu -> nhấp nháy ngẫu nhiên
    if (thoiGianTroiQua < THOI_GIAN_BO_GO_MS) {
      if (now - thoiGianChuyenTrangThaiBoGo >= thoiGianGiuTrangThaiHienTai) {
        thoiGianChuyenTrangThaiBoGo = now;
        trangThaiBoGoHienTai = !trangThaiBoGoHienTai;
        if (trangThaiBoGoHienTai) {
          chiSoMauHienTai = random(0, SO_MAU_BO_GO);
          thoiGianGiuTrangThaiHienTai =
              CAC_MAU_BO_GO[chiSoMauHienTai].thoiGianBatMs;
          batChanPCF(RELAY_BO_GO_P14); // BẬT
        } else {
          thoiGianGiuTrangThaiHienTai =
              CAC_MAU_BO_GO[chiSoMauHienTai].thoiGianTatMs;
          tatChanPCF(RELAY_BO_GO_P14); // TẮT
        }
      }
    }
    // Giai đoạn 2: Đủ 30s -> Tắt dứt điểm bộ gõ P14
    else if (!daKetThuc30sBoGo) {
      tatChanPCF(RELAY_BO_GO_P14);
      daKetThuc30sBoGo = true;
    }

    // Giai đoạn 3: Sau đó 10s (30s + 10s = 40s từ khi mất tín hiệu A5):
    // 👉 TẮT TẤT CẢ RELAY CỦA CÁC CHÂN P0 ĐẾN P14, CHỈ RIÊNG DUY NHẤT CHÂN P2 BẬT & GỬI LỆNH 'J'
    // 👉 BẮT ĐẦU ĐẾM THỜI GIAN CHỜ 50S ĐỂ TẮT P2, BẬT LẠI P0 VÀ P1
    if (thoiGianTroiQua >=
        THOI_GIAN_BO_GO_MS + THOI_GIAN_SAU_BO_GO_TAT_DEN_MS) {
      if (!daTatDenSauBoGo) {
        tatTatCaRelay();          // 1. Tắt tất cả các relay từ P0 đến P14
        batChanPCF(RELAY_LOA_P2); // 2. BẬT DUY NHẤT CHÂN P2 (Loa hiệu ứng P2)
        if (!daGuiLenhJ) {
          daGuiLenhJ = true;
          guiLenhSound('J');      // 3. Gửi lệnh 'J' cho bo STM32_SOUND đúng 1 lần duy nhất!
        }
        thoiGianGuiJ = now;       // 4. Bắt đầu đếm thời gian 50 giây chờ
        daGuiJ_BatP2 = true;
        daTatP2_BatP0_P1 = false;
        daTatDenSauBoGo = true;
        daGuiLenhJ = true;
        ghiCheckpointFlashRelay(5, 1); // 💾 Lưu checkpoint Mốc 5 (Sau bộ gõ, bật P2)
        daNhanLenhH = false;      // Ngăn chặn kịch bản hẹn giờ H bật lại đèn
        daBatR7_SauH = true;      // Khóa hẳn chuỗi H không bao giờ kích lại
        daBatP10_P3_SauH = true;  // Khóa hẳn chuỗi H không bao giờ kích lại
        boGoDangChay = false;     // Hoàn tất chu kỳ bộ gõ
        trangThai = TRANG_THAI_CHO_50S_P2; // Chuyển trạng thái sang chờ 50s P2
        while (SoundSerial.available() > 0) {
          SoundSerial.read();
        }
      }
    }
  }

  // ==========================================================================
  // XỬ LÝ CHỜ 50S SAU KHI GỬI LỆNH 'J' & BẬT P2 (BƯỚC 20 -> 21 THEO FLOWCHART):
  // - Đúng 50 giây sau:
  //   1. TẮT chân P2
  //   2. BẬT LẠI THÊM P0 VÀ P1 (Loa tổng P0 & Loa trên gác P1)
  //   3. Chuyển sang máy trạng thái chờ nhận lệnh K từ bo Sound
  // ==========================================================================
  if (daGuiJ_BatP2 && !daTatP2_BatP0_P1) {
    if (now - thoiGianGuiJ >= THOI_GIAN_SAU_J_TAT_P2_BAT_P0_P1_MS) {
      // 1. Chuyển pha nguyên khối (Atomic): TẮT P2, BẬT LẠI P0 VÀ P1 đồng thời trong 1 lần ghi I2C duy nhất
      // Loại trừ 100% hiện tượng chập chờn / kích chéo rơ-le khác!
      chuyenPha_TatP2_BatP0P1();

      // 2. Chuyển sang máy trạng thái chờ nhận lệnh K từ bo Sound
      trangThai = TRANG_THAI_CHO_LENH_K;
      daTatP2_BatP0_P1 = true;

      // 3. Xóa sạch bộ đệm UART để chuẩn bị đón nhận lệnh 'K' từ STM32_SOUND chuẩn xác 100%
      while (SoundSerial.available() > 0) {
        SoundSerial.read();
      }
    }
  }
}
