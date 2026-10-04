# 🔬 MÔ TẢ CHI TIẾT 4 NHIỆM VỤ — MEMBER A (Tuấn)
> Phân công theo: [`docs/06_ke_hoach_trien_khai.md`](06_ke_hoach_trien_khai.md) | Nhánh: `tuan`

---

## NHIỆM VỤ 1 — Cài đặt Models: `Card` & `Account`
> **Commit:** `3eed0d8` — `feat(models): implement Card and Account models with financial rules`  
> **Files:** [`include/Card.h`](../include/Card.h) · [`src/Card.cpp`](../src/Card.cpp) · [`include/Account.h`](../include/Account.h) · [`src/Account.cpp`](../src/Account.cpp)

### Mục tiêu
Xây dựng tầng Domain (lớp Model) đại diện cho **thẻ ATM** và **tài khoản ngân hàng**, đảm bảo đầy đủ quy tắc nghiệp vụ và chuẩn Hungarian Notation.

---

### 1.1 — Class `Card` (Thẻ từ ATM)

**Bước 1: Khai báo thuộc tính** *(trong `include/Card.h`)*

Đặt tên theo Hungarian Notation, tiền tố `_` cho biến thành viên:

```cpp
class Card {
private:
    std::string _strId;           // Mã thẻ 14 chữ số
    std::string _strPin;          // Mã PIN 6 chữ số
    int         _iFailedAttempts; // Số lần nhập sai PIN liên tiếp
    bool        _bIsLocked;       // Trạng thái khóa thẻ
    ...
```

**Bước 2: Khai báo Constructor** — 3 overload cho linh hoạt khi dùng:

```cpp
Card();                                                            // Default
Card(const std::string& strId, const std::string& strPin);        // Khi tạo thẻ mới
Card(const std::string& strId, const std::string& strPin, bool bIsLocked); // Khi đọc từ file
```

**Bước 3: Hiện thực các phương thức nghiệp vụ** *(trong `src/Card.cpp`)*

| Phương thức | Logic cốt lõi |
|-------------|--------------|
| `isDefaultPin()` | So sánh `_strPin == DEFAULT_PIN` ("123456") |
| `checkPin(input)` | So sánh `_strPin == strInput` |
| `recordFailedAttempt()` | Tăng `_iFailedAttempts++`. Nếu `>= MAX_FAILED_LOGINS (3)` → tự động set `_bIsLocked = true` |
| `resetFailedAttempts()` | Reset về `0` và `_bIsLocked = false` (dùng khi Admin mở khóa) |
| `changePin(newPin)` | Gán trực tiếp `_strPin = strNewPin` |

> **Lưu ý quan trọng:** Hằng số `MAX_FAILED_LOGINS = 3` và `DEFAULT_PIN = "123456"` được định nghĩa ở [`include/Common.h`](../include/Common.h) — không hardcode trong class.

---

### 1.2 — Class `Account` (Tài khoản ngân hàng)

**Bước 1: Khai báo thuộc tính** *(trong `include/Account.h`)*

```cpp
class Account {
private:
    std::string _strId;       // Mã tài khoản (= ID thẻ)
    std::string _strName;     // Tên chủ tài khoản (NGUYEN VAN A)
    long        _lBalance;    // Số dư (dùng long, đơn vị VNĐ)
    std::string _strCurrency; // Đơn vị tiền tệ ("VND")
    ...
```

> **Lý do dùng `long`:** Số dư có thể lên đến hàng tỷ VNĐ — `int` chỉ chứa tối đa ~2.1 tỷ, `long` an toàn hơn.

**Bước 2: Hiện thực hàm kiểm tra ràng buộc tài chính** *(phần quan trọng nhất)*

```cpp
ErrorCode Account::canWithdraw(long lAmount) const {
    // Ràng buộc 1: Số tiền phải >= 50,000 VNĐ
    if (lAmount < MIN_TRANSACTION)
        return ERR_INVALID_AMOUNT;

    // Ràng buộc 2: Số tiền phải là bội số của 50,000
    if (lAmount % MIN_TRANSACTION != 0)
        return ERR_NOT_MULTIPLE;

    // Ràng buộc 3: Sau khi rút phải còn >= 50,000 VNĐ (số dư duy trì tối thiểu)
    if (this->_lBalance - lAmount < MIN_BALANCE_RESERVE)
        return ERR_INSUFFICIENT_FUNDS;

    return ERR_NONE; // Hợp lệ
}
```

