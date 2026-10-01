# 03. KIẾN TRÚC HỆ THỐNG (SYSTEM ARCHITECTURE)

Tài liệu này mô tả kiến trúc tổng thể, mô hình phân tầng, luồng xử lý dữ liệu và thiết kế thư mục của hệ thống mô phỏng máy ATM bằng C++.

---

## I. MÔ HÌNH KIẾN TRÚC PHÂN TẦNG (LAYERED ARCHITECTURE)

Hệ thống được thiết kế theo mô hình kiến trúc phân tầng (Layered Architecture) nhằm đảm bảo tính độc lập giữa Giao diện (UI), Nghiệp vụ (Business Logic) và Truy xuất Tệp tin (Data Access).

```mermaid
graph TD
    subgraph Presentation_Layer["1. TẦNG GIAO DIỆN (Presentation Layer)"]
        CV[ConsoleView<br>• Hiển thị menu ANSI Color<br>• Ẩn mã PIN thành dấu *<br>• Bẫy lỗi nhập số cin.fail]
    end

    subgraph Controller_Layer["2. TẦNG ĐIỀU PHỐI (Controller Layer)"]
        AC[AtmController / AtmSystem<br>• Điều khiển vòng lặp chính<br>• Xác thực quyền Admin/User<br>• Quản lý Session & Giao dịch]
    end

    subgraph Domain_Layer["3. TẦNG THỰC THỂ (Domain Model Layer)"]
        M1[Admin]
        M2[Card]
        M3[Account]
        M4[Transaction]
    end

    subgraph Storage_Layer["4. TẦNG DỊCH VỤ DỮ LIỆU (Data Service Layer)"]
        FS[FileService<br>• Đọc / Ghi các tệp .txt<br>• Đảm bảo Rule 18: Open/Close]
    end

    subgraph Foundation_Layer["5. TẦNG CẤU TRÚC DỮ LIỆU (Data Structure Layer)"]
        LL[Template LinkedList&lt;T&gt;<br>• Quản lý danh sách trong RAM<br>• Quản lý cấp phát & thu hồi động]
    end

    subgraph File_System["6. HỆ THỐNG TỆP TIN (Disk Storage)"]
        F1[(Admin.txt)]
        F2[(TheTu.txt)]
        F3[(KhoaThe.txt)]
        F4[([ID].txt)]
        F5[([LichSuID].txt)]
    end

    CV <-->|Nhập/Xuất giao diện| AC
    AC -->|Thao tác nghiệp vụ| Domain_Layer
    AC -->|Quản lý bộ nhớ RAM| Foundation_Layer
    AC -->|Yêu cầu lưu trữ| FS
    FS <-->|Đọc / Ghi tệp| File_System
```

---

## II. CHI TIẾT CÁC TẦNG CHỨC NĂNG

### 1. Tầng Giao diện (Presentation Layer - `ConsoleView`)
* **Nhiệm vụ**: Đảm nhận toàn bộ tương tác nhập/xuất trên màn hình console.
* **Đặc điểm**:
  * Không chứa logic tính toán tiền bạc hay kiểm tra nghiệp vụ.
  * Hỗ trợ bắt phím thời gian thực bằng `_getch()` (trên Windows) hoặc `termios` (trên Linux) để in dấu `*` bảo mật.
  * Tích hợp mã màu ANSI chuẩn (đỏ cho lỗi, xanh lá cho thành công, vàng cho cảnh báo, cyan cho bảng biểu).
  * Bộ lọc `cin.fail()` ngăn chặn triệt để lỗi treo màn hình khi người dùng gõ chữ vào trường nhập số.

### 2. Tầng Điều phối (Controller Layer - `AtmController`)
* **Nhiệm vụ**: Trung tâm điều khiển toàn bộ luồng xử lý của hệ thống.
* **Đặc điểm**:
  * Khởi tạo và nắm giữ các danh sách `LinkedList<Admin>`, `LinkedList<Card>`, `LinkedList<string>` (danh sách ID bị khóa) trong bộ nhớ RAM.
  * Điều phối quá trình đăng nhập, kiểm tra số lần sai để khóa thẻ.
  * Thực thi các giao dịch tài chính (Rút tiền, Chuyển tiền) đảm bảo tính nguyên tử (Atomicity).

### 3. Tầng Thực thể (Domain Model Layer)
* **Nhiệm vụ**: Đại diện cho các đối tượng nghiệp vụ cốt lõi trong hệ thống:
  * `Admin`: Lưu thông tin đăng nhập quản trị viên.
  * `Card`: Lưu mã số thẻ (14 số), mã PIN (6 số), trạng thái khóa, số lần nhập sai.
  * `Account`: Lưu ID, họ tên chủ thẻ, số dư hiện tại, loại tiền tệ.
  * `Transaction`: Lưu vết chi tiết một giao dịch (loại, số tiền, mốc thời gian, mô tả).
* **Đặc điểm**: Tuân thủ chuẩn OOP đóng gói (Encapsulation), toàn bộ thuộc tính là `private` có tiền tố `_`, truy xuất thông qua các hàm Getter/Setter có từ khóa `const`.

### 4. Tầng Dịch vụ Tệp tin (Data Service Layer - `FileService`)
* **Nhiệm vụ**: Đảm bảo việc đọc và ghi dữ liệu ra hệ thống tệp tin vật lý trên đĩa.
* **Đặc điểm**:
  * Toàn bộ phương thức là `static`.
  * Tuyệt đối không mở file ở một hàm mà đóng file ở hàm khác (tuân thủ Rule 18 của Coding Standard).
  * Truyền danh sách `LinkedList<T>&` qua tham chiếu để tránh rò rỉ bộ nhớ hoặc lỗi copy nông.

