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
	TermParser pr(ir);
	pr.parse();
	main_t = pr.getResult();
	compact(main_t, 0, egraph);
	collectVars(main_t, local_variables);
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
	Symbol s;
	s.type = SymbolType::Delay;
	s.val = meta.front();
	symbols[block->name] = s;
}

void IR::unfoldOutport(Block* block)
{
	ir += "set(" + block->name + ',';
	for (auto& dep : block->indeps)
	{
		parse(dep.second.block);
	}
	ir += ')';
	Symbol s;
	s.type = SymbolType::Outport;
	symbols[block->name] = s;
}

void IR::unfoldInport(Block* block)
{
	ir += block->name;
	Symbol s;
	s.type = SymbolType::Inport;
	symbols[block->name] = s;
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

inline void IR::collectVars(Term* t, std::vector<std::pair<Term*, std::unique_ptr<Term>>>& variables)
{
	if (t->label.find("var") != std::string::npos)
	{
		return;
	}
	if (t->children.empty())
	{
		//auto s = getSymbol(t->label);
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
				if (helpers::find(par->children[i]) == helpers::find(t))
				{
					par->children[i] = var.get();
				}
			}
		}
		Symbol s;
		s.type = SymbolType::LocalVar;
		helpers::printTerm(t, s.val);
		symbols[var->label] = s;
		variables.push_back({ t, std::move(var) });
	}
}

void TermParser::parse()
{
	while (m_pos != m_str.size())
	{
		int term_start = m_pos;
		consumeTermName();
		int label_end = m_pos;
		auto t = new Term();
		t->e_reps.push_back(t);
		t->e_rep = t;
		m_current_term = t;
		if (m_parent_term)
		{
			m_current_term->parents.insert(m_parent_term);
			m_parent_term->children.push_back(m_current_term);
		}
		if (m_pos > m_str.size())
		{
			return;
		}
		if (m_str[m_pos] == '(')
		{
			++m_pos;
			m_parent_term = m_current_term;
			parse();
			m_current_term = m_parent_term;
			m_parent_term = !m_current_term->parents.empty() ? *m_current_term->parents.begin() : nullptr;
		}
		m_current_term->label = m_str.substr(term_start, label_end - term_start);
		m_current_term->term_str = m_str.substr(term_start, m_pos - term_start);
		if (m_pos >= m_str.size())
		{
			return;
		}
		if (m_str[m_pos] == ')')
		{
			++m_pos;
			return;
		}
		++m_pos;
	}
}

void TermParser::consumeTermName()
{
	int i = m_pos;
	for (; i < m_str.size(); ++i)
	{
		if (m_str[i] == '(' || m_str[i] == ')' || m_str[i] == ',')
		{
			break;
		}
	}
	m_pos = i;
}
