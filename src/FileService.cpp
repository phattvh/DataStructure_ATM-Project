#include "FileService.h"
#include "SecurityService.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <cstdio>
#include <vector>

#ifdef _WIN32
#include <process.h>
#define GET_CURRENT_PID() _getpid()

class FileLockGuard {
public:
    FileLockGuard(const std::string&) {}
    bool isLocked() const { return true; }
};
#else
#include <unistd.h>
#include <sys/file.h>
#include <fcntl.h>
#define GET_CURRENT_PID() getpid()

class FileLockGuard {
private:
    int _fd;
    bool _bLocked;

public:
    FileLockGuard(const std::string& strLockPath) : _fd(-1), _bLocked(false) {
        _fd = open(strLockPath.c_str(), O_RDWR | O_CREAT, 0666);
        if (_fd >= 0) {
            if (flock(_fd, LOCK_EX) == 0) {
                _bLocked = true;
            }
        }
    }

    ~FileLockGuard() {
        if (_fd >= 0) {
            if (_bLocked) {
                flock(_fd, LOCK_UN);
            }
            close(_fd);
        }
    }

    bool isLocked() const {
        return this->_bLocked;
    }
};
#endif

namespace fs = std::filesystem;


//Ham noi bo: Dam bao thu muc DATA_DIR ton tai
static void ensureDataDirExists() {
    try {
        if (!fs::exists(DATA_DIR)) {
            fs::create_directories(DATA_DIR);
        }
    } catch (...) {

    }
}

//Ham noi bo: Trim khoang trang dau va cuoi chuoi
static std::string trimString(const std::string& str) {
    size_t iStart = str.find_first_not_of(" \t\r\n");
    if (iStart == std::string::npos) return "";
    size_t iEnd = str.find_last_not_of(" \t\r\n");
    return str.substr(iStart, iEnd - iStart + 1);
}

//Ham noi bo: Ghi file nguyen tu (Atomic Write) qua file tam + tien to PID
static bool atomicWriteFile(const std::string& strPath, const std::string& strContent) {
    ensureDataDirExists();
    FileLockGuard lock(DATA_DIR + ".atm_data.lock");
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
    if (!ec) {
        return true;
    }

    // Neu rename that bai (vi du tren he dieu hanh khoa tap tin dich)
    std::string strBakPath = strPath + strSuffix + ".bak";
    std::error_code ecBak;
    if (fs::exists(strPath)) {
        fs::copy_file(strPath, strBakPath, fs::copy_options::overwrite_existing, ecBak);
    }

    fs::rename(strTempPath, strPath, ec);
    if (ec) {
        if (!ecBak && fs::exists(strBakPath)) {
            std::error_code ecRestore;
            fs::copy_file(strBakPath, strPath, fs::copy_options::overwrite_existing, ecRestore);
        }
        std::error_code ecCleanTmp;
        fs::remove(strTempPath, ecCleanTmp);
        return false;
    }

    if (!ecBak && fs::exists(strBakPath)) {
        std::error_code ecDelBak;
        fs::remove(strBakPath, ecDelBak);
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

        size_t iSpace = strLine.find(' ');
        if (iSpace != std::string::npos) {
            std::string strUser = trimString(strLine.substr(0, iSpace));
            std::string strPass = trimString(strLine.substr(iSpace + 1));
            if (!strUser.empty() && !strPass.empty()) {
                listAdmins.addTail(Admin(strUser, strPass));
            }
        }
    }

    fin.close();
    return true;
}

bool FileService::saveAdmins(const LinkedList<Admin>& listAdmins) {
    ensureDataDirExists();
    std::string strPath = DATA_DIR + "Admin.txt";
    std::ostringstream oss;
    Node<Admin>* pCur = listAdmins.getHead();
    while (pCur != nullptr) {
        std::string strPass = pCur->_data.getPassword();
        if (strPass.length() != 32) {
            strPass = SecurityService::hashPassword(strPass);
        }
        oss << pCur->_data.getUsername() << " " << strPass << "\n";
        pCur = pCur->_pNext;
    }
    return atomicWriteFile(strPath, oss.str());
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
            auto pDup = listCards.findIf([&strId](const Card& c) {
                return c.getId() == strId;
            });
            if (pDup != nullptr) {
                continue;
            }

            bool bIsLocked = false;
            // Kiem tra the co trong danh sach the khoa khong
            const std::string* pLocked = listLockedIds.findIf([&strId](const std::string& lockedId) {
                return lockedId == strId;
            });
            if (pLocked != nullptr) {
                bIsLocked = true;
            }

            int iFailed = getFailedAttempts(strId);
            Card card(strId, strPin, bIsLocked);
            if (iFailed > 0) {
                card.setFailedAttempts(iFailed);
            }

            listCards.addTail(card);
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
        std::string strPin = pCur->_data.getPin();
        if (strPin.length() != 32) {
            strPin = SecurityService::hashPin(strPin);
        }
        oss << pCur->_data.getId() << " " << strPin << "\n";
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
        strCurrency = "VND"; 
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

    // 2. Tao file LichSu[ID].txt: Neu file lich su da ton tai tu truoc tien hanh luu tru 
    std::string strHistoryPath = DATA_DIR + "LichSu" + strId + ".txt";
    if (fs::exists(strHistoryPath) && fs::file_size(strHistoryPath) > 0) {
        std::string strArchive = DATA_DIR + "Archive_LichSu" + strId + "_" + std::to_string(std::time(nullptr)) + ".bak";
        std::error_code ec;
        fs::rename(strHistoryPath, strArchive, ec);
    }

    // Khoi tao file LichSu moi trang tinh
    std::ofstream foutHist(strHistoryPath, std::ios::trunc);
    if (!foutHist.is_open()) {
        deleteAccountFile(strId);
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
    if (fout.fail()) {
        fout.close();
        return false;
    }
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

    std::string strMarkerPath = DATA_DIR + ".system_initialized";
    bool bInitialized = fs::exists(strMarkerPath);

    // 1. Khoi tao Admin.txt neu he thong chua tung khoi tao
    std::string strAdminPath = DATA_DIR + "Admin.txt";
    if (!bInitialized && (!fs::exists(strAdminPath) || fs::file_size(strAdminPath) == 0)) {
        std::ofstream fout(strAdminPath);
        if (fout.is_open()) {
            fout << "admin1 " << SecurityService::hashPassword("123456") << "\n";
            fout << "admin2 " << SecurityService::hashPassword("123456") << "\n";
            fout << "superadmin " << SecurityService::hashPassword("888888") << "\n";
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

    // 3. Khoi tao TheTu.txt va cac file [ID].txt neu he thong chua tung duoc khoi tao + marker .system_initialized 
    std::string strTheTuPath = DATA_DIR + "TheTu.txt";
    if (!bInitialized) {
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
                    foutTheTu << card.szId << " " << SecurityService::hashPin(card.szPin) << "\n";
                    // Tao file [ID].txt va LichSu[ID].txt
                    createAccountFiles(card.szId, card.szName, card.lBalance, "VND");
                }
                foutTheTu.close();
            }
        }

        std::ofstream foutMarker(strMarkerPath);
        if (foutMarker.is_open()) {
            foutMarker << "INITIALIZED\n";
            foutMarker.close();
        }
    }
}

