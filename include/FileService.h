#ifndef FILESERVICE_H_INCLUDED_
#define FILESERVICE_H_INCLUDED_

#include <string>
#include "Common.h"
#include "LinkedList.h"
#include "Admin.h"
#include "Card.h"
#include "Account.h"
#include "Transaction.h"

/******************************************************************************
 * @Description: Lop tien ich xu ly Doc/Ghi tap tin co so du lieu he thong
 * Toan bo cac ham la static, tuan thu nguyen tac Rule 18 (co open la phai co close)
 * va xu ly an toan cho cac truong hop file rong, chua ton tai hoac co khoang trang.
 * (Nhiem vu cot loi cua Thanh vien B - Tri)
 ******************************************************************************/
class FileService {
public:
    /**********************************************************
     * @Description Doc danh sach quan tri vien tu tap tin data/Admin.txt
     * @param listAdmins Tham chieu danh sach de nap du lieu vao RAM
     * @return true neu doc thanh cong, false neu file khong mo duoc
     **********************************************************/
    static bool loadAdmins(LinkedList<Admin>& listAdmins);

    /**********************************************************
     * @Description Doc danh sach ma so the dang bi khoa tu data/KhoaThe.txt
     * @param listLockedIds Tham chieu danh sach ID bi khoa
     * @return true neu doc thanh cong, false neu khong mo duoc file
     **********************************************************/
    static bool loadLockedIds(LinkedList<std::string>& listLockedIds);

    /**********************************************************
     * @Description Doc toan bo the tu tu data/TheTu.txt va doi chieu voi
     * danh sach the khoa de thiet lap co _bIsLocked cho Card
     * @param listCards Tham chieu danh sach the tu nap vao RAM
     * @param listLockedIds Danh sach cac ID dang bi khoa
     * @return true neu doc thanh cong, false neu gap loi
     **********************************************************/
    static bool loadCards(LinkedList<Card>& listCards,
                          const LinkedList<std::string>& listLockedIds);

    /**********************************************************
     * @Description Ghi de lai toan bo the tu vao data/TheTu.txt
     * @param listCards Danh sach the tu can luu
     * @return true neu ghi thanh cong
     **********************************************************/
    static bool saveCards(const LinkedList<Card>& listCards);

    /**********************************************************
     * @Description Cap nhat ma PIN moi cho the truc tiep tren dia
     * ma khong can cho den khi nguoi dung dang xuat
     * @param strId Ma so the can doi PIN
     * @param strNewPin Ma PIN moi gom 6 chu so
     * @return true neu cap nhat thanh cong
     **********************************************************/
    static bool updateCardPin(const std::string& strId, const std::string& strNewPin);

    /**********************************************************
     * @Description Ghi de lai danh sach the bi khoa vao data/KhoaThe.txt
     * @param listLockedIds Danh sach cac ma so ID the bi khoa
     * @return true neu ghi thanh cong
     **********************************************************/
    static bool saveLockedIds(const LinkedList<std::string>& listLockedIds);

    /**********************************************************
     * @Description Ghi noi them 1 ma ID the bi khoa vao data/KhoaThe.txt
     * (Giai quyet triet de Loi #3 khi khoa the tu dong sau 3 lan sai PIN)
     * @param strId Ma so the can ghi vao danh sach khoa
     * @return true neu ghi thanh cong
     **********************************************************/
    static bool appendLockedCard(const std::string& strId);

    /**********************************************************
     * @Description Doc thong tin chi tiet tai khoan tu tap tin data/[ID].txt
     * Xu ly doc theo dong (getline) de giu nguyen ho ten co khoang trang
     * @param strId Ma so tai khoan can doc (14 so)
     * @param acc Tham chieu doi tuong Account de nap du lieu vao
     * @return ErrorCode (ERR_NONE neu thanh cong, ERR_FILE_NOT_FOUND neu file khong ton tai)
     **********************************************************/
    static ErrorCode loadAccount(const std::string& strId, Account& acc);

