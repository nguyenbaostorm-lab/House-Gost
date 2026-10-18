# QUY TẮC BẮT BUỘC (MANDATORY RULES)

1. **Không tự ý tạo file ngoài yêu cầu:** Tuyệt đối không tự ý sinh file mới nếu người dùng không yêu cầu rõ ràng.
2. **Chỉ làm đúng và trúng yêu cầu:** Chỉ thao tác trên đúng các file và phạm vi người dùng chỉ định.
3. **CẤU TRÚC CODE DỄ CHỈNH SỬA NHẤT CHO NGƯỜI DÙNG (BẮT BUỘC):**
   - **File `config.h`:** Chuyên chứa toàn bộ khai báo cấu hình chân cắm, thông số phần cứng đã sử dụng.
   - **File thư viện / driver:** Đóng gói toàn bộ việc khởi tạo và giao tiếp phần cứng phức tạp (I2C, PCF8575, UART...).
   - **File `main.cpp`:** Chỉ chứa thuật toán, kịch bản, các lệnh BẬT/TẮT trực quan, đơn giản, dễ đọc nhất để người dùng tự do tùy biến kịch bản theo ý muốn.

4. **TỰ ĐỘNG XUẤT FILE .BIN KHI TẠO/SỬA PHIÊN BẢN (V1, V2, V3...):**
   - Mỗi khi tạo hoặc sửa file code thuộc bất kỳ phiên bản nào (V1, V2, V3,...):
     + PHẢI tự động chạy biên dịch PlatformIO (`pio run -e ...`).
     + Tự động xuất và sao chép file `firmware.bin` vào đúng thư mục trong `BIN_CODE/` (ví dụ: `BIN_CODE/V1_CHUAN/`, `BIN_CODE/V2_RESET_PB0/`, `BIN_CODE/V3_.../`).
     + Cập nhật thêm vào `platformio.ini` và menu `NAP_CODE.bat` / `NAP_CODE.ps1` nếu có phiên bản mới để luôn sẵn sàng nạp ngay.
