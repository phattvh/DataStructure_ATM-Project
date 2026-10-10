# 08. SƠ ĐỒ UML LỚP VÀ LUỒNG DỮ LIỆU BÁO CÁO (UML & DATA FLOW)

---

## I. SƠ ĐỒ LỚP TỔNG THỂ (COMPREHENSIVE UML CLASS DIAGRAM)

```mermaid
classDiagram
    direction TB

    %% Template Cấu trúc dữ liệu
    class Node {
        +T _data
        +Node* _pNext
        +Node(data)
    }

    class LinkedList {
        -Node* _pHead
        -Node* _pTail
        -int _iSize
        +LinkedList()
        +destruct()
        +isEmpty() bool
        +getSize() int
        +getHead() Node*
        +getTail() Node*
        +addTail(item) void
        +findIf(pred) T*
        +removeIf(pred) bool
        +clear() void
    }

    Node --* LinkedList : Chứa

    %% Tầng Thực thể (Entity Models)
    class Admin {
        -string _strUsername
        -string _strPassword
        +Admin()
        +Admin(strUser, strPass)
        +getUsername() string
        +getPassword() string
        +verifyPassword(strPass) bool
    }

    class Card {
        -string _strId
        -string _strPin
        -int _iFailedAttempts
        -bool _bIsLocked
        +Card()
        +Card(strId, strPin)
        +Card(strId, strPin, bIsLocked)
        +getId() string
        +getPin() string
        +isLocked() bool
        +isDefaultPin() bool
        +checkPin(strInputPin) bool
        +recordFailedAttempt() void
        +resetFailedAttempts() void
        +unlockCard() void
        +lockCard() void
        +setLocked(bLocked) void
        +changePin(strNewPin) bool
        +getFailedAttempts() int
        +isValidPinFormat(strPin)$ bool
    }

    class Account {
        -string _strId
        -string _strName
        -long _lBalance
        -string _strCurrency
        +Account()
        +Account(strId, strName, lBalance, strCurrency)
        +getId() string
        +getName() string
        +getBalance() long
        +getCurrency() string
        +setName(strName) void
        +setBalance(lBalance) void
        +setCurrency(strCurrency) void
        +canWithdraw(lAmount) ErrorCode
        +withdraw(lAmount) bool
        +deposit(lAmount) bool
    }

    class Transaction {
        -string _strId
        -TransactionType _eType
        -long _lAmount
        -string _strTimestamp
        -string _strDetail
        +Transaction()
        +Transaction(strId, eType, lAmount, strTimestamp, strDetail)
        +getId() string
        +getType() TransactionType
        +getTypeName() string
        +getAmount() long
        +getTimestamp() string
        +getDetail() string
        +formatForFile() string
        +toString() string
        +parseFromFileLine(strId, strLine)$ Transaction
        +getCurrentTimestamp()$ string
    }

    %% Tầng Dịch vụ Tệp tin (Persistence Service)
    class FileService {
        +loadAdmins(listAdmins)$ bool
        +loadCards(listCards, listLockedIds)$ bool
        +saveCards(listCards)$ bool
        +loadLockedIds(listLockedIds)$ bool
        +saveLockedIds(listLockedIds)$ bool
        +appendLockedCard(strId)$ bool
        +updateCardPin(strId, strNewPin)$ bool
        +loadAccount(strId, acc)$ ErrorCode
        +saveAccount(acc)$ bool
        +deleteAccountFile(strId)$ bool
        +createAccountFiles(strId, strName, lBalance, strCurrency)$ bool
        +appendTransaction(strId, trans)$ bool
        +loadTransactions(strId, listTrans)$ bool
        +initSampleData()$ void
        +appendAdminLog(strAction, strDetail)$ bool
    }

    %% Tầng Giao diện (Presentation Layer)
    class ConsoleView {
        +printHeader(strTitle)$ void
        +printPrompt(strPrompt)$ void
        +printError(strMsg)$ void
        +printSuccess(strMsg)$ void
        +printWarning(strMsg)$ void
        +printInfo(strMsg)$ void
        +inputPassword(strPrompt, pInStream)$ string
        +inputPin(strPrompt)$ string
        +inputMoney(strPrompt, inStream)$ long
        +inputMenuChoice(iMin, iMax, strPrompt, inStream)$ int
        +inputLine(strPrompt)$ string
        +confirmAction(strPrompt)$ bool
        +printMainMenu()$ void
        +printAdminMenu()$ void
        +printUserMenu()$ void
        +clearScreen()$ void
        +pauseScreen()$ void
        +printReceipt(strId, strAction, lAmount, lRemBalance, strTime, strCurrency)$ void
        +displayAccountDetails(strId, strName, lBalance, strCurrency)$ void
        +displayAccountInfo(account)$ void
    }

    %% Tầng Điều phối Nghiệp vụ (Controllers)
    class UserController {
        +isValidPinFormat(strPin)$ bool
        +isValidIdFormat(strId)$ bool
        +authenticate(card, strInputPin, bOutCardLocked)$ bool
        +enforceDefaultPinChange(card)$ bool
        +processWithdraw(acc, lAmount)$ ErrorCode
        +processWithdrawAndPersist(acc, lAmount, bOutCardFailed)$ ErrorCode
        +processTransfer(senderAcc, receiverAcc, lAmount)$ ErrorCode
        +processTransferAndPersist(senderAcc, strReceiverId, lAmount, strCurrency)$ ErrorCode
        +processChangePin(card, strOldPin, strNewPin, strConfirmPin, strOutMsg)$ bool
        +processChangePinAndPersist(card, strNewPin)$ bool
        +displayTransactionHistory(strId, bPause)$ bool
        +displayAccountInfo(acc)$ void
        +runUserSession(card, acc, pReceiverMock)$ void
    }

    class AdminController {
        -LinkedList _listAdmins
        -LinkedList _listCards
        -LinkedList _listLockedIds
        +AdminController()
        +loadAllData() bool
        +verifyAdmin(strUser, strPass) bool
        +viewCardList() void
        +addNewCard() void
        +deleteCard() void
        +unlockCard() void
        +processAdminLogin() bool
        +processAdminMenu() void
    }

    class AtmController {
        -LinkedList _listAdmins
        -LinkedList _listCards
        -LinkedList _listLockedIds
        -Account* _pCurrentAccount
        -Card* _pCurrentCard
        -UserRole _eCurrentRole
        +AtmController()
        +destruct()
        +initData() bool
        +run() void
        +cleanupSession() void
        +authenticateAdmin(strUser, strPass) bool
        +processAdminLogin() void
        +processAdminMenu() void
        +adminViewCards() void
        +addCardAccount(strId, strName, lBalance, strCurrency) ErrorCode
        +adminAddCard() void
        +deleteCardAccount(strId) ErrorCode
        +adminDeleteCard() void
        +unlockCardAccount(strId) ErrorCode
        +adminUnlockCard() void
        +processUserLogin() void
        +processUserMenu() void
    }

    %% Mối quan hệ giữa các lớp
    AtmController o-- LinkedList : Chứa danh sách
    AtmController o-- Account : Quản lý phiên
    AtmController o-- Card : Quản lý phiên
    AtmController ..> UserController : Điều hướng User
    AtmController ..> AdminController : Tích hợp Admin
    AtmController ..> FileService : Nạp dữ liệu
    AtmController ..> ConsoleView : Hiển thị Menu

    AdminController o-- LinkedList : Chứa danh sách
    AdminController ..> FileService : Đọc ghi tệp
    AdminController ..> ConsoleView : Giao diện Admin

    UserController ..> Account : Biến đổi số dư
    UserController ..> Card : Đổi PIN hoặc Khóa
    UserController ..> FileService : Lưu tức thì
    UserController ..> ConsoleView : Giao diện User
```

