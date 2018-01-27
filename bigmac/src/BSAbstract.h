#ifndef BIGSWITCH_ABSTRACTION
#define BIGSWITCH_ABSTRACTION

#include "Table.h"
#include "BucketHandler.h"

class BSA {
private:
    Mtable switchingTable;
    Mtable mgmtTable;
    
    BucketHandler bHandler;

public:
    void addMgmtRules();
    void addSwitchRules();
};

#endif
