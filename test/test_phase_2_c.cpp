/******************************************************************************
 * @file: test_phase_2_c.cpp
 * @description: Bo kiem thu chuyen sau Phase 2 danh cho Thanh vien C (Phat)
 *               Kiem thu toan dien getNowTimestamp(), AtmController,
 *               xac thuc Admin, nghiep vu Admin (Xem, Them, Xoa, Mo khoa)
 *               va quan ly phien lam viec (Session management).
 *
 * Cach chay:
 *   make test_phase_2_c
 ******************************************************************************/

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>

#include "Common.h"
#include "LinkedList.h"
#include "Admin.h"
#include "Card.h"
#include "Account.h"
#include "Transaction.h"
#include "FileService.h"
#include "AtmController.h"

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
 * 1. KIỂM THỬ HÀM TIỆN ÍCH getNowTimestamp() TRONG Common.h (C08)
 ******************************************************************************/
void testGetNowTimestamp() {
    std::cout << "\n======================================================\n";
    std::cout << " 1. KIEM THU getNowTimestamp() TRONG Common.h (C08)\n";
    std::cout << "======================================================\n";

    std::string strTime = getNowTimestamp();
    std::cout << "  Chuoi thoi gian he thong: " << strTime << "\n";

    TEST_ASSERT(strTime.length() == 19, "Do dai chuoi thoi gian phai dung 19 ky tu");
    TEST_ASSERT(strTime[4] == '-' && strTime[7] == '-', "Dinh dang nam-thang-ngay co dau '-'");
    TEST_ASSERT(strTime[10] == ' ', "Phan cach giua ngay va gio bang dau cach");
    TEST_ASSERT(strTime[13] == ':' && strTime[16] == ':', "Dinh dang gio:phut:giay co dau ':'");

    // Kiem tra cac chu so la digit
    bool bDigitsOk = true;
    for (size_t i = 0; i < strTime.length(); ++i) {
        if (i == 4 || i == 7 || i == 10 || i == 13 || i == 16) continue;
        if (!std::isdigit(static_cast<unsigned char>(strTime[i]))) {
            bDigitsOk = false;
            break;
        }
    }
    TEST_ASSERT(bDigitsOk, "Cac thanh phan con lai deu la chu so hop le");
}

/******************************************************************************
 * 2. KIỂM THỬ KHỞI TẠO VÀ NẠP DỮ LIỆU AtmController (C09, C10)
 ******************************************************************************/
void testAtmControllerInit() {
    std::cout << "\n======================================================\n";
    std::cout << " 2. KIEM THU KHOI TAO & NAP DU LIEU AtmController (C09)\n";
    std::cout << "======================================================\n";

    AtmController atm;
    TEST_ASSERT(atm.getCurrentRole() == ROLE_NONE, "Vai tro ban dau phai la ROLE_NONE");
    TEST_ASSERT(atm.getCurrentAccount() == nullptr, "Con tro Account ban dau bang nullptr");
    TEST_ASSERT(atm.getCurrentCard() == nullptr, "Con tro Card ban dau bang nullptr");

    bool bInit = atm.initData();
    TEST_ASSERT(bInit, "initData() nap thanh cong du lieu he thong");
    TEST_ASSERT(atm.getAdmins().getSize() >= 3, "Danh sach Admin nap duoc it nhat 3 tai khoan");
    TEST_ASSERT(atm.getCards().getSize() >= 10, "Danh sach TheTu nap duoc it nhat 10 the");
}

/******************************************************************************
 * 3. KIỂM THỬ XÁC THỰC QUẢN TRỊ VIÊN ADMIN (C11)
 ******************************************************************************/
void testAdminAuthentication() {
    std::cout << "\n======================================================\n";
    std::cout << " 3. KIEM THU XAC THUC QUAN TRI VIEN ADMIN (C11)\n";
    std::cout << "======================================================\n";

    AtmController atm;
    atm.initData();

    // 3.1 Xac thuc dung admin1 / 123456
    TEST_ASSERT(atm.authenticateAdmin("admin1", "123456"), "Dang nhap admin1 dung mat khau tra ve true");

    // 3.2 Xac thuc sai mat khau
    TEST_ASSERT(!atm.authenticateAdmin("admin1", "wrongpass"), "Dang nhap admin1 sai mat khau tra ve false");

    // 3.3 Xac thuc user khong ton tai
    TEST_ASSERT(!atm.authenticateAdmin("ghost_admin", "123456"), "User admin khong ton tai tra ve false");

    // 3.4 Username/Pass rong
    TEST_ASSERT(!atm.authenticateAdmin("", "admin123"), "Username rong tra ve false");
    TEST_ASSERT(!atm.authenticateAdmin("admin1", ""), "Password rong tra ve false");
}

/******************************************************************************
 * 4. KIỂM THỬ ADMIN: THÊM THẺ MỚI (C11)
 ******************************************************************************/
