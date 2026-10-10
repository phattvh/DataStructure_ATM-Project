#include "Account.h"
#include <limits>

Account::Account()
    : _strId(""), _strName(""), _lBalance(0), _strCurrency("VND") {}

Account::Account(const std::string& strId,
                 const std::string& strName,
                 long lBalance,
                 const std::string& strCurrency)
    : _strId(strId),
      _strName(strName),
      _lBalance(lBalance < 0 ? 0 : lBalance),
      _strCurrency(strCurrency) {}

std::string Account::getId() const {
    return this->_strId;
}

std::string Account::getName() const {
    return this->_strName;
}

long Account::getBalance() const {
    return this->_lBalance;
}

std::string Account::getCurrency() const {
    return this->_strCurrency;
}

void Account::setName(const std::string& strName) {
    this->_strName = strName;
}

void Account::setBalance(long lBalance) {
    if (lBalance >= 0) {
        this->_lBalance = lBalance;
    }
}

void Account::setCurrency(const std::string& strCurrency) {
    this->_strCurrency = strCurrency;
}

ErrorCode Account::canWithdraw(long lAmount) const {
    long lMinTrans = (this->_strCurrency == "USD") ? 10 : MIN_TRANSACTION;
    long lMinReserve = (this->_strCurrency == "USD") ? 10 : MIN_BALANCE_RESERVE;

    if (lAmount < lMinTrans) {
        return ERR_INVALID_AMOUNT;
    }
    
    if (lAmount % lMinTrans != 0) {
        return ERR_NOT_MULTIPLE;
    }

    if (this->_lBalance < lAmount || (this->_lBalance - lAmount) < lMinReserve) {
        return ERR_INSUFFICIENT_FUNDS;
    }
    
    return ERR_NONE;
}

bool Account::withdraw(long lAmount) {
    if (this->canWithdraw(lAmount) != ERR_NONE) {
        return false;
    }
    
    this->_lBalance -= lAmount;
    return true;
}

bool Account::deposit(long lAmount) {
    if (lAmount <= 0) {
        return false;
    }

    if (std::numeric_limits<long>::max() - this->_lBalance < lAmount) {
        return false;
    }
    this->_lBalance += lAmount;
    return true;
}
