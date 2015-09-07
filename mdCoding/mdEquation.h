#ifndef MD_EQUATION_H
#define MD_EQUATION_H

#include <map>
#include <unordered_set>
#include <string>
#include <chrono>
#include <random>
#include <algorithm>
#include <set>
#include <queue>
#include <cassert>
#include "mathTools.h"

using std::map;
using std::pair;
using std::unordered_set;
using std::string;
using std::vector;
using std::set;
using std::queue;

class mdEquation {
private:
    map<int, bool> param;  // id, bit
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count(); // a time-based seed
public:
    // map<mdEquation *, bool> cohereNeigh;
    int bitIdx = -1;
    bool bit = false;
    bool hasByPass = false;
    unordered_set<mdEquation *> conflictNeigh;
    pair<unsigned int, unsigned int> mdCode; // value, mask

public:
    mdEquation() {};

    // dependency add / check
    inline void addDep(int idx, bool pos) {
        param[idx] = pos;
    }

    inline void addDep(string str);

    inline void checkDep(vector<set<mdEquation *> > & sToNsMap);

    inline vector<int> depSRule();

    inline void initTag(map<int, pair<int, int> > & tagSRule); // init tag when need to

    inline void assign(map<int, pair<int, int> > & tagSRule);

    // conflict check
    inline bool calConflict(mdEquation * anotherEq);

    // debug
    void randGenMdEq(int degree, vector<int>& sRuleIDs, int sRuleNo, double rewiringProb, double bypassProb);
};

void mdEquation::addDep(string str) {
    string::size_type i = 0;
    string::size_type j = str.find('+');
    string::size_type k = str.find('-');

    // "1+2+3-4+5-"
    while (j != string::npos || k != string::npos) {
        string::size_type nextIdx;
        if (j == string::npos) { // '+' is not found
            param[std::stoi(str.substr(i, k-i), &nextIdx)] = false;
            i = ++k;
            k = str.find('-',k);
            continue;
        }

        if (k == string::npos) { // '-' is not found
            param[std::stoi(str.substr(i, j-i), &nextIdx)] = true;
            i = ++j;
            j = str.find('+',j);
            continue;
        }

        if (j < k) { // '+' is earlier than '-'
            param[std::stoi(str.substr(i, j-i))] = true;
            j = str.find('+',j);
        }
        else { // '-' is earlier than '+'
            param[std::stoi(str.substr(i, k-i))] = false;
            k = str.find('-',k);
        }
    }
}

void mdEquation::checkDep(vector <set<mdEquation *> > & sToNsMap) {
    for (auto dep : param) {
        sToNsMap[dep.first].insert(this);
    }
}

vector<int> mdEquation::depSRule() {
    vector<int> sRuleIDs;
    for (auto dep : param) {
        sRuleIDs.push_back(dep.first);
    }
    return sRuleIDs;
}

bool mdEquation::calConflict(mdEquation * rPtr) {
    bool positive = true;
    bool negative = true;

    for (auto iter = param.begin(); iter != param.end(); ++iter) {
        if (rPtr->param.find(iter->first) != rPtr->param.end()) {
            if (rPtr->param[iter->first] == iter->second)
                negative = false;
            else
                positive = false;
        }
    }

    if (!positive && !negative) {
        conflictNeigh.insert(rPtr);
        rPtr->conflictNeigh.insert(this);
        return true;
    }

    return false;
}

void mdEquation::initTag(map<int, pair<int, int> > & sRuleTag) {
    for (auto iter = param.begin(); iter != param.end(); ++iter) {
        sRuleTag[iter->first] = std::make_pair(0,0);
    }
}

void mdEquation::assign(map<int, pair<int, int> > & tagSRule) {
    bit = true;

    for (auto iter = param.begin(); iter != param.end(); ++iter) {  // set the bit for nsRule first
        auto sTagIter = tagSRule.find(iter->first);
           
	assert (sTagIter != tagSRule.end());

	if (checkBit(sTagIter->second.second, bitIdx) == 0) //
		continue;
	if (checkBit(sTagIter->second.first, bitIdx) == iter->second){
		bit = false;
		break;
	}
    }

    for (auto iter = param.begin(); iter != param.end(); ++iter) {
        auto sTagIter = tagSRule.find(iter->first);

        if (sRulePtr != sRuleBit.end()) { // assure conflict analysis
            assert (sRulePtr->second != (iter->second ^ bit));
        }
        else {
            sRuleBit[iter->first] = !(iter->second ^ bit);
        }
    }
}

void mdEquation::randGenMdEq(int degree, vector<int> & sRuleIDs, int sRuleNo, double rewiringProb, double bypassProb) {
    // the rela id
    std::random_shuffle(sRuleIDs.begin(), sRuleIDs.end());

    degree = rand() % degree + degree/2;
    for (int i = 0; i < degree && i < sRuleIDs.size(); ++i) {
        // calculate whether bypass or let through
        bool action = true;
        if ((double)rand()/RAND_MAX > bypassProb) {
            action = false;
            hasByPass = true;
        }

        // calculate whether to rewire
        if ((double)rand()/RAND_MAX > rewiringProb)
            addDep(sRuleIDs[i], action);
        else
            addDep(rand()%sRuleNo, action);
    }
}

#endif
