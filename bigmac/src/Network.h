#ifndef NETWORK_H
#define NETWORK_H

#include "Table.h"
#include "Switch.h"
#include "BSAbstract.h"
#include <list>
#include <vector>

using std::vector;
using std::list;


class Network {
public:
    int nodesNo;

    vector<list<pair<int, int>>> adj;
    vector<Switch> switches;
    vector<vector<int>> routes;

    void addEdge(int u, int v, int w);
public:
    Network(int V);

    void calShortestPath(int src);
    int getNextHop(int cur, int dest);

    void packetArrival(Packet & pkt, BSA & bsa);
    void cacheEntries(Packet & pkt, BSA & bsa);
};

#endif