    /**********************************************************
     * @Description Ghi de thong tin tai khoan gom 4 dong vao data/[ID].txt
     * @param acc Doi tuong Account can luu
     * @return true neu luu thanh cong
     **********************************************************/
    static bool saveAccount(const Account& acc);

    /**********************************************************
     * @Description Xoa tap tin thong tin tai khoan data/[ID].txt tren dia
     * @param strId Ma so tai khoan can xoa
     * @return true neu xoa thanh cong
     **********************************************************/
    static bool deleteAccountFile(const std::string& strId);

    /**********************************************************
     * @Description Khoi tao ca 2 tap tin data/[ID].txt va data/LichSu[ID].txt
     * khi Admin them the tu moi vao he thong
     * @param strId Ma so the 14 so
     * @param strName Ho ten chu tai khoan
     * @param lInitialBalance So du ban dau
     * @param strCurrency Don vi tien te (mac dinh "VND")
     * @return true neu tao thanh cong ca 2 tap tin
     **********************************************************/
    static bool createAccountFiles(const std::string& strId,
                                  const std::string& strName,
                                  long lInitialBalance,
                                  const std::string& strCurrency = "VND");

    /**********************************************************
     * @Description Ghi noi mot giao dich moi vao data/LichSu[ID].txt
     * @param strId Ma so tai khoan
     * @param trans Doi tuong Transaction chua thong tin giao dich
     * @return true neu ghi thanh cong
     **********************************************************/
    static bool appendTransaction(const std::string& strId, const Transaction& trans);

    /**********************************************************
     * @Description Doc toan bo lich su giao dich tu data/LichSu[ID].txt
     * @param strId Ma so tai khoan
     * @param listTrans Danh sach de nap cac giao dich vao RAM
     * @return true neu doc thanh cong
     **********************************************************/
    static bool loadTransactions(const std::string& strId, LinkedList<Transaction>& listTrans);

    /**********************************************************
     * @Description Tu dong khoi tao thu muc data/ va sinh du lieu mau
     * gom 3 Admin, 10 The tu kem theo cac tap tin tai khoan tuong ung
     * neu he thong chay lan dau tien hoac thieu du lieu (Auto-Recovery)
     * @return void
     **********************************************************/
    static void initSampleData();

    /**********************************************************
     * @Description Ghi noi nhat ky kiem toan quan tri vao data/AdminLog.txt
     * Format: <Timestamp>|<Action>|<Detail>
     * @param strAction Hanh dong (ADD_CARD, DELETE_CARD, UNLOCK_CARD, v.v.)
     * @param strDetail Chi tiet hanh dong
     * @return true neu ghi thanh cong
     **********************************************************/
    static bool appendAdminLog(const std::string& strAction, const std::string& strDetail);

    /**********************************************************
     * @Description Luu tru tap tin lich su LichSu[ID].txt thanh file .bak khi xoa the
     * @param strId Ma so tai khoan
     * @return true neu luu tru thanh cong
     **********************************************************/
    static bool archiveHistoryFile(const std::string& strId);

    /**********************************************************
     * @Description Lay so lan dang nhap sai ben vung tu data/FailedAttempts.txt
     * @param strId Ma so the
     * @return So lan dang nhap sai
     **********************************************************/
    static int getFailedAttempts(const std::string& strId);

    /**********************************************************
     * @Description Ghi nhan 1 lan dang nhap sai ben vung vao data/FailedAttempts.txt
     * @param strId Ma so the
     * @return So lan sai moi
     **********************************************************/
    static int recordFailedAttempt(const std::string& strId);

    /**********************************************************
     * @Description Xoa bo dem dang nhap sai cua the khoi data/FailedAttempts.txt
     * @param strId Ma so the
     * @return true neu reset thanh cong
     **********************************************************/
    static bool resetFailedAttempts(const std::string& strId);
};

#endif // FILESERVICE_H_INCLUDED_
