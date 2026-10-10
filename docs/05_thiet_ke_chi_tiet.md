# 05. THIẾT KẾ CHI TIẾT HỆ THỐNG (DETAILED SYSTEM DESIGN)

Tài liệu này quy định chi tiết toàn bộ cấu trúc các lớp (Class), cấu trúc dữ liệu (Struct), thuộc tính (Attributes), phương thức (Methods), kiểu dữ liệu liệt kê (Enums), hằng số hệ thống, và các thuật toán cốt lõi của phần mềm Mô phỏng Máy ATM. Toàn bộ thiết kế tuân thủ nghiêm ngặt **C++ Coding Standard Version 2** (HCMUE), đảm bảo an toàn bộ nhớ RAII 100% (0 byte leak), cơ chế ghi nguyên tử (Atomic Write), và kiến trúc phân tầng MVC mở rộng.

---

## 5.1. THIẾT KẾ HẰNG SỐ HỆ THỐNG, MÃ LỖI VÀ CẤU HÌNH ĐA TIỀN TỆ (`include/Common.h`)

File `include/Common.h` đóng vai trò là từ điển dữ liệu trung tâm của toàn bộ dự án, định nghĩa các hằng số không đổi, cấu hình tiền tệ, và danh mục mã lỗi tài chính.

### 5.1.1. Các hằng số hệ thống
Tuân thủ Rule 4 (Tên hằng số viết hoa toàn bộ - UPPERCASE) và Rule 19 (Không sử dụng magic numbers trong mã nguồn):
- `DEFAULT_PIN`: Chuỗi `"123456"`, mã PIN mặc định khi Admin tạo tài khoản thẻ mới.
- `MIN_TRANSACTION`: `50,000` (long), hạn mức giao dịch tối thiểu cho đơn vị VND.
- `MIN_BALANCE_RESERVE`: `50,000` (long), số dư tối thiểu bắt buộc duy trì trong tài khoản VND.
- `MAX_FAILED_LOGINS`: `3` (int), số lần nhập sai mã PIN tối đa trước khi khóa thẻ tự động.
- `ID_LENGTH`: `14` (int), độ dài cố định của mã định danh thẻ từ.
- `PIN_LENGTH`: `6` (int), độ dài cố định của mã PIN bảo mật.
- `DATA_DIR`: Chuỗi `"data/"`, đường dẫn thư mục cơ sở dữ liệu tệp tin.
- `MAX_BALANCE_DEFAULT`: `10,000,000,000` (10 tỷ VND), giới hạn số dư tối đa mặc định nhằm chống tràn số signed integer.

### 5.1.2. Cấu trúc cấu hình tiền tệ (`CurrencyConfig`)
Cho phép hệ thống mở rộng hỗ trợ nhiều loại tiền tệ mà không cần sửa đổi logic cốt lõi:
```cpp
struct CurrencyConfig {
    std::string strCode;            // Mã tiền tệ: VND, USD, EUR, JPY, GBP, SGD
    std::string strName;            // Tên gọi đầy đủ của đồng tiền
    long lMinTransaction;           // Hạn mức giao dịch tối thiểu và bội số bắt buộc
    long lMinReserve;               // Số dư tối thiểu cần duy trì trong tài khoản
    long lMaxBalance;               // Số dư tối đa cho phép lưu trữ
    long lExchangeRateToVND;        // Tỷ giá quy đổi tham chiếu sang VND tại ATM
};
```

Bảng tham số cấu hình quy chuẩn cho các đồng tiền:
| Mã tiền tệ | Tên gọi | Min Transaction | Min Reserve | Max Balance | Tỷ giá sang VND |
|:---:|:---|:---:|:---:|:---:|:---:|
| **VND** | Vietnamese Dong | 50,000 VND | 50,000 VND | 10,000,000,000 VND | 1 |
| **USD** | US Dollar | 10 USD | 10 USD | 500,000 USD | 25,400 |
| **EUR** | Euro | 10 EUR | 10 EUR | 500,000 EUR | 27,500 |
| **JPY** | Japanese Yen | 1,000 JPY | 1,000 JPY | 50,000,000 JPY | 170 |
| **GBP** | British Pound | 10 GBP | 10 GBP | 400,000 GBP | 32,800 |
| **Khác** | Tùy biến | 10 Đơn vị | 10 Đơn vị | 1,000,000 Đơn vị | 25,000 |

### 5.1.3. Kiểu dữ liệu liệt kê (Enums)
- **`TransactionType`**: Xác định bản chất giao dịch tài chính:
  - `WITHDRAW = 1`: Rút tiền mặt tại cây ATM.
  - `TRANSFER = 2`: Chuyển tiền đến tài khoản thụ hưởng.
  - `RECEIVE = 3`: Nhận tiền từ tài khoản khác chuyển đến.
- **`UserRole`**: Định danh vai trò đăng nhập của phiên làm việc:
  - `ROLE_NONE = 0`: Chưa đăng nhập / Phiên kết thúc.
  - `ROLE_ADMIN = 1`: Quản trị viên hệ thống.
  - `ROLE_USER = 2`: Khách hàng sử dụng thẻ ATM.
