#include "Mtable.h"

int Mtable::getMatchRule(Packet & pkt){
    for (int i = 0; i < table.size(); ++i){
        if (table[i].isMatch(pkt))
            return i;
    }

    return -1;
}
