/******************************************************************************
 * @file: test_phase_2_a.cpp
 * @description: Bo kiem thu chuyen sau Phase 2 danh cho Thanh vien A (Member A)
 *               Nhiem vu: Rap logic Admin (Dang nhap, Xem DS, Them/Xoa tai khoan, Mo khoa the)
 *               DoD: Admin them account moi sinh ra dung va du 2 file [ID].txt va [LichSuID].txt.
 *
 * Cach chay:
 *   g++ -std=c++17 -Wall -Wextra -Iinclude src/ConsoleView.cpp src/Card.cpp src/Account.cpp \
 *       src/UserController.cpp src/Admin.cpp src/Transaction.cpp src/FileService.cpp \
 *       src/AdminController.cpp test/test_phase_2_a.cpp -o build/test_phase_2_a
 *   ./build/test_phase_2_a
 ******************************************************************************/

#include <iostream>
#include <iomanip>
#include <string>
#include <filesystem>

#include "Common.h"
#include "LinkedList.h"
#include "Admin.h"
#include "Card.h"
#include "Account.h"
#include "Transaction.h"
#include "FileService.h"
#include "AdminController.h"

namespace fs = std::filesystem;

static int g_nPass = 0;
static int g_nFail = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (cond) { \
            g_nPass++; \
            std::cout << "  \033[32m[PASS]\033[0m " << (msg) << "\n"; \
        } else { \
            g_nFail++; \
            std::cout << "  \033[31m[FAIL]\033[0m " << (msg) \
                      << " (Line: " << __LINE__ << ")\n"; \
        } \
    } while (0)

/******************************************************************************
 * 1. KIEM THU DANG NHAP ADMIN (XAC THUC DUNG / SAI TU FILE ADMIN.TXT)
 ******************************************************************************/
void testAdminLogin() {
    std::cout << "\n======================================================\n";
    std::cout << " 1. KIEM THU DANG NHAP ADMIN (MEMBER A)\n";
    std::cout << "======================================================\n";

    FileService::initSampleData();

    AdminController controller;
    bool bLoaded = controller.loadAllData();
    TEST_ASSERT(bLoaded, "AdminController nap thanh cong du lieu tu cac file");
    TEST_ASSERT(controller.getAdmins().getSize() >= 3, "Danh sach Admin nap vao RAM co it nhat 3 tai khoan");

    // Kiem tra xac thuc qua du lieu da load
    const Admin* pAdmin1 = controller.getAdmins().findIf([](const Admin& a) {
        return a.getUsername() == "admin1";
    });
    TEST_ASSERT(pAdmin1 != nullptr, "Tim thay tai khoan admin1");
    TEST_ASSERT(pAdmin1 != nullptr && pAdmin1->verifyPassword("123456"), "admin1 dang nhap dung mat khau 123456");
    TEST_ASSERT(pAdmin1 != nullptr && !pAdmin1->verifyPassword("wrongpass"), "admin1 tu choi mat khau sai");

    const Admin* pAdminGhost = controller.getAdmins().findIf([](const Admin& a) {
        return a.getUsername() == "khong_ton_tai";
    });
    TEST_ASSERT(pAdminGhost == nullptr, "Tai khoan khong ton tai tra ve nullptr");
}

/******************************************************************************
 * 2. KIEM THU XEM DANH SACH THE TU
 ******************************************************************************/
void testAdminViewCardList() {
    std::cout << "\n======================================================\n";
    std::cout << " 2. KIEM THU XEM DANH SACH THE TU (MEMBER A)\n";
    std::cout << "======================================================\n";

    FileService::initSampleData();

    AdminController controller;
    controller.loadAllData();

    TEST_ASSERT(controller.getCards().getSize() >= 10, "He thong co it nhat 10 the tu mau");

    // Kiem tra the ton tai va trang thai
    const Card* pCard1 = controller.getCards().findIf([](const Card& c) {
        return c.getId() == "10014504500001";
    });
    TEST_ASSERT(pCard1 != nullptr, "Tim thay the 10014504500001");
    TEST_ASSERT(pCard1 != nullptr && !pCard1->isLocked(), "The 10014504500001 dang o trang thai hoat dong");
}

/******************************************************************************
 * 3. KIEM THU THEM TAI KHOAN MOI (DEFINITION OF DONE - DOD)
 * "Admin them account moi sinh ra dung va du 2 file [ID].txt va [LichSuID].txt"
 ******************************************************************************/
