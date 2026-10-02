#include "Admin.h"

Admin::Admin() : _strUsername(""), _strPassword("") {}

Admin::Admin(const std::string& strUsername, const std::string& strPassword)
    : _strUsername(strUsername), _strPassword(strPassword) {}

std::string Admin::getUsername() const {
    return this->_strUsername;
}

std::string Admin::getPassword() const {
    return this->_strPassword;
}

bool Admin::verifyPassword(const std::string& strInput) const {
    return (this->_strPassword == strInput);
}

void Admin::setPassword(const std::string& strNewPassword) {
    this->_strPassword = strNewPassword;
}
