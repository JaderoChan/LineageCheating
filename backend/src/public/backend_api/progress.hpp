#pragma once

#include <ostream>
#include <string>

class Progress
{
public:
    Progress() = default;
    Progress(int current, int total) : current(current), total(total) {}

    static Progress fromString(const std::string& text);

    double progress() const { return total == 0 ? 0 : static_cast<double>(current) / total; }
    bool isValid() const { return current <= total; }

    friend std::ostream& operator<<(std::ostream& os, const Progress& progress);

    int current = 0;
    int total = 0;
};