void testAdminAddCard() {
    std::cout << "\n======================================================\n";
    std::cout << " 4. KIEM THU ADMIN: THEM THE MOI (C11)\n";
    std::cout << "======================================================\n";

    AtmController atm;
    atm.initData();

    std::string strTestId = "10014504509999";
    std::string strTestName = "Nguyen Van Test";
    long lTestBalance = 500000;

    // Don dep phong khi file da ton tai
    FileService::deleteAccountFile(strTestId);

    // 4.1 Them the hop le
    ErrorCode err = atm.addCardAccount(strTestId, strTestName, lTestBalance, "VND");
    TEST_ASSERT(err == ERR_NONE, "Them the hop le tra ve ERR_NONE");

    // Kiem tra the da ton tai trong RAM
    auto pCard = atm.getCards().findIf([&](const Card& c) {
        return c.getId() == strTestId;
    });
    TEST_ASSERT(pCard != nullptr, "The moi duoc them vao danh sach RAM");
    if (pCard != nullptr) {
        TEST_ASSERT(pCard->getPin() == DEFAULT_PIN, "Mã PIN ban dau dung 123456");
        TEST_ASSERT(!pCard->isLocked(), "The moi tao o trang thai chua bi khoa");
    }

    // Kiem tra tap tin vat ly da duoc sinh ra
    TEST_ASSERT(std::filesystem::exists("data/" + strTestId + ".txt"), "File data/[ID].txt da duoc tao");
    TEST_ASSERT(std::filesystem::exists("data/LichSu" + strTestId + ".txt"), "File data/LichSu[ID].txt da duoc tao");

    // Kiem tra noi dung file
    Account accRead;
    ErrorCode errRead = FileService::loadAccount(strTestId, accRead);
    TEST_ASSERT(errRead == ERR_NONE, "Doc lai du lieu file [ID].txt thanh cong");
    TEST_ASSERT(accRead.getName() == strTestName, "Ho ten doc tu file dung chuan");
    TEST_ASSERT(accRead.getBalance() == lTestBalance, "So du doc tu file dung chuan");

    // 4.2 Chan them the trung ID
    ErrorCode errDup = atm.addCardAccount(strTestId, "Trung Lap", 100000, "VND");
    TEST_ASSERT(errDup == ERR_ID_EXISTS, "Chan them the trung ID tra ve ERR_ID_EXISTS");

    // 4.3 Chan them the sai do dai ID
    ErrorCode errShort = atm.addCardAccount("123", "Ngan Qua", 100000, "VND");
    TEST_ASSERT(errShort == ERR_INVALID_FORMAT, "Chan ID ngan tra ve ERR_INVALID_FORMAT");

    // 4.4 Chan them the ID chua chu cai
    ErrorCode errAlpha = atm.addCardAccount("1001450450ABCD", "Chua Chu", 100000, "VND");
    TEST_ASSERT(errAlpha == ERR_INVALID_FORMAT, "Chan ID chua chu tra ve ERR_INVALID_FORMAT");

    // 4.5 Chan so du duoi 50k
    ErrorCode errLow = atm.addCardAccount("10014504508881", "It Tien", 20000, "VND");
    TEST_ASSERT(errLow == ERR_INVALID_AMOUNT, "Chan so du < 50k tra ve ERR_INVALID_AMOUNT");

    // 4.6 Chan so du khong phai boi so 50k
    ErrorCode errNotMul = atm.addCardAccount("10014504508882", "Le Tien", 75000, "VND");
    TEST_ASSERT(errNotMul == ERR_NOT_MULTIPLE, "Chan so du khong chia het cho 50k tra ve ERR_NOT_MULTIPLE");
}

/******************************************************************************
 * 5. KIỂM THỬ ADMIN: XÓA THẺ (C11)
 ******************************************************************************/
void testAdminDeleteCard() {
    std::cout << "\n======================================================\n";
    std::cout << " 5. KIEM THU ADMIN: XOA THE (C11)\n";
    std::cout << "======================================================\n";

    AtmController atm;
    atm.initData();

    std::string strTestId = "10014504509999";

    // 5.1 Xoa the ton tai
    ErrorCode err = atm.deleteCardAccount(strTestId);
    TEST_ASSERT(err == ERR_NONE, "Xoa the hop le tra ve ERR_NONE");

    // Kiem tra the khong con trong RAM
    auto pCard = atm.getCards().findIf([&](const Card& c) {
        return c.getId() == strTestId;
    });
    TEST_ASSERT(pCard == nullptr, "The da bi xoa khoi danh sach trong RAM");

    // File data/[ID].txt phai bi xoa
    TEST_ASSERT(!std::filesystem::exists("data/" + strTestId + ".txt"), "File data/[ID].txt da bi xoa tren dia");

    // File LichSu[ID].txt phai DUOC GIU LAI de tra soat theo yeu cau de bai
    TEST_ASSERT(std::filesystem::exists("data/LichSu" + strTestId + ".txt"),
                "File LichSu[ID].txt van duoc giu lai de tra soat kiem toan");

    // Don dep file LichSu sau khi test
    std::filesystem::remove("data/LichSu" + strTestId + ".txt");

    // 5.2 Xoa the khong ton tai
    ErrorCode errNotFound = atm.deleteCardAccount("99999999999999");
    TEST_ASSERT(errNotFound == ERR_ID_NOT_FOUND, "Xoa the khong ton tai tra ve ERR_ID_NOT_FOUND");
}

