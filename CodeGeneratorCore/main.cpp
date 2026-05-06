#include "Parser.h"
#include "IR.h"
#include "CodeGenerator.h"
#include <iostream>
int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: CodeGeneratorCore <input.xml> <output.cpp>\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];

    Parser pr;
    auto res = pr.parse(input_file);
    if (!res.has_value())
    {
        return 0;
    }
    IR ir;
    ir.setStructName("nwocg");
    if (!ir.init(*res))
    {
        return 0;
    }

    CodeGenerator cg;
    cg.setOutFile(output_file);
    cg.generate(ir);
    return 0;
}