- **`ErrorCode`**: Bảng mã lỗi tiêu chuẩn của hệ thống:
  - `ERR_NONE = 0`: Thao tác thành công hoàn toàn.
  - `ERR_INVALID_AMOUNT = 1`: Số tiền không hợp lệ (nhỏ hơn hạn mức tối thiểu hoặc <= 0).
  - `ERR_NOT_MULTIPLE = 2`: Số tiền không phải là bội số của mệnh giá quy định.
  - `ERR_INSUFFICIENT_FUNDS = 3`: Số dư không đủ hoặc vi phạm hạn mức duy trì tối thiểu.
  - `ERR_FILE_NOT_FOUND = 4`: Lỗi I/O không tìm thấy tệp tin hoặc ghi tệp thất bại.
  - `ERR_CARD_LOCKED = 5`: Thẻ đang ở trạng thái bị khóa.
  - `ERR_ID_EXISTS = 6`: Mã định danh tài khoản đã tồn tại trên hệ thống.
  - `ERR_ID_NOT_FOUND = 7` (hoặc `ERR_RECIPIENT_NOT_FOUND`): Không tìm thấy tài khoản.
  - `ERR_INVALID_FORMAT = 8`: Sai định dạng (độ dài, ký tự chữ, hoặc khác loại tiền tệ).
  - `ERR_SAME_ACCOUNT = 9`: Tự chuyển tiền cho chính số tài khoản của mình.
  - `ERR_SYSTEM_OVERFLOW = 10`: Nguy cơ tràn số dư tài khoản người nhận.

### 5.1.4. Hàm tiện ích thời gian thực
- `inline std::string getNowTimestamp()`: Trả về chuỗi thời gian hiện tại chuẩn định dạng `YYYY-MM-DD HH:MM:SS` sử dụng thư viện `<chrono>` và `<ctime>`, an toàn thread-safe trên cả Windows (`localtime_s`) và Linux/POSIX (`localtime_r`).

---

## 5.2. THIẾT KẾ CẤU TRÚC DỮ LIỆU GENERIC `LinkedList<T>` (`include/LinkedList.h`)

Để thỏa mãn yêu cầu bắt buộc của đồ án môn Cấu trúc dữ liệu và Giải thuật, toàn bộ các tập hợp trong chương trình không sử dụng thư viện STL (`std::vector`, `std::list`), mà được xây dựng từ đầu bằng cấu trúc Danh sách liên kết đơn Generic Template có quản lý cả con trỏ đầu `_pHead` và con trỏ cuối `_pTail`.

### 5.2.1. Cấu trúc `Node<T>`
```cpp
template <typename T>
struct Node {
    T _data;
    Node<T>* _pNext;
    Node(const T& data) : _data(data), _pNext(nullptr) {}
};
```

### 5.2.2. Lớp `LinkedList<T>`
- **Thuộc tính riêng tư (Private Attributes)**:
  - `Node<T>* _pHead`: Con trỏ trỏ tới phần tử đầu tiên của danh sách (khởi tạo `nullptr`).
  - `Node<T>* _pTail`: Con trỏ trỏ tới phần tử cuối cùng của danh sách (khởi tạo `nullptr`).
  - `int _iSize`: Biến nguyên lưu trữ số lượng phần tử hiện tại (khởi tạo `0`).
- **Nguyên tắc Quản lý Bộ nhớ (Memory Management - Rule of Five)**:
  - *Vô hiệu hóa sao chép*: Để triệt tiêu hoàn toàn lỗi Double Free (hai danh sách cùng trỏ vào một vùng nhớ node trên Heap), Copy Constructor và Copy Assignment Operator bị xóa bỏ hoàn toàn:
    ```cpp
    LinkedList(const LinkedList<T>&) = delete;
    LinkedList<T>& operator=(const LinkedList<T>&) = delete;
    ```
  - *Destructor*: Tự động thu hồi 100% các node thông qua phương thức `clear()`.
- **Bảng phương thức và Độ phức tạp thuật toán (Time Complexity)**:

| Tên phương thức | Kiểu trả về | Tham số | Mô tả chức năng | Độ phức tạp thời gian |
|:---|:---:|:---|:---|:---:|
| `LinkedList()` | N/A | Không | Khởi tạo danh sách rỗng (`_pHead=_pTail=nullptr, _iSize=0`). | O(1) |
| `~LinkedList()` | N/A | Không | Giải phóng toàn bộ node trên heap khi danh sách hủy. | O(N) |
| `bool isEmpty() const` | `bool` | Không | Kiểm tra danh sách rỗng (`_pHead == nullptr`). | O(1) |
| `int getSize() const` | `int` | Không | Trả về số lượng node hiện tại (`_iSize`). | O(1) |
| `Node<T>* getHead() const` | `Node<T>*` | Không | Lấy con trỏ phần tử đầu danh sách. | O(1) |
| `Node<T>* getTail() const` | `Node<T>*` | Không | Lấy con trỏ phần tử cuối danh sách. | O(1) |
| `void addTail(const T& item)` | `void` | `const T& item` | Cấp phát node mới, gán vào sau `_pTail`, cập nhật `_pTail`. | O(1) |
| `findIf(Predicate pred)` | `T*` | `Predicate pred` | Duyệt tuần tự từ `_pHead`, trả về con trỏ tới phần tử đầu tiên thỏa `pred(data) == true`. | O(N) |
| `removeIf(Predicate pred)` | `bool` | `Predicate pred` | Tìm kiếm và ngắt kết nối node thỏa mãn, cập nhật `_pHead/_pTail`, gọi `delete` node, giảm `_iSize`. | O(N) |
| `void clear()` | `void` | Không | Duyệt và `delete` từng node, gán lại `_pHead=_pTail=nullptr, _iSize=0`. | O(N) |

