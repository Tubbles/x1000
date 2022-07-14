#pragma once

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <locale>
#include <string>
#include <vector>

#define ARR_LEN(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

inline void u8_ser(uint8_t *buf, uint8_t data) {
    buf[0] = data;
}

inline void u16_ser(uint8_t *buf, uint16_t data) {
    buf[1] = data >> 8;
    buf[0] = data;
}

inline void u32_ser(uint8_t *buf, uint32_t data) {
    buf[3] = data >> 24;
    buf[2] = data >> 16;
    buf[1] = data >> 8;
    buf[0] = data;
}

inline uint8_t u8_des(uint8_t *buf) {
    return buf[0];
}

inline uint16_t u16_des(uint8_t *buf) {
    return (((uint16_t)buf[1]) << 8) | ((uint16_t)buf[0]);
}

inline uint32_t u32_des(uint8_t *buf) {
    return (((uint32_t)buf[3]) << 24) | (((uint32_t)buf[2]) << 16) | (((uint32_t)buf[1]) << 8) | ((uint32_t)buf[0]);
}

inline std::string &ltrim(std::string &str) {
    auto it2 = std::find_if(str.begin(), str.end(), [](char ch) {
        return !std::isspace<char>(ch, std::locale::classic());
    });
    str.erase(str.begin(), it2);
    return str;
}

inline std::string &rtrim(std::string &str) {
    auto it1 = std::find_if(str.rbegin(), str.rend(), [](char ch) {
        return !std::isspace<char>(ch, std::locale::classic());
    });
    str.erase(it1.base(), str.end());
    return str;
}

inline std::string &trim(std::string &str) {
    return ltrim(rtrim(str));
}

inline std::vector<uint8_t> binary_file_to_vector(const std::string &filename) {
    // open the file:
    std::ifstream file(filename, std::ios::binary);

    // read the data:
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}
