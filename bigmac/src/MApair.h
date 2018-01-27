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

    bool isMatch(const Packet & pkt){
        return match == (pkt.header & mask);
    };
};

class Rule:public MApair{
public:
    Rule();
    Rule(string & line);
};

class Bucket:public MApair{

};

#endif
