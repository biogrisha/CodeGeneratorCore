#include "Parser.h"
#include "IR.h"
#include "CodeGenerator.h"
#include <iostream>
int main()
{
    Parser pr;
    auto res = pr.parse("C:/Users/biogr/OneDrive/Документы/MATLAB/test.xml");

    IR ir;
    ir.setStructName("nwocg");
    if (!ir.init(*res))
    {
        return 0;
    }

    CodeGenerator cg;
    cg.generate(ir);
    return 0;
}