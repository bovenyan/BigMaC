#ifndef SWITCH_H
#define SWITCH_H

#include "Table.h"
#include "Packet.h"
#include "BSAbstract.h"

enum TABLE_MGMT {TIME_OUT, LRU};

class Switch {
private:
    Ftable fwdTable;
    Ftable mgmtTable;
    TABLE_MGMT tbl_mgmt;

public:
    bool fwdPacket(Packet & pkt, BSA & bsa);  // return ctrlPort
};

#endif
