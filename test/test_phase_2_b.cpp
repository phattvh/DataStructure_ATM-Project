/******************************************************************************
 * @file: test_phase_2_b.cpp
 * @description: Bo kiem thu chuyen sau Phase 2 danh cho Thanh vien B (Tri)
 *               Kiem thu toan dien Generic Template LinkedList<T>,
 *               Model Admin, Model Transaction va Tang FileService (5 loai file).
 *
 * Cach chay:
 *   make test_phase_2
 ******************************************************************************/

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cassert>
#include <chrono>
#include <filesystem>

#include "Common.h"
#include "LinkedList.h"
#include "Admin.h"
#include "Card.h"
#include "Account.h"
#include "Transaction.h"
#include "FileService.h"

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
 * 1. KIỂM THỬ GENERIC TEMPLATE LINKEDLIST<T>
 ******************************************************************************/
void testLinkedListTemplate() {
    std::cout << "\n======================================================\n";
    std::cout << " 1. KIEM THU GENERIC TEMPLATE LINKEDLIST<T> (TRI)\n";
    std::cout << "======================================================\n";

    // 1.1 Danh sach rong
    LinkedList<int> listInt;
    TEST_ASSERT(listInt.isEmpty(), "Danh sach moi khoi tao phai rong");
    TEST_ASSERT(listInt.getSize() == 0, "Kich thuoc danh sach ban dau bang 0");
    TEST_ASSERT(listInt.getHead() == nullptr, "Con tro _pHead ban dau bang nullptr");
    TEST_ASSERT(listInt.getTail() == nullptr, "Con tro _pTail ban dau bang nullptr");

    // 1.2 addTail phan tu dau tien O(1)
    listInt.addTail(100);
    TEST_ASSERT(!listInt.isEmpty(), "Danh sach sau khi addTail khong con rong");
    TEST_ASSERT(listInt.getSize() == 1, "Kich thuoc bang 1");
    TEST_ASSERT(listInt.getHead() != nullptr && listInt.getHead()->_data == 100, "Head mang gia tri 100");
    TEST_ASSERT(listInt.getTail() != nullptr && listInt.getTail()->_data == 100, "Tail mang gia tri 100");
    TEST_ASSERT(listInt.getHead() == listInt.getTail(), "Khi co 1 phan tu, Head va Tail trung nhau");

    // 1.3 addTail them cac phan tu tiep theo
    listInt.addTail(200);
    listInt.addTail(300);
    TEST_ASSERT(listInt.getSize() == 3, "Kich thuoc bang 3 sau khi them 3 phan tu");
    TEST_ASSERT(listInt.getHead()->_data == 100, "Head van la 100");
    TEST_ASSERT(listInt.getTail()->_data == 300, "Tail la phan tu cuoi 300");

    // 1.4 findIf voi Lambda
    int* pFound = listInt.findIf([](int val) { return val == 200; });
    TEST_ASSERT(pFound != nullptr && *pFound == 200, "findIf tim thay gia tri 200");

    int* pNotFound = listInt.findIf([](int val) { return val == 999; });
    TEST_ASSERT(pNotFound == nullptr, "findIf khong tim thay gia tri 999 tra ve nullptr");

    // 1.5 removeIf o giua
    bool bRemovedMid = listInt.removeIf([](int val) { return val == 200; });
    TEST_ASSERT(bRemovedMid, "removeIf xoa node o giua (200) thanh cong");
    TEST_ASSERT(listInt.getSize() == 2, "Kich thuoc giam con 2");
    TEST_ASSERT(listInt.findIf([](int val) { return val == 200; }) == nullptr, "Khong con 200 trong list");

    // 1.6 removeIf o dau (Head)
    bool bRemovedHead = listInt.removeIf([](int val) { return val == 100; });
    TEST_ASSERT(bRemovedHead, "removeIf xoa node o dau (Head: 100) thanh cong");
    TEST_ASSERT(listInt.getSize() == 1, "Kich thuoc con 1");
    TEST_ASSERT(listInt.getHead()->_data == 300, "Head moi tro dung vao 300");
    TEST_ASSERT(listInt.getHead() == listInt.getTail(), "Head va Tail gio cung tro vao 300");

    // 1.7 removeIf o cuoi (Tail)
    bool bRemovedTail = listInt.removeIf([](int val) { return val == 300; });
    TEST_ASSERT(bRemovedTail, "removeIf xoa node cuoi cung thanh cong");
    TEST_ASSERT(listInt.isEmpty(), "Danh sach tro lai trang thai rong");
    TEST_ASSERT(listInt.getHead() == nullptr && listInt.getTail() == nullptr, "Head va Tail deu ve nullptr");

    // 1.8 Stress test 10,000 phan tu do bo nho & toc do thu hoi destructor
    {
        auto tStart = std::chrono::high_resolution_clock::now();
        LinkedList<std::string> listStr;
        for (int i = 0; i < 10000; ++i) {
            listStr.addTail("ID_" + std::to_string(i));
        }
        TEST_ASSERT(listStr.getSize() == 10000, "LinkedList them thanh cong 10,000 node");
        listStr.clear();
        TEST_ASSERT(listStr.isEmpty(), "clear() thu hoi sach se 10,000 node");
        auto tEnd = std::chrono::high_resolution_clock::now();
        double dMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        std::cout << "  Thoi gian cap phat & giai phong 10,000 node: " << dMs << " ms\n";
        TEST_ASSERT(dMs < 100.0, "Hieu nang quan ly bo nho LinkedList dat chuan (< 100 ms)");
    }
}

