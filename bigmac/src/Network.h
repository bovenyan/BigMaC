#ifndef NETWORK_H
#define NETWORK_H

#include "Table.h"
#include <list>
#include <vector>

using std::vector;
using std::list;

class Switch {
public:
    Ftable fwdBucketTable;
    Ftable fwdTable;
    Ftable mgmtBucketTable;
    Ftable mgmtTable;
};

class Network {
public:
    vector<list<pair<int, int>>> adj;
    vector<Switch> devices;
    vector<vector<int>> routes;
    int nodesNo;

public:
    Network(int V);

    void addEdge(int u, int v, int w);
    void calShortestPath(int src);
    int getNextHop(int cur, int dest);
};

#endif
