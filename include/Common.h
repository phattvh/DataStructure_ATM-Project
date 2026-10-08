#ifndef COMMON_H_INCLUDED_
#define COMMON_H_INCLUDED_

#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

/**********************************************************
 * @Description: Cac hang so he thong ATM tuan thu quy tac UPPERCASE
 * (Rule 4 trong C++ Coding Standard Version 2)
 * Su dung inline (C++17) de tranh khoi tao trung lap giua cac TU
 **********************************************************/
inline const std::string DEFAULT_PIN = "123456";
inline constexpr long MIN_TRANSACTION = 50000;
inline constexpr long MIN_BALANCE_RESERVE = 50000;
inline constexpr int MAX_FAILED_LOGINS = 3;
inline constexpr int ID_LENGTH = 14;
inline constexpr int PIN_LENGTH = 6;
inline const std::string DATA_DIR = "data/";

/**********************************************************
 * @Description: Loai giao dich ngan hang
 **********************************************************/
enum TransactionType {
    WITHDRAW = 1,
    TRANSFER = 2,
    RECEIVE = 3
};

/**********************************************************
 * @Description: Vai tro nguoi dung trong phien lam viec
 **********************************************************/
enum UserRole {
    ROLE_NONE = 0,
    ROLE_ADMIN = 1,
    ROLE_USER = 2
};

/**********************************************************
 * @Description: Cac ma loi tra ve tu nghiep vu tai chinh & he thong
 **********************************************************/
enum ErrorCode {
    ERR_NONE = 0,
    ERR_INVALID_AMOUNT = 1,
    ERR_NOT_MULTIPLE = 2,
    ERR_INSUFFICIENT_FUNDS = 3,
    ERR_FILE_NOT_FOUND = 4,
    ERR_CARD_LOCKED = 5,
    ERR_ID_EXISTS = 6,
    ERR_ID_NOT_FOUND = 7,
    ERR_RECIPIENT_NOT_FOUND = ERR_ID_NOT_FOUND,
    ERR_INVALID_FORMAT = 8,
    ERR_SAME_ACCOUNT = 9,
    ERR_SYSTEM_OVERFLOW = 10
};

/**********************************************************
 * @Description: Ham tien ich lay thoi gian he thong hien tai
 * dinh dang YYYY-MM-DD HH:MM:SS (C++17 inline)
 **********************************************************/
inline std::string getNowTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t tNow = std::chrono::system_clock::to_time_t(now);
    std::tm tmNow;
#ifdef _WIN32
    localtime_s(&tmNow, &tNow);
#else
    localtime_r(&tNow, &tmNow);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmNow, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

#endif // COMMON_H_INCLUDED_
