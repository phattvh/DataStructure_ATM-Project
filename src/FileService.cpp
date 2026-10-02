#include "FileService.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

bool FileService::loadAdmins(LinkedList<Admin>& listAdmins) {
    listAdmins.clear();
    std::string strPath = DATA_DIR + "Admin.txt";
    std::ifstream file(strPath);
    if (!file.is_open()) {
        return false;
    }

    std::string strUser;
    std::string strPass;
    while (file >> strUser >> strPass) {
        if (!strUser.empty()) {
            Admin admin(strUser, strPass);
            listAdmins.addTail(admin);
        }
    }

    file.close();
    return true;
}

bool FileService::loadLockedIds(LinkedList<std::string>& listLockedIds) {
    listLockedIds.clear();
    std::string strPath = DATA_DIR + "KhoaThe.txt";
    std::ifstream file(strPath);
    if (!file.is_open()) {
        return false;
    }

    std::string strId;
    while (file >> strId) {
        if (!strId.empty()) {
            listLockedIds.addTail(strId);
        }
    }

    file.close();
    return true;
}

bool FileService::loadCards(LinkedList<Card>& listCards, const LinkedList<std::string>& listLockedIds) {
    listCards.clear();
    std::string strPath = DATA_DIR + "TheTu.txt";
    std::ifstream file(strPath);
    if (!file.is_open()) {
        return false;
    }

    std::string strId;
    std::string strPin;
    while (file >> strId >> strPin) {
        if (!strId.empty()) {
            bool bIsLocked = false;
            // Kiem tra xem co nam trong danh sach khoa hay khong
            const std::string* pLocked = listLockedIds.findIf([&strId](const std::string& item) {
                return item == strId;
            });
            if (pLocked != nullptr) {
                bIsLocked = true;
            }

            Card card(strId, strPin, bIsLocked);
            listCards.addTail(card);
        }
    }

    file.close();
    return true;
}

bool FileService::saveCards(const LinkedList<Card>& listCards) {
    std::string strPath = DATA_DIR + "TheTu.txt";
    std::ofstream file(strPath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }

    Node<Card>* pCur = listCards.getHead();
    while (pCur != nullptr) {
        file << pCur->_data.getId() << " " << pCur->_data.getPin() << "\n";
        pCur = pCur->_pNext;
    }

    file.close();
    return true;
}

bool FileService::saveLockedIds(const LinkedList<std::string>& listLockedIds) {
    std::string strPath = DATA_DIR + "KhoaThe.txt";
    std::ofstream file(strPath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }

    Node<std::string>* pCur = listLockedIds.getHead();
    while (pCur != nullptr) {
        file << pCur->_data << "\n";
        pCur = pCur->_pNext;
    }

    file.close();
    return true;
}

ErrorCode FileService::loadAccount(const std::string& strId, Account& acc) {
    std::string strPath = DATA_DIR + strId + ".txt";
    std::ifstream file(strPath);
    if (!file.is_open()) {
        return ERR_FILE_NOT_FOUND;
    }

    std::string strLineId;
    std::string strName;
    std::string strBalance;
    std::string strCurrency;

    if (std::getline(file, strLineId) &&
        std::getline(file, strName) &&
        std::getline(file, strBalance) &&
        std::getline(file, strCurrency)) {
        
        long lBalance = 0;
        try {
            lBalance = std::stol(strBalance);
        } catch (...) {
            lBalance = 0;
        }

        acc = Account(strLineId, strName, lBalance, strCurrency);
        file.close();
        return ERR_NONE;
    }

    file.close();
    return ERR_FILE_NOT_FOUND;
}

bool FileService::saveAccount(const Account& acc) {
    std::string strPath = DATA_DIR + acc.getId() + ".txt";
    std::ofstream file(strPath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }

    file << acc.getId() << "\n";
    file << acc.getName() << "\n";
    file << acc.getBalance() << "\n";
    file << acc.getCurrency() << "\n";

    file.close();
    return true;
}

bool FileService::deleteAccountFile(const std::string& strId) {
    std::string strPath = DATA_DIR + strId + ".txt";
    return (std::remove(strPath.c_str()) == 0);
}

