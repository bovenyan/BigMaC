#ifndef SWITCH_TRACER
#define SWITCH_TRACER

#include "../stdafx.h"
#include "../rules/RuleList.hpp"

#include <map>
#include <set>

using std::vector; using std::map; using std::pair;
using std::set;

struct flow_tables { 
    int sEntryNo = 0;
    map<int, double> nsEntries;

};

struct sRule_rec{
    double activeTime;
    vector<int> path;
};

class switch_tracker{
    private:
        rule_list * rList;
        map<int, sRule_rec> bs_sRule; 
        vector<flow_tables> switch_rec;

        //  

    public:
    switch_tracker(rule_list * rL);

    void MaC(const int sRuleID, const vector<int> & path, double curTime);
    void clear_obsolte(double curTime, double timeout = 10);
    vector<double> check_usage();

    ~switch_tracker();
};

#endif
