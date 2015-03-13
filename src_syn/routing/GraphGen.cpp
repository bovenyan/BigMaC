#include "GraphGen.h"
#include <fstream>
using namespace std;
using namespace boost;

InternetGraph::InternetGraph(){}

void InternetGraph::CalShortestPath(string sp_file){
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

    // Record the shortest path
    // Format :   edgeID node1 node2 node3 ...
    ofstream shortest_path_rec(sp_file.c_str());
    
    for (int i = 0; i < edge_ID.size(); ++i){
        vector <int> rec = path_map[i];
        shortest_path_rec<<edge_ID[i];

        for (int j = 0; j < rec.size(); ++j){
            shortest_path_rec<<" "<<rec[j];
        }
        shortest_path_rec<<endl;
    }
}

void InternetGraph::ReadPath(string sp_file){
    ifstream shortest_path_rec(sp_file);

    edge_ID.clear();
    path_map.clear();

    for (string line; getline(shortest_path_rec, line); ){
        vector<string> temp;
        boost::split(temp, line, boost::is_any_of(" "));
        edge_ID.push_back(atoi(temp[0].c_str()));
        temp.erase(temp.begin());

        vector<int> rec;
        for (string s : temp){
            rec.push_back(atoi(s.c_str()));
        }

        path_map.push_back(rec);
    }

    vertice_no = path_map[0].size();
}

vector<int> InternetGraph::query_path(int src_eID, int dst_eID){
	vector<int> path;
    int src = edge_ID[src_eID];
    int dst = edge_ID[dst_eID];

	while (dst != src){
		path.push_back(dst);
		dst = path_map[src_eID][dst];
	}

	path.push_back(src);

    return path;
}

string InternetGraph::print_path(int src, int dst){
    vector<int> path = query_path(src, dst);
    stringstream str;
	// print path
	for (auto iter = path.begin(); iter != path.end(); ++iter)
		str<<*iter<<"->";
    
    return str.str();
}


bool pair_comp(pair<int, int> lhs, pair<int, int> rhs){
	return lhs.second < rhs.second;	
}

void InternetGraph::ReadGraph(const char file_name [], int edge_node_no){
	ifstream file (file_name);

	vector<pair<int,int> > dist_rec;
	
    int counter = 0;
    int geo_size = 100;
    vertice_no = 100;

	for (string line; getline(file, line); ++counter){
        // retrieve info from 2nd line
        if (counter == 1){
            vector<string> tmp; 
            split(tmp, line, is_any_of(" "));

            // obtain node_no
            vertice_no = atoi(tmp[0].c_str());
	        Graph = Graph_T(vertice_no);
            edge_node_no = min(edge_node_no, vertice_no);

            // obtain geo_size
            geo_size = atoi(tmp[tmp.size()-1].c_str());
        }

        // dummy info.
		if (counter < 4 || counter == vertice_no + 4 || counter == vertice_no + 5) 
			continue;

		// split
		vector<string> strs;
		int info[4];

		split(strs, line, is_any_of(" "));

		for (int i = 0; i < 4; ++i){
			info[i] = atoi(strs[i].c_str());
		}
		
		// select edge node   &   add edge
		if (counter < vertice_no + 4){ // vertices
			int margin = info[2]*info[2] + info[3]*info[3];
			int tmp = (geo_size-info[2])*(geo_size-info[2]) + info[3]*info[3];
			margin = min(margin, tmp);
			
            tmp = (geo_size-info[2])*(geo_size-info[2]) + info[3]*info[3];
            margin = min(margin, tmp);

			tmp = (geo_size-info[3])*(geo_size-info[3]) + 
                  (geo_size-info[2])*(geo_size-info[2]);
			margin = min(margin, tmp);
			
			pair<int, int> rec = make_pair(counter-4, margin);
			dist_rec.push_back(rec);
		}
		else{
			property<edge_weight_t, int> weight(1);
			add_edge(info[0], info[1], weight, Graph);
		}

	}
	
	// obtain edge
	sort(dist_rec.begin(), dist_rec.end(), pair_comp);

	for (int i = 0; i < edge_node_no; ++i)
		edge_ID.push_back(dist_rec[i].first);

	sort(edge_ID.begin(), edge_ID.end());
}

// ------------------------- FatTreeGraph ----------------------------

FatTreeGraph::FatTreeGraph(int k):kVal(k),kVal_2(k*k),kVal_hf(k/2){};

// k ports/switch, (k/2)^2 core switches, k pods
// k/2 aggr/pod, k/2 edge switches/pod
// total switches: 5^2/4 
// path organized in this way  sw1 [range], sw2 [range], sw3 [range]
// paths within same pod (k paths), within diff pods (k^2 paths)
inline vector<int> FatTreeGraph::query_path(const int & src, const int & dst, 
                                            const int & p_idx, const int & c_idx){
    int src_pod = src/kVal_hf;
    int dst_pod = dst/kVal_hf;
    
    if (src_pod == dst_pod){
        return vector<int>(1, kVal_2 + src_pod * kVal_hf + p_idx); 
    }
    else{
        vector<int> res;
        
        res.push_back(kVal_2 + src_pod * kVal_hf + p_idx);
        res.push_back(2*kVal_2 + p_idx * kVal_hf + c_idx);
        res.push_back(kVal_2 + dst_pod * kVal_hf + p_idx);

        return res;
    }
}

// K-ECMP
inline vector<vector<int> > FatTreeGraph::query_all_path(const int & src, const int & dst, int kpath){
    int src_pod = src/kVal_hf;
    int dst_pod = dst/kVal_hf;

    if (kpath > kVal_hf)
        kpath = kVal_hf;

    vector<vector<int> > res;
    
    vector<int> random_list_1, random_list_2, random_list_3;
    for (int i = 0; i < kVal_hf; ++i){
        random_list_1.push_back(i);
        random_list_2.push_back(i);
    }
    for (int i = 0; i < kVal_hf*kVal_hf; ++i){
        random_list_3.push_back(i);
    }

    random_shuffle(random_list_1.begin(), random_list_1.end());
    random_shuffle(random_list_2.begin(), random_list_2.end());
    random_shuffle(random_list_3.begin(), random_list_3.end());

    if (src_pod == dst_pod){
        for (int i = 0; i < kpath; ++i){
            vector<int> route;
            route.push_back(src);
            route.push_back(kVal_2/2 + src_pod * kVal_hf + random_list_1[i]);
            route.push_back(dst);
            res.push_back(route);
        }
    }
    else{
        for (int i = 0; i < kpath; ++ i){
            vector<int> route;
            route.push_back(src);
            route.push_back(kVal_2/2 + src_pod * kVal_hf + random_list_1[i]);
            route.push_back(kVal_2 + random_list_3[i]);
            route.push_back(kVal_2/2 + dst_pod * kVal_hf + random_list_2[i]);
            route.push_back(dst);
            res.push_back(route);
        }
    }
    return res;
}


