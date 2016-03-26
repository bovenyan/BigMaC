#ifndef ROUTING_H
#define ROUTING_H

#include "../sharedHeader.h"
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <vector>

using namespace boost;
using std::string;
using std::vector;

class routing {
    typedef adjacency_list<vecS, vecS, undirectedS, property<vertex_index_t, int >,
            property<edge_weight_t, int > > Graph;
    typedef graph_traits<Graph>::vertex_descriptor vertex_descriptor;
    typedef graph_traits<Graph>::vertices_size_type size_type;
    typedef graph_traits<Graph>::adjacency_iterator Iter_adj;

private:
    Graph topo;
    // routing record
    vector<vector<vertex_descriptor> > shortest_parents;
    vector<vector<vector<vector<int> > > > shortest_k_paths;

    // rule-routing record
    vector<vector<int> > rule_routing_map;

    logging_src::severity_logger< severity_level > logger_routing;

public:
    // read a routing file
    routing(string filename);
    
    // cal culate path
    void cal_all_shortest();
    vector<int> cal_e2e_shortest(int src, int dst);
    void cal_all_shortest_k(int K);
    vector<int> get_e2e_shortest(int src, int dst);

    // assign rule-path
    void assign_paths_rand(int rule_no, int k, string filename);
    void load_paths(string filename);

    vector<int> get_rule_path(int rule_id);

    // debug
    void print_e2e_shortest(int src, int dst);
    void print_all_shortest();
};

#endif
