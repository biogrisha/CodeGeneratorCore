#include "TRS.h"
#include "TermParser.h"
#include "Helpers.h"
#include <iostream>

void TRSOptimizer::optimize()
{
	std::vector<Identity> sat_identities = {
        {"add(`a,`b)", "add(`b,`a)"},
        {"add(add(`a,`b),`c)", "add(`a,add(`b,`c))"},
        {"add(`a,add(`b,`c))", "add(add(`a,`b),`c)"},
        {"mul(`a,`b)", "mul(`b,`a)"},
        {"mul(mul(`a,`b),`c)", "mul(`a,mul(`b,`c))"},
        {"mul(`a,mul(`b,`c))", "mul(mul(`a,`b),`c)"},
        {"mul(add(`a,`b),`c)", "add(mul(`a,`c),mul(`b,`c))"},
        {"add(mul(`a,`c),mul(`b,`c))","mul(add(`a,`b),`c)"},
    };

	std::vector<Identity> red_identities = {
        {"add(`a,0)", "`a"},
        {"add(neg(`a),neg(`a))", "0"},
        {"mul(`b,1)", "`b"},
        {"mul(`b,0)", "0"},
    };

    Term* t_lhs = nullptr;
    Term* t_rhs = nullptr;
    std::map<std::string, std::unique_ptr<Term>> terms_map;


    for (auto& id : sat_identities)
    {
        {
            TermParser pr(id.lhs);
            pr.parse();
            TRSHelpers::compact(pr.getResult(), 0, terms_map);
            id.t_lhs = terms_map.find(id.lhs)->second.get();
            TRSHelpers::markPatternNodes(id.t_lhs);
            TRSHelpers::setupOrder(id.t_lhs);
        }

        {
            TermParser pr(id.rhs);
            pr.parse();
            TRSHelpers::compact(pr.getResult(), 0, terms_map);
            id.t_rhs = terms_map.find(id.rhs)->second.get();
            TRSHelpers::markPatternNodes(id.t_rhs);
            TRSHelpers::setupOrder(id.t_rhs);
        }
    }


    for (auto& id : red_identities)
    {
        {
            TermParser pr(id.lhs);
            pr.parse();
            TRSHelpers::compact(pr.getResult(), 0, terms_map);
            id.t_lhs = terms_map.find(id.lhs)->second.get();
            TRSHelpers::markPatternNodes(id.t_lhs);
            TRSHelpers::setupOrder(id.t_lhs);
        }

        {
            TermParser pr(id.rhs);
            pr.parse();
            TRSHelpers::compact(pr.getResult(), 0, terms_map);
            id.t_rhs = terms_map.find(id.rhs)->second.get();
            TRSHelpers::markPatternNodes(id.t_rhs);
            TRSHelpers::setupOrder(id.t_rhs);
        }
    }
    saturate(sat_identities, 2);
    reduce(red_identities);
    minimize();
}

void TRSOptimizer::saturate(const std::vector<Identity>& ids, int iterations)
{
    for (int i = 0; i < iterations; ++i)
    {
        int id_i = 0;
        for (auto& id : ids)
        {
            std::vector<Identity> new_ids;
            for (auto& t : egraph)
            {
                if (t.second->pat)
                {
                    continue;
                }
                if (t.second->e_rep != t.second.get())
                {
                    continue;
                }
                if (t.second->ids_passed.contains(id_i))
                {
                    continue;
                }
                TRSHelpers::Matcher mc;
                if (mc.match(id.t_lhs, t.second.get()))
                {
                    t.second->ids_passed.insert(id_i);

                    std::string str;
                    TRSHelpers::rewrite(id.t_rhs, mc.args, str);
                    auto& new_id = new_ids.emplace_back();
                    new_id.t_lhs = t.second.get();
                    new_id.rhs = std::move(str);

                    std::cout << "==================== \n";
                    std::cout << id.lhs << " => " << id.rhs << '\n';
                    std::cout << t.second->term_str << " => " << new_id.rhs << '\n';
                    for (auto& arg : mc.args)
                    {
                        std::cout << arg.first->label << " = " << arg.second.term->term_str << '\n';
                    }
                }
            }

            for (auto& new_id : new_ids)
            {
                if (new_id.t_lhs->term_str == new_id.rhs)
                {
                    continue;
                }
                auto found = egraph.find(new_id.rhs);
                if (found != egraph.end())
                {
                    new_id.t_rhs = found->second.get();
                }
                else
                {
                    TermParser pr(new_id.rhs);
                    pr.parse();
                    TRSHelpers::compact(pr.getResult(), 0, egraph, false);
                    new_id.t_rhs = egraph.find(new_id.rhs)->second.get();
                    TRSHelpers::updateCongruence(new_id.t_rhs);
                }
                TRSHelpers::merge(new_id.t_lhs, new_id.t_rhs);
            }


            id_i++;
        }
    }
}

void TRSOptimizer::reduce(const std::vector<Identity>& ids)
{
    std::cout << "REDUCTION";
    for (auto& id : ids)
    {
        std::vector<Identity> new_ids;
        for (auto& t : egraph)
        {
            if (t.second->pat)
            {
                continue;
            }
            if (t.second->e_rep != t.second.get())
            {
                continue;
            }
            TRSHelpers::Matcher mc;
            if (mc.match(id.t_lhs, t.second.get()))
            {
                std::string str;
                TRSHelpers::rewrite(id.t_rhs, mc.args, str);
                auto& new_id = new_ids.emplace_back();
                new_id.t_lhs = t.second.get();
                new_id.rhs = std::move(str);

                std::cout << "==================== \n";
                std::cout << id.lhs << " => " << id.rhs << '\n';
                std::cout << t.second->term_str << " => " << new_id.rhs << '\n';
                for (auto& arg : mc.args)
                {
                    std::cout << arg.first->label << " = " << arg.second.term->term_str << '\n';
                }
            }
        }

        for (auto& new_id : new_ids)
        {
            if (new_id.t_lhs->term_str == new_id.rhs)
            {
                continue;
            }
            auto found = egraph.find(new_id.rhs);
            if (found != egraph.end())
            {
                new_id.t_rhs = found->second.get();
            }
            else
            {
                TermParser pr(new_id.rhs);
                pr.parse();
                TRSHelpers::compact(pr.getResult(), 0, egraph, false);
                new_id.t_rhs = egraph.find(new_id.rhs)->second.get();
                TRSHelpers::updateCongruence(new_id.t_rhs);
            }
            TRSHelpers::merge(new_id.t_lhs, new_id.t_rhs);
        }
    }
}

void TRSOptimizer::minimize()
{
    for (auto& t : egraph)
    {
        if (t.second->pat)
        {
            continue;
        }
        if (t.second->e_rep != t.second.get())
        {
            continue;
        }
        Term* min_rep = nullptr;
        int norm = INT_MAX;
        for (auto rep : t.second->e_reps)
        {
            int leaf_count = 0;
            TRSHelpers::leafsCount(rep, leaf_count);
            if (leaf_count < norm)
            {
                min_rep = rep;
                norm = leaf_count;
            }
        }
        TRSHelpers::makeChild(min_rep);
    }
}
