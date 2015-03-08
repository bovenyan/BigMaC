#include "switchTracer.h"


switch_tracker::switch_tracker(rule_list * rL){
    rList = rL;
}

void switch_tracker::MaC(const int sRuleID, const vector<int> & path, double curTime){
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

    for (auto nsRule : assoc_rules){
        auto res = nsEntries.insert(std::make_pair(nsRule, curTime));
        
        if (!res.second)
            nsEntries[nsRule] = curTime;
    }
}

void switch_tracker::clear_obsolte(double curTime, double timeout){
    for (auto iter_s = bs_sRule.begin(); iter_s != bs_sRule.end(); ++iter_s ){
        if (iter_s->second.activeTime + timeout < curTime){ // this switching entry is to be del
            
            const vector<int> & path = iter_s->second.path;
            const vector<int> & assoc_rules = rList->assoc_map[iter_s->first];
            set<int> assoc_rules_set(assoc_rules.begin(), assoc_rules.end());

            // check associated entries: 
            for (auto iter_node = path.begin(); iter_node < path.end(); ++iter_node){
                --switch_rec[*iter_node].sEntryNo; // delete current switching entry

                auto nsEntries = switch_rec[*iter_node].nsEntries;
                // delete the dependent non switching entry
                for (auto iter_ns = nsEntries.begin(); iter_ns != nsEntries.end(); ++iter_ns){
                    if (iter_ns->second + timeout < curTime &&  // timed out
                        assoc_rules_set.find(iter_ns->first) != assoc_rules_set.end()){ // find
                        nsEntries.erase(iter_ns);
                    }
                }
            }


            bs_sRule.erase(iter_s); 
        }
    }
}

vector<double> switch_tracker::check_usage(){
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

    vector<double> res;
    res.push_back(double(min_usage));
    res.push_back(avg_usage);
    res.push_back(double(max_usage));

    return res;
}
