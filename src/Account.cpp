#include "Account.h"

Account::Account() : _strId(""), _strName(""), _lBalance(0), _strCurrency("VND") {}

Account::Account(const std::string& strId, const std::string& strName, long lBalance, const std::string& strCurrency)
    : _strId(strId), _strName(strName), _lBalance(lBalance), _strCurrency(strCurrency) {}

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
    this->_lBalance = lBalance;
}

void Account::setCurrency(const std::string& strCurrency) {
    this->_strCurrency = strCurrency;
}

ErrorCode Account::canWithdraw(long lAmount) const {
    if (lAmount < MIN_TRANSACTION) {
        return ERR_INVALID_AMOUNT;
    }
    if (lAmount % MIN_TRANSACTION != 0) {
        return ERR_NOT_MULTIPLE;
    }
    if (this->_lBalance - lAmount < MIN_BALANCE_RESERVE) {
        return ERR_INSUFFICIENT_FUNDS;
    }
    return ERR_NONE;
}

void Account::withdraw(long lAmount) {
    this->_lBalance -= lAmount;
}

void Account::deposit(long lAmount) {
    this->_lBalance += lAmount;
}
