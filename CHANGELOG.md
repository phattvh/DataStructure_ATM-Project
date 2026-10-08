# CHANGELOG - DỰ ÁN MÔ PHỎNG ATM (NHÁNH FIX & NHÁNH PHAT)

Tài liệu này ghi lại toàn bộ lịch sử thay đổi, thiết kế kỹ thuật, sửa lỗi bảo mật và nâng cấp mã nguồn được thực hiện trên nhánh `phat` và nhánh tích hợp `fix`.

## [2.0.0] - 2026-10-08 (Phase 2 Integration, Adversarial Hardening & Production Release Ready)

Hoàn thành tích hợp 100% Phase 2 giữa 3 thành viên (Member A - Tuấn, Member B - Trí, Member C - Phát) trên nhánh `fix`, trải qua 3 vòng Code Review chuyên sâu và gia cố độ bền:

### 🛡️ Gia cố Bảo mật & Phòng chống Tấn công Phản biện (Adversarial Hardening)
- **Triệt tiêu Signed Integer Underflow / Undefined Behavior**:
  - `Account::canWithdraw()` kiểm tra `this->_lBalance < lAmount` trước khi tính toán hiệu số, ngăn chặn hoàn toàn việc truyền số tiền cực lớn (`LONG_MAX`) vượt qua bước kiểm tra số dư.
- **Kiểm soát Tương thích Tiền tệ (Cross-Currency Transfer Protection)**:
  - `UserController::processTransfer()` chặn chuyển tiền chéo loại tiền tệ (`VND` vs `USD`), trả về mã lỗi `ERR_INVALID_FORMAT` và hiển thị cảnh báo thân thiện.
  - `ConsoleView::printReceipt()` và bảng lịch sử giao dịch hỗ trợ hiển thị đơn vị tiền tệ linh hoạt theo thuộc tính tài khoản (`acc.getCurrency()`).
- **Chặn Chuyển Tiền Đến Tài Khoản Đang Bị Khóa (`KhoaThe.txt`)**:
  - `UserController::runUserSession()` kiểm tra ID người nhận đối chiếu với `KhoaThe.txt` ngay khi nhập số tài khoản, ngăn chặn việc đóng băng tiền của người gửi.
- **Chống Lỗi Hồi Sinh Thẻ/Tài Khoản Đã Bị Xóa (Account Resurrection Guard)**:
  - `AtmController::processUserMenu()` chỉ lưu tài khoản và thẻ nếu tài khoản vẫn tồn tại trong RAM và tệp trên đĩa chưa bị xóa bởi Admin.
- **Cơ Chế Hủy Thao Tác Đổi Mã PIN (Cancelable PIN Change)**:
  - Cho phép người dùng nhập `0` hoặc nhấn Enter (chuỗi rỗng) để hủy thao tác đổi PIN và quay lại menu chính an toàn.
- **Bảo Vệ Tính Toàn Vẹn Tệp & Tránh Rò Rỉ Dữ Liệu**:
  - Chống Delimiter Injection: Chặn ký tự `'|'`, `'\n'`, `'\r'` trong tên chủ thẻ.
  - Chống Orphan File Collision: Kiểm tra `fs::exists()` trước khi tạo tệp tài khoản mới.
  - Tự động lưu trữ (Archive `.bak`) tệp lịch sử khi tái sử dụng ID thẻ cũ để bảo vệ quyền riêng tư của chủ thẻ mới.
  - Ghi tệp nguyên tử (Atomic Write qua `.tmp` và `rename`) cho toàn bộ thao tác cập nhật dữ liệu.

### 🏛️ Chuẩn hóa Kiến trúc & Quy chuẩn Lập trình (Coding Standards)
- **Hợp nhất Kiến trúc Phân hệ Admin**:
  - Tích hợp lớp `AdminController` do Member A phát triển vào `AtmController::processAdminLogin()`, loại bỏ 100% mã chết (*dead code*).
- **Bộ Phân Giải Lịch Sử Giao Dịch Đa Định Dạng (Dual-Format Transaction Parser)**:
  - `Transaction::parseFromFileLine()` nhận diện linh hoạt cả định dạng đặc tả 5 trường (`ID | TypeName | Amount | Timestamp | Detail`) và định dạng nội bộ 4 trường của Member B, sử dụng `findNthChar` bảo toàn trọn vẹn mô tả có chứa ký tự `'|'`.
