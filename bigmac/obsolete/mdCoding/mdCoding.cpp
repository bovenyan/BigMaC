#include "mdCoding.h"
#include <cstdlib>
#include <queue>
#include <set>
#include <map>
#include <limits>

using std::queue;
using std::set;
using std::map;
using std::make_pair;

mdCoding::mdCoding() {}

void mdCoding::addEquation(mdEquation mde) {
    eqList.push_back(mde);
}

/*
void mdCoding::calAllConflict() {
    for (int i=0; i < eqList.size(); ++i) {
        for (int j = i+1; j < eqList.size(); ++j) {
            eqList[i].calConflict(&eqList[j]);
        }
    }
}

void mdCoding::calConflict(vector<mdEquation *> & group) {
    for (int i = 0; i < group.size(); ++i) {
        BOOST_LOG_SEV(logger_mdCoding, debug)<<"Eq "<<i<<" Conflicts";

        for (int j = i+1; j < group.size(); ++j) {
            if (group[i]->calConflict(group[j]))
                BOOST_LOG_SEV(logger_mdCoding, debug)<<"\t eq "<<j;
        }
    }
    BOOST_LOG_SEV(logger_mdCoding, debug)<<"Finished Calculating Conflicts";
}
*/

void mdCoding::grouping() {
    /*
     * If nsRule Tagged, sRule must be tagged
     * If sRule Tagged, nsRule need not be tagged
    */

    set<mdEquation *> ToProc;
    vector<set<mdEquation *> > sToNsMap(sRuleNo, set<mdEquation *>());

    for (auto & eq : eqList) {  // create the s-to-ns map
        eq.checkDep(sToNsMap);

        if (eq.hasByPass) {
            ToProc.insert(&eq);
        }

        // BOOST_LOG_SEV(logger_mdCoding, debug)<<eq.toStr();
    }


    while (!ToProc.empty()) {
        unordered_set<mdEquation *> processed_ns; // ns processed
        unordered_set<int> processed_s; // s processed

        vector<mdEquation *> group; // record the group

        queue<mdEquation *> ToProcMem; // BFS queue

        ToProcMem.push(*ToProc.begin());
        processed_ns.insert(*ToProc.begin());

        // BOOST_LOG_SEV(logger_mdCoding, debug)<<"Group: ";

        while(!ToProcMem.empty()) {
            mdEquation * nextNS = ToProcMem.front();

            vector<int> rela = nextNS->depSRule();

            for (int & sRuleID : rela) {
                if (processed_s.find(sRuleID) != processed_s.end())
                    continue;

                processed_s.insert(sRuleID);

                for (mdEquation * nsRulePtr : sToNsMap[sRuleID]) {
                    // process if: 1. has bypass, 2. never processed
                    if ((processed_ns.find(nsRulePtr) == processed_ns.end()) &&
                            nsRulePtr->hasByPass) {
                        ToProcMem.push(nsRulePtr);
                        processed_ns.insert(nsRulePtr);
                    }
                }
            }

            // if (ToProc.find(nextNS) == ToProc.end()) {
            //     BOOST_LOG_SEV(logger_mdCoding, debug)<<"This is it cannot erase";
            //     exit(0);
            // }

            ToProc.erase(ToProc.find(nextNS));
            nextNS->initTag(tagSRule); // initiate the SRule Tag
            group.push_back(nextNS);
            // BOOST_LOG_SEV(logger_mdCoding, debug)<<"\t eq: "<<nextNS->toStr();

            ToProcMem.pop();
        }

        codingGroups.push_back(group);
    }
    BOOST_LOG_SEV(logger_mdCoding, debug)<<"Finished Coding Group: "<<codingGroups.size()<<" groups";
}

int mdCoding::codeNSrule(vector<mdEquation *> & group, int mode=0) {
    // TODO: sort by size

    vector <pair <mdEquation, int> > nonConfSets; // idx: setID {first: merged eq, second: size}

    for (auto eqPtr : group) {
        vector<pair<int, bool> > choiceRec; // first: no. of Eq. second: reverse?
	vector<int> feasibleChoice;

        // check whether it conflicts existing groups
        int minSize = std::numeric_limits<int>::max();
        int choice = -1;

	bool reverse = false;

        for (int idx = 0; idx < nonConfSets.size(); ++idx) {
	    auto confCheck = nonConfSets[idx].first.calConflict(eqPtr);

            if (!confCheck.first) { // not conflicting
                choiceRec.push_back(std::make_pair(idx, confCheck.second));
		feasibleChoice.push_back(idx);

		if (nonConfSets[idx].second < minSize){ // for min 
		    choice = idx;
		    minSize = nonConfSets[idx].second;
		    reverse = confCheck.second; 
		}
            }
        }

        if (feasibleChoice.empty()) {
	    eqPtr->bitIdx = nonConfSets.size();
	    eqPtr->bit = 1;
            mdEquation merged = *eqPtr;
            nonConfSets.push_back(std::make_pair(merged, 1));
        }
        else {
            if (mode == 0) { // choose the smallest set
		eqPtr->bitIdx = choice;
		eqPtr->bit = !reverse;
            }
            if (mode == 1) { // choose random
		choice = rand()%feasibleChoice.size(); // first find in feasible term
		reverse = choiceRec[choice].second; 
		choice = feasibleChoice[choice];
		eqPtr->bitIdx = choice;
		eqPtr->bit = !reverse;
            }

	    nonConfSets[choice].first.mergeNonConf(eqPtr, reverse);
        }
    }

    return nonConfSets.size(); 
}