void FileService::createAccountFiles(const std::string& strId, 
                                     const std::string& strName, 
                                     long lInitialBalance, 
                                     const std::string& strCurrency) {
    // Tao thu muc neu chua ton tai
    try {
        if (!fs::exists(DATA_DIR)) {
            fs::create_directories(DATA_DIR);
        }
    } catch (...) {}

    // 1. Tao file [ID].txt
    std::string strAccPath = DATA_DIR + strId + ".txt";
    std::ofstream accFile(strAccPath, std::ios::out | std::ios::trunc);
    if (accFile.is_open()) {
        accFile << strId << "\n";
        accFile << strName << "\n";
        accFile << lInitialBalance << "\n";
        accFile << strCurrency << "\n";
        accFile.close();
    }

    // 2. Tao file LichSu[ID].txt (file rong khoi tao)
    std::string strHistoryPath = DATA_DIR + "LichSu" + strId + ".txt";
    std::ifstream checkFile(strHistoryPath);
    if (!checkFile.is_open()) {
        std::ofstream histFile(strHistoryPath, std::ios::out | std::ios::trunc);
        if (histFile.is_open()) {
            histFile.close();
        }
    } else {
        checkFile.close();
    }
}

bool FileService::appendTransaction(const std::string& strId, const Transaction& trans) {
    std::string strPath = DATA_DIR + "LichSu" + strId + ".txt";
    std::ofstream file(strPath, std::ios::app);
    if (!file.is_open()) {
        return false;
    }

    file << trans.formatForFile() << "\n";
    file.close();
    return true;
}

bool FileService::loadTransactions(const std::string& strId, LinkedList<Transaction>& listTrans) {
    listTrans.clear();
    std::string strPath = DATA_DIR + "LichSu" + strId + ".txt";
    std::ifstream file(strPath);
    if (!file.is_open()) {
        return false;
    }

    std::string strLine;
    while (std::getline(file, strLine)) {
        if (!strLine.empty()) {
            Transaction trans(strId, WITHDRAW, 0, "", strLine);
            listTrans.addTail(trans);
        }
    }

    file.close();
    return true;
}

void FileService::initMockDataIfMissing() {
    try {
        if (!fs::exists(DATA_DIR)) {
            fs::create_directories(DATA_DIR);
        }
    } catch (...) {}

    std::string strAdminPath = DATA_DIR + "Admin.txt";
    std::ifstream adminCheck(strAdminPath);
    if (!adminCheck.is_open()) {
        std::ofstream adminFile(strAdminPath);
        if (adminFile.is_open()) {
            adminFile << "admin1 123456\n";
            adminFile << "admin2 123456\n";
            adminFile << "admin3 123456\n";
            adminFile.close();
        }
    } else {
        adminCheck.close();
    }

    std::string strKhoaThePath = DATA_DIR + "KhoaThe.txt";
    std::ifstream lockCheck(strKhoaThePath);
    if (!lockCheck.is_open()) {
        std::ofstream lockFile(strKhoaThePath);
        if (lockFile.is_open()) {
            lockFile.close();
        }
    } else {
        lockCheck.close();
    }

    std::string strTheTuPath = DATA_DIR + "TheTu.txt";
    std::ifstream cardCheck(strTheTuPath);
    if (!cardCheck.is_open()) {
        const std::string mockCards[10][4] = {
            {"10014504500001", "123456", "Nguyen Van An", "500000"},
            {"10014504500002", "123456", "Tran Thi Binh", "1200000"},
            {"10014504500003", "123456", "Le Van Cuong", "750000"},
            {"10014504500004", "123456", "Pham Minh Duc", "3000000"},
            {"10014504500005", "123456", "Hoang Thi Em", "250000"},
            {"10014504500006", "123456", "Doan Van Giap", "900000"},
            {"10014504500007", "123456", "Vu Thi Hoa", "450000"},
            {"10014504500008", "123456", "Bui Van Khoa", "1500000"},
            {"10014504500009", "123456", "Ngo Thi Lan", "600000"},
            {"10014504500010", "123456", "Nguyen Trung Kien", "2000000"}
        };

        std::ofstream cardFile(strTheTuPath);
        if (cardFile.is_open()) {
            for (int i = 0; i < 10; i++) {
                cardFile << mockCards[i][0] << " " << mockCards[i][1] << "\n";
                createAccountFiles(mockCards[i][0], mockCards[i][2], std::stol(mockCards[i][3]), "VND");
            }
            cardFile.close();
        }
    } else {
        cardCheck.close();
    }
}
