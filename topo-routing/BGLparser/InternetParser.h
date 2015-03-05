#include "stdafx.hpp"
#include <boost/graph/dijkstra_shortest_paths.hpp>

using namespace boost;
using std::vector; using std::pair;
// define graph property
//struct is_edge_t{
//	vertex_property_tag isEdge;
//};
//typedef property<is_edge_t, bool> IsEdgeProperty;


class GenGraph{
	public:
	typedef property<vertex_index_t, int> VertexProperty;
	typedef property<edge_weight_t, int> EdgeProperty;

	typedef adjacency_list<vecS, vecS, undirectedS, VertexProperty, EdgeProperty > Graph_T;
	typedef graph_traits<Graph_T>::vertex_descriptor vertex_descriptor;

	public:
	Graph_T Graph;
	vector<int> edge_ID;

	vector<vector<int> > path_map; // src [ parent(dst1), parent(dst2), ...] 
	vector<vector<pair<int,int> > > associate_path_map; // vetex [ (src, dst), (src, dst), ...] 

	public:
	void CalShortestPath();
	void CalAssociateMap();
	vector<int> query_path(int src, int dst, bool debug = false);
};

class InternetGraph:GenGraph{
	public:
	InternetGraph();

	void ReadGraph(const char file_name [], int vertice_no = 100, int edge_node_no = 50);

	~InternetGraph();
};

class FatTreeGraph:GenGraph{

};
