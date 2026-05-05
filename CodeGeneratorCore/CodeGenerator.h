#pragma once
#include "Structs.h"
#include "IR.h"
#include <memory>
#include <set>

class CodeGenerator
{
public:
	void generate(IR& ir);
};