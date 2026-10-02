#include "AtmController.h"
#include "ConsoleView.h"
#include "FileService.h"
#include <iostream>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <sstream>

AtmController::AtmController()
    : _pCurrentAccount(nullptr), _currentUserRole(ROLE_NONE), _bShouldExit(false) {}

AtmController::~AtmController() {
    if (this->_pCurrentAccount != nullptr) {
        delete this->_pCurrentAccount;
        this->_pCurrentAccount = nullptr;
    }
    this->_listAdmins.clear();
    this->_listCards.clear();
    this->_listLockedIds.clear();
}

void AtmController::initSystem() {
    FileService::initMockDataIfMissing();
    FileService::loadAdmins(this->_listAdmins);
    FileService::loadLockedIds(this->_listLockedIds);
    FileService::loadCards(this->_listCards, this->_listLockedIds);
}

bool AtmController::isValidIdFormat(const std::string& strId) {
    if (strId.length() != static_cast<size_t>(ID_LENGTH)) {
        return false;
    }
    for (char ch : strId) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    return true;
}

void AtmController::run() {
    this->initSystem();

    while (true) {
        ConsoleView::printMainMenu();
        int iChoice = ConsoleView::inputMenuChoice(0, 2, "Nhap lua chon cua ban (0-2): ");

        switch (iChoice) {
            case 1:
                this->processAdminLogin();
                break;
            case 2:
                this->processUserLogin();
                // Kiem tra co hieu thoat chuong trinh (doc dong 168: sai 3 lan -> thoat CT)
                if (this->_bShouldExit) {
                    this->_listAdmins.clear();
                    this->_listCards.clear();
                    this->_listLockedIds.clear();
                    if (this->_pCurrentAccount != nullptr) {
                        delete this->_pCurrentAccount;
                        this->_pCurrentAccount = nullptr;
                    }
                    return;
                }
                break;
            case 0:
                ConsoleView::printInfo("Cam on ban da su dung dich vu ATM. Tam biet!");
                this->_listAdmins.clear();
                this->_listCards.clear();
                this->_listLockedIds.clear();
                if (this->_pCurrentAccount != nullptr) {
                    delete this->_pCurrentAccount;
                    this->_pCurrentAccount = nullptr;
                }
                return;
            default:
                ConsoleView::printError("Lua chon khong hop le!");
                break;
        }
    }
}

void AtmController::processAdminLogin() {
    ConsoleView::printHeader("DANG NHAP QUAN TRI VIEN (ADMIN)");
    const int MAX_ADMIN_ATTEMPTS = 3;
    int iAttempts = 0;

    while (iAttempts < MAX_ADMIN_ATTEMPTS) {
        std::string strUser = ConsoleView::inputLine("Ten dang nhap Admin: ");
        if (strUser.empty()) {
            ConsoleView::printWarning("Ten dang nhap khong duoc de trong!");
            continue;
        }

        std::string strPass = ConsoleView::inputPassword("Mat khau Admin: ");

        Admin* pAdmin = this->_listAdmins.findIf([&strUser, &strPass](const Admin& admin) {
            return (admin.getUsername() == strUser && admin.verifyPassword(strPass));
        });

        if (pAdmin != nullptr) {
            this->_currentUserRole = ROLE_ADMIN;
            ConsoleView::printSuccess("Dang nhap thanh cong! Chao mung " + strUser + ".");
            ConsoleView::pauseScreen();
            this->processAdminMenu();
            return;
        } else {
            iAttempts++;
            int iRemaining = MAX_ADMIN_ATTEMPTS - iAttempts;
            ConsoleView::printError("Sai ten dang nhap hoac mat khau!");
            if (iRemaining > 0) {
                ConsoleView::printWarning("Ban con " + std::to_string(iRemaining) + " lan thu.");
            } else {
                ConsoleView::printError("Ban da nhap sai qua 3 lan! Tro ve menu chinh.");
                ConsoleView::pauseScreen();
            }
        }
    }
}

