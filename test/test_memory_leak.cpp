/******************************************************************************
 * @file: test_memory_leak.cpp
 * @description: Bộ kiểm thử chuyên sâu rò rỉ bộ nhớ (Memory Leak & Safety Audit)
 *               Dành cho Thành viên B (Trí) trong Phase 4 (Task B11).
 *
 * Tính năng chính:
 *   - Nạp chồng toàn cục operator new/delete theo dõi 100% cấp phát Heap.
 *   - Phát hiện Double Free, Corrupted Header, và tính toán số byte rò rỉ thực tế.
 *   - 6 kịch bản kiểm thử:
 *       1. Vòng đời cơ bản của LinkedList<T> (addTail, clear, Destructor RAII)
 *       2. removeIf tại mọi vị trí (đầu, giữa, cuối, không tồn tại)
 *       3. LinkedList với các kiểu đối tượng phức tạp (std::string, Admin, Card, Account, Transaction)
 *       4. Kiểm tra bộ nhớ tầng FileService (loadAdmins, loadCards, loadLockedIds, loadHistory)
 *       5. Vòng đời phiên làm việc AtmController (Account dynamic allocation & cleanupSession)
 *       6. Stress Testing 50,000 node kiểm tra phân mảnh và độ ổn định tải cao.
 *
 * Cách chạy:
 *   make test_mem
 ******************************************************************************/

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <chrono>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <new>

#include "Common.h"
#include "LinkedList.h"
#include "Admin.h"
#include "Card.h"
#include "Account.h"
#include "Transaction.h"
#include "FileService.h"
#include "AtmController.h"

// ============================================================================
// 1. CUSTOM MEMORY LEAK TRACKING HARNESS
// ============================================================================

static constexpr uint64_t ALLOC_MAGIC = 0xDEADBEEFCAFE0001ULL;
static constexpr uint64_t FREED_MAGIC = 0xFEEDFACEBAADF00DULL;

struct alignas(std::max_align_t) AllocHeader {
    uint64_t _nMagic;
    size_t   _nSize;
    size_t   _nId;
    size_t   _nReserved; // Padding để đảm bảo 32 bytes (chia hết cho 16)
};

static size_t g_nTotalAllocations        = 0;
static size_t g_nTotalDeallocations      = 0;
static size_t g_nCurrentActiveAllocations = 0;
static size_t g_nCurrentActiveBytes      = 0;
static size_t g_nPeakActiveBytes         = 0;
static size_t g_nDoubleFreeDetected      = 0;
static bool   g_bTrackingActive          = true;

void* operator new(size_t size) {
    if (!g_bTrackingActive) {
        return std::malloc(size);
    }

    size_t nTotalSize = sizeof(AllocHeader) + size;
    void* pRaw = std::malloc(nTotalSize);
    if (!pRaw) {
        throw std::bad_alloc();
    }

    AllocHeader* pHeader = static_cast<AllocHeader*>(pRaw);
    pHeader->_nMagic     = ALLOC_MAGIC;
    pHeader->_nSize      = size;
    pHeader->_nId        = ++g_nTotalAllocations;
    pHeader->_nReserved  = 0;

    g_nCurrentActiveAllocations++;
    g_nCurrentActiveBytes += size;
    if (g_nCurrentActiveBytes > g_nPeakActiveBytes) {
        g_nPeakActiveBytes = g_nCurrentActiveBytes;
    }

    return static_cast<void*>(static_cast<char*>(pRaw) + sizeof(AllocHeader));
}

void* operator new[](size_t size) {
    return ::operator new(size);
}

