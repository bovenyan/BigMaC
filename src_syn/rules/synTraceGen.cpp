#include "synTraceGen.h"

using std::make_pair;

synTraceGen::synTraceGen(rule_list * rL, const char trace_conf[]){
    rList = rL;

    ifstream file(trace_conf);

    for (string line; getline(file, line);){
        vector<string> temp;
        boost::split(temp, line, boost::is_any_of("\t"));
        if (temp[0] == "flow rate"){
            flow_rate = boost::lexical_cast<double>(temp[1]);
        }
        if (temp[0] == "fetch_time"){
            flow_rate = boost::lexical_cast<double>(temp[1]);
        }
    }

    headers = rList->header_prep();

    on_arrival = false;
    prev_arrival = 0;
}

double synTraceGen::gen_int(){
    // y = k log x
    // inter-arrival [0.000010, 0.000200] 0.1  [0.000200, 0.2] 0.88 [0.2,1] 0.02
    double key = rand()/RAND_MAX;
    double val;
    if ( key < 0.1)
        val = key*13+1;
    else{
        if (key < 0.98)
            val = key/0.44 + 2.27727;
        else
            val = key*85 + 79; 
    }
    val = val/3;
    val = pow(10, val); // ms
    
    return val;
}

double synTraceGen::gen_len(){
    // flow-len [0.000100, 20] 1
    double key = rand()/RAND_MAX;
    double val = 4.2*key + 0.1;
    val = val/3;
    val = pow(10, val); // ms

    return val;
}

pair<addr_5tup, bool> synTraceGen::fetch_next(){
    if (!on_arrival){ // generate new packet
        next_arr_flow = headers.at(rand()%headers.size());
        next_arr_flow.timestamp = prev_arrival + gen_int();
        
        addr_5tup next_leave_flow = next_arr_flow;
        next_leave_flow.timestamp += gen_len();
        active_flows.push(next_leave_flow);
        on_arrival = true;
    }

    if (next_arr_flow.timestamp > active_flows.top().timestamp){ // leave first
        pair<addr_5tup, bool> res(active_flows.top(), false);
        active_flows.pop();
        return res;
    }
    else{ // arrival first
        on_arrival = false;
        return make_pair(next_arr_flow, true);
    }
}
