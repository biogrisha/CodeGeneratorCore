#pragma once
#include <vector>
#include <string>

namespace helpers
{
    inline std::vector<std::string> split(const std::string& s)
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

    inline Term* find(Term* t)
    {
        while (t->e_rep != t)
        {
            t = t->e_rep;
        }
        return t;
    }

    inline void printTerm(Term* pat, std::string& res)
    {
        res += pat->label;
        if (!pat->children.empty())
        {
            res += '(';
            for (auto& ch : pat->children)
            {
                printTerm(ch, res);
                res += ',';
            }
            res.back() = ')';
        }
    }

    inline std::string removeWhitespace(std::string s)
    {
        s.erase(std::remove_if(s.begin(), s.end(),
            [](unsigned char c) { return std::isspace(c); }),
            s.end());
        return s;
    }
}