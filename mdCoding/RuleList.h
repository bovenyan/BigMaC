#ifndef RULELIST_H
#define RULELIST_H

#include "stdafx.h"
#include "Address.hpp"
#include "Rule.hpp"
#include <unordered_map>

class rule_list
{
public:
    std::vector<p_rule> list;
    std::unordered_map <uint32_t, std::vector<uint32_t> > dep_map;
    std::vector<size_t> occupancy;

public:
    rule_list();
    rule_list(std::string &, bool = false);

    void obtain_dep();
    r_rule get_micro_rule (const addr_5tup &);
    int linear_search(const addr_5tup &);

    // debug and print
    void print(const std::string &);
    
    // void clearHitFlag();   // obsolete
    // void rule_dep_analysis();
};

class sRuleSet: public rule_list {

};

class nsRuleSet: public rule_list {

};

class sFlowSet: public rule_list {
public:
    int switch_id;
};

class nsFlowSet: public rule_list {
public:
    int switch_id;
};

#endif
