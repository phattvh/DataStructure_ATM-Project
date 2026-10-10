# 📋 NHẬT KÝ THAY ĐỔI DỰ ÁN (PROJECT CHANGELOG)

Tài liệu này ghi lại toàn bộ lịch sử phát triển, nâng cấp kiến trúc, tích hợp liên phân hệ và sửa lỗi trong suốt quá trình thực hiện đồ án **Mô phỏng Hệ thống Cây ATM Ngân hàng (DataStructure_ATM-Project)**.

Dự án được thực hiện bởi nhóm 3 thành viên:
- **Hứa Nhựt Tuấn (Thành viên A - Member A)**: Phụ trách Tầng Nghiệp vụ Khách hàng (User Business Logic), Tầng Giao diện (`ConsoleView`), Mô hình `Card` & `Account`, Giao dịch Tài chính Đĩa (Rút tiền, Chuyển tiền nguyên tử, Đổi PIN, Lịch sử), Kịch bản Demo và Báo cáo Word.
- **Ngô Trí (Thành viên B - Member B)**: Phụ trách Cấu trúc Dữ liệu Tự tạo (`LinkedList<T>` Generic Template), Tầng Lưu trữ Tệp (`FileService` 5 file vật lý), Mô hình `Admin` & `Transaction`, Kiểm định Rò rỉ Bộ nhớ (Memory Leak Audit & Valgrind - 0 byte leak) và Báo cáo CTDL & Thuật toán Big-O.
- **Trần Vũ Hỏa Phát (Thành viên C - Member C / Tech Lead)**: Phụ trách Kiến trúc Hệ thống Tổng thể, Định nghĩa Chung (`Common.h`), Makefile & Build Scripts, Bộ Điều phối Trung tâm (`AtmController`), Phân hệ Quản trị (`AdminController`), Bảo mật Terminal Raw Mode, Gia cố Tính toàn vẹn Dữ liệu (Atomic Write, Adversarial Hardening) và Hợp nhất Mã nguồn Đa nhánh.

---

## [2.2.0] - 2026-10-10: Hoàn thiện Toàn diện, Gia cố Chiều sâu & Chuẩn bị Bàn giao (Final Production Hardening)

Phiên bản hoàn thiện toàn diện, giải quyết toàn bộ các lỗ hổng biên phát hiện qua vòng Adversarial Code Review, tối ưu hóa trải nghiệm người dùng và chuẩn hóa 100% hồ sơ tài liệu kỹ thuật.

### 🛡️ Gia cố An toàn Dữ liệu & Bảo mật Hệ thống (Security & Data Integrity)
- **Cô lập Tệp tin Tạm Atomic Write bằng PID (`d46d555`)**:
  - *Đóng góp: Thành viên C (Phát)*
  - `FileService::atomicWriteFile()` tạo tệp tạm với định dạng `.tmp.<PID>` thay vì tên cố định `.tmp`, ngăn chặn hoàn toàn xung đột ghi đè giữa các tiến trình chạy song song.
- **Chống Tấn công Path Traversal khi Chuyển tiền (`a6ad564`)**:
  - *Đóng góp: Thành viên C (Phát) & Thành viên A (Tuấn)*
  - `UserController::processTransferAndPersist()` bổ sung bước kiểm tra định dạng số thẻ nhận nghiêm ngặt (`isValidIdFormat`), chặn tuyệt đối các chuỗi đặc biệt (`../../`) can thiệp vào cây thư mục hệ thống.
- **Bảo toàn Tính Nhất quán khi Xóa Thẻ ATM (`5cd0204`)**:
  - *Đóng góp: Thành viên C (Phát)*
  - `AtmController::deleteCardAccount()` và `AdminController::deleteCard()` chỉ thực thi xóa file tài khoản trên đĩa (`[ID].txt`) sau khi `FileService::saveCards()` đã ghi thành công danh sách thẻ mới vào `TheTu.txt`, triệt tiêu rủi ro mất tài khoản nếu I/O thẻ gặp sự cố.
