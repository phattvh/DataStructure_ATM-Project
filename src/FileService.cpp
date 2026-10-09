#include "FileService.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <cstdio>

#ifdef _WIN32
#include <process.h>
#define GET_CURRENT_PID() _getpid()
#else
#include <unistd.h>
#define GET_CURRENT_PID() getpid()
#endif

namespace fs = std::filesystem;

/**********************************************************
 * Ham noi bo: Dam bao thu muc DATA_DIR ton tai
 **********************************************************/
static void ensureDataDirExists() {
    try {
        if (!fs::exists(DATA_DIR)) {
            fs::create_directories(DATA_DIR);
        }
    } catch (...) {
        // Bo qua neu thu muc da ton tai hoac khong tao duoc
    }
}

/**********************************************************
 * Ham noi bo: Trim khoang trang dau va cuoi chuoi
 **********************************************************/
static std::string trimString(const std::string& str) {
    size_t iStart = str.find_first_not_of(" \t\r\n");
    if (iStart == std::string::npos) return "";
    size_t iEnd = str.find_last_not_of(" \t\r\n");
    return str.substr(iStart, iEnd - iStart + 1);
}

/**********************************************************
 * Ham noi bo: Ghi file nguyen tu (Atomic Write) qua file tam
 * chong mat mat hoac cat trang du lieu ve 0 byte khi sap nguon,
 * ket hop tien to PID tranh va cham ghi file giua nhieu tien trinh
 **********************************************************/
static bool atomicWriteFile(const std::string& strPath, const std::string& strContent) {
    ensureDataDirExists();
    std::string strSuffix = "." + std::to_string(GET_CURRENT_PID());
    std::string strTempPath = strPath + strSuffix + ".tmp";
    std::ofstream fout(strTempPath, std::ios::trunc);
    if (!fout.is_open()) {
        return false;
    }

    fout << strContent;
    fout.flush();
    if (fout.fail()) {
        fout.close();
        std::error_code ec;
        fs::remove(strTempPath, ec);
        return false;
    }
    fout.close();

    std::error_code ec;
    fs::rename(strTempPath, strPath, ec);
    if (ec) {
        // Fallback an toan co sao luu phong ngua he dieu hanh khong cho rename de len file da ton tai
        std::string strBakPath = strPath + strSuffix + ".bak";
        std::error_code ecBak;
        if (fs::exists(strPath)) {
            fs::rename(strPath, strBakPath, ecBak);
        }

        fs::rename(strTempPath, strPath, ec);
        if (ec) {
            // Neu rename lan 2 van that bai, khoi phuc file goc tu backup
            if (!ecBak && fs::exists(strBakPath)) {
                std::error_code ecRestore;
                fs::rename(strBakPath, strPath, ecRestore);
            }
            std::error_code ecCleanTmp;
            fs::remove(strTempPath, ecCleanTmp);
            return false;
        }

        // Rename thanh cong -> don dep file backup
        if (!ecBak && fs::exists(strBakPath)) {
            std::error_code ecDelBak;
            fs::remove(strBakPath, ecDelBak);
        }
    }
    return true;
}

bool FileService::loadAdmins(LinkedList<Admin>& listAdmins) {
    ensureDataDirExists();
    std::string strPath = DATA_DIR + "Admin.txt";
    std::ifstream fin(strPath);
    if (!fin.is_open()) {
        return false;
    }

    std::string strLine;
    while (std::getline(fin, strLine)) {
        strLine = trimString(strLine);
        if (strLine.empty()) continue;

        std::istringstream iss(strLine);
        std::string strUser, strPass;
        if (iss >> strUser >> strPass) {
            listAdmins.addTail(Admin(strUser, strPass));
        }
    }

    fin.close();
    return true;
}

