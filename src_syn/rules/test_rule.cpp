#include "../stdafx.h"
#include "RuleList.hpp"
#include "synTraceGen.h"


int main(int argc, char* argv[]){
    if (argc < 3){
        cout<<"missing input: -r <rule_file> -sr <ev_rule_file>"<<endl;
        return -1;
    } 

    
    if (!strcmp(argv[1], "-r")){
        rule_list rList(argv[2]);
    
        if ( argc >= 5 && !strcmp(argv[3], "-sr"))
            rList.evolve_gen(argv[4], 2, 3, 0);
        else{
            rList.evolve_gen("../../data/srule_10k.dat", 2, 3, 0);
        }
        
        /*
        synTraceGen tGen (&rList, "~/BigMaC/conf/conf_set");
        
        for (int i = 0; i < 10000; ++i){
            auto packet = tGen.fetch_next();
            if (packet.second)
                cout<<"flow: "<<packet.first.str_readable()<<" arrives"<<endl;
            else
                cout<<"flow: "<<packet.first.str_readable()<<" leaves"<<endl;
                
        }
        */
    }
}
