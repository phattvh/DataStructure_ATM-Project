# 05. THIẾT KẾ CHI TIẾT (DETAILED DESIGN SPECIFICATION)

Tài liệu này quy định chi tiết cấu trúc các lớp (Class), thuộc tính (Attributes), phương thức (Methods), kiểu dữ liệu và quy tắc đặt tên chuẩn theo **C++ Coding Standard Version 2** của tất cả các file mã nguồn trong dự án.

---

## I. MODULE CỐT LÕI: COMMON & CẤU TRÚC DỮ LIỆU

### 1. File `include/Common.h`
Định nghĩa toàn bộ hằng số và kiểu dữ liệu liệt kê (Enum) dùng chung cho toàn bộ dự án (tuân thủ Rule 19: không dùng magic numbers).

* **Header Guard**:
  ```cpp
  #ifndef COMMON_H_INCLUDED_
  #define COMMON_H_INCLUDED_
  ```
* **Hằng số hệ thống (UPPERCASE)**:
  * `const std::string DEFAULT_PIN = "123456";` // Mã PIN mặc định khi tạo mới
  * `const long MIN_TRANSACTION = 50000;` // Số tiền tối thiểu cho 1 giao dịch
  * `const long MIN_BALANCE_RESERVE = 50000;` // Số dư tối thiểu cần duy trì trong tài khoản
  * `const int MAX_FAILED_LOGINS = 3;` // Số lần đăng nhập sai tối đa trước khi khóa thẻ
  * `const int ID_LENGTH = 14;` // Độ dài cố định của mã số thẻ ID
  * `const int PIN_LENGTH = 6;` // Độ dài cố định của mã PIN
  * `const std::string DATA_DIR = "data/";` // Thư mục lưu trữ cơ sở dữ liệu tệp tin
* **Kiểu dữ liệu Enum (PascalCase, giá trị UPPERCASE)**:
  ```cpp
  enum TransactionType {
      WITHDRAW = 1,
      TRANSFER = 2,
      RECEIVE = 3
  };

  enum UserRole {
      ROLE_NONE = 0,
      ROLE_ADMIN = 1,
      ROLE_USER = 2
  };

  enum ErrorCode {
      ERR_NONE = 0,
      ERR_INVALID_AMOUNT = 1,
      ERR_NOT_MULTIPLE = 2,
      ERR_INSUFFICIENT_FUNDS = 3,
      ERR_FILE_NOT_FOUND = 4,
      ERR_CARD_LOCKED = 5,
      ERR_ID_EXISTS = 6,
      ERR_ID_NOT_FOUND = 7,
      ERR_RECIPIENT_NOT_FOUND = ERR_ID_NOT_FOUND,
      ERR_INVALID_FORMAT = 8,
      ERR_SAME_ACCOUNT = 9,
      ERR_SYSTEM_OVERFLOW = 10
  };
  ```

---

### 2. File `include/LinkedList.h`
Cài đặt lớp mẫu Generic Template Cấu trúc dữ liệu danh sách liên kết đơn có con trỏ `_pTail`.

* **Cấu trúc `Node<T>`**:
  * `T _data`: Biến lưu trữ giá trị dữ liệu.
  * `Node<T>* _pNext`: Con trỏ trỏ tới node tiếp theo.
* **Lớp `LinkedList<T>`**:
  * `Node<T>* _pHead`: Con trỏ đầu danh sách.
  * `Node<T>* _pTail`: Con trỏ cuối danh sách.
  * `int _iSize`: Số lượng phần tử hiện tại.
* **Các phương thức chính**:
  * `void addTail(const T& item)`: Thêm một phần tử vào cuối danh sách ($\mathcal{O}(1)$).
  * `template <typename Predicate> bool removeIf(Predicate pred)`: Xóa phần tử đầu tiên thỏa mãn điều kiện `pred` và giải phóng bộ nhớ node đó.
  * `template <typename Predicate> T* findIf(Predicate pred)`: Tìm kiếm phần tử thỏa mãn điều kiện và trả về con trỏ tới dữ liệu bên trong node.
  * `void clear()`: Duyệt và `delete` toàn bộ các node trong danh sách.
  * `int getSize() const`: Trả về số lượng phần tử.
  * `bool isEmpty() const`: Kiểm tra danh sách rỗng.
  * `Node<T>* getHead() const`: Lấy con trỏ đầu danh sách phục vụ duyệt.

---

## II. MODULE THỰC THỂ (DOMAIN MODELS)

Mỗi thực thể được tách bạch hoàn toàn thành tệp khai báo `.h` và tệp hiện thực `.cpp` (Rule 25). Toàn bộ thuộc tính có tiền tố `_` kết hợp chuẩn Hungarian Notation.

### 1. Lớp `Admin` (`include/Admin.h`, `src/Admin.cpp`)
* **Thuộc tính**:
  * `std::string _strUsername`: Tên đăng nhập của quản trị viên.
  * `std::string _strPassword`: Mật khẩu quản trị.
