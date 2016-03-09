#ifndef BIG_RULE_H
#define BIG_RULE_H

#include "stdafx.h"
#include "Rule.hpp"

class fwd_rule : public p_rule {
    public:
        inline fwd_rule() : p_rule() {};

        inline fwd_rule(const fwd_rule & another) 
            : p_rule((p_rule)another) {};

        inline fwd_rule (const std::string & file_name, 
                bool test_bed)
            : p_rule(file_name, test_bed) {};

};

class mgmt_rule : public p_rule {
    public:
        inline mgmt_rule() : p_rule() {};

        inline mgmt_rule(const mgmt_rule & another)
            : p_rule((p_rule) another){};

        inline mgmt_rule (const std::string & file_name,
                bool test_bed)
            : p_rule(file_name, test_bed) {};
};
#endif
