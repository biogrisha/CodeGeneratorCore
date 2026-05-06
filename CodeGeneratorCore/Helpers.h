#pragma once
#include <vector>
#include <string>
#include <memory>
#include <map>
#include "Structs.h"
#include "TermParser.h"

namespace helpers
{
    std::vector<std::string> split(const std::string& s);

    std::string removeWhitespace(std::string s);
}

namespace TRSHelpers
{
    void makeChild(Term* t);

    void leafsCount(Term* t, int& num);

    void printTerm(Term* pat, std::string& res);

    void deleteRecursive(Term* term);

    void compact(Term* term, int term_id, std::map<std::string, std::unique_ptr<Term>>& terms_map, bool before_merge = true);

    Term* find(Term* t);

    void unionTerms(Term* t1, Term* t2);

    bool cong(Term* t1, Term* t2);

    void merge(Term* t1, Term* t2);

    void markPatternNodes(Term* t);

    void setupOrder(Term* t);

    void reduce(Term* a, Term* b);

    class Matcher
    {
        struct BStackEl
        {
            int parent_i = -1;
            int child_i = 0;
            int eq_i = -1;
            Term* lhs = nullptr;
            Term* rhs = nullptr;
            Term* rhs_main = nullptr;

            void nextRhs();
            bool updateEq();
            bool getChild(Term*& out_lhs, Term*& out_rhs);
        };
    public:

        bool match(Term* lhs, Term* rhs);

        // false means we finished comparisson
        bool next();

        bool back();

        bool in();

        std::vector<BStackEl> bstack;
        std::map<Term*, Arg> args;
    };


    void rewrite(Term* pat, std::map<Term*, Arg>& args, std::string& res);

    Term* instantiate(Term* pat, std::map<Term*, Arg>& args, std::map<std::string, std::unique_ptr<Term>>& terms_map);

    void updateCongruence(Term* t);

}