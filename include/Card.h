#ifndef CARD_H_INCLUDED_
#define CARD_H_INCLUDED_

#include <string>

/**********************************************************
 * @Description: Lop dai dien cho Thuc the The tu trong he thong ATM
 * (Phu trach boi Member C - Task 3.2)
 **********************************************************/
class Card {
private:
    std::string _strId;
    std::string _strPin;
    int _iFailedAttempts;
    bool _bIsLocked;

public:
    /**********************************************************
     * @Description Constructor mac dinh
     **********************************************************/
    Card();

    /**********************************************************
     * @Description Constructor khoi tao co tham so
     * @param strId Ma so the (14 chu so)
     * @param strPin Ma PIN (6 chu so)
     * @param bIsLocked Trang thai the bi khoa hay khong
     **********************************************************/
    Card(const std::string& strId, const std::string& strPin, bool bIsLocked = false);

    /**********************************************************
     * @Description Lay ma so ID cua the
     * @return Chuoi ID
     **********************************************************/
    std::string getId() const;

    /**********************************************************
     * @Description Lay ma PIN cua the
     * @return Chuoi PIN
     **********************************************************/
    std::string getPin() const;

    /**********************************************************
     * @Description Kiem tra the co dang bi khoa khong
     * @return true neu bi khoa, nguoc lai false
     **********************************************************/
    bool isLocked() const;

    /**********************************************************
     * @Description Kiem tra the co dang mang ma PIN mac dinh khong
     * @return true neu PIN la 123456
     **********************************************************/
    bool isDefaultPin() const;

    /**********************************************************
     * @Description Kiem tra ma PIN nhap vao co khop khong
     * @param strInputPin Ma PIN can kiem tra
     * @return true neu khop, nguoc lai false
     **********************************************************/
    bool checkPin(const std::string& strInputPin) const;

    /**********************************************************
     * @Description Ghi nhan 1 lan dang nhap sai PIN.
     * Neu so lan sai >= MAX_FAILED_LOGINS (3 lan) thi tu dong khoa the
     * @return void
     **********************************************************/
    void recordFailedAttempt();

    /**********************************************************
     * @Description Dat lai so lan dang nhap sai ve 0 khi dang nhap thanh cong
     * (Khong tu dong mo khoa the da bi khoa)
     * @return void
     **********************************************************/
    void resetFailedAttempts();

    /**********************************************************
     * @Description Mo khoa the va dat lai so lan dang nhap sai ve 0
     * @return void
     **********************************************************/
    void unlockCard();

    /**********************************************************
     * @Description Chu dong khoa the
     * @return void
     **********************************************************/
    void lockCard();

    /**********************************************************
     * @Description Thay doi ma PIN moi cho the co kiem tra hop le:
     * phai dung PIN_LENGTH (6 ky tu) va toan bo la chu so
     * @param strNewPin Ma PIN moi
     * @return true neu doi thanh cong, false neu ma PIN sai dinh dang
     **********************************************************/
    bool changePin(const std::string& strNewPin);

    /**********************************************************
     * @Description Lay so lan dang nhap sai hien tai
     * @return So lan sai kieu int
     **********************************************************/
    int getFailedAttempts() const;

    /**********************************************************
     * @Description Ham tien ich tinh kiem tra dinh dang ma PIN
     * @param strPin Ma PIN can kiem tra
     * @return true neu dung 6 chu so
     **********************************************************/
    static bool isValidPinFormat(const std::string& strPin);
};

#endif // CARD_H_INCLUDED_
