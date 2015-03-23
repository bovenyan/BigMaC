#ifndef BUCKET_TREE
#define BUCKET_TREE

#include "../stdafx.h"
#include "Address.hpp"
#include "Rule.hpp"
#include "RuleList.hpp"
#include "Bucket.hpp"
#include <cmath>
#include <set>
#include <deque>

class bucket_tree {
    private:
    bucket * root;
    rule_list * rList;
    
    void delNode(bucket *);
    vector<vector<int> > candi_cuts;

    void gen_candi_cuts(int sum);

    public:
    // constructor
    bucket_tree();
    bucket_tree(rule_list * rL, int level, int cut_no);
    ~bucket_tree();

    // build tree
    void make_tree(bucket * ptr, int level);

    // search   
    std::pair<bucket *, int> search_bucket(const addr_5tup & packet, bucket* buck) const; // returns the bucket and match switching rule

    // debug
    void print_bucket(std::ofstream &, bucket *, bool); // const
    void print_tree(const string &, bool = false); // const

    // unit tests
    void search_test(const string &) ;
};

#endif


