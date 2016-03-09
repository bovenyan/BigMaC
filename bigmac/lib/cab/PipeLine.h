#ifndef PIPELINE_H
#define PIPELINE_H

#include "stdafx.h"
#include "Address.hpp"
#include "BigRule.hpp"
#include <unordered_map>

class pipe_line {
private:
    boost::log::sources::logger lg;
    bool test_bed; // testbed would use all prefix rules
    std::unordered_map <uint32_t, std::vector<uint32_t> > fwd_dep_map;
    std::unordered_map <uint32_t, std::vector<uint32_t> > mgmt_dep_map;
    
    void obtain_dep(); // obtain the dependency between rules

public:
    std::vector<fwd_rule> fwd_table;
    std::vector<mgmt_rule> mgmt_table;

    // linear search
    std::pair<int, int> get_matched_rules(const addr_5tup & packet);
    std::pair<r_rule, r_rule> get_micro_rules(const addr_5tup & packet);


public:
    pipe_line ();
    pipe_line (std::string & fwd_table_file, 
            std::string & mgmt_table_file, 
            bool test_bed = false);

    // debug
    void print(const std::string &);
};

#endif
