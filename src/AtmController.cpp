#include "AtmController.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <filesystem>

AtmController::AtmController()
    : _pCurrentAccount(nullptr),
      _pCurrentCard(nullptr),
      _eCurrentRole(ROLE_NONE) {}

AtmController::~AtmController() {
    this->cleanupSession();
}

bool AtmController::initData() {
    // Dam bao thu muc va du lieu mau luon ton tai
    FileService::initSampleData();

    this->_listAdmins.clear();
    this->_listCards.clear();
    this->_listLockedIds.clear();

    bool bAdminsOk = FileService::loadAdmins(this->_listAdmins);
    bool bLockedOk = FileService::loadLockedIds(this->_listLockedIds);
    bool bCardsOk = FileService::loadCards(this->_listCards, this->_listLockedIds);

    return (bAdminsOk && bLockedOk && bCardsOk);
}

void AtmController::run() {
    this->initData();

    bool bRunning = true;
    while (bRunning) {
        ConsoleView::clearScreen();
        ConsoleView::printMainMenu();

        int iChoice = ConsoleView::inputMenuChoice(0, 2, "Nhap lua chon cua ban [0-2]: ");
        switch (iChoice) {
            case 1: {
                this->processAdminLogin();
                break;
            }
            case 2: {
                this->processUserLogin();
                break;
            }
            case 0: {
                ConsoleView::printHeader("CAM ON QUY KHACH DA SU DUNG DICH VU ATM!");
                std::cout << "  He thong da dong ket noi an toan.\n";
                this->cleanupSession();
                bRunning = false;
                break;
            }
            default: {
                break;
            }
        }
    }
}

void AtmController::cleanupSession() {
    if (this->_pCurrentAccount != nullptr) {
        delete this->_pCurrentAccount;
        this->_pCurrentAccount = nullptr;
    }
    this->_pCurrentCard = nullptr;
    this->_eCurrentRole = ROLE_NONE;
}

//ADMIN MODULE
bool AtmController::authenticateAdmin(const std::string& strUser, const std::string& strPass) const {
    if (strUser.empty() || strPass.empty()) {
        return false;
    }

    auto pCur = this->_listAdmins.getHead();
    while (pCur != nullptr) {
        if (pCur->_data.getUsername() == strUser && pCur->_data.verifyPassword(strPass)) {
            return true;
        }
        pCur = pCur->_pNext;
    }
    return false;
}

void AtmController::processAdminLogin() {
    ConsoleView::clearScreen();
    ConsoleView::printHeader("DANG NHAP QUAN TRI VIEN (ADMIN)");

    std::string strUser = ConsoleView::inputLine("Ten dang nhap Admin (Enter hoac 0 de huy): ");
    if (strUser.empty() || strUser == "0") {
        ConsoleView::printInfo("Da huy thao tac dang nhap Quan tri vien.");
        ConsoleView::pauseScreen();
        return;
    }
    std::string strPass = ConsoleView::inputPassword("Mat khau Admin (Enter hoac 0 de huy): ");
    if (strPass.empty() || strPass == "0") {
        ConsoleView::printInfo("Da huy thao tac dang nhap Quan tri vien.");
        ConsoleView::pauseScreen();
        return;
    }

    if (this->authenticateAdmin(strUser, strPass)) {
        FileService::appendAdminLog("ADMIN_LOGIN_SUCCESS", "Admin dang nhap thanh cong: " + strUser);
        ConsoleView::printSuccess("Dang nhap Quan tri vien thanh cong!");
        this->_eCurrentRole = ROLE_ADMIN;
        ConsoleView::pauseScreen();

        // Tich hop module AdminController vao luong he thong chinh
        AdminController adminCtrl;
        adminCtrl.loadAllData();
        adminCtrl.processAdminMenu();

        // Dong bo lai du lieu cua AtmController sau khi cap nhat
        this->initData();
        this->cleanupSession();
    } else {
        FileService::appendAdminLog("ADMIN_LOGIN_FAIL", "Dang nhap Admin that bai: " + strUser);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ConsoleView::printError("Ten dang nhap hoac mat khau Admin khong chinh xac!");
        ConsoleView::pauseScreen();
    }
}

