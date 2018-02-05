#ifndef BIGSWITCH_ABSTRACTION
#define BIGSWITCH_ABSTRACTION

#include "Table.h"
#include "BucketHandler.h"

class BSA {
public:
    Btable switchingTable;
    Btable managementTable;

    void addMgmtRules();
    void addSwitchRules();
};

#endif
