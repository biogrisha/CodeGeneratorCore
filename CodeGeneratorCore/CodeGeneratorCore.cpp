#include "Parser.h"
#include <iostream>
int main()
{
    Parser pr;
    auto res = pr.parse("C:/Users/biogr/OneDrive/Документы/MATLAB/untitled1.xml");
    if (res.has_value())
    {
        auto& blocks = *res;
        for (auto& bl : blocks)
        {
            std::cout << bl.first;
        }
    }
    return 0;
}