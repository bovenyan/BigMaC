#ifndef BIG_RULE_H
#define BIG_RULE_H

#include "stdafx.h"
#include "Rule.hpp"

class fwd_rule : public p_rule {
private:
    int occupancy; // # of buckets associated
public:
    bool is_cached;

    inline fwd_rule() : p_rule() {
        occupancy = 0;
        is_cached = false;
    }

    inline fwd_rule(const fwd_rule & another)
        : p_rule((p_rule)another) {
        occupancy = 0;
        is_cached = false;
    }

    inline fwd_rule (const std::string & file_name,
                     bool test_bed)
        : p_rule(file_name, test_bed) {
        occupancy = 0;
        is_cached = false;
    }

    inline void inc_occupancy(){++occupancy;}
    inline void dec_occupancy(){--occupancy;}
    inline void clear_occupancy(){occupancy = 0;}
    inline int get_occupancy(){return occupancy;}

    inline void clear_stats(){occupancy = 0; hit = false; is_cached = false;}
};

class mgmt_rule : public p_rule {
private:
    int occupancy; // # of buckets associated
public:
    bool is_cached;

    inline mgmt_rule() : p_rule() {
        occupancy = 0;
        is_cached = false;
    }

    inline mgmt_rule(const mgmt_rule & another)
        : p_rule((p_rule) another) {
        occupancy = 0;
        is_cached = false;
    }

    inline mgmt_rule (const std::string & file_name,
                      bool test_bed)
        : p_rule(file_name, test_bed) {
        occupancy = 0;
        is_cached = false;
    }

    inline void inc_occupancy(){++occupancy;}
    inline void dec_occupancy(){--occupancy;}
    inline void clear_occupancy(){occupancy = 0;}
    inline int get_occupancy(){return occupancy;}
    
    inline void clear_stats(){occupancy = 0; hit = false; is_cached = false;}
};
#endif