void testAdminAddAccount_DoD() {
    std::cout << "\n======================================================\n";
    std::cout << " 3. KIEM THU THEM TAI KHOAN - DEFINITION OF DONE (DOD)\n";
    std::cout << "======================================================\n";

    FileService::initSampleData();

    const std::string strNewId    = "10014504509991";
    const std::string strName     = "Tran Van MemberA Test";
    const long        lBalance    = 2000000;
    const std::string strCurrency = "VND";

    std::string strIdFile   = "data/" + strNewId + ".txt";
    std::string strHistFile = "data/LichSu" + strNewId + ".txt";

    // Xoa sach file test truoc do neu co
    fs::remove(strIdFile);
    fs::remove(strHistFile);

    // Thuc hien tao tai khoan moi
    bool bCreate = FileService::createAccountFiles(strNewId, strName, lBalance, strCurrency);
    TEST_ASSERT(bCreate, "createAccountFiles() tra ve true");

    // KIEM TRA DOD: Sinh ra dung va du 2 file
    TEST_ASSERT(fs::exists(strIdFile),   "[DoD] File data/" + strNewId + ".txt TON TAI");
    TEST_ASSERT(fs::exists(strHistFile), "[DoD] File data/LichSu" + strNewId + ".txt TON TAI");

    // Kiem tra noi dung file [ID].txt da sinh ra
    Account accCreated;
    ErrorCode err = FileService::loadAccount(strNewId, accCreated);
    TEST_ASSERT(err == ERR_NONE, "Doc thanh cong file tai khoan moi sinh ra");
    TEST_ASSERT(accCreated.getId() == strNewId, "ID tai khoan luu chinh xac");
    TEST_ASSERT(accCreated.getName() == strName, "Ho ten luu dung voi khoang trang");
    TEST_ASSERT(accCreated.getBalance() == lBalance, "So du ban dau luu dung 2,000,000 VND");
    TEST_ASSERT(accCreated.getCurrency() == strCurrency, "Don vi tien te luu dung VND");

    // Cap nhat TheTu.txt
    LinkedList<Card> listCards;
    LinkedList<std::string> listLocked;
    FileService::loadLockedIds(listLocked);
    FileService::loadCards(listCards, listLocked);
    int iCountBefore = listCards.getSize();

    listCards.addTail(Card(strNewId, DEFAULT_PIN, false));
    bool bSaved = FileService::saveCards(listCards);
    TEST_ASSERT(bSaved, "Cap nhat the moi vao data/TheTu.txt thanh cong");

    LinkedList<Card> listCardsReload;
    FileService::loadCards(listCardsReload, listLocked);
    TEST_ASSERT(listCardsReload.getSize() == iCountBefore + 1, "TheTu.txt da tang them 1 the");

    Card* pNewCard = listCardsReload.findIf([&strNewId](const Card& c) {
        return c.getId() == strNewId;
    });
    TEST_ASSERT(pNewCard != nullptr, "The moi ton tai trong TheTu.txt");
    TEST_ASSERT(pNewCard != nullptr && pNewCard->checkPin(DEFAULT_PIN), "The moi mang PIN mac dinh 123456");

    // Don dep sau khi test
    FileService::deleteAccountFile(strNewId);
    fs::remove(strHistFile);
    listCardsReload.removeIf([&strNewId](const Card& c) { return c.getId() == strNewId; });
    FileService::saveCards(listCardsReload);
}

/******************************************************************************
 * 4. KIEM THU XOA TAI KHOAN THE TU
 * - Xoa [ID].txt khoi dia
 * - Giu lai LichSu[ID].txt cho muc dich kiem toan
 * - Xoa the khoi TheTu.txt
 ******************************************************************************/
void testAdminDeleteCard() {
    std::cout << "\n======================================================\n";
    std::cout << " 4. KIEM THU XOA TAI KHOAN THE TU (MEMBER A)\n";
    std::cout << "======================================================\n";

    FileService::initSampleData();

    const std::string strDelId = "10014504509992";
    FileService::createAccountFiles(strDelId, "Tai Khoan Xoa Test", 500000);

    LinkedList<Card> listCards;
    LinkedList<std::string> listLocked;
    FileService::loadLockedIds(listLocked);
    FileService::loadCards(listCards, listLocked);
    listCards.addTail(Card(strDelId, DEFAULT_PIN, false));
    FileService::saveCards(listCards);

    std::string strIdFile   = "data/" + strDelId + ".txt";
    std::string strHistFile = "data/LichSu" + strDelId + ".txt";
    TEST_ASSERT(fs::exists(strIdFile),   "File [ID].txt ton tai truoc khi xoa");
    TEST_ASSERT(fs::exists(strHistFile), "File LichSu[ID].txt ton tai truoc khi xoa");

    // Thuc hien xoa:
    // 1. Xoa khoi RAM & TheTu.txt
    listCards.removeIf([&strDelId](const Card& c) { return c.getId() == strDelId; });
    FileService::saveCards(listCards);

    // 2. Xoa file [ID].txt (giu lai LichSu[ID].txt)
    bool bDeleted = FileService::deleteAccountFile(strDelId);
    TEST_ASSERT(bDeleted, "deleteAccountFile() tra ve true");
    TEST_ASSERT(!fs::exists(strIdFile),  "File data/" + strDelId + ".txt da bi XOA khoi dia");
    TEST_ASSERT(fs::exists(strHistFile), "File LichSu" + strDelId + ".txt VAN DUOC GIU LAI");

    // Kiem tra khong con trong TheTu.txt
    LinkedList<Card> listReload;
    FileService::loadCards(listReload, listLocked);
    Card* pFound = listReload.findIf([&strDelId](const Card& c) { return c.getId() == strDelId; });
    TEST_ASSERT(pFound == nullptr, "The " + strDelId + " da khong con trong TheTu.txt");

    // Don dep
    fs::remove(strHistFile);
}

