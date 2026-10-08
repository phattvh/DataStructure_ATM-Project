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

/******************************************************************************
 * 8. KIỂM THỬ CÁC BẢN VÁ TỪ CODE REVIEW (PRE-RELEASE FIXES)
 ******************************************************************************/
void testReviewBugFixes() {
    std::cout << "\n======================================================\n";
    std::cout << " 8. KIEM THU CAC BAN VA TU CODE REVIEW (FIXES)\n";
    std::cout << "======================================================\n";

    // 8.1 Chặn đổi mã PIN mới trùng mã PIN mặc định DEFAULT_PIN (123456)
    {
        Card card("10014504500001", "123456");
        std::string strMsg;
        bool bResDefault = UserController::processChangePin(card, "123456", "123456", "123456", strMsg);
        TEST_ASSERT(!bResDefault, "Chan doi PIN moi ve ma mac dinh 123456");
        TEST_ASSERT(strMsg.find(DEFAULT_PIN) != std::string::npos, "Thong bao loi co chua DEFAULT_PIN");

        bool bResValid = UserController::processChangePin(card, "123456", "654321", "654321", strMsg);
        TEST_ASSERT(bResValid, "Doi sang ma PIN hop le (654321) thanh cong");
        TEST_ASSERT(card.getPin() == "654321", "Ma PIN moi da duoc cap nhat vao the");

        bool bResBackToDefault = UserController::processChangePin(card, "654321", "123456", "123456", strMsg);
        TEST_ASSERT(!bResBackToDefault, "Chan doi PIN nguoc ve ma mac dinh 123456");
        TEST_ASSERT(strMsg.find(DEFAULT_PIN) != std::string::npos, "Thong bao loi bao trung DEFAULT_PIN");
    }

    // 8.2 An toàn biệt lệ khi parse dòng lịch sử giao dịch bị lỗi/hỏng (Bug C2)
    {
        bool bNoThrow = true;
        try {
            // Dòng sai định dạng số (chữ cái thay vì số nguyên)
            Transaction tx1 = Transaction::parseFromFileLine("10014504500001", "2026-10-08 10:00:00\tabc\txyz\tLoi corrupt");
            TEST_ASSERT(tx1.getAmount() == 0, "Dòng corrupt amount duoc fallback an toan ve 0");

            // Dòng thiếu trường dữ liệu
            Transaction tx2 = Transaction::parseFromFileLine("10014504500001", "2026-10-08 10:00:00\t1");
            TEST_ASSERT(tx2.getAmount() == 0, "Dòng thieu tokens duoc fallback an toan");
        } catch (...) {
            bNoThrow = false;
        }
        TEST_ASSERT(bNoThrow, "Transaction::parseFromFileLine an toan tuyet doi khong throw unhandled exception");
    }

    // 8.3 Mở khóa thẻ loại bỏ triệt để mọi bản ghi trùng lặp (Bug M3)
    {
        AtmController atm;
        atm.initData();
        std::string strDupId = "10014504506666";
        atm.addCardAccount(strDupId, "Nguoi Dung Test Dup", 300000, "VND");

        // Giả lập thẻ bị khoá
        auto pCard = atm.getCards().findIf([&](const Card& c) {
            return c.getId() == strDupId;
        });
        if (pCard != nullptr) {
            pCard->setLocked(true);
            // Ghi đúp 2 lần vào danh sách khóa
            FileService::appendLockedCard(strDupId);
            FileService::appendLockedCard(strDupId);
            atm.initData();

            // Mở khóa
            ErrorCode errUnlock = atm.unlockCardAccount(strDupId);
            TEST_ASSERT(errUnlock == ERR_NONE, "Mo khoa the co duplicate ID thanh cong");

            // Đảm bảo không còn bất kỳ bản sao nào trong RAM
            int nCount = 0;
            auto pCur = atm.getLockedIds().getHead();
            while (pCur != nullptr) {
                if (pCur->_data == strDupId) {
                    nCount++;
                }
                pCur = pCur->_pNext;
            }
            TEST_ASSERT(nCount == 0, "Khong con bat ky ban sao nao cua ID trong danh sach khoa");
        }
        atm.deleteCardAccount(strDupId);
        std::filesystem::remove("data/LichSu" + strDupId + ".txt");
    }

    // 8.4 Kiểm thử tính toàn vẹn số dư trong chuyển tiền (Bảo toàn tổng tài sản)
    {
        Account accSender("10014504500001", "Nguoi Gui", 1000000, "VND");
        Account accReceiver("10014504500002", "Nguoi Nhan", 500000, "VND");
        long lTotalBefore = accSender.getBalance() + accReceiver.getBalance();

        ErrorCode err = UserController::processTransfer(accSender, accReceiver, 200000);
        TEST_ASSERT(err == ERR_NONE, "Chuyen tien hop le giua 2 tai khoan thanh cong");
        TEST_ASSERT(accSender.getBalance() == 800000, "So du nguoi gui giam dung 200,000 VND");
        TEST_ASSERT(accReceiver.getBalance() == 700000, "So du nguoi nhan tang dung 200,000 VND");
        long lTotalAfter = accSender.getBalance() + accReceiver.getBalance();
        TEST_ASSERT(lTotalBefore == lTotalAfter, "Tong so du he thong duoc bao toan tuyet doi (Khong mat tien)");
    }
}

