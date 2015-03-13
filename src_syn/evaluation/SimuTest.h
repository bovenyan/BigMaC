#ifndef EVALUATION
#define EVALUATION

#include "../rules/synTraceGen.h"
#include "../routing/GraphGen.h"
#include "BigMaC.h"

class SimuTest{
    private:
    // graph
    bool graph_random;
    string graph_file;
    int edge_no;
    string routing_file;
    bool create_routing_file;
    int kVal;
   
    // rule set
    string ns_rule_file;
    string s_rule_file;
    bool create_sRule_file;
    double evolving_para[3];

    // simu config
    int simuT; // in sec
    double flow_rate;

    // data structure
    InternetGraph iGraph;
    FatTreeGraph fGraph;
    rule_list rList;
    synTraceGen tGen;
    vector<vector<int> > rule_route_map;
    vector<vector<vector<int> > > rule_route_map_ecmp;
    
    public:
    SimuTest();
    void setConf(const char file_name[]);
    void init();
    void setFlowRate(double fRate);

    public:
    void evaluatePerFlow();
    void evaluateOBS();
    void evaluateBigMaC();
};
#endif