void AtmController::processAdminMenu() {
    while (this->_currentUserRole == ROLE_ADMIN) {
        ConsoleView::printAdminMenu();
        int iChoice = ConsoleView::inputMenuChoice(0, 4, "Nhap lua chon Admin (0-4): ");

        switch (iChoice) {
            case 1:
                this->handleViewCards();
                break;
            case 2:
                this->handleAddCard();
                break;
            case 3:
                this->handleDeleteCard();
                break;
            case 4:
                this->handleUnlockCard();
                break;
            case 0:
                this->_currentUserRole = ROLE_NONE;
                ConsoleView::printSuccess("Dang xuat phan he Admin thanh cong.");
                ConsoleView::pauseScreen();
                return;
            default:
                ConsoleView::printError("Lua chon khong hop le!");
                break;
        }
    }
}

void AtmController::handleViewCards() {
    ConsoleView::displayCardList(this->_listCards, this->_listLockedIds);
    ConsoleView::pauseScreen();
}

void AtmController::handleAddCard() {
    ConsoleView::printHeader("THEM TAI KHOAN THE MOI");
    std::string strId = ConsoleView::inputLine("Nhap ma so tai khoan (ID) 14 chu so: ");

    if (!this->isValidIdFormat(strId)) {
        ConsoleView::printError("ID phai dung 14 chu so khong chua ky tu dac biet!");
        ConsoleView::pauseScreen();
        return;
    }

    Card* pExist = this->_listCards.findIf([&strId](const Card& c) {
        return c.getId() == strId;
    });

    if (pExist != nullptr) {
        ConsoleView::printError("Ma the " + strId + " da ton tai trong he thong!");
        ConsoleView::pauseScreen();
        return;
    }

    std::string strName = ConsoleView::inputLine("Nhap ho va ten chu the: ");
    if (strName.empty()) {
        strName = "Khach Hang Moi";
    }

    long lBalance = ConsoleView::inputMoney("Nhap so du ban dau (toi thieu 50000 VND, boi so 50000): ");
    if (lBalance < MIN_TRANSACTION || (lBalance % MIN_TRANSACTION != 0)) {
        ConsoleView::printError("So du ban dau phai >= 50000 VND va la boi so cua 50000 VND!");
        ConsoleView::pauseScreen();
        return;
    }

    Card newCard(strId, DEFAULT_PIN);
    this->_listCards.addTail(newCard);

    FileService::saveCards(this->_listCards);
    FileService::createAccountFiles(strId, strName, lBalance, "VND");

    ConsoleView::printSuccess("Them the thanh cong! Ma the: " + strId);
    ConsoleView::printInfo("Ma PIN mac dinh duoc cap la: " + DEFAULT_PIN);
    ConsoleView::printInfo("Da tu dong tao file du lieu " + strId + ".txt va LichSu" + strId + ".txt");
    ConsoleView::pauseScreen();
}

void AtmController::handleDeleteCard() {
    ConsoleView::printHeader("XOA TAI KHOAN THE");
    std::string strId = ConsoleView::inputLine("Nhap ID the can xoa (14 so): ");

    Card* pExist = this->_listCards.findIf([&strId](const Card& c) {
        return c.getId() == strId;
    });

    if (pExist == nullptr) {
        ConsoleView::printError("Khong tim thay the voi ID: " + strId);
        ConsoleView::pauseScreen();
        return;
    }

    std::string strConfirm = ConsoleView::inputLine("Ban co chac chan muon xoa the nay? (y/n): ");
    if (strConfirm != "y" && strConfirm != "Y") {
        ConsoleView::printInfo("Da huy thao tac xoa the.");
        ConsoleView::pauseScreen();
        return;
    }

    this->_listCards.removeIf([&strId](const Card& c) {
        return c.getId() == strId;
    });

    this->_listLockedIds.removeIf([&strId](const std::string& item) {
        return item == strId;
    });

    FileService::saveCards(this->_listCards);
    FileService::saveLockedIds(this->_listLockedIds);
    FileService::deleteAccountFile(strId);

    ConsoleView::printSuccess("Da xoa the " + strId + " khoi he thong va xoa file du lieu tai khoan.");
    ConsoleView::printInfo("Lich su giao dich LichSu" + strId + ".txt duoc giu nguyen theo quy dinh kiem toan.");
    ConsoleView::pauseScreen();
}

