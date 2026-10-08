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
    // Dinh dang luu file phan tach bang ky tu '|':
    // [TIMESTAMP]|[TYPE_NUM]|[AMOUNT]|[DETAIL]
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

Transaction Transaction::parseFromFileLine(const std::string& strId, const std::string& strLine) {
    if (strLine.empty()) {
        return Transaction();
    }

    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(strLine);
    while (std::getline(tokenStream, token, '|')) {
        tokens.push_back(token);
    }

    // Neu khong phai format co dau '|', coi nhu dong van ban tu do
    if (tokens.size() < 4) {
        return Transaction(strId, WITHDRAW, 0, "", strLine);
    }

    std::string strTimestamp = tokens[0];
    int iType = 1;
    long lAmount = 0;
    try {
        iType = std::stoi(tokens[1]);
        lAmount = std::stol(tokens[2]);
    } catch (...) {
        return Transaction(strId, WITHDRAW, 0, strTimestamp, strLine);
    }
    std::string strDetail = tokens[3];

    TransactionType eType = WITHDRAW;
    if (iType == 2) eType = TRANSFER;
    else if (iType == 3) eType = RECEIVE;

    return Transaction(strId, eType, lAmount, strTimestamp, strDetail);
}

std::string Transaction::getCurrentTimestamp() {
    return getNowTimestamp();
}