/******************************************************************************
 * 6. KIỂM THỬ ADMIN: MỞ KHÓA THẺ BỊ KHÓA (C11)
 ******************************************************************************/
void testAdminUnlockCard() {
    std::cout << "\n======================================================\n";
    std::cout << " 6. KIEM THU ADMIN: MO KHOA THE BI KHOA (C11)\n";
    std::cout << "======================================================\n";

    AtmController atm;
    atm.initData();

    std::string strTestId = "10014504507777";
    atm.addCardAccount(strTestId, "Tran Khoa The", 200000, "VND");

    // Gia lap the bi khoa do nhap sai PIN 3 lan
    auto pCard = atm.getCards().findIf([&](const Card& c) {
        return c.getId() == strTestId;
    });
    TEST_ASSERT(pCard != nullptr, "Tim thay the test mo khoa");

    if (pCard != nullptr) {
        pCard->recordFailedAttempt();
        pCard->recordFailedAttempt();
        pCard->recordFailedAttempt();
        TEST_ASSERT(pCard->isLocked(), "The da bi khoa sau 3 lan recordFailedAttempt");

        // Ghi vao KhoaThe.txt
        FileService::appendLockedCard(strTestId);

        // Nap lai de atm dong bo trang thai khoa
        atm.initData();

        // Kiem tra strTestId co trong _listLockedIds
        auto pLock = atm.getLockedIds().findIf([&](const std::string& id) {
            return id == strTestId;
        });
        TEST_ASSERT(pLock != nullptr, "The da duoc ghi nhan trong danh sach khoa");

        // Thuc hien mo khoa
        ErrorCode errUnlock = atm.unlockCardAccount(strTestId);
        TEST_ASSERT(errUnlock == ERR_NONE, "Mo khoa the tra ve ERR_NONE");

        // Kiem tra the khong con trong danh sach khoa
        auto pLockAfter = atm.getLockedIds().findIf([&](const std::string& id) {
            return id == strTestId;
        });
        TEST_ASSERT(pLockAfter == nullptr, "The da bi go khoi danh sach khoa trong RAM");

        // Kiem tra Card tren RAM da mo khoa va reset failed attempts
        auto pCardAfter = atm.getCards().findIf([&](const Card& c) {
            return c.getId() == strTestId;
        });
        TEST_ASSERT(pCardAfter != nullptr && !pCardAfter->isLocked(), "Card khong con o trang thai khoa");
        TEST_ASSERT(pCardAfter != nullptr && pCardAfter->getFailedAttempts() == 0,
                    "So lan nhap sai cua Card da duoc reset ve 0");
    }

    // Don dep the test
    atm.deleteCardAccount(strTestId);
    std::filesystem::remove("data/LichSu" + strTestId + ".txt");
}

/******************************************************************************
 * 7. KIỂM THỬ QUẢN LÝ PHIÊN (SESSION & RAII) (C09)
 ******************************************************************************/
void testAtmSessionLifecycle() {
    std::cout << "\n======================================================\n";
    std::cout << " 7. KIEM THU SESSION LIFECYCLE & RAII (C09)\n";
    std::cout << "======================================================\n";

    {
        AtmController atm;
        atm.initData();
        TEST_ASSERT(atm.getCurrentRole() == ROLE_NONE, "Role ban dau dung ROLE_NONE");
        atm.cleanupSession();
        TEST_ASSERT(atm.getCurrentAccount() == nullptr, "cleanupSession dat _pCurrentAccount ve nullptr an toan");
    }
    TEST_ASSERT(true, "Destructor cua AtmController thu hoi bo nho an toan khong crash");
}

int main() {
    std::cout << "##############################################################\n";
    std::cout << "#      BO KIEM THU CHUYEN SAU PHASE 2 - THANH VIEN C (PHAT)  #\n";
    std::cout << "##############################################################\n";

    testGetNowTimestamp();
    testAtmControllerInit();
    testAdminAuthentication();
    testAdminAddCard();
    testAdminDeleteCard();
    testAdminUnlockCard();
    testAtmSessionLifecycle();

    std::cout << "\n======================================================\n";
    std::cout << "             TONG KET KIEM THU PHASE 2 (PHAT)         \n";
    std::cout << "======================================================\n";
    std::cout << "  So test THANH CONG [PASS]: \033[32m" << g_nPass << "\033[0m\n";
    std::cout << "  So test THAT BAI   [FAIL]: \033[31m" << g_nFail << "\033[0m\n";
    std::cout << "------------------------------------------------------\n";

    if (g_nFail == 0) {
        std::cout << "  >>> KET LUAN: 100% KIEM THU PHASE 2 CUA PHAT DA DAT! <<<\n";
    } else {
        std::cout << "  >>> CANH BAO: CO " << g_nFail << " KIEM THU THAT BAI! <<<\n";
    }
    std::cout << "======================================================\n\n";

    return (g_nFail == 0) ? 0 : 1;
}
