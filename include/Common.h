#ifndef COMMON_H_INCLUDED_
#define COMMON_H_INCLUDED_

#include <string>

/**
 * @Description: Cac hang so he thong ATM tuan thu quy tac UPPERCASE
 */
const std::string DEFAULT_PIN = "123456";
const long MIN_TRANSACTION = 50000;
const long MIN_BALANCE_RESERVE = 50000;
const int MAX_FAILED_LOGINS = 3;
const int ID_LENGTH = 14;
const int PIN_LENGTH = 6;
const std::string DATA_DIR = "data/";

/**
 * @Description: Loai giao dich ngan hang
 */
enum TransactionType {
    WITHDRAW = 1,
    TRANSFER = 2,
    RECEIVE = 3
};

/**
 * @Description: Vai tro nguoi dung trong he thong
 */
enum UserRole {
    ROLE_NONE = 0,
    ROLE_ADMIN = 1,
    ROLE_USER = 2
};

/**
 * @Description: Ma loi tra ve tu cac nghiep vu
 */
enum ErrorCode {
    ERR_NONE = 0,
    ERR_INVALID_AMOUNT = 1,
    ERR_NOT_MULTIPLE = 2,
    ERR_INSUFFICIENT_FUNDS = 3,
    ERR_FILE_NOT_FOUND = 4,
    ERR_CARD_LOCKED = 5,
    ERR_ID_EXISTS = 6,
    ERR_ID_NOT_FOUND = 7,
    ERR_INVALID_FORMAT = 8,
    ERR_SAME_ACCOUNT = 9
};

#endif // COMMON_H_INCLUDED_
