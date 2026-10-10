#include "Transaction.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>

Transaction::Transaction()
    : _strId(""),
      _eType(WITHDRAW),
      _lAmount(0),
      _strTimestamp(""),
      _strDetail("") {}

Transaction::Transaction(const std::string& strId,
                         TransactionType eType,
                         long lAmount,
                         const std::string& strTimestamp,
                         const std::string& strDetail)
    : _strId(strId),
      _eType(eType),
      _lAmount(lAmount),
      _strTimestamp(strTimestamp),
      _strDetail(strDetail) {}

std::string Transaction::getId() const {
    return this->_strId;
}

TransactionType Transaction::getType() const {
    return this->_eType;
}

std::string Transaction::getTypeName() const {
    switch (this->_eType) {
        case WITHDRAW: return "RUT TIEN";
        case TRANSFER: return "CHUYEN TIEN";
        case RECEIVE:  return "NHAN TIEN";
        default:       return "GIAO DICH";
    }
}

long Transaction::getAmount() const {
    return this->_lAmount;
}

std::string Transaction::getTimestamp() const {
    return this->_strTimestamp;
}

std::string Transaction::getDetail() const {
    return this->_strDetail;
}

std::string Transaction::formatForFile() const {
    return this->_strTimestamp + "|" +
           std::to_string(static_cast<int>(this->_eType)) + "|" +
           std::to_string(this->_lAmount) + "|" +
           this->_strDetail;
}

std::string Transaction::toString() const {
    std::ostringstream oss;
    oss << "[" << this->_strTimestamp << "] "
        << std::left << std::setw(12) << this->getTypeName() << ": "
        << std::right << std::setw(10) << this->_lAmount << " VND"
        << " - " << this->_strDetail;
    return oss.str();
}

static std::string trimToken(const std::string& str) {
    size_t iStart = str.find_first_not_of(" \t\r\n");
    if (iStart == std::string::npos) return "";
    size_t iEnd = str.find_last_not_of(" \t\r\n");
    return str.substr(iStart, iEnd - iStart + 1);
}

static size_t findNthChar(const std::string& str, char target, int n) {
    int count = 0;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == target) {
            count++;
            if (count == n) {
                return i;
            }
        }
    }
    return std::string::npos;
}

Transaction Transaction::parseFromFileLine(const std::string& strId, const std::string& strLine) {
    if (strLine.empty()) {
        return Transaction();
    }

    std::vector<std::string> vecTokens;
    std::string strToken;
    std::istringstream issStream(strLine);
    while (std::getline(issStream, strToken, '|')) {
        vecTokens.push_back(trimToken(strToken));
    }

    if (vecTokens.size() < 4) {
        return Transaction(strId, WITHDRAW, 0, "", strLine);
    }

    // DINH DANG 1: Chuan dac ta docs/02_yeu_cau_va_pham_vi.md (>= 5 truong: ID | Loai GD | So tien | Thoi gian | Chi tiet)
    bool bIsSpecFormat = false;
    if (vecTokens.size() >= 5 && vecTokens[0].length() == 14) {
        bool bAllDigits = true;
        for (char c : vecTokens[0]) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                bAllDigits = false;
                break;
            }
        }
        if (bAllDigits) {
            bIsSpecFormat = true;
        }
    }

    if (bIsSpecFormat) {
        std::string strRecordId = vecTokens[0];
        std::string strTypeRaw = vecTokens[1];
        long lAmount = 0;
        try {
            lAmount = std::stol(vecTokens[2]);
        } catch (...) {
            return Transaction(strId, WITHDRAW, 0, vecTokens[3], strLine);
        }
        std::string strTimestamp = vecTokens[3];
        size_t iPipe4 = findNthChar(strLine, '|', 4);
        std::string strDetail = (iPipe4 != std::string::npos) ? trimToken(strLine.substr(iPipe4 + 1)) : "";

        TransactionType eType = WITHDRAW;
        if (strTypeRaw == "2" || strTypeRaw == "CHUYEN TIEN" || strTypeRaw == "TRANSFER") {
            eType = TRANSFER;
        } else if (strTypeRaw == "3" || strTypeRaw == "NHAN TIEN" || strTypeRaw == "RECEIVE") {
            eType = RECEIVE;
        } else {
            eType = WITHDRAW;
        }

        return Transaction(strRecordId, eType, lAmount, strTimestamp, strDetail);
    }

    // DINH DANG 2: Dinh dang noi bo cua Member B (4 truong: Thoi gian | Loai GD | So tien | Chi tiet)
    std::string strTimestamp = vecTokens[0];
    int iType = 1;
    long lAmount = 0;
    try {
        iType = std::stoi(vecTokens[1]);
        lAmount = std::stol(vecTokens[2]);
    } catch (...) {
        return Transaction(strId, WITHDRAW, 0, strTimestamp, strLine);
    }
    size_t iPipe3 = findNthChar(strLine, '|', 3);
    std::string strDetail = (iPipe3 != std::string::npos) ? strLine.substr(iPipe3 + 1) : "";

    TransactionType eType = WITHDRAW;
    if (iType == 2) eType = TRANSFER;
    else if (iType == 3) eType = RECEIVE;

    return Transaction(strId, eType, lAmount, strTimestamp, strDetail);
}

std::string Transaction::getCurrentTimestamp() {
    return getNowTimestamp();
}
