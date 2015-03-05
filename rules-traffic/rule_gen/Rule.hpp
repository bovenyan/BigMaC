#ifndef RULE_H
#define RULE_H

#include "stdafx.h"
#include "Address.hpp"
#include <boost/functional/hash.hpp>

using std::pair;

class p_rule {
  public:
    pref_addr hostpair[2];
    range_addr portpair[2];
    bool proto;

    bool hit;

  public:

    // constructor
    inline p_rule();
    inline p_rule(const p_rule & pr);
    inline p_rule(const std::string & rule_string, bool is_testbed = false);

    // comparator
    inline bool operator==(const p_rule & rhs) const;
    inline bool dep_rule(const p_rule & rhs) const;
    inline bool packet_hit(const addr_5tup & packet) const;

    // generate header
    inline addr_5tup get_corner() const;
    inline addr_5tup get_random() const;

    // gen rule
    inline vector<p_rule> evolve_rule(const p_rule & pr, double offspring,
		    		      double scale, double offset) const;
    inline std::pair<p_rule, bool> join_rule(p_rule) const;

    // debug and print
    inline std::string get_str() const;
    inline void print() const;
};

class r_rule {
  public:
    range_addr addrs[4];
    bool proto;

  public:
    inline r_rule();
    inline r_rule(const r_rule &);
    inline r_rule(const p_rule &);

    inline bool operator==(const r_rule &) const ;
    inline friend uint32_t hash_value(r_rule const &);

    inline bool overlap(const r_rule &) const;
    inline void prune_mic_rule(const r_rule &, const addr_5tup &); // Mar 14

    inline std::string get_str() const;
};

/* ----------------------------- p_rule----------------------------------
 * brief:
 * bucket_rules: two dim are prefix address, the other two are range address
 */

/* constructors
 *
 * options:
 * 	()			default
 * 	(const p_rule &)	copy function
 * 	(const string &)	generate from a string "#srcpref/mask \t dstpref/mask \t ..."
 */
inline p_rule::p_rule():hit(false) {}

inline p_rule::p_rule(const p_rule & pr) {
    hostpair[0] = pr.hostpair[0];
    hostpair[1] = pr.hostpair[1];
    portpair[0] = pr.portpair[0];
    portpair[1] = pr.portpair[1];
    proto = pr.proto;

    hit = pr.hit;
}

inline p_rule::p_rule(const string & rule_str, bool test_bed) {
    vector<string> temp;
    boost::split(temp, rule_str, boost::is_any_of("\t"));
    temp[0].erase(0,1);
    hostpair[0] = pref_addr(temp[0]);
    hostpair[1] = pref_addr(temp[1]);

    if (test_bed) {
        portpair[0] = range_addr();
        portpair[1] = range_addr();
    } else {
        portpair[0] = range_addr(temp[2]);
        portpair[1] = range_addr(temp[3]);
    }
    proto = true;
    hit = false;
}

inline bool p_rule::operator==(const p_rule & rhs) const {
    if (!(hostpair[0] == rhs.hostpair[0]))
        return false;
    if (!(hostpair[1] == rhs.hostpair[1]))
        return false;
    if (!(portpair[0] == rhs.portpair[0]))
        return false;
    if (!(portpair[1] == rhs.portpair[1]))
        return false;
    return true;
}

inline bool p_rule::dep_rule(const p_rule & rl) const { // check whether a rule is directly dependent
    if (!hostpair[0].match(rl.hostpair[0]))
        return false;
    if (!hostpair[1].match(rl.hostpair[1]))
        return false;
    if (!portpair[0].overlap(rl.portpair[0]))
        return false;
    if (!portpair[1].overlap(rl.portpair[1]))
        return false;

    return true;
}

inline bool p_rule::packet_hit(const addr_5tup & packet) const { // check whehter a rule is hit.
    if (!hostpair[0].hit(packet.addrs[0]))
        return false;
    if (!hostpair[1].hit(packet.addrs[1]))
        return false;
    if (!portpair[0].hit(packet.addrs[2]))
        return false;
    if (!portpair[1].hit(packet.addrs[3]))
        return false;
    // ignore proto check
    return true;
}

