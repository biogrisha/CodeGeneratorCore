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
    Symbol* getSymbol(const std::string& name);
    const std::vector<std::unique_ptr<Term>>& getLocalVars() { return local_variables; }
    const std::unordered_set<Term*>& getPorts() { return ports; }
    const std::unordered_set<Term*>& getDelays() { return delays; }
    std::map<std::string, std::unique_ptr<Term>>& getEGraph() { return egraph;}
    Term* getMainTerm() { return main_t; }
private:
	void parse(Block* block);
	void unfoldGain(Block* block);
	void unfoldSum(Block* block, int i);
	void unfoldDelay(Block* block);
	void unfoldOutport(Block* block);
    void unfoldInport(Block* block);
    void deleteRecursive(Term* term);
    void compact(Term* term, int term_id, std::map<std::string, std::unique_ptr<Term>>& terms_map, bool before_merge = true);
	void collectVars(Term* t, std::vector<std::unique_ptr<Term>>& variables);

	std::map<std::string, std::unique_ptr<Term>> egraph;

    std::map<std::string, Symbol> symbols;

    std::unordered_set<Term*> ports;
    std::unordered_set<Term*> delays;
    std::vector<std::unique_ptr<Term>> local_variables;
	std::string ir;
    Term* main_t = nullptr;
};