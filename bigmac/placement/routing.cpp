#include "routing.h"
#include <fstream>

using std::ifstream;
using std::string;
using std::stringstream;
using std::pair;

Topology::Topology(string filename) {
    ifstream file(filename);

    string line;

    while (getline(file, line)) {
        // node no
        if (line.find("node:") != line.npos) {
            size_t lptr = line.find_first_of(":");

            int vertexNo = std::atoi(line.substr(lptr + 1).c_str());

            topo = Graph(vertexNo);

            vector<vertex_descriptor> parent(vertexNo);

            for (size_type p = 0; p < vertexNo; ++p) {
                parent[p] = p;
            }

            for (int i = 0; i < vertexNo; ++i) {
                shortestParents.push_back(parent);

                shortestKPaths = vector<vector<vector<vector<int> > > > (vertexNo,
                                 vector<vector<vector<int> > > (vertexNo,
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
}

void Topology::allShortest() {
    for (int i = 0; i < num_vertices(topo); ++i) {
        dijkstra_shortest_paths(topo, i, predecessor_map(&shortestParents[i][0]));
    }
}

vector<int> Topology::calShortest(int src, int dst) {
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

void Topology::calShortestK(int K) {
    property<edge_weight_t, int> edge_weight(1);

    for (int dst = 0; dst < num_vertices(topo); ++dst) {

        for (int src = dst + 1; src < num_vertices(topo); ++ src) {
            // for each pair

            vector< vector<int> > shortestKpair;
            shortestKpair.push_back(calShortest(src, dst));

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

                    vector<int> spurPath = calShortest(spurNode, dst);

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
            
            shortestKPaths[src][dst] = shortestKpair;
        }
    }
}

vector<int> Topology::FetchShortestPath(int src, int dst) {
    vector <int> path;

    int v = src;

    const vector<vertex_descriptor> & parent = shortestParents[dst];


    do {
        // BOOST_LOG_SEV(logger_routing, debug) << "srcNode: " << v;
        path.push_back(v);
        v = parent[v];
    } while (v != parent[v]);

    path.push_back(dst);

    return path;
}

void Topology::printShortestPath(int src, int dst) {
    auto path = FetchShortestPath(src, dst);
    stringstream ss;
    ss<<"[";
    for (int i : path) {
        ss << i+1 << " , ";
    }
    ss<<"]";

    BOOST_LOG_SEV(logger_routing, debug) << "srcNode: " << src+1
                                         << " dstNode: " << dst+1 << " " << ss.str();
}

void Topology::printAllShortestPath() {
    for (int i = 0; i < num_vertices(topo); ++i) {
        for ( int j = i + 1; j < num_vertices(topo); ++j) {
            printShortestPath(i, j);
        }
    }
}
