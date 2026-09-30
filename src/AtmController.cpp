#include "AtmController.h"
#include "ConsoleView.h"
#include "FileService.h"
#include <iostream>
#include <cctype>

AtmController::AtmController()
    : _pCurrentAccount(nullptr), _currentUserRole(ROLE_NONE) {}

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

void AtmController::processUserLogin() {
    ConsoleView::printHeader("DANG NHAP KHACH HANG (USER)");
    ConsoleView::printInfo("Chuc nang User dang trong qua trinh phat trien (Ke hoach Tuan 2).");
    ConsoleView::printInfo("Vui long su dung Menu Admin de quan tri the va tai khoan.");
    ConsoleView::pauseScreen();
}