* **Phương thức**:
  * `Admin()`: Constructor mặc định.
  * `Admin(const std::string& strUser, const std::string& strPass)`: Constructor khởi tạo.
  * `std::string getUsername() const`: Lấy tên đăng nhập.
  * `bool verifyPassword(const std::string& strPass) const`: Đối soát mật khẩu.

---

### 2. Lớp `Card` (`include/Card.h`, `src/Card.cpp`)
* **Thuộc tính**:
  * `std::string _strId`: Chuỗi đúng 14 ký tự số định danh thẻ.
  * `std::string _strPin`: Chuỗi 6 chữ số mã PIN.
  * `int _iFailedAttempts`: Đếm số lần nhập sai PIN liên tiếp.
  * `bool _bIsLocked`: Cờ đánh dấu trạng thái thẻ có bị khóa hay không.
* **Phương thức**:
  * `Card()`: Constructor mặc định.
  * `Card(const std::string& strId, const std::string& strPin, bool bIsLocked = false)`: Constructor khởi tạo.
  * `std::string getId() const`: Lấy ID thẻ.
  * `std::string getPin() const`: Lấy mã PIN hiện tại.
  * `bool isLocked() const`: Kiểm tra thẻ có đang bị khóa không.
  * `bool isDefaultPin() const`: Kiểm tra thẻ có đang mang mã PIN mặc định `123456` hay không.
  * `bool checkPin(const std::string& strInputPin) const`: So khớp mã PIN nhập vào.
  * `void recordFailedAttempt()`: Tăng số lần nhập sai. Nếu `_iFailedAttempts >= MAX_FAILED_LOGINS`, tự động đổi `_bIsLocked = true`.
  * `void resetFailedAttempts()`: Đặt lại `_iFailedAttempts = 0` và mở khóa `_bIsLocked = false`.
  * `void changePin(const std::string& strNewPin)`: Cập nhật mã PIN mới cho thẻ.

---

### 3. Lớp `Account` (`include/Account.h`, `src/Account.cpp`)
* **Thuộc tính**:
  * `std::string _strId`: Mã số tài khoản (14 số).
  * `std::string _strName`: Họ và tên chủ tài khoản (cho phép chứa khoảng trắng).
  * `long _lBalance`: Số dư tài khoản (dùng kiểu số nguyên `long` để loại bỏ hoàn toàn sai số dấu phẩy động của `double/float`).
  * `std::string _strCurrency`: Loại tiền tệ (mặc định: `VND`).
* **Phương thức**:
  * `Account()`: Constructor mặc định.
  * `Account(const std::string& strId, const std::string& strName, long lBalance, const std::string& strCurrency = "VND")`: Constructor khởi tạo.
  * `std::string getId() const`, `std::string getName() const`, `long getBalance() const`, `std::string getCurrency() const`: Các hàm lấy thông tin.
  * `ErrorCode canWithdraw(long lAmount) const`: Kiểm tra các điều kiện:
    * `lAmount < MIN_TRANSACTION` $\rightarrow$ trả về `ERR_INVALID_AMOUNT`.
    * `lAmount % MIN_TRANSACTION != 0` $\rightarrow$ trả về `ERR_NOT_MULTIPLE`.
    * `(this->_lBalance - lAmount) < MIN_BALANCE_RESERVE` $\rightarrow$ trả về `ERR_INSUFFICIENT_FUNDS`.
    * Thỏa mãn tất cả $\rightarrow$ trả về `ERR_NONE`.
  * `void withdraw(long lAmount)`: Trừ số tiền khỏi số dư `this->_lBalance -= lAmount;`.
  * `void deposit(long lAmount)`: Cộng số tiền vào số dư `this->_lBalance += lAmount;`.

---

### 4. Lớp `Transaction` (`include/Transaction.h`, `src/Transaction.cpp`)
* **Thuộc tính**:
  * `std::string _strId`: ID của tài khoản thực hiện.
  * `TransactionType _eType`: Loại giao dịch (`WITHDRAW`, `TRANSFER`, `RECEIVE`).
  * `long _lAmount`: Số tiền thực hiện giao dịch.
  * `std::string _strTimestamp`: Chuỗi thời gian giao dịch (`YYYY-MM-DD HH:MM:SS`).
  * `std::string _strDetail`: Mô tả chi tiết giao dịch (ví dụ: chuyển tiền cho ai, nhận từ ai).
* **Phương thức**:
  * `std::string formatForFile() const`: Xuất ra chuỗi chuẩn để ghi nối vào tệp `[LichSuID].txt`.
  * `std::string toString() const`: Xuất chuỗi định dạng đẹp mắt để hiển thị lên bảng console.

---

## III. TẦNG DỊCH VỤ DỮ LIỆU: `FileService`

Toàn bộ các hàm trong `FileService` được khai báo dạng `static`. Đảm bảo mở tệp là phải đóng tệp (`file.close()`) theo đúng Rule 18.

