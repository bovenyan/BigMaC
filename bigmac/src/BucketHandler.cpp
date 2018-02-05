#include "BucketHandler.h"
#include <cfloat>

void BucketHandler::dfsBuckTreeGen(Bucket * bkt, vector<Rule> & rList){
    if (bkt == NULL)
        return;

    if (bkt->getSize() <= thres)
        return;

    vector<FIELD> cuts;
    vector<FIELD> resCut; 

    bkt->getMinCostSplit(rList, cutNo, cuts, resCut, DBL_MAX);

    if (resCut.empty()) // not cuttable;
        return;

    // commit split
    bkt->split(resCut, rList);

    // dfs    
    for (Bucket * son : bkt->sons)
        dfsBuckTreeGen(son, rList); 
}

void BucketHandler::genBucketTree(vector<Rule> & rList){
    root = new Bucket(LongUINT(0, 0, 0, 0), LongUINT(0,0,0,0));
    dfsBuckTreeGen(root, rList);
}

pair<Bucket *, int> dfsPacketSearch(const LongUINT & header, Bucket * bkt, vector<Rule> & rList){
    if (bkt->sons.empty()){
        int rID = bkt->getMatchedRule(rList, header);
        return make_pair(bkt, rID);
    }

    for (Bucket * son : bkt->sons){
        if (son->isMatch(header))
            return dfsPacketSearch(header, son, rList);
    }

    // shoundn't hit
    assert(false);
    return make_pair((Bucket*)NULL, -1);
}

pair<Bucket *, int> BucketHandler::searchBucket(const Packet & pkt){
    return dfsPacketSearch(pkt.header, root); 
}
