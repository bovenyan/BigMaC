#ifndef FLOWENTRY_H
#define FLOWENTRY_H

#include "bigMacRule.hpp"

class sFlowEntry : public sRule {
public:
    int placement;
    sRule * origSRule;

};

class nsFlowEntry : public nsRule {
public:
    int placement;
    nsRule * origNSRule;

};
#endif