* **Các phương thức thao tác tệp tin**:
  * `static bool loadAdmins(LinkedList<Admin>& listAdmins)`: Đọc file `data/Admin.txt`.
  * `static bool loadCards(LinkedList<Card>& listCards, const LinkedList<std::string>& listLockedIds)`: Đọc `data/TheTu.txt`, đối chiếu với danh sách bị khóa để gán cờ `_bIsLocked`.
  * `static bool loadLockedIds(LinkedList<std::string>& listLockedIds)`: Đọc file `data/KhoaThe.txt`.
  * `static bool saveCards(const LinkedList<Card>& listCards)`: Ghi đè lại toàn bộ thẻ vào `data/TheTu.txt` khi thêm/xóa/đổi PIN.
  * `static bool saveLockedIds(const LinkedList<std::string>& listLockedIds)`: Ghi đè danh sách các ID bị khóa vào `data/KhoaThe.txt`.
  * `static ErrorCode loadAccount(const std::string& strId, Account& acc)`: Đọc file `data/[ID].txt` bằng `std::getline()`.
  * `static bool saveAccount(const Account& acc)`: Ghi đè 4 dòng thông tin vào `data/[ID].txt`.
  * `static bool deleteAccountFile(const std::string& strId)`: Xóa file `data/[ID].txt` (dùng hàm hệ thống `std::remove()`).
  * `static bool appendTransaction(const std::string& strId, const Transaction& trans)`: Mở `data/LichSu[ID].txt` ở chế độ `std::ios::app` để ghi nối lịch sử giao dịch.
  * `static bool loadTransactions(const std::string& strId, LinkedList<Transaction>& listTrans)`: Đọc toàn bộ lịch sử từ `data/LichSu[ID].txt`.
  * `static bool createAccountFiles(const std::string& strId, const std::string& strName, long lInitialBalance, const std::string& strCurrency)`: Tự động khởi tạo cả 2 file `[ID].txt` và `LichSu[ID].txt` khi Admin thêm thẻ.

---

## IV. TẦNG GIAO DIỆN CONSOLE: `ConsoleView`

* **Mã màu chuẩn ANSI**:
  ```cpp
  #define COLOR_RESET   "\033[0m"
  #define COLOR_RED     "\033[31m"      // Báo lỗi
  #define COLOR_GREEN   "\033[32m"      // Thành công
  #define COLOR_YELLOW  "\033[33m"      // Cảnh báo, PIN
  #define COLOR_BLUE    "\033[34m"      // Viền khung
  #define COLOR_CYAN    "\033[36m"      // Tiêu đề, thông tin
  #define COLOR_BOLD    "\033[1m"
  ```
* **Các hàm hiển thị và bắt phím**:
  * `static std::string inputPassword(const std::string& strPrompt)`: Ẩn phím thành ký tự `*`, hỗ trợ phím xóa lùi `\b` (Backspace) và phím `Enter`.
  * `static long inputMoney(const std::string& strPrompt)`: Bắt lỗi `std::cin.fail()` chống sập chương trình khi nhập chữ.
  * `static void printHeader(const std::string& strTitle)`: Vẽ khung tiêu đề ASCII nổi bật.
  * `static void printError(const std::string& strMsg)`: In thông báo lỗi màu đỏ kèm âm báo hoặc viền.
  * `static void printSuccess(const std::string& strMsg)`: In thông báo hoàn thành màu xanh lá.
  * `static void printAdminMenu()`: Vẽ menu quản trị viên 5 chức năng theo mẫu đề bài.
  * `static void printUserMenu()`: Vẽ menu khách hàng 6 chức năng theo mẫu đề bài.
  * `static void pauseScreen()`: Dừng màn hình chờ người dùng ấn Enter để xem kết quả.

---

## V. TẦNG ĐIỀU PHỐI: `AtmController`

Quản lý trạng thái và luồng điều khiển toàn cục:
* **Thuộc tính phiên làm việc**:
  * `LinkedList<Admin> _listAdmins;`
  * `LinkedList<Card> _listCards;`
  * `LinkedList<std::string> _listLockedIds;`
  * `Account* _pCurrentAccount;` // Con trỏ tài khoản người dùng đăng nhập hiện tại
  * `Card* _pCurrentCard;` // Con trỏ thẻ từ đang sử dụng
  * `UserRole _eCurrentRole;` // Quyền hạn hiện tại
* **Phương thức điều khiển**:
  * `void run()`: Khởi tạo dữ liệu từ file, chạy vòng lặp menu chính.
  * `void processAdminLogin()`: Xử lý đăng nhập Admin.
  * `void processAdminMenu()`: Điều hướng 4 chức năng Admin (Xem DS, Thêm thẻ, Xóa thẻ, Mở khóa).
  * `void processUserLogin()`: Xử lý đăng nhập User, đếm sai 3 lần để khóa, chặn thẻ bị khóa, bắt buộc đổi PIN mặc định.
  * `void processUserMenu()`: Điều hướng 5 chức năng User.
  * `void processWithdraw()`: Luồng rút tiền.
  * `void processTransfer()`: Luồng chuyển tiền đảm bảo tính nguyên tử (Atomicity).
  * `void processChangePin()`: Luồng đổi PIN với 2 lần xác nhận.
  * `void cleanupSession()`: Giải phóng vùng nhớ con trỏ `_pCurrentAccount` khi đăng xuất.