void AtmController::processAdminMenu() {
    bool bInAdminMenu = true;
    while (bInAdminMenu && this->_eCurrentRole == ROLE_ADMIN) {
        ConsoleView::clearScreen();
        ConsoleView::printAdminMenu();

        int iChoice = ConsoleView::inputMenuChoice(0, 4, "Chon chuc nang Admin [0-4]: ");
        switch (iChoice) {
            case 1: {
                this->adminViewCards();
                ConsoleView::pauseScreen();
                break;
            }
            case 2: {
                this->adminAddCard();
                ConsoleView::pauseScreen();
                break;
            }
            case 3: {
                this->adminDeleteCard();
                ConsoleView::pauseScreen();
                break;
            }
            case 4: {
                this->adminUnlockCard();
                ConsoleView::pauseScreen();
                break;
            }
            case 0: {
                ConsoleView::printInfo("Dang dang xuat khoi phan he Quan tri vien...");
                this->cleanupSession();
                bInAdminMenu = false;
                ConsoleView::pauseScreen();
                break;
            }
            default: {
                break;
            }
        }
    }
}

void AtmController::adminViewCards() {
    ConsoleView::clearScreen();
    ConsoleView::displayCardList(this->_listCards, this->_listLockedIds);
}

ErrorCode AtmController::addCardAccount(const std::string& strId,
                                        const std::string& strName,
                                        long lInitialBalance,
                                        const std::string& strCurrency) {
    // 1. Kiem tra dinh dang ID 
    if (strId.length() != static_cast<size_t>(ID_LENGTH)) {
        return ERR_INVALID_FORMAT;
    }
    for (char c : strId) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return ERR_INVALID_FORMAT;
        }
    }

    // 2. Kiem tra ten chu the
    size_t iStart = strName.find_first_not_of(" \t\r\n");
    if (iStart == std::string::npos) {
        return ERR_INVALID_FORMAT;
    }
    if (strName.find('|') != std::string::npos ||
        strName.find('\n') != std::string::npos ||
        strName.find('\r') != std::string::npos) {
        return ERR_INVALID_FORMAT;
    }

    // 3. Kiem tra trung lap ma ID trong he thong hoac tap tin tai khoan vo chu tren dia
    auto pExisting = this->_listCards.findIf([&](const Card& c) {
        return c.getId() == strId;
    });
    if (pExisting != nullptr) {
        return ERR_ID_EXISTS;
    }
    if (std::filesystem::exists(DATA_DIR + strId + ".txt")) {
        return ERR_ID_EXISTS;
    }

    // 4. Kiem tra so du ban dau theo cau hinh don vi tien te
    CurrencyConfig cfg = getCurrencyConfig(strCurrency);
    if (lInitialBalance < cfg.lMinReserve || lInitialBalance > cfg.lMaxBalance) {
        return ERR_INVALID_AMOUNT;
    }
    if (lInitialBalance % cfg.lMinTransaction != 0) {
        return ERR_NOT_MULTIPLE;
    }

    // 5. Tao 2 tap tin vat ly tren dia: data/[ID].txt va data/LichSu[ID].txt
    bool bFileCreated = FileService::createAccountFiles(strId, strName, lInitialBalance, strCurrency);
    if (!bFileCreated) {
        return ERR_FILE_NOT_FOUND;
    }

    // 6. Them doi tuong Card moi vao LinkedList trong RAM voi PIN mac dinh (123456)
    Card newCard(strId, DEFAULT_PIN, false);
    this->_listCards.addTail(newCard);

    // 7. Ghi de cap nhat danh sach the vao data/TheTu.txt
    bool bSaved = FileService::saveCards(this->_listCards);
    if (!bSaved) {
        return ERR_FILE_NOT_FOUND;
    }

    // 8. Ghi nhat ky kiem toan quan tri (Admin Audit Log)
    FileService::appendAdminLog("ADD_CARD", "Them the " + strId + " (" + strName + "), So du: " + std::to_string(lInitialBalance) + " " + strCurrency);

    return ERR_NONE;
}