Hàm trả về `ErrorCode` (enum trong `Common.h`) thay vì `bool` → cho phép caller biết **lý do cụ thể** tại sao không rút được để hiển thị đúng thông báo lỗi.

**Bước 3: Hiện thực `withdraw` và `deposit`** — tách riêng khỏi `canWithdraw`:

```cpp
void Account::withdraw(long lAmount) { this->_lBalance -= lAmount; }
void Account::deposit(long lAmount)  { this->_lBalance += lAmount; }
```

> Caller **phải** tự gọi `canWithdraw()` trước. Tách như vậy để `processTransfer()` có thể trừ người gửi rồi cộng người nhận mà không validate lại 2 lần.

---

## NHIỆM VỤ 2 — Cài đặt Giao diện Console: `ConsoleView`
> **Commit:** `a1a1c36` — `feat(ui): implement ConsoleView with ANSI color, masked password, and cin validation`  
> **Files:** [`include/ConsoleView.h`](../include/ConsoleView.h) · [`src/ConsoleView.cpp`](../src/ConsoleView.cpp)

### Mục tiêu
Xây dựng toàn bộ lớp tiện ích UI cho terminal: màu ANSI, nhập mật khẩu ẩn, và bẫy lỗi `cin.fail()`.

---

### 2.1 — Màu ANSI (ANSI Color Codes)

**Bước 1:** Khai báo các hằng chuỗi escape code ở đầu `ConsoleView.cpp`:

```cpp
const std::string ANSI_RESET  = "\033[0m";
const std::string ANSI_BOLD   = "\033[1m";
const std::string ANSI_RED    = "\033[31m";   // Lỗi
const std::string ANSI_GREEN  = "\033[32m";   // Thành công
const std::string ANSI_YELLOW = "\033[33m";   // Cảnh báo
const std::string ANSI_CYAN   = "\033[36m";   // Header / tiêu đề
```

**Bước 2:** Hiện thực 5 hàm in theo ngữ cảnh — tất cả `static`, gọi trực tiếp không cần object:

```cpp
// Ví dụ:
void ConsoleView::printError(const std::string& strMsg) {
    std::cout << ANSI_RED << ANSI_BOLD << "[LOI] " << strMsg << ANSI_RESET << "\n";
}
// Tương tự cho printSuccess (GREEN), printWarning (YELLOW), printInfo (BLUE), printHeader (CYAN)
```

---

### 2.2 — Hàm nhập mật khẩu ẩn: `inputPassword()`

Vấn đề: `std::cin` in ký tự ra màn hình khi gõ. Cần ẩn thành `*`.

**Bước 1:** Xử lý đa nền tảng — `_WIN32` dùng `conio.h`, Linux dùng `termios`:

```cpp
#ifdef _WIN32
#include <conio.h>          // Có sẵn hàm _getch()
#else
#include <termios.h>        // Phải tự viết _getch() bằng cách tắt ECHO của terminal
#include <unistd.h>
static char _getch() {
    struct termios old = {0};
    tcgetattr(0, &old);
    old.c_lflag &= ~ICANON; // Tắt chế độ dòng (đọc từng ký tự)
    old.c_lflag &= ~ECHO;   // Tắt hiển thị ký tự
    tcsetattr(0, TCSANOW, &old);
    char buf = 0;
    read(0, &buf, 1);
    // Khôi phục cài đặt terminal cũ
    old.c_lflag |= ICANON; old.c_lflag |= ECHO;
    tcsetattr(0, TCSADRAIN, &old);
    return buf;
}
#endif
```

**Bước 2:** Vòng lặp đọc từng ký tự và xử lý các phím đặc biệt:

```cpp
std::string ConsoleView::inputPassword(const std::string& strPrompt) {
    std::string strPass = "";
    while (true) {
        char ch = _getch();
        if (ch == '\r' || ch == '\n') { std::cout << "\n"; break; }      // Enter → kết thúc
        else if (ch == '\b' || ch == 127) {   // Backspace (Windows=\b, Linux=127)
            if (!strPass.empty()) {
                strPass.pop_back();
                std::cout << "\b \b"; // Xóa dấu * cuối cùng trên màn hình
            }
        } else if (ch == 3) { return ""; }   // Ctrl+C → hủy
        else if (ch >= 32 && ch <= 126) {    // Ký tự in được
            strPass.push_back(ch);
            std::cout << "*";              // In * thay cho ký tự thật
        }
    }
    return strPass;
}
```

