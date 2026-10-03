#ifndef CONSOLEVIEW_H_INCLUDED_
#define CONSOLEVIEW_H_INCLUDED_

#include <string>

/**********************************************************
 * @Description: Lop tien ich xu ly giao dien dong lenh (Console CLI)
 * Bao gom mau sac ANSI, an mat khau thanh dau *, va bay loi nhap lieu
 * (Phu trach boi Member C - Task 3.1)
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
     * @Description Nhap mat khau / ma PIN che giau thanh dau *
     * Ho tro ca Windows (_getch) va Linux (termios) kem xoa lui Backspace
     * @param strPrompt Loi nhac nhap
     * @return Chuoi mat khau nguoi dung da nhap
     **********************************************************/
    static std::string inputPassword(const std::string& strPrompt);

    /**********************************************************
     * @Description Nhap so tien an toan, bay loi cin.fail()
     * chong sap chuong trinh khi nguoi dung nhap chu
     * @param strPrompt Loi nhac nhap
     * @return So tien hop le kieu long
     **********************************************************/
    static long inputMoney(const std::string& strPrompt);

    /**********************************************************
     * @Description Tam dung man hinh cho nguoi dung nhan Enter
     * @return void
     **********************************************************/
    static void pauseScreen();
};

#endif // CONSOLEVIEW_H_INCLUDED_
