#include "Helpers.h"

namespace helpers
{
    std::vector<std::string> helpers::split(const std::string& s)
    {
        std::vector<std::string> result;

        size_t start = 0;
        size_t pos;

        while ((pos = s.find(',', start)) != std::string::npos)
        {
            result.emplace_back(s.substr(start, pos - start));
            start = pos + 1;
        }

        result.emplace_back(s.substr(start)); // last part

        return result;
    }

    std::string helpers::removeWhitespace(std::string s)
    {
        s.erase(std::remove_if(s.begin(), s.end(),
            [](unsigned char c) { return std::isspace(c); }),
            s.end());
        return s;
    }
}

namespace TRSHelpers
{
    void makeChild(Term* t)
    {
        auto top = find(t);
        for (auto par : top->parents)
        {
            for (int i = 0; i < par->children.size(); ++i)
            {
                if (top == find(par->children[i]))
                {
                    par->children[i] = t;
                }
            }
        }
    }

    void leafsCount(Term* t, int& num)
    {
        if (t->children.empty())
        {
            ++num;
            return;
        }
        for (auto ch : t->children)
        {
            leafsCount(ch, num);
        }
    }

    void printTerm(Term* pat, std::string& res)
    {
        res += pat->label;
        if (!pat->children.empty())
        {
            res += '(';
            for (auto& ch : pat->children)
            {
                printTerm(ch, res);
                res += ',';
            }
            res.back() = ')';
        }
    }

    void deleteRecursive(Term* term)
    {
        for (auto ch : term->children)
        {
            deleteRecursive(ch);
        }
        delete term;
    }

    void compact(Term* term, int term_id, std::map<std::string, std::unique_ptr<Term>>& terms_map, bool before_merge)
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

    Term* find(Term* t)
    {
        while (t->e_rep != t)
        {
            t = t->e_rep;
        }
        return t;
    }

    void unionTerms(Term* t1, Term* t2)
    {
        t1 = find(t1);
        t2 = find(t2);
        if (t1 == t2)
        {
            return;
        }
        Term* main_t = t1->e_reps.size() > t2->e_reps.size() ? t1 : t2;
        Term* sub_t = t1 == main_t ? t2 : t1;

        for (auto par : sub_t->parents)
        {
            main_t->parents.insert(par);
        }
        sub_t->parents.clear();
        main_t->e_reps.insert(main_t->e_reps.end(), sub_t->e_reps.begin(), sub_t->e_reps.end());
        sub_t->e_reps.clear();
        sub_t->e_rep = main_t;
    }

    bool cong(Term* t1, Term* t2)
    {
        if (t1->label != t2->label)
        {
            return false;
        }
        for (int i = 0; i < t1->children.size(); ++i)
        {
            if (find(t1->children[i]) != find(t2->children[i]))
            {
                return false;
            }
        }
        return true;
    }

    void merge(Term* t1, Term* t2)
    {
        t1 = find(t1);
        t2 = find(t2);
        auto pars1 = t1->parents;
        auto pars2 = t2->parents;
        unionTerms(t1, t2);
        for (auto par1 : pars1)
        {
            if (find(par1) == t1)
            {
                continue;
            }
            for (auto par2 : pars2)
            {
                if (find(par2) == t1)
                {
                    continue;
                }
                if (par1 != par2 && cong(par1, par2))
                {
                    merge(par1, par2);
                }
            }
        }
    }

    void markPatternNodes(Term* t)
    {
        bool pat_temp = false;
        for (auto ch : t->children)
        {
            markPatternNodes(ch);
            pat_temp |= ch->pat;
        }
        if (pat_temp)
        {
            t->pat = true;
            return;
        }
        if (t->label[0] == '`')
        {
            t->pat = true;
        }
    }

    void setupOrder(Term* t)
    {
        for (int i = 0; i < t->children.size(); ++i)
        {
            if (!t->children[i]->pat)
            {
                t->comp_order.push_back(i);
            }
        }
        for (int i = 0; i < t->children.size(); ++i)
        {
            if (t->children[i]->pat)
            {
                t->comp_order.push_back(i);
            }
        }
        for (auto ch : t->children)
        {
            setupOrder(ch);
        }
    }

    void reduce(Term* t1, Term* t2)
    {
        Term* main_t = t2;
        Term* main_top = find(t2);
        Term* sub_t = find(t1);

        if (main_t != main_top)
        {
            //replace in parendts
            for (auto par : main_top->parents)
            {
                for (int i = 0; i < par->children.size(); ++i)
                {
                    if (find(par->children[i]) == main_top)
                    {
                        par->children[i] = main_t;
                    }
                }
            }
            //we can move parents since if main_t has top then it does not have parents
            main_t->parents = std::move(main_top->parents);
            main_top->parents.clear();
            main_top->e_reps.clear();
            main_top->e_rep = main_t;
        }
        if (sub_t == main_top)
        {
            return;
        }

        //replace in parendts
        for (auto par : sub_t->parents)
        {
            for (int i = 0; i < par->children.size(); ++i)
            {
                if (find(par->children[i]) == sub_t)
                {
                    par->children[i] = main_t;
                }
            }
        }

        for (auto par : sub_t->parents)
        {
            main_t->parents.insert(par);
        }
        sub_t->parents.clear();
        sub_t->e_reps.clear();
        sub_t->e_rep = main_t;
        main_t->e_rep = main_t;
    }

    void rewrite(Term* pat, std::map<Term*, Arg>& args, std::string& res)
    {
        if (pat->label[0] == '`')
        {
            auto arg = args.find(pat);
            if (arg == args.end())
            {
                res += pat->label;
            }
            else
            {
                res += arg->second.term->term_str;
            }
            return;
        }
        res += pat->label;
        if (!pat->children.empty())
        {
            res += '(';
            for (auto& ch : pat->children)
            {
                rewrite(ch, args, res);
                res += ',';
            }
            res.back() = ')';
        }
    }