---

### 2.3 — Bẫy lỗi `cin.fail()`: `inputMoney()` và `inputMenuChoice()`

Vấn đề: Nếu user gõ chữ ("abc") vào trường số tiền → `cin` vào trạng thái `fail` → vòng lặp vô hạn hoặc crash.

**Giải pháp — vòng lặp `while(true)` với `cin.clear()` + `cin.ignore()`:**

```cpp
long ConsoleView::inputMoney(const std::string& strPrompt) {
    long lAmount = 0;
    while (true) {
        std::cout << strPrompt;
        if (std::cin >> lAmount) {                          // Đọc thành công
            std::cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Xả bộ đệm
            if (lAmount >= 0) return lAmount;
            ConsoleView::printError("So tien khong the am!");
        } else {
            std::cin.clear();                               // Xóa cờ lỗi
            std::cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Xả ký tự rác
            ConsoleView::printError("Nhap sai dinh dang so! Vui long nhap lai.");
        }
    }
}
```

> `cin.clear()` xóa cờ `failbit` — nếu không gọi thì mọi lần `cin >>` tiếp theo đều bị bỏ qua ngay lập tức.

---

### 2.4 — Template hàm `displayCardList()`

Hàm này cần nhận 2 danh sách (`LinkedList<Card>` và `LinkedList<string>`) từ Member B (Trí) — nhưng tại thời điểm viết, `LinkedList<T>` chưa có. Giải pháp: dùng **C++ Template**:

```cpp
template <typename ListCardType, typename ListLockedType>
static void displayCardList(const ListCardType& listCards, const ListLockedType& listLockedIds) {
    // Duyệt bằng con trỏ node .getHead() -> _pNext
    // Kiểm tra trạng thái khóa từ cả 2 nguồn: card._bIsLocked và file KhoaThe.txt
}
```

→ Hàm này sẽ compile đúng với bất kỳ kiểu `LinkedList<T>` nào Member B cung cấp.

---

## NHIỆM VỤ 3 — Luồng nghiệp vụ Phân hệ User: `UserController`
> **Commit:** `f6c8bde` — `feat(user): implement User module business logic and session handler`  
> **Files:** [`include/UserController.h`](../include/UserController.h) · [`src/UserController.cpp`](../src/UserController.cpp)

### Mục tiêu
Hiện thực toàn bộ 9 chức năng của Phân hệ Khách hàng theo luồng nghiệp vụ thực tế.

---

### 3.1 — Xác thực đăng nhập: `authenticate()`

```
Luồng: Nhập ID → Nhập PIN → Kiểm tra khóa → Kiểm tra PIN → Tăng sai/Reset
```

```cpp
bool UserController::authenticate(Card& card, const std::string& strInputPin, bool& bOutCardLocked) {
    bOutCardLocked = false;
    if (card.isLocked()) { bOutCardLocked = true; return false; } // Thẻ đã bị khóa trước đó

    if (card.checkPin(strInputPin)) {
        card.resetFailedAttempts();   // Đăng nhập đúng → reset bộ đếm
        return true;
    }
    card.recordFailedAttempt();       // Sai → tăng đếm, có thể tự khóa bên trong Card
    if (card.isLocked()) bOutCardLocked = true;
    return false;
}
```

**Lý do dùng output parameter `bOutCardLocked`:** Cho phép caller (`AtmController`) biết ngay kết quả khóa để ghi vào `KhoaThe.txt` mà không cần kiểm tra lại `card.isLocked()`.

---

### 3.2 — Ép đổi PIN mặc định: `enforceDefaultPinChange()`

```
Luồng: Kiểm tra isDefaultPin() → Hiện cảnh báo → Vòng lặp yêu cầu nhập PIN mới
```

Các điều kiện kiểm tra PIN mới (theo thứ tự):

| # | Điều kiện | Thông báo lỗi |
|---|-----------|--------------|
| 1 | Phải đúng 6 chữ số (`isValidPinFormat`) | "Ma PIN moi phai chua dung 6 chu so!" |
| 2 | Không được trùng với `DEFAULT_PIN` | "Ma PIN moi khong duoc trung voi ma PIN mac dinh!" |
| 3 | Xác nhận lần 2 phải khớp | "Hai lan nhap ma PIN khong khop nhau!" |

