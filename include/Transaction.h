#ifndef TRANSACTION_H_INCLUDED_
#define TRANSACTION_H_INCLUDED_

#include <string>
#include "Common.h"

/******************************************************************************
 * @Description: Lop dai dien cho Thuc the Giao dich tai chinh (Transaction)
 * Luu vet chi tiet cac giao dich Rut tien, Chuyen tien, Nhan tien
 * (Tuan thu C++ Coding Standard V2)
 ******************************************************************************/
class Transaction {
private:
    std::string _strId;
    TransactionType _eType;
    long _lAmount;
    std::string _strTimestamp;
    std::string _strDetail;

public:
    /**********************************************************
     * @Description Constructor mac dinh
     **********************************************************/
    Transaction();

    /**********************************************************
     * @Description Constructor khoi tao co day du tham so
     * @param strId Ma so tai khoan thuc hien giao dich
     * @param eType Loai giao dich (WITHDRAW, TRANSFER, RECEIVE)
     * @param lAmount So tien giao dich (VND)
     * @param strTimestamp Thoi gian thuc hien (YYYY-MM-DD HH:MM:SS)
     * @param strDetail Chi tiet mo ta giao dich
     **********************************************************/
    Transaction(const std::string& strId,
                TransactionType eType,
                long lAmount,
                const std::string& strTimestamp,
                const std::string& strDetail);

    // Getters
    std::string getId() const;
    TransactionType getType() const;
    std::string getTypeName() const;
    long getAmount() const;
    std::string getTimestamp() const;
    std::string getDetail() const;

    /**********************************************************
     * @Description Dinh dang dong du lieu chuan de ghi vao tap tin LichSu[ID].txt
     * @return Chuoi dong du lieu
     **********************************************************/
    std::string formatForFile() const;

    /**********************************************************
     * @Description Dinh dang hien thi dep mat tren man hinh Console
     * @return Chuoi hien thi
     **********************************************************/
    std::string toString() const;

    /**********************************************************
     * @Description Ham tien ich phan tich mot dong van ban tu file thanh doi tuong
     * @param strId Ma so tai khoan
     * @param strLine Dong van ban tu file LichSu[ID].txt
     * @return Doi tuong Transaction
     **********************************************************/
    static Transaction parseFromFileLine(const std::string& strId, const std::string& strLine);

    /**********************************************************
     * @Description Ham tien ich tinh lay thoi gian thuc he thong (YYYY-MM-DD HH:MM:SS)
     * @return Chuoi thoi gian hien tai
     **********************************************************/
    static std::string getCurrentTimestamp();
};

#endif // TRANSACTION_H_INCLUDED_
