#ifndef ROUTING_H
#define ROUTING_H

#include "../sharedHeader.h"
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <vector>

using namespace boost;
using std::string;
using std::vector;

class Topology {
    typedef adjacency_list<vecS, vecS, undirectedS, property<vertex_index_t, int >,
            property<edge_weight_t, int > > Graph;
    typedef graph_traits<Graph>::vertex_descriptor vertex_descriptor;
    typedef graph_traits<Graph>::vertices_size_type size_type;
    typedef graph_traits<Graph>::adjacency_iterator Iter_adj;

private:
    Graph topo;
    vector<vector<vertex_descriptor> > shortestParents;

    vector<vector<vector<vector<int> > > > shortestKPaths;
    
    logging_src::severity_logger< severity_level > logger_routing;

public:
    Topology(string filename );
    
    void allShortest();
    vector<int> calShortest(int src, int dst);
    void calShortestK(int K);

    vector<int> FetchShortestPath(int src, int dst);

    // debug
    void printShortestPath(int src, int dst);
    void printAllShortestPath();
};

#endif
