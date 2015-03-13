#include "synTraceGen.h"

using std::make_pair;

synTraceGen::synTraceGen(){}

synTraceGen::synTraceGen(rule_list * rL, double fRate){
    rList = rL;
    flow_rate = fRate;

    headers = rList->header_prep();
    on_arrival = false;
    prev_arrival = 0;
}

synTraceGen::synTraceGen(rule_list * rL, const char trace_conf[]){
    rList = rL;

    ifstream file(trace_conf);

    for (string line; getline(file, line);){
        vector<string> temp;
        boost::split(temp, line, boost::is_any_of("\t"));
        if (temp[0] == "flow rate"){
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
   
    // scaling to the right flow rate
    double scaling_val = 500;
    scaling_val /= flow_rate;

    return val*scaling_val;
}

double synTraceGen::gen_len(){
    // flow-len [0.000100, 20] 1
    double key = rand()/RAND_MAX;
    double val = 5.3*key + 2;
    val = val/3;
    val = pow(10, val); // ms

    return val+1000;
}