bool FileService::loadLockedIds(LinkedList<std::string>& listLockedIds) {
    ensureDataDirExists();
    std::string strPath = DATA_DIR + "KhoaThe.txt";
    std::ifstream fin(strPath);
    if (!fin.is_open()) {
        return false;
    }

    std::string strLine;
    while (std::getline(fin, strLine)) {
        strLine = trimString(strLine);
        if (!strLine.empty()) {
            listLockedIds.addTail(strLine);
        }
    }

    fin.close();
    return true;
}

bool FileService::loadCards(LinkedList<Card>& listCards,
                            const LinkedList<std::string>& listLockedIds) {
    ensureDataDirExists();
    std::string strPath = DATA_DIR + "TheTu.txt";
    std::ifstream fin(strPath);
    if (!fin.is_open()) {
        return false;
    }

    std::string strLine;
    while (std::getline(fin, strLine)) {
        strLine = trimString(strLine);
        if (strLine.empty()) continue;

        std::istringstream iss(strLine);
        std::string strId, strPin;
        if (iss >> strId >> strPin) {
            // Phong chong the trung lap (Deduplication): bo qua neu da ton tai
            auto pDup = listCards.findIf([&strId](const Card& c) {
                return c.getId() == strId;
            });
            if (pDup != nullptr) {
                continue;
            }

            bool bIsLocked = false;
            // Kiem tra xem the co nam trong danh sach the khoa khong
            const std::string* pLocked = listLockedIds.findIf([&strId](const std::string& lockedId) {
                return lockedId == strId;
            });
            if (pLocked != nullptr) {
                bIsLocked = true;
            }

            listCards.addTail(Card(strId, strPin, bIsLocked));
        }
    }

    fin.close();
    return true;
}

bool FileService::saveCards(const LinkedList<Card>& listCards) {
    std::string strPath = DATA_DIR + "TheTu.txt";
    std::ostringstream oss;
    Node<Card>* pCur = listCards.getHead();
    while (pCur != nullptr) {
        oss << pCur->_data.getId() << " " << pCur->_data.getPin() << "\n";
        pCur = pCur->_pNext;
    }
    return atomicWriteFile(strPath, oss.str());
}

bool FileService::updateCardPin(const std::string& strId, const std::string& strNewPin) {
    LinkedList<std::string> listLocked;
    loadLockedIds(listLocked);
    LinkedList<Card> listCards;
    if (!loadCards(listCards, listLocked)) {
        return false;
    }

    auto pCard = listCards.findIf([&strId](const Card& c) {
        return c.getId() == strId;
    });
    if (pCard == nullptr) {
        return false;
    }

    if (!pCard->changePin(strNewPin)) {
        return false;
    }

    return saveCards(listCards);
}

bool FileService::saveLockedIds(const LinkedList<std::string>& listLockedIds) {
    std::string strPath = DATA_DIR + "KhoaThe.txt";
    std::ostringstream oss;
    Node<std::string>* pCur = listLockedIds.getHead();
    while (pCur != nullptr) {
        oss << pCur->_data << "\n";
        pCur = pCur->_pNext;
    }
    return atomicWriteFile(strPath, oss.str());
}

bool FileService::appendLockedCard(const std::string& strId) {
    ensureDataDirExists();
    std::string strPath = DATA_DIR + "KhoaThe.txt";
    std::ofstream fout(strPath, std::ios::app);
    if (!fout.is_open()) {
        return false;
    }

    fout << strId << "\n";
    fout.close();
    return true;
}

