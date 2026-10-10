#include "SecurityService.h"
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstring>

namespace {
    // 4 ham phi tuyen co ban cua MD5
    inline uint32_t F(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (~x & z); }
    inline uint32_t G(uint32_t x, uint32_t y, uint32_t z) { return (x & z) | (y & ~z); }
    inline uint32_t H(uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; }
    inline uint32_t I(uint32_t x, uint32_t y, uint32_t z) { return y ^ (x | ~z); }

    // Phep xoay trai bit
    inline uint32_t rotateLeft(uint32_t x, uint32_t n) {
        return (x << n) | (x >> (32 - n));
    }

    // Bang hang so sin K[i] = floor(abs(sin(i + 1)) * 2^32)
    const uint32_t K[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };

    // So buoc dich bit cho moi vong
    const uint32_t S[64] = {
        7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
        5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
        4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
        6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
    };

    const char* const PIN_SALT = "ATM_SECURE_SALT_2026_";
    const char* const PASS_SALT = "ADMIN_SECURE_SALT_v2_";
}

std::string SecurityService::md5(const std::string& strInput) {
    // 1. Khoi tao trang thai ban dau
    uint32_t a0 = 0x67452301;
    uint32_t b0 = 0xefcdab89;
    uint32_t c0 = 0x98badcfe;
    uint32_t d0 = 0x10325476;

    // 2. Padding thong diep
    uint64_t initialLen = strInput.length();
    uint64_t initialBitLen = initialLen * 8;

    std::vector<uint8_t> msg(strInput.begin(), strInput.end());
    msg.push_back(0x80); // Them bit 1

    while ((msg.size() % 64) != 56) {
        msg.push_back(0x00);
    }

    // Ghi do dai 64-bit little endian
    for (int i = 0; i < 8; ++i) {
        msg.push_back(static_cast<uint8_t>((initialBitLen >> (i * 8)) & 0xFF));
    }

    // 3. Xu ly tung khoi 512-bit (64 bytes)
    for (size_t offset = 0; offset < msg.size(); offset += 64) {
        uint32_t M[16];
        for (int i = 0; i < 16; ++i) {
            M[i] = static_cast<uint32_t>(msg[offset + i * 4]) |
                   (static_cast<uint32_t>(msg[offset + i * 4 + 1]) << 8) |
                   (static_cast<uint32_t>(msg[offset + i * 4 + 2]) << 16) |
                   (static_cast<uint32_t>(msg[offset + i * 4 + 3]) << 24);
        }

        uint32_t A = a0;
        uint32_t B = b0;
        uint32_t C = c0;
        uint32_t D = d0;

        for (int i = 0; i < 64; ++i) {
            uint32_t f = 0;
            uint32_t g = 0;

            if (i < 16) {
                f = F(B, C, D);
                g = i;
            } else if (i < 32) {
                f = G(B, C, D);
                g = (5 * i + 1) % 16;
            } else if (i < 48) {
                f = H(B, C, D);
                g = (3 * i + 5) % 16;
            } else {
                f = I(B, C, D);
                g = (7 * i) % 16;
            }

            uint32_t temp = D;
            D = C;
            C = B;
            B = B + rotateLeft(A + f + K[i] + M[g], S[i]);
            A = temp;
        }

        a0 += A;
        b0 += B;
        c0 += C;
        d0 += D;
    }

    // 4. Xuat chuoi hex 32 ky tu
    uint32_t digest[4] = {a0, b0, c0, d0};
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < 4; ++i) {
        for (int b = 0; b < 4; ++b) {
            oss << std::setw(2) << static_cast<int>((digest[i] >> (b * 8)) & 0xFF);
        }
    }

    return oss.str();
}

std::string SecurityService::hashPin(const std::string& strPin) {
    return md5(PIN_SALT + strPin);
}

std::string SecurityService::hashPassword(const std::string& strPassword) {
    return md5(PASS_SALT + strPassword);
}

#include <cctype>

bool SecurityService::verifyHash(const std::string& strRaw, const std::string& strStored) {
    if (strRaw.empty() || strStored.empty()) {
        return false;
    }

    bool bStoredIsHex32 = (strStored.length() == 32);
    if (bStoredIsHex32) {
        for (char c : strStored) {
            if (!std::isxdigit(static_cast<unsigned char>(c))) {
                bStoredIsHex32 = false;
                break;
            }
        }
    }

    // Neu stored la chuoi hex 32 ky tu (hash MD5)
    if (bStoredIsHex32) {
        // 1. Kiem tra MD5 salted PIN
        if (strStored == hashPin(strRaw)) {
            return true;
        }

        // 2. Kiem tra MD5 salted Password
        if (strStored == hashPassword(strRaw)) {
            return true;
        }

        // 3. Kiem tra MD5 raw
        if (strStored == md5(strRaw)) {
            return true;
        }

        return false;
    }

    // Neu khong phai chuoi hash 32 hex: so sanh truc tiep voi plaintext legacy
    return (strRaw == strStored);
}
