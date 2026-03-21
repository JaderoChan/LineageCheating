#pragma once

#include <string>

class Progress
{
public:
    Progress() = default;
    Progress(int current, int total) : current(current), total(total) {}

    static Progress fromString(const std::string& text);
    std::string toString() const;

    double getPercentage() const { return total == 0 ? 0 : static_cast<double>(current) / total; }
    bool isValid() const { return current <= total; }

    int current = 0;
    int total = 0;
};
