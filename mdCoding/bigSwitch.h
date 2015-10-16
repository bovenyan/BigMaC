#ifndef BIG_SWITCH_H
#define BIG_SWITCH_H

#include "bigMacRule.hpp"

class bigSwitch {
public:
    vector<sRule> sList;
    vector<nsRule> nsList;

private:
    void calByPass(nsRule * nsr);

public:
    // Analysis
    vector<nsRule *> EPpolicyOnPath(sRule * sr);
    vector<sRule *> routeOnEPpolicy(nsRule * nsr);

    // Routing
    void genRoute();  // TODO: Associate route to Topologys

    // Verification
    void veriFlow();  // TODO verify the flow to bypass
};
#endif
