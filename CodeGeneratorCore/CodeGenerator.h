#pragma once
#include "Structs.h"
#include "IR.h"
#include <memory>
#include <set>

class CodeGenerator
{
public:
	void generate(IR& ir);
	void setOutFile(const std::string& file_path);
private:
	void generateHeader(std::ofstream& out);
	std::string m_file_path;
};