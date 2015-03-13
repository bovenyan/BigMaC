#include "./evaluation/SimuTest.h"

int main(int argc, const char * argv[]){
    if (argc < 3){
        cout<<"-f config_file"<<endl;
        return -1;
    }

    SimuTest test;
    test.setConf(argv[2]);
    test.init();
    for (int idx = 1; idx <= 15; ++idx){
        test.setFlowRate(1000*idx);
        test.evaluateBigMaC();
        //test.evaluateOBS();
    }
}
