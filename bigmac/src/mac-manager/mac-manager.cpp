#include "mac-manager.h"
#include <boost/algorithm/string.hpp>

using boost::split;
using boost::is_any_of;
using std::ifstream;


mac_manager::mac_manager(string config_file) {
    parse_config(config_file);

    if (fwd_rule_dir != "" && mgmt_rule_dir != "") {
        p_line = pipe_line(fwd_rule_dir, mgmt_rule_dir, is_testbed);
        b_tree = bucket_tree (p_line, bucket_size_thres, is_testbed, preload_no);
        b_tree.pre_alloc();

        cout<<"Tree inititation Done"<<endl;
    } else {
        cout<<"Error config, tree not built"<<endl;
        exit(1);
    }

    if (topo_dir != "") {
        network = routing(topo_dir);

        if (sav_routes == "") { // create new path
            network.assign_paths_rand(p_line.fwd_table.size(), k_path_no, route_rec_name);
        } else {  // load existing path
            network.load_paths(sav_routes);
        }
    } else {
        cout<<"Error config, routing not built"<<endl;
        exit(1);
    }
}

void mac_manager::parse_config(string config_file_name) {
    // parse config file
    ifstream conf_file(config_file_name);
    string line;

    while (getline(conf_file, line)) {
        vector<string> res;
        split(res, line, is_any_of(" \t"));

        string item;

        item = "fwd_rule_list";
        if (line.compare(0, item.length(), item) == 0) {
            fwd_rule_dir = res[1];
            continue;
        }

        item = "mgmt_rule_list";
        if (line.compare(0, item.length(), item) == 0) {
            mgmt_rule_dir = res[1];
            continue;
        }

        item = "threshold";
        if (line.compare(0, item.length(), item) == 0) {
            bucket_size_thres = std::atoi(res[1].c_str());
            continue;
        }

        item = "testbed";
        if (line.compare(0, item.length(), item) == 0) {
            if (res[1] == "yes")
                is_testbed = true;
            continue;
        }

        item = "preload";
        if (line.compare(0, item.length(), item) == 0) {
            preload_no = std::atoi(res[1].c_str());
            continue;
        }

        item = "topology";
        if (line.compare(0, item.length(), item) == 0) {
            topo_dir = res[1];
            continue;
        }

        item = "sav_routes";
        if (line.compare(0, item.length(), item) == 0) {
            sav_routes = res[1];
            continue;
        }

        item = "rec_name";
        if (line.compare(0, item.length(), item) == 0) {
            route_rec_name = res[1];
            continue;
        }

        item = "k_path_no";
        if (line.compare(0, item.length(), item) == 0) {
            k_path_no = std::atoi(res[1].c_str());
            continue;
        }

        // TODO: add more items

    }

}

void mac_manager::cache_on_request(addr_5tup packet, vector<int> & path,
                                   bucket * & buck, vector<int> & fwd_rule_ids,
                                   vector<int> & mgmt_rule_ids) {
    auto res = b_tree.search_bucket(packet, b_tree.root);
    buck = res.first;

}

void mac_manager::place_prealloc(vector<pair<int, int> > & fwd_rule_plc,
                                 vector<pair<int, int> > & mgmt_rule_plc) {
    // calculate all the routes for all the fwding placement
    vector<unordered_set<int> > (network.get_topo_size(),
                                 unordered_set<int>());


    // calculate the "path cover problem" for all the mgmt placement
    // TODO: the pre-alloc mgmt rules
    // TODO: the assoc paths (rule)...
    // TODO: the path cover
}