Chỉ khi qua **cả 3 kiểm tra** mới gọi `card.changePin(strNewPin)`.

---

### 3.3 — Rút tiền: `processWithdraw()`

Thiết kế tách biệt validation khỏi action:

```cpp
ErrorCode UserController::processWithdraw(Account& acc, long lAmount) {
    ErrorCode err = acc.canWithdraw(lAmount);  // Validate 3 ràng buộc (bên Account)
    if (err == ERR_NONE) {
        acc.withdraw(lAmount);                  // Chỉ trừ tiền khi hợp lệ
    }
    return err;  // Trả về lý do lỗi để UI hiển thị đúng message
}
```

Trong `runUserSession()`, caller nhận `ErrorCode` và hiển thị đúng thông báo:

```
ERR_INVALID_AMOUNT   → "So tien rut toi thieu phai tu 50,000 VND!"
ERR_NOT_MULTIPLE     → "So tien rut phai la boi so cua 50,000 VND!"
ERR_INSUFFICIENT_FUNDS → "So du khong du! Can giu lai it nhat 50,000 VND."
ERR_NONE             → "Rut tien thanh cong!"
```

---

### 3.4 — Chuyển tiền nguyên tử: `processTransfer()`

```
Luồng: Kiểm tra cùng tài khoản → Validate người gửi → Trừ người gửi → Cộng người nhận
```

```cpp
ErrorCode UserController::processTransfer(Account& senderAcc, Account& receiverAcc, long lAmount) {
    // Guard: Không cho chuyển tiền cho chính mình
    if (senderAcc.getId() == receiverAcc.getId())
        return ERR_SAME_ACCOUNT;

    // Validate đủ tiền và ràng buộc số tiền từ phía người gửi
    ErrorCode err = senderAcc.canWithdraw(lAmount);
    if (err != ERR_NONE) return err;

    // Thực hiện nguyên tử: trừ rồi cộng ngay lập tức
    senderAcc.withdraw(lAmount);
    receiverAcc.deposit(lAmount);
    return ERR_NONE;
}
```

> **Nguyên tắc nguyên tử (Atomicity):** Nếu `withdraw` thành công thì `deposit` phải luôn thực thi ngay sau. Không có bước nào ở giữa có thể làm gián đoạn. Member B (Trí) sẽ đảm bảo tính nguyên tử ở tầng ghi file.

---

### 3.5 — Đổi mã PIN: `processChangePin()`

4 kiểm tra validation lần lượt, trả về `false` ngay khi thất bại đầu tiên:

```cpp
bool UserController::processChangePin(Card& card, const std::string& strOldPin,
                                      const std::string& strNewPin, const std::string& strConfirmPin,
                                      std::string& strOutMessage) {
    if (!card.checkPin(strOldPin))           { strOutMessage = "Ma PIN cu khong chinh xac!"; return false; }
    if (!isValidPinFormat(strNewPin))        { strOutMessage = "Ma PIN moi phai 6 chu so!"; return false; }
    if (strNewPin == strOldPin)              { strOutMessage = "Ma PIN moi phai khac PIN hien tai!"; return false; }
    if (strNewPin != strConfirmPin)          { strOutMessage = "Xac nhan ma PIN khong khop!"; return false; }
    card.changePin(strNewPin);
    strOutMessage = "Doi ma PIN thanh cong!";
    return true;
}
```

---

### 3.6 — Vòng lặp phiên làm việc: `runUserSession()`

Vòng lặp `while(true)` điều phối toàn bộ menu User:

```
Hiển thị menu (5 chức năng + 0=Thoát)
    → inputMenuChoice(0, 5) [bẫy lỗi cin.fail]
    → switch(iChoice):
        case 1: displayAccountInfo()       [Xem thông tin]
        case 2: processWithdraw()          [Rút tiền]
        case 3: processTransfer()          [Chuyển tiền]
        case 4: [Xem lịch sử - kết nối Member B]
        case 5: processChangePin()         [Đổi PIN]
        case 0: break                      [Đăng xuất]
```

---

## NHIỆM VỤ 4 — Kiểm thử tự động: `test/test_member_a.cpp`
> **Commit:** `15bd3a8` — `test & docs: add comprehensive unit test suite and demo script for Member A`  
> **Files:** [`test/test_member_a.cpp`](../test/test_member_a.cpp) · [`docs/DemoScript_User.md`](DemoScript_User.md)

