/******************************************************************************
 * @file: AdminController.cpp
 * @description: Hien thuc Bo dieu phoi Phan he Quan tri vien (Admin Module)
 *               Bao gom: Dang nhap Admin, Xem DS the, Them the, Xoa the, Mo khoa the.
 * (Phase 2 - Member A - Ngay 6-7)
 * Tuan thu chat che HCMUE C++ Coding Standard V2 (Rule 17: this->)
 * Va tich hop cac co che bao mat & kiem toan nang cao.
 ******************************************************************************/

#include "AdminController.h"
#include "ConsoleView.h"
#include "FileService.h"
#include "UserController.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

//=============================================================================
// Constructor
//=============================================================================

AdminController::AdminController() {
    // Cac LinkedList duoc khoi tao rong qua constructor mac dinh
}

//=============================================================================
// Tai du lieu tu file vao RAM
//=============================================================================

bool AdminController::loadAllData() {
    this->_listAdmins.clear();
    this->_listCards.clear();
    this->_listLockedIds.clear();

    bool bAdmins = FileService::loadAdmins(this->_listAdmins);
    FileService::loadLockedIds(this->_listLockedIds);
    bool bCards  = FileService::loadCards(this->_listCards, this->_listLockedIds);

    return bAdmins && bCards;
}

//=============================================================================
// Xac thuc mat khau Admin
//=============================================================================

bool AdminController::verifyAdmin(const std::string& strUser,
                                  const std::string& strPass) const {
    const Admin* pAdmin = this->_listAdmins.findIf([&strUser](const Admin& a) {
        return a.getUsername() == strUser;
    });
    return (pAdmin != nullptr) && pAdmin->verifyPassword(strPass);
}

//=============================================================================
// Chuc nang 1: Xem danh sach the tu
//=============================================================================

void AdminController::viewCardList() const {
    ConsoleView::printHeader("DANH SACH THE TU HE THONG");

    if (this->_listCards.isEmpty()) {
        ConsoleView::printWarning("Hien khong co the tu nao trong he thong.");
        ConsoleView::pauseScreen();
        return;
    }

    std::cout << "\033[36m";
    std::cout << "+-----+----------------+--------------------+\n";
    std::cout << "| STT | MA SO THE (ID) | TRANG THAI         |\n";
    std::cout << "+-----+----------------+--------------------+\n";
    std::cout << "\033[0m";

    int iIndex = 1;
    auto pCur = this->_listCards.getHead();
    while (pCur != nullptr) {
        const Card& card = pCur->_data;
        std::string strId     = card.getId();
        bool        bIsLocked = card.isLocked();

        const std::string* pLock = this->_listLockedIds.findIf([&strId](const std::string& id) {
            return id == strId;
        });
        if (pLock != nullptr) bIsLocked = true;

        std::string strStatus;
        if (bIsLocked) {
            strStatus = "\033[31mBi Khoa\033[0m";
        } else if (card.isDefaultPin()) {
            strStatus = "\033[33mChua doi PIN\033[0m";
        } else {
            strStatus = "\033[32mHoat dong\033[0m";
        }

        std::cout << "| " << std::left << std::setw(4) << iIndex++
                  << "| " << std::setw(15) << strId
                  << "| " << strStatus << "\n";

        pCur = pCur->_pNext;
    }

    std::cout << "\033[36m+-----+----------------+--------------------+\033[0m\n";
    std::cout << "  Tong cong: " << this->_listCards.getSize() << " the\n";
    ConsoleView::pauseScreen();
}

//=============================================================================
// Chuc nang 2: Them tai khoan the moi (DoD: tao dung 2 file)
//=============================================================================

