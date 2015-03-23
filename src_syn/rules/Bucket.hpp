/*
 * FileName: Bucket.hpp
 * Contributer: Bo Yan (NYU)
 * Description:
 *      Structure for Bucket Tree Node
 */
#ifndef BUCKET_H
#define BUCKET_H

#include "../stdafx.h"
#include "Address.hpp"
#include "Rule.hpp"
#include "RuleList.hpp"
#include <set>

using std::set;
/* bucket: Inherited Structure from b_rule for Bucket Tree node
 *  sonList: list of son nodes
 *  related_rules: related rules to this node
 *  curArr[4]: the no. of cut on each dim e.g. [2,3,0,0] means 2 cuts on dim 0, 3 cuts on dim 1
 */
class bucket: public b_rule {
  private:
    set<int> related_rules; // sRule overlapped with bucket
    vector<int> cutBits; // [2,2,2] cut three times at dstIP

  public:
    std::vector<bucket*> sonList;   
    
    // constructor
    bucket();
    bucket(const bucket &); // cpy
    bucket(const rule_list * rL, bool all);

    // access
    int rela_size() const;
    bool is_rela(const int &) const;
    bucket * gotoSon(addr_5tup packet);  
    int search_rela(addr_5tup packet, rule_list * rList);

    // node modify
    bool splittable(const vector<int> & cBits);
    pair<double, int> split(const vector<int> & cBits, rule_list * rList); // !! Need to check feasibility before using;
    void cleanSon();
    void delSubTree(); // Mar 20
    void clearNonRela(rule_list * rList);

    // debug
    string get_str(bool detail) const;
};


// implementation

bucket::bucket(){}

bucket::bucket(const bucket & bk) : b_rule(bk) {
    sonList = vector<bucket*>();
    related_rules = bk.related_rules;
}

bucket::bucket(const rule_list * rL, bool all = false) {
    for (size_t idx = 0; idx != rL->sRule_size(); ++idx)
        if (all || assoc_prule(rL->sRuleAt(idx)))
            related_rules.insert(idx);
}

int bucket::rela_size() const{
    return related_rules.size();
}

bool bucket::is_rela(const int & checkID) const{
    return (related_rules.find(checkID) != related_rules.end());
}

bucket * bucket::gotoSon(addr_5tup packet){
    uint32_t masks[4];
    for (int i = 0; i < 4; ++i)
        masks[i] = addrs[i].mask; 

    int idx = 0;
    for (auto iter = cutBits.begin(); iter != cutBits.end(); ++iter){
        uint32_t mask = masks[*iter] ^ ((1<<31)+(masks[*iter]>>1)); // 000..00100..000
        if ((packet.addrs[*iter] & mask) != 0)
            idx = (idx<<1) + 1;
        else
            idx = (idx<<1);

        masks[*iter] = (masks[*iter]>>1) + (1<<31);
    }
    
    return sonList[idx];
}

int bucket::search_rela(addr_5tup packet, rule_list * rList){
    for (auto iter_rela = related_rules.begin(); iter_rela != related_rules.end();
            ++iter_rela){
        if (rList->sRuleAt(*iter_rela).packet_hit(packet)){
            return *iter_rela;
        }
    }
    return -1;
}

bool bucket::splittable(const vector<int> & cBits){
    uint32_t masks[4];
    for (int i = 0; i < 4; ++i)
        masks[i] = addrs[i].mask;  

    for (int bit_idx = 0; bit_idx < cBits.size() - 1; ++bit_idx){
        if (masks[bit_idx] == (~0))
            return false;
        masks[bit_idx] = (masks[bit_idx]) + (1 << 31);
    }
    return true;
}

pair<double, int> bucket::split(const vector<int> & cBits, rule_list *rList) {
    if (!sonList.empty()){ // clear son
        for (auto iter = sonList.begin(); iter != sonList.end(); ++iter)
            delete *iter;

        sonList.clear();
    }

    int sonList_size = 1;
    for (int i = 0; i < cBits.size(); ++i)
        sonList_size *= 2;

    double cost = 0;
    int max_size = 0;

    for (int son_idx = 0; son_idx < sonList_size; ++son_idx){
        bucket * curBucket = new bucket(*this);
        
        // cal prefix mask for each bit: MSB -> LSB
        for (int bit_idx = 0; bit_idx < cBits.size() - 1; ++bit_idx){
            uint32_t & mask = curBucket->addrs[cBits[bit_idx]].mask;
            mask = (mask >> 1) + (1 << 31);

            uint32_t & pref = curBucket->addrs[cBits[bit_idx]].pref;
            uint32_t incre = (~mask) + 1;

            if ((son_idx & (1 << bit_idx)) > 0) // sonList msb/lsb reverse
                pref+=incre;
        }

        // delete non-related rule;
        curBucket->clearNonRela(rList);
        sonList.push_back(curBucket);

        cost += curBucket->rela_size();
        max_size = std::max(max_size, curBucket->rela_size());
    }

    return std::make_pair(cost/sonList_size, max_size);
}

void bucket::cleanSon(){
    for (auto iter = sonList.begin(); iter != sonList.end(); ++iter)
        delete *iter;
}

void bucket::delSubTree(){
    for (auto iter = sonList.begin(); iter != sonList.end(); ++iter)
        (*iter)->delSubTree();
    delete this;
}

void bucket::clearNonRela(rule_list * rList){
    for (auto iter_rule_id = related_rules.begin(); 
              iter_rule_id != related_rules.end();){
        if (!assoc_prule(rList->sRuleAt(*iter_rule_id)))
            iter_rule_id = related_rules.erase(iter_rule_id);
        else
            ++iter_rule_id;
    }
}

string bucket::get_str(bool detail) const {
    stringstream ss;
    ss << b_rule::get_str() << "\t" << related_rules.size();
    if (detail){
        ss << "\n\t" << "related_rules: ";
        for (auto iter = related_rules.begin(); iter != related_rules.end(); ++iter)
            ss<<*iter<<" ";
    }
    return ss.str();
}

#endif
