#ifndef BIG_MAC
#define BIG_MAC

#include "../stdafx.h"
#include "../rules/RuleList.hpp"

#include <map>
#include <set>

using std::vector; using std::map; using std::pair;
using std::set;

struct flow_table { 
    int sEntryNo = 0;
    map<int, double> nsEntries;

};

struct sRule_rec{
    double activeTime = -10;
    bool active = false;
    vector<int> path;
};

class bigmac{
    private:
        rule_list * rList;
        int node_no;

        //map<int, sRule_rec> bs_sRule;
        vector<sRule_rec> bs_sRule_rec;
        vector<flow_table> switch_rec;

        // map<int, vector<pair<int, int> > > ns_loc_rec;
        map<int, int> ns_loc_rec;

    public:
    bigmac(rule_list * rL, int node_no, vector<vector<int> > & path_info);

    // test big mac
    void MaC(const int sRuleID, double curTime);
    void clear_obsolte(double curTime, double timeout = 10);
    vector<double> check_usage();
};

#endif