- **Giới hạn Độ dài Bộ đệm Nhập Mật khẩu (`fcf8e2f`)**:
  - *Đóng góp: Thành viên C (Phát) & Thành viên A (Tuấn)*
  - `ConsoleView::inputPassword()` giới hạn tối đa 32 ký tự, ngăn chặn người dùng cố tình dán dữ liệu khổng lồ gây tràn bộ nhớ console hoặc treo giao diện.
- **Hỗ trợ Đa Tiền tệ cho Tài khoản Ngoại tệ (`699ac3a`)**:
  - *Đóng góp: Thành viên C (Phát) & Thành viên A (Tuấn)*
  - `Account::canWithdraw()` nhận diện loại tiền tệ của tài khoản, tự động áp dụng hạn mức rút tối thiểu và bội số phù hợp cho USD ($10) thay vì ép buộc mức VND (50.000 VND).
- **Chuẩn hóa Mã Lỗi Chuyển tiền (`8878245`)**:
  - *Đóng góp: Thành viên C (Phát) & Thành viên A (Tuấn)*
  - `UserController::processTransferAndPersist()` trả về chính xác mã lỗi `ERR_RECIPIENT_NOT_FOUND` khi tài khoản nhận không tồn tại, giúp thông báo lỗi hiển thị chính xác và trực quan.
- **Tối ưu Hóa I/O Đổi PIN Mặc định (`5f8947c`)**:
  - *Đóng góp: Thành viên C (Phát)*
  - Loại bỏ thao tác ghi đĩa dư thừa `saveCards()` sau khi đã hoàn tất `processChangePinAndPersist()`, giảm 50% thao tác I/O đĩa trong luồng đăng nhập lần đầu.

### 🎨 Tinh chỉnh Giao diện & Trải nghiệm Người dùng (UX Enhancement)
- **Phân biệt Lỗi Đọc Tệp và Lịch sử Trống (`3aa4a91`)**:
  - *Đóng góp: Thành viên A (Tuấn)*
  - `UserController::displayTransactionHistory()` phân định rõ ràng giữa lỗi I/O không thể đọc tệp lịch sử (`ERR_FILE_NOT_FOUND`) và trạng thái tài khoản mới hợp lệ chưa phát sinh giao dịch nào.

### 📚 Tài liệu Kỹ thuật & Báo cáo Đồ án (Documentation & Standards)
- **Chuẩn hóa Hệ thống Tài liệu (`56fd6b4`, `298bf49`)**:
  - *Đóng góp: Cả 3 thành viên*
  - Đánh số lại các sơ đồ kỹ thuật UML/DFD (`docs/08_so_do_uml_va_luong_du_lieu.md`), cập nhật cây thư mục chuẩn và cập nhật bảng mục lục `docs/README.md`.
  - Cập nhật tài liệu phân tích CTDL và độ phức tạp thuật toán Big-O (`docs/04_ctdl_va_thuat_toan.md`) do Thành viên B (Trí) biên soạn.
  - Đồng bộ hóa toàn bộ tiến độ dự án đạt 95% (38/40 nhiệm vụ hoàn thành) trên `docs/TIEN_DO_CONG_VIEC.md`.

---

## [2.1.0] - 2026-10-09: Tích hợp Giao dịch Tài chính Đĩa Phase 3 & Kiểm định Rò rỉ Bộ nhớ Phase 4 (Persistence & Memory Audit)

Giai đoạn tích hợp quan trọng đưa toàn bộ dữ liệu giao dịch tài chính xuống tầng tệp tin vật lý với tính nguyên tử ACID, cùng bộ kiểm định rò rỉ bộ nhớ chuyên sâu đạt chuẩn tuyệt đối.

### 💾 Tích hợp Tầng Lưu trữ cho Giao dịch Khách hàng (`c2ea57e`)
- **[A10] Kết nối `FileService` vào Nghiệp vụ Rút tiền**:
  - *Đóng góp: Thành viên A (Tuấn)*
  - Triển khai `processWithdrawAndPersist()`: Sau khi trừ tiền hợp lệ trong RAM, tự động gọi `FileService::saveAccount()` cập nhật số dư vào `data/[ID].txt` và `FileService::appendTransaction()` ghi nhật ký giao dịch vào `data/LichSu[ID].txt`.
  - Các giao dịch bị từ chối do vi phạm quy tắc số dư duy trì hoặc bội số tiền sẽ **không** bị ghi vết rác vào file lịch sử.
