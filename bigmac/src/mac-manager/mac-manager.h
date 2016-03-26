#ifndef MAC_MANAGER_H
#define MAC_MANAGER_H

#include "../sharedHeader.h"
#include "BucketTree.h"
#include "BigRule.hpp"
#include "Address.hpp"
#include "routing.h"
#include "Rule.hpp"

class mac_manager {
private:
    pipe_line p_line;
    bucket_tree b_tree;
    routing network;   

public:
    mac_manager(){};

    mac_manager(string config_file);

    void cache_on_request(addr_5tup packet, vector<int> & path,
                          vector<int> fwd_rule_ids, vector<int> mgmt_rule_ids);

    void gather_usage();
};

#endif
