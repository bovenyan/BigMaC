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
        
        // path record
        vector<vector<int> > sRule_path_map;
        vector<vector<vector<int> > > sRule_path_map_ecmp;

        vector<flow_table> switch_rec;

        // Non-switching rule location
        map<int, int> ns_loc_rec;
        map<int, set<int> > ns_locs;
        map<int, vector<int> > ns_loc_rec_ecmp;
        // Active switching rule record
        map<int, int> active_sRule;

        // Statistics
        int tag_counter;

    public:
        // Shortest Path
        bigmac(rule_list * rL, int node_no, const vector<vector<int> > &); 
        void MaC(const int & sRuleID);
        void Evict(const int & sRuleID);

        // ECMP
        bigmac(rule_list * rL, int node_no, const vector<vector<vector<int> > > &);
        void MaC_ecmp(const int & sRuleID);
        void Evict_ecmp(const int & sRuleID);
       
        // Statistic
        vector<double> check_usage();
        int check_tags();
};

#endif
