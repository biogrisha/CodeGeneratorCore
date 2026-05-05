#pragma once
#include "Structs.h"
#include <memory>

class TermParser
{
public:
    TermParser(const std::string& str)
        : m_str(str)
    {
    }

    void parse();
    Term* getResult() { return m_current_term; }
private:
    void consumeTermName();

    Term* m_current_term = nullptr;
    Term* m_parent_term = nullptr;
    const std::string& m_str;
    int m_pos = 0;
};

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
    void deleteRecursive(Term* term);
    void compact(Term* term, int term_id, std::map<std::string, std::unique_ptr<Term>>& terms_map, bool before_merge = true);

	std::map<std::string, std::unique_ptr<Term>> egraph;
	std::string ir;
};