---

## 5.3. THIẾT KẾ THỰC THỂ THẺ TỪ (`include/Card.h`, `src/Card.cpp`)

Quản lý thông tin định danh và bảo mật vật lý của thẻ ngân hàng.

### 5.3.1. Thuộc tính (Attributes)
- `std::string _strId`: Chuỗi đúng 14 chữ số định danh duy nhất của thẻ từ.
- `std::string _strPin`: Chuỗi mã PIN (dạng plaintext 6 số hoặc chuỗi băm 32 ký tự hex).
- `int _iFailedAttempts`: Biến đếm số lần nhập sai mã PIN liên tiếp trong phiên hoặc từ tệp tin.
- `bool _bIsLocked`: Cờ logic đánh dấu trạng thái khóa (`true`: bị khóa; `false`: hoạt động).

### 5.3.2. Phương thức (Methods)
- `Card()`: Constructor mặc định (`_strId="", _strPin="", _iFailedAttempts=0, _bIsLocked=false`).
- `Card(const std::string& strId, const std::string& strPin, bool bIsLocked = false)`: Constructor khởi tạo đầy đủ.
- `std::string getId() const`: Trả về mã số thẻ.
- `std::string getPin() const`: Trả về mã PIN.
- `bool isLocked() const`: Trả về trạng thái khóa của thẻ.
- `void setLocked(bool bLocked)`: Thiết lập cờ khóa.
- `int getFailedAttempts() const`: Lấy số lần nhập sai PIN hiện tại.
- `void setFailedAttempts(int iAttempts)`: Thiết lập số lần nhập sai PIN từ dữ liệu tệp tin.
- `bool isDefaultPin() const`: Kiểm tra mã PIN có trùng với mã mặc định `"123456"` hay không (thông qua `SecurityService::verifyHash`).
- `bool checkPin(const std::string& strInputPin) const`: Xác thực mã PIN nhập vào bằng cơ chế kiểm tra đa tầng (so khớp trực tiếp hoặc băm MD5 kèm salt).
- `void recordFailedAttempt()`: Tăng `_iFailedAttempts++`. Nếu `_iFailedAttempts >= MAX_FAILED_LOGINS`, tự động gán `_bIsLocked = true`.
- `void resetFailedAttempts()`: Đặt lại `_iFailedAttempts = 0` và `_bIsLocked = false`.
- `void changePin(const std::string& strNewPin)`: Cập nhật mã PIN mới (được băm bảo mật).

---

## 5.4. THIẾT KẾ THỰC THỂ TÀI KHOẢN NGÂN HÀNG (`include/Account.h`, `src/Account.cpp`)

Đại diện cho tài khoản thanh toán và các nghiệp vụ kiểm soát số dư của khách hàng.

### 5.4.1. Thuộc tính (Attributes)
- `std::string _strId`: Mã số tài khoản (14 chữ số, tương ứng với thẻ từ).
- `std::string _strName`: Họ và tên chủ tài khoản (hỗ trợ chuỗi có khoảng trắng).
- `long _lBalance`: Số dư khả dụng của tài khoản (sử dụng kiểu số nguyên `long` để loại bỏ hoàn toàn sai số làm tròn số thực của `double/float`).
- `std::string _strCurrency`: Đơn vị tiền tệ (mặc định: `"VND"`, hoặc `"USD"`, `"EUR"`, `"JPY"`, `"GBP"`, `"SGD"`).

### 5.4.2. Phương thức (Methods)
- `Account()`: Constructor mặc định.
- `Account(const std::string& strId, const std::string& strName, long lBalance, const std::string& strCurrency = "VND")`: Constructor có tham số.
- `std::string getId() const`: Lấy mã số tài khoản.
- `std::string getName() const`: Lấy tên chủ tài khoản.
- `long getBalance() const`: Lấy số dư hiện tại.
- `std::string getCurrency() const`: Lấy đơn vị tiền tệ.
- `void setBalance(long lBalance)`: Cập nhật số dư (dùng khi đồng bộ hóa từ đĩa).
- `ErrorCode canWithdraw(long lAmount) const`: Kiểm tra điều kiện rút tiền theo bảng quy tắc:
  $$\begin{cases} 
  \text{ERR\_INVALID\_AMOUNT} & \text{khi } lAmount < lMinTransaction \text{ hoặc } lAmount \le 0 \\
  \text{ERR\_NOT\_MULTIPLE} & \text{khi } lAmount \pmod{lMinTransaction} \ne 0 \\
  \text{ERR\_INSUFFICIENT\_FUNDS} & \text{khi } (\_lBalance - lAmount) < lMinReserve \\
  \text{ERR\_NONE} & \text{khi thỏa mãn toàn bộ các điều kiện trên}
  \end{cases}$$
- `bool withdraw(long lAmount)`: Nếu `canWithdraw(lAmount) == ERR_NONE`, thực hiện trừ số dư: `_lBalance -= lAmount;` và trả về `true`.
- `bool deposit(long lAmount)`: Kiểm tra chống tràn số nguyên:
  $$\text{Nếu } (LONG\_MAX - lAmount < \_lBalance) \text{ hoặc } (\_lBalance + lAmount > lMaxBalance) \implies \text{trả về } false$$
  Ngược lại thực hiện: `_lBalance += lAmount;` và trả về `true`.

