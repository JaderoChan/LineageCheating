#include "progress.hpp"

Progress Progress::fromString(const std::string& text)
{
    std::string current, total;

    bool isCurrentPart = true;
    for (const auto& ch : text)
    {
        std::string& handledStr = isCurrentPart ? current : total;
        if (isdigit(ch))
            handledStr += ch;
        else if (ch == '/' && isCurrentPart)
            isCurrentPart = false;
        else if (!isCurrentPart)
            break;
    }

    Progress progress;
    progress.current = std::atoi(current.c_str());
    progress.total = std::atoi(total.c_str());

    return progress;
}

std::ostream& operator<<(std::ostream& os, const Progress& progress)
{
    os << progress.current << "/" << progress.total;
    return os;
}
