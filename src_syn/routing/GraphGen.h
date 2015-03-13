#ifndef ROUTING
#define ROUTING

#include "../stdafx.h"
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include<boost/graph/adjacency_list.hpp>
#include<boost/algorithm/string.hpp>

using namespace boost;
using std::vector; using std::pair;
using std::string;

class InternetGraph{
    private:
	typedef property<vertex_index_t, int> VertexProperty;
	typedef property<edge_weight_t, int> EdgeProperty;

	typedef adjacency_list<vecS, vecS, undirectedS, VertexProperty, EdgeProperty > Graph_T;
	typedef graph_traits<Graph_T>::vertex_descriptor vertex_descriptor;

    public:
    vector<vector<int> > path_map; // src [ parent(dst1), parent(dst2), ...] 
	vector<int> edge_ID; // map  edgeID -> graphID
	Graph_T Graph;
    int vertice_no;
	
    public:
    InternetGraph();

    void CalShortestPath(string sp_file);
	void ReadGraph(const char file_name [], int edge_node_no = 50);
    void ReadPath(string sp_file);
	
    vector<int> query_path(int src_eID, int dst_eID);
    string print_path(int src, int dst);

};

class FatTreeGraph{
    public:
    const int kVal;
    const int kVal_2;
    const int kVal_hf;

    FatTreeGraph(){};
    FatTreeGraph(int k);

    inline vector<int> query_path(const int & src, const int & dst, 
                                  const int & a_idx, const int & c_idx);
    inline vector<vector<int> > query_all_path(const int & src, const int & dst, int kpath);
    ~FatTreeGraph();
};

#endif
