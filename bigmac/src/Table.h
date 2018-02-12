#ifndef M_TABLE
#define M_TABLE

#include <vector>
#include <unordered_map>
#include <list>
#include "MApair.h"
#include "Packet.h"
#include "BucketHandler.h"

using std::vector;
using std::pair;
using std::unordered_map;
using std::list;


class Entry{
private:
    int rID; // mapped from rule
    int ingress; // -1: any, 0,...,n-1: valid edge

public:
    int nextHop; // -1: invalid, 0,...,n-1: valid switch; 
    bool egress;  
};

class Btable {
private:
    vector<Rule> table;
    BucketHandler bHandler;
public:
    pair<Bucket *, int> getMatch(Packet & pkt);
    Rule & getRule(int rID){ return table[rID]; }
    int getAction(int rID){ return table[rID].action; }
};

class Ftable {
public:
    // lru cache
    int capacity;
    list<Bucket *> bucketCache; 
    unordered_map<Bucket *, list<Bucket *>::iterator> bucketCacheMap;

public:
    unordered_map<int, Entry> entryCacheMap;
};

class FtableTO: public Ftable{
private:
    unordered_map<Bucket *, double> bucketCacheMap;
    unordered_map<int, Entry> entryCacheMap;
    double timeout;

public:
    bool findBucket(Bucket * bkt){return bucketCacheMap.count(bkt);}
    Entry & findEntry(int rID){assert(entryCacheMap.count(rID)); return entryCacheMap[rID];}

    pair<int, int> getStats();
};

class FtableLRU: public Ftable{
private:
    unordered_map<Bucket *, list<Bucket *>::iterator> bucketCacheMap;
    unordered_map<int, Entry> entryCacheMap;
    int capacity;

public:
    bool findBucket(Bucket * bkt){return bucketCacheMap.count(bkt);}
    Entry & findEntry(int rID){assert(entryCacheMap.count(rID)); return entryCacheMap[rID];}
};

#endif