---

## 5.5. THIẾT KẾ THỰC THỂ QUẢN TRỊ VIÊN (`include/Admin.h`, `src/Admin.cpp`)

Quản lý thông tin đăng nhập của bộ phận vận hành và bảo trì máy ATM.

### 5.5.1. Thuộc tính (Attributes)
- `std::string _strUsername`: Tên đăng nhập của Admin (tối thiểu 3 ký tự, không chứa khoảng trắng).
- `std::string _strPassword`: Mật khẩu quản trị (được băm MD5 kèm Pepper bí mật).

### 5.5.2. Phương thức (Methods)
- `Admin()`: Constructor mặc định.
- `Admin(const std::string& strUser, const std::string& strPass)`: Constructor khởi tạo.
- `std::string getUsername() const`: Trả về tên đăng nhập.
- `std::string getPassword() const`: Trả về chuỗi mật khẩu/mã băm lưu trữ.
- `bool verifyPassword(const std::string& strInputPass) const`: Đối soát mật khẩu nhập vào với dữ liệu lưu trữ thông qua `SecurityService::verifyHash`.

---

## 5.6. THIẾT KẾ THỰC THỂ LỊCH SỬ GIAO DỊCH (`include/Transaction.h`, `src/Transaction.cpp`)

Đại diện cho một bản ghi kiểm toán tài chính trong tệp nhật ký `LichSu[ID].txt`.

### 5.6.1. Thuộc tính (Attributes)
- `std::string _strId`: Mã số tài khoản thực hiện hoặc thụ hưởng giao dịch.
- `TransactionType _type`: Loại giao dịch (`WITHDRAW=1`, `TRANSFER=2`, `RECEIVE=3`).
- `long _lAmount`: Giá trị tiền tệ của giao dịch.
- `std::string _strTimestamp`: Dấu thời gian thực hiện (`YYYY-MM-DD HH:MM:SS`).
- `std::string _strDetail`: Nội dung giải trình chi tiết của giao dịch.

### 5.6.2. Phương thức (Methods)
- `Transaction()`: Constructor mặc định.
- `Transaction(const std::string& strId, TransactionType type, long lAmount, const std::string& strTimestamp, const std::string& strDetail)`: Constructor khởi tạo đầy đủ.
- `std::string getId() const`, `TransactionType getType() const`, `long getAmount() const`, `std::string getTimestamp() const`, `std::string getDetail() const`: Các hàm getter thông tin.
- `std::string formatForFile() const`: Chuẩn hóa dữ liệu thành chuỗi 5 trường phân cách bởi dấu gạch đứng `|` để lưu trữ bền vững:
  $$\text{strId} \mid \text{static\_cast<int>(\_type)} \mid \text{lAmount} \mid \text{strTimestamp} \mid \text{strDetail}$$
- `static Transaction parseFromFileLine(const std::string& strFallbackId, const std::string& strLine)`: Tách dòng văn bản thành các token, chuyển đổi kiểu dữ liệu an toàn và trả về đối tượng `Transaction`.

---

## 5.7. THIẾT KẾ DỊCH VỤ BẢO MẬT & MÃ HÓA SALTED-MD5 (`include/SecurityService.h`, `src/SecurityService.cpp`)

Cung cấp các giải thuật mật mã học thuần C++ (Zero external dynamic dependencies), độc lập hoàn toàn với các thư viện bên ngoài.

### 5.7.1. Thuật toán cốt lõi
- **MD5 Hash Engine (RFC 1321)**: Hiện thực đầy đủ thuật toán băm Message Digest Algorithm 5 với 4 hằng số trạng thái ban đầu:
  $$A = \text{0x67452301}, \quad B = \text{0xefcdab89}, \quad C = \text{0x98badcfe}, \quad D = \text{0x10325476}$$
  Biến đổi qua 64 vòng lặp sử dụng 4 hàm phi tuyến $F, G, H, I$, phép cộng modulo $2^{32}$ và phép quay bit trái.
- **Salt & Pepper Security**:
  - `PIN Salt`: `"ATM_PIN_SECRET_SALT_2026_#!"` được gắn kèm vào mã PIN trước khi băm, vô hiệu hóa các cuộc tấn công tra cứu bảng cầu vồng (Rainbow Table).
  - `Admin Pepper`: `"ADMIN_SECURE_AUTH_PEPPER_2026_@$"` tăng cường bảo mật cho mật khẩu quản trị.

### 5.7.2. Bảng phương thức tĩnh (Static Methods)
- `static std::string md5(const std::string& strInput)`: Băm chuỗi dữ liệu tùy ý thành chuỗi 32 ký tự hex viết thường.
- `static std::string hashPin(const std::string& strPin)`: Trả về `md5(strPin + PIN_SALT)`.
- `static std::string hashPassword(const std::string& strPassword)`: Trả về `md5(strPassword + ADMIN_PEPPER)`.
- `static bool verifyHash(const std::string& strRaw, const std::string& strStored)`: Xác thực đa tầng thông minh:
  - Nếu `strRaw == strStored` $\implies$ Khớp plaintext (hỗ trợ dữ liệu kiểm thử cũ).
  - Nếu `hashPin(strRaw) == strStored` $\implies$ Khớp mã PIN băm kèm muối.
  - Nếu `hashPassword(strRaw) == strStored` $\implies$ Khớp mật khẩu Admin băm kèm tiêu.
  - Ngược lại $\implies$ Từ chối xác thực (`false`).

