#pragma once

#include <string>
#include <sstream>

inline std::string formatString(const std::string& fmtStr)
{
    return fmtStr;
}

template <typename T>
std::string formatString(const std::string& fmtStr, const T& arg)
{
    std::stringstream ss;

    if (fmtStr.size() < 4)
    {
        size_t pos = fmtStr.find("{}");
        if (pos == std::string::npos)
            return fmtStr;

        ss << fmtStr.substr(0, pos);
        ss << arg;

        return ss.str() + fmtStr.substr(pos + 2);
    }

    std::string window(4, '\0');
    for (size_t i = 0; i < fmtStr.size();)
    {
        window[0] = fmtStr[i];
        window[1] = i < fmtStr.size() - 1 ? fmtStr[i + 1] : '\0';
        window[2] = i < fmtStr.size() - 2 ? fmtStr[i + 2] : '\0';
        window[3] = i < fmtStr.size() - 3 ? fmtStr[i + 3] : '\0';

        if (window == "{{}}")
        {
            ss << "{}";
            i += 4;
            continue;
        }

        if (window[0] == '{' && window[1] == '}')
        {
            ss << arg;
            return ss.str() + fmtStr.substr(i + 2);
        }
        else
        {
            ss << window[0];
            i += 1;
            continue;
        }
    }

    return ss.str();
}

template <typename T, typename... Args>
std::string formatString(const std::string& fmtStr, const T& arg, Args&&... args)
{
    std::stringstream ss;

    if (fmtStr.size() < 4)
    {
        size_t pos = fmtStr.find("{}");
        if (pos == std::string::npos)
            return fmtStr;

        ss << fmtStr.substr(0, pos);
        ss << arg;

        return ss.str() + fmtStr.substr(pos + 2);
    }

    std::string window(4, '\0');
    for (size_t i = 0; i < fmtStr.size();)
    {
        window[0] = fmtStr[i];
        window[1] = i < fmtStr.size() - 1 ? fmtStr[i + 1] : '\0';
        window[2] = i < fmtStr.size() - 2 ? fmtStr[i + 2] : '\0';
        window[3] = i < fmtStr.size() - 3 ? fmtStr[i + 3] : '\0';

        if (window == "{{}}")
        {
            ss << "{}";
            i += 4;
            continue;
        }

        if (window[0] == '{' && window[1] == '}')
        {
            ss << arg;
            return ss.str() + formatString(fmtStr.substr(i + 2), std::forward<Args>(args)...);
        }
        else
        {
            ss << window[0];
            i += 1;
            continue;
        }
    }

    return ss.str();
}
