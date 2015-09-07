#ifndef MD_CODING_H
#define MD_CODING_H

#include "mdEquation.h"
#include <vector>
#include <random>

using std::vector;

class mdCoding {
private:
    vector<mdEquation> eqList;
    vector<vector<mdEquation *> > codingGroups;
    int sRuleNo;
    int nsRuleNo;

private:
    void grouping();  // pre-processing, divide into non-joint coding groups.

    void calAllConflict();  // pre-processing, calculate all conflicts
    void calConflict(vector<mdEquation *> group);  // pre-processing, calculate conflict among certain group

    int calNorm(int mode, vector<mdEquation *> eqToProc);  // calculate and coding for each group

    void addEquation(mdEquation mde);  

public:
    map<int, pair<int, int> > tagSRule;

public:
    mdCoding();

    int coding();  // coding the things 

    // debug:
    /*
     * eqNo: nsRule No.; varNo: sRule No.;
     * avgDep: average Depedent sRule of each ns rule
     * avgBypass: average Negative dep to by pass.
     */
    void randGenEqList(int eqNo, int varNo, int avgDep,
                       int avgBypass, int group_size, double rewiring);

};

#endif