void AdminController::addNewCard() {
    ConsoleView::printHeader("THEM TAI KHOAN THE TU MOI");

    // Nhap va kiem tra dinh dang ID (14 chu so)
    std::string strNewId;
    while (true) {
        strNewId = ConsoleView::inputLine("Nhap ma so the moi (14 chu so, Enter/0 de huy): ");
        if (strNewId.empty() || strNewId == "0") {
            ConsoleView::printInfo("Da huy thao tac them the moi.");
            ConsoleView::pauseScreen();
            return;
        }

        if (!UserController::isValidIdFormat(strNewId)) {
            ConsoleView::printError("Ma so the phai bao gom dung 14 chu so! Vui long nhap lai.");
            continue;
        }

        const Card* pExist = this->_listCards.findIf([&strNewId](const Card& c) {
            return c.getId() == strNewId;
        });
        if (pExist != nullptr) {
            ConsoleView::printError("Ma so the " + strNewId + " da ton tai trong he thong! Vui long nhap ID khac.");
            continue;
        }

        // Chặn ghi đè tệp tin tài khoản mồ côi trên đĩa
        if (fs::exists(DATA_DIR + strNewId + ".txt")) {
            ConsoleView::printError("Tap tin tai khoan " + strNewId + ".txt da ton tai tren dia! Vui long chon ID khac.");
            continue;
        }

        break;
    }

    // Nhap ten chu the
    std::string strName;
    while (true) {
        strName = ConsoleView::inputLine("Nhap ho ten chu tai khoan (Enter/0 de huy): ");
        if (strName.empty() || strName == "0") {
            ConsoleView::printInfo("Da huy thao tac them the moi.");
            ConsoleView::pauseScreen();
            return;
        }
        size_t s = strName.find_first_not_of(" \t\r\n");
        if (s == std::string::npos) {
            ConsoleView::printError("Ho ten khong duoc de trong!");
            continue;
        }
        bool bHasInvalidChar = false;
        for (unsigned char c : strName) {
            if (c < 32 || c == 127 || c == '|' || c == 27) {
                bHasInvalidChar = true;
                break;
            }
        }
        if (bHasInvalidChar) {
            ConsoleView::printError("Ho ten khong duoc chua ky tu dac biet, ky tu dieu khien hoac ANSI escape!");
            continue;
        }
        break;
    }

    // Nhap so du ban dau (toi thieu 50k va la boi so cua 50k)
    long lBalance = 0;
    while (true) {
        lBalance = ConsoleView::inputMoney("Nhap so du ban dau (toi thieu 50,000 VND, boi so 50k, 0 de huy): ");
        if (lBalance == 0) {
            ConsoleView::printInfo("Da huy thao tac them the moi.");
            ConsoleView::pauseScreen();
            return;
        }
        if (lBalance < MIN_BALANCE_RESERVE) {
            ConsoleView::printError("So du ban dau phai tu " +
                                    std::to_string(MIN_BALANCE_RESERVE) + " VND tro len!");
            continue;
        }
        if (lBalance % MIN_TRANSACTION != 0) {
            ConsoleView::printError("So du ban dau phai la boi so cua " +
                                    std::to_string(MIN_TRANSACTION) + " VND!");
            continue;
        }
        break;
    }

    // Nhap loai tien te (kiem tra dinh dang 3-5 chu cai viet hoa)
    std::string strCurrency;
    while (true) {
        strCurrency = ConsoleView::inputLine("Don vi tien te (Enter de chon VND): ");
        if (strCurrency.empty()) {
            strCurrency = "VND";
            break;
        }
        bool bValidCurr = (strCurrency.length() >= 3 && strCurrency.length() <= 5);
        for (char c : strCurrency) {
            if (!std::isupper(static_cast<unsigned char>(c))) {
                bValidCurr = false;
                break;
            }
        }
        if (!bValidCurr) {
            ConsoleView::printError("Don vi tien te khong hop le (chi chap nhan 3-5 chu cai viet hoa, vi du: VND, USD)!");
            continue;
        }
        break;
    }

    // Xac nhan truoc khi tao
    std::cout << "\n  Thong tin the moi:\n";
    std::cout << "    Ma the  : " << strNewId     << "\n";
    std::cout << "    Ho ten  : " << strName      << "\n";
    std::cout << "    So du   : " << lBalance     << " " << strCurrency << "\n";
    std::cout << "    Ma PIN  : " << DEFAULT_PIN  << " (mac dinh - yeu cau doi khi dang nhap lan dau)\n\n";

    if (!ConsoleView::confirmAction("Xac nhan them the moi?")) {
        ConsoleView::printInfo("Da huy them tai khoan.");
        ConsoleView::pauseScreen();
        return;
    }

    // Tao ca 2 file: [ID].txt va LichSu[ID].txt (DoD)
    bool bCreate = FileService::createAccountFiles(strNewId, strName, lBalance, strCurrency);
    if (!bCreate) {
        ConsoleView::printError("Loi khi tao file tai khoan! Vui long kiem tra thu muc data/.");
        ConsoleView::pauseScreen();
        return;
    }

    // Cap nhat RAM: them Card moi voi PIN mac dinh
    this->_listCards.addTail(Card(strNewId, DEFAULT_PIN, false));

    // Cap nhat file TheTu.txt
    bool bSave = FileService::saveCards(this->_listCards);
    if (!bSave) {
        ConsoleView::printError("Loi khi cap nhat TheTu.txt! Du lieu tren dia co the khong dong bo.");
        ConsoleView::pauseScreen();
        return;
    }

    // Ghi nhat ky kiem toan Admin (Audit Log)
    FileService::appendAdminLog("ADD_CARD", "Them the " + strNewId + " (" + strName + "), So du: " + std::to_string(lBalance) + " " + strCurrency);

    ConsoleView::printSuccess("Them tai khoan thanh cong! Ma the: " + strNewId);
    std::cout << "  + File data/" << strNewId        << ".txt da duoc tao.\n";
    std::cout << "  + File data/LichSu" << strNewId  << ".txt da duoc tao.\n";
    ConsoleView::pauseScreen();
}

