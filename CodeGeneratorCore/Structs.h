#pragma once
#include <map>
#include <string>
#include <vector>
#include <unordered_set>
struct Dep
{
    struct Block* block = nullptr;
    int port = 0;
};

struct Block
{
    std::string type;
    std::string name;
    std::string sid;
    std::string meta;
    std::map<int, Dep> indeps;
    std::vector<Dep> outdeps;
};

struct Term
{
    std::string term_str;
    std::string label;
    std::vector<Term*> children;
    std::unordered_set<Term*> parents;
    std::vector<Term*> e_reps;
    Term* e_rep = nullptr;
    bool pat = false;
    std::vector<int> comp_order;
    std::unordered_set<int> ids_passed;
    bool pending_cong = false;
};

enum class SymbolType
{
    Inport,
    Outport,
    Delay,
    LocalVar,
    None
};
struct Symbol
{
    SymbolType type = SymbolType::None;
    std::string val;
};

struct Identity
{
    std::string lhs;
    std::string rhs;
    Term* t_lhs = nullptr;
    Term* t_rhs = nullptr;
};