### 5. Tầng Cấu trúc Dữ liệu (Foundation Layer - `LinkedList<T>`)
* **Nhiệm vụ**: Cung cấp cấu trúc danh sách liên kết tự tạo dạng Template đa năng.
* **Đặc điểm**:
  * Tự quản lý cấp phát động bằng `new` và giải phóng toàn bộ node bằng `delete` trong Destructor (tuân thủ Rule 17).
  * Thay thế hoàn toàn cho `std::vector` hoặc `std::list` nhằm đạt trọn 2.0 điểm tiêu chí Cấu trúc dữ liệu & Template.

---

## III. CƠ CHẾ BẢO VỆ & XỬ LÝ NGOẠI LỆ

1. **Bảo vệ Luồng Nhập liệu (Input Stream Protection)**:
   * Mọi hàm nhập số (Số tiền, Lựa chọn Menu) đều được bọc trong vòng lặp kiểm tra trạng thái luồng nhập. Nếu xảy ra lỗi ép kiểu, lập tức gọi `cin.clear()` và `cin.ignore()` để dọn sạch bộ đệm.
2. **Cô lập Phiên làm việc (Session Isolation)**:
   * Dữ liệu tài khoản của người dùng chỉ được tải vào con trỏ `Account* _pCurrentAccount` đúng thời điểm đăng nhập thành công. Khi chọn Thoát hoặc giao dịch bị hủy, con trỏ được giải phóng (`delete`) và gán về `nullptr`.
3. **Tự động Phục hồi Cấu trúc Tệp (Auto-Recovery)**:
   * Khi khởi động, nếu hệ thống không tìm thấy thư mục `data/` hoặc các tệp tin cơ sở (`Admin.txt`, `TheTu.txt`), chương trình không bị crash mà sẽ tự động tạo thư mục và sinh dữ liệu mẫu ban đầu để tiếp tục vận hành.

---

## IV. CẤU TRÚC THƯ MỤC NGUỒN (DIRECTORY STRUCTURE)

```
DataStructure_ATM-Project/
├── Makefile                          # Script biên dịch (g++ -std=c++17 -Wall -Wextra)
├── README.md                         # Giới thiệu tổng quan repository
├── data/                             # Thư mục chứa các tệp cơ sở dữ liệu
│   ├── Admin.txt                     # Danh sách Admin (>= 3 tài khoản)
│   ├── TheTu.txt                     # Danh sách thẻ từ (>= 10 thẻ)
│   ├── KhoaThe.txt                   # Danh sách ID thẻ đang bị khóa
│   ├── 10014504500001.txt            # Thông tin tài khoản chi tiết
│   ├── LichSu10014504500001.txt      # Lịch sử giao dịch chi tiết
│   └── ...
├── docs/                             # Tài liệu kỹ thuật chi tiết của dự án
│   ├── 01_tieu_chi_cham.md
│   ├── 02_yeu_cau_va_pham_vi.md
│   ├── 03_kien_truc_he_thong.md
│   ├── 04_ctdl_va_thuat_toan.md
│   ├── 05_thiet_ke_chi_tiet.md
│   ├── 06_ke_hoach_trien_khai.md
│   └── 07_ke_hoach_kiem_thu.md
├── include/                          # Tệp tin Header (*.h)
│   ├── Common.h                      # Hằng số, Enum, ErrorCode
│   ├── LinkedList.h                  # Template class Cấu trúc dữ liệu LinkedList<T>
│   ├── Admin.h                       # Khai báo lớp Admin
│   ├── Card.h                        # Khai báo lớp Card
│   ├── Account.h                     # Khai báo lớp Account
│   ├── Transaction.h                 # Khai báo lớp Transaction
│   ├── FileService.h                 # Khai báo lớp FileService
│   ├── ConsoleView.h                 # Khai báo lớp ConsoleView
│   └── AtmController.h               # Khai báo lớp AtmController
├── src/                              # Tệp tin Hiện thực (*.cpp)
│   ├── Admin.cpp
│   ├── Card.cpp
│   ├── Account.cpp
│   ├── Transaction.cpp
│   ├── FileService.cpp
│   ├── ConsoleView.cpp
│   ├── AtmController.cpp
│   └── main.cpp                      # Điểm vào chính của ứng dụng
└── test/                             # Kiểm thử tự động & Unit Test
    └── test_atm.cpp                  # Test suite cho LinkedList và Business Logic
```

---

## V. ĐỊNH HƯỚNG MỞ RỘNG (EXTENSIBILITY)

1. **Sẵn sàng Đa tiền tệ (Multi-currency Ready)**:
   * Giữ nguyên thuộc tính `_strCurrency` trong class `Account`. Trong tương lai có thể bổ sung module `ExchangeRateService` để tự động quy đổi ngoại tệ khi rút hoặc chuyển tiền liên tiền tệ.
2. **Nhật ký Quản trị (Admin Audit Logging)**:
   * Mở rộng thêm phương thức `appendAdminAuditLog()` trong `FileService` để ghi nhận các hành vi nhạy cảm của Admin (như tạo thẻ mới, xóa thẻ, mở khóa) vào tệp `AdminLog.txt`.
3. **Trừu tượng hóa Định dạng Tệp tin (Format Abstraction)**:
   * Tách logic đọc từng dòng thành các hàm phân tích cú pháp (Parser) riêng biệt. Khi cần chuyển đổi từ định dạng `.txt` sang `.json` hoặc `.csv`, chỉ cần thay đổi parser mà không ảnh hưởng đến tầng Controller.
