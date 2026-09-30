#include "Card.h"

Card::Card() : _strId(""), _strPin(DEFAULT_PIN), _iFailedAttempts(0), _bIsLocked(false) {}

Card::Card(const std::string& strId, const std::string& strPin)
    : _strId(strId), _strPin(strPin), _iFailedAttempts(0), _bIsLocked(false) {}

Card::Card(const std::string& strId, const std::string& strPin, bool bIsLocked)
    : _strId(strId), _strPin(strPin), _iFailedAttempts(0), _bIsLocked(bIsLocked) {}

std::string Card::getId() const {
    return this->_strId;
}

std::string Card::getPin() const {
    return this->_strPin;
}

bool Card::isDefaultPin() const {
    return (this->_strPin == DEFAULT_PIN);
}

bool Card::isLocked() const {
    return this->_bIsLocked;
}

int Card::getFailedAttempts() const {
    return this->_iFailedAttempts;
}

void Card::setLocked(bool bLocked) {
    this->_bIsLocked = bLocked;
}

bool Card::checkPin(const std::string& strInput) const {
    return (this->_strPin == strInput);
}

void Card::recordFailedAttempt() {
    this->_iFailedAttempts++;
    if (this->_iFailedAttempts >= MAX_FAILED_LOGINS) {
        this->_bIsLocked = true;
    }
}

void Card::resetFailedAttempts() {
    this->_iFailedAttempts = 0;
    this->_bIsLocked = false;
}

void Card::changePin(const std::string& strNewPin) {
    this->_strPin = strNewPin;
}
