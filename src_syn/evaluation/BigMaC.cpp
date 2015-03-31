#include "BigMaC.h"
#include <algorithm>

bigmac::bigmac(rule_list * rL, int nNo, const vector<vector<int> > & path_map){
    sRule_path_map = path_map;
    rList = rL;
    node_no = nNo;

    tag_counter = 0;
   
    // init flow table record
    switch_rec = vector<flow_table> (node_no, flow_table());
}

bigmac::bigmac(rule_list * rL, int nNo, const vector<vector<vector<int> > > & path_map_ecmp){
    sRule_path_map_ecmp = path_map_ecmp;
    rList = rL;
    node_no = nNo;

    tag_counter = 0;

    // init flow table record
    switch_rec = vector<flow_table> (node_no, flow_table());
}

void bigmac::MaC(const int & sRuleID){
    if (active_sRule.find(sRuleID) == active_sRule.end()) // not found
        active_sRule[sRuleID] = 1;
    else { // already cached
        ++active_sRule[sRuleID];
        return;
    }

    // obtain NS rules and path
    const vector<int> & assoc_rules = rList->get_assoc_ns(sRuleID);
    const vector<int> & path = sRule_path_map[sRuleID]; 

    // optimize placement (NS placed on one switch)
    vector<int> usage_rec; // cost
    vector<int> incre_rec; // incre

    for (auto node : path){
        ++switch_rec[node].sEntryNo; // ++ switching rule no

        const map<int, int> & nsEntries = switch_rec[node].nsEntries;
        int cost = assoc_rules.size();

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
        usage_rec.push_back(nsEntries.size());
        incre_rec.push_back(cost);
    }

    // obtain the min-max * cost 
    auto max_iter = std::max_element(usage_rec.begin(), usage_rec.end());
    int min_cost = INT_MAX;
    int min_max = INT_MAX;
    int choosen_node = -1;
    int choosen_node_2 = -1;
    for (int i = 0; i < usage_rec.size(); ++i){
        if (usage_rec[i] + incre_rec[i] < *max_iter){ // min cost after min_max
            if (min_cost > incre_rec[i]){
                min_cost = incre_rec[i];
                choosen_node = path[i];
            }
        }
        if (min_max > usage_rec[i]+incre_rec[i]){ // min_max
            min_max = usage_rec[i]+incre_rec[i];
            choosen_node_2 = path[i];
        }
    }
    
    if (choosen_node == -1)
        choosen_node = choosen_node_2;

    // applying ns rules
    map<int, int> & nsEntries = switch_rec[choosen_node].nsEntries;

    for (auto nsRuleID : assoc_rules){
        if (nsEntries.find(nsRuleID) != nsEntries.end())
            ++nsEntries[nsRuleID]; // inc occupancy
        else{ // insert new rules
            nsEntries[nsRuleID] = 1;

            // verification: applying filtering tags;
            //  for ns: nsRuleID.dep 
            //      if (ns cached on node other than choosen_no)  
            //          for s : nsRuleID.assoc
            //              if (s cap ns cap nsRuleID != emp)
            //                  check s path crossing ? choosen_node;
            
            // check breach
            const vector<int> & dep_rules = rList->get_dep_ns(nsRuleID);
            for (int dep_nsRuleID : dep_rules){
                // check assoc path
                
                if (ns_locs.find(dep_nsRuleID) != ns_locs.end()){ // cached
                    
                }
            }
            // check duplicate
        }
        // sRule-nsRule : location = sRule*nsSize + nsRule;
        ns_loc_rec[sRuleID * rList->nsRule_size() + nsRuleID] = choosen_node;
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

    const vector<int> & assoc_rules = rList->get_assoc_ns(sRuleID);
    const vector<int> & path = sRule_path_map[sRuleID]; // path associated;

    // deal with switching entries
    for (auto node : path){
        --switch_rec[node].sEntryNo; 
    }

    // deal with non switching entries
    for (const int & nsRuleID : assoc_rules){
        int loc = sRuleID * rList->nsRule_size() + nsRuleID;
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

    const vector<int> & assoc_rules = rList->get_assoc_ns(sRuleID);
    vector<int> choosen_node_list;

    for (auto & path : sRule_path_map_ecmp[sRuleID]){
    
        // get non-assoc pieces - puting every rule on one switch first...
        int min_cost = rList->nsRule_size();
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
        ns_loc_rec_ecmp[sRuleID * rList->nsRule_size() + nsRuleID] = vector<int>();

        for (auto choosen_node : choosen_node_list){
            map<int, int> & nsEntries = switch_rec[choosen_node].nsEntries;

            if (nsEntries.find(nsRuleID) != nsEntries.end())
                ++nsEntries[nsRuleID]; // inc occupancy
            else
                nsEntries[nsRuleID] = 1;
            ns_loc_rec_ecmp[sRuleID * rList->nsRule_size() + nsRuleID].push_back(choosen_node);
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

    const vector<int> & assoc_rules = rList->get_assoc_ns(sRuleID);
    const vector<int> & path = sRule_path_map[sRuleID]; // path associated;

    // deal with switching entries
    for (auto node : path){
        --switch_rec[node].sEntryNo; 
    }

    // deal with non switching entries
    for (const int & nsRuleID : assoc_rules){
        int loc_id = sRuleID * rList->nsRule_size() + nsRuleID;
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
    int min_usage = rList->nsRule_size() + rList->sRule_size();

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
                if (rList->nsRuleAt(iter->first).overlap(rList->sRuleAt(sRule_iter->first))){
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

