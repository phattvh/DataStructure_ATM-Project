/******************************************************************************
 * @file: AdminController.cpp
 * @description: Hien thuc Bo dieu phoi Phan he Quan tri vien (Admin Module)
 *               Bao gom: Dang nhap Admin, Xem DS the, Them the, Xoa the, Mo khoa the.
 * (Phase 2 - Member A - Ngay 6-7)
 * Definition of Done: Admin them account moi sinh dung du 2 file [ID].txt
 * va [LichSuID].txt; xoa the chi xoa [ID].txt, giu LichSu; mo khoa the
 * xoa khoi KhoaThe.txt va reset failed attempts.
 ******************************************************************************/

#include "AdminController.h"
#include "ConsoleView.h"
#include "FileService.h"
#include "UserController.h"
#include <iostream>
#include <iomanip>
#include <string>

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
    _listAdmins.clear();
    _listCards.clear();
    _listLockedIds.clear();

    bool bAdmins = FileService::loadAdmins(_listAdmins);
    FileService::loadLockedIds(_listLockedIds);
    bool bCards  = FileService::loadCards(_listCards, _listLockedIds);

    return bAdmins && bCards;
}

//=============================================================================
// Xac thuc mat khau Admin
//=============================================================================

bool AdminController::verifyAdmin(const std::string& strUser,
                                  const std::string& strPass) const {
    const Admin* pAdmin = _listAdmins.findIf([&strUser](const Admin& a) {
        return a.getUsername() == strUser;
    });
    return (pAdmin != nullptr) && pAdmin->verifyPassword(strPass);
}

//=============================================================================
// Chuc nang 1: Xem danh sach the tu
//=============================================================================

