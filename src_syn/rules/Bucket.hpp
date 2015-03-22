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

/* bucket: Inherited Structure from b_rule for Bucket Tree node
 *  sonList: list of son nodes
 *  related_rules: related rules to this node
 *  curArr[4]: the no. of cut on each dim e.g. [2,3,0,0] means 2 cuts on dim 0, 3 cuts on dim 1
 */
class bucket: public b_rule {
  private:
    std::vector<bucket*> sonList;   
    std::vector<uint32_t> related_rules;
    vector<int> cutBits; // [2,2,2] cut three times at dstIP

  public:
    // constructor
    bucket();
    bucket(const bucket &); // cpy
    bucket(const rule_list *);

    // access
    bucket * gotoSon(addr_5tup packet, int * ptrs); // Mar 20

    // node modify
    std::pair<double, size_t> split(vector<int> cBits, rule_list * rList);
    void deleteSubTree(); // Mar 20

    // debug
    string get_str() const;
};


// implementation
using std::list;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::set;

bucket::bucket(){}

bucket::bucket(const bucket & bk) : b_rule(bk) {
    sonList = vector<bucket*>();
    related_rules = vector<uint32_t>();
}

bucket::bucket(const rule_list * rL) {
    for (size_t idx = 0; idx != rL->sRule_size(); ++idx)
        if (assoc_prule(rL->sRuleAt(idx)))
            related_rules.push_back(idx);
}

bucket * bucket::gotoSon(addr_5tup packet, int * ptrs){
    int idx = 0;
    
    for (auto iter = cutBits.begin(); iter != cutBits.end(); ++iter){
        if (ptrs[*iter] < 0)
            return NULL;

        if ((1<<ptrs[*iter]) > 0)
            idx = (idx<<1) + 1);
        else
            idx = (idx<<1);
        
        --ptrs[*iter];
    }

    return sonList[idx];
}

pair<double, size_t> bucket::split(vector<int> cBits, rule_list *rList) {
    
    if (!sonList.empty()){ // clear son
        for (auto iter = sonList.begin(); iter != sonList.end(); ++iter){
            delte *iter;
        }
    }

    // Mar 21 job save

    uint32_t new_masks[4];
    size_t total_son_no = 1;

    for (size_t i = 0; i < 4; ++i) { // new mask
        new_masks[i] = addrs[i].mask;

        for (size_t j = 0; j < dim[i]; ++j) {
            if (~(new_masks[i]) == 0)
                return std::make_pair(-1, 0);

            new_masks[i] = (new_masks[i] >> 1) + (1 << 31);
            total_son_no *= 2;
        }
    }


    size_t total_rule_no = 0;
    size_t largest_rule_no = 0;

    for (size_t i = 0; i < total_son_no; ++i) {
        bucket * son_ptr = new bucket(*this);

        uint32_t id = i;
        for (size_t j = 0; j < 4; ++j) { // new pref
            son_ptr->addrs[j].mask = new_masks[j];
            size_t incre = (~(new_masks[j]) + 1);
            son_ptr->addrs[j].pref += (id % (1 << dim[j]))*incre;
            id = (id >> dim[j]);
        }

        for (Iter_id iter = related_rules.begin(); iter != related_rules.end(); ++iter) { // rela rule
            if (son_ptr->match_rule(rList->list[*iter]))
                son_ptr->related_rules.push_back(*iter);
        }

        total_rule_no += son_ptr->related_rules.size();
        largest_rule_no = std::max(largest_rule_no, son_ptr->related_rules.size());

        sonList.push_back(son_ptr);
    }
    return std::make_pair(double(total_rule_no)/total_son_no, largest_rule_no);
}


string bucket::get_str() const {
    stringstream ss;
    ss << b_rule::get_str() << "\t" << related_rules.size();
    return ss.str();
}

void bucket::clearHitFlag() {
    hit = false;
    for (auto iter = sonList.begin(); iter != sonList.end(); ++iter) {
        (*iter)->clearHitFlag();
    }
}

void bucket::cleanson() {
    for (auto iter = sonList.begin(); iter != sonList.end(); ++iter)
        delete (*iter);
    sonList.clear();
}

#endif

