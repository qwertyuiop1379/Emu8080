#pragma once

#include <string>

inline uint8_t ExtractBits8(uint8_t n, uint8_t pos, uint8_t count) { return (((1 << count) - 1) & (n >> (pos - 1))); }

std::string FormatString(const std::string fmt_str, ...);
std::string HexString(const uint8_t *data, int len);
bool IsValidHexString(const std::string &str);
uint64_t UIntFromHex(const std::string &hex);
std::string LowercaseString(std::string str);
std::string ReplaceSubstring(const std::string &str, const std::string &find, const std::string &replace);
std::string TrimSurroundingWhitespace(const std::string &str);
std::string RemoveWhitespaceFromString(const std::string &str);