    Term* instantiate(Term* pat, std::map<Term*, Arg>& args, std::map<std::string, std::unique_ptr<Term>>& terms_map)
    {
        std::string str;
        rewrite(pat, args, str);
        auto found = terms_map.find(str);
        if (found != terms_map.end())
        {
            return found->second.get();
        }
        TermParser pr(str);
        pr.parse();
        compact(pr.getResult(), 0, terms_map);

        return terms_map.find(str)->second.get();
    }

    void updateCongruence(Term* t)
    {
        if (!t->pending_cong)
        {
            return;
        }
        if (t->children.empty())
        {
            return;
        }
        for (auto ch : t->children)
        {
            updateCongruence(ch);
        }
        auto pars = find(t->children.back())->parents;
        for (auto par : pars)
        {
            if (find(t) != find(par) && cong(t, par))
            {
                unionTerms(t, par);
            }
        }
    }

    void Matcher::BStackEl::nextRhs()
    {
        ++eq_i;
        if (lhs->label[0] == '`')
        {
            return;
        }
        for (; eq_i < rhs_main->e_reps.size(); ++eq_i)
        {
            if (rhs_main->e_reps[eq_i]->label == lhs->label)
            {
                return;
            }
        }
    }

    bool Matcher::BStackEl::updateEq()
    {
        nextRhs();
        if (eq_i >= rhs_main->e_reps.size())
        {
            return false;
        }
        rhs = rhs_main->e_reps[eq_i];
        child_i = 0;
        return true;
    }

    bool Matcher::BStackEl::getChild(Term*& out_lhs, Term*& out_rhs)
    {
        if (rhs->children.empty())
        {
            return false;
        }
        if (child_i >= lhs->children.size())
        {
            return false;
        }
        out_lhs = lhs->children[lhs->comp_order[child_i]];
        out_rhs = rhs->children[lhs->comp_order[child_i]];
        return true;
    }

    bool Matcher::match(Term* lhs, Term* rhs)
    {
        BStackEl el;
        el.lhs = lhs;
        el.rhs_main = find(rhs);
        if (!el.updateEq())
        {
            return false;
        }
        bstack.push_back(el);
        while (true)
        {
            BStackEl& top = bstack.back();
            if (find(top.lhs) == find(top.rhs))
            {
                if (!next())
                {
                    return true;
                }
                BStackEl& new_top = bstack.back();
                if (!new_top.updateEq())
                {
                    if (!back())
                    {
                        return false;
                    }
                }
            }
            else if (top.lhs->label[0] == '`')
            {
                auto found_arg = args.find(top.lhs);
                if (found_arg == args.end())
                {
                    auto new_arg = args.emplace(top.lhs, Arg());
                    new_arg.first->second.node_id = bstack.size();
                    new_arg.first->second.term = top.rhs;
                    if (!next())
                    {
                        return true;
                    }
                    BStackEl& new_top = bstack.back();
                    if (!new_top.updateEq())
                    {
                        if (!back())
                        {
                            return false;
                        }
                    }
                }
                else if (find(found_arg->second.term) != find(top.rhs))
                {
                    if (!back())
                    {
                        return false;
                    }
                }
                else
                {
                    if (!next())
                    {
                        return true;
                    }
                    BStackEl& new_top = bstack.back();
                    if (!new_top.updateEq())
                    {
                        if (!back())
                        {
                            return false;
                        }
                    }
                }
            }
            else
            {
                if (!lhs->pat)
                {
                    if (!back())
                    {
                        return false;
                    }
                    continue;
                }
                if (!in())
                {
                    return true;
                }
                BStackEl& new_top = bstack.back();
                if (!new_top.updateEq())
                {
                    if (!back())
                    {
                        return false;
                    }
                }

            }

        }

    }

    // false means we finished comparisson
    bool Matcher::next()
    {
        auto& top = bstack.back();
        int parent_i = top.parent_i;
        while (parent_i >= 0)
        {
            auto& par_el = bstack[parent_i];
            Term* lhs = nullptr;
            Term* rhs = nullptr;
            if (par_el.getChild(lhs, rhs))
            {
                ++par_el.child_i;
                BStackEl new_el;
                new_el.lhs = lhs;
                new_el.rhs_main = find(rhs);
                new_el.parent_i = parent_i;
                bstack.push_back(new_el);
                return true;
            }
            else
            {
                parent_i = par_el.parent_i;
            }
        }
        return false;
    }

    bool Matcher::back()
    {
        for (int i = bstack.size() - 1; i >= 0; --i)
        {
            auto& top = bstack.back();
            if (top.updateEq())
            {
                // clear args
                for (auto it = args.begin(); it != args.end();)
                {
                    if (it->second.node_id > i)
                    {
                        it = args.erase(it); // returns next valid iterator
                    }
                    else
                    {
                        ++it;
                    }
                }
                return true;
            }
            if (top.parent_i >= 0)
            {
                auto& par = bstack[top.parent_i];
                --par.child_i;
            }
            bstack.pop_back();
        }
        return false;
    }

    bool Matcher::in()
    {
        auto& top = bstack.back();
        Term* lhs = nullptr;
        Term* rhs = nullptr;

        if (!top.getChild(lhs, rhs))
        {
            return next();
        }
        ++top.child_i;
        BStackEl new_el;
        new_el.lhs = lhs;
        new_el.rhs_main = find(rhs);
        new_el.parent_i = bstack.size() - 1;
        bstack.push_back(new_el);
        return true;
    }
}