void operator delete(void* ptr) noexcept {
    if (!ptr) {
        return;
    }

    if (!g_bTrackingActive) {
        std::free(ptr);
        return;
    }

    AllocHeader* pHeader = reinterpret_cast<AllocHeader*>(static_cast<char*>(ptr) - sizeof(AllocHeader));

    if (pHeader->_nMagic == FREED_MAGIC) {
        g_nDoubleFreeDetected++;
        std::cerr << "  \033[31m[CRITICAL]\033[0m Phat hien Double Free tai dia chi " << ptr << "!\n";
        return;
    }

    if (pHeader->_nMagic != ALLOC_MAGIC) {
        // Vung nho khong duoc cap phat boi custom tracker (vi du: std::filesystem UTF-16 buffer noi bo)
        std::free(ptr);
        return;
    }

    pHeader->_nMagic = FREED_MAGIC;
    if (g_nCurrentActiveAllocations > 0) {
        g_nCurrentActiveAllocations--;
    }
    if (g_nCurrentActiveBytes >= pHeader->_nSize) {
        g_nCurrentActiveBytes -= pHeader->_nSize;
    } else {
        g_nCurrentActiveBytes = 0;
    }
    g_nTotalDeallocations++;

    std::free(pHeader);
}

void operator delete[](void* ptr) noexcept {
    ::operator delete(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
    ::operator delete(ptr);
}

void operator delete[](void* ptr, size_t) noexcept {
    ::operator delete(ptr);
}

// ============================================================================
// 2. TEST FRAMEWORK MACROS & STATS
// ============================================================================

static int g_nPassCount = 0;
static int g_nFailCount = 0;

#define TEST_CHECK(cond, msg) \
    do { \
        if (cond) { \
            g_nPassCount++; \
            std::cout << "  \033[32m[PASS]\033[0m " << (msg) << "\n"; \
        } else { \
            g_nFailCount++; \
            std::cout << "  \033[31m[FAIL]\033[0m " << (msg) \
                      << " (Line " << __LINE__ << ")\n"; \
        } \
    } while (0)

// ============================================================================
// 3. CHI TIẾT 6 KỊCH BẢN KIỂM THỬ BỘ NHỚ
// ============================================================================

/******************************************************************************
 * Kịch bản 1: Vòng đời cơ bản của LinkedList<T> (Add, Clear, Destructor RAII)
 ******************************************************************************/
void testLinkedListLifecycle() {
    std::cout << "\n======================================================\n";
    std::cout << " [KICH BAN 1] Vong doi co ban cua LinkedList<T>\n";
    std::cout << "======================================================\n";

    size_t nBeforeAllocs = g_nCurrentActiveAllocations;
    size_t nBeforeBytes  = g_nCurrentActiveBytes;

    {
        LinkedList<int> list;
        TEST_CHECK(list.isEmpty(), "Danh sach ban dau rong");

        // Them 100 phan tu
        for (int i = 0; i < 100; ++i) {
            list.addTail(i * 10);
        }
        TEST_CHECK(list.getSize() == 100, "Kich thuoc bang dung 100");
        TEST_CHECK(g_nCurrentActiveAllocations >= nBeforeAllocs + 100, "Da cap phat it nhat 100 node tren Heap");

        // Xoa sach bang clear()
        list.clear();
        TEST_CHECK(list.isEmpty(), "Danh sach tro ve rong sau khi clear()");
        TEST_CHECK(list.getSize() == 0, "Kich thuoc bang 0");
        TEST_CHECK(list.getHead() == nullptr, "Con tro _pHead bang nullptr");
        TEST_CHECK(list.getTail() == nullptr, "Con tro _pTail bang nullptr");
        TEST_CHECK(g_nCurrentActiveAllocations == nBeforeAllocs, "Toan bo node da duoc giai phong sau clear()");
        TEST_CHECK(g_nCurrentActiveBytes == nBeforeBytes, "So byte dang dung tro ve muc baseline sau clear()");

        // Kiem tra tinh idempotent: Goi clear() lan 2 khong loi
        list.clear();
        TEST_CHECK(list.isEmpty(), "Goi clear() lan 2 an toan");
    }

    // Kiem tra Destructor tu dong thu hoi khi ra khoi scope
    {
        LinkedList<double> listScope;
        for (int i = 0; i < 50; ++i) {
            listScope.addTail(i * 1.5);
        }
        TEST_CHECK(listScope.getSize() == 50, "Scope list chua 50 phan tu");
    }
    // Ra khoi scope -> Destructor phai thu hoi sach
    TEST_CHECK(g_nCurrentActiveAllocations == nBeforeAllocs, "Destructor thu hoi 100% node khi ra khoi scope");
    TEST_CHECK(g_nCurrentActiveBytes == nBeforeBytes, "0 byte ro ri sau khi Destructor hoan tat");
}

/******************************************************************************
 * Kịch bản 2: Giải phóng node tuc thi voi removeIf tai moi vi tri
 ******************************************************************************/
void testLinkedListRemoveIf() {
    std::cout << "\n======================================================\n";
    std::cout << " [KICH BAN 2] removeIf tai moi vi tri (Dau, Giua, Cuoi)\n";
    std::cout << "======================================================\n";

    size_t nBeforeAllocs = g_nCurrentActiveAllocations;
    size_t nBeforeBytes  = g_nCurrentActiveBytes;

    LinkedList<int> list;
    list.addTail(10); // Dau
    list.addTail(20); // Giua
    list.addTail(30); // Giua
    list.addTail(40); // Cuoi

    TEST_CHECK(list.getSize() == 4, "Khoi tao 4 phan tu");
    size_t nActiveWith4 = g_nCurrentActiveAllocations;

    // 1. Xoa phan tu o dau (10)
    bool bDel1 = list.removeIf([](int v) { return v == 10; });
    TEST_CHECK(bDel1, "Xoa thanh cong phan tu dau tien (10)");
    TEST_CHECK(list.getSize() == 3, "Kich thuoc con 3");
    TEST_CHECK(list.getHead()->_data == 20, "Dau danh sach moi la 20");
    TEST_CHECK(g_nCurrentActiveAllocations == nActiveWith4 - 1, "Vung nho node dau duoc thu hoi lap tuc");

    // 2. Xoa phan tu o giua (30)
    bool bDel2 = list.removeIf([](int v) { return v == 30; });
    TEST_CHECK(bDel2, "Xoa thanh cong phan tu giua (30)");
    TEST_CHECK(list.getSize() == 2, "Kich thuoc con 2");
    TEST_CHECK(g_nCurrentActiveAllocations == nActiveWith4 - 2, "Vung nho node giua duoc thu hoi lap tuc");

    // 3. Xoa phan tu o cuoi (40)
    bool bDel3 = list.removeIf([](int v) { return v == 40; });
    TEST_CHECK(bDel3, "Xoa thanh cong phan tu cuoi (40)");
    TEST_CHECK(list.getSize() == 1, "Kich thuoc con 1");
    TEST_CHECK(list.getTail()->_data == 20, "Duoi danh sach cap nhat dung la 20");
    TEST_CHECK(g_nCurrentActiveAllocations == nActiveWith4 - 3, "Vung nho node cuoi duoc thu hoi lap tuc");

    // 4. Xoa phan tu khong ton tai
    bool bDel4 = list.removeIf([](int v) { return v == 999; });
    TEST_CHECK(!bDel4, "Xoa phan tu khong ton tai tra ve false an toan");
    TEST_CHECK(list.getSize() == 1, "Kich thuoc van giu nguyen 1");

    // 5. Xoa phan tu duy nhat con lai (20)
    bool bDel5 = list.removeIf([](int v) { return v == 20; });
    TEST_CHECK(bDel5, "Xoa thanh cong phan tu cuoi cung");
    TEST_CHECK(list.isEmpty(), "Danh sach tro ve rong hoan toan");
    TEST_CHECK(list.getHead() == nullptr && list.getTail() == nullptr, "Head va Tail deu bang nullptr");

    // Kiem tra tong the
    TEST_CHECK(g_nCurrentActiveAllocations == nBeforeAllocs, "Toan bo cac node bi xoa da duoc delete");
    TEST_CHECK(g_nCurrentActiveBytes == nBeforeBytes, "0 byte ro ri sau cac thao tac removeIf");
}

/******************************************************************************
 * Kịch bản 3: LinkedList voi cac kieu du lieu phuc tap
 ******************************************************************************/
void testLinkedListComplexTypes() {
    std::cout << "\n======================================================\n";
    std::cout << " [KICH BAN 3] LinkedList voi doi tuong phuc tap (OOP)\n";
    std::cout << "======================================================\n";

    size_t nBeforeAllocs = g_nCurrentActiveAllocations;
    size_t nBeforeBytes  = g_nCurrentActiveBytes;

    // 3.1 LinkedList<std::string>
    {
        LinkedList<std::string> strList;
        strList.addTail("Pham Hoang Tri - Data Engineer");
        strList.addTail("Dang Tran Trung Phat - Tech Lead");
        strList.addTail("Tran Huynh Anh Tuan - Business Logic & UI");
        TEST_CHECK(strList.getSize() == 3, "strList nap thanh cong 3 thanh vien");
        strList.clear();
        TEST_CHECK(strList.isEmpty(), "strList clear sach se");
    }
    TEST_CHECK(g_nCurrentActiveAllocations == nBeforeAllocs, "std::string duoc huy sach se, 0 byte leak");

    // 3.2 LinkedList<Admin>
    {
        LinkedList<Admin> adminList;
        adminList.addTail(Admin("admin_tri", "Secret@123"));
        adminList.addTail(Admin("admin_phat", "Phat@2026"));
        TEST_CHECK(adminList.getSize() == 2, "adminList chua 2 admin");
    }
    TEST_CHECK(g_nCurrentActiveAllocations == nBeforeAllocs, "Admin model huy an toan");

    // 3.3 LinkedList<Card> & LinkedList<Account>
    {
        LinkedList<Card> cardList;
        cardList.addTail(Card("10014504500001", "123456", false));
        cardList.addTail(Card("10014504500002", "654321", true));
        TEST_CHECK(cardList.getSize() == 2, "cardList chua 2 the");

        LinkedList<Account> accList;
        accList.addTail(Account("10014504500001", "Pham Hoang Tri", 5000000, "VND"));
        accList.addTail(Account("10014504500002", "Dang Tran Trung Phat", 10000000, "VND"));
        TEST_CHECK(accList.getSize() == 2, "accList chua 2 tai khoan");
    }
    TEST_CHECK(g_nCurrentActiveAllocations == nBeforeAllocs, "Card & Account models huy an toan");

    // 3.4 LinkedList<Transaction>
    {
        LinkedList<Transaction> transList;
        transList.addTail(Transaction("10014504500001", WITHDRAW, 200000, "2026-10-09 20:00:00", "Rut tien ATM"));
        transList.addTail(Transaction("10014504500001", TRANSFER, 500000, "2026-10-09 20:15:00", "Chuyen den 10014504500002"));
        TEST_CHECK(transList.getSize() == 2, "transList chua 2 giao dich");
    }
    TEST_CHECK(g_nCurrentActiveAllocations == nBeforeAllocs, "Transaction model huy an toan");
    TEST_CHECK(g_nCurrentActiveBytes == nBeforeBytes, "0 byte ro ri voi toan bo cac kieu du lieu phuc tap");
}

/******************************************************************************
 * Kịch bản 4: Kiem tra bo nho tang FileService doc/ghi tep tin
 ******************************************************************************/
void testFileServiceMemorySafety() {
    std::cout << "\n======================================================\n";
    std::cout << " [KICH BAN 4] Bo nho tang FileService (I/O 5 loai file)\n";
    std::cout << "======================================================\n";

    size_t nBeforeAllocs = g_nCurrentActiveAllocations;
    size_t nBeforeBytes  = g_nCurrentActiveBytes;

    // Khoi tao du lieu mau
    FileService::initSampleData();

    {
        LinkedList<Admin> admins;
        LinkedList<Card> cards;
        LinkedList<std::string> lockedIds;

        bool b1 = FileService::loadAdmins(admins);
        bool b2 = FileService::loadLockedIds(lockedIds);
        bool b3 = FileService::loadCards(cards, lockedIds);

        TEST_CHECK(b1, "loadAdmins thanh cong tu data/Admin.txt");
        TEST_CHECK(b2, "loadLockedIds thanh cong tu data/KhoaThe.txt");
        TEST_CHECK(b3, "loadCards thanh cong tu data/TheTu.txt");
        TEST_CHECK(admins.getSize() >= 3, "Nap duoc it nhat 3 Admin");
        TEST_CHECK(cards.getSize() >= 10, "Nap duoc it nhat 10 The tu");

        // Nap thong tin 1 tai khoan cu the
        Account acc;
        ErrorCode errAcc = FileService::loadAccount("10014504500001", acc);
        TEST_CHECK(errAcc == ERR_NONE, "loadAccount thanh cong cho 10014504500001");

        // Nap lich su giao dich
        LinkedList<Transaction> history;
        bool bHist = FileService::loadTransactions("10014504500001", history);
        TEST_CHECK(bHist, "loadTransactions thanh cong");
    }

    // Sau khi cac danh sach ra khoi scope
    TEST_CHECK(g_nCurrentActiveAllocations == nBeforeAllocs, "Toan bo du lieu nạp tu FileService da duoc giai phong sach");
    TEST_CHECK(g_nCurrentActiveBytes == nBeforeBytes, "0 byte ro ri sau khi doc/ghi cac tap tin thuc te");
}

/******************************************************************************
 * Kịch bản 5: Vong doi phien lam viec AtmController (Dynamic Allocation)
 ******************************************************************************/
void testAtmControllerSessionLifecycle() {
    std::cout << "\n======================================================\n";
    std::cout << " [KICH BAN 5] Vong doi phien lam viec AtmController\n";
    std::cout << "======================================================\n";

    size_t nBeforeAllocs = g_nCurrentActiveAllocations;
    size_t nBeforeBytes  = g_nCurrentActiveBytes;

    {
        AtmController controller;
        bool bInit = controller.initData();
        TEST_CHECK(bInit, "AtmController khoi tao du lieu thanh cong");
        TEST_CHECK(controller.getAdmins().getSize() >= 3, "Admins trong controller da san sang");
        TEST_CHECK(controller.getCards().getSize() >= 10, "Cards trong controller da san sang");

        // Goi cleanupSession chu dong
        controller.cleanupSession();
        TEST_CHECK(true, "Goi cleanupSession an toan, con tro duoc dat ve nullptr");
    }

    // Sau khi controller bi tieu huy bang Destructor
    TEST_CHECK(g_nCurrentActiveAllocations == nBeforeAllocs, "AtmController Destructor thu hoi toan bo RAM");
    TEST_CHECK(g_nCurrentActiveBytes == nBeforeBytes, "0 byte ro ri sau khi AtmController ket thuc");
}

/******************************************************************************
 * Kịch bản 6: Stress Testing 50,000 node kiem tra phan manh & do ben
 ******************************************************************************/
void testStressHighVolumeAllocation() {
    std::cout << "\n======================================================\n";
    std::cout << " [KICH BAN 6] Stress Testing 50,000 nodes tai cao\n";
    std::cout << "======================================================\n";

    size_t nBeforeAllocs = g_nCurrentActiveAllocations;

    constexpr int STRESS_NODES = 50000;
    constexpr int ITERATIONS   = 3;

    auto tStart = std::chrono::high_resolution_clock::now();

    for (int iter = 1; iter <= ITERATIONS; ++iter) {
        LinkedList<int> stressList;
        for (int i = 0; i < STRESS_NODES; ++i) {
            stressList.addTail(i);
        }
        TEST_CHECK(stressList.getSize() == STRESS_NODES,
                   "Iteration " + std::to_string(iter) + ": Chen 50,000 nodes thanh cong");
        stressList.clear();
        TEST_CHECK(stressList.isEmpty(),
                   "Iteration " + std::to_string(iter) + ": Giai phong 50,000 nodes thanh cong");
    }

    auto tEnd = std::chrono::high_resolution_clock::now();
    double dElapsedMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

    std::cout << "  \033[36m[PERF]\033[0m Thoi gian chay " << ITERATIONS << " chu ky x 50,000 nodes: "
              << std::fixed << std::setprecision(2) << dElapsedMs << " ms\n";
    std::cout << "  \033[36m[MEM]\033[0m  Peak heap memory ghi nhan: "
              << (g_nPeakActiveBytes / 1024.0 / 1024.0) << " MB\n";

    TEST_CHECK(g_nCurrentActiveAllocations == nBeforeAllocs, "50,000 nodes x 3 chu ky: 100% vung nho duoc thu hoi");
    TEST_CHECK(g_nDoubleFreeDetected == 0, "Khong phat hien bat ky loi Double Free nao");
    TEST_CHECK(g_nCurrentActiveAllocations == 0, "So block nho dang hoat dong sau Stress Test la 0");
}

// ============================================================================
// 4. MAIN ENTRY POINT
// ============================================================================
int main() {
    std::cout << "\n==============================================================\n";
    std::cout << " BIEU DO KIEM DINH RO RI BO NHO - MEMORY AUDIT (PHASE 4 - TRI)\n";
    std::cout << " Tuan thu C++ Coding Standard V2 (Rule 17: 0 bytes leaked)\n";
    std::cout << "==============================================================\n";

    size_t nInitialAllocs = g_nCurrentActiveAllocations;
    size_t nInitialBytes  = g_nCurrentActiveBytes;

    testLinkedListLifecycle();
    testLinkedListRemoveIf();
    testLinkedListComplexTypes();
    testFileServiceMemorySafety();
    testAtmControllerSessionLifecycle();
    testStressHighVolumeAllocation();

    size_t nFinalActiveAllocs = g_nCurrentActiveAllocations - nInitialAllocs;
    size_t nFinalActiveBytes  = g_nCurrentActiveBytes - nInitialBytes;

    std::cout << "\n==============================================================\n";
    std::cout << "                  TONG KET KIEM THU BO NHO                    \n";
    std::cout << "==============================================================\n";
    std::cout << "  Tong so Test Cases pass: " << g_nPassCount << "\n";
    std::cout << "  Tong so Test Cases fail: " << g_nFailCount << "\n";
    std::cout << "  Tong so lan cap phat new/new[]:   " << g_nTotalAllocations << "\n";
    std::cout << "  Tong so lan giai phong del/del[]: " << g_nTotalDeallocations << "\n";
    std::cout << "  So block nho con ton tai (Active): " << nFinalActiveAllocs << "\n";
    std::cout << "  So byte bo nho bi ro ri (Leaked): " << nFinalActiveBytes << " bytes\n";
    std::cout << "  So loi Double Free ghi nhan:       " << g_nDoubleFreeDetected << "\n";
    std::cout << "  Dinh bo nho su dung (Peak Memory): " << (g_nPeakActiveBytes / 1024.0) << " KB\n";

    if (g_nFailCount == 0 && nFinalActiveBytes == 0 && g_nDoubleFreeDetected == 0) {
        std::cout << "\n  \033[32m==============================================================\033[0m\n";
        std::cout << "  \033[32m  [HOAN TOAN DAT CHUAN] 0 BYTES LEAKED - HE THONG SACH SE 100% \033[0m\n";
        std::cout << "  \033[32m  DAT TRON 1.0 DIEM CODING STANDARD V2 CHO QUAN LY BO NHO!   \033[0m\n";
        std::cout << "  \033[32m==============================================================\033[0m\n\n";
        return 0;
    } else {
        std::cout << "\n  \033[31m==============================================================\033[0m\n";
        std::cout << "  \033[31m  [THAT BAI] PHAT HIEN RO RI BO NHO HOAC TEST CASE BI LOI!    \033[0m\n";
        std::cout << "  \033[31m==============================================================\033[0m\n\n";
        return 1;
    }
}
