#ifndef CARD_H_INCLUDED_
#define CARD_H_INCLUDED_

#include <string>
#include "Common.h"

/**
 * @Description: Lop dai dien cho the tu ATM cua nguoi dung
 */
class Card {
private:
    std::string _strId;
    std::string _strPin;
    int _iFailedAttempts;
    bool _bIsLocked;

public:
    Card();
    Card(const std::string& strId, const std::string& strPin);
    Card(const std::string& strId, const std::string& strPin, bool bIsLocked);

    std::string getId() const;
    std::string getPin() const;
    bool isDefaultPin() const;
    bool isLocked() const;
    int getFailedAttempts() const;

    void setLocked(bool bLocked);
    bool checkPin(const std::string& strInput) const;
    void recordFailedAttempt();
    void resetFailedAttempts();
    void changePin(const std::string& strNewPin);
};

#endif // CARD_H_INCLUDED_
