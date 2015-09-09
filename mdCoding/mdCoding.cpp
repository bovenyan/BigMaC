#include "mdCoding.h"
#include <cstdlib>
#include <queue>
#include <set>
#include <limits>

using std::queue;
using std::set;

mdCoding::mdCoding() {}

void mdCoding::addEquation(mdEquation mde) {
    eqList.push_back(mde);
}

void mdCoding::calAllConflict() {
    for (int i=0; i < eqList.size(); ++i) {
        for (int j = i+1; j < eqList.size(); ++j) {
            eqList[i].calConflict(&eqList[j]);
        }
    }
}

void mdCoding::grouping() {
    set<mdEquation *> ToProc;
    vector<set<mdEquation *> > sToNsMap(sRuleNo, set<mdEquation *>());

    for (auto eq : eqList) {
        eq.checkDep(sToNsMap);

        if (eq.hasByPass) {
            ToProc.insert(&eq);
        }
    }

    while (!ToProc.empty()) {
        vector<mdEquation *> group;

        queue<mdEquation *> ToProcMem;
        ToProcMem.push(*ToProc.begin());

        while(!ToProcMem.empty()) {
            mdEquation * next = ToProcMem.front();

            if (next->hasByPass) {
                vector<int> rela = next->depSRule();
                for (int sRuleID : rela) {
                    for (mdEquation * nsRulePtr : sToNsMap[sRuleID])
                        ToProcMem.push(nsRulePtr);
                }

                ToProc.erase(ToProc.find(next));
            }

            group.push_back(next);

	    next->initTag(tagSRule); // initiate the SRule Tag

            ToProcMem.pop();
        }

        codingGroups.push_back(group);
    }
}

int mdCoding::coding() {
    grouping();

    int metabit = 0;

    // code within each group
    for (auto group : codingGroups) {
        calConflict(group);
        int used = calNorm(0, group);

        if (metabit < used)
            metabit = used;

        for (auto eqPtr : group) {
            eqPtr->assign(tagSRule);
        }
    }

    return metabit;
}

int mdCoding::calNorm(int mode, vector<mdEquation *> eqToProc) {
    map<int, int> groupCount;

    for (auto eqPtr : eqToProc) {
        map<int, int> candiGroupCount = groupCount;

        for (auto neighPtr : eqPtr->conflictNeigh) {
            candiGroupCount.erase(neighPtr->bitIdx);
        }

        int chosen_group = -1;

        if (candiGroupCount.empty()) { // add a new group
            chosen_group = groupCount.size()+1;
            groupCount[chosen_group] = 1;
        }
        else {
            switch (mode) {
            case 0: { // group to smaller groups
                int small = std::numeric_limits<int>::max();
                for (auto iter = candiGroupCount.begin(); iter != candiGroupCount.end(); ++iter) {
                    if (iter->second < small) { // increment counter
                        chosen_group = iter->first;
                        small = iter->second;
                    }
                }
                ++groupCount[chosen_group];
                break;
            }

            case 1: {// randomly group
                auto iter = candiGroupCount.begin();
                std::advance(iter, rand()%candiGroupCount.size());
                ++iter->second;
                chosen_group = iter->first;
                break;
            }
            default:
                break;
            }
        }

        eqPtr->bitIdx = chosen_group;
    }

    return groupCount.size();
}


void mdCoding::randGenEqList(int nsRuleNo, int sRuleNo, int avgDep,
                             int avgBypass, int groupNo, double rewiringProb) {
    this->sRuleNo = sRuleNo;
    this->nsRuleNo = nsRuleNo;
    // devide the sRules into groups
    vector<int> sGroupDiv;
    int sGroupSize = sRuleNo/groupNo;

    for (int i = 0; i< groupNo -1; ++i) {
        int nextDelim = sGroupSize * i;

        if (i != 0)  // randomize
            nextDelim += rand()%sGroupSize - sGroupSize/2;

        sGroupDiv.push_back(nextDelim);
    }

    sGroupDiv.push_back(sRuleNo);

    // devide the nsRules into groups
    vector<int> nsGroupDiv;
    int nsGroupSize = sRuleNo/groupNo;

    for (int i = 0; i< groupNo -1; ++i) {
        int nextDelim = nsGroupSize * i;

        if (i != 0)  // randomize
            nextDelim += rand()%nsGroupSize - nsGroupSize/2;

        nsGroupDiv.push_back(nextDelim);
    }

    nsGroupDiv.push_back(nsRuleNo);

    double bypassProb = (double)avgBypass/(nsRuleNo*avgDep);

    for (int groupID = 0; groupID < groupNo; ++groupID) {
        for (int nsRuleID = nsGroupDiv[groupID]; nsRuleID < nsGroupDiv[groupID+1]+1; ++nsRuleID) {
            mdEquation eq;

            vector<int> sRuleIDs;
            for (int i = sGroupDiv[groupID]; i < sGroupDiv[groupID+1]; ++i)
                sRuleIDs.push_back(i);

            eq.randGenMdEq(avgDep, sRuleIDs, sRuleNo, rewiringProb, bypassProb);
            eqList.push_back(eq);
        }
    }
}