- **[A11] Kết nối `FileService` vào Đổi PIN & Xem Lịch sử Giao dịch**:
  - *Đóng góp: Thành viên A (Tuấn)*
  - Triển khai `processChangePinAndPersist()`: Lưu trực tiếp mã PIN mới vào `data/TheTu.txt` ngay khi đổi thành công mà không cần chờ người dùng đăng xuất.
  - Triển khai `displayTransactionHistory()`: Đọc danh sách giao dịch từ `data/LichSu[ID].txt` qua `FileService::loadTransactions()`, trình bày dạng bảng phân trang với đầy đủ thời gian, loại giao dịch, số tiền và nội dung chi tiết.
- **[B10] Chuyển tiền Nguyên tử 2 Đầu trên Đĩa (Atomic Two-Way Transfer)**:
  - *Đóng góp: Thành viên A (Tuấn) phối hợp cùng Thành viên B (Trí)*
  - Triển khai `processTransferAndPersist()` đảm bảo tính toàn vẹn tuyệt đối:
    1. Ghi đĩa số dư mới của người gửi (`saveAccount`). Nếu lỗi: hủy thao tác, bảo toàn tiền.
    2. Ghi đĩa số dư mới của người nhận (`saveAccount`). Nếu lỗi: kích hoạt Rollback hoàn trả tiền người gửi.
    3. Ghi lịch sử giao dịch chuyển tiền cho cả 2 bên.
  - Kiểm tra và từ chối chuyển tiền nếu thẻ nhận nằm trong danh sách khóa `data/KhoaThe.txt`.
- **[A09] Định dạng Thời gian Thực trên Biên lai**:
  - *Đóng góp: Thành viên A (Tuấn)*
  - Tích hợp `getNowTimestamp()` (`YYYY-MM-DD HH:MM:SS`), đảm bảo toàn bộ biên lai in ra và dòng ghi vết tệp tin đều mang thời gian thực chính xác.

### 🧠 Kiểm định Rò rỉ Bộ nhớ & Độ Bền Hệ thống (`742c72a`, `d2132d2`)
- **[B11] Bộ Kiểm thử Bộ nhớ Chuyên sâu (`test/test_memory_leak.cpp`)**:
  - *Đóng góp: Thành viên B (Trí)*
  - Xây dựng hệ thống Memory Tracking chuyên dụng ghi đè toán tử `new`/`delete` để giám sát heap memory theo thời gian thực.
  - Thực hiện 6 kịch bản kiểm thử:
    1. Vòng đời cơ bản và Destructor của `LinkedList<T>`.
    2. Thao tác xóa `removeIf` tại mọi vị trí (đầu, giữa, cuối).
    3. Quản lý bộ nhớ với các đối tượng phức tạp (`std::string`, `Card`, `Account`, `Admin`, `Transaction`).
    4. Tải và giải phóng dữ liệu qua 5 file vật lý của `FileService`.
    5. Vòng đời toàn bộ phiên làm việc của `AtmController`.
    6. **Stress Testing 50.000 nodes $\times$ 3 chu kỳ**: Thực hiện 150.306 lần cấp phát và giải phóng.
  - **Kết quả nghiệm thu**: **67/67 test cases PASS (100%)**, **0 bytes leaked**, **0 lỗi double-free**, đỉnh sử dụng bộ nhớ chỉ ~781 KB. Đạt trọn vẹn điểm chuẩn C++ Coding Standard V2.
- **Tự động hóa Kiểm thử Valgrind (`scripts/valgrind_check.sh`)**:
  - *Đóng góp: Thành viên B (Trí)*
  - Cung cấp script tự động chạy Valgrind Memcheck với cờ `--leak-check=full --show-leak-kinds=all --track-origins=yes`.

