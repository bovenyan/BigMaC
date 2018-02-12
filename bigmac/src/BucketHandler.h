#ifndef BUCKET_HANDLER
#define BUCKET_HANDLER

#include "MApair.h"

class BucketHandler{
private:
    Bucket * root;
    int cutNo;
    int thres;

    void dfsBuckTreeGen(Bucket * bkt, vector<Rule> & rList);
    pair<Bucket *, int> dfsPacketSearch(const LongUINT & header, Bucket * bkt);

public:
    BucketHandler(){root = NULL;}
    BucketHandler(int t, int c) : cutNo(c), thres(t){root = NULL;}

    void genBucketTree(vector<Rule> & rList);

    // ret: bucket, rule id
    pair<Bucket *, int> searchBucket(const Packet & pkt);

    ~BucketHandler(){
        if (root != NULL) 
            delete root;
    }
};

#endif
