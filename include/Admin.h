#ifndef ADMIN_H_INCLUDED_
#define ADMIN_H_INCLUDED_

#include <string>

/**
 * @Description: Lop dai dien cho thuc the Admin quan tri he thong ATM
 */
class Admin {
private:
    std::string _strUsername;
    std::string _strPassword;

public:
    Admin();
    Admin(const std::string& strUsername, const std::string& strPassword);

    std::string getUsername() const;
    std::string getPassword() const;

    bool verifyPassword(const std::string& strInput) const;
    void setPassword(const std::string& strNewPassword);
};

#endif // ADMIN_H_INCLUDED_
