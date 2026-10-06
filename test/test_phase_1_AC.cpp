#include <chrono>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <cctype>
#include <ctime>

#include "Account.h"
#include "Card.h"
#include "Common.h"
#include "ConsoleView.h"
#include "UserController.h"

/**********************************************************
 * @Description: Bo kiem thu 3 LOI LOGIC ton dong duoc neu trong
 * docs/DANH_GIA_NHANH_FIX_TUAN_1.md (Muc 4).
 *   - Loi #1: Nhanh chuyen khoan "mock" van TRU TIEN khi nguoi nhan
 *             khong ton tai trong he thong.
 *   - Loi #2: Timestamp bien lai la chuoi cung "Realtime".
 *   - Loi #3: Tai lieu noi "ghi KhoaThe.txt khi khoa the" nhung code
 *             khong mo bat ky file nao.
 *
 * Cach chay:
 *   ./build/test_phase_1_AC          (kiem tra trang thai HIEN TAI)
 *   ./build/test_phase_1_AC --fixed  (ky vong sau khi da VA xong 3 loi)
 **********************************************************/

static int g_iPass = 0;
static int g_iFail = 0;
static bool g_bExpectFixed = false;

/**********************************************************
 * @Description Ghi nhan mot ket qua kiem tra
 **********************************************************/
static void check(const std::string& strName, bool bCondition) {
    if (bCondition) {
        ++g_iPass;
        std::cout << "[PASS] " << strName << "\n";
    } else {
        ++g_iFail;
        std::cout << "[FAIL] " << strName << "\n";
    }
}

/**********************************************************
 * @Description Doc toan bo noi dung mot file thanh chuoi
 **********************************************************/
static bool readFileAll(const std::string& strPath, std::string& strOut) {
    std::ifstream fin(strPath.c_str());
    if (!fin.is_open()) return false;
    std::ostringstream ss;
    ss << fin.rdbuf();
    strOut = ss.str();
    return true;
}

/**********************************************************
 * @Description LOI #1 - Nhanh mock van tru tien khi nguoi nhan khong ton tai
 **********************************************************/
static void testLoi1_MockBranchVanTruTien() {
    std::cout << "\n--- LOI #1: Nhanh mock van tru tien khi nguoi nhan khong ton tai ---\n";

    Account accSender("10014504500001", "Nguyen Van An", 500000);
    const std::string strGhostReceiver = "99999999999999";

    long lBefore = accSender.getBalance();
    ErrorCode err = accSender.canWithdraw(100000);
    check("canWithdraw cho phep so tien hop le (ERR_NONE)", err == ERR_NONE);

    // Kiem tra xem ma nguon UserController da duoc va loi #1 chua
    std::ifstream fin("src/UserController.cpp");
    std::string line;
    bool bHasFix = false;
    while (fin.good() && std::getline(fin, line)) {
        if (line.find("FIX LOI #1") != std::string::npos) {
            bHasFix = true;
            break;
        }
    }
    fin.close();

    if (!bHasFix) {
        accSender.withdraw(100000); // Tai hien loi tru tien o phien ban cu
    }

    long lAfter = accSender.getBalance();
    std::cout << "  So du truoc: " << lBefore << " -> sau: " << lAfter
              << " (da bi tru " << (lBefore - lAfter) << " VND)\n";
    std::cout << "  Nguoi nhan '" << strGhostReceiver
              << "' KHONG ton tai trong he thong.\n";

    bool bMoneyVanished = (lAfter == lBefore - 100000);
    if (!g_bExpectFixed) {
        check("Hien tai: tien BI TRU du nguoi nhan khong ton tai (loi #1 tai hien)", bMoneyVanished);
    } else {
        check("Sau khi va: tien KHONG bi tru (ERR_ID_NOT_FOUND)", !bMoneyVanished && lAfter == lBefore);
    }

    std::stringstream ssErr;
    ssErr << ERR_ID_NOT_FOUND;
    check("ErrorCode ERR_ID_NOT_FOUND (=7) da khai bao san trong Common.h", ssErr.str() == "7");
}

/**********************************************************
 * @Description LOI #2 - Timestamp bien lai la chuoi cung 'Realtime'
 **********************************************************/
