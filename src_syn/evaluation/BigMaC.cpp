#include "BigMaC.h"


bigmac::bigmac(rule_list * rL, int nNo, const vector<vector<int> > & path_map){
    sRule_path_map = path_map;
    rList = rL;
    node_no = nNo;

    total_cost = 0;
    cache_counter = 0;
    // init flow table record
    switch_rec = vector<flow_table> (node_no, flow_table());
}

bigmac::bigmac(rule_list * rL, int nNo, const vector<vector<vector<int> > > & path_map_ecmp){
    sRule_path_map_ecmp = path_map_ecmp;
    rList = rL;
    node_no = nNo;

    total_cost = 0;
    cache_counter = 0;
    // init flow table record
    switch_rec = vector<flow_table> (node_no, flow_table());
}

void bigmac::MaC(const int & sRuleID){
    
    if (active_sRule.find(sRuleID) == active_sRule.end()){ // not found
        active_sRule[sRuleID] = 1;
    }
    else {
        ++active_sRule[sRuleID];
        return;
    }

    const vector<int> & assoc_rules = rList->assoc_map[sRuleID];
    const vector<int> & path = sRule_path_map[sRuleID]; // path associated;

    // get non-assoc pieces - puting every rule on one switch first...
    int min_cost = rList->list.size();
    int choosen_node = -1;

    for (auto node : path){
        ++switch_rec[node].sEntryNo; // ++ switching rules

        // finding min cost non-switching rules
        const map<int, int> & nsEntries = switch_rec[node].nsEntries;
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
        if (cost < min_cost){
            choosen_node = node;
            min_cost = cost;
        }
    }

    total_cost += min_cost;
    cache_counter += 1;

    // applying ns rules
    map<int, int> & nsEntries = switch_rec[choosen_node].nsEntries;

    for (auto nsRuleID : assoc_rules){
        if (nsEntries.find(nsRuleID) != nsEntries.end())
            ++nsEntries[nsRuleID]; // inc occupancy
        else
            nsEntries[nsRuleID] = 1;
        // sRule-nsRule : location = sRule*nsSize + nsRule;
        ns_loc_rec[sRuleID * rList->list.size() + nsRuleID] = choosen_node;
    }
}


void bigmac::Evict(const int & sRuleID){
    if (active_sRule.find(sRuleID) == active_sRule.end()){
        cout<<"error BigMaC: no paired flow begin"<<endl;
        return;
    }
    else{
        if (active_sRule[sRuleID] > 1){ // still active
            --active_sRule[sRuleID];
            return;
        }
        else{ // release
            active_sRule.erase(sRuleID);
        }
    }

    const vector<int> & assoc_rules = rList->assoc_map[sRuleID];
    const vector<int> & path = sRule_path_map[sRuleID]; // path associated;

    // deal with switching entries
    for (auto node : path){
        --switch_rec[node].sEntryNo; 
    }

    // deal with non switching entries
    for (const int & nsRuleID : assoc_rules){
        int loc = sRuleID * rList->list.size() + nsRuleID;
        loc = ns_loc_rec[loc];
        auto nsEntries = switch_rec[loc].nsEntries;

        // remove ns occupancy
        auto ns_target = nsEntries.find(nsRuleID);
        if (ns_target != nsEntries.end()){
            if (ns_target->second == 1)
                nsEntries.erase(ns_target);
            else
                --ns_target->second;
        }
    }
}