//=============================================================================
// Chuc nang 3: Xoa tai khoan the tu
//=============================================================================

void AdminController::deleteCard() {
    ConsoleView::printHeader("XOA TAI KHOAN THE TU");

    if (this->_listCards.isEmpty()) {
        ConsoleView::printWarning("Khong co the nao trong he thong de xoa.");
        ConsoleView::pauseScreen();
        return;
    }

    std::string strDelId = ConsoleView::inputLine("Nhap ma so the can xoa (14 chu so, Enter/0 de huy): ");
    if (strDelId.empty() || strDelId == "0") {
        ConsoleView::printInfo("Da huy thao tac xoa the.");
        ConsoleView::pauseScreen();
        return;
    }

    if (!UserController::isValidIdFormat(strDelId)) {
        ConsoleView::printError("Ma so the phai bao gom dung 14 chu so!");
        ConsoleView::pauseScreen();
        return;
    }

    const Card* pExist = this->_listCards.findIf([&strDelId](const Card& c) {
        return c.getId() == strDelId;
    });

    if (pExist == nullptr) {
        ConsoleView::printError("Khong tim thay ma so the " + strDelId + " trong he thong!");
        ConsoleView::pauseScreen();
        return;
    }

    // Doc thong tin so du hien tai truoc khi xoa
    Account acc;
    ErrorCode errAcc = FileService::loadAccount(strDelId, acc);
    long lBalance = (errAcc == ERR_NONE) ? acc.getBalance() : 0;
    std::string strName = (errAcc == ERR_NONE) ? acc.getName() : "Khong xac dinh";

    std::cout << "\n  Thong tin the can xoa:\n";
    std::cout << "    Ma the    : " << strDelId << "\n";
    std::cout << "    Chu the   : " << strName << "\n";
    std::cout << "    Trang thai: " << (pExist->isLocked() ? "Bi Khoa" : "Hoat dong") << "\n\n";

    if (lBalance > 0) {
        ConsoleView::printWarning("==================== CANH BAO QUAN TRONG ====================");
        ConsoleView::printWarning("Tai khoan the " + strDelId + " (" + strName + ") van con so du:");
        ConsoleView::printWarning(">> SO DU HIEN TAI: " + std::to_string(lBalance) + " VND <<");
        ConsoleView::printWarning("Hanh dong xoa the se vo hieu hoa tai khoan va dong so du tren!");
        ConsoleView::printWarning("=============================================================\n");
    }

    std::cout << "  \033[33m[CANH BAO] Hanh dong nay se:\033[0m\n";
    std::cout << "    - Xoa the khoi danh sach TheTu.txt\n";
    std::cout << "    - Xoa file data/" << strDelId << ".txt\n";
    std::cout << "    - GIU LAI file data/LichSu" << strDelId << ".txt (phuc vu kiem toan)\n\n";

    if (!ConsoleView::confirmAction("Xac nhan xoa the " + strDelId + "?")) {
        ConsoleView::printInfo("Da huy xoa tai khoan.");
        ConsoleView::pauseScreen();
        return;
    }

    // Xoa triet de khoi RAM (moi ban sao neu co)
    while (this->_listCards.removeIf([&strDelId](const Card& c) {
        return c.getId() == strDelId;
    })) {}

    // Neu the dang bi khoa, xoa triet de khoi danh sach khoa trong RAM
    bool bWasLocked = false;
    while (this->_listLockedIds.removeIf([&strDelId](const std::string& id) {
        return id == strDelId;
    })) {
        bWasLocked = true;
    }

    // Cap nhat TheTu.txt va KhoaThe.txt
    FileService::saveCards(this->_listCards);
    if (bWasLocked) {
        FileService::saveLockedIds(this->_listLockedIds);
    }

    // Xoa file [ID].txt (giu lai LichSu[ID].txt)
    bool bDelFile = FileService::deleteAccountFile(strDelId);
    if (!bDelFile) {
        ConsoleView::printWarning("The da xoa khoi danh sach nhung khong xoa duoc file data/" +
                                  strDelId + ".txt (co the file khong ton tai).");
    } else {
        ConsoleView::printSuccess("Xoa tai khoan " + strDelId + " thanh cong!");
        std::cout << "  + Da xoa: data/" << strDelId << ".txt\n";
        std::cout << "  + Giu lai: data/LichSu" << strDelId << ".txt\n";
    }

    // Ghi nhat ky kiem toan
    std::string strDetail = "Xoa the " + strDelId;
    if (!strName.empty()) {
        strDetail += " (" + strName + ")";
    }
    strDetail += ", So du con lai: " + std::to_string(lBalance) + " VND";
    FileService::appendAdminLog("DELETE_CARD", strDetail);

    ConsoleView::pauseScreen();
}

