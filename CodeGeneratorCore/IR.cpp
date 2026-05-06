#include "IR.h"
#include "Helpers.h"
#include <iostream>
#include "TRS.h"

bool IR::init(const std::map<std::string, std::unique_ptr<Block>>& blocks)
{
	if (checkLoops(blocks))
	{
		std::cerr << "Algebraic loop detected, generation failed";
		return false;
	}
	std::vector<Block*> outports;

	for (auto& bl : blocks)
	{
		if (bl.second->outdeps.empty() && bl.second->type == "UnitDelayOut" || bl.second->type == "Outport")
		{
			outports.push_back(bl.second.get());
		}
	}


	ir += "prog(";


	for (int i = 0; i < outports.size() - 1; ++i)
	{
		if (outports[i]->type == "UnitDelayOut")
		{
			unfoldDelayOut(outports[i]);
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
		unfoldDelayOut(last);
	}
	else if (last->type == "Outport")
	{
		unfoldOutport(last);
	}


	ir += ')';

	{
		TermParser pr(ir);
		pr.parse();
		main_t = pr.getResult();
		compact(main_t, 0, egraph);
		TRSOptimizer trs(egraph);
		trs.optimize();
	}

	{
		std::string new_prog;
		TRSHelpers::printTerm(main_t, new_prog);
		egraph.clear();
		TermParser pr(new_prog);
		pr.parse();
		main_t = pr.getResult();
		compact(main_t, 0, egraph);
		collectVars(main_t, local_variables);
	}
	return true;
}

Symbol* IR::getSymbol(const std::string& name)
{
	auto found = symbols.find(name);
	if (found == symbols.end())
	{
		return nullptr;
	}
	return &found->second;
}

void IR::setStructName(const std::string& name)
{
	struct_name = name;
}

bool IR::checkLoops(const std::map<std::string, std::unique_ptr<Block>>& blocks)
{
	std::map<Block*, int> block_rels;
	std::vector<Block*> no_rels;
	for (auto& bl : blocks)
	{
		block_rels.emplace(bl.second.get(), bl.second->outdeps.size());
		if (bl.second->outdeps.empty())
		{
			no_rels.push_back(bl.second.get());
		}
	}

	while (!no_rels.empty())
	{
		for (int i = no_rels.size() - 1; i >= 0; --i)
		{
			Block* bl = no_rels[i];
			std::swap(no_rels[i], no_rels.back());
			no_rels.pop_back();
			bool has_refs = false;
			for (auto ch : bl->indeps)
			{
				auto found_ch = block_rels.find(ch.second.block);
				if (found_ch != block_rels.end())
				{
					found_ch->second--;
					if (found_ch->second == 0)
					{
						no_rels.push_back(found_ch->first);
					}
					else
					{
						has_refs = true;
					}
				}
			}
			if (no_rels.empty() && has_refs)
			{
				return true;
			}
		}
	}

	return false;
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
	else if (block->type == "Inport")
	{
		unfoldInport(block);
	}
	else if (block->type == "UnitDelayIn")
	{
		unfoldDelayIn(block);
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

void IR::unfoldDelayOut(Block* block)
{
	std::string full_name = struct_name + '.' + block->name;
	ir += "set(" + full_name + ',';
	for (auto& dep : block->indeps)
	{
		parse(dep.second.block);
	}
	auto meta = helpers::split(block->meta);
	ir += ',' + meta.back();
	ir += ')';
	Symbol s;
	s.type = SymbolType::Delay;
	s.val = meta.front();
	s.name = block->name;
	symbols[full_name] = s;
}

void IR::unfoldDelayIn(Block* block)
{
	std::string full_name = struct_name + '.' + block->name;
	ir += full_name;
}

void IR::unfoldOutport(Block* block)
{
	std::string full_name = struct_name + '.' + block->name;
	ir += "set(" + full_name + ',';
	for (auto& dep : block->indeps)
	{
		parse(dep.second.block);
	}
	ir += ')';
	Symbol s;
	s.type = SymbolType::Outport;
	s.name = block->name;
	symbols[full_name] = s;
}

void IR::unfoldInport(Block* block)
{
	std::string full_name = struct_name + '.' + block->name;
	ir += full_name;
	Symbol s;
	s.type = SymbolType::Inport;
	s.name = block->name;
	symbols[full_name] = s;
}

void IR::deleteRecursive(Term* term)
{
	for (auto ch : term->children)
	{
		deleteRecursive(ch);
	}
	delete term;
}

void IR::compact(Term* term, int term_id, std::map<std::string, std::unique_ptr<Term>>& terms_map, bool before_merge)
{
	auto found_term = terms_map.find(term->term_str);
	if (found_term == terms_map.end())
	{
		//in this case this means that this term and its parents are not in terms_map
		//therefor we just put it into map without changing its parents
		term->pending_cong = !before_merge;
		terms_map.emplace(term->term_str, std::unique_ptr<Term>(term));
	}
	else
	{
		//term already in map, this means, that it and its children are in terms_map
		//its parent still added into terms_map only once
		//but two identical terms could have same parent e.g. f(a,a)
		// so we need to add parent uniquely here
		// update this element in parent with found term
		//->delete this term recursively
		if (!term->parents.empty())
		{
			found_term->second->parents.insert(*term->parents.begin());
			(*term->parents.begin())->children[term_id] = found_term->second.get();
		}

		deleteRecursive(term);
		return;
	}
	int i = 0;
	for (auto ch : term->children)
	{
		compact(ch, i, terms_map, before_merge);
		++i;
	}
}

void IR::collectVars(Term* t, std::vector<std::unique_ptr<Term>>& variables)
{
	if (t->label.find("var") != std::string::npos)
	{
		return;
	}
	if (t->children.empty())
	{
		auto s = getSymbol(t->label);
		if (!s)
		{
			return;
		}

		switch (s->type) {
		case SymbolType::Inport:
		case SymbolType::Outport:
			ports.insert(t);
			break;
		case SymbolType::Delay:
			delays.insert(t);
			break;
		default:
			break;
		}

		return;
	}
	for (auto ch : t->children)
	{
		collectVars(ch, variables);
	}
	if (t->parents.size() > 1)
	{
		auto var = std::make_unique<Term>();
		var->label = "var" + std::to_string(variables.size());
		var->parents = std::move(t->parents);
		var->e_rep = var.get();
		t->parents.clear();
		for (auto& par : var->parents)
		{
			for (int i = 0; i < par->children.size(); ++i)
			{
				if (TRSHelpers::find(par->children[i]) == TRSHelpers::find(t))
				{
					par->children[i] = var.get();
				}
			}
		}
		Symbol s;
		s.type = SymbolType::LocalVar;
		TRSHelpers::printTerm(t, s.val);
		symbols[var->label] = s;
		variables.push_back(std::move(var));
	}
}