### 🧪 Hệ thống Kiểm thử Tích hợp Toàn diện (`9d489ca`, `123aa88`)
- **Bộ Kiểm thử Member A (`test/test_phase_3_a.cpp`)**: 51/51 test cases PASS (100%), bao quát rút tiền persistence, chuyển tiền nguyên tử, đổi PIN, định dạng thời gian và kịch bản toàn trình E2E.
- **Bộ Kiểm thử Tích hợp Phase 3 (`test/test_phase_3.cpp`)**: 58/58 test cases PASS (100%), do Thành viên C (Phát) xây dựng để kiểm thử toàn diện các luồng liên kết giữa 3 thành viên.
- **Sanitizer Testing (`make test_asan`)**: Tích hợp công cụ biên dịch AddressSanitizer (ASan) và UndefinedBehaviorSanitizer (UBSan), đảm bảo không có lỗi truy cập vùng nhớ bất hợp pháp.

---

## [2.0.0] - 2026-10-08: Tích hợp Toàn diện Phase 2 - CTDL Tự viết, Tầng File I/O & Bộ điều phối (Full Phase 2 Integration)

Mốc chuyển giao kiến trúc lớn nhất của dự án: Hợp nhất mã nguồn của cả 3 thành viên, kết nối cấu trúc dữ liệu `LinkedList<T>` tự cài đặt, hoàn thiện tầng truy xuất 5 loại tệp tin và xây dựng bộ điều phối ứng dụng hoàn chỉnh.

### 🧱 Cấu trúc Dữ liệu Tự tạo & Tầng Lưu trữ Tệp (`7c49b54`)
- **[B01, B02, B03] Cài đặt Generic Template `LinkedList<T>`**:
  - *Đóng góp: Thành viên B (Trí)*
  - Tự hiện thực danh sách liên kết đơn độc lập hoàn toàn (không sử dụng `std::list` hay `std::vector`), quản lý hai con trỏ `_pHead` và `_pTail` giúp thao tác thêm cuối `addTail()` đạt độ phức tạp $\mathcal{O}(1)$.
  - Cung cấp đầy đủ các phương thức: `addTail`, `removeIf`, `findIf`, `clear`, `getSize`, `isEmpty`, `getHead`.
  - Đảm bảo an toàn bộ nhớ: Thu hồi toàn bộ node trong Destructor `~LinkedList()`, vô hiệu hóa Copy Constructor và Copy Assignment Operator (`= delete`) để triệt tiêu lỗi sao chép nông gây Double Free.
- **[B04, B05] Cài đặt Thực thể Quản trị & Nhật ký Giao dịch**:
  - *Đóng góp: Thành viên B (Trí)*
  - Lớp `Admin`: Quản lý thông tin ban quản trị, hàm xác thực mật khẩu `verifyPassword()`.
  - Lớp `Transaction`: Quản lý lịch sử giao dịch, hỗ trợ định dạng chuỗi ghi tệp và phân giải dòng tệp `parseFromFileLine()`.
- **[B06, B07, B08, B09] Hiện thực Tầng `FileService` Truy xuất Tệp**:
  - *Đóng góp: Thành viên B (Trí)*
  - Đọc/Ghi danh sách Admin (`data/Admin.txt`), danh sách Thẻ từ (`data/TheTu.txt`), danh sách Thẻ bị khóa (`data/KhoaThe.txt`).
  - Đọc/Ghi/Xóa tệp tài khoản cá nhân (`data/[ID].txt`), xử lý chính xác họ tên có dấu cách qua `std::getline()`.
  - Ghi nối nhật ký thời gian thực vào tệp `data/LichSu[ID].txt` bằng chế độ `std::ios::app`.
  - Cơ chế tự động khôi phục dữ liệu mẫu (`initSampleData()`): Tự khởi tạo thư mục và sinh sẵn 3 Admin, 10 Thẻ từ và 10 tệp tài khoản cá nhân nếu hệ thống chạy lần đầu.

### 🎮 Phân hệ Quản trị Admin & Bộ Điều phối Trung tâm (`f69a552`, `dd340b5`)
- **Xây dựng Phân hệ Admin (`AdminController`)**:
  - *Đóng góp: Thành viên A (Tuấn)*
  - Hiện thực luồng đăng nhập Admin, xem danh sách thẻ, thêm thẻ ATM mới (tạo đủ 2 tệp tin), khóa/mở khóa thẻ và reset mã PIN về mặc định `123456`.