void AtmController::adminAddCard() {
    ConsoleView::clearScreen();
    ConsoleView::printHeader("THEM TAI KHOAN THE TU MOI");

    std::string strId = ConsoleView::inputLine("Nhap ma so the moi (dung 14 chu so): ");
    std::string strName = ConsoleView::inputLine("Nhap ho va ten chu the: ");
    long lBalance = ConsoleView::inputMoney("Nhap so du ban dau (>= 50,000 VND, boi so 50k): ");

    ErrorCode err = this->addCardAccount(strId, strName, lBalance, "VND");
    switch (err) {
        case ERR_NONE: {
            ConsoleView::printSuccess("Them tai khoan the moi thanh cong!");
            std::cout << "  --------------------------------------------------\n";
            std::cout << "  Ma so the ID    : " << strId << "\n";
            std::cout << "  Ma PIN khoi tao : " << DEFAULT_PIN << " (Bat buoc doi khi dang nhap)\n";
            std::cout << "  Chu tai khoan   : " << strName << "\n";
            std::cout << "  So du ban dau   : " << lBalance << " VND\n";
            std::cout << "  --------------------------------------------------\n";
            break;
        }
        case ERR_INVALID_FORMAT: {
            ConsoleView::printError("Dinh dang thong tin khong hop le (ID phai gom 14 chu so, ten khong rong)!");
            break;
        }
        case ERR_ID_EXISTS: {
            ConsoleView::printError("Ma so the " + strId + " da ton tai trong he thong!");
            break;
        }
        case ERR_INVALID_AMOUNT: {
            ConsoleView::printError("So du ban dau khong duoc nho hon so du toi thieu (50,000 VND)!");
            break;
        }
        case ERR_NOT_MULTIPLE: {
            ConsoleView::printError("So du ban dau phai la boi so cua 50,000 VND!");
            break;
        }
        default: {
            ConsoleView::printError("Loi I/O he thong khi khoi tao tap tin luu tru!");
            break;
        }
    }
}

ErrorCode AtmController::deleteCardAccount(const std::string& strId) {
    // 1. Tim xem the co ton tai khong
    auto pCard = this->_listCards.findIf([&](const Card& c) {
        return c.getId() == strId;
    });
    if (pCard == nullptr) {
        return ERR_ID_NOT_FOUND;
    }

    // Doc thong tin so du truoc khi xoa de ghi log kiem toan
    Account acc;
    long lBal = 0;
    std::string strAccName = "";
    if (FileService::loadAccount(strId, acc) == ERR_NONE) {
        lBal = acc.getBalance();
        strAccName = acc.getName();
    }

    // 2. Xoa the khoi danh sach the trong RAM (xoa moi ban sao neu co)
    while (this->_listCards.removeIf([&](const Card& c) {
        return c.getId() == strId;
    })) {}

    // 3. Neu the dang trong danh sach khoa, xoa khoi danh sach khoa trong RAM
    bool bWasLocked = false;
    while (this->_listLockedIds.removeIf([&](const std::string& id) {
        return id == strId;
    })) {
        bWasLocked = true;
    }

    // 4. Cap nhat cac tap tin tren dia
    bool bSavedCards = FileService::saveCards(this->_listCards);
    if (!bSavedCards) {
        return ERR_FILE_NOT_FOUND;
    }
    if (bWasLocked) {
        FileService::saveLockedIds(this->_listLockedIds);
    }

    // 5. Xoa tap tin thong tin tai khoan data/[ID].txt va archive file LichSu[ID].txt de tra soat
    FileService::deleteAccountFile(strId);
    FileService::archiveHistoryFile(strId);
    FileService::resetFailedAttempts(strId);

    // 6. Admin audit log write
    std::string strLogDetail = "Xoa the " + strId;
    if (!strAccName.empty()) {
        strLogDetail += " (" + strAccName + ")";
    }
    strLogDetail += ", So du con lai: " + std::to_string(lBal) + " VND";
    FileService::appendAdminLog("DELETE_CARD", strLogDetail);

    return ERR_NONE;
}

