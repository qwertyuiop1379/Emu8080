#include "Util.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

std::string FormatString(const std::string fmt_str, ...)
{
    int final_n, n = ((int)fmt_str.size()) * 2;
    std::unique_ptr<char[]> formatted;
    va_list ap;

    while (1) {
        formatted.reset(new char[n]);
        strcpy(&formatted[0], fmt_str.c_str());
        va_start(ap, fmt_str);
        final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }

    return std::string(formatted.get());
}

std::string HexString(const uint8_t *data, int len)
{
     std::stringstream ss;
     ss << std::hex;

     for( int i(0) ; i < len; ++i )
         ss << std::setw(2) << std::setfill('0') << (int)data[i];

     return ss.str();
}

bool IsValidHexString(const std::string &str)
{
    return str.find_first_not_of("0123456789abcdefABCDEF", 2) == std::string::npos;
}

uint64_t UIntFromHex(const std::string &hex)
{
    if (hex.length() == 0)
        throw std::runtime_error("UIntFromHex: Hex string must not be empty.");
    
    if (IsValidHexString(hex) == false)
        throw std::runtime_error(FormatString("UIntFromHex: '%s' is an invalid hex string.", hex.c_str()));

    uint64_t x;
    std::stringstream ss;
    ss << std::hex << hex;
    ss >> x;
    return x;
}

std::string LowercaseString(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
    return str;
}

std::string ReplaceSubstring(const std::string &str, const std::string &find, const std::string &replace)
{
    std::string result;
    size_t find_len = find.size();
    size_t pos,from = 0;

    while ((pos = str.find(find, from) != std::string::npos)) {
        result.append(str, from, pos - from);
        result.append(replace);
        from = pos + find_len;
    }

    result.append(str, from, std::string::npos);
    return result;
}

std::string TrimSurroundingWhitespace(const std::string &_str)
{
    std::string str = _str;
    const char *s = str.c_str();

    while (true) {
        char c = *(s);
        if (c == ' ' || c == '\n' || c == '\t') {
            s++;
            continue;
        }
        break;
    }

    str = s;
    size_t len = str.length();
    s = str.c_str();

    while (true) {
        char c = *(s + len - 1);
        if (c == ' ' || c == '\n' || c == '\t') {
            len--;
            continue;
        }
        break;
    }

    return str.substr(0, len);
}

std::string RemoveWhitespaceFromString(const std::string &_str)
{
    std::string str = _str;
    str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
    return str;
}