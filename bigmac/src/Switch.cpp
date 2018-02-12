#include "Switch.h"

bool Switch::fwdPacket(Packet & pkt, BSA & bsa) {
    auto res = bsa.getSwitchMatch(pkt);
    Bucket * fwdBkt = res.first;
    int fwdRID = res.second;

    assert(fwdBkt != NULL);

    if (fwdTable.bucketCacheMap.count(fwdBkt) == 0) {
        // cache miss
        return true;
    } else {
        assert(fwdTable.entryCacheMap.count(fwdRID));

        Entry & e = fwdTable.entryCacheMap[fwdRID];

        if (e.nextHop == -1 && !e.egress) {
            // negative rule
            fwdTable.entryCacheMap.erase(fwdRID);
            // cache miss, install rule
            return true;
        }

        if (pkt.mgmtAction == -1) { // mgmt not applied before
            auto mgmtRes = bsa.getMgmtMatch(pkt);
            Bucket * mgmtBkt = mgmtRes.first;
            int mgmtRID = res.second;

            if (mgmtBkt != NULL &&
                    mgmtTable.entryCacheMap.count(mgmtRID)) {
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
