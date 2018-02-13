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


class Entry {
private:
    int rID; // mapped from rule
    int ingress; // -1: any, 0,...,n-1: valid edge
    int depBucketCnt;

public:
    int nextHop; // -1: invalid, 0,...,n-1: valid switch;
    bool egress;

    Entry(int ruleID, int ingressPort):rID(ruleID),ingress(ingressPort),depBucketCnt(0){}

    void increDep() {
        ++depBucketCnt;
    }
    void decreDep() {
        --depBucketCnt;
    }
    bool canErase() {
        return depBucketCnt == 0;
    }
};

class Btable {
private:
    vector<Rule> table;
    BucketHandler bHandler;
public:
    pair<Bucket *, int> getMatch(Packet & pkt) {
        return bHandler.searchBucket(pkt);
    }
    Rule & getRule(int rID) {
        return table[rID];
    }
    int getAction(int rID) {
        return table[rID].action;
    }
};

class Ftable {
public:
    virtual bool hasBucket(Bucket * bkt) = 0;
    virtual bool hasEntry(int rID) = 0;
    virtual Entry & fetchEntry(int rID) = 0;
    virtual void eraseEntry(int rID) = 0;

    virtual pair<int, int> getTableStats(double ts) = 0;
};

class FtableTO: public Ftable {
private:
    unordered_map<Bucket *, double> bucketCacheMap;
    unordered_map<int, Entry> entryCacheMap;
    double timeout;

public:
    bool hasBucket(Bucket * bkt) {
        return bucketCacheMap.count(bkt);
    }
    bool hasEntry(int rID) {
        return entryCacheMap.count(rID);
    }

    Entry & fetchEntry(int rID) {
        assert(hasEntry(rID));
        return entryCacheMap[rID];
    }

    void eraseEntry(int rID){entryCacheMap.erase(rID);}

    pair<int, int> getTableStats(double ts); 
};

class FtableLRU: public Ftable {
private:
    unordered_map<Bucket *, list<Bucket *>::iterator> bucketCacheMap;
    unordered_map<int, Entry> entryCacheMap;
    int capacity;

public:
    bool hasBucket(Bucket * bkt) {
        return bucketCacheMap.count(bkt);
    }
    bool hasEntry(int rID) {
        return entryCacheMap.count(rID);
    }

    Entry & fetchEntry(int rID) {
        assert(hasEntry(rID));
        return entryCacheMap[rID];
    }
    
    void eraseEntry(int rID){entryCacheMap.erase(rID);}

    pair<int, int> getTableStats(double ts) {
        return make_pair((int)bucketCacheMap.size(), (int)entryCacheMap.size());
    }
};

#endif