int mdCoding::coding() {
    grouping();

    int metabit = 0;

    // code within each group
    for (auto group : codingGroups) {
        // calConflict(group);  //TODO: rephrase
        // int used = calNorm(0, group);
	int used = codeNSrule(group);

        if (metabit < used)
            metabit = used;

        for (auto eqPtr : group) {
            eqPtr->assignSrule(tagSRule);
        }
    }

    BOOST_LOG_SEV(logger_mdCoding, debug)<<"Used: "<< metabit<<" metabit";
    return metabit;
}

/*
int mdCoding::calNorm(int mode, vector<mdEquation *> & group) {
    map <int, int> nonConfSets;

    BOOST_LOG_SEV(logger_mdCoding, debug)<<"Entering calNorm!!";
    BOOST_LOG_SEV(logger_mdCoding, debug)<<"Entering calNorm!!";
    BOOST_LOG_SEV(logger_mdCoding, debug)<<"Entering calNorm!!";
    BOOST_LOG_SEV(logger_mdCoding, debug)<<"Important Things Say Thrice !!";

    for (auto eqPtr : group) {
        BOOST_LOG_SEV(logger_mdCoding, debug)<<"Process an eq";

        map<int, int> candiNonConfSets = nonConfSets;

        int choiceNo = candiNonConfSets.size();

        for (auto neighPtr : eqPtr->conflictNeigh) {
            int tagIdx = neighPtr->bitIdx;
            if (tagIdx != -1 && candiNonConfSets.find(tagIdx) != candiNonConfSets.end())  // tagged
                candiNonConfSets.erase(tagIdx);
        }

        int choice = -1;

        // choice is from 0 to n-1
        if (candiNonConfSets.empty()) { // add a new group if conflicting with any
            choice = nonConfSets.size();
            nonConfSets[choice] = 1;
        }
        else {
            switch (mode) {
            case 0: { // greedily group to smallest groups
                int small = std::numeric_limits<int>::max();

                for (auto setRec : candiNonConfSets) {
                    if (setRec.second < small) {
                        choice = setRec.first;
                        small = setRec.second;
                    }
                }

                ++nonConfSets[choice];
                break;
            }

            case 1: {// randomly group
                auto iter = candiNonConfSets.begin();
                std::advance(iter, rand()%candiNonConfSets.size());
                ++iter->second;
                choice = iter->first;
                break;
            }

            default:
                break;
            }
        }

        eqPtr->bitIdx = choice;
    }

    BOOST_LOG_SEV(logger_mdCoding, debug)<<"bit: "<<nonConfSets.size();
    return nonConfSets.size();
}
*/

void mdCoding::randGenEqList(int nsRuleNo, int sRuleNo, int avgDep,
                             double bypassProb, int groupNo, double rewiringProb) {
    this->sRuleNo = sRuleNo;
    this->nsRuleNo = nsRuleNo;

    // devide the sRules into groups
    vector<int> sGroupDiv;
    int sGroupSize = sRuleNo/groupNo;

    for (int i = 0; i < groupNo; ++i) {
        int nextDelim = sGroupSize * i;

        if (i != 0)  // randomize
            nextDelim += rand()%sGroupSize - sGroupSize/2;

        sGroupDiv.push_back(nextDelim);
        // BOOST_LOG_SEV(logger_mdCoding, debug)<<"sgroup: "<<nextDelim;
    }

    sGroupDiv.push_back(sRuleNo);
    // BOOST_LOG_SEV(logger_mdCoding, debug)<<"sgroup: "<<sRuleNo;

    // devide the nsRules into groups
    vector<int> nsGroupDiv;
    int nsGroupSize = sRuleNo/groupNo;

    for (int i = 0; i < groupNo ; ++i) {
        int nextDelim = nsGroupSize * i;

        if (i != 0)  // randomize
            nextDelim += rand()%nsGroupSize - nsGroupSize/2;

        nsGroupDiv.push_back(nextDelim);
        // BOOST_LOG_SEV(logger_mdCoding, debug)<<"nsgroup: "<<nextDelim;
    }

    nsGroupDiv.push_back(nsRuleNo);
    // BOOST_LOG_SEV(logger_mdCoding, debug)<<"nsgroup: "<<nsRuleNo;


    // BOOST_LOG_SEV(logger_mdCoding, debug)<<"groupNo: "<<groupNo;
    // BOOST_LOG_SEV(logger_mdCoding, debug)<<"rewiringProb: "<<rewiringProb;

    for (int groupID = 0; groupID < groupNo; ++groupID) {
        // BOOST_LOG_SEV(logger_mdCoding, debug)<<"groupNo: "<<groupID;

        vector<int> sRuleIDs;  // create sRuleIDs for shuffling
        for (int i = sGroupDiv[groupID]; i < sGroupDiv[groupID+1]; ++i) {
            sRuleIDs.push_back(i);
        }


        for (int nsRuleID = nsGroupDiv[groupID]; nsRuleID < nsGroupDiv[groupID+1]; ++nsRuleID) {
            // BOOST_LOG_SEV(logger_mdCoding, debug)<<"\t nsRule: "<<nsRuleID << " init ";
            mdEquation eq;
            eq.randGenMdEq(avgDep, sRuleIDs, sRuleNo, rewiringProb, bypassProb);
            eqList.push_back(eq);
        }
    }

    // debug,
    /*
    for (auto eq: eqList){
    BOOST_LOG_SEV(logger_mdCoding, debug)<<eq.toStr();
    }*/
}

void mdCoding::codingVerify() {
    bool result;
    for (auto eq : eqList) {
        if(!eq.verifyEq(tagSRule)){
            BOOST_LOG_SEV(logger_mdCoding, error) << "Verification Failed";	
	    exit(0);
	}
    }

    BOOST_LOG_SEV(logger_mdCoding, info) << "Verification Passed";

}
