#include "Transaction.h"
#include <sstream>

Transaction::Transaction()
    : _strId(""), _eType(WITHDRAW), _lAmount(0), _strTimestamp(""), _strDetail("") {}

Transaction::Transaction(const std::string& strId, TransactionType eType, long lAmount,
                         const std::string& strTimestamp, const std::string& strDetail)
    : _strId(strId), _eType(eType), _lAmount(lAmount),
      _strTimestamp(strTimestamp), _strDetail(strDetail) {}

std::string Transaction::getId() const {
    return this->_strId;
}

TransactionType Transaction::getType() const {
    return this->_eType;
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
    std::ostringstream oss;
    std::string strTypeName;
    switch (this->_eType) {
        case WITHDRAW:
            strTypeName = "RUT_TIEN";
            break;
        case TRANSFER:
            strTypeName = "CHUYEN_TIEN";
            break;
        case RECEIVE:
            strTypeName = "NHAN_TIEN";
            break;
        default:
            strTypeName = "KHAC";
            break;
    }
    oss << this->_strTimestamp << " | " << strTypeName << " | " << this->_lAmount << " VND | " << this->_strDetail;
    return oss.str();
}

std::string Transaction::toString() const {
    return this->formatForFile();
}
