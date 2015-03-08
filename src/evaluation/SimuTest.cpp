#include "SimuTest.h"


SimuTest::SimuTest(){};

void setConf(const char file_name []){
    ifstream file(file_name);

    for (string line; getline(file, line); ){
        vector<string> temp;
        boost::split(temp, line, boost::is_any_of("\t"));

        if (temp[0] == "config"){
            temp[1];
            // .... 
        }

    }
}

void SimuTest::evaluatePerFlow(){

}

void SimuTest::evaluateOBS(){

    // Statically place all rules
    
    
    // Run traces

}

void SimuTest::evaluateBigMaC(){
    // Run packet
    
    double curT = 0;

    boost::unordered_set<addr_5tup> flow_rec;
    
    try {
        boost::iostreams::filtering_istream in;
        in.push(boost::iostreams::gzip_decompressor());
        ifstream infile(tracefile_str);
        in.push(infile);

        string str; // tune curT to that of first packet
        getline(in,str);
        addr_5tup first_packet(str);
        curT = first_packet.timestamp;

        while(getline(in, str)) {
            addr_5tup packet(str);
            curT = packet.timestamp;
            auto res = flow_rec.insert(packet);

            if (res.second){// new flow
                pair<int,int> rules_idx = rList->search_match_rules(packet);
            }

            if (curT > simuT)
                break;
        }
    } catch (const io::gzip_error & e) {
        cout<<e.what()<<endl;
    }

}
