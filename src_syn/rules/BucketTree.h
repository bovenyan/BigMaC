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
    
    private:
    // node modificaiton
    void splitNode_fix(bucket * = NULL);
    void delNode(bucket *);

    public:
    // constructor
    bucket_tree();
    bucket_tree(rule_list &, uint32_t);
    ~bucket_tree();

    // search 
    std::pair<bucket *, int> search_bucket(const addr_5tup &, bucket* ) const;
    bucket * search_bucket_seri(const addr_5tup &, bucket* ) const;


    // debug
    void print_bucket(std::ofstream &, bucket *, bool); // const
    void print_tree(const string &, bool = false); // const

  public:
    // unit tests
    void search_test(const string &) ;
};

#endif


