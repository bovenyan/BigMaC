#include "MApair.h"
#include <cfloat>

Rule::Rule (string & line):MApair() {
    vector<unsigned> items;
    int prev = 1;

    while(int idx = line.find_first_of(".:\t", prev) != string::npos) {
        items.push_back(stoul(line.substr(prev, idx-prev)));
        prev = idx + 1;
    }

    unsigned ipSrc = 0;
    unsigned ipSrcMask = 0;
    unsigned ipDst = 0;
    unsigned ipDstMask = 0;

    for(int i = 0; i < 4; ++i) {
        ipSrc <<= 4;
        ipSrc += items[i];
    }


    ipSrcMask = (~(unsigned)0);
    if (items[5] != 32) {
        ipSrcMask >>= (32 - items[5]);
        ipSrcMask <<= (32 - items[5]);
    }

    for(int i = 5; i < 9; ++i) {
        ipDst <<= 4;
        ipDst += items[i];
    }

    ipSrcMask = (~(unsigned)0);
    if (items[9] != 32) {
        ipDstMask >>= (32 - items[9]);
        ipDstMask <<= (32 - items[9]);
    }

    auto srcP = approxPortRangeToPrefix(make_pair(items[10], items[11]));
    auto dstP = approxPortRangeToPrefix(make_pair(items[12], items[13]));
    match = LongUINT(ipSrc, ipDst, srcP.first, dstP.first);
    mask = LongUINT(ipSrcMask, ipDstMask, srcP.second, dstP.second);
}

pair<unsigned, unsigned> approxPortRangeToPrefix(pair<unsigned,
        unsigned> range) {
    unsigned prefix = 0;
    unsigned mask = 0xFFFF0000;

    if (range.second == 65535 && range.first == 0)
        return make_pair(prefix, mask);

    int length = range.second - range.first + 1;
    unsigned app_len = 1;

    while (length/2 > 0) {
        app_len = app_len * 2;
        length = length/2;
    }

    unsigned mid = range.second - range.second % app_len;
    if (mid + app_len - 1 <= range.second ) {
        prefix = mid;
    } else {
        if (mid - app_len >= range.first)
            prefix = mid - app_len;
        else {
            app_len = app_len/2;
            if (mid + app_len - 1 <= range.second)
                prefix = mid;
            else
                prefix = mid - app_len/2;
        }
    }

    mask = ~(unsigned)0;
    while (app_len > 1) {
        mask = mask << 1;
        app_len = app_len/2;
    }

    mask = mask | ((~unsigned(0))<<16);
    prefix = prefix & mask;
    return make_pair(prefix, mask);
}

Bucket::Bucket(const Bucket & bkt):MApair(bkt){}

Bucket::Bucket(const LongUINT & mat, const LongUINT & mas){
    match = mat;
    mask = mas;
}

void Bucket::fillAssoc(vector<int> & refAssoc, bool setBit, 
            const LongUINT & checkBit, 
            const vector<Rule> & rList){
    for (int rID : refAssoc){
        if (!(rList[rID].mask & checkBit))
            assoc.push_back(rID);
        else{
            if ((rList[rID].match & checkBit) ^ setBit)
                continue;
            else
                assoc.push_back(rID);
        }
    } 
}

void Bucket::split(vector<FIELD> fields, const vector<Rule> & rList){
    vector<Bucket *> sons;

    vector<Bucket *> scratch;
    Bucket * bkt = new Bucket(*this);
    scratch.push_back(bkt);

    for (FIELD field : fields){
        vector<Bucket *> newScratch;

        for (Bucket * bkt : scratch){
            LongUINT mask = bkt->mask;
            
            if (mask.divMask(field)){
                LongUINT match1 = bkt->match;
                Bucket * son1 = new Bucket(match1, mask);
                son1->fillAssoc(bkt->assoc, false, mask ^ bkt->mask, rList);

                LongUINT match2 = bkt->match ^ (mask ^ bkt->mask);
                Bucket * son2 = new Bucket(match2, mask);
                son2->fillAssoc(bkt->assoc, false, mask ^ bkt->mask, rList);
                
                newScratch.push_back(new Bucket(match1, mask));
                newScratch.push_back(new Bucket(match2, mask));
                delete bkt;
            }
            else{
                if (*bkt != *this)
                    sons.push_back(bkt);
            }
        }

        scratch = newScratch;
    }
}

double Bucket::getCutCost(){
    double cost = 0;

    for (Bucket * son : sons){
        cost += son->assoc.size();    
    }

    if (sons.size() != 0){
        cost /= sons.size();
    }

    return DBL_MAX;
}

void Bucket::getMinCostSplit(const vector<Rule> & rList, 
        int cutNo, vector<FIELD> & cuts,
        vector<FIELD> & resCut, double minCost = DBL_MAX){
    if (cutNo == 0){
        split(cuts, rList);
        
        double cost = getCutCost();

        if (cost < minCost){
            resCut = cuts;
            minCost = cost;
        }

        for (Bucket * bkt : sons)
            delete bkt;

        sons.clear();

        return;
    }

    for ( int i; i < FIELD_END; ++i){
        cuts.push_back(FIELD(i)); 
        getMinCostSplit(rList, cutNo-1, cuts, resCut, minCost);
        cuts.pop_back();
    } 
}

Bucket::~Bucket(){
    for (Bucket * bkt : sons)
        delete bkt;
    sons.clear();
}