- **Tuân thủ 100% HCMUE C++ Coding Standard V2**:
  - Chuẩn hóa 100% truy xuất thuộc tính thành viên bằng tiền tố `this->` (Rule 17) trong toàn bộ các lớp.
  - Áp dụng đầy đủ Hungarian Notation (Rule 3), hằng số chữ hoa (Rule 4) và RAII giải phóng tài nguyên tệp/bộ nhớ (Rule 18).

### 🧪 Hệ thống Kiểm thử Toàn diện (100% Pass)
- Mở rộng bộ kiểm thử `test_phase_2_c` lên 106 test case phủ kín 12 nhóm nghiệp vụ.
- Toàn bộ 7 bộ test suite (`test_runner`, `test_phase_2_a`, `test_phase_2_b`, `test_phase_2_c`, `test_member_a`, `test_member_c`, `test_ac`) vượt qua 100% với 0 cảnh báo biên dịch (`-Wall -Wextra`).

---

## [1.2.0] - 2026-10-04 (Branch Fix: Tích hợp Hợp nhất & Vá Lỗ hổng Sâu)

Giai đoạn hợp nhất mã nguồn giữa Tuấn (Member A) và Phát (Member C), giải quyết triệt để các xung đột bằng cách chọn lọc mã nguồn tối ưu và khắc phục các lỗ hổng logic nghiệp vụ:

### 🚀 Nâng cấp & Sửa lỗi Nghiệp vụ (`UserController`)
- **Vá lỗi chuyển tiền làm biến mất số dư trong `runUserSession()`**:
  - Khắc phục lỗ hổng nghiêm trọng: Menu chuyển tiền trước đó chỉ trừ tiền người gửi mà không hỏi tài khoản nhận.
  - Bổ sung yêu cầu nhập số tài khoản người nhận, kiểm tra định dạng 14 chữ số, chặn tự chuyển tiền cho chính mình (`ERR_SAME_ACCOUNT`).
- **Bảo đảm Tính nguyên tử ACID với cơ chế Rollback**:
  - Trong `processTransfer()`: Nếu quá trình nạp tiền vào tài khoản người nhận gặp lỗi (ví dụ tràn số nguyên `ERR_SYSTEM_OVERFLOW`), hệ thống tự động hoàn tiền lại nguyên vẹn cho người gửi.
- **Tích hợp hiển thị Biên lai giao dịch chuẩn**:
  - Tích hợp `ConsoleView::printReceipt()` vào các giao dịch Rút tiền và Chuyển tiền thành công trong phiên khách hàng.
- **Tái sử dụng mã nguồn (DRY Principle)**:
  - Loại bỏ định nghĩa lặp lại của `isValidPinFormat` trong `UserController`, ủy thác trực tiếp sang `Card::isValidPinFormat`.
- **Kiểm tra kết quả đổi mã PIN**:
  - `enforceDefaultPinChange()` và `processChangePin()` kiểm tra cẩn thận giá trị trả về của `card.changePin(strNewPin)` trước khi thông báo thành công.

### 🧪 Đồng bộ & Mở rộng Bộ kiểm thử (`Makefile` & `test/`)
- Cập nhật `Makefile` biên dịch đồng thời `ConsoleView`, `Card`, `Account`, `UserController`.
- `make test` thực thi song song cả 2 bộ kiểm thử:
  - `test_member_c`: Kiểm thử biên, kiểm thử I/O stream tự động.
  - `test_member_a`: Bổ sung kiểm thử cơ chế Rollback nguyên tử và mở khóa thẻ chuẩn SRP.

---

## [1.1.0] - 2026-10-04 (Security Hardening & Refactoring Phase)

Đợt nâng cấp toàn diện nhằm giải quyết triệt để các lỗ hổng bảo mật, bẫy I/O terminal, lỗi đóng gói mô hình và hiện đại hóa mã nguồn theo chuẩn C++17 trên nhánh `phat`.

### 🛡️ Bảo mật & Xử lý I/O Terminal (`ConsoleView`)
- **Vá lỗ hổng chuỗi thoát ANSI (Escape Sequence)**:
  - Triển khai lớp RAII `LinuxTerminalRawGuard` quản lý bật/tắt chế độ terminal raw mode duy nhất 1 lần thay vì gọi system call liên tục trên từng ký tự.
  - Xây dựng cơ chế phát hiện và drain sạch sẽ chuỗi escape (mã `27` / `\033` theo sau bởi `[`, `A`, `B`, `~` khi người dùng bấm phím mũi tên hoặc phím điều hướng), ngăn tuyệt đối việc làm hỏng mã PIN hay in thừa dấu `**`.
