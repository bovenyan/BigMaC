#include "BigMaC.h"


bigmac::bigmac(rule_list * rL, int nNo, vector<vector<int> > & path_info){
    rList = rL;
    node_no = nNo;

    // init path and Big Switch Switching rules
    for (auto path : path_info){
        sRule_rec record;
        record.path = path;
        bs_sRule_rec.push_back(record);
    }

    // init flow table record
    switch_rec = vector<flow_table> (node_no, flow_table());
}

void bigmac::MaC(const int sRuleID, double curTime){
    // if the sRule is cached already
    if (bs_sRule_rec[sRuleID].active){
        // update switching rule time
        bs_sRule_rec[sRuleID].activeTime = curTime;

        // update assoc non-switching rule time
        const vector<int> & assoc_rules = rList->assoc_map[sRuleID];

        // sRule-nsRule : location  =  sRule*nsSize + nsRule;
        for (int nsRuleID: assoc_rules){
            int loc = ns_loc_rec[sRuleID * rList->list.size() + nsRuleID];
            switch_rec[loc].nsEntries[nsRuleID] = curTime;
        }
    }

    // activate the sRule
    bs_sRule_rec[sRuleID].active = true;
    bs_sRule_rec[sRuleID].activeTime = curTime;
    
    const vector<int> & path = bs_sRule_rec[sRuleID].path;

    // get non-assoc pieces - puting every rule on one switch first...
    int min_cost = rList->list.size();
    int choosen_node = -1;
    auto assoc_rules = rList->assoc_map[sRuleID];

    for (auto node : path){
        ++switch_rec[node].sEntryNo; // ++ switching rules

        // finding min cost non-switching rules
        const map<int, double> & nsEntries = switch_rec[node].nsEntries;
        int cost = assoc_rules.size() + nsEntries.size();

        auto iter_1 = assoc_rules.begin();
        auto iter_2 = nsEntries.begin();

        while (iter_1 != assoc_rules.end() && iter_2 != nsEntries.end()){
            if (*iter_1 == iter_2->first){
                ++iter_1;
                ++iter_2;
                --cost;
            }
            else{
                if (*iter_1 > iter_2->first)
                    ++iter_2;
                else
                    ++iter_1;
            }
        }
        if (cost < min_cost)
            choosen_node = node;
    }

    // applying ns rules
    map<int, double> & nsEntries = switch_rec[choosen_node].nsEntries;

    for (auto nsRuleID : assoc_rules){
        nsEntries[nsRuleID] = curTime;

        ns_loc_rec[sRuleID * rList->list.size() + nsRuleID] = choosen_node;
    }
}

void bigmac::clear_obsolte(double curTime, double timeout){

    // check every sRule
    for (int sRuleID = 0; sRuleID < bs_sRule_rec.size(); ++sRuleID){
        sRule_rec & sRule = bs_sRule_rec[sRuleID];
        
        if (sRule.active && 
            sRule.activeTime + timeout < curTime){ // this switching entry is to be del
            
            const vector<int> & assoc_rules = rList->assoc_map[sRuleID];

            // check associated entries:
            for (auto nsRuleID : assoc_rules){
                int loc = ns_loc_rec[sRuleID*rList->list.size() + nsRuleID];
                
                if (switch_rec[loc].nsEntries[nsRuleID] + timeout < curTime){
                    switch_rec[loc].nsEntries.erase(nsRuleID); // erase entry
                    ns_loc_rec.erase(sRuleID*rList->list.size() + nsRuleID); // erase loc
                }
            }

            sRule.active = false;
        }
    }
}

vector<double> bigmac::check_usage(){
    double avg_usage = 0;
    int max_usage = 0;
    int min_usage = rList->list.size() + rList->list_switching.size();

    for (auto iter_node = switch_rec.begin(); iter_node != switch_rec.end(); ++iter_node){
        int usage = (iter_node->sEntryNo + iter_node->nsEntries.size());
        avg_usage += usage;
        if (usage > max_usage)
            max_usage = usage;
        if (usage < min_usage)
            min_usage = usage;
    }

    avg_usage /= node_no; 

    vector<double> res;
    res.push_back(double(min_usage));
    res.push_back(avg_usage);
    res.push_back(double(max_usage));

    return res;
}
