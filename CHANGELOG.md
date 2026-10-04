# CHANGELOG - NHÁNH PHAT (MEMBER C - TUẦN 1)

Tài liệu này ghi lại toàn bộ lịch sử thay đổi, thiết kế kỹ thuật, sửa lỗi bảo mật và nâng cấp mã nguồn được thực hiện trên nhánh `phat` của dự án **DataStructure_ATM-Project**.

---

## [1.1.0] - 2026-10-04 (Security Hardening & Refactoring Phase)

Đợt nâng cấp toàn diện nhằm giải quyết triệt để các lỗ hổng bảo mật, bẫy I/O terminal, lỗi đóng gói mô hình và hiện đại hóa mã nguồn theo chuẩn C++17.

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
- Bổ sung mã lỗi `ERR_SYSTEM_OVERFLOW = 9` vào `enum ErrorCode`.

### 🧪 Nâng cấp Bộ kiểm thử Tự động (`test/test_member_c.cpp`)
- Thay thế hoàn toàn thư viện `<cassert>` bằng macro tùy biến `TEST_CHECK`, ngăn chặn việc bị vô hiệu hóa khi biên dịch với cờ tối ưu `-DNDEBUG`.
- Bổ sung bộ kiểm thử Stream tự động bằng `std::istringstream` để test `inputMoney()` và `inputPassword()` mà không cần người dùng nhập liệu thủ công.
- Phủ kín 100% kịch bản kiểm thử biên (Boundary & Edge Cases).

---

## [1.0.0] - 2026-10-04 (Initial Deliverables Phase)

Giai đoạn khởi tạo các thành phần nền tảng cho Tuần 1 theo phân công đồ án.

### ✨ Tính năng đã thêm
- **`include/Common.h`**: Khởi tạo hằng số hệ thống (`DEFAULT_PIN`, `MIN_TRANSACTION`, `MIN_BALANCE_RESERVE`, `MAX_FAILED_LOGINS`, `ID_LENGTH`, `PIN_LENGTH`, `DATA_DIR`) và các enum nghiệp vụ (`TransactionType`, `UserRole`, `ErrorCode`).
- **`include/ConsoleView.h` & `src/ConsoleView.cpp`**: 
  - Hiển thị màu sắc ANSI (`[LOI]`, `[THANH CONG]`, `[CANH BAO]`).
  - Hàm nhập mật khẩu che giấu dấu `*` (`inputPassword`).
  - Khung viền menu quản trị và hiển thị danh sách thẻ Admin.
- **`include/Card.h` & `src/Card.cpp`**: 
  - Khởi tạo thực thể thẻ từ.
  - Cơ chế đếm số lần sai và tự động khóa sau 3 lần.
- **`include/Account.h` & `src/Account.cpp`**: 
  - Khởi tạo thực thể tài khoản khách hàng.
  - Hiện thực các ràng buộc tài chính: Rút tối thiểu 50k, bội số 50k, duy trì số dư tối thiểu 50k.
- **`Makefile` & `.gitignore`**: 
  - Xây dựng Makefile biên dịch với cờ `-std=c++17 -Wall -Wextra`, tách riêng thư mục `build/`.
  - Cấu hình file thực thi test tự động `make test`.

---

## Danh sách Commit trên nhánh `phat`

1. `f67d0a7` - `feat(common): initialize shared system constants and error codes`
2. `e429e2e` - `feat(ui): implement ConsoleView with ANSI colors and secure input masking`
3. `e8d9302` - `feat(models): implement Card and Account entity models following C++ standard`
4. `529dbb3` - `feat(ui): add admin layout helpers, build script and unit tests for member C deliverables`
5. `d750350` - `refactor(common): modernize constants with inline linkage and expand error codes`
6. `b55e2b3` - `fix(models): harden Card and Account invariants with strict validation`
7. `a2a4321` - `fix(ui): patch escape sequence bug, EOF hang and add User UI helpers in ConsoleView`
8. `8eed08e` - `test(qa): overhaul test suite with robust assertions and edge case coverage`
