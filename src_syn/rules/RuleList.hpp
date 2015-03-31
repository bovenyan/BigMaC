/*
 * FileName: RuleList.hpp
 * Contributer: Bo Yan (NYU)
 * Description:
 *      Structure for Rule Sets
 */

#ifndef RULELIST_H
#define RULELIST_H

#include "../stdafx.h"
#include "Address.hpp"
#include "Rule.hpp"
#include <map>

using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;
using std::map;

/* rule_list: Rule Set
 *  list: Non-switching rule set
 *  list-switching: Switching rule set
 *  assoc_map: map from switching rule -> overlapped non switching rules
 *  ns_dep_map: map from non-switching rule -> dependent ns rules 
 */
class rule_list
{
  private:
    vector<p_rule> sList;
    vector<p_rule> nsList;
    vector<vector<int> > assoc_s2ns_map;
    vector<vector<int> > dep_ns_map;
    vector<vector<int> > assoc_ns2s_map; // Mar 28

  private:
    vector<int> cal_dep_nsRule(int nsRuleID);  // Mar 28
    vector<int> cal_assoc_nsRule(int sRuleID); // Mar 28
    vector<int> cal_assoc_sRule(int nsRuleID); // Mar 28

  public:
    // constructor
    inline rule_list();
    inline rule_list(const char []);
    inline rule_list(const char [], const char []);

    // initialize
    inline void evolve_gen(const char file_name [], int offspring = 2, double scale = 2, double offset = 1);
    void cal_assoc_dep(bool assoc, bool dep); // 
    vector<addr_5tup> header_prep(); // generate headers

    // access
    int sRule_size() const;
    int nsRule_size() const;
    const p_rule & sRuleAt(int sRuleID) const; // Mar 20
    const p_rule & nsRuleAt(int nsRuleID) const; // Mar 20
    const vector<int> & get_assoc_ns(const int & sRuleID) const; // Mar 20
    const vector<int> & get_dep_ns(const int & nsRuleID) const;  // Mar 20
    const vector<int> & get_assoc_s(const int & nsRuleID) const; // Mar 28

    // linear search related
    int search_match_sRule(addr_5tup packet);
    pair<int,int> search_match_rules(addr_5tup packet);

    // debug
    void print(const char[], const char[]) const;
};


inline rule_list::rule_list() {}

inline rule_list::rule_list(const char filename []) {
    ifstream file;
    file.open(filename);
    string sLine = "";
    getline(file, sLine);
    while (!file.eof()) {
        p_rule sRule(sLine);
        nsList.push_back(sRule);
        getline(file, sLine);
    }
    file.close();
}

inline rule_list::rule_list(const char sRule_file [], const char nsRule_file []){
    ifstream file(sRule_file);

    for (string line; getline(file, line);){
        p_rule sRule(line);
        sList.push_back(sRule);
    }

    file.close();

    file.open(nsRule_file);

    for (string line; getline(file, line);){
        p_rule nsRule(line);
        nsList.push_back(nsRule);
    }

    file.close();
}

inline void rule_list::evolve_gen(const char file_name [], int offspring, double scale, double offset){ 
    // evolving 
    for (auto iter = nsList.begin(); iter != nsList.end(); ++iter){
        auto res = (*iter).evolve_rule(offspring, scale, offset);
        sList.insert(sList.end(), res.begin(), res.end());
    }

    // remove overlap
    for (auto iter_r = sList.end() - 1; iter_r != sList.begin() + 1; --iter_r){
        for (auto iter = sList.begin(); iter != iter_r; ++iter){
            if ((*iter).overlap(*iter_r)){
                iter_r = sList.erase(iter_r);
                break;
            }
        }
    }

    // print the switching rules
    ofstream file(file_name);
    for (p_rule rule : sList){
        file<<rule.get_str()<<endl;
    }
    file.close();
}


inline int rule_list::search_match_sRule(addr_5tup packet){
    for(int i = 0; i < sList.size(); ++i)
        if (sList[i].packet_hit(packet))
            return i;
    cout<<"error RuleList.hpp : packet not matched"<<endl;
    return -1;
}

inline pair<int, int> rule_list::search_match_rules(addr_5tup packet){
    pair<int, int> res;

    for(int i = 0; i < sList.size(); ++i){
        if (sList[i].packet_hit(packet)){
            res.first = i;
            break;
        }
    }

    for(int j = 0; j < nsList.size(); ++j){
        if (nsList[j].packet_hit(packet)){
            res.second = j;
            break;
        }
    }

    return res;
}

inline vector<int> rule_list::cal_dep_nsRule(int nsRuleID){
    vector<int> res;

    for (int i = 0; i < nsRuleID; ++i){
        if (nsList[i].overlap(nsList[nsRuleID]))
            res.push_back(i);
    }

    return res;
}

inline vector<int> rule_list::cal_assoc_nsRule(int sRuleID){
    vector<int> res;

    for (int i = 0; i < nsList.size(); ++i){
        if (nsList[i].overlap(sList[sRuleID]))
            res.push_back(i);
    }

    return res;
}

inline vector<int> rule_list::cal_assoc_sRule(int nsRuleID){
    vector<int> res;

    for (int i = 0; i < sList.size(); ++i){
        if (sList[i].overlap(nsList[nsRuleID]))
            res.push_back(i);
    }

    return res;
}

inline void rule_list::cal_assoc_dep(bool cal_assoc, bool cal_dep){
    if (cal_assoc){
        for (int i = 0; i < sList.size(); ++i)
            assoc_s2ns_map.push_back(cal_assoc_nsRule(i));
        for (int i = 0; i < nsList.size(); ++i)
            assoc_ns2s_map.push_back(cal_assoc_sRule(i));
    }

    if (cal_dep){
        for (int i = 0; i < nsList.size(); ++i)
            dep_ns_map.push_back(cal_dep_nsRule(nsList[i]));
    }
}

inline vector<addr_5tup> rule_list::header_prep(){
    vector<addr_5tup> headers;

    for (auto iter = sList.begin(); iter != sList.end(); ++iter){
        vector<addr_5tup> all_corners = (*iter).get_all_corner();
        headers.insert(headers.end(), all_corners.begin(), all_corners.end());
    }
    
    return headers;
}

inline int rule_list::sRule_size() const {
    return sList.size();
}

inline int rule_list::nsRule_size() const{
    return nsRule_size();
}

inline const p_rule & rule_list::sRuleAt(int sRuleID) const{
    return sList[sRuleID]; 
}

inline const p_rule & rule_list::nsRuleAt(int nsRuleID) const{
    return nsList[nsRuleID];
}

inline const vector<int> & rule_list::get_assoc_ns(const int & sRuleID) const{
    return assoc_s2ns_map[sRuleID];
}

inline const vector<int> & rule_list::get_assoc_s(const int & nsRuleID) const{
    return assoc_ns2s_map[nsRuleID];
}

inline const vector<int> & rule_list::get_dep_ns(const int & nsRuleID) const{
    return dep_ns_map[nsRuleID];
}

inline void rule_list::print(const char sfile[], const char nsfile[]) const{
    ofstream file(sfile);
    for (auto iter = sList.begin(); iter != sList.end(); iter++) 
        file<<iter->get_str()<<endl;
    
    file.close();
    file.open(nsfile);
    
    for (auto iter = nsList.begin(); iter != nsList.end(); iter++) 
        file<<iter->get_str()<<endl;
    
    file.close();
}


#endif