- **Xây dựng Bộ Điều phối Trung tâm (`AtmController`)**:
  - *Đóng góp: Thành viên C (Phát)*
  - Quản lý vòng đời ứng dụng và trạng thái phiên làm việc (`_pCurrentAccount`, `_pCurrentCard`, `_eCurrentRole`).
  - Điều hướng người dùng giữa Menu Đăng nhập User, Menu Đăng nhập Admin và Thoát chương trình kèm dọn dẹp bộ nhớ RAM.
  - Điểm vào chính `src/main.cpp`: Khởi động hệ thống an toàn qua `FileService::initSampleData()` và kích hoạt `AtmController::run()`.

### 🛡️ Gia cố Bảo mật & Phòng chống Tấn công Phản biện (Adversarial Hardening)
- **Hợp nhất Mã nguồn Đa nhánh (`4b583e0`, `9f4e2c8`)**:
  - *Đóng góp: Thành viên C (Phát)*
  - Tích hợp lớp `AdminController` của Thành viên A vào luồng điều phối trung tâm của `AtmController`, loại bỏ mã dư thừa.
  - Xây dựng bộ phân giải giao dịch kép (`Transaction::parseFromFileLine`) tương thích cả chuẩn 5 trường và chuẩn 4 trường, sử dụng `findNthChar` để bảo toàn mô tả có chứa ký tự phân cách `'|'`.
- **Phòng chống Tấn công Toán học & Trạng thái Bất thường (`6381204`, `74b4a8d`, `f6a8e7a`, `607356b`)**:
  - *Đóng góp: Thành viên C (Phát) & Thành viên A (Tuấn)*
  - Triệt tiêu Signed Integer Underflow: Kiểm tra `_lBalance < lAmount` trước khi tính hiệu số trong `canWithdraw()`, chặn đứng các giá trị cực lớn (`LONG_MAX`).
  - Kiểm tra tương thích tiền tệ: Chặn chuyển tiền chéo giữa tài khoản VND và USD.
  - Chặn chuyển tiền vào tài khoản bị khóa trong `data/KhoaThe.txt`.
  - Cơ chế ghi tệp nguyên tử (Atomic Write qua tệp `.tmp` và `rename`) cho toàn bộ thao tác lưu trữ.
  - Chống hồi sinh tài khoản: Không lưu đè dữ liệu nếu thẻ đã bị Admin xóa trên đĩa trong khi phiên User đang chạy.
  - Cơ chế hủy đổi PIN an toàn: Cho phép bấm `0` hoặc Enter để quay lại menu chính.
  - Chuẩn hóa 100% quy tắc Rule 17 của C++ Coding Standard V2 (`this->` cho mọi thuộc tính thành viên).

---

## [1.2.0] - 2026-10-04: Hợp nhất Tuần 1 & Cơ chế Giao dịch Nguyên tử In-Memory (First Team Merge & Atomic Rollback)

Mốc hợp nhất phiên bản đầu tiên giữa Thành viên A và Thành viên C trên nhánh tích hợp `fix`, khắc phục xung đột mã nguồn và giải quyết triệt để các lỗ hổng tài chính in-memory.

### 🚀 Nâng cấp & Sửa lỗi Nghiệp vụ Khách hàng (`126e8af`, `b17e3b4`)
- **Khắc phục Lỗ hổng Biến mất Tiền khi Chuyển khoản**:
  - *Đóng góp: Thành viên C (Phát) phối hợp cùng Thành viên A (Tuấn)*
  - Phát hiện và sửa lỗi nghiêm trọng: Menu chuyển tiền trước đó chỉ trừ tiền người gửi mà không nhận diện số tài khoản đích.
  - Bổ sung bước nhập tài khoản nhận, kiểm tra định dạng 14 chữ số, chặn tự chuyển tiền cho chính mình (`ERR_SAME_ACCOUNT`).
- **Triển khai Cơ chế Hoàn tiền Tự động (Atomic Rollback)**:
  - *Đóng góp: Thành viên C (Phát) & Thành viên A (Tuấn)*
  - Trong `processTransfer()`: Nếu quá trình nạp tiền vào tài khoản người nhận thất bại (ví dụ chạm trần số nguyên `ERR_SYSTEM_OVERFLOW`), hệ thống tự động hoàn trả số dư nguyên vẹn cho người gửi.
