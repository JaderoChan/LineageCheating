#include "progress.hpp"

Progress Progress::fromString(const std::string& text)
{
    std::string current, total;

    bool isCurrentPart = true;
    for (char ch : text)
    {
        std::string& handledStr = isCurrentPart ? current : total;
        if (std::isdigit(ch))
            handledStr += ch;
        else if (ch == '/' && isCurrentPart)
            isCurrentPart = false;
        else if (std::isblank(ch))
            continue;
        else if (!isCurrentPart)
            break;
    }

    Progress progress;
    progress.current = std::atoi(current.c_str());
    progress.total = std::atoi(total.c_str());

    return progress;
}

std::string Progress::toString() const
{
    return std::to_string(current) + "/" + std::to_string(total);
}
