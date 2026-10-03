#include "Card.h"
#include "Common.h"

Card::Card()
    : _strId(""), _strPin(DEFAULT_PIN), _iFailedAttempts(0), _bIsLocked(false) {}

Card::Card(const std::string& strId, const std::string& strPin, bool bIsLocked)
    : _strId(strId), _strPin(strPin), _iFailedAttempts(0), _bIsLocked(bIsLocked) {}

std::string Card::getId() const {
    return this->_strId;
}

std::string Card::getPin() const {
    return this->_strPin;
}

bool Card::isLocked() const {
    return this->_bIsLocked;
}

bool Card::isDefaultPin() const {
    return (this->_strPin == DEFAULT_PIN);
}

bool Card::checkPin(const std::string& strInputPin) const {
    return (this->_strPin == strInputPin);
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

int Card::getFailedAttempts() const {
    return this->_iFailedAttempts;
}