- **Vá lỗi bẫy số thực & EOF trong `inputMoney()`**:
  - Chuyển sang đọc theo dòng `std::getline()`, loại bỏ việc `cin >> amount` âm thầm nuốt phần nguyên của số thập phân (`50000.75`) hoặc chuỗi kèm chữ (`50000abc`).
  - Kiểm tra tính hợp lệ từng ký tự số bằng `std::isdigit()`, từ chối số âm, số 0 và chuỗi rác.
  - Xử lý dứt điểm tín hiệu `EOF` (`Ctrl+D`): Phát hiện luồng đóng và thoát an toàn, ngăn chặn 100% nguy cơ treo máy vòng lặp vô hạn (Infinite Loop 100% CPU).
  - Hỗ trợ tiêm luồng đầu vào (`std::istream& inStream = std::cin`) phục vụ kiểm thử tự động.
- **Bổ sung UI Phân hệ User (Đúng phạm vi trách nhiệm)**:
  - Bổ sung `printUserMenu()` cho khách hàng (Xem thông tin, Rút tiền, Chuyển tiền, Đổi PIN, Đăng xuất).
  - Bổ sung `printReceipt()` in biên lai giao dịch tài chính chuẩn hóa.
  - Tách rời khớp nối cứng: Cung cấp `displayAccountDetails()` với các kiểu nguyên thủy độc lập bên cạnh `displayAccountInfo()`.

### 🔒 Củng cố Mô hình Nghiệp vụ (`Card` & `Account`)
- **Mô hình Thẻ từ (`Card`)**:
  - Bổ sung hàm kiểm tra định dạng tĩnh `isValidPinFormat()`: Yêu cầu chính xác 6 ký tự số.
  - `changePin()`: Trả về kiểu `bool`, từ chối mọi mã PIN rỗng, sai độ dài, hoặc chứa ký tự không phải chữ số.
  - Phân định rõ ràng nguyên lý Đơn trách nhiệm (SRP):
    - `resetFailedAttempts()`: Chỉ đặt lại số lần thử sai về 0 khi đăng nhập thành công.
    - `unlockCard()`: Nghiệp vụ riêng của Admin, mở khóa thẻ và đặt lại số lần sai.
  - `recordFailedAttempt()`: Chặn tăng biến đếm vô hạn khi thẻ đã ở trạng thái khóa.
- **Mô hình Tài khoản (`Account`)**:
  - `deposit()`: Bịt kín lỗ hổng backdoor rút tiền bằng cách chặn tham số $\le 0$; bổ sung kiểm tra chống tràn số nguyên `std::numeric_limits<long>::max()`.
  - `withdraw()`: Tự động gọi và kiểm tra ràng buộc `canWithdraw()`, chặn việc trừ âm tài khoản nếu vi phạm điều kiện.

### ⚙️ Hiện đại hóa Cấu hình Hệ thống (`Common.h`)
- Chuyển toàn bộ các hằng số hệ thống sang `inline constexpr` và `inline const std::string` (chuẩn C++17) để triệt tiêu việc duplicate dữ liệu tĩnh giữa các Translation Units (`.cpp`).
- Bổ sung mã lỗi `ERR_SYSTEM_OVERFLOW = 10` và `ERR_SAME_ACCOUNT = 9` vào `enum ErrorCode`.

### 🧪 Nâng cấp Bộ kiểm thử Tự động (`test/test_member_c.cpp`)
- Thay thế hoàn toàn thư viện `<cassert>` bằng macro tùy biến `TEST_CHECK`, ngăn chặn việc bị vô hiệu hóa khi biên dịch với cờ tối ưu `-DNDEBUG`.
- Bổ sung bộ kiểm thử Stream tự động bằng `std::istringstream` để test `inputMoney()` và `inputPassword()` mà không cần người dùng nhập liệu thủ công.
- Phủ kín 100% kịch bản kiểm thử biên (Boundary & Edge Cases).

---

## [1.0.0] - 2026-10-04 (Initial Deliverables Phase)

Giai đoạn khởi tạo các thành phần nền tảng cho Tuần 1 theo phân công đồ án.