//=============================================================================
// Chuc nang 4: Mo khoa the bi khoa
//=============================================================================

void AdminController::unlockCard() {
    ConsoleView::printHeader("MO KHOA THE TU");

    if (this->_listLockedIds.isEmpty()) {
        bool bAnyLocked = false;
        auto pCur = this->_listCards.getHead();
        while (pCur != nullptr) {
            if (pCur->_data.isLocked()) {
                bAnyLocked = true;
                break;
            }
            pCur = pCur->_pNext;
        }

        if (!bAnyLocked) {
            ConsoleView::printSuccess("Khong co the nao dang bi khoa. He thong binh thuong.");
            ConsoleView::pauseScreen();
            return;
        }
    }

    std::cout << "\n  Danh sach the dang bi khoa:\n";
    std::cout << "\033[36m+-----+----------------+\033[0m\n";
    std::cout << "\033[36m| STT | MA SO THE      |\033[0m\n";
    std::cout << "\033[36m+-----+----------------+\033[0m\n";

    int iCount = 0;
    auto pLockCur = this->_listLockedIds.getHead();
    while (pLockCur != nullptr) {
        std::cout << "| " << std::left << std::setw(4) << (iCount + 1)
                  << "| \033[31m" << std::setw(15) << pLockCur->_data << "\033[0m|\n";
        iCount++;
        pLockCur = pLockCur->_pNext;
    }

    auto pCardCur = this->_listCards.getHead();
    while (pCardCur != nullptr) {
        if (pCardCur->_data.isLocked()) {
            const std::string& strLockedId = pCardCur->_data.getId();
            const std::string* pInList = this->_listLockedIds.findIf([&strLockedId](const std::string& id) {
                return id == strLockedId;
            });
            if (pInList == nullptr) {
                std::cout << "| " << std::left << std::setw(4) << (iCount + 1)
                          << "| \033[31m" << std::setw(15) << strLockedId << "\033[0m|\n";
                iCount++;
            }
        }
        pCardCur = pCardCur->_pNext;
    }

    std::cout << "\033[36m+-----+----------------+\033[0m\n";

    if (iCount == 0) {
        ConsoleView::printSuccess("Khong co the nao dang bi khoa.");
        ConsoleView::pauseScreen();
        return;
    }

    std::string strUnlockId = ConsoleView::inputLine("Nhap ma so the can mo khoa (Enter/0 de huy): ");
    if (strUnlockId == "0" || strUnlockId.empty()) {
        ConsoleView::printInfo("Da huy thao tac mo khoa.");
        ConsoleView::pauseScreen();
        return;
    }

    if (!UserController::isValidIdFormat(strUnlockId)) {
        ConsoleView::printError("Ma so the phai bao gom dung 14 chu so!");
        ConsoleView::pauseScreen();
        return;
    }

    bool bFoundLocked = false;
    const std::string* pLock = this->_listLockedIds.findIf([&strUnlockId](const std::string& id) {
        return id == strUnlockId;
    });
    if (pLock != nullptr) bFoundLocked = true;

    Card* pCard = this->_listCards.findIf([&strUnlockId](const Card& c) {
        return c.getId() == strUnlockId;
    });
    if (pCard != nullptr && pCard->isLocked()) bFoundLocked = true;

    if (!bFoundLocked) {
        ConsoleView::printError("The " + strUnlockId + " khong dang bi khoa hoac khong ton tai!");
        ConsoleView::pauseScreen();
        return;
    }

    if (!ConsoleView::confirmAction("Xac nhan mo khoa the " + strUnlockId + "?")) {
        ConsoleView::printInfo("Da huy mo khoa.");
        ConsoleView::pauseScreen();
        return;
    }

    // 1. Mo khoa trong RAM & reset failed attempts
    if (pCard != nullptr) {
        pCard->unlockCard();
    }

    // 2. Xoa triet de moi ban sao khoi KhoaThe.txt
    while (this->_listLockedIds.removeIf([&strUnlockId](const std::string& id) {
        return id == strUnlockId;
    })) {}
    FileService::saveLockedIds(this->_listLockedIds);

    // 3. Dong bo lai TheTu.txt
    FileService::saveCards(this->_listCards);

    // 4. Ghi nhat ky kiem toan
    FileService::appendAdminLog("UNLOCK_CARD", "Mo khoa the " + strUnlockId);

    ConsoleView::printSuccess("Mo khoa the " + strUnlockId + " thanh cong!");
    std::cout << "  + So lan nhap sai da reset ve 0.\n";
    std::cout << "  + The da duoc xoa khoi data/KhoaThe.txt.\n";
    ConsoleView::pauseScreen();
}

