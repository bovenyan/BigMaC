#ifndef MA_PAIR
#define MA_PAIR

#include "Packet.h"
#include "utility.h"
#include <string>

using std::string;

class MApair{
public:
    LongUINT match;
    LongUINT mask;

    int action; // positive: fwd port, negative mgmt 

public:
    MApair(){}
    MApair(const MApair & rhs):match(rhs.match), mask(rhs.mask){}
    friend bool operator==(const MApair & lhs, const MApair & rhs){
        return lhs.match == rhs.match && lhs.mask == rhs.mask;}
    friend bool operator!=(const MApair & lhs, const MApair & rhs){
        return !(lhs == rhs);
    }

    bool isMatch(const LongUINT & header){
        return match == (header & mask);
    };
};

class Rule:public MApair{
public:
    Rule();
    Rule(string & line);
};

class Bucket:public MApair{
private:
    vector<int> assoc;
    void fillAssoc(vector<int> & refAssoc, bool setBit, 
            const LongUINT & checkBit, 
            const vector<Rule> & rList);

public:
    vector<Bucket *> sons;

    Bucket(const Bucket & bkt);
    Bucket(const LongUINT & mat, const LongUINT & mas);
    ~Bucket();  // delete bkt and its sons.

    void setRootAssoc(const vector<Rule> & rList){ 
        for(int i = 0; i < rList.size(); ++i) assoc.push_back(i); 
    }
    int getSize(){
        return sons.size();
    }
    
    double getCutCost();
    
    void split(vector<FIELD> fields, const vector<Rule> & rList);
    void getMinCostSplit(const vector<Rule> & rList, 
            int cutNo, vector<FIELD> & cuts,
            vector<FIELD> & resCut, double minCost);

    int getMatchedRule(vector<Rule> & rList, const LongUINT header){
        for (int rID : assoc){
            if (rList[rID].isMatch(header))
                return rID;
        }

        return -1;
    }
};

#endif
