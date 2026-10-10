#include "Card.h"
#include "Common.h"
#include "SecurityService.h"
#include <cctype>

bool Card::isValidPinFormat(const std::string& strPin) {
    if (strPin.length() != static_cast<size_t>(PIN_LENGTH)) {
        return false;
    }
    for (char c : strPin) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

Card::Card()
    : _strId(""), _strPin(DEFAULT_PIN), _iFailedAttempts(0), _bIsLocked(false) {}

Card::Card(const std::string& strId, const std::string& strPin)
    : _strId(strId),
      _strPin(strPin),
      _iFailedAttempts(0),
      _bIsLocked(false) {}

Card::Card(const std::string& strId, const std::string& strPin, bool bIsLocked)
    : _strId(strId),
      _strPin(strPin),
      _iFailedAttempts(0),
      _bIsLocked(bIsLocked) {}

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
    return (this->_strPin == DEFAULT_PIN || SecurityService::verifyHash(DEFAULT_PIN, this->_strPin));
}

bool Card::checkPin(const std::string& strInputPin) const {
    return SecurityService::verifyHash(strInputPin, this->_strPin);
}

void Card::recordFailedAttempt() {
    if (this->_bIsLocked) {
        return;
    }
    this->_iFailedAttempts++;
    if (this->_iFailedAttempts >= MAX_FAILED_LOGINS) {
        this->_bIsLocked = true;
    }
}

void Card::resetFailedAttempts() {
    this->_iFailedAttempts = 0;
}

void Card::unlockCard() {
    this->_bIsLocked = false;
    this->_iFailedAttempts = 0;
}

void Card::lockCard() {
    this->_bIsLocked = true;
}

void Card::setLocked(bool bLocked) {
    this->_bIsLocked = bLocked;
    if (!bLocked) {
        this->_iFailedAttempts = 0;
    }
}

bool Card::changePin(const std::string& strNewPin) {
    if (!isValidPinFormat(strNewPin)) {
        return false;
    }
    this->_strPin = strNewPin;
    return true;
}

int Card::getFailedAttempts() const {
    return this->_iFailedAttempts;
}

void Card::setFailedAttempts(int iAttempts) {
    this->_iFailedAttempts = iAttempts;
    if (this->_iFailedAttempts >= MAX_FAILED_LOGINS) {
        this->_bIsLocked = true;
    }
}

