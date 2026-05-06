#include "TermParser.h"


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