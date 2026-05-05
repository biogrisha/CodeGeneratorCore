#include "IR.h"
#include "Helpers.h"

void IR::init(const std::map<std::string, std::unique_ptr<Block>>& blocks)
{
	std::vector<Block*> outports;
	for (auto& bl : blocks)
	{
		if (bl.second->outdeps.empty())
		{
			outports.push_back(bl.second.get());
		}
	}


	ir += "prog(";
	for (int i = 0; i < outports.size() - 1; ++i)
	{
		if (outports[i]->type == "UnitDelayOut")
		{
			unfoldDelay(outports[i]);
		}
		else if (outports[i]->type == "Outport")
		{
			unfoldOutport(outports[i]);
		}
		ir += ',';
	}
	auto last = outports.back();
	if (last->type == "UnitDelayOut")
	{
		unfoldDelay(last);
	}
	else if (last->type == "Outport")
	{
		unfoldOutport(last);
	}
	ir += ')';
}

void IR::parse(Block* block)
{
	if (block->type == "Gain")
	{
		unfoldGain(block);
	}
	else if (block->type == "Sum")
	{
		unfoldSum(block,0);
	}
	else
	{
		ir += block->name;
	}
}

void IR::unfoldGain(Block* block)
{
	ir += "mul(";
	if (block->indeps.empty())
	{
		ir += '1,';
	}
	else
	{
		for (auto& dep : block->indeps)
		{
			parse(dep.second.block);
		}
	}
	ir += ',' + block->meta + ')';
}

void IR::unfoldSum(Block* block, int i)
{
	auto dep = block->indeps.find(i + 1);
	bool last = i >= block->meta.size() - 1;
	if (!last)
	{
		ir += "add(";
	}
	
	if (block->meta[i] == '-')
	{
		ir += "neg(";
	}
	if (dep != block->indeps.end())
	{
		parse(dep->second.block);
	}
	else
	{
		ir += '0';
	}
	if (block->meta[i] == '-')
	{
		ir += ")";
	}
	if (last)
	{
		return;
	}
	ir += ",";
	unfoldSum(block, i + 1);
	ir += ")";

}

void IR::unfoldDelay(Block* block)
{
	ir += "set(" + block->name + ',';
	for (auto& dep : block->indeps)
	{
		parse(dep.second.block);
	}
	auto meta = helpers::split(block->meta);
	ir += ',' + meta.back();
	ir += ')';
}

void IR::unfoldOutport(Block* block)
{
	ir += "set(" + block->name + ',';
	for (auto& dep : block->indeps)
	{
		parse(dep.second.block);
	}
	ir += ')';
}