void AtmController::adminDeleteCard() {
    ConsoleView::clearScreen();
    ConsoleView::printHeader("XOA TAI KHOAN THE");

    std::string strId = ConsoleView::inputLine("Nhap ma so the can xoa (14 chu so): ");
    auto pCard = this->_listCards.findIf([&](const Card& c) {
        return c.getId() == strId;
    });

    if (pCard == nullptr) {
        ConsoleView::printError("Khong tim thay the co ma so " + strId + " trong he thong!");
        return;
    }

    Account acc;
    ErrorCode errAcc = FileService::loadAccount(strId, acc);
    long lBalance = (errAcc == ERR_NONE) ? acc.getBalance() : 0;
    std::string strName = (errAcc == ERR_NONE) ? acc.getName() : "Khong xac dinh";

    if (lBalance > 0) {
        std::cout << "\n";
        ConsoleView::printWarning("==================== CANH BAO QUAN TRONG ====================");
        ConsoleView::printWarning("Tai khoan the " + strId + " (" + strName + ") van con so du:");
        ConsoleView::printWarning(">> SO DU HIEN TAI: " + std::to_string(lBalance) + " VND <<");
        ConsoleView::printWarning("Hanh dong xoa the se vo hieu hoa tai khoan va huy bo so du tren!");
        ConsoleView::printWarning("=============================================================\n");
    }

    bool bConfirm = ConsoleView::confirmAction("Ban co chac chan muon xoa the " + strId + "?");
    if (!bConfirm) {
        ConsoleView::printInfo("Da huy thao tac xoa the.");
        return;
    }

    ErrorCode err = this->deleteCardAccount(strId);
    if (err == ERR_NONE) {
        ConsoleView::printSuccess("Xoa the " + strId + " thanh cong!");
        ConsoleView::printInfo("(File data/" + strId + ".txt da duoc xoa, file LichSu duoc luu giu tra soat)");
    } else {
        ConsoleView::printError("Xoa the that bai!");
    }
}

ErrorCode AtmController::unlockCardAccount(const std::string& strId) {
    // 1. Tim the trong danh sach the hoac danh sach khoa
    auto pCard = this->_listCards.findIf([&](const Card& c) {
        return c.getId() == strId;
    });

    bool bInLockedList = (this->_listLockedIds.findIf([&strId](const std::string& id) {
        return id == strId;
    }) != nullptr);

    if (pCard == nullptr && !bInLockedList) {
        return ERR_ID_NOT_FOUND;
    }

    // 2. Xoa khoi danh sach ID khoa trong RAM (xoa triet de moi ban sao)
    while (this->_listLockedIds.removeIf([&](const std::string& id) {
        return id == strId;
    })) {}

    // 3. Dat lai so lan sai va co khoa tren Card neu the ton tai
    if (pCard != nullptr) {
        pCard->unlockCard();
    }
    FileService::resetFailedAttempts(strId);

    // 4. Luu thay doi ben vung vao disk
    FileService::saveLockedIds(this->_listLockedIds);
    if (pCard != nullptr) {
        FileService::saveCards(this->_listCards);
    }

    // 5. Ghi Admin audit log
    FileService::appendAdminLog("UNLOCK_CARD", "Mo khoa the " + strId);

    return ERR_NONE;
}

