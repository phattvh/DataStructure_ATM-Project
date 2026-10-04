#ifndef CONSOLEVIEW_H_INCLUDED_
#define CONSOLEVIEW_H_INCLUDED_

#include <string>
#include <iostream>
#include <iomanip>
#include "Common.h"

// Forward declaration
class Account;

/**********************************************************
 * @Description: Lop tien ich xu ly giao dien dong lenh (Console CLI)
 * Bao gom mau sac ANSI, an mat khau thanh dau *, bay loi nhap lieu
 * va dinh dang khung vien menu Admin / User
 **********************************************************/
class ConsoleView {
public:
    /**********************************************************
     * @Description In tieu de khung vien noi bat
     * @param strTitle Chuoi tieu de can in
     * @return void
     **********************************************************/
    static void printHeader(const std::string& strTitle);

    /**********************************************************
     * @Description In loi nhac lenh cho nguoi dung nhap lieu
     * @param strPrompt Chuoi nhac lenh
     * @return void
     **********************************************************/
    static void printPrompt(const std::string& strPrompt);

    /**********************************************************
     * @Description In thong bao loi mau do
     * @param strMsg Noi dung loi
     * @return void
     **********************************************************/
    static void printError(const std::string& strMsg);

    /**********************************************************
     * @Description In thong bao thanh cong mau xanh la
     * @param strMsg Noi dung thanh cong
     * @return void
     **********************************************************/
    static void printSuccess(const std::string& strMsg);

    /**********************************************************
     * @Description In thong bao canh bao mau vang
     * @param strMsg Noi dung canh bao
     * @return void
     **********************************************************/
    static void printWarning(const std::string& strMsg);

    /**********************************************************
     * @Description In thong bao thong tin mau xanh duong
     * @param strMsg Noi dung thong tin
     * @return void
     **********************************************************/
    static void printInfo(const std::string& strMsg);

    /**********************************************************
     * @Description Nhap mat khau / ma PIN che giau thanh dau *
     * Ho tro xu ly mui ten/escape sequences, Backspace, va stream mock
     * @param strPrompt Loi nhac nhap
     * @param pInStream Con tro luong nhap tuy chon (nullptr: doc truc tiep terminal)
     * @return Chuoi mat khau nguoi dung da nhap
     **********************************************************/
    static std::string inputPassword(const std::string& strPrompt,
                                     std::istream* pInStream = nullptr);

    /**********************************************************
     * @Description Alias goi ham inputPassword cho ma PIN
     * @param strPrompt Loi nhac nhap
     * @return Chuoi PIN
     **********************************************************/
    static std::string inputPin(const std::string& strPrompt);

    /**********************************************************
     * @Description Nhap so tien an toan theo dong, chong troi lenh,
     * chan so thuc, chu cai, so am va xu ly EOF khong bi treo loop
     * @param strPrompt Loi nhac nhap
     * @param inStream Luong du lieu dau vao (mac dinh: std::cin)
     * @return So tien hop le kieu long (hoac 0 neu gap EOF)
     **********************************************************/
    static long inputMoney(const std::string& strPrompt,
                           std::istream& inStream = std::cin);

    /**********************************************************
     * @Description Nhap lua chon menu trong khoang [iMin, iMax] co bay loi
     * @param iMin Gia tri nho nhat
     * @param iMax Gia tri lon nhat
     * @param strPrompt Loi nhac nhap
     * @return So nguyen lua chon hop le
     **********************************************************/
    static int inputMenuChoice(int iMin, int iMax, const std::string& strPrompt);

    /**********************************************************
     * @Description Nhap mot dong chuoi van ban co khoang trang
     * @param strPrompt Loi nhac nhap
     * @return Chuoi nguoi dung da nhap
     **********************************************************/
    static std::string inputLine(const std::string& strPrompt);

    /**********************************************************
     * @Description Xac nhan hanh dong Co / Khong (y/n)
     * @param strPrompt Loi nhac xac nhan
     * @return true neu y/Y, nguoc lai false
     **********************************************************/
    static bool confirmAction(const std::string& strPrompt);

