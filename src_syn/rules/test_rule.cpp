#include "../stdafx.h"
#include "RuleList.hpp"
#include "synTraceGen.h"
#include "BucketTree.h"

int main(int argc, char* argv[]){
    if (argc < 3){
        cout<<"missing input: -r <rule_file> -sr <ev_rule_file>"<<endl;
        return -1;
    } 

    srand(time(NULL));

    if (!strcmp(argv[1], "-r")){
        rule_list rList;
    
        if ( argc >= 5 && !strcmp(argv[3], "-sr"))
            rList = rule_list(argv[2], argv[4]);
        else{
            rList = rule_list(argv[2]);
            rList.evolve_gen("../../data/srule_10k.dat", 2, 3, 0);
        }
       
        ofstream file("trace_rec.txt");

        synTraceGen tGen (&rList, "~/BigMaC/config/traf_gen.conf");
    
        int arrival_count = 0;
        for (int i = 0; i < 1000000; ++i){
            auto packet = tGen.fetch_next();
            if (packet.second){
                file<<"flow: "<<packet.first.str_readable()<<" arrives"<<endl;
                ++arrival_count;
            }
            else
                file<<"flow: "<<packet.first.str_readable()<<" leaves"<<endl;
        }
        cout<<"arrival rate : "<<double(arrival_count)/1000<<endl;
    }
}
