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
    
    priority_queue<addr_5tup, vector<addr_5tup>, cmp_addr_5tup> active_flows;
    addr_5tup next_arr_flow;

    bool on_arrival;
    double prev_arrival;

    double gen_int();
    double gen_len();

    public:
    synTraceGen();
    synTraceGen(rule_list * rL, double flow_rate);
    synTraceGen(rule_list * rL, const char trace_conf []);

    // flow starts or ends
    inline pair<addr_5tup, bool> fetch_next();
};

inline pair<addr_5tup, bool> synTraceGen::fetch_next(){
    if (!on_arrival){ // generate new packet
        next_arr_flow = headers.at(rand()%headers.size());
        next_arr_flow.timestamp = prev_arrival + gen_int();
        
        addr_5tup next_leave_flow = next_arr_flow;
        next_leave_flow.timestamp += gen_len();

        active_flows.push(next_leave_flow);
        prev_arrival = next_arr_flow.timestamp;

        on_arrival = true;
    }

    if (next_arr_flow.timestamp > active_flows.top().timestamp){ // leave first
        pair<addr_5tup, bool> res(active_flows.top(), false);
        active_flows.pop();
        return res;
    }
    else{ // arrival first
        on_arrival = false;
        return std::make_pair(next_arr_flow, true);
    }
}

#endif
