#pragma once
#include <vector>
#include <string>

namespace helpers
{
    std::vector<std::string> split(const std::string& s)
    {
        std::vector<std::string> result;

        size_t start = 0;
        size_t pos;

        while ((pos = s.find(',', start)) != std::string::npos)
        {
            result.emplace_back(s.substr(start, pos - start));
            start = pos + 1;
        }

        result.emplace_back(s.substr(start)); // last part

        return result;
    }
}