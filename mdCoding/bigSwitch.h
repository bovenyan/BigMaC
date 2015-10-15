#ifndef RULELIST_H
#define RULELIST_H

#include "sharedHeader.h"
#include "Address.hpp"
#include "bigMacRule.hpp"
#include <unordered_map>

class bigSwitch
{
public:
    vector<sRule> swTable;
    vector<nsRule> mgmtTable;

public:
    bigSwitch();
    bigSwitch(std::string & swTableFile, std::string & mgmtTableFile);

    void removeSwDep();
    void calAssoc();

    sRule * matchSRule();
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