---

## 5.8. THIẾT KẾ DỊCH VỤ QUẢN LÝ TỆP TIN & LƯU TRỮ BỀN VỮNG (`include/FileService.h`, `src/FileService.cpp`)

Chịu trách nhiệm toàn bộ các thao tác Nhập/Xuất (I/O) tệp tin với cơ chế an toàn cấp độ hệ điều hành.

### 5.8.1. Cơ chế An toàn Tệp tin
- **Khóa tệp liên tiến trình (`FileLockGuard`)**: 
  - Khởi tạo: Mở tệp `.atm_data.lock` và yêu cầu khóa độc quyền `flock(fd, LOCK_EX)` trên Linux/POSIX hoặc `LockFileEx` trên Windows.
  - Hủy bỏ (RAII): Giải phóng khóa `flock(fd, LOCK_UN)` và đóng file descriptor trong Destructor.
- **Ghi nguyên tử (`atomicWriteFile`)**:
  - Ghi toàn bộ dữ liệu ra tệp tạm có tiền tố Process ID: `[Path].[PID].tmp`.
  - Gọi `fout.flush()` và đóng tệp.
  - Sử dụng lệnh hệ thống `std::filesystem::rename` để tráo đổi tệp nguyên tử trên hệ thống tệp tin (File System Atomic Rename). Nếu rename thất bại, tự động sao lưu bản backup `.bak`.

### 5.8.2. Danh mục phương thức tĩnh chính
- `static bool loadAdmins(LinkedList<Admin>& listAdmins)`: Nạp danh sách quản trị viên từ `data/Admin.txt`.
- `static bool loadCards(LinkedList<Card>& listCards, const LinkedList<std::string>& listLockedIds)`: Nạp danh sách thẻ từ `data/TheTu.txt`, đồng bộ cờ khóa thẻ.
- `static bool loadLockedIds(LinkedList<std::string>& listLockedIds)`: Đọc danh sách ID bị khóa từ `data/KhoaThe.txt`.
- `static bool saveCards(const LinkedList<Card>& listCards)`: Ghi đè danh sách thẻ vào `TheTu.txt` qua `atomicWriteFile`.
- `static bool saveLockedIds(const LinkedList<std::string>& listLockedIds)`: Ghi danh sách thẻ khóa vào `KhoaThe.txt`.
- `static bool appendLockedCard(const std::string& strId)`: Ghi nối một ID bị khóa vào `KhoaThe.txt`.
- `static ErrorCode loadAccount(const std::string& strId, Account& acc)`: Đọc thông tin tài khoản từ `data/[ID].txt`.
- `static bool saveAccount(const Account& acc)`: Ghi 4 dòng thông tin tài khoản vào `data/[ID].txt` qua `atomicWriteFile`.
- `static bool deleteAccountFile(const std::string& strId)`: Xóa tệp `data/[ID].txt` khỏi ổ đĩa.
- `static bool archiveHistoryFile(const std::string& strId)`: Đổi tên tệp lịch sử thành `data/Archive_LichSu[ID]_[timestamp].bak` khi xóa thẻ.
- `static bool appendTransaction(const std::string& strId, const Transaction& trans)`: Ghi nối một giao dịch vào `data/LichSu[ID].txt`.
- `static bool loadTransactions(const std::string& strId, LinkedList<Transaction>& listTrans)`: Nạp toàn bộ lịch sử giao dịch từ đĩa.
- `static int getFailedAttempts(const std::string& strId)`: Đọc số lần nhập sai PIN từ `data/FailedAttempts.txt`.
- `static int recordFailedAttempt(const std::string& strId)`: Tăng và lưu bền vững số lần nhập sai PIN vào `FailedAttempts.txt`.
- `static bool resetFailedAttempts(const std::string& strId)`: Xóa bản ghi đếm sai của thẻ khỏi `FailedAttempts.txt`.
- `static void initSampleData()`: Khởi tạo dữ liệu mẫu ban đầu nếu hệ thống chạy lần đầu tiên.

---

## 5.9. THIẾT KẾ GIAO DIỆN ĐIỀU KHIỂN CONSOLE (`include/ConsoleView.h`, `src/ConsoleView.cpp`)

Quản lý tương tác người dùng, định dạng văn bản màu sắc ANSI, bảng biểu, phân trang và bẫy lỗi luồng nhập `std::cin`.

### 5.9.1. Mã màu chuẩn ANSI
- `COLOR_RESET` (`\033[0m`): Hoàn nguyên màu sắc mặc định.
- `COLOR_RED` (`\033[31m`): Báo lỗi hệ thống, giao dịch thất bại.
- `COLOR_GREEN` (`\033[32m`): Thông báo thành công, số dư dương.
- `COLOR_YELLOW` (`\033[33m`): Cảnh báo, hiển thị mã PIN hoặc tỷ giá.
- `COLOR_BLUE` (`\033[34m`): Đường kẻ viền, khung menu.
- `COLOR_CYAN` (`\033[36m`): Tiêu đề, thông tin chủ tài khoản.

