#ifndef PER_FLOW
#define PER_FLOW

#include "../stdafx.h"

#include <map>

using std::vector; using std::map;

struct flow_rec{
    double activeTime = -10;
    vector<int> path;
    int ns_loc;
};

class perflow{
    private:
        vector<flow_rec> 
};

#endif
