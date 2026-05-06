#pragma once
#include "Structs.h"

class TermParser
{
public:
    TermParser(const std::string& str)
        : m_str(str)
    {
    }

    void parse();
    Term* getResult() { return m_current_term; }
private:
    void consumeTermName();

    Term* m_current_term = nullptr;
    Term* m_parent_term = nullptr;
    const std::string& m_str;
    int m_pos = 0;
};