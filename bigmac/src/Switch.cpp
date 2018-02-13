#include "Switch.h"

Switch::Switch(TABLE_MGMT mgmt){
    switch (mgmt){
        case TIME_OUT:
            fwdTable = new FtableTO();
            mgmtTable = new FtableTO();
            break;

        case LRU:
            fwdTable = new FtableLRU();
            mgmtTable = new FtableLRU();
            break;
            
        default:
            break;
    }
}

Switch::~Switch(){
    delete fwdTable;
    delete mgmtTable;
}

bool Switch::fwdPacket(Packet & pkt, BSA & bsa) {
    auto res = bsa.getSwitchMatch(pkt);
    Bucket * fwdBkt = res.first;
    int fwdRID = res.second;

    assert(fwdBkt != NULL);

    if (!fwdTable->hasBucket(fwdBkt)) {
        // cache miss
        return true;
    } else {
        Entry & e = fwdTable->fetchEntry(fwdRID);

        if (e.nextHop == -1 && !e.egress) {
            // negative rule
            fwdTable->eraseEntry(fwdRID);
            // cache miss, install rules
            return true;
        }

        if (pkt.mgmtAction == -1) { // mgmt not applied before
            auto mgmtRes = bsa.getMgmtMatch(pkt);
            Bucket * mgmtBkt = mgmtRes.first;
            int mgmtRID = res.second;

            if (mgmtBkt != NULL && mgmtTable->hasEntry(mgmtRID)) {
                pkt.mgmtAction = bsa.getMgmtRule(mgmtRID).action;
            }
        }

        if (e.egress)
            pkt.egressPort = pkt.curSwitch; 
        else
            pkt.curSwitch = e.nextHop;

        return false;
    }
}
