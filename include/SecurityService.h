#ifndef SECURITYSERVICE_H_INCLUDED_
#define SECURITYSERVICE_H_INCLUDED_

#include <string>

/******************************************************************************
 * @Description: Lop tien ich bao mat, ma hoa va bam mat khau / ma PIN
 * Cung cap thuat toan bam MD5 chuan hoa (RFC 1321) thuan C++ (Zero-dependency).
 * Ho tro xac thuc thong minh da tang (kiem tra ca ma hash lan plaintext de tuong thich nguoc).
 ******************************************************************************/
class SecurityService {
public:
    /**********************************************************
     * @Description Tinh toan chuoi ma bam MD5 32 ky tu hex
     * @param strInput Chuoi du lieu can bam
     * @return Chuoi ma hash MD5 chu thuong (32 ky tu)
     **********************************************************/
    static std::string md5(const std::string& strInput);

    /**********************************************************
     * @Description Bam ma PIN kem muoi bao mat he thong
     * @param strPin Ma PIN 6 chu so
     * @return Chuoi ma hash
     **********************************************************/
    static std::string hashPin(const std::string& strPin);

    /**********************************************************
     * @Description Bam mat khau Admin
     * @param strPassword Mat khau can bam
     * @return Chuoi ma hash
     **********************************************************/
    static std::string hashPassword(const std::string& strPassword);

    /**********************************************************
     * @Description Xac thuc thong minh giua chuoi nhap vao va gia tri luu tru
     * Ho tro ca truong hop gia tri luu tru la plaintext (du lieu cu/test)
     * lan gia tri luu tru da duoc bam MD5.
     * @param strRaw Chuoi nguoi dung nhap vao
     * @param strStored Gia tri luu trong RAM hoac tap tin
     * @return true neu khop, false neu sai
     **********************************************************/
    static bool verifyHash(const std::string& strRaw, const std::string& strStored);
};

#endif // SECURITYSERVICE_H_INCLUDED_