---

## II. SƠ ĐỒ LUỒNG DỮ LIỆU TỔNG THỂ (SYSTEM DATA FLOW DIAGRAM - DFD)

```mermaid
flowchart TD
    User(["Khách hàng / Quản trị viên"])

    subgraph UI ["Tầng Giao Diện ConsoleView"]
        CV["ConsoleView<br>- Hiển thị menu ANSI<br>- Bắt phím ẩn mã PIN *<br>- Bẫy lỗi nhập số cin.fail"]
    end

    subgraph Controller ["Tầng Điều Phối Controllers"]
        ATM["AtmController<br>- Quản lý phiên làm việc<br>- Điều phối vòng lặp chính"]
        UC["UserController<br>- Rút tiền persistence<br>- Chuyển tiền nguyên tử ACID<br>- Đổi mã PIN & Lịch sử"]
        AC["AdminController<br>- Xem DS thẻ<br>- Thêm thẻ mới 2 file<br>- Xóa thẻ an toàn<br>- Mở khóa thẻ"]
    end

    subgraph Memory ["Tầng Bộ Nhớ RAM"]
        RAM_Cards["LinkedList Card"]
        RAM_Locked["LinkedList LockedIDs"]
        RAM_Admins["LinkedList Admin"]
        RAM_Acc["Account Hien Tai"]
    end

    subgraph Service ["Tầng Dịch Vụ Lưu Trữ FileService"]
        FS["FileService<br>- atomicWriteFile co PID va .bak<br>- Doc 4 dong getline ho ten co khoang trang<br>- Ghi log noi std::ios::app"]
    end

    subgraph Disk ["Tầng Tệp Vật Lý Thư Mục data/"]
        F_Admin[("data/Admin.txt")]
        F_TheTu[("data/TheTu.txt")]
        F_KhoaThe[("data/KhoaThe.txt")]
        F_Acc[("data/ID.txt")]
        F_History[("data/LichSuID.txt")]
        F_Log[("data/AdminLog.txt")]
    end

    User <-->|Nhập lệnh / Xem kết quả| CV
    CV <-->|Điều hướng| ATM
    ATM -->|Phân quyền User| UC
    ATM -->|Phân quyền Admin| AC

    ATM <-->|Duyệt & Quản lý RAM| RAM_Cards
    ATM <-->|Quản lý thẻ khóa| RAM_Locked
    ATM <-->|Xác thực Admin| RAM_Admins
    UC <-->|Thao tác số dư| RAM_Acc

    UC -->|Ghi đĩa ngay khi giao dịch| FS
    AC -->|Cập nhật danh sách & Thẻ| FS
    ATM -->|Khởi tạo & Nạp dữ liệu| FS

    FS <-->|I/O Admin| F_Admin
    FS <-->|I/O Danh sách thẻ| F_TheTu
    FS <-->|I/O Thẻ bị khóa| F_KhoaThe
    FS <-->|I/O Tài khoản| F_Acc
    FS -->|Ghi nối lịch sử| F_History
    FS -->|Ghi nhật ký kiểm toán| F_Log
```

