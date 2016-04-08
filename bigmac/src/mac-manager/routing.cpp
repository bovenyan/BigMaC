#include "routing.h"
#include <fstream>
#include <boost/algorithm/string.hpp>

using std::ifstream;
using std::ofstream;
using std::string;
using std::stringstream;
using std::pair;
using std::rand;
using boost::split;
using boost::is_any_of;

routing::routing(string filename) {
    ifstream file(filename);

    string line;
    vertex_no = 0;

    while (getline(file, line)) {
        // vertex
        if (line.find("node:") != line.npos) {
            size_t lptr = line.find_first_of(":");

            vertex_no = std::atoi(line.substr(lptr + 1).c_str());

            topo = Graph(vertex_no);

            vector<vertex_descriptor> parent(vertex_no);

            for (size_type p = 0; p < vertex_no; ++p) {
                parent[p] = p;
            }

            for (int i = 0; i < vertex_no; ++i) {
                shortest_parents.push_back(parent);

                shortest_k_paths = vector<vector<vector<vector<int> > > > (vertex_no,
                                   vector<vector<vector<int> > > (vertex_no,
                                           vector<vector<int> >()
                                                                 )
                                                                          );
            }
        }

        // edges
        if (line.find("-") != line.npos) {
            size_t dptr = line.find_first_of("-");

            int srcDes = std::atoi(line.substr(0, dptr).c_str()) - 1;
            int dstDes = std::atoi(line.substr(dptr+1).c_str()) - 1;

            property<edge_weight_t, int> edge_weight(1);

            add_edge(srcDes, dstDes, edge_weight, topo);
        }
    }

    table_usage = vector<int>(vertex_no, 0);
}

void routing::cal_all_shortest() {
    for (int i = 0; i < num_vertices(topo); ++i) {
        dijkstra_shortest_paths(topo, i, predecessor_map(&shortest_parents[i][0]));
    }
}

vector<int> routing::cal_e2e_shortest(int src, int dst) {
    vector<vertex_descriptor> parent(num_vertices(topo));

    dijkstra_shortest_paths(topo, dst, predecessor_map(&parent));

    int v = src;
    vector<int> path;

    do {
        path.push_back(v);
        v = parent[v];
    } while (v != parent[v]);

    path.push_back(dst);

    return path;
}

void routing::cal_all_shortest_k(int K) {
    property<edge_weight_t, int> edge_weight(1);

    for (int dst = 0; dst < num_vertices(topo); ++dst) {

        for (int src = dst + 1; src < num_vertices(topo); ++ src) {
            // for each pair

            vector< vector<int> > shortestKpair;
            shortestKpair.push_back(cal_e2e_shortest(src, dst));

            vector< vector<int> > shortestKcandi;

            for (int k = 0; k < K; ++k) {
                auto prevPath = shortestKpair[k-1];
                for (int i = 0; i < prevPath.size()-1; ++i) {
                    int spurNode = prevPath[i];
                    vector<int> rootPath (prevPath.begin(), prevPath.begin()+i+1);

                    vector<pair<int, int> > removed_edges;
                    // compare the root path and remove the overlap
                    for ( auto kPath : shortestKpair) {
                        for (int j = 0; j < kPath.size(); ++j) {
                            if (kPath[j] != rootPath[j]) {
                                break;
                            } else {
                                if (kPath[j] == spurNode) {

                                    if (kPath[j] < kPath[j+1]) {
                                        removed_edges.push_back(std::make_pair(kPath[j], kPath[j+1]));
                                        remove_edge(kPath[j], kPath[j+1], topo);
                                    } else {
                                        removed_edges.push_back(std::make_pair(kPath[j+1], kPath[j]));
                                        remove_edge(kPath[j+1], kPath[j], topo);
                                    }
                                }
                            }
                        }
                    }

                    // delete root path
                    for (int j = 0; j < i; ++j) {
                        Iter_adj vi, vi_end;

                        for (tie(vi, vi_end) = adjacent_vertices(prevPath[j], topo); vi != vi_end; ++vi) {
                            if (prevPath[j] < *vi) {
                                remove_edge(prevPath[j], *vi, topo);
                                removed_edges.push_back(std::make_pair(prevPath[j], *vi));
                            } else {
                                remove_edge(*vi, prevPath[j], topo);
                                removed_edges.push_back(std::make_pair(*vi, prevPath[j]));
                            }
                        }
                    }

                    vector<int> spurPath = cal_e2e_shortest(spurNode, dst);

                    vector<int> totalPath = rootPath;
                    for(int i = 1; i < spurPath.size(); ++i) {
                        totalPath.push_back(spurPath[i]);
                    }

                    shortestKcandi.push_back(totalPath);

                    for (auto backup : removed_edges) {
                        add_edge(backup.first, backup.second, edge_weight, topo);
                    }
                }

                if (shortestKcandi.empty())
                    break;

                int minSize = num_vertices(topo)+1;
                int minIdx = -1;

                for (int i = 0; i < shortestKcandi.size(); ++i) {
                    if (shortestKcandi[i].size() < minSize) {
                        minSize = shortestKcandi[i].size();
                        minIdx = i;
                    }
                }

                shortestKpair.push_back(shortestKcandi[minIdx]);

                shortestKcandi.erase(shortestKcandi.begin()+minIdx);
            }

            shortest_k_paths[src][dst] = shortestKpair;
        }
    }
}

vector<int> routing::get_e2e_shortest(int src, int dst) {
    vector <int> path;

    int v = src;

    const vector<vertex_descriptor> & parent = shortest_parents[dst];

    do {
        // BOOST_LOG_SEV(logger_routing, debug) << "srcNode: " << v;
        path.push_back(v);
        v = parent[v];
    } while (v != parent[v]);

    path.push_back(dst);

    return path;
}

void routing::print_e2e_shortest(int src, int dst) {
    auto path = get_e2e_shortest(src, dst);
    stringstream ss;
    ss<<"[";
    for (int i : path) {
        ss << i+1 << " , ";
    }
    ss<<"]";

    BOOST_LOG_SEV(logger_routing, debug) << "srcNode: " << src+1
                                         << " dstNode: " << dst+1 << " " << ss.str();
}

void routing::print_all_shortest() {
    for (int i = 0; i < num_vertices(topo); ++i) {
        for ( int j = i + 1; j < num_vertices(topo); ++j) {
            print_e2e_shortest(i, j);
        }
    }
}

void routing::assign_paths_rand(int rule_no, int k, string filename) {
    cal_all_shortest_k(k);

    ofstream rec_file (filename);
    rule_routing_map = vector<vector<int> >(rule_no, vector<int>());

    for (int i = 0; i < rule_no; ++i) {
        int src = rand() % shortest_k_paths.size();
        int dst = rand() % shortest_k_paths[src].size();

        int path_id = rand() % shortest_k_paths[src][dst].size();

        rule_routing_map[i] = shortest_k_paths[src][dst][path_id];

        stringstream ss;

        for (int node : rule_routing_map[i]) {
            ss << node << " ";
        }

        rec_file << ss.str() + "\n";
    }

    rec_file.close();
}

void routing::load_paths(string filename) {
    ifstream rec_file (filename);

    string line;

    while (getline(rec_file, line)) {
        vector<string> comp;
        split(line, comp, is_any_of(" "));
        vector<int> path;
        for (string ele : comp) {
            path.push_back(std::atoi(ele.c_str()));
        }
        rule_routing_map.push_back(path);
    }
}

vector<int> routing::get_rule_path(int rule_id) {
    return rule_routing_map[rule_id];
}
