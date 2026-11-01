#include <Arduino.h>

// ==============================================================================
// CHƯƠNG TRÌNH TEST KIỂM TRA STM32 SỐNG / CHẾT VÀ TEST CÁC CHÂN GPIO
// Bo mạch: STM32F103C8T6 (Blue Pill)
// ==============================================================================
// NGUYÊN LÝ HOẠT ĐỘNG:
// 1. KHỞI ĐỘNG 3 GIÂY ĐẦU:
//    - Đèn PC13 (LED onboard) nháy CHẬM (500ms BẬT, 500ms TẮT x 3 lần = 3s).
//    - Báo hiệu: Vi điều khiển đã nạp code thành công và bootloader/core chạy tốt.
//
// 2. SAU 3 GIÂY (CHẾ ĐỘ SẴN SÀNG TEST):
//    - Đèn PC13 chuyển sang nháy NHANH liên tục (100ms BẬT, 100ms TẮT).
//    - Báo hiệu: STM32 sống hoàn hảo, đang ở trạng thái chờ test GPIO.
//
// 3. TEST CÁC CHÂN GPIO (KÉO XUỐNG GND):
//    - Tất cả các chân GPIO được cấu hình INPUT_PULLUP (kéo lên 3.3V nội bộ).
//    - Dùng 1 sợi dây nối vào chân GND, chạm vào bất kỳ chân GPIO nào (hoặc bấm nút nối GND):
//      -> Nếu chân đó HOẠT ĐỘNG BÌNH THƯỜNG: Đèn PC13 SẼ TẮT trong suốt thời gian đang chạm/kéo GND!
//      -> Đồng thời gửi thông báo qua Serial (115200): "[GPIO OK] Chan ... duoc keo xuong GND".
//    - Khi nhả chân ra khỏi GND: Đèn PC13 lập tức quay lại nháy NHANH như cũ.
//
// LƯU Ý BẢO VỆ:
// - Giữ nguyên PA13, PA14 làm chân nạp ST-Link (SWDIO, SWCLK), không can thiệp.
// - Giải phóng PB3, PB4, PA15 khỏi JTAG để test GPIO an toàn.
// ==============================================================================

#define LED_PC13     PC13
#define LED_BAT      LOW   // LED PC13 trên Blue Pill tích cực mức LOW (LOW = Sáng)
#define LED_TAT      HIGH  // HIGH = Tắt

// Cấu trúc định nghĩa chân GPIO cần test
struct TestPinInfo {
    uint8_t pin;
    const char* name;
};

const TestPinInfo TEST_PINS[] = {
    // Port A (loại trừ PA13, PA14 để giữ cổng ST-Link SWD)
    {PA0,  "PA0"},  {PA1,  "PA1"},  {PA2,  "PA2"},  {PA3,  "PA3"},
    {PA4,  "PA4"},  {PA5,  "PA5"},  {PA6,  "PA6"},  {PA7,  "PA7"},
    {PA8,  "PA8"},  {PA11, "PA11"}, {PA12, "PA12"}, {PA15, "PA15"},

    // Port B (15 chân)
    {PB0,  "PB0"},  {PB1,  "PB1"},  {PB3,  "PB3"},  {PB4,  "PB4"},
    {PB5,  "PB5"},  {PB6,  "PB6"},  {PB7,  "PB7"},  {PB8,  "PB8"},
    {PB9,  "PB9"},  {PB10, "PB10"}, {PB11, "PB11"}, {PB12, "PB12"},
    {PB13, "PB13"}, {PB14, "PB14"}, {PB15, "PB15"},

    // Port C
    {PC14, "PC14"}, {PC15, "PC15"}
};

const uint8_t SO_CHAN_TEST = sizeof(TEST_PINS) / sizeof(TEST_PINS[0]);

// Stub thư viện cho GCC ARM
extern "C" int _write(int file, char *ptr, int len) {
    (void)file; (void)ptr;
    return len;
}

uint32_t thoi_gian_nhay_truoc = 0;
bool trang_thai_led = false;
int chan_dang_keo_truoc_do = -1;