ErrorCode FileService::loadAccount(const std::string& strId, Account& acc) {
    ensureDataDirExists();
    std::string strPath = DATA_DIR + strId + ".txt";
    std::ifstream fin(strPath);
    if (!fin.is_open()) {
        return ERR_FILE_NOT_FOUND;
    }

    std::string strFileId;
    std::string strName;
    std::string strBalanceLine;
    std::string strCurrency;

    // Doc 4 dong tuan thu Bay ky thuat doc file co khoang trang
    if (!std::getline(fin, strFileId)) { fin.close(); return ERR_FILE_NOT_FOUND; }
    if (!std::getline(fin, strName)) { fin.close(); return ERR_FILE_NOT_FOUND; }
    if (!std::getline(fin, strBalanceLine)) { fin.close(); return ERR_FILE_NOT_FOUND; }
    if (!std::getline(fin, strCurrency)) {
        strCurrency = "VND"; // Mac dinh neu thieu dong 4
    }

    strFileId = trimString(strFileId);
    strName = trimString(strName);
    strBalanceLine = trimString(strBalanceLine);
    strCurrency = trimString(strCurrency);

    // Kiem tra tinh toan ven: ID doc tu noi dung file phai khop voi ID truy van
    if (strFileId != strId) {
        fin.close();
        return ERR_INVALID_FORMAT;
    }

    long lBalance = 0;
    try {
        size_t idx = 0;
        lBalance = std::stol(strBalanceLine, &idx);
        // Neu co ky tu la o cuoi dong so du, coi nhu file bi hong dinh dang
        if (idx != strBalanceLine.length()) {
            fin.close();
            return ERR_INVALID_FORMAT;
        }
    } catch (...) {
        fin.close();
        return ERR_INVALID_FORMAT;
    }

    if (lBalance < 0) {
        fin.close();
        return ERR_INVALID_FORMAT;
    }

    acc = Account(strFileId, strName, lBalance, strCurrency);
    fin.close();
    return ERR_NONE;
}

bool FileService::saveAccount(const Account& acc) {
    std::string strPath = DATA_DIR + acc.getId() + ".txt";
    std::ostringstream oss;
    oss << acc.getId() << "\n"
        << acc.getName() << "\n"
        << acc.getBalance() << "\n"
        << acc.getCurrency() << "\n";
    return atomicWriteFile(strPath, oss.str());
}

bool FileService::deleteAccountFile(const std::string& strId) {
    std::string strPath = DATA_DIR + strId + ".txt";
    return (std::remove(strPath.c_str()) == 0);
}

bool FileService::createAccountFiles(const std::string& strId,
                                    const std::string& strName,
                                    long lInitialBalance,
                                    const std::string& strCurrency) {
    ensureDataDirExists();

    // 1. Tao file [ID].txt
    Account newAcc(strId, strName, lInitialBalance, strCurrency);
    if (!saveAccount(newAcc)) {
        return false;
    }

    // 2. Tao file LichSu[ID].txt: Neu file lich su da ton tai tu truoc (cua chu the cu tung bi xoa),
    // tien hanh luu tru (archive) de tranh ro ri thong tin cho chu the moi
    std::string strHistoryPath = DATA_DIR + "LichSu" + strId + ".txt";
    if (fs::exists(strHistoryPath) && fs::file_size(strHistoryPath) > 0) {
        std::string strArchive = DATA_DIR + "Archive_LichSu" + strId + "_" + std::to_string(std::time(nullptr)) + ".bak";
        std::error_code ec;
        fs::rename(strHistoryPath, strArchive, ec);
    }

    // Khoi tao file LichSu moi trang tinh
    std::ofstream foutHist(strHistoryPath, std::ios::trunc);
    if (!foutHist.is_open()) {
        return false;
    }
    foutHist.close();

    return true;
}

bool FileService::appendTransaction(const std::string& strId, const Transaction& trans) {
    ensureDataDirExists();
    std::string strPath = DATA_DIR + "LichSu" + strId + ".txt";
    std::ofstream fout(strPath, std::ios::app);
    if (!fout.is_open()) {
        return false;
    }

    fout << trans.formatForFile() << "\n";
    fout.close();
    return true;
}

bool FileService::loadTransactions(const std::string& strId, LinkedList<Transaction>& listTrans) {
    ensureDataDirExists();
    std::string strPath = DATA_DIR + "LichSu" + strId + ".txt";
    std::ifstream fin(strPath);
    if (!fin.is_open()) {
        return false;
    }

    std::string strLine;
    while (std::getline(fin, strLine)) {
        strLine = trimString(strLine);
        if (strLine.empty()) continue;

        Transaction trans = Transaction::parseFromFileLine(strId, strLine);
        listTrans.addTail(trans);
    }

    fin.close();
    return true;
}