void AtmController::adminUnlockCard() {
    ConsoleView::clearScreen();
    ConsoleView::printHeader("MO KHOA THE TU");

    bool bAnyLocked = !this->_listLockedIds.isEmpty();
    if (!bAnyLocked) {
        auto pCur = this->_listCards.getHead();
        while (pCur != nullptr) {
            if (pCur->_data.isLocked()) {
                bAnyLocked = true;
                break;
            }
            pCur = pCur->_pNext;
        }
    }

    if (!bAnyLocked) {
        ConsoleView::printInfo("Hien tai khong co the nao bi khoa trong he thong.");
        return;
    }

    std::cout << "Danh sach cac the dang bi khoa:\n";
    auto pCur = this->_listLockedIds.getHead();
    int iIndex = 1;
    while (pCur != nullptr) {
        std::cout << "  " << iIndex++ << ". Ma the ID: \033[31m" << pCur->_data << "\033[0m\n";
        pCur = pCur->_pNext;
    }

    auto pCardCur = this->_listCards.getHead();
    while (pCardCur != nullptr) {
        if (pCardCur->_data.isLocked()) {
            const std::string& strCardId = pCardCur->_data.getId();
            const std::string* pInList = this->_listLockedIds.findIf([&strCardId](const std::string& id) {
                return id == strCardId;
            });
            if (pInList == nullptr) {
                std::cout << "  " << iIndex++ << ". Ma the ID: \033[31m" << strCardId << "\033[0m\n";
            }
        }
        pCardCur = pCardCur->_pNext;
    }
    std::cout << "------------------------------------------------------\n";

    std::string strId = ConsoleView::inputLine("Nhap ma so the can mo khoa (Enter de thoat): ");
    if (strId.empty()) {
        return;
    }

    ErrorCode err = this->unlockCardAccount(strId);
    if (err == ERR_NONE) {
        ConsoleView::printSuccess("Mo khoa the " + strId + " thanh cong! The da hoat dong binh thuong.");
    } else if (err == ERR_ID_NOT_FOUND) {
        ConsoleView::printError("Ma so the khong ton tai trong he thong!");
    } else {
        ConsoleView::printError("Mo khoa that bai!");
    }
}

