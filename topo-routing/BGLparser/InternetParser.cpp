#include "InternetParser.h"
#include <fstream>
using namespace std;
using namespace boost;

void GenGraph::CalShortestPath(){
	typedef graph_traits<Graph_T>::vertex_iterator vertex_iterator;
	typedef graph_traits<Graph_T>::vertices_size_type size_type;
	typedef property_map<Graph_T, vertex_index_t>::type IndexMap;
	IndexMap index = get(vertex_index, Graph);

	// Cal shortest path for each vertex
	for (int i = 0; i < edge_ID.size(); ++i){
		vertex_iterator vi = vertices(Graph).first + edge_ID[i];
		vertex_descriptor src = *vi;
		
		// Setup parent property map to record shortest-path tree
		vector<vertex_descriptor> parent(num_vertices(Graph));
		
		// All vertices started out as there own parent
		for (size_type p = 0; p < num_vertices(Graph); ++ p)
			parent[p] = p;

		dijkstra_shortest_paths(Graph, src, predecessor_map(&parent[0]));

		vector <int> shortestVec(num_vertices(Graph), 0);
		for (size_type p = 0; p < num_vertices(Graph); ++p){
			shortestVec[p] = index[parent[p]];
		}
		path_map.push_back(shortestVec);
	}
}

vector<int> GenGraph::query_path(int src, int dst, bool debug){
	vector<int> path;
	
	while (dst != src){
		path.push_back(dst);
		dst = path_map[src][dst];
	}
	path.push_back(src);

	// print path
	if (debug){
		for (auto iter = path.begin(); iter != path.end(); ++iter)
			cout<<*iter<<" ";
		cout<<endl;
	}

	return path;
}

void GenGraph::CalAssociateMap(){

}

InternetGraph::InternetGraph(){

}

bool pair_comp(pair<int, int> lhs, pair<int, int> rhs){
	return lhs.second > rhs.second;	
}

void InternetGraph::ReadGraph(const char file_name [], int vertice_no, int edge_node_no){
	ifstream file (file_name);
	char line[50];
	int counter = 0;

	Graph = Graph_T(vertice_no); // graph has size 100
	vector<pair<int,int> > dist_rec;

	while (file.getline(line, 50)){
		if (counter < 4 || counter == vertice_no + 4 || counter == vertice_no + 5){ // dummy info.
			++counter;
			continue;
		}

		// split
		vector<string> strs;
		int info[4];

		split(strs, line, is_any_of(" "));

		for (int i = 0; i < 4; ++i){
			info[i] = atoi(strs[i].c_str());
		}
		
		// select edge node   &   add edge
		if (counter < vertice_no + 4){ // vertices
			int margin = info[0]*info[0] + info[1]*info[1];
			int tmp = (100-info[0])*(100-info[0]) + info[1]*info[1];
			margin = min(margin, tmp);
			tmp = (100-info[1])*(100-info[1]) + info[0]*info[0];
			margin = min(margin, tmp);
			tmp = (100-info[1])*(100-info[1]) + (100-info[0])*(100-info[0]);
			margin = min(margin, tmp);
			
			pair<int, int> rec = make_pair(counter-4, margin);
			dist_rec.push_back(rec);
		}
		else{
			property<edge_weight_t, int> weight(1);
			add_edge(info[0], info[1], weight, Graph);
		}

		++counter;
	}
	
	// obtain edge
	sort(dist_rec.begin(), dist_rec.end(), pair_comp);

	for (int i = 0; i < edge_node_no; ++i){
		edge_ID.push_back(dist_rec[i].first);
	}

	sort(edge_ID.begin(), edge_ID.end());

	// debug
	cout << "Internet Graph Loaded" << endl;
}