/******************************************************************************
 * 2. KIỂM THỬ CÁC THỰC THỂ MỚI: ADMIN & TRANSACTION
 ******************************************************************************/
void testDomainModels() {
    std::cout << "\n======================================================\n";
    std::cout << " 2. KIEM THU THUC THE ADMIN & TRANSACTION (TRI)\n";
    std::cout << "======================================================\n";

    // 2.1 Model Admin
    Admin adminDefault;
    TEST_ASSERT(adminDefault.getUsername().empty(), "Admin mac dinh co username rong");

    Admin admin("admin1", "123456");
    TEST_ASSERT(admin.getUsername() == "admin1", "Admin username dung");
    TEST_ASSERT(admin.verifyPassword("123456"), "verifyPassword dung tra ve true");
    TEST_ASSERT(!admin.verifyPassword("sai_pass"), "verifyPassword sai tra ve false");

    // 2.2 Model Transaction
    std::string strNow = Transaction::getCurrentTimestamp();
    TEST_ASSERT(strNow.length() == 19, "Timestamp format dung 19 ky tu (YYYY-MM-DD HH:MM:SS)");
    TEST_ASSERT(strNow[4] == '-' && strNow[7] == '-' && strNow[10] == ' ', "Dinh dang timestamp chuan");

    Transaction trans("10014504500001", WITHDRAW, 200000, strNow, "Rut tien tai ATM cay 01");
    TEST_ASSERT(trans.getId() == "10014504500001", "ID giao dich dung");
    TEST_ASSERT(trans.getType() == WITHDRAW, "Loai giao dich WITHDRAW");
    TEST_ASSERT(trans.getTypeName() == "RUT TIEN", "Ten loai giao dich 'RUT TIEN'");
    TEST_ASSERT(trans.getAmount() == 200000, "So tien giao dich 200,000 VND");

    // Test format for file & parse back
    std::string strFileLine = trans.formatForFile();
    Transaction parsed = Transaction::parseFromFileLine("10014504500001", strFileLine);
    TEST_ASSERT(parsed.getAmount() == 200000, "Parse lai tu dong file dung so tien");
    TEST_ASSERT(parsed.getType() == WITHDRAW, "Parse lai tu dong file dung loai");
    TEST_ASSERT(parsed.getDetail() == "Rut tien tai ATM cay 01", "Parse lai tu dong file dung mo ta");
}

/******************************************************************************
 * 3. KIỂM THỬ FILESERVICE (I/O 5 LOẠI TỆP TIN TRONG DATA/)
 ******************************************************************************/