### 5.9.2. Bảng phương thức giao diện và bắt lỗi luồng nhập
- `static std::string inputPassword(const std::string& strPrompt)`: Che giấu ký tự mật khẩu thành dấu `*` trên màn hình, hỗ trợ phím xóa lùi Backspace (`\b`), phím Enter, và phát hiện tín hiệu EOF (`Ctrl+D` / `Ctrl+Z`).
- `static std::string inputLine(const std::string& strPrompt)`: Đọc chuỗi văn bản hoàn chỉnh sử dụng `std::getline`, tự động loại bỏ khoảng trắng thừa đầu cuối (`trim`).
- `static int inputChoice(int iMin, int iMax)`: Nhập lựa chọn menu từ `iMin` đến `iMax`, chống treo chương trình khi nhập chữ hoặc ký tự đặc biệt.
- `static long inputMoneyRange(const std::string& strPrompt, long lMinLimit, long lMaxLimit, const std::string& strCurrency)`: Nhập số tiền giao dịch có kiểm tra giới hạn biên $[lMinLimit, lMaxLimit]$, phát hiện lỗi tràn số `ERANGE` của `std::strtoll`.
- `static std::string formatMoney(long lAmount)`: Định dạng số tiền có dấu phân cách hàng nghìn (ví dụ: `50000` $\implies$ `"50,000"`).
- `static void printHeader(const std::string& strTitle)`: Vẽ khung tiêu đề ASCII nổi bật.
- `static void printError(const std::string& strMsg)` / `printSuccess(const std::string& strMsg)`: In thông điệp trạng thái có viền và màu sắc tương ứng.
- `static void printReceipt(...)`: In biên lai giao dịch tài chính chuẩn hóa với đầy đủ mã tài khoản, loại giao dịch, số tiền, tỷ giá (nếu có ngoại tệ), số dư còn lại, và thời gian thực hiện.
- `static bool displayTransactionHistory(const std::string& strId, int iPage)`: Hiển thị lịch sử giao dịch dưới dạng bảng biểu có chia trang (10 bản ghi/trang), điều hướng linh hoạt (Trang trước/Trang sau/Thoát).

---

## 5.10. THIẾT KẾ BỘ ĐIỀU PHỐI QUẢN TRỊ VIÊN (`include/AdminController.h`, `src/AdminController.cpp`)

Quản trị toàn bộ các chức năng của phân hệ Admin theo yêu cầu đề bài.

### 5.10.1. Thuộc tính phiên làm việc
- `LinkedList<Admin> _listAdmins`: Danh sách các quản trị viên nạp từ `Admin.txt`.
- `LinkedList<Card> _listCards`: Danh sách toàn bộ thẻ từ nạp từ `TheTu.txt`.
- `LinkedList<std::string> _listLockedIds`: Danh sách ID các thẻ đang bị khóa nạp từ `KhoaThe.txt`.

### 5.10.2. Các quy trình nghiệp vụ (Workflows)
1. **Xác thực Admin (`verifyAdmin`)**: Đối chiếu thông tin username và password với danh sách quản trị viên thông qua giải thuật băm MD5 kèm Pepper.
2. **Xem danh sách thẻ (`viewCardList`)**: Xuất bảng biểu toàn bộ thẻ từ trong hệ thống gồm: STT, Mã số ID (14 số), Mã PIN (ẩn dạng `******`), và Trạng thái (HOẠT ĐỘNG hoặc BỊ KHÓA).
3. **Thêm tài khoản mới (`addNewCard`)**:
   - Nhập mã thẻ 14 chữ số, kiểm tra định dạng và chống trùng lặp ID.
   - Nhập tên chủ thẻ (hỗ trợ tiếng Việt có dấu/khoảng trắng).
   - Chọn loại tiền tệ (`VND`, `USD`, `EUR`, `JPY`, `GBP`, `SGD`).
   - Nhập số dư ban đầu trong khoảng cho phép của đồng tiền tương ứng.
   - Mã PIN mặc định là `"123456"` (được băm MD5 kèm Salt).
   - Tự động tạo đồng thời: `data/[ID].txt`, `data/LichSu[ID].txt`, và cập nhật `TheTu.txt`.
4. **Xóa tài khoản (`deleteCard`)**:
   - Nhập ID cần xóa, yêu cầu xác nhận xác thực từ Admin (`Y/N`).
   - Xóa thẻ khỏi RAM và cập nhật `TheTu.txt`.
   - Xóa tệp `data/[ID].txt`.
   - Đổi tên tệp lịch sử thành `Archive_LichSu[ID]_[timestamp].bak` để phục vụ kiểm toán tài chính và chống xung đột dữ liệu nếu sau này tạo lại ID này.
   - Đặt lại bộ đếm sai trong `FailedAttempts.txt`.
5. **Mở khóa tài khoản (`unlockCard`)**:
   - Hiển thị danh sách các thẻ đang bị khóa từ `KhoaThe.txt`.
   - Admin chọn ID muốn mở khóa.
   - Loại bỏ ID khỏi `KhoaThe.txt`, cập nhật trạng thái thẻ trong RAM thành không khóa, và reset số lần sai trong `FailedAttempts.txt` về 0.

---

## 5.11. THIẾT KẾ BỘ ĐIỀU PHỐI NGƯỜI DÙNG & GIAO DỊCH (`include/UserController.h`, `src/UserController.cpp`)

Quản trị toàn bộ các chức năng tài chính của khách hàng sử dụng thẻ ATM.

