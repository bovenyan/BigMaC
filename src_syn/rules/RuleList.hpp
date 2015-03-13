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

class rule_list
{
public:
    vector<p_rule> list;
    vector<p_rule> list_switching;

    vector<vector<int> > assoc_map;
    vector<vector<int> > ns_dep_map;

    // handler
    inline rule_list();
    inline rule_list(const char []);
    inline rule_list(const char [], const char []);

    // generation
    inline void evolve_gen(const char file_name [], int offspring = 2, double scale = 2, double offset = 1);
    vector<addr_5tup> header_prep();

    // serach related
    int search_match_sRule(addr_5tup packet);
    pair<int,int> search_match_rules(addr_5tup packet);
    vector<int> dep_ns_rule(p_rule nsRule);
    vector<int> assoc_ns_rule(p_rule sRule);
    
    // analyzing
    void obtain_ns_assoc();
    void obtain_ns_dep();
    
    // debug
    inline void print(const std::string &);
};


inline rule_list::rule_list() {}

inline rule_list::rule_list(const char filename []) {
    ifstream file;
    file.open(filename);
    string sLine = "";
    getline(file, sLine);
    while (!file.eof()) {
        p_rule sRule(sLine);
        list.push_back(sRule);
        getline(file, sLine);
    }
    file.close();
}

inline rule_list::rule_list(const char sRule_file [], const char nsRule_file []){
    ifstream file(sRule_file);

    for (string line; getline(file, line);){
        p_rule sRule(line);
        list_switching.push_back(sRule);
    }

    file.close();

    file.open(nsRule_file);

    for (string line; getline(file, line);){
        p_rule nsRule(line);
        list.push_back(nsRule);
    }

    file.close();
}

inline void rule_list::evolve_gen(const char file_name [], int offspring, double scale, double offset){ 
    // evolving 
    for (auto iter = list.begin(); iter != list.end(); ++iter){
        auto res = (*iter).evolve_rule(offspring, scale, offset);
        list_switching.insert(list_switching.end(), res.begin(), res.end());
    }

    // remove overlap
    for (auto iter_r = list_switching.end() - 1; iter_r != list_switching.begin() + 1; --iter_r){
        for (auto iter = list_switching.begin(); iter != iter_r; ++iter){
            if ((*iter).dep_rule(*iter_r)){
                iter_r = list_switching.erase(iter_r);
                break;
            }
        }
    }

    // print the switching rules
    ofstream file(file_name);
    for (p_rule rule : list_switching){
        file<<rule.get_str()<<endl;
    }
    file.close();
}

inline vector<addr_5tup> rule_list::header_prep(){
    vector<addr_5tup> headers;

    for (auto iter = list_switching.begin(); iter != list_switching.end(); ++iter){
        vector<addr_5tup> all_corners = (*iter).get_all_corner();
        headers.insert(headers.end(), all_corners.begin(), all_corners.end());
    }
    
    return headers;
}

inline int rule_list::search_match_sRule(addr_5tup packet){
    for(int i = 0; i < list_switching.size(); ++i)
        if (list_switching[i].packet_hit(packet))
            return i;

    cout<<"error RuleList.hpp : packet not matched"<<endl;
    return -1;
}

inline pair<int, int> rule_list::search_match_rules(addr_5tup packet){
    pair<int, int> res;

    for(int i = 0; i < list_switching.size(); ++i){
        if (list_switching[i].packet_hit(packet)){
            res.first = i;
            break;
        }
    }

    for(int j = 0; j < list.size(); ++j){
        if (list[j].packet_hit(packet)){
            res.second = j;
            break;
        }
    }

    return res;
}

inline vector<int> rule_list::dep_ns_rule(p_rule nsRule){
    vector<int> res;

    for (int i = 0; i < list.size(); ++i){
        if (list[i].dep_rule(nsRule)){
            res.push_back(i);
        }
    }
    return res;
}

inline vector<int> rule_list::assoc_ns_rule(p_rule sRule){
    vector<int> res;

    for (int i = 0; i < list.size(); ++i){
        if (list[i].dep_rule(sRule)){
            res.push_back(i);
        }
    }

    return res;
}


inline void rule_list::obtain_ns_dep() { // obtain the dependency map
    for (int i = 0; i < list.size(); ++i){
        ns_dep_map.push_back(dep_ns_rule(list[i]));
    }
}

inline void rule_list::obtain_ns_assoc() {
    for (int i = 0; i < list_switching.size(); ++i){
        assoc_map.push_back(assoc_ns_rule(list_switching[i]));
    }
}

/*
 * debug and print
 */
inline void rule_list::print(const string & filename) {
    ofstream file;
    file.open(filename.c_str());
    for (vector<p_rule>::iterator iter = list.begin(); iter != list.end(); iter++) {
        file<<iter->get_str()<<endl;
    }
    file.close();
}


#endif