void AtmController::handleUnlockCard() {
    ConsoleView::printHeader("MO KHOA TAI KHOAN THE");

    if (this->_listLockedIds.isEmpty()) {
        ConsoleView::printInfo("Hien tai khong co the nao bi khoa trong he thong.");
        ConsoleView::pauseScreen();
        return;
    }

    std::cout << "Danh sach the dang bi khoa:\n";
    Node<std::string>* pCur = this->_listLockedIds.getHead();
    int iIdx = 1;
    while (pCur != nullptr) {
        std::cout << "  " << iIdx++ << ". ID: " << pCur->_data << "\n";
        pCur = pCur->_pNext;
    }

    std::string strId = ConsoleView::inputLine("Nhap ID the can mo khoa: ");
    std::string* pLocked = this->_listLockedIds.findIf([&strId](const std::string& item) {
        return item == strId;
    });

    if (pLocked == nullptr) {
        ConsoleView::printError("The " + strId + " khong nam trong danh sach the bi khoa!");
        ConsoleView::pauseScreen();
        return;
    }

    Card* pCard = this->_listCards.findIf([&strId](const Card& c) {
        return c.getId() == strId;
    });

    if (pCard != nullptr) {
        pCard->resetFailedAttempts();
    }

    this->_listLockedIds.removeIf([&strId](const std::string& item) {
        return item == strId;
    });

    FileService::saveCards(this->_listCards);
    FileService::saveLockedIds(this->_listLockedIds);

    ConsoleView::printSuccess("Mo khoa the " + strId + " thanh cong! So lan nhap sai da duoc dat lai ve 0.");
    ConsoleView::pauseScreen();
}


// =====================================================================
//  TIEN ICH HE THONG
// =====================================================================

bool AtmController::isValidPinFormat(const std::string& strPin) {
    if (strPin.length() != static_cast<size_t>(PIN_LENGTH)) {
        return false;
    }
    for (char ch : strPin) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    return true;
}