// Feb. 4
inline vector<p_rule> p_rule::evolve_rule(const p_rule & pr, double offspring,
		   			  double scale, double offset) const{
    vector<p_rule> new_rules;
    int offspring_no = rand() % int(offset*2)+1;    // 1 ~ floor(offset)*2+1

    for (int i = 0; i < offspring_no; ++i){	
        p_rule gen_rule = pr;
        gen_rule.hostpair[0].shrink_shift( rand() % int(scale * 2) + 1,
                                           rand() % int(offset * 2) + 1);
        gen_rule.hostpair[1].shrink_shift( rand() % int(scale * 2) + 1,
                                           rand() % int(offset * 2) + 1);
        gen_rule.portpair[0].shrink_shift( rand() % int(scale * 2) + 1,
                                           (rand() % 2 * 2 -1) * (rand() % int(offset) + 1));
        gen_rule.portpair[1].shrink_shift( rand() % int(scale * 2) + 1,
                                           (rand() % 2 * 2 -1) * (rand() % int(offset) + 1));
        new_rules.push_back(gen_rule);
    }
    return new_rules;
}

inline pair<p_rule, bool> p_rule::join_rule(p_rule pr) const { // use this rule to join another
    if (!hostpair[0].truncate(pr.hostpair[0]))
        return std::make_pair(p_rule(), false);
    if (!hostpair[1].truncate(pr.hostpair[1]))
        return std::make_pair(p_rule(), false);
    if (!portpair[0].truncate(pr.portpair[0]))
        return std::make_pair(p_rule(), false);
    if (!portpair[1].truncate(pr.portpair[1]))
        return std::make_pair(p_rule(), false);
    return std::make_pair(pr, true);
}

inline addr_5tup p_rule::get_corner() const { // generate the corner as did by ClassBench
    addr_5tup header;
    header.addrs[0] = hostpair[0].get_extreme(rand()%2);
    header.addrs[1] = hostpair[1].get_extreme(rand()%2);
    header.addrs[2] = portpair[0].get_extreme(rand()%2);
    header.addrs[3] = portpair[1].get_extreme(rand()%2);
    header.proto = true;
    return header;
}

inline addr_5tup p_rule::get_random() const { // generate a random header from a rule
    addr_5tup header;
    header.addrs[0] = hostpair[0].get_random();
    header.addrs[1] = hostpair[1].get_random();
    header.addrs[2] = portpair[0].get_random();
    header.addrs[3] = portpair[1].get_random();
    header.proto = true;
    return header;
}

inline void p_rule::print() const {
    cout<<get_str()<<endl;
}

inline string p_rule::get_str() const {
    stringstream ss;
    ss<<hostpair[0].get_str()<<"\t";
    ss<<hostpair[1].get_str()<<"\t";
    ss<<portpair[0].get_str()<<"\t";
    ss<<portpair[1].get_str()<<"\t";
    if (proto)
        ss<<"tcp";
    else
        ss<<"udp";
    return ss.str();
}


inline r_rule::r_rule() {
    addrs[0] = range_addr(0, ~uint32_t(0));
    addrs[1] = range_addr(0, ~uint32_t(0));
    addrs[2] = range_addr(0, (~uint32_t(0) >> 16));
    addrs[3] = range_addr(0, (~uint32_t(0) >> 16));
}

inline r_rule::r_rule(const r_rule & pr) {
    for (uint32_t i = 0; i < 4; ++i)
        addrs[i] = pr.addrs[i];
}

inline r_rule::r_rule(const p_rule & pr) {
    addrs[0] = range_addr(pr.hostpair[0]);
    addrs[1] = range_addr(pr.hostpair[1]);
    addrs[2] = pr.portpair[0];
    addrs[3] = pr.portpair[1];
    proto = pr.proto;
}

/* operator functions
 *
 * for hash based containers
 */

inline bool r_rule::operator==(const r_rule & rhs) const {
    for (uint32_t i = 0; i < 4; i++)
        if (!(addrs[i] == rhs.addrs[i]))
            return false;
    return true;
}

inline uint32_t hash_value(r_rule const & rr) { // hash the detailed range value for the value;
    size_t seed = 0;
    for (uint32_t i = 0; i < 4; ++i) {
        boost::hash_combine(seed, hash_value(rr.addrs[i]));
    }
    return seed;
}

/* member functions
 */
inline bool r_rule::overlap (const r_rule & rr) const { // check whether another r_rule overlap with it.
    for (uint32_t i = 0; i < 4; ++i) {
        if (!addrs[i].overlap(rr.addrs[i]))
            return false;
    }
    return true;
}


inline void r_rule::prune_mic_rule(const r_rule & rr, const addr_5tup & pack) { // use another r_rule, and pack to prune to a r_rule that tightest to the packet hit.
    for (uint32_t i =0; i< 4; ++i) {
        addrs[i].getTighter(pack.addrs[i], rr.addrs[i]);
    }
}

/* print and debug
 */
inline string r_rule::get_str() const {
    stringstream ss;
    for(uint32_t i = 0; i < 4; i++) {
        ss<<addrs[i].get_str()<<"\t";
    }
    return ss.str();
}

#endif
