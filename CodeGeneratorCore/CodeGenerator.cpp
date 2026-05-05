#include "CodeGenerator.h"
#include "Helpers.h"
#include <iostream>

void CodeGenerator::generate(IR& ir)
{
	Term* prog = ir.getMainTerm();

	std::string code;
	helpers::printTerm(prog, code);
	std::cout << "struct {";
	

	std::cout << code;
}