//Ham noi bo: Khử trùng chuỗi ghi nhật ký (chống CWE-117)
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
    if (outFile.fail()) {
        outFile.close();
        return false;
    }
    outFile.close();
    return true;
}

bool FileService::archiveHistoryFile(const std::string& strId) {
    ensureDataDirExists();
    std::string strHistoryPath = DATA_DIR + "LichSu" + strId + ".txt";
    if (fs::exists(strHistoryPath) && fs::file_size(strHistoryPath) > 0) {
        std::string strArchive = DATA_DIR + "Archive_LichSu" + strId + "_" + std::to_string(std::time(nullptr)) + ".bak";
        std::error_code ec;
        fs::rename(strHistoryPath, strArchive, ec);
        return !ec;
    }
    return true;
}

int FileService::getFailedAttempts(const std::string& strId) {
    ensureDataDirExists();
    std::string strPath = DATA_DIR + "FailedAttempts.txt";
    std::ifstream fin(strPath);
    if (!fin.is_open()) {
        return 0;
    }
    std::string strLine;
    while (std::getline(fin, strLine)) {
        strLine = trimString(strLine);
        if (strLine.empty()) continue;
        std::istringstream iss(strLine);
        std::string strCurrentId;
        int iCount = 0;
        if (iss >> strCurrentId >> iCount) {
            if (strCurrentId == strId) {
                fin.close();
                return iCount;
            }
        }
    }
    fin.close();
    return 0;
}

int FileService::recordFailedAttempt(const std::string& strId) {
    ensureDataDirExists();
    std::string strPath = DATA_DIR + "FailedAttempts.txt";
    std::ifstream fin(strPath);
    std::vector<std::pair<std::string, int>> entries;
    bool bFound = false;
    int iNewCount = 1;

    if (fin.is_open()) {
        std::string strLine;
        while (std::getline(fin, strLine)) {
            strLine = trimString(strLine);
            if (strLine.empty()) continue;
            std::istringstream iss(strLine);
            std::string strCurrentId;
            int iCount = 0;
            if (iss >> strCurrentId >> iCount) {
                if (strCurrentId == strId) {
                    iCount++;
                    iNewCount = iCount;
                    bFound = true;
                }
                entries.push_back({strCurrentId, iCount});
            }
        }
        fin.close();
    }

    if (!bFound) {
        entries.push_back({strId, 1});
        iNewCount = 1;
    }

    std::ostringstream oss;
    for (const auto& item : entries) {
        oss << item.first << " " << item.second << "\n";
    }
    atomicWriteFile(strPath, oss.str());

    if (iNewCount >= MAX_FAILED_LOGINS) {
        appendLockedCard(strId);
    }
    return iNewCount;
}

bool FileService::resetFailedAttempts(const std::string& strId) {
    ensureDataDirExists();
    std::string strPath = DATA_DIR + "FailedAttempts.txt";
    std::ifstream fin(strPath);
    if (!fin.is_open()) {
        return true;
    }

    std::vector<std::pair<std::string, int>> entries;
    std::string strLine;
    bool bChanged = false;
    while (std::getline(fin, strLine)) {
        strLine = trimString(strLine);
        if (strLine.empty()) continue;
        std::istringstream iss(strLine);
        std::string strCurrentId;
        int iCount = 0;
        if (iss >> strCurrentId >> iCount) {
            if (strCurrentId == strId) {
                bChanged = true;
            } else {
                entries.push_back({strCurrentId, iCount});
            }
        }
    }
    fin.close();

    if (bChanged) {
        std::ostringstream oss;
        for (const auto& item : entries) {
            oss << item.first << " " << item.second << "\n";
        }
        return atomicWriteFile(strPath, oss.str());
    }
    return true;
}