/******************************************************************************
 * 9. KIỂM THỬ GIA CỐ BẢO MẬT & ĐỘ BỀN DỮ LIỆU (ADVERSARIAL HARDENING FIXES)
 ******************************************************************************/
void testAdversarialHardening() {
    std::cout << "\n======================================================\n";
    std::cout << " 9. KIEM THU GIA CO ADVERSARIAL HARDENING FIXES\n";
    std::cout << "======================================================\n";

    // 9.1 Phòng chống lỗi Silent Balance Annihilation (Cắt trắng số dư về 0)
    {
        std::string strCorruptId = "10014504509991";
        std::ofstream fout("data/" + strCorruptId + ".txt");
        fout << strCorruptId << "\n";
        fout << "Khach Hang Loi\n";
        fout << "5000000_VND\n"; // Loi du lieu dinh dang so
        fout << "VND\n";
        fout.close();

        Account accTest;
        ErrorCode err = FileService::loadAccount(strCorruptId, accTest);
        TEST_ASSERT(err == ERR_INVALID_FORMAT, "File so du bi loi dinh dang tra ve ERR_INVALID_FORMAT");
        TEST_ASSERT(accTest.getBalance() == 0, "Account khong duoc nap vao he thong");

        std::filesystem::remove("data/" + strCorruptId + ".txt");
    }

    // 9.2 Bền vững mã PIN: Lưu ngay tức thì xuống TheTu.txt không đợi đăng xuất
    {
        std::string strPinCardId = "10014504500001";
        bool bUpdated = FileService::updateCardPin(strPinCardId, "654321");
        TEST_ASSERT(bUpdated, "updateCardPin truc tiep tren dia thanh cong");

        // Doc lai TheTu.txt doc lap tu dia de kiem chung tinh ben vung
        LinkedList<std::string> listLocked;
        FileService::loadLockedIds(listLocked);
        LinkedList<Card> listCardsDisk;
        FileService::loadCards(listCardsDisk, listLocked);

        auto pCard = listCardsDisk.findIf([&](const Card& c) {
            return c.getId() == strPinCardId;
        });
        TEST_ASSERT(pCard != nullptr && pCard->getPin() == "654321", "TheTu.txt tren dia da luu ma PIN moi ngay lap tuc");

        // Tra lai ma PIN cu de bao toan bo test
        FileService::updateCardPin(strPinCardId, "123456");
    }

    // 9.3 Tách biệt lịch sử kiểm toán chống rò rỉ thông tin khi tái cấp thẻ cũ
    {
        std::string strRecycledId = "10014504509992";
        // 1. Tao the cu va co lich su
        FileService::createAccountFiles(strRecycledId, "Chu The Cu", 500000, "VND");
        Transaction txOld(strRecycledId, WITHDRAW, 100000, getNowTimestamp(), "Rut tien chu the cu");
        FileService::appendTransaction(strRecycledId, txOld);

        // 2. Xoa file tai khoan chu the cu (giu lai LichSu theo quy dinh de tra soat)
        FileService::deleteAccountFile(strRecycledId);

        // 3. Admin tao the moi cho chu the khac voi cung ID
        FileService::createAccountFiles(strRecycledId, "Chu The Moi", 1000000, "VND");

        // 4. Kiem tra lich su cua chu the moi phai sach tinh 100%
        LinkedList<Transaction> listNewHistory;
        FileService::loadTransactions(strRecycledId, listNewHistory);
        TEST_ASSERT(listNewHistory.isEmpty(), "Chu the moi co lich su trang tinh khong bi ro ri thong tin chu cu");

        // Don dep
        FileService::deleteAccountFile(strRecycledId);
        std::filesystem::remove("data/LichSu" + strRecycledId + ".txt");
        // Xoa file archive neu co
        for (const auto& entry : std::filesystem::directory_iterator("data/")) {
            std::string pathStr = entry.path().string();
            if (pathStr.find("Archive_LichSu" + strRecycledId) != std::string::npos) {
                std::filesystem::remove(entry.path());
            }
        }
    }

    // 9.4 Chặn ký tự phân cách '|' và tên chỉ toàn khoảng trắng (Delimiter Injection Defense)
    {
        AtmController atm;
        atm.initData();

        ErrorCode errInject = atm.addCardAccount("10014504509993", "Nguyen Van A|1|999999", 100000, "VND");
        TEST_ASSERT(errInject == ERR_INVALID_FORMAT, "Chan ho ten chua ky tu phan cach '|'");

        ErrorCode errSpaces = atm.addCardAccount("10014504509994", "    ", 100000, "VND");
        TEST_ASSERT(errSpaces == ERR_INVALID_FORMAT, "Chan ho ten chi toan khoang trang");
    }

    // 9.5 Ghép nối an toàn chi tiết giao dịch khi chứa ký tự phân cách '|'
    {
        std::string strLine = "2026-10-08 10:00:00|2|100000|Chuyen tien | Kem loi nhan | Uu tien";
        Transaction tx = Transaction::parseFromFileLine("10014504500001", strLine);
        TEST_ASSERT(tx.getDetail() == "Chuyen tien | Kem loi nhan | Uu tien", "Bao toan toan ven noi dung chi tiet chua ky tu '|'");
        TEST_ASSERT(tx.getAmount() == 100000, "Parse dung so tien 100,000 VND");
        TEST_ASSERT(tx.getType() == TRANSFER, "Parse dung loai giao dich TRANSFER");
    }
}

