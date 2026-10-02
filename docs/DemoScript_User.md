# KỊCH BẢN DEMO PHÂN HỆ KHÁCH HÀNG (USER MODULE DEMO SCRIPT)
**Dự án**: Mô phỏng Hệ thống Cây ATM Ngân hàng (C++ OOP & Data Structures)  
**Phân công**: Member A (Tuấn) — Business Logic & User Flow & Presentation Layer

---

## 1. Dữ liệu Kiểm thử Phân hệ User
- **Thẻ 1 (Mặc định)**: 10014504500001 (Chủ TK: Nguyễn Văn An, Số dư: 500.000 VND, PIN mặc định: 123456)
- **Thẻ 2 (Chuyển tiền)**: 10014504500002 (Chủ TK: Trần Thị Bình, Số dư: 1.200.000 VND, PIN: 123456)
- **Thẻ 3 (Nhận tiền)**: 10014504500003 (Chủ TK: Lê Văn Cường, Số dư: 750.000 VND)

---

## 2. Kịch bản Trình diễn Chi tiết (Phân hệ User của Member A)

### KỊCH BẢN 1: BẢO VỆ ĐĂNG NHẬP & CƠ CHẾ KHÓA THẺ (3 LẦN SAI)
1. **Bắt lỗi định dạng ID**:
   - Nhập ID ngắn hơn hoặc dài hơn 14 chữ số (ví dụ: 12345).
   - *Kết quả mong đợi*: Báo lỗi màu đỏ ID the khong hop le! Phai la 14 chu so.
2. **Ẩn mật khẩu bằng dấu * và xử lý Backspace**:
   - Nhập phím số thấy hiển thị ký tự *.
   - Bấm Backspace xóa lùi đúng số lượng ký tự *.
3. **Cơ chế đếm sai 3 lần**:
   - Nhập sai mã PIN lần 1: Hệ thống thông báo cảnh báo còn 2 lần thử.
   - Nhập sai mã PIN lần 2: Hệ thống cảnh báo còn 1 lần thử duy nhất.
   - Nhập sai mã PIN lần 3: Thẻ lập tức bị khóa, thông báo màu đỏ và thoát phiên giao dịch.

### KỊCH BẢN 2: BẮT BUỘC ĐỔI MÃ PIN MẶC ĐỊNH LẦN ĐẦU (123456)
1. **Phát hiện PIN mặc định**:
   - Đăng nhập thẻ mới với PIN gốc 123456.
   - *Kết quả mong đợi*: Hệ thống phát hiện isDefaultPin() == true, chặn không cho vào Menu và buộc đổi mã PIN mới.
2. **Xác thực 2 lần khi đổi PIN**:
   - Nhập trùng PIN mặc định: Báo lỗi không được đặt lại 123456.
   - Nhập 2 lần không khớp: Báo lỗi và yêu cầu nhập lại.
   - Nhập đúng 6 số mới (ví dụ: 888888): Báo thành công và chuyển vào Menu chính.

### KỊCH BẢN 3: RÚT TIỀN & KIỂM TRA TOÀN BỘ RÀNG BUỘC TÀI CHÍNH
1. **Bẫy lỗi cin.fail() khi nhập chữ**:
   - Tại màn hình nhập số tiền, gõ chuỗi chữ cái (ví dụ: bc, mot trieu).
   - *Kết quả mong đợi*: Chương trình không bị văng/crash, dọn sạch buffer và yêu cầu nhập lại số nguyên.
2. **Kiểm tra mức tối thiểu (>= 50.000 VND)**:
   - Nhập 20000 -> Báo lỗi So tien rut toi thieu phai tu 50,000 VND!.
3. **Kiểm tra bội số của 50.000 VND**:
   - Nhập 75000 hoặc 120000 -> Báo lỗi So tien rut phai la boi so cua 50,000 VND!.
4. **Kiểm tra số dư tối thiểu duy trì (50.000 VND)**:
   - Với tài khoản có 500.000 VND, rút 500000 -> Báo lỗi So du khong du! Can giu lai it nhat 50,000 VND.
5. **Rút tiền thành công**:
   - Rút 200000 -> Báo thành công màu xanh lá, in số dư còn lại 300.000 VND.

### KỊCH BẢN 4: CHUYỂN TIỀN NGUYÊN TỬ (ATOMICITY)
1. **Kiểm tra số tài khoản nhận**:
   - Chuyển cho chính mình -> Báo lỗi ERR_SAME_ACCOUNT.
2. **Kiểm tra hạn mức & số dư người gửi**:
   - Nhập số tiền vượt quá số dư khả dụng -> Báo lỗi không đủ tiền.
3. **Thực thi giao dịch 2 chiều**:
   - Chuyển 200000 cho tài khoản nhận.
   - *Kết quả mong đợi*: Tài khoản người gửi trừ đúng 200.000 VND, tài khoản người nhận tăng đúng 200.000 VND.
