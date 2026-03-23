#include "monster_name.hpp"

#include <vector>
#include <unordered_set>

// Determine whether a Unicode code point is a CJK character (including Simplified/Traditional).
static bool isCJKUnifiedIdeograph(char32_t codepoint)
{
    if (codepoint >= 0x4E00 && codepoint <= 0x9FFF) return true;
    if (codepoint >= 0x3400 && codepoint <= 0x4DBF) return true;
    if (codepoint >= 0x20000 && codepoint <= 0x2A6DF) return true;
    if (codepoint >= 0x2A700 && codepoint <= 0x2B73F) return true;
    if (codepoint >= 0x2B740 && codepoint <= 0x2B81F) return true;
    if (codepoint >= 0x2B820 && codepoint <= 0x2CEAF) return true;
    if (codepoint >= 0x2CEB0 && codepoint <= 0x2EBEF) return true;
    if (codepoint >= 0xF900 && codepoint <= 0xFAFF) return true;
    if (codepoint >= 0x2F800 && codepoint <= 0x2FA1F) return true;

    return false;
}

// Simple UTF-8 decoding and check if it contains Chinese characters
static bool containsChinese(const std::string& utf8)
{
    const unsigned char* s = reinterpret_cast<const unsigned char*>(utf8.data());
    size_t len = utf8.size();
    size_t i = 0;

    while (i < len)
    {
        unsigned char c = s[i];
        char32_t codepoint = 0;
        size_t extra = 0;

        if (c <= 0x7F)
        { // 1-byte ASCII
            codepoint = c;
            extra = 0;
        } else if ((c & 0xE0) == 0xC0)
        { // 2-byte
            if (i + 1 >= len) break;  // Incomplete UTF-8, directly terminated
            codepoint = (c & 0x1F) << 6 |
                        (s[i + 1] & 0x3F);
            extra = 1;
        } else if ((c & 0xF0) == 0xE0)
        { // 3-byte
            if (i + 2 >= len) break;
            codepoint = (c & 0x0F) << 12 |
                        (s[i + 1] & 0x3F) << 6 |
                        (s[i + 2] & 0x3F);
            extra = 2;
        } else if ((c & 0xF8) == 0xF0)
        { // 4-byte
            if (i + 3 >= len) break;
            codepoint = (c & 0x07) << 18 |
                        (s[i + 1] & 0x3F) << 12 |
                        (s[i + 2] & 0x3F) << 6 |
                        (s[i + 3] & 0x3F);
            extra = 3;
        } else
        {
            // Illegal or unsupported prefix, skipping this byte
            ++i;
            continue;
        }

        if (isCJKUnifiedIdeograph(codepoint))
        {
            return true; // Find any Chinese character.
        }

        i += 1 + extra;
    }
    return false;
}

// 从 UTF-8 字符串中提取每个 Unicode 字符（以 std::string 形式）
std::vector<std::string> splitUtf8(const std::string& str)
{
    std::vector<std::string> chars;
    size_t i = 0;
    while (i < str.size())
    {
        int len = 1;
        unsigned char c = static_cast<unsigned char>(str[i]);
        if (c >= 0xF0)      len = 4;
        else if (c >= 0xE0) len = 3;
        else if (c >= 0xC0) len = 2;
        chars.push_back(str.substr(i, len));
        i += len;
    }
    return chars;
}

// 判断 text 中是否包含 charSet 里的任意一个 UTF-8 字符
bool containsAnyChar(const std::unordered_set<std::string>& charSet, const std::string& text)
{
    std::vector<std::string> textChars = splitUtf8(text);
    for (const auto& ch : textChars)
    {
        if (charSet.count(ch))
            return true;
    }
    return false;
}

std::unordered_set<std::string> getCharSet()
{
    auto tableChars = splitUtf8(
        "蛇女妖灵怪熊骷髅猪黑犬烈焰头鸟莱克龙王亡精兵手史托枪卫骑士蜘蛛瓦野"
    );
    std::unordered_set<std::string> charSet(tableChars.begin(), tableChars.end());
    return charSet;
}

bool isMonsterName(const std::string& text)
{
    static auto charSet = getCharSet();
    return containsAnyChar(charSet, text);
}