- **Tích hợp In Biên lai Giao dịch Chuẩn**:
  - *Đóng góp: Thành viên A (Tuấn)*
  - Tích hợp `ConsoleView::printReceipt()` hiển thị rõ ràng thông tin giao dịch Rút tiền và Chuyển tiền thành công.
- **Tái sử dụng Mã nguồn & Kiểm tra Đổi PIN**:
  - *Đóng góp: Thành viên C (Phát)*
  - Chuyển việc thẩm định mã PIN trong `UserController` sang sử dụng trực tiếp hàm tĩnh `Card::isValidPinFormat`.
  - Bắt buộc kiểm tra kết quả trả về của `card.changePin()` trước khi thông báo thành công.

---

## [1.1.0] - 2026-10-04: Tái cấu trúc, Bảo mật I/O & Củng cố Tính Toàn vẹn (Security Hardening & Refactoring)

Đợt nâng cấp kỹ thuật tập trung vào việc gia cố bảo mật tầng hiển thị terminal, bẫy lỗi luồng nhập xuất và chuẩn hóa mô hình thực thể.

### 🛡️ Bảo mật & Xử lý I/O Terminal (`ConsoleView`) (`a2a4321`)
- **Triệt tiêu Lỗ hổng Chuỗi Thoát ANSI (Escape Sequences)**:
  - *Đóng góp: Thành viên C (Phát)*
  - Xây dựng lớp RAII `LinuxTerminalRawGuard` quản lý bật/tắt chế độ terminal raw mode an toàn theo vòng đời đối tượng.
  - Xây dựng cơ chế phát hiện và drain sạch sẽ các byte escape sequence (phím mũi tên, phím chức năng), ngăn chặn việc làm sai lệch mã PIN hoặc hiển thị thừa dấu `*`.
- **Bẫy Lỗi Số thực và Xử lý Tín hiệu EOF (`inputMoney`)**:
  - *Đóng góp: Thành viên C (Phát) & Thành viên A (Tuấn)*
  - Chuyển sang đọc chuỗi theo dòng bằng `std::getline()`, loại bỏ lỗi `cin >> amount` âm thầm bỏ sót phần thập phân hoặc chuỗi kèm ký tự chữ.
  - Kiểm tra từng ký tự số bằng `std::isdigit()`, từ chối số âm, số 0 và chuỗi không hợp lệ.
  - Bắt tín hiệu đóng luồng `EOF` (`Ctrl+D`), thoát an toàn và ngăn chặn 100% lỗi lặp vô hạn gây treo 100% CPU.
  - Hỗ trợ tiêm luồng đầu vào (`std::istream& inStream`) phục vụ kiểm thử tự động.
- **Mở rộng Giao diện Người dùng Phân hệ User**:
  - *Đóng góp: Thành viên A (Tuấn)*
  - Bổ sung `printUserMenu()` với đầy đủ tùy chọn chức năng.
  - Bổ sung `printReceipt()` in biên lai giao dịch tài chính chuẩn hóa với khung viền ASCII.

### 🔒 Củng cố Mô hình Nghiệp vụ (`Card` & `Account`) (`b55e2b3`)
- **Mô hình Thẻ từ (`Card`)**:
  - *Đóng góp: Thành viên A (Tuấn) & Thành viên C (Phát)*
  - Bổ sung hàm kiểm tra định dạng tĩnh `isValidPinFormat()`: Yêu cầu chính xác 6 ký tự số.
  - `changePin()` trả về kiểu `bool`, từ chối mã PIN sai độ dài hoặc chứa ký tự lạ.
  - Tách bạch rõ ràng theo nguyên lý Đơn trách nhiệm (SRP):
    - `resetFailedAttempts()`: Chỉ đặt lại biến đếm sai về 0 khi đăng nhập thành công.
    - `unlockCard()`: Nghiệp vụ riêng của Admin, mở khóa thẻ và đặt lại số lần sai.
  - `recordFailedAttempt()`: Chặn tăng biến đếm sai khi thẻ đã ở trạng thái khóa.
- **Mô hình Tài khoản (`Account`)**:
  - *Đóng góp: Thành viên A (Tuấn) & Thành viên C (Phát)*
  - `deposit()`: Chặn tham số nạp tiền $\le 0$, bổ sung kiểm tra chống tràn số nguyên `numeric_limits<long>::max()`.
  - `withdraw()`: Tự động kiểm tra ràng buộc `canWithdraw()`, chặn trừ tiền nếu vi phạm quy định tài chính.