### Mục tiêu
Viết bộ Unit Test tự động kiểm tra **100% logic** của 3 class trên mà không cần chạy ứng dụng.

---

### 4.1 — Cơ chế test: Macro `ASSERT_TEST`

Thay vì viết `if/else` lặp đi lặp lại, định nghĩa macro in màu kết quả:

```cpp
#define ASSERT_TEST(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "\033[31m[FAIL] " << msg << "\033[0m\n"; \
            return false; \
        } else { \
            std::cout << "\033[32m[PASS] " << msg << "\033[0m\n"; \
        } \
    } while (0)
```

`[PASS]` in xanh, `[FAIL]` in đỏ — nhìn một cái biết ngay kết quả.

---

### 4.2 — 3 nhóm test case

**Nhóm 1: `testCardModel()`** — Kiểm tra class `Card`

| Test case | Điều kiện kiểm tra |
|-----------|-------------------|
| Khởi tạo đúng ID, PIN | `card.getId() == "10014504500001"` |
| Phát hiện PIN mặc định | `card.isDefaultPin() == true` |
| Sai 1, 2 lần chưa khóa | `getFailedAttempts() == 1` và `!isLocked()` |
| Sai 3 lần → tự khóa | `getFailedAttempts() == 3` và `isLocked() == true` |
| Reset mở khóa thành công | `getFailedAttempts() == 0` và `!isLocked()` |
| Đổi PIN thành công | `card.getPin() == "888888"` |

**Nhóm 2: `testAccountModel()`** — Kiểm tra ràng buộc tài chính

| Test case | Giá trị test | Kết quả mong đợi |
|-----------|-------------|-----------------|
| Rút dưới 50k | 20,000 | `ERR_INVALID_AMOUNT` |
| Rút không bội số 50k | 75,000 | `ERR_NOT_MULTIPLE` |
| Rút để lại < 50k duy trì | 500,000 (số dư = 500k) | `ERR_INSUFFICIENT_FUNDS` |
| Rút hợp lệ | 450,000 (giữ lại đúng 50k) | `ERR_NONE` |
| Thực hiện rút/nạp thực tế | `withdraw(200k)` → `deposit(100k)` | Số dư đúng |

**Nhóm 3: `testUserControllerLogic()`** — Kiểm tra nghiệp vụ

| Test case | Mô tả |
|-----------|-------|
| Sai PIN 3 lần qua `authenticate()` | `bOutCardLocked == true` |
| Đúng PIN sau reset | `authenticate() == true` |
| Chuyển tiền cho chính mình | `ERR_SAME_ACCOUNT` |
| Chuyển tiền hợp lệ 300k | Người gửi còn 700k, người nhận lên 500k |
| Đổi PIN sai PIN cũ | `processChangePin() == false` |
| Đổi PIN xác nhận không khớp | `processChangePin() == false` |
| Đổi PIN thành công | `card.getPin() == "654321"` |

---

### 4.3 — Cách chạy Unit Test

```bash
# Biên dịch riêng file test (không cần main.cpp)
g++ -std=c++17 -Wall -Wextra -I include \
    src/Card.cpp src/Account.cpp src/ConsoleView.cpp src/UserController.cpp \
    test/test_member_a.cpp \
    -o test_atm.exe

# Chạy
./test_atm.exe

# Kết quả mong đợi:
# [PASS] Khoi tao dung ID the
# [PASS] Phat hien ma PIN mac dinh 123456
# ... (tất cả PASS)
# [THANH CONG] TAT CA CAC KIEM THU PHAN HE MEMBER A DEU DAT (100% PASS)!
```

---

## 🗺️ SƠ ĐỒ QUAN HỆ GIỮA CÁC FILE

```
Common.h  ──────────────────────────────────────────────┐
  (hằng số: DEFAULT_PIN, MIN_TRANSACTION, ErrorCode...)  │
              ↓                    ↓                      │
           Card.h              Account.h                  │
           Card.cpp            Account.cpp                │
              ↓                    ↓                      │
        ConsoleView.h ─────────────────────────────────── ┘
        ConsoleView.cpp
              ↓
        UserController.h  (dùng Card + Account + ConsoleView)
        UserController.cpp
              ↓
        test_member_a.cpp (test toàn bộ phần trên)
              ↓
        [Chờ tích hợp]
        AtmController.cpp (Member Phát) + FileService.cpp (Member Trí)
```
