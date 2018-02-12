#ifndef BIGSWITCH_ABSTRACTION
#define BIGSWITCH_ABSTRACTION

#include "Table.h"
#include "BucketHandler.h"

class BSA {
private:
    Btable switchingTable;
    Btable managementTable;

public:
    // setter
    void addMgmtRules();
    void addSwitchRules();

    // getter
    Rule & getSwitchRule(int rID){return switchingTable.getRule(rID);};
    Rule & getMgmtRule(int rID){return managementTable.getRule(rID);};

    // matcher 
    pair<Bucket *, int> getSwitchMatch(Packet & pkt){return switchingTable.getMatch(pkt);}
    pair<Bucket *, int> getMgmtMatch(Packet & pkt){return managementTable.getMatch(pkt);}
};

#endif
