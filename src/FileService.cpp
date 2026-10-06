#include "FileService.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <cstdio>

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
    ensureDataDirExists();
    std::string strPath = DATA_DIR + "TheTu.txt";
    std::ofstream fout(strPath, std::ios::trunc);
    if (!fout.is_open()) {
        return false;
    }

    Node<Card>* pCur = listCards.getHead();
    while (pCur != nullptr) {
        fout << pCur->_data.getId() << " " << pCur->_data.getPin() << "\n";
        pCur = pCur->_pNext;
    }

    fout.close();
    return true;
}

bool FileService::saveLockedIds(const LinkedList<std::string>& listLockedIds) {
    ensureDataDirExists();
    std::string strPath = DATA_DIR + "KhoaThe.txt";
    std::ofstream fout(strPath, std::ios::trunc);
    if (!fout.is_open()) {
        return false;
    }

    Node<std::string>* pCur = listLockedIds.getHead();
    while (pCur != nullptr) {
        fout << pCur->_data << "\n";
        pCur = pCur->_pNext;
    }

    fout.close();
    return true;
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

    long lBalance = 0;
    try {
        lBalance = std::stol(strBalanceLine);
    } catch (...) {
        lBalance = 0;
    }

    acc = Account(strFileId, strName, lBalance, strCurrency);
    fin.close();
    return ERR_NONE;
}

bool FileService::saveAccount(const Account& acc) {
    ensureDataDirExists();
    std::string strPath = DATA_DIR + acc.getId() + ".txt";
    std::ofstream fout(strPath, std::ios::trunc);
    if (!fout.is_open()) {
        return false;
    }

    fout << acc.getId() << "\n";
    fout << acc.getName() << "\n";
    fout << acc.getBalance() << "\n";
    fout << acc.getCurrency() << "\n";

    fout.close();
    return true;
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

    // 2. Tao file LichSu[ID].txt (khoi tao rong)
    std::string strHistoryPath = DATA_DIR + "LichSu" + strId + ".txt";
    std::ofstream foutHist(strHistoryPath, std::ios::app);
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

    // 3. Khoi tao TheTu.txt va cac file [ID].txt neu TheTu.txt chua co
    std::string strTheTuPath = DATA_DIR + "TheTu.txt";
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
}
