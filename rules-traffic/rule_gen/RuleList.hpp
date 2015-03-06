#ifndef RULELIST_H
#define RULELIST_H

#include "stdafx.h"
#include "Address.hpp"
#include "Rule.hpp"
#include <unordered_map>

using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;

class rule_list
{
public:
    vector<p_rule> list;
    vector<p_rule> list_switching;

    std::unordered_map <uint32_t, std::vector<uint32_t> > dep_map;

    // handler
    rule_list();
    rule_list(std::string &, bool = false);

    // generation
    void evolve_gen(int offspring = 2, double scale = 2, double offset = 1);
    vector<addr_5tup> header_prep();
    
    // analyzing
    void obtain_dep();
    
    // debug
    inline void print(const std::string &);
};


rule_list::rule_list() {}

rule_list::rule_list(string & filename, bool test_bed) {
    ifstream file;
    file.open(filename.c_str());
    string sLine = "";
    getline(file, sLine);
    while (!file.eof()) {
        p_rule sRule(sLine, test_bed);
        list.push_back(sRule);
        getline(file, sLine);
    }
    file.close();

    if (test_bed) { // remove rule with same hostpair
        for(auto iter = list.begin(); iter != list.end(); ++iter) {
            for (auto iter_cp = iter+1; iter_cp != list.end(); ) {
                if (*iter == *iter_cp) 
                    iter_cp = list.erase(iter_cp);
		else
			++iter_cp;
            }
        }
    }
}

void rule_list::evolve_gen(int offspring, double scale, double offset){
    // evolving 
    for (auto iter = list.begin(); iter != list.end(); ++iter){
        auto res = (*iter).evolve_rule(offspring, scale, offset);
        list_switching.insert(list_switching.end(), res.begin(), res.end());
    }

    // remove overlap
    for (auto iter_r = list.end() - 1; iter_r != list.begin() + 1; --iter_r){
        for (auto iter = list.begin(); iter != iter_r; ++iter){
            if ((*iter).dep_rule(*iter_r)){
                iter_r = list.erase(iter_r);
                break;
            }
        }
    }
}

vector<addr_5tup> rule_list::header_prep(){
    vector<addr_5tup> headers;

    for (auto iter = list_switching.begin(); iter != list_switching.end(); ++iter){
        vector<addr_5tup> all_corners = (*iter).get_all_corner();
        headers.insert(headers.end(), all_corners.begin(), all_corners.end());
    }
    
    return headers;
}


void rule_list::obtain_dep() { // obtain the dependency map
    for(uint32_t idx = 0; idx < list.size(); ++idx) {
        vector <uint32_t> dep_rules;
        for (uint32_t idx1 = 0; idx1 < idx; ++idx1) {
            if (list[idx].dep_rule(list[idx1])) {
                dep_rules.push_back(idx1);
            }
        }
        dep_map[idx] = dep_rules;
    }
}

/*
 * debug and print
 */
void rule_list::print(const string & filename) {
    ofstream file;
    file.open(filename.c_str());
    for (vector<p_rule>::iterator iter = list.begin(); iter != list.end(); iter++) {
        file<<iter->get_str()<<endl;
    }
    file.close();
}


#endif
