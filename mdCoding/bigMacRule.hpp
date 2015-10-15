#ifndef BIGMAC_RULE_H
#define BIGMAC_RULE_H

#include "Address.hpp"
#include <unordered_set>

using std::unordered_set;

// root class for all rules and flow entries
class sRule;
class nsRule;

class rule {
private:
    unordered_set<int> placement;

public:
    pref_addr matchFields[4];

public:
    inline rule();
    inline rule(const rule & rRule);
    inline rule(const std::string & rStr, bool cBenchRule);

    // copy
    inline bool operator==(const rule & rRule) const;

    // header analysis
    inline bool overlap(const rule & rRule) const;
    inline bool hit(const addr_5tup & rRule) const;

    // gen traffic
    inline addr_5tup randCornerPacket() const;
    inline addr_5tup randPacket() const;

    // debug
    inline std::string toStr() const;
};

class sRule : public rule {
public:
    vector<int> route;

public:
    // constructor 
    inline sRule():rule() {};
    inline sRule(const std::string & rStr, bool cBenchRule)
        :rule(rStr, cBenchRule) {};

    inline void genRoute();
};

class nsRule : public rule {
private:
    vector<sRule *> depSRule;
    unordered_set<int> placement;

public:
    inline nsRule();
};


/* Definitions
 * Class: Rule
 */

/* Constructor
 */
inline rule::rule() {
    matchFields[2].mask = ((~0)<<16); // port is limited to 0~65535
    matchFields[3].mask = ((~0)<<16);
}

inline rule::rule(const rule & r) { // copy constructor
    for(uint32_t i=0; i<4; i++) {
        matchFields[i] = r.matchFields[i];
    }
}

inline rule::rule(const std::string & rule_str, bool cBenchRule = true) {
    vector<string> temp;
    boost::split(temp, rule_str, boost::is_any_of("\t"));

    if (cBenchRule) { // cast to prefix rule, the range port pairs will be casted
        temp[0].erase(0,1);
        matchFields[0] = pref_addr(temp[0]);
        matchFields[1] = pref_addr(temp[1]);
        matchFields[2] = range_addr(temp[2]).approx(true);
        matchFields[3] = range_addr(temp[3]).approx(true);
    }
    else { // directly a prefix rule
        for(uint32_t i=0; i<4; i++) {
            matchFields[i] = pref_addr(temp[i]);
        }
    }
}

/* Copy
 */
inline bool rule::operator==(const rule & rRule) const {
    for (int i=0; i<4; ++i) {
        if(!(matchFields[i] == rRule.matchFields[i]))
            return false;
    }
    return true;
}

/* Header Analysis
 */
inline bool rule::overlap(const rule & r) const {
    for (uint32_t i = 0; i < 4; ++i) {
        if (!matchFields[i].match(r.matchFields[i]))
            return false;
    }
    return true;
}

inline bool rule::hit(const addr_5tup & packet) const {
    for (uint32_t i = 0; i < 4; i++) {
        if (!matchFields[i].hit(packet.addrs[i]))
            return false;
    }
    return true;
}

/* Generate Traffic
 */
inline addr_5tup rule::randCornerPacket() const {
    addr_5tup packet;

    for (int i = 0; i < 4; ++i) {
        packet.addrs[i] = matchFields[i].get_extreme(rand()%2);
    }
    packet.proto = true;

    return packet;
}

inline addr_5tup rule::randPacket() const {
    addr_5tup packet;

    for (int i = 0; i < 4; ++i) {
        packet.addrs[i] = matchFields[i].get_random();
    }
    packet.proto = true;

    return packet;
}

/* Generate Traffic
 */
inline string rule::toStr() const { // print func
    stringstream ss;
    for(uint32_t i = 0; i < 4; i++) {
        ss<<matchFields[i].get_str()<<"\t";
    }
    return ss.str();
}

/* Definitions
 * Class: sRule
 */
#endif