/******************************************************************************
 * 5. KIEM THU MO KHOA THE BI KHOA
 * - Xoa ID khoi KhoaThe.txt
 * - Reset failed attempts ve 0 trong Card
 ******************************************************************************/
void testAdminUnlockCard() {
    std::cout << "\n======================================================\n";
    std::cout << " 5. KIEM THU MO KHOA THE (MEMBER A)\n";
    std::cout << "======================================================\n";

    FileService::initSampleData();

    const std::string strLockId = "10014504500005";

    // 1. Khoa the: ghi vao KhoaThe.txt
    FileService::appendLockedCard(strLockId);

    LinkedList<std::string> listLocked;
    FileService::loadLockedIds(listLocked);
    std::string* pLocked = listLocked.findIf([&strLockId](const std::string& id) {
        return id == strLockId;
    });
    TEST_ASSERT(pLocked != nullptr, "The " + strLockId + " co trong KhoaThe.txt");

    // 2. Admin mo khoa:
    // Xoa khoi listLockedIds va luu lai KhoaThe.txt
    while (listLocked.removeIf([&strLockId](const std::string& id) {
        return id == strLockId;
    })) {}
    bool bSaveLock = FileService::saveLockedIds(listLocked);
    TEST_ASSERT(bSaveLock, "saveLockedIds() sau khi mo khoa thanh cong");

    // Kiem tra trong file
    LinkedList<std::string> listLockedCheck;
    FileService::loadLockedIds(listLockedCheck);
    std::string* pCheck = listLockedCheck.findIf([&strLockId](const std::string& id) {
        return id == strLockId;
    });
    TEST_ASSERT(pCheck == nullptr, "The " + strLockId + " da bi xoa khoi KhoaThe.txt");

    // 3. Reset failed attempts & unlock Card
    LinkedList<Card> listCards;
    FileService::loadCards(listCards, listLockedCheck);
    Card* pCard = listCards.findIf([&strLockId](const Card& c) {
        return c.getId() == strLockId;
    });
    if (pCard != nullptr) {
        pCard->unlockCard();
        TEST_ASSERT(!pCard->isLocked(), "Card khong con o trang thai bi khoa");
        TEST_ASSERT(pCard->getFailedAttempts() == 0, "So lan nhap sai da duoc reset ve 0");
    } else {
        TEST_ASSERT(false, "Khong tim thay the de kiem tra");
    }
}

/******************************************************************************
 * MAIN
 ******************************************************************************/
int main() {
    std::cout << "##############################################################\n";
    std::cout << "#      BO KIEM THU PHASE 2 - THANH VIEN A (LOGIC ADMIN)      #\n";
    std::cout << "#      Dang nhap | Xem DS | Them/Xoa the | Mo khoa the       #\n";
    std::cout << "##############################################################\n";

    testAdminLogin();
    testAdminViewCardList();
    testAdminAddAccount_DoD();
    testAdminDeleteCard();
    testAdminUnlockCard();

    std::cout << "\n======================================================\n";
    std::cout << "       TONG KET KIEM THU PHASE 2 - MEMBER A (ADMIN)   \n";
    std::cout << "======================================================\n";
    std::cout << "  So test THANH CONG [PASS]: \033[32m" << g_nPass << "\033[0m\n";
    std::cout << "  So test THAT BAI   [FAIL]: \033[31m" << g_nFail << "\033[0m\n";
    std::cout << "------------------------------------------------------\n";

    if (g_nFail == 0) {
        std::cout << "  \033[32m>>> KET LUAN: 100% KIEM THU LOGIC ADMIN CUA MEMBER A DA DAT! <<<\033[0m\n";
    } else {
        std::cout << "  \033[31m>>> KET LUAN: CO " << g_nFail << " KIEM THU THAT BAI! <<<\033[0m\n";
    }
    std::cout << "======================================================\n\n";

    return (g_nFail == 0) ? 0 : 1;
}
