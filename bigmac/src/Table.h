#ifndef M_TABLE
#define M_TABLE

#include<vector>
#include"MApair.h"
#include"Packet.h"

using std::vector;
using std::pair;

class Mtable {
private:
    vector<Rule> table;
public:
    int getMatchRule(Packet & pkt);
};

class Ftable {
private:
    vector<unsigned> cacheFmask;
    vector<unsigned> cacheMmask;
};

#endif
