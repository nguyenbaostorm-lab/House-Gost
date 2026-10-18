# QUY TẮC BẮT BUỘC (MANDATORY RULES)

1. **Không tự ý tạo file ngoài yêu cầu:** Tuyệt đối không tự ý sinh file mới nếu người dùng không yêu cầu rõ ràng.
2. **Chỉ làm đúng và trúng yêu cầu:** Chỉ thao tác trên đúng các file và phạm vi người dùng chỉ định.
3. **CẤU TRÚC CODE DỄ CHỈNH SỬA NHẤT CHO NGƯỜI DÙNG (BẮT BUỘC):**
   - **File `config.h`:** Chuyên chứa toàn bộ khai báo cấu hình chân cắm, thông số phần cứng đã sử dụng.
   - **File thư viện / driver:** Đóng gói toàn bộ việc khởi tạo và giao tiếp phần cứng phức tạp (I2C, PCF8575, UART...).
   - **File `main.cpp`:** Chỉ chứa thuật toán, kịch bản, các lệnh BẬT/TẮT trực quan, đơn giản, dễ đọc nhất để người dùng tự do tùy biến kịch bản theo ý muốn.