//USER MODULE
void AtmController::processUserLogin() {
    this->cleanupSession();
    ConsoleView::clearScreen();
    ConsoleView::printHeader("DANG NHAP KHACH HANG (USER)");

    std::string strId = ConsoleView::inputLine("Nhap ma so the (14 chu so, Enter hoac 0 de huy): ");
    if (strId.empty() || strId == "0") {
        ConsoleView::printInfo("Da huy thao tac dang nhap Khach hang.");
        ConsoleView::pauseScreen();
        return;
    }

    if (strId.length() != static_cast<size_t>(ID_LENGTH)) {
        ConsoleView::printError("Dinh dang ma so the khong hop le (phai gom dung 14 chu so)!");
        ConsoleView::pauseScreen();
        return;
    }

    auto pCard = this->_listCards.findIf([&](const Card& c) {
        return c.getId() == strId;
    });

    if (pCard == nullptr) {
        ConsoleView::printError("The khong ton tai trong he thong!");
        ConsoleView::pauseScreen();
        return;
    }

    // Kiem tra the co dang bi khoa hay khong
    bool bLocked = pCard->isLocked();
    auto pLockCur = this->_listLockedIds.getHead();
    while (pLockCur != nullptr) {
        if (pLockCur->_data == strId) {
            bLocked = true;
            break;
        }
        pLockCur = pLockCur->_pNext;
    }

    if (bLocked) {
        ConsoleView::printError("The nay da bi KHOA do nhap sai ma PIN qua 3 lan!");
        ConsoleView::printWarning("Vui long lien he quan tri vien (Admin) de duoc ho tro mo khoa the.");
        ConsoleView::pauseScreen();
        return;
    }

    // Nhap ma PIN
    std::string strPin = ConsoleView::inputPassword("Nhap ma PIN (Enter hoac 0 de huy): ");
    if (strPin.empty() || strPin == "0") {
        ConsoleView::printInfo("Da huy thao tac dang nhap Khach hang.");
        ConsoleView::pauseScreen();
        return;
    }
    bool bOutLocked = false;
    bool bAuth = UserController::authenticate(*pCard, strPin, bOutLocked);

    if (!bAuth) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        int iPersistentFailed = FileService::recordFailedAttempt(strId);
        pCard->setFailedAttempts(iPersistentFailed);
        if (bOutLocked || iPersistentFailed >= MAX_FAILED_LOGINS) {
            pCard->lockCard();
            auto pExistingLock = this->_listLockedIds.findIf([&](const std::string& id) {
                return id == strId;
            });
            if (pExistingLock == nullptr) {
                this->_listLockedIds.addTail(strId);
                FileService::appendLockedCard(strId);
            }
            FileService::saveCards(this->_listCards);
            ConsoleView::printError("Ban da nhap sai PIN 3 lan lien tiep! The da bi KHOA.");
        } else {
            int iRemaining = MAX_FAILED_LOGINS - pCard->getFailedAttempts();
            ConsoleView::printWarning("Ma PIN khong chinh xac! Con lai " + std::to_string(iRemaining) + " lan thu.");
        }
        ConsoleView::pauseScreen();
        return;
    }

    // Dang nhap thanh cong: reset bo dem sai ben vung
    FileService::resetFailedAttempts(strId);
    pCard->resetFailedAttempts();

    // Dang nhap thanh cong -> Khoi tao phien lam viec
    this->_pCurrentCard = pCard;
    this->_eCurrentRole = ROLE_USER;
    this->_pCurrentAccount = new Account();

    ErrorCode errAcc = FileService::loadAccount(strId, *(this->_pCurrentAccount));
    if (errAcc != ERR_NONE) {
        ConsoleView::printError("Khong the tai thong tin tai khoan tu tap tin data/" + strId + ".txt!");
        this->cleanupSession();
        ConsoleView::pauseScreen();
        return;
    }

    // Kiem tra ma PIN mac dinh (123456)
    if (pCard->isDefaultPin()) {
        ConsoleView::printWarning("\nDay la lan dau tien su dung the (PIN mac dinh: 123456).");
        ConsoleView::printWarning("Quy khach bat buoc phai doi ma PIN moi de tiep tuc su dung!");
        bool bChanged = UserController::enforceDefaultPinChange(*pCard);
        if (!bChanged) {
            ConsoleView::printError("Doi PIN bat buoc that bai. Phien giao dich bi huy.");
            this->cleanupSession();
            ConsoleView::pauseScreen();
            return;
        }
    }

    ConsoleView::printSuccess("Xac thuc thanh cong! Chuyen tiep vao phan he Khach hang...");
    ConsoleView::pauseScreen();

    this->processUserMenu();
}

void AtmController::processUserMenu() {
    if (this->_pCurrentCard == nullptr || this->_pCurrentAccount == nullptr) {
        return;
    }

    UserController::runUserSession(*(this->_pCurrentCard), *(this->_pCurrentAccount));

    this->cleanupSession();
}

// GETTERS CHO KIEM THU
const LinkedList<Admin>& AtmController::getAdmins() const {
    return this->_listAdmins;
}

LinkedList<Admin>& AtmController::getAdmins() {
    return this->_listAdmins;
}

const LinkedList<Card>& AtmController::getCards() const {
    return this->_listCards;
}

LinkedList<Card>& AtmController::getCards() {
    return this->_listCards;
}

const LinkedList<std::string>& AtmController::getLockedIds() const {
    return this->_listLockedIds;
}

LinkedList<std::string>& AtmController::getLockedIds() {
    return this->_listLockedIds;
}

UserRole AtmController::getCurrentRole() const {
    return this->_eCurrentRole;
}

const Account* AtmController::getCurrentAccount() const {
    return this->_pCurrentAccount;
}

const Card* AtmController::getCurrentCard() const {
    return this->_pCurrentCard;
}
