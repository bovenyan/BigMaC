#ifndef SYN_TRACE_GEN_H
#define SYN_TRACE_GEN_H

#include "../stdafx.h"
#include <cmath>
#include "RuleList.hpp"
#include <queue>

using std::vector;
using std::pair;
using std::priority_queue;

class cmp_addr_5tup{
    public:
        bool operator()(const addr_5tup & lhs, const addr_5tup & rhs) const{
            return (lhs.timestamp > rhs.timestamp);
        }
};

class synTraceGen{
    private:
    rule_list * rList;
    vector<addr_5tup> headers;

    double flow_rate;
    double fetch_time;
    
    priority_queue<addr_5tup, vector<addr_5tup>, cmp_addr_5tup> active_flows;
    addr_5tup next_arr_flow;
    bool on_arrival;
    double prev_arrival;

    double gen_int();
    double gen_len();

    public:
    synTraceGen(rule_list * rL, const char trace_conf []);

    // flow starts or ends
    pair<addr_5tup, bool> fetch_next();
};

#endif