void testFileService() {
    std::cout << "\n======================================================\n";
    std::cout << " 3. KIEM THU FILESERVICE & I/O 5 TEP TIN (TRI)\n";
    std::cout << "======================================================\n";

    // 3.1 Khoi tao du lieu mau Auto-Recovery
    FileService::initSampleData();
    TEST_ASSERT(std::filesystem::exists("data/Admin.txt"), "data/Admin.txt da duoc tao");
    TEST_ASSERT(std::filesystem::exists("data/TheTu.txt"), "data/TheTu.txt da duoc tao");
    TEST_ASSERT(std::filesystem::exists("data/KhoaThe.txt"), "data/KhoaThe.txt da duoc tao");

    // 3.2 Load danh sach Admin
    LinkedList<Admin> listAdmins;
    bool bLoadAdmin = FileService::loadAdmins(listAdmins);
    TEST_ASSERT(bLoadAdmin, "loadAdmins thanh cong");
    TEST_ASSERT(listAdmins.getSize() >= 3, "Danh sach Admin co it nhat 3 tai khoan");
    Admin* pAdmin = listAdmins.findIf([](const Admin& a) { return a.getUsername() == "admin1"; });
    TEST_ASSERT(pAdmin != nullptr && pAdmin->verifyPassword("123456"), "Tim thay admin1 va pass khop");

    // 3.3 Load danh sach KhoaThe va TheTu
    LinkedList<std::string> listLocked;
    FileService::loadLockedIds(listLocked);

    LinkedList<Card> listCards;
    bool bLoadCards = FileService::loadCards(listCards, listLocked);
    TEST_ASSERT(bLoadCards, "loadCards thanh cong");
    TEST_ASSERT(listCards.getSize() >= 10, "TheTu.txt co it nhat 10 the mau");

    Card* pCard1 = listCards.findIf([](const Card& c) { return c.getId() == "10014504500001"; });
    TEST_ASSERT(pCard1 != nullptr, "Tim thay the 10014504500001");
    TEST_ASSERT(pCard1->checkPin("123456"), "Ma PIN the 10014504500001 dung 123456");

    // 3.4 Load thong tin tai khoan [ID].txt (Bay ky thuat Ho ten co khoang trang)
    Account accUser;
    ErrorCode errLoad = FileService::loadAccount("10014504500001", accUser);
    TEST_ASSERT(errLoad == ERR_NONE, "loadAccount 10014504500001 thanh cong (ERR_NONE)");
    TEST_ASSERT(accUser.getName() == "Nguyen Trung Kien", "Ho ten doc dung co khoang trang 'Nguyen Trung Kien'");
    TEST_ASSERT(accUser.getBalance() == 5000000, "So du doc dung 5,000,000 VND");

    // 3.5 Test Save tai khoan va cap nhat so du
    accUser.setBalance(5500000);
    bool bSaveAcc = FileService::saveAccount(accUser);
    TEST_ASSERT(bSaveAcc, "saveAccount thanh cong");

    Account accReload;
    FileService::loadAccount("10014504500001", accReload);
    TEST_ASSERT(accReload.getBalance() == 5500000, "So du duoc ghi de va luu ben vung tren dia");

    // Khoi phuc lai so du goc
    accUser.setBalance(5000000);
    FileService::saveAccount(accUser);

    // 3.6 Test appendLockedCard (GIAI QUYET TRIET DE LOI #3)
    const std::string strLockedIdTest = "10014504509999";
    bool bAppendLock = FileService::appendLockedCard(strLockedIdTest);
    TEST_ASSERT(bAppendLock, "appendLockedCard ghi ID vao data/KhoaThe.txt thanh cong");

    LinkedList<std::string> listLockedNew;
    FileService::loadLockedIds(listLockedNew);
    std::string* pFoundLock = listLockedNew.findIf([&strLockedIdTest](const std::string& id) {
        return id == strLockedIdTest;
    });
    TEST_ASSERT(pFoundLock != nullptr, "KhoaThe.txt thuc su chua ma ID bi khoa vua ghi");

    // Xoa ID test khoi KhoaThe.txt de giu sach du lieu
    listLockedNew.removeIf([&strLockedIdTest](const std::string& id) {
        return id == strLockedIdTest;
    });
    FileService::saveLockedIds(listLockedNew);

    // 3.7 Test ghi va doc LichSu[ID].txt
    std::string strTime = Transaction::getCurrentTimestamp();
    Transaction transTest("10014504500001", TRANSFER, 100000, strTime, "Chuyen tien test Member B");
    bool bAppTrans = FileService::appendTransaction("10014504500001", transTest);
    TEST_ASSERT(bAppTrans, "appendTransaction vao LichSu10014504500001.txt thanh cong");

    LinkedList<Transaction> listHist;
    bool bLoadHist = FileService::loadTransactions("10014504500001", listHist);
    TEST_ASSERT(bLoadHist, "loadTransactions thanh cong");
    TEST_ASSERT(listHist.getSize() >= 1, "Doc duoc it nhat 1 giao dich trong lich su");

    // 3.8 Test tao va xoa file tai khoan moi
    const std::string strNewId = "10014504509988";
    bool bCreate = FileService::createAccountFiles(strNewId, "Le Thi Test", 1000000);
    TEST_ASSERT(bCreate, "createAccountFiles tao ca 2 file thanh cong");
    TEST_ASSERT(std::filesystem::exists("data/" + strNewId + ".txt"), "File [ID].txt ton tai");
    TEST_ASSERT(std::filesystem::exists("data/LichSu" + strNewId + ".txt"), "File LichSu[ID].txt ton tai");

    bool bDel = FileService::deleteAccountFile(strNewId);
    TEST_ASSERT(bDel, "deleteAccountFile xoa file [ID].txt thanh cong");
    TEST_ASSERT(!std::filesystem::exists("data/" + strNewId + ".txt"), "File [ID].txt da bi xoa khoi dia");
    std::filesystem::remove("data/LichSu" + strNewId + ".txt");
}

int main() {
    std::cout << "##############################################################\n";
    std::cout << "#      BO KIEM THU CHUYEN SAU PHASE 2 - THANH VIEN B (TRI)   #\n";
    std::cout << "##############################################################\n";

    testLinkedListTemplate();
    testDomainModels();
    testFileService();

    std::cout << "\n======================================================\n";
    std::cout << "             TONG KET KIEM THU PHASE 2                \n";
    std::cout << "======================================================\n";
    std::cout << "  So test THANH CONG [PASS]: \033[32m" << g_nPass << "\033[0m\n";
    std::cout << "  So test THAT BAI   [FAIL]: \033[31m" << g_nFail << "\033[0m\n";
    std::cout << "------------------------------------------------------\n";

    if (g_nFail == 0) {
        std::cout << "  \033[32m>>> KET LUAN: 100% KIEM THU PHASE 2 CUA TRI DA DAT! <<<\033[0m\n";
    } else {
        std::cout << "  \033[31m>>> KET LUAN: CO " << g_nFail << " KIEM THU THAT BAI! <<<\033[0m\n";
    }
    std::cout << "======================================================\n\n";

    return (g_nFail == 0) ? 0 : 1;
}
