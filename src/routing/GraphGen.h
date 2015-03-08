#include "stdafx.hpp"
#include <boost/graph/dijkstra_shortest_paths.hpp>

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
	vector<int> edge_ID;
	Graph_T Graph;
	
    public:
    InternetGraph();

    void CalShortestPath(string sp_file);
	void ReadGraph(const char file_name [], int vertice_no = 100, int edge_node_no = 50);
	
    vector<int> query_path(int src, int dst, bool debug = false);

    void ReadPath(string sp_file);

	~InternetGraph();
};

class FatTreeGraph{
    public:
    const int kVal;
    const int kVal_2;
    const int kVal_hf;
    FatTreeGraph(int k);

    inline vector<int> query_path(const int & src, const int & dst, 
                                  const int & a_idx, const int & c_idx);

    ~FatTreeGraph();
};
