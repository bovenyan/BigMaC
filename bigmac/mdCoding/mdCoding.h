#ifndef MD_CODING_H
#define MD_CODING_H

#include "mdEquation.h"
#include "../sharedHeader.h"

#include <vector>
#include <random>
// #include <iostream>

using std::vector;
// using std::cout; using std::endl;

class mdCoding {
private:
    vector<mdEquation> eqList;
    vector<vector<mdEquation *> > codingGroups;
    int sRuleNo;
    int nsRuleNo;
    
    logging_src::severity_logger< severity_level > logger_mdCoding;

private:
    void grouping();  // pre-processing, divide into non-joint coding groups.

    // void calAllConflict();  // pre-processing, calculate all conflicts
    // void calConflict(vector<mdEquation *> & group);  // pre-processing, calculate conflict among certain group, conflict online cal

    // int calNorm(int mode, vector<mdEquation *> & group);  // calculate and coding for each group
    
    int codeNSrule(vector<mdEquation *> & group, int mode);  // calculate and coding for each group

    void addEquation(mdEquation mde);  

public:
    // SRule Tag:  key = sRuleID,  pair.first = bit, pair.second = whetherSet
    map<int, pair<int, int> > tagSRule;

public:
    mdCoding();

    void randGenEqList(int eqNo, int varNo, int avgDep,
                       double avgBypass, int groupNo, double rewiring);

    int coding();  // coding the things 

    // debug:
    /*
     * eqNo: nsRule No.; varNo: sRule No.;
     * avgDep: average Depedent sRule of each ns rule
     * avgBypass: average Negative dep to by pass.
     */
    void codingVerify();
};

#endif