void bigmac::MaC_ecmp(const int & sRuleID){
    
    if (active_sRule.find(sRuleID) == active_sRule.end()){ // not found
        active_sRule[sRuleID] = 1;
    }
    else {
        ++active_sRule[sRuleID];
        return;
    }

    const vector<int> & assoc_rules = rList->assoc_map[sRuleID];
    vector<int> choosen_node_list;

    for (auto & path : sRule_path_map_ecmp[sRuleID]){
    
        // get non-assoc pieces - puting every rule on one switch first...
        int min_cost = rList->list.size();
        int choosen_node = -1;
    
        for (auto node : path){
            ++switch_rec[node].sEntryNo; // ++ switching rules
    
            // finding min cost non-switching rules
            const map<int, int> & nsEntries = switch_rec[node].nsEntries;
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
            if (cost < min_cost){
                choosen_node = node;
                min_cost = cost;
            }
        }

        choosen_node_list.push_back(choosen_node);
    }
    
    for (auto nsRuleID : assoc_rules){
        ns_loc_rec_ecmp[sRuleID * rList->list.size() + nsRuleID] = vector<int>();

        for (auto choosen_node : choosen_node_list){
            map<int, int> & nsEntries = switch_rec[choosen_node].nsEntries;

            if (nsEntries.find(nsRuleID) != nsEntries.end())
                ++nsEntries[nsRuleID]; // inc occupancy
            else
                nsEntries[nsRuleID] = 1;
            ns_loc_rec_ecmp[sRuleID * rList->list.size() + nsRuleID].push_back(choosen_node);
        }
    }
}


void bigmac::Evict_ecmp(const int & sRuleID){
    if (active_sRule.find(sRuleID) == active_sRule.end()){
        cout<<"error BigMaC: no paired flow begin"<<endl;
        return;
    }
    else{
        if (active_sRule[sRuleID] > 1){ // still active
            --active_sRule[sRuleID];
            return;
        }
        else{ // release
            active_sRule.erase(sRuleID);
        }
    }

    const vector<int> & assoc_rules = rList->assoc_map[sRuleID];
    const vector<int> & path = sRule_path_map[sRuleID]; // path associated;

    // deal with switching entries
    for (auto node : path){
        --switch_rec[node].sEntryNo; 
    }

    // deal with non switching entries
    for (const int & nsRuleID : assoc_rules){
        int loc_id = sRuleID * rList->list.size() + nsRuleID;
        vector <int> locs = ns_loc_rec_ecmp[loc_id];

        for (int loc : locs){
            auto nsEntries = switch_rec[loc].nsEntries;
            // remove ns occupancy
            auto ns_target = nsEntries.find(nsRuleID);
            if (ns_target != nsEntries.end()){
                if (ns_target->second == 1)
                    nsEntries.erase(ns_target);
                else
                    --ns_target->second;
            }
        }
    }
}

vector<double> bigmac::check_usage(){
    double avg_usage = 0;
    int max_usage = 0;
    int min_usage = rList->list.size() + rList->list_switching.size();

    int non_zero_usage = 0;

    for (auto iter_node = switch_rec.begin(); iter_node != switch_rec.end(); ++iter_node){
        int usage = (iter_node->sEntryNo + iter_node->nsEntries.size());
        if (usage == 0)
            continue;

        ++non_zero_usage;
        avg_usage += usage;
        if (usage > max_usage)
            max_usage = usage;
        if (usage < min_usage)
            min_usage = usage;
    }

    avg_usage /= non_zero_usage; 

    vector<double> res;
    res.push_back(double(min_usage));
    res.push_back(avg_usage);
    res.push_back(double(max_usage));

    return res;
}

int bigmac::check_tags(){
    int dup_tags = 0;

    for (auto sRule_iter = active_sRule.begin(); 
              sRule_iter != active_sRule.end(); 
              ++sRule_iter){ // check every sRule;

        set<int> assoc_ns;

        for (const int & nodeID : sRule_path_map[sRule_iter->first]){ // check every node;
            const map<int, int> & Entry = switch_rec[nodeID].nsEntries;

            for (auto iter = Entry.begin(); iter != Entry.end(); ++iter){
                if (rList->list[iter->first].dep_rule(rList->list_switching[sRule_iter->first])){
                    auto res = assoc_ns.insert(iter->first);
                    if (!res.second){ // already matched
                        ++dup_tags; // tags apply to ns_rule
                        // also need to cal bypass tags;
                    }
                }
            }
        }
    }

    return dup_tags;
}

int bigmac::cal_avg_cost(){
    return total_cost/cache_counter;
}