### 5.11.1. Các quy trình nghiệp vụ chính
1. **Xác thực khách hàng (`authenticate`)**:
   - Kiểm tra định dạng ID (14 số) và PIN (6 số).
   - Kiểm tra thẻ có nằm trong danh sách khóa `KhoaThe.txt` không.
   - So khớp mã PIN: nếu sai, gọi `FileService::recordFailedAttempt(strId)` để ghi nhận bền vững xuống đĩa. Nếu sai đủ 3 lần, tự động khóa thẻ ngay lập tức.
2. **Bắt buộc đổi PIN mặc định (`enforceDefaultPinChange`)**:
   - Nếu thẻ đang mang mã PIN `"123456"`, hệ thống chặn mọi giao dịch và hiển thị màn hình bắt buộc đổi PIN.
   - Khách hàng phải nhập mã PIN mới 2 lần để xác nhận (mã mới không được trùng PIN cũ và không được là `"123456"`).
3. **Rút tiền tài khoản (`processWithdrawAndPersist`)**:
   - *Bước 1 (Chống Stale Read)*: Đọc lại số dư thực tế từ đĩa qua `FileService::loadAccount`.
   - *Bước 2*: Kiểm tra toàn bộ điều kiện rút tiền qua `Account::canWithdraw`.
   - *Bước 3*: Trừ tiền trong RAM và gọi `FileService::saveAccount`.
   - *Bước 4*: Nếu ghi đĩa thất bại, hoàn tiền trên RAM (`deposit`) và trả về lỗi I/O.
   - *Bước 5*: Ghi nối giao dịch vào `LichSu[ID].txt` và in biên lai giao dịch.
4. **Rút tiền quy đổi ngoại tệ (Multi-Currency FX Withdrawal)**:
   - Áp dụng khi khách hàng sở hữu tài khoản ngoại tệ (`USD`, `EUR`,...) nhưng máy ATM tại Việt Nam chi trả tiền mặt bằng `VND`.
   - *Công thức trừ ngoại tệ bằng phép chia làm tròn lên (Ceiling Division)*:
     $$lDeduct = \left\lfloor \frac{lVndAmount + lExchangeRateToVND - 1}{lExchangeRateToVND} \right\rfloor$$
   - Số tiền VND thực tế tương ứng: $lVndEquivalent = lDeduct \times lExchangeRateToVND$.
   - Chênh lệch làm tròn hoàn lại khách hàng: $lRemainder = lVndEquivalent - lVndAmount$.
5. **Chuyển tiền liên tài khoản (`processTransferAndPersist`)**:
   - Kiểm tra định dạng tài khoản thụ hưởng, chặn tự chuyển tiền cho chính mình.
   - Kiểm tra tài khoản nhận có bị khóa trong `KhoaThe.txt` không.
   - Chặn tuyệt đối chuyển tiền khác loại tiền tệ (`senderAcc.getCurrency() != receiverAcc.getCurrency()`).
   - Đọc số dư mới nhất của cả người gửi và người nhận từ đĩa.
   - Trừ tiền người gửi, cộng tiền người nhận có kiểm tra chống tràn số.
   - Lưu tuần tự hai tài khoản xuống đĩa; nếu có bất kỳ lỗi I/O nào xảy ra, hệ thống tự động Rollback hoàn trả toàn bộ số dư ban đầu cho cả hai bên.
   - Ghi đồng thời 2 bản ghi lịch sử: `TRANSFER` cho người gửi và `RECEIVE` cho người nhận.

---

## 5.12. THIẾT KẾ BỘ ĐIỀU PHỐI TỔNG THỂ ATM (`include/AtmController.h`, `src/AtmController.cpp`)

Đóng vai trò là Mediator điều khiển luồng chương trình cấp cao nhất (Application Lifecycle Manager).

### 5.12.1. Quản lý trạng thái phiên
- `Account* _pCurrentAccount`: Con trỏ tài khoản người dùng đang đăng nhập (khởi tạo `nullptr`).
- `Card* _pCurrentCard`: Con trỏ thẻ từ đang giao dịch (khởi tạo `nullptr`).
- `UserRole _eCurrentRole`: Vai trò hiện tại (`ROLE_NONE`, `ROLE_ADMIN`, `ROLE_USER`).

### 5.12.2. Vòng đời ứng dụng (`run`)
- Khởi tạo thư mục và tệp tin mẫu ban đầu (`FileService::initSampleData`).
- Nạp danh mục quản trị viên, thẻ từ và danh sách khóa vào bộ nhớ.
- Hiển thị menu chào đón:
  - Phím 1: Đăng nhập Quản trị viên (Chuyển quyền điều khiển cho `AdminController`).
  - Phím 2: Đăng nhập Khách hàng sử dụng thẻ (Chuyển quyền điều khiển cho `UserController`).
  - Phím 3: Thoát chương trình.
- Khi người dùng đăng xuất, gọi hàm `cleanupSession()` để đặt con trỏ tài khoản về `nullptr`, xóa sạch dấu vết phiên làm việc trong RAM.
- Destructor của `AtmController` tự động giải phóng toàn bộ các danh sách liên kết, bảo toàn 0 byte rò rỉ bộ nhớ.

---

## 5.13. THIẾT KẾ CƠ CHẾ AN TOÀN: GHI NGUYÊN TỬ VÀ KHÓA FILE LIÊN TIẾN TRÌNH

Nhằm đáp ứng các tiêu chuẩn khắt khe về độ tin cậy của phần mềm tài chính ngân hàng, hệ thống cài đặt cơ chế xử lý tệp tin hai lớp:

