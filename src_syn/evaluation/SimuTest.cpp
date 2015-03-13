#include "SimuTest.h"

SimuTest::SimuTest(){
    graph_random = true; // default Internet;
};

void SimuTest::setConf(const char file_name []){
    ifstream file(file_name);

    for (string line; getline(file, line); ){
        vector<string> temp;
        boost::split(temp, line, boost::is_any_of("\t"));

        // Graph
        if (temp[0] == "Graph type:"){
            if (temp[1] == "-I"){
                graph_random = true;
                continue;
            }
            if (temp[1] == "-F"){
                graph_random = false;
                continue;
            }
        }

        if (temp[0] == "Graph file:"){
            graph_file = temp[1];
        }

        if (temp[0] == "Edge switch No.:")
            edge_no = atoi(temp[1].c_str());
            
        if (temp[0] == "Routing file:"){
            routing_file = temp[1];
            if (temp[2] == "0")
                create_routing_file = false;
            else
                create_routing_file = true;
        }

        if (temp[0] == "FatTree K Val"){
            kVal = atoi(temp[1].c_str());
        }

        // Rule set
        if (temp[0] == "Non-switching rules:")
            ns_rule_file = temp[1];

        if (temp[0] == "Switching rules:"){
            s_rule_file = temp[1];
            if (temp[2] == "0")
                create_sRule_file = false;
            else
                create_sRule_file = true;
        }

        if (temp[0] == "Evolving para:"){
            vector<string> temp_1;
            split(temp_1, temp[1], is_any_of(" "));
            for (int i = 0; i < 3; ++i)
                evolving_para[i] = boost::lexical_cast<double>(temp_1[i]);
        }

        // Simulation control
        if (temp[0] == "Simulation time:")
            simuT = atoi(temp[1].c_str());
        if (temp[0] == "Flow rate:")
            flow_rate = boost::lexical_cast<double>(temp[1]);
    }
}

void SimuTest::init(){

    // init graph
    if (graph_random){  // Internet
        if (create_routing_file){
            iGraph.ReadGraph(graph_file.c_str(), edge_no);
            iGraph.CalShortestPath(routing_file);
        }
        else{
            iGraph.ReadPath(routing_file);
        }
    }
    else{ // Fat tree
        fGraph = FatTreeGraph(kVal);
    }

    cout<<"info: SimuTest: Graph Loaded"<<endl;

    // init rule set
    if (create_sRule_file){
        rList = rule_list(ns_rule_file.c_str());
        rList.evolve_gen(s_rule_file.c_str(), evolving_para[0],
                         evolving_para[1], evolving_para[2]);
    }
    else
        rList = rule_list(ns_rule_file.c_str(), s_rule_file.c_str());

    rList.obtain_ns_assoc();

    cout<<"info: SimuTest: Rules Loaded"<<endl;

    // generate mapping from sRule to routes
    if (graph_random){
        // Internet
        vector<int> rand_src;
        for (int i = 0; i < edge_no; ++i)
            rand_src.push_back(i);
    
        for (int i = 0; i < rList.list_switching.size(); ++i){
            random_shuffle(rand_src.begin(), rand_src.end());
            rule_route_map.push_back(iGraph.query_path(rand_src[0], rand_src[1]));
        }
    }
    else{
        // Fat Tree
        vector<int> rand_src_fattree;
        for (int i = 0; i < kVal*kVal/2; ++i)
            rand_src_fattree.push_back(i);

        for (int i = 0; i < rList.list_switching.size(); ++i){
            random_shuffle(rand_src_fattree.begin(), rand_src_fattree.end());
            rule_route_map_ecmp.push_back(fGraph.query_all_path(rand_src_fattree[0], 
                                                              rand_src_fattree[1], kVal/2)
                                         );
        }
    }

    cout<<"info: SimuTest: Rule Routing Map Generated"<<endl;
}

void SimuTest::setFlowRate(double fRate){
    flow_rate = fRate;
}

void SimuTest::evaluatePerFlow(){
    // init traffic generator
    tGen = synTraceGen(&rList, flow_rate);
    
    double curTime = 0.0; // in msec
    double check_point = 10000.0;
    double check_interval = 10000.0;
    int check_point_count = 1;

}

void SimuTest::evaluateOBS(){
    // init traffic generator
    tGen = synTraceGen(&rList, flow_rate);
    cout<<"BigMaC running at "<<flow_rate <<"/sec"<<endl; 

    srand(time(NULL));

    double curTime = 0.0; // in msec
    double check_point = 10000.0;
    double check_interval = 10000.0;
    int check_point_count = 1;

    if (graph_random){
        bigmac bigmac_sys = bigmac(&rList, iGraph.vertice_no, rule_route_map);  
    
        while (curTime < simuT*1000){
            auto flow = tGen.fetch_next();
            curTime = flow.first.timestamp;
    
            if (flow.second){ // arrives
                int sRuleID = rList.search_match_sRule(flow.first);
                bigmac_sys.MaC(sRuleID);
            }
    
            if (curTime > check_point){
                cout<<"checkpoint: "<<check_point/1000<<" sec"<<endl;
                ++check_point_count;
    
                check_point += check_interval;
            }
        }
        auto usage = bigmac_sys.check_usage();
    
        cout << "flow table usage:" <<endl;
        cout << "   min: "<<usage[0]<<endl;
        cout << "   avg: "<<usage[1]<<endl;
        cout << "   max: "<<usage[2]<<endl;
        }
    }
}

void SimuTest::evaluateBigMaC(){
    // init traffic generator
    tGen = synTraceGen(&rList, flow_rate);
    cout<<"BigMaC running at "<<flow_rate <<"/sec"<<endl; 

    srand(time(NULL));

    double curTime = 0.0; // in msec
    double check_point = 10000.0;
    double check_interval = 10000.0;
    int check_point_count = 1;

    double tag_check_point = 100.0;
    int tag_check_point_count = 1;
    double tag_check_interval = 100.0;

    vector<double> res_usage(3,0);
    int tags = 0;

    bigmac bigmac_sys = bigmac(&rList, iGraph.vertice_no, rule_route_map);  

    while (curTime < simuT*1000){
        auto flow = tGen.fetch_next();
        curTime = flow.first.timestamp;

        if (flow.second){ // arrives
            int sRuleID = rList.search_match_sRule(flow.first);
            bigmac_sys.MaC(sRuleID);
        }
        else{ // leaves
            int sRuleID = rList.search_match_sRule(flow.first);
            bigmac_sys.Evict(sRuleID);
        }

        if (curTime > check_point){
            cout<<"checkpoint: "<<check_point/1000<<" sec"<<endl;
            ++check_point_count;

            auto usage = bigmac_sys.check_usage();

            for(int i = 0; i < 3; ++i)
                res_usage[i] += usage[i];

            check_point += check_interval;
        }
        
        if (curTime > tag_check_point){
            tags += bigmac_sys.check_tags();
            tag_check_point += tag_check_interval;
            ++tag_check_point_count; 
        }
    }

    cout << "flow table usage:" <<endl;
    cout << "   min: "<<res_usage[0]/check_point_count <<endl;
    cout << "   avg: "<<res_usage[1]/check_point_count <<endl;
    cout << "   max: "<<res_usage[2]/check_point_count <<endl;

    cout << "tags: "<< tags << " in " << tag_check_point_count <<" checks" <<endl;

    cout << "costs:" << bigmac_sys.cal_avg_cost() <<endl;
}
