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
#include "../sharedHeader.h"
#include "../mathTools.h"
#include <sstream>

using std::map;
using std::pair;
using std::unordered_set;
using std::string;
using std::stringstream;
using std::vector;
using std::set;
using std::queue;

class mdEquation {
private:
    map<int, bool> param;  // id, bit
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count(); // a time-based seed

    logging_src::severity_logger< severity_level > logger_mdEquation;
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

    // initiates SRule Tag to 0
    inline void initTag(map<int, pair<int, int> > & tagSRule);

    inline void assignSrule(map<int, pair<int, int> > & tagSRule);

    // conflict check
    inline pair<bool, bool> calConflict(mdEquation * anotherEq);
    inline void mergeNonConf(mdEquation * anotherEq, bool reverse);

    // debug
    inline void randGenMdEq(int degree, vector<int>& sRuleIDs, int sRuleNo, double rewiringProb, double bypassProb);

    inline bool verifyEq (map<int, pair<int, int> > & tagSRule);

    inline string toStr();
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

pair<bool, bool> mdEquation::calConflict(mdEquation * rPtr) {
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
        return std::make_pair(true, false);
    }

    return std::make_pair(false, negative);
}

void mdEquation::mergeNonConf( mdEquation * rPtr, bool rev){
    if (!rev){
    	for (auto para: rPtr->param){
	    if (param.find(para.first) != param.end()){
	    	assert (param[para.first] == para.second);
	    }
	    else{
	   	param[para.first] = para.second; 
	    }
	}
    }
    else{
    	for (auto para: rPtr->param){
	    if (param.find(para.first) != param.end()){
	    	assert (param[para.first] != para.second);
	    }
	    else{
	   	param[para.first] = !para.second; 
	    }
	}
    }
}

void mdEquation::initTag(map<int, pair<int, int> > & sRuleTag) {
    for (auto iter = param.begin(); iter != param.end(); ++iter) {
        sRuleTag[iter->first] = std::make_pair(0,0);
    }
}

void mdEquation::assignSrule(map<int, pair<int, int> > & sRuleTag) {
    // BOOST_LOG_SEV(logger_mdEquation, debug) << "assign " <<toStr();

    for (auto iter = param.begin(); iter != param.end(); ++iter) {  // determine the sRule Tags.
        auto sTagIter = sRuleTag.find(iter->first);

        assert (sTagIter != sRuleTag.end());

        bool sTagBit = checkBit(sTagIter->second.first, bitIdx);
        bool sTagSet = checkBit(sTagIter->second.second, bitIdx);

    	// BOOST_LOG_SEV(logger_mdEquation, debug) << "\t Before assign sRule " << iter->first;
    	// BOOST_LOG_SEV(logger_mdEquation, debug) << "\t\t\t sTag: " << sTagIter->second.first<<" mask:"<<sTagIter->second.second;

        if (sTagSet)
            assert (sTagBit != (iter->second ^ bit));
        else {
            setBit(sTagIter->second.second, bitIdx);  // make it a set

            if (!(iter->second ^ bit))
                setBit(sTagIter->second.first, bitIdx);
        }

    	// BOOST_LOG_SEV(logger_mdEquation, debug) << "\t After assigned sRule " << iter->first;
    	// BOOST_LOG_SEV(logger_mdEquation, debug) << "\t\t\t sTag: " << sTagIter->second.first<<" mask:"<<sTagIter->second.second;
    }
}

void mdEquation::randGenMdEq(int degree, vector<int> & sRuleIDs, int sRuleNo, double rewiringProb, double bypassProb) {
    // the rela id
    std::random_shuffle(sRuleIDs.begin(), sRuleIDs.end());
    degree = rand() % degree + degree/2;

    for (int i = 0; i < degree && i < sRuleIDs.size(); ++i) {
        // calculate whether bypass or let through
        bool action = true;

        // double val = (double)rand()/RAND_MAX;
        if ((double)rand()/RAND_MAX < bypassProb) {
            // if (val < bypassProb) {
            //    BOOST_LOG_SEV(logger_mdEquation, debug) << "0 " <<val;
            action = false;
            hasByPass = true;
        }

        // calculate whether to rewire
        if ((double)rand()/RAND_MAX < rewiringProb) { // rewire
            addDep(rand()%sRuleNo, action);
        }
        else {
            addDep(sRuleIDs[i], action);
        }
    }
}

bool mdEquation::verifyEq(map<int, pair<int, int> > & tagSRule) {
    // BOOST_LOG_SEV(logger_mdEquation, debug) << "verification: ";
    // BOOST_LOG_SEV(logger_mdEquation, debug) << "\t nsRule: bitIdx = " << bitIdx \
	    <<" , bit = " << bit;

    if (bitIdx == -1) { // if ns not tagged, has bypass => false, no bypass => true
	return !hasByPass;
    }

    // BOOST_LOG_SEV(logger_mdEquation, debug) << "\t sRule:";

    for (auto para: param) { // ns is tagged
	if (tagSRule.find(para.first) == tagSRule.end()) // s is not tagged
	    return false; 
    
        int sTag = tagSRule[para.first].first;

	//BOOST_LOG_SEV(logger_mdEquation, debug) << "\t\t param Idx: "<< para.first \
	// 	<<" tag: " << sTag;

        if (checkBit(sTag, bitIdx) == (bit ^ para.second) )  // s tag is not correct 
            return false;
    }
    return true;
}

string mdEquation::toStr() {
    stringstream ss;
    ss<<"Equation Info: \n";
    ss<<"\t bitIdx: "<< bitIdx <<"\n";
    ss<<"\t bit: "<< bit<<"\n";
    ss<<"\t hassByPass: "<<hasByPass<<"\n";
    ss<<"\t param: \n";
    for (auto para : param) {
        ss<<"\t\t X:"<<para.first<<" pos:"<<para.second<<"\n";
    }
    return ss.str();
}

#endif