void testFinalProductionHardening() {
    std::cout << "\n======================================================\n";
    std::cout << " 10. KIEM THU GIA CO CUOI CUNG (FINAL PRODUCTION POLISHING)\n";
    std::cout << "======================================================\n";

    // 10.1 Kiem tra bat loi nhap chuoi ky tu du trong menu (Strict Numeric Menu Input)
    {
        std::istringstream iss("1abc\nxyz\n  \n2\n");
        int iChoice = ConsoleView::inputMenuChoice(1, 3, "", iss);
        TEST_ASSERT(iChoice == 2, "inputMenuChoice tu choi cac chuoi '1abc', 'xyz', khoang trang va nhan gia tri 2");
    }

    // 10.2 Kiem tra tu choi va cham tap tin tai khoan mo coi (Orphan Account Collision)
    {
        AtmController atm;
        atm.initData();

        std::string strOrphanId = "10014504508888";
        // Tao thu cong file tai khoan mo coi tren dia ma khong co trong danh sach the
        std::ofstream fout("data/" + strOrphanId + ".txt");
        fout << strOrphanId << "\nOrphan User\n500000\nVND\n";
        fout.close();

        // Admin co gang them the co ID trung voi file mo coi tren dia
        ErrorCode errOrphan = atm.addCardAccount(strOrphanId, "New Card Holder", 100000, "VND");
        TEST_ASSERT(errOrphan == ERR_ID_EXISTS, "addCardAccount tu choi de file tai khoan mo coi tren dia (ERR_ID_EXISTS)");

        // Don dep file mo coi
        std::filesystem::remove("data/" + strOrphanId + ".txt");
    }

    // 10.3 Kiem thu nhat ky kiem toan quan tri (Admin Audit Log)
    {
        AtmController atm;
        atm.initData();

        std::string strAuditId = "10014504507777";
        // 1. Them the moi
        ErrorCode errAdd = atm.addCardAccount(strAuditId, "Audit Subject", 150000, "VND");
        TEST_ASSERT(errAdd == ERR_NONE, "Them the audit thanh cong");

        // 2. Kiem tra log AdminLog.txt da ghi lai ADD_CARD
        std::ifstream finLog("data/AdminLog.txt");
        TEST_ASSERT(finLog.is_open(), "File data/AdminLog.txt duoc tao tu dong");
        std::string strLogContent((std::istreambuf_iterator<char>(finLog)),
                                  std::istreambuf_iterator<char>());
        finLog.close();

        TEST_ASSERT(strLogContent.find("ADD_CARD") != std::string::npos, "AdminLog co ghi su kien ADD_CARD");
        TEST_ASSERT(strLogContent.find(strAuditId) != std::string::npos, "AdminLog co ghi ma the " + strAuditId);

        // 3. Xoa the va kiem tra log DELETE_CARD
        ErrorCode errDel = atm.deleteCardAccount(strAuditId);
        TEST_ASSERT(errDel == ERR_NONE, "Xoa the audit thanh cong");

        std::ifstream finLog2("data/AdminLog.txt");
        std::string strLogContent2((std::istreambuf_iterator<char>(finLog2)),
                                   std::istreambuf_iterator<char>());
        finLog2.close();

        TEST_ASSERT(strLogContent2.find("DELETE_CARD") != std::string::npos, "AdminLog co ghi su kien DELETE_CARD");
        TEST_ASSERT(strLogContent2.find("So du con lai: 150000 VND") != std::string::npos, "AdminLog ghi dung so du con lai truoc khi xoa");

        // Don dep
        std::filesystem::remove("data/LichSu" + strAuditId + ".txt");
        std::filesystem::remove("data/AdminLog.txt");
    }
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
    testReviewBugFixes();
    testAdversarialHardening();
    testFinalProductionHardening();

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
