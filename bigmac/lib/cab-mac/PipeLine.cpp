#include "PipeLine.h"

using std::ifstream;
using std::ofstream;
using std::string;

pipe_line::pipe_line() {};

pipe_line::pipe_line(string & fwd_table_file,
                     string & mgmt_table_file,
                     bool test_bed) {
    this->test_bed = test_bed;

    // process fwding file
    ifstream file;
    file.open(fwd_table_file.c_str());

    string sLine = "";
    getline(file, sLine);

    while (!file.eof()) {
        fwd_rule sRule(sLine, test_bed);
        fwd_table.push_back(sRule);
        getline(file, sLine);
    }
    file.close();

    if (test_bed) {
        for(auto iter = fwd_table.begin(); iter != fwd_table.end(); ++iter) {
            for (auto iter_cp = iter+1; iter_cp != fwd_table.end(); ) {
                if (*iter == *iter_cp)
                    iter_cp = fwd_table.erase(iter_cp);
                else
                    ++iter_cp;
            }
        }
    }

    // process mgmt file
    file.open(mgmt_table_file.c_str());

    sLine = "";
    getline(file, sLine);

    while (!file.eof()) {
        mgmt_rule sRule(sLine, test_bed);
        mgmt_table.push_back(sRule);
        getline(file, sLine);
    }
    file.close();

    if (test_bed) {
        for(auto iter = mgmt_table.begin(); iter != mgmt_table.end(); ++iter) {
            for (auto iter_cp = iter+1; iter_cp != mgmt_table.end(); ) {
                if (*iter == *iter_cp)
                    iter_cp = mgmt_table.erase(iter_cp);
                else
                    ++iter_cp;
            }
        }
    }
}

void pipe_line::obtain_dep() {
    // process fwd rule
    for(uint32_t idx = 0; idx < fwd_table.size(); ++idx) {
        vector <uint32_t> dep_rules;
        for (uint32_t idx1 = 0; idx1 < idx; ++idx1) {
            if (fwd_table[idx].dep_rule(fwd_table[idx1])) {
                dep_rules.push_back(idx1);
            }
        }
        fwd_dep_map[idx] = dep_rules;
    }

    // process mgmt rule
    for(uint32_t idx = 0; idx < mgmt_table.size(); ++idx) {
        vector <uint32_t> dep_rules;
        for (uint32_t idx1 = 0; idx1 < idx; ++idx1) {
            if (mgmt_table[idx].dep_rule(mgmt_table[idx1])) {
                dep_rules.push_back(idx1);
            }
        }
        mgmt_dep_map[idx] = dep_rules;
    }
}

pair<r_rule, r_rule> pipe_line::get_micro_rules(const addr_5tup & packet) {
    // fwd rule
    uint32_t rule_hit_idx = 0;
    for ( ; rule_hit_idx < fwd_table.size(); ++rule_hit_idx ) {
        if (fwd_table[rule_hit_idx].packet_hit(packet))
            break;
    }

    if (rule_hit_idx == fwd_table.size()) {  // do not find
        cout <<"Packet hitting no fwding"<<endl;
        exit(0);
    }

    r_rule f_rule = fwd_table[rule_hit_idx];
    for (auto iter = fwd_dep_map[rule_hit_idx].begin();
            iter != fwd_dep_map[rule_hit_idx].end(); ++iter) {
        f_rule.prune_mic_rule(fwd_table[*iter], packet);
    }

    // mgmt rule
    rule_hit_idx = 0;
    for ( ; rule_hit_idx < mgmt_table.size(); ++rule_hit_idx ) {
        if (mgmt_table[rule_hit_idx].packet_hit(packet))
            break;
    }

    if (rule_hit_idx == fwd_table.size()) {  // do not find
        cout <<"Packet hitting no mgmt"<<endl;
        exit(0);
    }

    r_rule m_rule = mgmt_table[rule_hit_idx];
    for (auto iter = mgmt_dep_map[rule_hit_idx].begin();
            iter != mgmt_dep_map[rule_hit_idx].end(); ++iter) {
        m_rule.prune_mic_rule(mgmt_table[*iter], packet);
    }

    return std::make_pair(f_rule, m_rule);
}

pair<int, int> pipe_line::get_matched_rules(const addr_5tup & packet) {
    size_t i = 0;
    for (; i < fwd_table.size(); ++i) {
        if (fwd_table[i].packet_hit(packet))
            break;
    }

    assert (i != fwd_table.size());

    int j = 0;
    for (; i < mgmt_table.size(); ++i) {
        if (mgmt_table[i].packet_hit(packet))
            break;
    }

    assert (j != mgmt_table.size());

    return std::make_pair(i, j);
}

/*
 * debug and print
 */
void pipe_line::print(const string & filename) {
    ofstream file;
    file.open(filename.c_str());

    file << "FWDing Rules" << endl;
    for (auto iter = fwd_table.begin(); 
            iter != fwd_table.end(); iter++) {
        file<<iter->get_str()<<endl;
    }

    file << endl;
    file << "MGMT Rules" << endl;

    for (auto iter = mgmt_table.begin(); 
            iter != mgmt_table.end(); iter++) {
        file<<iter->get_str()<<endl;
    }

    file.close();
}