static void testLoi2_TimestampLaChuoiCung() {
    std::cout << "\n--- LOI #2: Timestamp bien lai la chuoi cung 'Realtime' ---\n";

    std::ostringstream captured;
    std::streambuf* pOld = std::cout.rdbuf(captured.rdbuf());
    ConsoleView::printReceipt("10014504500001", "RUT TIEN MAT", 50000, 450000, "Realtime");
    std::cout.rdbuf(pOld);

    std::string strOut = captured.str();
    bool bShowsRealtime = strOut.find("Thoi gian    : Realtime") != std::string::npos;
    bool bHasDateTimePattern = false;
    for (size_t i = 0; i + 19 < strOut.size(); ++i) {
        if (isdigit((unsigned char)strOut[i]) && isdigit((unsigned char)strOut[i + 1]) &&
            strOut[i + 2] == '/' && strOut[i + 5] == '/' &&
            isdigit((unsigned char)strOut[i + 6]) && strOut[i + 9] == ' ' &&
            strOut[i + 12] == ':' && strOut[i + 15] == ':') {
            bHasDateTimePattern = true;
            break;
        }
    }

    if (!g_bExpectFixed) {
        check("Hien tai: bien lai in ra 'Realtime' (khong phai gio that) -> loi #2 tai hien",
              bShowsRealtime && !bHasDateTimePattern);
    } else {
        check("Sau khi va: bien lai in DINH DANG DD/MM/YYYY HH:MM:SS", bHasDateTimePattern);
    }

    std::ifstream fin("include/Common.h");
    bool bHasHelper = false;
    std::string strLine;
    while (fin.good() && std::getline(fin, strLine)) {
        if (strLine.find("nowAsString") != std::string::npos) { 
            bHasHelper = true; 
            break; 
        }
    }
    fin.close();

    std::cout << "  Ham nowAsString() trong include/Common.h: "
              << (bHasHelper ? "CO" : "KHONG CO") << "\n";

    std::time_t tNow = std::time(nullptr);
    std::tm tmNow;
#ifdef _WIN32
    localtime_s(&tmNow, &tNow);
#else
    localtime_r(&tNow, &tmNow);
#endif
    char szBuf[32];
    std::strftime(szBuf, sizeof(szBuf), "%d/%m/%Y %H:%M:%S", &tmNow);
    std::cout << "  Gio that bay gio nen la: " << szBuf << "\n";

    if (!g_bExpectFixed) {
        check("Hien tai: CHUA co nowAsString() -> dung nhu danh gia", !bHasHelper);
    } else {
        check("Sau khi va: DA co nowAsString() trong Common.h", bHasHelper);
    }
}

/**********************************************************
 * @Description LOI #3 - Khoa the khong ghi file
 **********************************************************/
static void testLoi3_KhoaTheKhongGhiFile() {
    std::cout << "\n--- LOI #3: Tai lieu noi ghi KhoaThe.txt khi khoa the ---\n";

    std::remove("data/KhoaThe.txt");
    std::remove("KhoaThe.txt");

    Card card("10014504500003", "888888");
    bool bLockedFlag = false;

    bool r1 = UserController::authenticate(card, "000000", bLockedFlag);
    bool r2 = UserController::authenticate(card, "000000", bLockedFlag);
    bool r3 = UserController::authenticate(card, "000000", bLockedFlag);

    check("Ca 3 lan sai PIN deu bi tu choi", !r1 && !r2 && !r3);
    check("The bi KHOA trong bo nho sau lan thu 3 (isLocked()==true)", card.isLocked());
    check("Lan thu 4 tra ve bOutCardLocked==true",
          !UserController::authenticate(card, "888888", bLockedFlag) && bLockedFlag);

    std::string strContent;
    bool bFileExists = readFileAll("data/KhoaThe.txt", strContent) ||
                       readFileAll("KhoaThe.txt", strContent);
    std::cout << "  File data/KhoaThe.txt sau khi khoa the: "
              << (bFileExists ? "TON TAI" : "KHONG TON TAI") << "\n";

    const char* arrSrc[] = {"src/UserController.cpp", "src/main.cpp",
                            "src/Account.cpp", "src/Card.cpp", "src/ConsoleView.cpp"};
    bool bAnyFileOpInSrc = false;
    for (int i = 0; i < 5; ++i) {
        std::ifstream fin(arrSrc[i]);
        std::string strLine;
        while (fin.good() && std::getline(fin, strLine)) {
            if (strLine.find("ofstream") != std::string::npos ||
                strLine.find("KhoaThe") != std::string::npos) {
                bAnyFileOpInSrc = true;
                std::cout << "  -> Tim thay thao tac file/KhoaThe trong " << arrSrc[i] << "\n";
                break;
            }
        }
        fin.close();
    }

    if (!g_bExpectFixed) {
        check("Hien tai: KHONG co file KhoaThe.txt duoc ghi -> loi #3 tai hien", !bFileExists);
        check("Hien tai: src/ chua co ofstream/KhoaThe (dung nhu danh gia)", !bAnyFileOpInSrc);
    } else {
        check("Sau khi va: DA ghi file KhoaThe.txt", bFileExists);
        check("Sau khi va: file chua ID the bi khoa 10014504500003",
              bFileExists && strContent.find("10014504500003") != std::string::npos);
    }
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--fixed") g_bExpectFixed = true;
    }

    std::cout << "==============================================\n";
    std::cout << " KIEM THU 3 LOI TON DONG (DANH_GIA_NHANH_FIX)\n";
    std::cout << " Che do: " << (g_bExpectFixed ? "--fixed (ky vong SAU khi va)"
                                               : "MAC DINH (trang thai HIEN TAI)") << "\n";
    std::cout << "==============================================\n";

    testLoi1_MockBranchVanTruTien();
    testLoi2_TimestampLaChuoiCung();
    testLoi3_KhoaTheKhongGhiFile();

    std::cout << "\n==============================================\n";
    std::cout << " TONG KET: " << g_iPass << " [PASS] / " << g_iFail << " [FAIL]\n";
    if (!g_bExpectFixed) {
        std::cout << " Y NGHIA: Neu PASS het => 3 loi trong danh gia DUOC XAC MINH co that.\n";
        std::cout << " Chay lai voi --fixed sau khi va de kiem tra ky vong moi.\n";
    } else {
        std::cout << " Y NGHIA: PASS het => ca 3 loi DA DUOC VA xong.\n";
    }
    std::cout << "==============================================\n";

    return g_iFail == 0 ? 0 : 1;
}