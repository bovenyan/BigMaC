#ifndef MAC_MANAGER_H
#define MAC_MANAGER_H

#include "../sharedHeader.h"
#include "BucketTree.h"
#include "BigRule.hpp"
#include "Address.hpp"
#include "routing.h"
#include "Rule.hpp"
#include <set>


class mac_manager {
private:
    string fwd_rule_dir = "";
    string mgmt_rule_dir = "";

    // bucket tree
    int bucket_size_thres = 10;
    bool is_testbed = false;
    int preload_no = 0; 

    // routing and topo
    string topo_dir = "";
    string sav_routes = "";
    string route_rec_name = "default.routes";
    int k_path_no = 2;

    void parse_config(string config_file);

private:
    pipe_line p_line;
    bucket_tree b_tree;
    routing network;   

private:
    std::set<int> cal_path_cover(int mgmt_rule_id);

public:
    mac_manager(){};

    mac_manager(string config_file);

    void place_prealloc(vector<pair<int, int> > & fwd_rule_plc, 
                        vector<pair<int, int> > & mgmt_rule_plc);
    
    void cache_on_request(addr_5tup packet, vector<int> & path, bucket * & buck,
                          vector<int> & fwd_rule_ids, vector<int> & mgmt_rule_ids);

    void get_usage();
};

#endif
