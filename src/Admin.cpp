#include "Admin.h"
#include "SecurityService.h"

Admin::Admin() : _strUsername(""), _strPassword("") {}

Admin::Admin(const std::string& strUser, const std::string& strPass)
    : _strUsername(strUser), _strPassword(strPass) {}

std::string Admin::getUsername() const {
    return this->_strUsername;
}

std::string Admin::getPassword() const {
    return this->_strPassword;
}

bool Admin::verifyPassword(const std::string& strPass) const {
    return SecurityService::verifyHash(strPass, this->_strPassword);
}