void AdminController::viewCardList() const {
    ConsoleView::printHeader("DANH SACH THE TU HE THONG");

    if (_listCards.isEmpty()) {
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
    auto pCur = _listCards.getHead();
    while (pCur != nullptr) {
        const Card& card = pCur->_data;
        std::string strId     = card.getId();
        bool        bIsLocked = card.isLocked();

        const std::string* pLock = _listLockedIds.findIf([&strId](const std::string& id) {
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
    std::cout << "  Tong cong: " << _listCards.getSize() << " the\n";
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
        strNewId = ConsoleView::inputLine("Nhap ma so the moi (14 chu so): ");

        if (!UserController::isValidIdFormat(strNewId)) {
            ConsoleView::printError("Ma so the phai bao gom dung 14 chu so! Vui long nhap lai.");
            continue;
        }

        const Card* pExist = _listCards.findIf([&strNewId](const Card& c) {
            return c.getId() == strNewId;
        });
        if (pExist != nullptr) {
            ConsoleView::printError("Ma so the " + strNewId + " da ton tai trong he thong! Vui long nhap ID khac.");
            continue;
        }
        break;
    }

    // Nhap ten chu the
    std::string strName;
    while (true) {
        strName = ConsoleView::inputLine("Nhap ho ten chu tai khoan: ");
        if (strName.empty()) {
            ConsoleView::printError("Ho ten khong duoc de trong!");
            continue;
        }
        break;
    }

    // Nhap so du ban dau
    long lBalance = 0;
    while (true) {
        lBalance = ConsoleView::inputMoney("Nhap so du ban dau (toi thieu 50,000 VND): ");
        if (lBalance < MIN_BALANCE_RESERVE) {
            ConsoleView::printError("So du ban dau phai tu " +
                                    std::to_string(MIN_BALANCE_RESERVE) + " VND tro len!");
            continue;
        }
        break;
    }

    // Nhap loai tien te
    std::string strCurrency = ConsoleView::inputLine("Don vi tien te (Enter de chon VND): ");
    if (strCurrency.empty()) strCurrency = "VND";

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
    _listCards.addTail(Card(strNewId, DEFAULT_PIN, false));

    // Cap nhat file TheTu.txt
    bool bSave = FileService::saveCards(_listCards);
    if (!bSave) {
        ConsoleView::printError("Loi khi cap nhat TheTu.txt! Du lieu tren dia co the khong dong bo.");
        ConsoleView::pauseScreen();
        return;
    }

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

    if (_listCards.isEmpty()) {
        ConsoleView::printWarning("Khong co the nao trong he thong de xoa.");
        ConsoleView::pauseScreen();
        return;
    }

    std::string strDelId = ConsoleView::inputLine("Nhap ma so the can xoa (14 chu so): ");

    if (!UserController::isValidIdFormat(strDelId)) {
        ConsoleView::printError("Ma so the phai bao gom dung 14 chu so!");
        ConsoleView::pauseScreen();
        return;
    }

    const Card* pExist = _listCards.findIf([&strDelId](const Card& c) {
        return c.getId() == strDelId;
    });

    if (pExist == nullptr) {
        ConsoleView::printError("Khong tim thay ma so the " + strDelId + " trong he thong!");
        ConsoleView::pauseScreen();
        return;
    }

    std::cout << "\n  Thong tin the can xoa:\n";
    std::cout << "    Ma the    : " << strDelId << "\n";
    std::cout << "    Trang thai: " << (pExist->isLocked() ? "Bi Khoa" : "Hoat dong") << "\n\n";
    std::cout << "  \033[33m[CANH BAO] Hanh dong nay se:\033[0m\n";
    std::cout << "    - Xoa the khoi danh sach TheTu.txt\n";
    std::cout << "    - Xoa file data/" << strDelId << ".txt\n";
    std::cout << "    - GIU LAI file data/LichSu" << strDelId << ".txt (phuc vu kiem toan)\n\n";

    if (!ConsoleView::confirmAction("Xac nhan xoa the " + strDelId + "?")) {
        ConsoleView::printInfo("Da huy xoa tai khoan.");
        ConsoleView::pauseScreen();
        return;
    }

    // Xoa khoi RAM
    _listCards.removeIf([&strDelId](const Card& c) {
        return c.getId() == strDelId;
    });

    // Neu the dang bi khoa, xoa khoi danh sach khoa
    _listLockedIds.removeIf([&strDelId](const std::string& id) {
        return id == strDelId;
    });

    // Cap nhat TheTu.txt va KhoaThe.txt
    FileService::saveCards(_listCards);
    FileService::saveLockedIds(_listLockedIds);

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
    ConsoleView::pauseScreen();
}

//=============================================================================
// Chuc nang 4: Mo khoa the bi khoa
//=============================================================================

void AdminController::unlockCard() {
    ConsoleView::printHeader("MO KHOA THE TU");

    if (_listLockedIds.isEmpty()) {
        bool bAnyLocked = false;
        auto pCur = _listCards.getHead();
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
    auto pLockCur = _listLockedIds.getHead();
    while (pLockCur != nullptr) {
        std::cout << "| " << std::left << std::setw(4) << (iCount + 1)
                  << "| \033[31m" << std::setw(15) << pLockCur->_data << "\033[0m|\n";
        iCount++;
        pLockCur = pLockCur->_pNext;
    }

    auto pCardCur = _listCards.getHead();
    while (pCardCur != nullptr) {
        if (pCardCur->_data.isLocked()) {
            const std::string& strLockedId = pCardCur->_data.getId();
            const std::string* pInList = _listLockedIds.findIf([&strLockedId](const std::string& id) {
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

    std::string strUnlockId = ConsoleView::inputLine("Nhap ma so the can mo khoa (0 = Huy): ");
    if (strUnlockId == "0") {
        ConsoleView::printInfo("Da huy mo khoa.");
        ConsoleView::pauseScreen();
        return;
    }

    if (!UserController::isValidIdFormat(strUnlockId)) {
        ConsoleView::printError("Ma so the phai bao gom dung 14 chu so!");
        ConsoleView::pauseScreen();
        return;
    }

    bool bFoundLocked = false;
    const std::string* pLock = _listLockedIds.findIf([&strUnlockId](const std::string& id) {
        return id == strUnlockId;
    });
    if (pLock != nullptr) bFoundLocked = true;

    Card* pCard = _listCards.findIf([&strUnlockId](const Card& c) {
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

    // 2. Xoa khoi KhoaThe.txt
    _listLockedIds.removeIf([&strUnlockId](const std::string& id) {
        return id == strUnlockId;
    });
    FileService::saveLockedIds(_listLockedIds);

    // 3. Dong bo lai TheTu.txt
    FileService::saveCards(_listCards);

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
    loadAllData();

    ConsoleView::printHeader("DANG NHAP QUAN TRI VIEN (ADMIN)");
    std::cout << "  (Nhap '0' de quay lai)\n\n";

    std::string strUser = ConsoleView::inputLine("Ten dang nhap Admin: ");
    if (strUser == "0") return false;

    std::string strPass = ConsoleView::inputPassword("Mat khau (hien thi dau *): ");
    if (strPass == "0") return false;

    if (verifyAdmin(strUser, strPass)) {
        ConsoleView::printSuccess("Dang nhap Admin thanh cong! Chao mung, " + strUser + "!");
        ConsoleView::pauseScreen();
        return true;
    } else {
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
            case 1: viewCardList(); break;
            case 2: addNewCard();   break;
            case 3: deleteCard();   break;
            case 4: unlockCard();   break;
            default:
                ConsoleView::printError("Lua chon khong hop le!");
                ConsoleView::pauseScreen();
                break;
        }
    }
}