void setup() {
    // 1. Giải phóng PA15, PB3, PB4 khỏi JTAG để làm GPIO thường (vẫn giữ SWD trên PA13/PA14)
    #if defined(__HAL_AFIO_REMAP_SWJ_NOJTAG)
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
    #endif

    // 2. Cấu hình đèn LED PC13
    pinMode(LED_PC13, OUTPUT);
    digitalWrite(LED_PC13, LED_TAT);

    // 3. Cấu hình tất cả các chân GPIO cần test ở chế độ INPUT_PULLUP
    for (uint8_t i = 0; i < SO_CHAN_TEST; i++) {
        pinMode(TEST_PINS[i].pin, INPUT_PULLUP);
    }

    // 4. Khởi động cổng Serial 115200 (PA9-TX, PA10-RX) để theo dõi chi tiết (nếu có cắm UART)
    Serial.begin(115200);

    Serial.println("\n========================================================");
    Serial.println("     CHUONG TRINH TEST STM32 SONG / CHET VA GPIO        ");
    Serial.println("========================================================");
    Serial.println(">> [B1] Khoi dong 3 giay: LED PC13 nhay CHAT (500ms)...");

    // 5. Khởi động 3 giây đầu: Đèn PC13 nháy CHẬM (3 lần x 1000ms = 3 giây)
    for (int i = 1; i <= 3; i++) {
        digitalWrite(LED_PC13, LED_BAT); // BẬT ĐÈN
        delay(500);
        digitalWrite(LED_PC13, LED_TAT); // TẮT ĐÈN
        delay(500);
        Serial.print("   -> Khoi dong giay thu: ");
        Serial.println(i);
    }

    Serial.println(">> [B2] Da qua 3 giay khoi dong thanh cong!");
    Serial.println(">> [B3] LED PC13 bat dau nhay NHANH (100ms) - STM32 SONG TOT!");
    Serial.println(">> [B4] CHE DO TEST GPIO:");
    Serial.println("   - Lay 1 soi day cam tu chan GND cham vao bat ky chan nao.");
    Serial.println("   - Neu chan do hoat dong tot: LED PC13 se TAT trong luc cham!");
    Serial.println("   - Nha day ra khoi GND: LED PC13 tiep tuc nhay nhanh.");
    Serial.println("========================================================\n");

    thoi_gian_nhay_truoc = millis();
}

void loop() {
    bool co_chan_keo_gnd = false;
    int index_chan_gnd = -1;

    // Quét toàn bộ các chân GPIO xem có chân nào chạm GND không
    for (uint8_t i = 0; i < SO_CHAN_TEST; i++) {
        if (digitalRead(TEST_PINS[i].pin) == LOW) {
            co_chan_keo_gnd = true;
            index_chan_gnd = i;
            break; // Tìm thấy chân đang chạm GND
        }
    }

    if (co_chan_keo_gnd) {
        // CÓ CHÂN ĐANG KÉO XUỐNG GND:
        // TẮT NGAY ĐÈN PC13 trong suốt thời gian chân đang kéo GND!
        digitalWrite(LED_PC13, LED_TAT);
        trang_thai_led = false;

        // In thông báo ra Serial 1 lần khi chân vừa được kéo
        if (chan_dang_keo_truoc_do != index_chan_gnd) {
            chan_dang_keo_truoc_do = index_chan_gnd;
            Serial.print("[GPIO TEST - OK] Chan [");
            Serial.print(TEST_PINS[index_chan_gnd].name);
            Serial.println("] duoc keo xuong GND -> LED PC13 DA TAT!");
        }
        delay(10); // Debounce chống rung tiếp xúc
    } else {
        // KHÔNG CÓ CHÂN NÀO CHẠM GND:
        if (chan_dang_keo_truoc_do != -1) {
            Serial.println("[GPIO TEST] Da nha khoi GND -> LED PC13 tiep tuc nhay nhanh.");
            chan_dang_keo_truoc_do = -1;
        }

        // Đèn PC13 nháy NHANH (chu kỳ 100ms BẬT / 100ms TẮT)
        uint32_t now = millis();
        if (now - thoi_gian_nhay_truoc >= 100) {
            thoi_gian_nhay_truoc = now;
            trang_thai_led = !trang_thai_led;
            digitalWrite(LED_PC13, trang_thai_led ? LED_BAT : LED_TAT);
        }
    }
}
