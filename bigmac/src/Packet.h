#ifndef PACKET_H
#define PACKET_H

#include "utility.h"

class Packet{
public:
    LongUINT header;
    int ingressPort;

    // meta:
    int curSwitch;
    int egressPort;
    int mgmtAction;

    // error
    int TTL;

public:
    Packet(){
        curSwitch = -1;
        egressPort = -1;
        mgmtAction = -1;
        TTL = 64;
    }

    Packet(string & str) : Packet(){
        
    }
};

#endif 