void FileService::initSampleData() {
    ensureDataDirExists();

    // 1. Khoi tao Admin.txt neu chua co
    std::string strAdminPath = DATA_DIR + "Admin.txt";
    if (!fs::exists(strAdminPath) || fs::file_size(strAdminPath) == 0) {
        std::ofstream fout(strAdminPath);
        if (fout.is_open()) {
            fout << "admin1 123456\n";
            fout << "admin2 123456\n";
            fout << "superadmin 888888\n";
            fout.close();
        }
    }

    // 2. Khoi tao KhoaThe.txt neu chua co
    std::string strLockedPath = DATA_DIR + "KhoaThe.txt";
    if (!fs::exists(strLockedPath)) {
        std::ofstream fout(strLockedPath);
        if (fout.is_open()) {
            fout.close();
        }
    }

    // 3. Khoi tao TheTu.txt va cac file [ID].txt neu he thong chua tung duoc khoi tao
    // Su dung marker .system_initialized de tranh phuc sinh de du lieu khi TheTu.txt bi lam rong boi Admin
    std::string strMarkerPath = DATA_DIR + ".system_initialized";
    std::string strTheTuPath = DATA_DIR + "TheTu.txt";
    if (!fs::exists(strMarkerPath)) {
        if (!fs::exists(strTheTuPath) || fs::file_size(strTheTuPath) == 0) {
            struct SampleCard {
                const char* szId;
                const char* szPin;
                const char* szName;
                long lBalance;
            };

            SampleCard sampleCards[] = {
                {"10014504500001", "123456", "Nguyen Trung Kien", 5000000},
                {"10014504500002", "123456", "Tran Thi Hoa",     10000000},
                {"10014504500003", "654321", "Le Van Cuong",      2500000},
                {"10014504500004", "123456", "Pham Minh Duc",      500000},
                {"10014504500005", "888888", "Hoang Quoc Bao",   12000000},
                {"10014504500006", "123456", "Vo Thi Mai",         800000},
                {"10014504500007", "123456", "Dang Tuan Anh",     3000000},
                {"10014504500008", "123456", "Bui Thi Lan",       1500000},
                {"10014504500009", "123456", "Doan Ngoc Hai",     7200000},
                {"10014504500010", "123456", "Truong Gia Binh",  20000000}
            };

            std::ofstream foutTheTu(strTheTuPath);
            if (foutTheTu.is_open()) {
                for (const auto& card : sampleCards) {
                    foutTheTu << card.szId << " " << card.szPin << "\n";
                    // Tao file [ID].txt va LichSu[ID].txt
                    createAccountFiles(card.szId, card.szName, card.lBalance, "VND");
                }
                foutTheTu.close();
            }
        }

        // Tao file marker ghi nhan khoi tao lan dau thanh cong
        std::ofstream foutMarker(strMarkerPath);
        if (foutMarker.is_open()) {
            foutMarker << "INITIALIZED\n";
            foutMarker.close();
        }
    }
}

/**********************************************************
 * Ham noi bo: Khử trùng chuỗi ghi nhật ký (chống CWE-117 Log Injection)
 **********************************************************/
static std::string sanitizeLogField(const std::string& strInput) {
    std::string strClean = strInput;
    for (char& c : strClean) {
        if (c == '\r' || c == '\n' || c == '|') {
            c = ' ';
        }
    }
    return strClean;
}

bool FileService::appendAdminLog(const std::string& strAction, const std::string& strDetail) {
    ensureDataDirExists();
    std::string strPath = DATA_DIR + "AdminLog.txt";
    std::ofstream outFile(strPath, std::ios::app);
    if (!outFile.is_open()) {
        return false;
    }
    outFile << getNowTimestamp() << "|" << sanitizeLogField(strAction) << "|" << sanitizeLogField(strDetail) << "\n";
    outFile.close();
    return true;
}
