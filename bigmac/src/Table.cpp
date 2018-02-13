#include "Table.h"

pair<int, int> FtableTO::getTableStats(double ts) {
    // clean and stats for TO impl
    for (auto p : bucketCacheMap) {
        if (p.second < ts) {
            bucketCacheMap.erase(p.first);
            for (int rID : p.first->getAssoc()) {
                if (entryCacheMap.count(rID)) {
                    entryCacheMap[rID].decreDep();
                    if (entryCacheMap[rID].canErase()) {
                        entryCacheMap.erase(rID);
                    }
                }
            }
        }
    }

    return make_pair((int)bucketCacheMap.size(), (int)entryCacheMap.size());
}