```
[Tiến trình ATM A]                     [Tiến trình ATM B]
        |                                      |
        v                                      v
   Yêu cầu Lock                           Yêu cầu Lock
        |                                      |
+------------------------------------------------------+
|       Tệp Khóa Độc Quyền: data/.atm_data.lock         |
|             (flock EXCLUSIVE MUTEX)                  |
+------------------------------------------------------+
        |                                      |
   [Được cấp Lock]                        [Chờ / Chặn]
        |                                      |
        v                                      |
Ghi file tạm: data/[ID].txt.[PID].tmp          |
        |                                      |
Gọi fout.flush() & fout.close()                |
        |                                      |
fs::rename(tmp, data/[ID].txt) [ATOMIC]        |
        |                                      |
Giải phóng Lock (flock UNLOCK)                 |
        |                                      v
        +-----------------------------> [Được cấp Lock]
```

1. **Inter-Process Mutex**: Sử dụng `flock(fd, LOCK_EX)` trên tệp `.atm_data.lock` ngăn chặn xung đột ghi tệp giữa nhiều máy ATM cùng truy xuất cơ sở dữ liệu dùng chung.
2. **Crash-Resilience**: Bất kể chương trình bị dừng đột ngột (`SIGINT`, mất điện, sập nguồn), tệp tin dữ liệu chính không bao giờ bị rơi vào trạng thái 0 byte hay rỗng một nửa, do thao tác hoán đổi tệp `rename` là atomic ở mức hệ điều hành.

---

## 5.14. THIẾT KẾ CƠ CHẾ PHÒNG THỦ: CHỐNG STALE READ, DOUBLE-SPENDING VÀ ROLLBACK

### 5.14.1. Phòng thủ Stale Read & Double-Spending
Trong mô hình cây ATM phân tán hoặc đa tiến trình, việc người dùng mở 2 phiên làm việc cùng lúc để rút tiền (Double-Spending) được triệt tiêu bằng nguyên tắc **Unconditional Disk Balance Synchronization**:
- Trước khi thực hiện trừ tiền hoặc kiểm tra điều kiện rút/chuyển, hệ thống luôn gọi `FileService::loadAccount` để lấy số dư mới nhất từ đĩa.
- Nếu số dư trên đĩa đã bị thay đổi bởi giao dịch khác khiến số dư không còn đủ duy trì, giao dịch hiện tại lập tức bị hủy bỏ với thông báo lỗi rõ ràng.

### 5.14.2. Cơ chế Rollback Giao dịch Chuyển tiền Hai đầu
Trong giao dịch chuyển tiền giữa 2 tài khoản, việc cập nhật phải thỏa mãn tính toàn vẹn ACID:
```
Thực hiện trừ tiền Người gửi & cộng tiền Người nhận trên RAM
                    |
          Lưu Người gửi xuống đĩa?
               /          \
           (Có)           (Không) ---> Báo lỗi I/O, hủy giao dịch
             |
     Lưu Người nhận xuống đĩa?
          /             \
      (Có)              (Không)
       |                   |
Thành công                 +---> ROLLBACK:
Ghi 2 file lịch sử                - Cộng lại tiền cho Người gửi
In hóa đơn                        - Trừ lại tiền Người nhận
                                  - Lưu lại Người gửi xuống đĩa
                                  - Ghi nhật ký cảnh báo AdminLog
```

---

## 5.15. THIẾT KẾ KIẾN TRÚC LƯU TRỮ TỆP TIN (DATA PERSISTENCE SCHEMA)

Cơ sở dữ liệu tệp tin được bố trí gọn gàng trong thư mục `data/` với định dạng chuẩn hóa:

| Tên tệp tin | Mục đích | Định dạng dòng dữ liệu | Ví dụ thực tế |
|:---|:---|:---|:---|
| `Admin.txt` | Danh sách tài khoản Quản trị viên | `[Username] [HashedPassword]` | `admin1 900150983cd24fb0d6963f7d28e17f72` |
| `TheTu.txt` | Danh sách toàn bộ thẻ từ ATM | `[ID] [HashedPin]` | `10014504500001 02c6acf3391d1e4344d2d48bf53c43fc` |
| `KhoaThe.txt` | Danh sách ID các thẻ đang bị khóa | `[ID]` (Mỗi ID trên một dòng) | `10014504500005` |
| `FailedAttempts.txt` | Nhật ký số lần nhập sai PIN liên tiếp | `[ID] [FailedCount]` | `10014504500002 2` |
| `[ID].txt` | Chi tiết tài khoản ngân hàng (4 dòng) | Dòng 1: ID<br>Dòng 2: Họ tên<br>Dòng 3: Số dư<br>Dòng 4: Đơn vị tiền tệ | `10014504500001`<br>`Nguyen Trung Kien`<br>`500000`<br>`VND` |
| `LichSu[ID].txt` | Nhật ký lịch sử giao dịch của tài khoản | `[ID]\|[Type]\|[Amount]\|[Timestamp]\|[Detail]` | `10014504500001\|1\|200000\|2026-10-10 22:56:45\|Rut tien mat tai ATM` |
| `.atm_data.lock` | Tệp khóa độc quyền liên tiến trình | Rỗng (Quản lý bởi FileLockGuard) | `[Lock Mutex]` |
| `.system_initialized` | Cờ đánh dấu hệ thống đã khởi tạo | Rỗng (Chống ghi đè dữ liệu Admin mẫu) | `[Marker File]` |
