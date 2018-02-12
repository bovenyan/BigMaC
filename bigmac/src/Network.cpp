#include "Network.h"

#include <queue>
#include <climits>

using std::priority_queue;
using std::greater;
using std::pair;

typedef pair<int,int> iPair;

Network::Network(int nodeNo) {
    this->nodesNo = nodeNo;
    adj = vector<list<iPair>>(nodeNo, list<iPair>());
    switches = vector<Switch>(nodeNo, Switch());
    routes = vector<vector<int>>(nodeNo, vector<int>(nodeNo, 0));
}

void Network::addEdge(int u, int v, int w) {
    adj[u].push_back(make_pair(v, w));
    adj[v].push_back(make_pair(u, w));
}

void Network::calShortestPath(int src) {
    priority_queue<iPair, vector<iPair>, greater<iPair> > pq;

    pq.push(make_pair(0, src));
    routes[src][src] = src;

    vector<int> dist(nodesNo, INT_MAX);
    dist[src] = 0;

    int root = src;

    while(!pq.empty()) {
        int u = pq.top().second;
        pq.pop();

        for (auto it = adj[u].begin();
                it != adj[u].end();
                ++it) {
            int v = it->first;
            int weight = it->second;

            if (dist[v] > dist[u] + weight) {
                dist[v] = dist[u] + weight;
                routes[src][v] = u;
                pq.push(make_pair(dist[v], v));
            }
        }
    }
}

int Network::getNextHop(int cur, int dest) {
    return routes[dest][cur];
}

void Network::packetArrival(Packet & pkt, BSA & bsa) {
    pkt.curSwitch = pkt.ingressPort;

    while(pkt.egressPort == -1){
        if (switches[pkt.curSwitch].fwdPacket(pkt, bsa)){
            cacheEntries(pkt, bsa);
        }

        --pkt.TTL;
        assert(pkt.TTL > 0);
    }

    // TODO: do consistent check
}

void Network::cacheEntries(Packet & pkt, BSA & bsa){
    auto swRes = bsa.getSwitchMatch(pkt); 
    auto mgmtRes = bsa.getSwitchMatch(pkt);

    // matched switching rule
    int hop = pkt.ingressPort;
    int dest = bsa.getSwitchRule(swRes.second).getEgress();

    while (hop != dest){
        hop = getNextHop(hop, dest);
    } 
}