//=============================================================================
// processAdminLogin: Dang nhap Admin
//=============================================================================

bool AdminController::processAdminLogin() {
    // Dam bao du lieu duoc nap tu dia
    this->loadAllData();

    ConsoleView::printHeader("DANG NHAP QUAN TRI VIEN (ADMIN)");
    std::cout << "  (Nhap '0' de quay lai)\n\n";

    std::string strUser = ConsoleView::inputLine("Ten dang nhap Admin: ");
    if (strUser == "0") return false;

    std::string strPass = ConsoleView::inputPassword("Mat khau (hien thi dau *): ");
    if (strPass == "0") return false;

    if (this->verifyAdmin(strUser, strPass)) {
        FileService::appendAdminLog("ADMIN_LOGIN_SUCCESS", "Admin dang nhap thanh cong: " + strUser);
        ConsoleView::printSuccess("Dang nhap Admin thanh cong! Chao mung, " + strUser + "!");
        ConsoleView::pauseScreen();
        return true;
    } else {
        FileService::appendAdminLog("ADMIN_LOGIN_FAIL", "Dang nhap Admin that bai: " + strUser);
        ConsoleView::printError("Ten dang nhap hoac mat khau khong chinh xac!");
        ConsoleView::pauseScreen();
        return false;
    }
}

//=============================================================================
// processAdminMenu: Vong lap Menu Admin
//=============================================================================

void AdminController::processAdminMenu() {
    while (true) {
        ConsoleView::printAdminMenu();
        int iChoice = ConsoleView::inputMenuChoice(0, 4, "Chon chuc nang: ");

        if (iChoice == 0) {
            ConsoleView::printSuccess("Da dang xuat Admin.");
            ConsoleView::pauseScreen();
            break;
        }

        switch (iChoice) {
            case 1: this->viewCardList(); break;
            case 2: this->addNewCard();   break;
            case 3: this->deleteCard();   break;
            case 4: this->unlockCard();   break;
            default:
                ConsoleView::printError("Lua chon khong hop le!");
                ConsoleView::pauseScreen();
                break;
        }
    }
}