---

## III. SƠ ĐỒ TUẦN TỰ: GIAO DỊCH CHUYỂN TIỀN NGUYÊN TỬ (TRANSFER SEQUENCE)

```mermaid
sequenceDiagram
    autonumber
    actor User as Khách hàng
    participant CV as ConsoleView
    participant UC as UserController
    participant FS as FileService
    participant SenderAcc as Account (Gửi)
    participant RecvAcc as Account (Nhận)
    participant Disk as Tệp Tin Đĩa

    User->>CV: Nhập mã nhận (14 số) và Số tiền
    CV->>UC: Chuyển thông tin giao dịch
    UC->>FS: loadAccount(ReceiverID)
    FS->>Disk: Đọc data/ReceiverID.txt
    Disk-->>FS: Dữ liệu người nhận
    FS-->>UC: Trả về đối tượng RecvAcc
    UC->>CV: Hiển thị xác nhận (Tên người nhận, Số tiền)
    CV->>User: Hỏi xác nhận (y/n)?
    User->>CV: Nhấn 'y' (Xác nhận)
    CV->>UC: Tiếp tục giao dịch

    Note over UC,FS: TOCTOU Prevention: Nạp lại số dư tươi mới nhất
    UC->>FS: loadAccount(ReceiverID, RecvAcc)
    FS-->>UC: Số dư mới nhất từ đĩa

    UC->>SenderAcc: canWithdraw(Amount)
    SenderAcc-->>UC: ERR_NONE (Đủ điều kiện)
    UC->>SenderAcc: withdraw(Amount)
    UC->>RecvAcc: deposit(Amount)

    Note over UC,Disk: Ghi đĩa nguyên tử 2 phía
    UC->>FS: saveAccount(SenderAcc)
    FS->>Disk: atomicWriteFile(SenderID.txt)
    UC->>FS: saveAccount(RecvAcc)
    FS->>Disk: atomicWriteFile(ReceiverID.txt)

    Note over UC,Disk: Ghi vết lịch sử 2 tài khoản
    UC->>FS: appendTransaction(SenderID, SenderTx)
    FS->>Disk: data/LichSuSenderID.txt (std::ios::app)
    UC->>FS: appendTransaction(ReceiverID, RecvTx)
    FS->>Disk: data/LichSuReceiverID.txt (std::ios::app)

    UC->>CV: printReceipt(CHUYEN TIEN, Amount, RemainingBalance)
    CV->>User: In biên lai chuyển tiền thành công
```
