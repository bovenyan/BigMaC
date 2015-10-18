#include "bigSwitch.h"

vector<sRule *> bigSwitch::routeOnEPpolicy(nsRule * nsr) {
    vector<sRule *> routes;
    
    for (int i = 0; i < sList.size(); ++i) {
        if (nsr->overlap(sList[i]))
            routes.push_back(&sList[i]);
    }

    return routes;
}

vector<nsRule *> bigSwitch::EPpolicyOnPath(sRule * sr) {
    vector<nsRule *> epPolicy;
    
    for (int i = 0; i < nsList.size(); ++i){
    	if (sr->overlap(nsList[i]))
	    epPolicy.push_back(&nsList[i]);
    }

    return epPolicy;
}
