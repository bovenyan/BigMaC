#ifndef RULELIST_H
#define RULELIST_H

#include "stdafx.h"
#include "Address.hpp"
#include "Rule.hpp"
#include <unordered_map>

using std::ifstream;
using std::ofstream;
using std::string;

class rule_list
{
public:
    std::vector<p_rule> list;
    std::unordered_map <uint32_t, std::vector<uint32_t> > dep_map;

    // handler
    inline rule_list();
    inline rule_list(std::string &, bool = false);
    
    // analyzing
    inline int linear_search(const addr_5tup &);
    
    inline void obtain_dep();
    inline void rule_dep_analysis();
    
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

/* member func
 */
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

int rule_list::linear_search(const addr_5tup & packet) {
    for (size_t i = 0; i < list.size(); ++i) {
        if (list[i].packet_hit(packet))
            return i;
    }
    return -1;
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

void rule_list::rule_dep_analysis() {
    ofstream ff("rule rec");
    for (uint32_t idx = 0; idx < list.size(); ++idx) {
        ff<<"rule : "<< list[idx].get_str() << endl;
        for ( uint32_t idx1 = 0; idx1 < idx; ++idx1) {
            auto result = list[idx].join_rule(list[idx1]);
            if (result.second)
                ff << result.first.get_str()<<endl;

        }
        ff<<endl;
    }
}





#endif
