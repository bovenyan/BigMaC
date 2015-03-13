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
    map<int, int> nsEntries; // count for occupation 

};

class bigmac{
    private:
        rule_list * rList;
        int node_no;
        vector<vector<int> > sRule_path_map;
        vector<vector<vector<int> > > sRule_path_map_ecmp;

        vector<flow_table> switch_rec;
        map<int, int> ns_loc_rec;
        map<int, vector<int> > ns_loc_rec_ecmp;
        map<int, int> active_sRule;

        vector<int> cost_cal;
        int total_cost;
        int cache_counter;

    public:
        bigmac(rule_list * rL, int node_no, const vector<vector<int> > &);
        bigmac(rule_list * rL, int node_no, const vector<vector<vector<int> > > &);

        void MaC(const int & sRuleID);
        void MaC_ecmp(const int & sRuleID);
        void Evict(const int & sRuleID);
        void Evict_ecmp(const int & sRuleID);
        
        vector<double> check_usage();
        int check_tags();

        int cal_avg_cost();
};

#endif