std::string AtmController::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t tNow = std::chrono::system_clock::to_time_t(now);
    struct tm* pTm = std::localtime(&tNow);
    std::ostringstream oss;
    oss << std::put_time(pTm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// =====================================================================
//  LUONG DANG NHAP KHACH HANG (USER LOGIN FLOW) - Phase 3 Task A
// =====================================================================

void AtmController::processUserLogin() {
    ConsoleView::printHeader("DANG NHAP KHACH HANG (USER)");

    // Buoc 1: Nhap ID the (14 so)
    std::string strId = ConsoleView::inputLine("Nhap ma so the (ID - 14 chu so): ");
    if (!AtmController::isValidIdFormat(strId)) {
        ConsoleView::printError("ID the khong hop le! Phai la 14 chu so.");
        ConsoleView::pauseScreen();
        return;
    }

    // Buoc 2: Tim the trong he thong
    Card* pCard = this->_listCards.findIf([&strId](const Card& c) {
        return c.getId() == strId;
    });

    if (pCard == nullptr) {
        ConsoleView::printError("Ma the " + strId + " khong ton tai trong he thong!");
        ConsoleView::pauseScreen();
        return;
    }

    // Buoc 3: Kiem tra trang thai khoa the
    bool bAlreadyLocked = pCard->isLocked();
    if (!bAlreadyLocked) {
        const std::string* pLockedId = this->_listLockedIds.findIf([&strId](const std::string& item) {
            return item == strId;
        });
        if (pLockedId != nullptr) {
            bAlreadyLocked = true;
        }
    }

    if (bAlreadyLocked) {
        ConsoleView::printError("The " + strId + " dang bi khoa do nhap sai PIN qua " +
                                std::to_string(MAX_FAILED_LOGINS) + " lan!");
        ConsoleView::printWarning("Vui long lien he Quan tri vien (Admin) de mo khoa.");
        ConsoleView::pauseScreen();
        return;
    }

    // Buoc 4: Vong lap nhap PIN (toi da MAX_FAILED_LOGINS lan)
    bool bLoginSuccess = false;
    while (pCard->getFailedAttempts() < MAX_FAILED_LOGINS) {
        int iRemaining = MAX_FAILED_LOGINS - pCard->getFailedAttempts();
        std::string strPin = ConsoleView::inputPin("Nhap ma PIN (6 so) [con " + std::to_string(iRemaining) + " lan]: ");

        if (pCard->checkPin(strPin)) {
            bLoginSuccess = true;
            break;
        } else {
            pCard->recordFailedAttempt();
            if (pCard->isLocked()) {
                // Ghi vao file KhoaThe.txt de Admin mo khoa sau
                this->_listLockedIds.addTail(strId);
                FileService::saveLockedIds(this->_listLockedIds);
                FileService::saveCards(this->_listCards);
                ConsoleView::printError("Nhap sai PIN qua " + std::to_string(MAX_FAILED_LOGINS) +
                                        " lan! The " + strId + " da bi KHOA TU DONG.");
                ConsoleView::printWarning("Vui long lien he Admin de mo khoa the.");
                ConsoleView::printInfo("Chuong trinh se dong. Tam biet!");
                ConsoleView::pauseScreen();
                // Tuan thu doc dong 168: "thoat chuong trinh" sau khi khoa the
                this->_bShouldExit = true;
                return;
            } else {
                int iLeft = MAX_FAILED_LOGINS - pCard->getFailedAttempts();
                ConsoleView::printError("PIN sai! Con " + std::to_string(iLeft) + " lan thu.");
            }
        }
    }

    if (!bLoginSuccess) {
        return;
    }

    // Buoc 5: Kiem tra PIN mac dinh - bat buoc doi PIN
    if (pCard->isDefaultPin()) {
        ConsoleView::printWarning("Ban dang su dung ma PIN mac dinh (123456). Phai doi PIN truoc khi vao he thong!");
        bool bPinChanged = false;
        while (!bPinChanged) {
            std::string strNewPin1 = ConsoleView::inputPin("Nhap ma PIN moi (6 so): ");
            if (!AtmController::isValidPinFormat(strNewPin1)) {
                ConsoleView::printError("PIN phai la dung 6 chu so!");
                continue;
            }
            if (strNewPin1 == DEFAULT_PIN) {
                ConsoleView::printError("PIN moi khong duoc trung voi PIN mac dinh (123456)!");
                continue;
            }
            if (strNewPin1 == pCard->getPin()) {
                ConsoleView::printError("PIN moi phai khac PIN cu!");
                continue;
            }
            std::string strNewPin2 = ConsoleView::inputPin("Nhap lai ma PIN moi de xac nhan: ");
            if (strNewPin1 != strNewPin2) {
                ConsoleView::printError("Hai lan nhap PIN khong khop! Vui long thu lai.");
                continue;
            }
            pCard->changePin(strNewPin1);
            FileService::saveCards(this->_listCards);
            ConsoleView::printSuccess("Doi PIN thanh cong! Dang chuyen vao he thong...");
            bPinChanged = true;
        }
    }

    // Buoc 6: Reset so lan nhap sai sau khi dang nhap thanh cong
    pCard->resetFailedAttempts();

    // Buoc 7: Tai thong tin tai khoan len RAM
    this->_pCurrentAccount = new Account();
    ErrorCode errLoad = FileService::loadAccount(strId, *this->_pCurrentAccount);
    if (errLoad != ERR_NONE) {
        ConsoleView::printError("Khong the tai du lieu tai khoan! File bi loi hoac khong ton tai.");
        delete this->_pCurrentAccount;
        this->_pCurrentAccount = nullptr;
        ConsoleView::pauseScreen();
        return;
    }

    this->_currentUserRole = ROLE_USER;
    ConsoleView::printSuccess("Dang nhap thanh cong! Chao mung " + this->_pCurrentAccount->getName() + ".");
    ConsoleView::pauseScreen();

    // Buoc 8: Vao Menu User
    this->processUserMenu(strId);
}

// =====================================================================
//  MENU CHINH KHACH HANG - Phase 3
// =====================================================================

void AtmController::processUserMenu(const std::string& strId) {
    while (this->_currentUserRole == ROLE_USER) {
        ConsoleView::printUserMenu();
        int iChoice = ConsoleView::inputMenuChoice(0, 5, "Nhap lua chon (0-5): ");

        switch (iChoice) {
            case 1:
                this->handleViewAccountInfo();
                break;
            case 2:
                this->handleWithdraw();
                break;
            case 3:
                this->handleTransfer();
                break;
            case 4:
                this->handleViewHistory();
                break;
            case 5:
                this->handleChangePin();
                break;
            case 0:
                // Giai phong bo nho tai khoan truoc khi dang xuat (Session Isolation)
                if (this->_pCurrentAccount != nullptr) {
                    delete this->_pCurrentAccount;
                    this->_pCurrentAccount = nullptr;
                }
                this->_currentUserRole = ROLE_NONE;
                ConsoleView::printSuccess("Tra the thanh cong. Cam on ban da su dung dich vu ATM!");
                ConsoleView::pauseScreen();
                return;
            default:
                ConsoleView::printError("Lua chon khong hop le!");
                break;
        }
    }
    (void)strId; // Suppress unused warning
}

// =====================================================================
//  CHUC NANG 1: XEM THONG TIN TAI KHOAN
// =====================================================================

void AtmController::handleViewAccountInfo() {
    if (this->_pCurrentAccount == nullptr) {
        ConsoleView::printError("Phien dang nhap loi! Vui long dang nhap lai.");
        this->_currentUserRole = ROLE_NONE;
        return;
    }

    // Reload so du moi nhat tu file
    FileService::loadAccount(this->_pCurrentAccount->getId(), *this->_pCurrentAccount);

    ConsoleView::printHeader("THONG TIN TAI KHOAN");
    std::cout << "  Ma so tai khoan : " << this->_pCurrentAccount->getId()     << "\n";
    std::cout << "  Ho va ten       : " << this->_pCurrentAccount->getName()    << "\n";
    std::cout << "  So du hien tai  : " << this->_pCurrentAccount->getBalance() << " "
              << this->_pCurrentAccount->getCurrency() << "\n";
    std::cout << "------------------------------------------------------\n";
    ConsoleView::pauseScreen();
}

// =====================================================================
//  CHUC NANG 2: RUT TIEN - Phase 3 Task C
// =====================================================================

void AtmController::handleWithdraw() {
    if (this->_pCurrentAccount == nullptr) {
        ConsoleView::printError("Phien dang nhap loi!");
        this->_currentUserRole = ROLE_NONE;
        return;
    }

    // Reload so du moi nhat truoc khi giao dich
    FileService::loadAccount(this->_pCurrentAccount->getId(), *this->_pCurrentAccount);

    ConsoleView::printHeader("RUT TIEN");
    std::cout << "  So du hien tai: " << this->_pCurrentAccount->getBalance()
              << " " << this->_pCurrentAccount->getCurrency() << "\n";
    std::cout << "  So tien co the rut toi da: "
              << (this->_pCurrentAccount->getBalance() - MIN_BALANCE_RESERVE) << " VND\n\n";

    // Vong lap cho phep nhap lai hoac huy (tuan thu doc dong 198)
    long lAmount = 0;
    bool bValidAmount = false;
    while (!bValidAmount) {
        lAmount = ConsoleView::inputMoney("Nhap so tien muon rut (boi so 50,000 VND, 0 = Huy): ");
        if (lAmount == 0) {
            ConsoleView::printInfo("Da huy giao dich rut tien.");
            ConsoleView::pauseScreen();
            return;
        }

        ErrorCode errCheck = this->_pCurrentAccount->canWithdraw(lAmount);
        switch (errCheck) {
            case ERR_INVALID_AMOUNT:
                ConsoleView::printError("So tien rut toi thieu la 50,000 VND!");
                break;
            case ERR_NOT_MULTIPLE:
                ConsoleView::printError("So tien phai la boi so cua 50,000 VND (vd: 50000, 100000, 250000...)!");
                break;
            case ERR_INSUFFICIENT_FUNDS:
                ConsoleView::printError("So du khong du! Phai duy tri toi thieu 50,000 VND.");
                ConsoleView::printInfo("So tien toi da co the rut: " +
                    std::to_string(this->_pCurrentAccount->getBalance() - MIN_BALANCE_RESERVE) + " VND");
                break;
            case ERR_NONE:
                bValidAmount = true;
                break;
            default:
                ConsoleView::printError("Loi khong xac dinh!");
                break;
        }
        if (!bValidAmount) {
            if (!ConsoleView::confirmAction("Ban co muon nhap lai so tien khong")) {
                ConsoleView::printInfo("Da huy giao dich.");
                ConsoleView::pauseScreen();
                return;
            }
        }
    }

    // Xac nhan giao dich truoc khi thuc hien
    std::string strConfirmMsg = "Xac nhan rut " + std::to_string(lAmount) + " VND";
    if (!ConsoleView::confirmAction(strConfirmMsg)) {
        ConsoleView::printInfo("Da huy giao dich rut tien.");
        ConsoleView::pauseScreen();
        return;
    }

    // Thuc hien rut tien trong RAM
    this->_pCurrentAccount->withdraw(lAmount);

    // Ghi file [ID].txt
    if (!FileService::saveAccount(*this->_pCurrentAccount)) {
        // Rollback RAM neu ghi file that bai
        this->_pCurrentAccount->deposit(lAmount);
        ConsoleView::printError("Loi he thong! Khong the ghi file. Giao dich bi huy an toan.");
        ConsoleView::pauseScreen();
        return;
    }

    // Ghi lich su rut tien vao [LichSuID].txt
    std::string strTimestamp = AtmController::getCurrentTimestamp();
    Transaction trans(this->_pCurrentAccount->getId(), WITHDRAW, lAmount, strTimestamp,
                      "Rut tien tai may ATM");
    FileService::appendTransaction(this->_pCurrentAccount->getId(), trans);

    ConsoleView::printSuccess("Rut tien thanh cong! " + std::to_string(lAmount) + " VND da duoc giao.");
    ConsoleView::printInfo("So du con lai: " + std::to_string(this->_pCurrentAccount->getBalance()) +
                           " " + this->_pCurrentAccount->getCurrency());
    ConsoleView::pauseScreen();
}

// =====================================================================
//  CHUC NANG 3: CHUYEN TIEN - Phase 3 (Atomicity - Pair B&C)
// =====================================================================

void AtmController::handleTransfer() {
    if (this->_pCurrentAccount == nullptr) {
        ConsoleView::printError("Phien dang nhap loi!");
        this->_currentUserRole = ROLE_NONE;
        return;
    }

    // Reload so du moi nhat
    FileService::loadAccount(this->_pCurrentAccount->getId(), *this->_pCurrentAccount);

    ConsoleView::printHeader("CHUYEN TIEN");
    std::cout << "  So du hien tai: " << this->_pCurrentAccount->getBalance()
              << " " << this->_pCurrentAccount->getCurrency() << "\n\n";

    // Buoc 1: Nhap ID tai khoan nhan
    std::string strReceiverId = ConsoleView::inputLine("Nhap ma so tai khoan nhan (14 chu so): ");
    if (!AtmController::isValidIdFormat(strReceiverId)) {
        ConsoleView::printError("Ma so tai khoan nhan khong hop le! Phai la 14 chu so.");
        ConsoleView::pauseScreen();
        return;
    }

    // Khong the chuyen tien cho chinh minh
    if (strReceiverId == this->_pCurrentAccount->getId()) {
        ConsoleView::printError("Khong the chuyen tien cho chinh tai khoan cua ban!");
        ConsoleView::pauseScreen();
        return;
    }

    // Kiem tra tai khoan nhan co ton tai trong he thong (TheTu.txt)
    Card* pReceiverCard = this->_listCards.findIf([&strReceiverId](const Card& c) {
        return c.getId() == strReceiverId;
    });
    if (pReceiverCard == nullptr) {
        ConsoleView::printError("Ma so tai khoan " + strReceiverId + " khong ton tai trong he thong!");
        ConsoleView::pauseScreen();
        return;
    }

    // Tai thong tin tai khoan nhan
    Account receiverAccount;
    ErrorCode errReceiver = FileService::loadAccount(strReceiverId, receiverAccount);
    if (errReceiver != ERR_NONE) {
        ConsoleView::printError("Khong the tai thong tin tai khoan nhan. File bi loi!");
        ConsoleView::pauseScreen();
        return;
    }

    // Hien thi thong tin nguoi nhan de xac nhan truoc
    ConsoleView::printInfo("Tai khoan nhan: " + receiverAccount.getName() + " (ID: " + strReceiverId + ")");
    std::cout << "  So du cua ban  : " << this->_pCurrentAccount->getBalance()
              << " " << this->_pCurrentAccount->getCurrency() << "\n";
    std::cout << "  Toi da chuyen  : "
              << (this->_pCurrentAccount->getBalance() - MIN_BALANCE_RESERVE) << " VND\n\n";

    // Vong lap nhap so tien: cho phep nhap lai hoac huy (tuan thu doc dong 198)
    long lAmount = 0;
    bool bValidTransfer = false;
    while (!bValidTransfer) {
        lAmount = ConsoleView::inputMoney("Nhap so tien chuyen (boi so 50,000 VND, 0 = Huy): ");
        if (lAmount == 0) {
            ConsoleView::printInfo("Da huy giao dich chuyen tien.");
            ConsoleView::pauseScreen();
            return;
        }

        ErrorCode errCheck = this->_pCurrentAccount->canWithdraw(lAmount);
        switch (errCheck) {
            case ERR_INVALID_AMOUNT:
                ConsoleView::printError("So tien chuyen toi thieu la 50,000 VND!");
                break;
            case ERR_NOT_MULTIPLE:
                ConsoleView::printError("So tien phai la boi so cua 50,000 VND!");
                break;
            case ERR_INSUFFICIENT_FUNDS:
                ConsoleView::printError("So du khong du! Phai duy tri toi thieu 50,000 VND.");
                ConsoleView::printInfo("So tien toi da co the chuyen: " +
                    std::to_string(this->_pCurrentAccount->getBalance() - MIN_BALANCE_RESERVE) + " VND");
                break;
            case ERR_NONE:
                bValidTransfer = true;
                break;
            default:
                ConsoleView::printError("Loi khong xac dinh!");
                break;
        }
        if (!bValidTransfer) {
            if (!ConsoleView::confirmAction("Ban co muon nhap lai so tien khong")) {
                ConsoleView::printInfo("Da huy giao dich.");
                ConsoleView::pauseScreen();
                return;
            }
        }
    }

    // Xac nhan giao dich lan cuoi
    std::string strConfirmMsg = "Xac nhan chuyen " + std::to_string(lAmount) +
                                " VND cho " + receiverAccount.getName();
    if (!ConsoleView::confirmAction(strConfirmMsg)) {
        ConsoleView::printInfo("Da huy giao dich chuyen tien.");
        ConsoleView::pauseScreen();
        return;
    }

    // ===== ATOMICITY: Tru/Cong trong RAM truoc, ghi file sau =====
    this->_pCurrentAccount->withdraw(lAmount);
    receiverAccount.deposit(lAmount);

    std::string strTimestamp = AtmController::getCurrentTimestamp();

    // Ghi file nguoi gui [ID_Gui].txt
    bool bSaveSender = FileService::saveAccount(*this->_pCurrentAccount);
    if (!bSaveSender) {
        // Rollback RAM
        this->_pCurrentAccount->deposit(lAmount);
        ConsoleView::printError("Loi he thong! Khong ghi duoc file nguoi gui. Giao dich bi huy.");
        ConsoleView::pauseScreen();
        return;
    }

    // Ghi file nguoi nhan [ID_Nhan].txt
    bool bSaveReceiver = FileService::saveAccount(receiverAccount);
    if (!bSaveReceiver) {
        // Rollback ca hai tai khoan trong RAM va file gui
        this->_pCurrentAccount->deposit(lAmount);
        FileService::saveAccount(*this->_pCurrentAccount); // Khoi phuc file nguoi gui
        ConsoleView::printError("Loi he thong! Khong ghi duoc file nguoi nhan. Giao dich da bi huy hoan toan.");
        ConsoleView::pauseScreen();
        return;
    }

    // Ghi lich su nguoi gui [LichSuID_Gui].txt
    Transaction transSender(this->_pCurrentAccount->getId(), TRANSFER, lAmount, strTimestamp,
                            "Chuyen tien cho TK: " + strReceiverId + " (" + receiverAccount.getName() + ")");
    FileService::appendTransaction(this->_pCurrentAccount->getId(), transSender);

    // Ghi lich su nguoi nhan [LichSuID_Nhan].txt
    Transaction transReceiver(strReceiverId, RECEIVE, lAmount, strTimestamp,
                              "Nhan tien tu TK: " + this->_pCurrentAccount->getId() +
                              " (" + this->_pCurrentAccount->getName() + ")");
    FileService::appendTransaction(strReceiverId, transReceiver);

    ConsoleView::printSuccess("Chuyen tien thanh cong!");
    ConsoleView::printInfo("Da chuyen " + std::to_string(lAmount) + " VND cho " + receiverAccount.getName());
    ConsoleView::printInfo("So du con lai: " + std::to_string(this->_pCurrentAccount->getBalance()) +
                           " " + this->_pCurrentAccount->getCurrency());
    ConsoleView::pauseScreen();
}

// =====================================================================
//  CHUC NANG 4: XEM LICH SU GIAO DICH
// =====================================================================

void AtmController::handleViewHistory() {
    if (this->_pCurrentAccount == nullptr) {
        ConsoleView::printError("Phien dang nhap loi!");
        this->_currentUserRole = ROLE_NONE;
        return;
    }

    ConsoleView::printHeader("LICH SU GIAO DICH - TK: " + this->_pCurrentAccount->getId());

    LinkedList<Transaction> listTrans;
    bool bLoaded = FileService::loadTransactions(this->_pCurrentAccount->getId(), listTrans);

    if (!bLoaded || listTrans.isEmpty()) {
        ConsoleView::printInfo("Chua co giao dich nao duoc ghi nhan.");
    } else {
        int iIndex = 1;
        Node<Transaction>* pCur = listTrans.getHead();
        while (pCur != nullptr) {
            std::cout << "  " << iIndex++ << ". " << pCur->_data.toString() << "\n";
            pCur = pCur->_pNext;
        }
        std::cout << "------------------------------------------------------\n";
        std::cout << "Tong so giao dich: " << listTrans.getSize() << "\n";
    }

    ConsoleView::pauseScreen();
}

// =====================================================================
//  CHUC NANG 5: DOI MA PIN
// =====================================================================

void AtmController::handleChangePin() {
    if (this->_pCurrentAccount == nullptr) {
        ConsoleView::printError("Phien dang nhap loi!");
        this->_currentUserRole = ROLE_NONE;
        return;
    }

    ConsoleView::printHeader("DOI MA PIN");

    // Tim lai Card object cua user hien tai
    const std::string strCurrentId = this->_pCurrentAccount->getId();
    Card* pCard = this->_listCards.findIf([&strCurrentId](const Card& c) {
        return c.getId() == strCurrentId;
    });

    if (pCard == nullptr) {
        ConsoleView::printError("Loi he thong! Khong tim thay thong tin the.");
        ConsoleView::pauseScreen();
        return;
    }

    // Buoc 1: Xac thuc PIN cu
    std::string strOldPin = ConsoleView::inputPin("Nhap ma PIN hien tai (6 so): ");
    if (!pCard->checkPin(strOldPin)) {
        ConsoleView::printError("Ma PIN hien tai khong chinh xac!");
        ConsoleView::pauseScreen();
        return;
    }

    // Buoc 2: Nhap PIN moi va xac nhan
    while (true) {
        std::string strNewPin1 = ConsoleView::inputPin("Nhap ma PIN moi (6 so): ");
        if (!AtmController::isValidPinFormat(strNewPin1)) {
            ConsoleView::printError("PIN moi phai la dung 6 chu so!");
            continue;
        }
        if (strNewPin1 == DEFAULT_PIN) {
            ConsoleView::printError("Khong duoc doi ve PIN mac dinh (123456)!");
            continue;
        }
        if (strNewPin1 == strOldPin) {
            ConsoleView::printError("PIN moi phai khac PIN cu!");
            continue;
        }

        std::string strNewPin2 = ConsoleView::inputPin("Nhap lai ma PIN moi de xac nhan: ");
        if (strNewPin1 != strNewPin2) {
            ConsoleView::printError("Hai lan nhap PIN khong khop! Vui long thu lai.");
            continue;
        }

        // Cap nhat PIN trong RAM va file
        pCard->changePin(strNewPin1);
        if (!FileService::saveCards(this->_listCards)) {
            ConsoleView::printError("Loi ghi file! Khong the luu PIN moi. Thu lai sau.");
            pCard->changePin(strOldPin); // Rollback
        } else {
            ConsoleView::printSuccess("Doi ma PIN thanh cong!");
        }
        break;
    }

    ConsoleView::pauseScreen();
}