### ⚙️ Hiện đại hóa Cấu hình Hệ thống & Kiểm thử (`d750350`, `8eed08e`)
- **Chuẩn hóa Hằng số Hệ thống (`Common.h`)**:
  - *Đóng góp: Thành viên C (Phát)*
  - Chuyển đổi toàn bộ hằng số sang `inline constexpr` và `inline const std::string` (chuẩn C++17) nhằm triệt tiêu việc duplicate dữ liệu tĩnh giữa các Translation Units.
  - Mở rộng tập mã lỗi: Bổ sung `ERR_SYSTEM_OVERFLOW = 10` và `ERR_SAME_ACCOUNT = 9`.
- **Nâng cấp Bộ Kiểm thử Tự động (`test/test_member_c.cpp`)**:
  - *Đóng góp: Thành viên C (Phát)*
  - Thay thế `<cassert>` bằng macro tùy biến `TEST_CHECK` không bị tắt khi biên dịch `-DNDEBUG`.
  - Bổ sung kịch bản kiểm thử stream tự động bằng `std::istringstream` cho `inputMoney()` và `inputPassword()`.

---

## [1.0.0] - 2026-10-03: Khởi tạo Kiến trúc Nền tảng & Phân công Tuần 1 (Foundation & Core Entities)

Phiên bản nền tảng đầu tiên thiết lập khung kiến trúc dự án, mô hình dữ liệu lõi và giao diện điều khiển console.

### 🏛️ Khung Dự án & Tầng Hiển thị Ban đầu
- **Khởi tạo Dự án & Tài liệu Kỹ thuật (`b167601`, `50e0b96`, `9267e28`, `044c9a1`)**:
  - *Đóng góp: Thành viên C (Phát)*
  - Thiết lập cấu trúc thư mục chuẩn (`src/`, `include/`, `data/`, `test/`, `docs/`).
  - Soạn thảo tài liệu đặc tả đồ án, kiến trúc hệ thống 5 tầng, kế hoạch triển khai và tiêu chí chấm điểm.
  - Viết `Makefile` tự động hóa quá trình biên dịch với cờ kiểm tra nghiêm ngặt `-std=c++17 -Wall -Wextra`.
- **Định nghĩa Hệ thống Chung (`include/Common.h`) (`f67d0a7`)**:
  - *Đóng góp: Thành viên C (Phát)*
  - Khai báo các hằng số: `DEFAULT_PIN = "123456"`, `MIN_TRANSACTION = 50000`, `MIN_BALANCE_RESERVE = 50000`, `MAX_FAILED_LOGINS = 3`, `ID_LENGTH = 14`, `PIN_LENGTH = 6`.
  - Khai báo các enum: `TransactionType`, `UserRole`, `ErrorCode`.

### 👥 Mô hình Thực thể & Nghiệp vụ Khách hàng Ban đầu
- **Cài đặt Lớp `Card` và Lớp `Account` (`3eed0d8`)**:
  - *Đóng góp: Thành viên A (Tuấn)*
  - Thực thể `Card`: Lưu trữ `_strId`, `_strPin`, trạng thái khóa `_bIsLocked`, số lần nhập sai `_iFailedAttempts`.
  - Thực thể `Account`: Quản lý số dư `_lBalance` (kiểu long), hàm `canWithdraw()` bẫy 3 ràng buộc tài chính (rút tối thiểu 50k, bội số của 50k, giữ lại ít nhất 50k trong tài khoản).
- **Cài đặt Tầng `ConsoleView` Cơ bản (`a1a1c36`, `e429e2e`)**:
  - *Đóng góp: Thành viên A (Tuấn) & Thành viên C (Phát)*
  - Hỗ trợ màu sắc ANSI trên Linux terminal, hàm `inputPassword()` ẩn ký tự bằng `*` và hỗ trợ xóa lùi bằng phím Backspace.
  - Bẫy lỗi `cin.fail()` khi người dùng nhập sai kiểu dữ liệu.
