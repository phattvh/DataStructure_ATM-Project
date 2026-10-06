#ifndef ADMIN_H_INCLUDED_
#define ADMIN_H_INCLUDED_

#include <string>

/******************************************************************************
 * @Description: Lop dai dien cho Thuc the Quan tri vien (Admin)
 * (Tuan thu C++ Coding Standard V2)
 ******************************************************************************/
class Admin {
private:
    std::string _strUsername;
    std::string _strPassword;

public:
    /**********************************************************
     * @Description Constructor mac dinh
     **********************************************************/
    Admin();

    /**********************************************************
     * @Description Constructor khoi tao co tham so
     * @param strUser Ten dang nhap
     * @param strPass Mat khau dang nhap
     **********************************************************/
    Admin(const std::string& strUser, const std::string& strPass);

    /**********************************************************
     * @Description Lay ten dang nhap cua quan tri vien
     * @return Chuoi username
     **********************************************************/
    std::string getUsername() const;

    /**********************************************************
     * @Description Lay mat khau cua quan tri vien
     * @return Chuoi password
     **********************************************************/
    std::string getPassword() const;

    /**********************************************************
     * @Description Xac thuc mat khau quan tri vien
     * @param strPass Mat khau can kiem tra
     * @return true neu mat khau khop, nguoc lai false
     **********************************************************/
    bool verifyPassword(const std::string& strPass) const;
};

#endif // ADMIN_H_INCLUDED_
