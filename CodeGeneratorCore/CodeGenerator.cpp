#include "CodeGenerator.h"
#include "Helpers.h"
#include <iostream>

void CodeGenerator::generate(IR& ir)
{
	auto& ports = ir.getPorts();
	auto& delays = ir.getDelays();
	auto& vars = ir.getLocalVars();
	std::cout << "struct {\n";
	for (auto port : ports)
	{
		std::cout << "    double " << port->label << ";\n";
	}
	for (auto del : delays)
	{
		std::cout << "    double " << del->label << ";\n";
	}
	std::cout << "} nwocg; \n";
	std::cout << "void nwocg_generated_init()\n";
	std::cout << "{\n";
	for (auto del : delays)
	{
		if (auto sym = ir.getSymbol(del->label))
		{
			std::cout <<"    " << del->label << " = " << sym->val << ";\n";
		}

	}
	
	std::cout << "}\n";
	std::cout << "void nwocg_generated_step()\n";
	std::cout << "{\n";
	for (auto& var : vars)
	{
		if (auto sym = ir.getSymbol(var->label))
		{
			std::cout << "    double " << var->label << " = " << sym->val << ";\n";
		}

	}
	for (auto ch : ir.getMainTerm()->children)
	{
		std::string code;
		helpers::printTerm(ch, code);
		std::cout << "    " << code << ";\n";
	}
	
	std::cout << "}\n";


}