    /**********************************************************
     * @Description In menu dieu huong chinh cua he thong ATM
     * @return void
     **********************************************************/
    static void printMainMenu();

    /**********************************************************
     * @Description In menu giao dien quan tri Admin theo de bai
     * @return void
     **********************************************************/
    static void printAdminMenu();

    /**********************************************************
     * @Description In menu giao dien khach hang User
     * @return void
     **********************************************************/
    static void printUserMenu();

    /**********************************************************
     * @Description Xoa man hinh console bang ma ANSI
     * @return void
     **********************************************************/
    static void clearScreen();

    /**********************************************************
     * @Description Tam dung man hinh cho nguoi dung nhan Enter
     * @return void
     **********************************************************/
    static void pauseScreen();

    /**********************************************************
     * @Description In tieu de bang danh sach the tu
     * @return void
     **********************************************************/
    static void printCardTableHeader();

    /**********************************************************
     * @Description In mot dong thong tin the tu trong bang
     * @param strId Ma so the 14 chu so
     * @param strPin Ma PIN the
     * @param bIsLocked Trang thai the bi khoa hay khong
     * @return void
     **********************************************************/
    static void printCardRow(const std::string& strId,
                             const std::string& strPin,
                             bool bIsLocked);

    /**********************************************************
     * @Description In duong vien ket thuc bang the tu
     * @return void
     **********************************************************/
    static void printCardTableFooter();

    /**********************************************************
     * @Description In bien lai giao dich tai chinh
     * @param strId Ma so tai khoan
     * @param strAction Ten loai giao dich
     * @param lAmount So tien giao dich
     * @param lRemainingBalance So du con lai
     * @param strTimestamp Thoi gian thuc hien
     * @return void
     **********************************************************/
    static void printReceipt(const std::string& strId,
                             const std::string& strAction,
                             long lAmount,
                             long lRemainingBalance,
                             const std::string& strTimestamp);

    /**********************************************************
     * @Description Hien thi thong tin chi tiet tai khoan (phien ban doc lap)
     * @param strId Ma so tai khoan
     * @param strName Ho ten chu tai khoan
     * @param lBalance So du kha dung
     * @param strCurrency Don vi tien te
     * @return void
     **********************************************************/
    static void displayAccountDetails(const std::string& strId,
                                      const std::string& strName,
                                      long lBalance,
                                      const std::string& strCurrency);

    /**********************************************************
     * @Description Hien thi thong tin tai khoan tu doi tuong Account
     * @param account Doi tuong Account can xem
     * @return void
     **********************************************************/
    static void displayAccountInfo(const Account& account);

    /**********************************************************
     * @Description Template hien thi danh sach the tu he thong
     **********************************************************/
    template <typename ListCardType, typename ListLockedType>
    static void displayCardList(const ListCardType& listCards, const ListLockedType& listLockedIds) {
        printHeader("DANH SACH THE TU HE THONG");
        std::cout << std::left 
                  << std::setw(6)  << "STT" 
                  << std::setw(20) << "MA THE (ID)" 
                  << std::setw(12) << "MA PIN" 
                  << std::setw(18) << "TRANG THAI" 
                  << "\n";
        std::cout << "------------------------------------------------------\n";

        int iIndex = 1;
        auto pCur = listCards.getHead();
        while (pCur != nullptr) {
            std::string strId = pCur->_data.getId();
            bool bIsLocked = pCur->_data.isLocked();

            auto pLockCur = listLockedIds.getHead();
            while (pLockCur != nullptr) {
                if (pLockCur->_data == strId) {
                    bIsLocked = true;
                    break;
                }
                pLockCur = pLockCur->_pNext;
            }

            std::string strStatus = bIsLocked ? "\033[31mBi khoa\033[0m" 
                                              : "\033[32mHoat dong\033[0m";

            std::cout << std::left 
                      << std::setw(6)  << iIndex++ 
                      << std::setw(20) << strId 
                      << std::setw(12) << "******" 
                      << std::setw(18) << strStatus 
                      << "\n";

            pCur = pCur->_pNext;
        }
        std::cout << "------------------------------------------------------\n";
    }
};

#endif // CONSOLEVIEW_H_INCLUDED_