- **Nghiệp vụ Phân hệ Khách hàng (`UserController`) (`f6c8bde`)**:
  - *Đóng góp: Thành viên A (Tuấn)*
  - Xử lý quy trình đăng nhập khách hàng, cơ chế khóa thẻ khi nhập sai 3 lần liên tiếp trong RAM, ép buộc đổi mã PIN mặc định `123456` ở lần đăng nhập đầu tiên.
  - Điều hướng menu các chức năng xem số dư, rút tiền và chuyển tiền in-memory.
- **Kiểm thử & Hướng dẫn Thực hiện (`15bd3a8`, `7144238`, `fce24f6`)**:
  - *Đóng góp: Thành viên A (Tuấn)*
  - Xây dựng bộ kiểm thử ban đầu `test/test_member_a.cpp` và tài liệu kịch bản demo `DemoScript_User.md`.

---

## 📊 BẢNG TỔNG HỢP ĐÓNG GÓP THEO THÀNH VIÊN (TEAM CONTRIBUTION MATRIX)

| Thành viên | Trách nhiệm chính | Các File & Module chủ đạo | Đóng góp nổi bật |
| :--- | :--- | :--- | :--- |
| **HỨA NHỰT TUẤN**<br>*(Thành viên A - Member A)* | **Business Logic / User Flow & UI** | `include/Card.h`, `src/Card.cpp`<br>`include/Account.h`, `src/Account.cpp`<br>`include/UserController.h`, `src/UserController.cpp`<br>`include/AdminController.h`, `src/AdminController.cpp`<br>`test/test_member_a.cpp`, `test/test_phase_3_a.cpp`<br>`docs/DemoScript_User.md` | - Phát triển toàn bộ logic phân hệ Khách hàng (User).<br>- Tích hợp Persistence cho Rút tiền, Chuyển tiền, Đổi PIN, Lịch sử giao dịch.<br>- Xây dựng kịch bản Demo và bộ test Phase 3 (51 test cases).<br>- Soạn thảo Báo cáo Word Đồ án. |
| **NGÔ TRÍ**<br>*(Thành viên B - Member B)* | **Data Structures / Memory & Storage Flow** | `include/LinkedList.h`<br>`include/Admin.h`, `src/Admin.cpp`<br>`include/Transaction.h`, `src/Transaction.cpp`<br>`include/FileService.h`, `src/FileService.cpp`<br>`test/test_memory_leak.cpp`<br>`scripts/valgrind_check.sh`<br>`docs/04_ctdl_va_thuat_toan.md` | - Cài đặt cấu trúc dữ liệu tự tạo Generic Template `LinkedList<T>` chuẩn $\mathcal{O}(1)$ thêm cuối.<br>- Xây dựng tầng `FileService` đọc/ghi 5 tệp tin vật lý và cơ chế Auto-Recovery.<br>- Xây dựng bộ kiểm định rò rỉ bộ nhớ Memory Audit (67 test cases, 0 bytes leaked, 0 double-free).<br>- Phân tích CTDL và độ phức tạp thuật toán Big-O. |
| **TRẦN VŨ HỎA PHÁT**<br>*(Thành viên C - Member C / Tech Lead)* | **Architecture / Core Controller & Admin Flow** | `include/Common.h`<br>`include/ConsoleView.h`, `src/ConsoleView.cpp`<br>`include/AtmController.h`, `src/AtmController.cpp`<br>`src/main.cpp`, `Makefile`<br>`test/test_member_c.cpp`, `test/test_phase_3.cpp`<br>`docs/03_kien_truc_he_thong.md`, `docs/08_so_do_uml_va_luong_du_lieu.md` | - Thiết kế kiến trúc hệ thống 5 tầng, `Common.h`, Makefile.<br>- Xây dựng bộ điều phối trung tâm `AtmController` và vòng lặp ứng dụng `main.cpp`.<br>- Bảo mật Terminal Raw Mode (lớp RAII, chống treo CPU khi gặp EOF/Escape).<br>- Gia cố an toàn dữ liệu: Atomic File Write cách ly PID, phòng chống Path Traversal, chống Integer Underflow, ASan & UBSan.<br>- Hợp nhất mã nguồn đa nhánh và chuẩn hóa tài liệu kỹ thuật. |
