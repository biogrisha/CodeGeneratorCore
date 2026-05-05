#pragma once
#include "Structs.h"
#include <memory>

class IR
{
public:
	void init(const std::map<std::string, std::unique_ptr<Block>>& blocks);
private:
	void parse(Block* block);
	void unfoldGain(Block* block);
	void unfoldSum(Block* block, int i);
	void unfoldDelay(Block* block);
	void unfoldOutport(Block* block);
	std::map<std::string, std::unique_ptr<Term>> egraph;
	std::string ir;
};