#include "CodeGenerator.h"
#include "Helpers.h"
#include <iostream>
#include <fstream>

void CodeGenerator::generate(IR& ir)
{
    const std::string filename = "generated.cpp"; // <-- you can change this

    std::ofstream out(filename);
    if (!out)
    {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }
    
    generateHeader(out);

    auto& ports = ir.getPorts();
    auto& delays = ir.getDelays();
    auto& vars = ir.getLocalVars();

    out << "struct {\n";

    for (const auto& port : ports)
    {
        Symbol* s = ir.getSymbol(port->label);
        if(s)
        {
            out << "    double " << s->name << ";\n";
        }
    }

    for (const auto& del : delays)
    {
        Symbol* s = ir.getSymbol(del->label);
        if (s)
        {
            out << "    double " << s->name << ";\n";
        }
    }

    out << "} nwocg; \n";

    out << "void nwocg_generated_init()\n";
    out << "{\n";

    for (const auto& del : delays)
    {
        if (auto sym = ir.getSymbol(del->label))
        {
            out << "    " << del->label << " = " << sym->val << ";\n";
        }
    }

    out << "}\n";

    out << "void nwocg_generated_step()\n";
    out << "{\n";

    for (const auto& var : vars)
    {
        if (auto sym = ir.getSymbol(var->label))
        {
            out << "    double " << var->label << " = " << sym->val << ";\n";
        }
    }

    for (auto ch : ir.getMainTerm()->children)
    {
        std::string value;
        Symbol* s = ir.getSymbol(ch->children[0]->label);
        if (s)
        {
            helpers::printTerm(ch->children[1], value);

            out << "    double " << s->name << "tmp = " << value << ";\n";
        }
    }

    for (auto ch : ir.getMainTerm()->children)
    {
        Symbol* s = ir.getSymbol(ch->children[0]->label);
        if (s)
        {
            out << "    " << ch->children[0]->label << " = " << s->name << "tmp;\n";
        }
    }
    out << "}\n";

    out << "static const nwocg_ExtPort\n";
    out << "    ext_ports[] =\n";
    out << "    {\n";

    for (const auto& port : ports)
    {
        Symbol* s = ir.getSymbol(port->label);
        if (s)
        {
            out << "        {\"" + s->name << "\"," << port->label << "," << (s->type == SymbolType::Inport ? "1" : "0") << "},\n";
        }
    }
    out << "        { 0, 0, 0 },\n";
    out << "    };\n";


    out << R"(
const nwocg_ExtPort * const
    nwocg_generated_ext_ports = ext_ports;
const size_t
    nwocg_generated_ext_ports_size = sizeof(ext_ports);)";

    out.close(); // optional, will close automatically
}

void CodeGenerator::generateHeader(std::ofstream& out)
{
    out << R"(
double add(double a, double b){ return a + b};
double mul(double a, double b){ return a * b};
double neg(double a){ return -a};

)";
}

