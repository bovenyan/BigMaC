#ifndef SWITCH_H
#define SWITCH_H

#include "Table.h"
#include "Packet.h"
#include "BSAbstract.h"

enum TABLE_MGMT {TIME_OUT, LRU};

class Switch {
private:
    Ftable * fwdTable;
    Ftable * mgmtTable;

public:
    Switch(TABLE_MGMT mgmt);
    ~Switch();
    bool fwdPacket(Packet & pkt, BSA & bsa);  // return ctrlPort
};

#endif
