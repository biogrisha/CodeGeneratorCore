#include "Parser.h"
#include "IR.h"
#include <iostream>
int main()
{
    Parser pr;
    auto res = pr.parse("C:/Users/biogr/OneDrive/Документы/MATLAB/untitled1.xml");

    IR ir;
    ir.init(*res);
    return 0;
}