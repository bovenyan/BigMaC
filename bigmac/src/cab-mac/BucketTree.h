#ifndef BUCKET_TREE
#define BUCKET_TREE

#include "stdafx.h"
#include "Address.hpp"
#include "Rule.hpp"
#include "PipeLine.h"
#include "Bucket.h"
#include <cmath>
#include <set>
#include <deque>
#include <list>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/filesystem.hpp>

class bucket_tree {
private:
    boost::log::sources::logger bTree_log;
public:
    bucket * root;
    pipe_line * p_line_ptr;

    uint32_t thres_soft;
    uint32_t thres_hard;

    uint32_t pa_rule_no;
    std::pair<std::set<uint32_t>, std::set<uint32_t> > pa_rules;

    int tree_depth;

    // for debug
    bool debug;

    // HyperCut related
    size_t max_cut_per_layer;
    double slow_prog_perc;
    std::vector<std::vector<size_t> > candi_split;

public:
    bucket_tree();
    bucket_tree(pipe_line & p_line, uint32_t threshold,
                bool test_bed = false, size_t pre_allocate_no = 0);

    ~bucket_tree();

    /* search buckets in tree and linear respectively
     * search_bucket returns the bucket's pointer and the matched rule id
     */
    std::pair<bucket *, pair<size_t, size_t> > search_bucket(
        const addr_5tup & packet,
        bucket * buck_ptr ) const;

    bucket * search_bucket_linear(const addr_5tup & packet, bucket* buck_ptr) const;

    void pre_alloc();
    void dyn_adjust();
    void cal_tree_depth(bucket *, int = 0);

private:
    // static related
    void gen_candi_split(bool, size_t = 2);

    void split_node_static(bucket * root_ptr = NULL);

    /* for pre allocation
     * cal_assoc_buckets calculates # of associated buckets for each rule
     * prune_tree_pre_alloc prunes the subtree of those small subtrees
     */
    void cal_assoc_buckets(bucket * buck,
                           std::vector<uint32_t> & fwd_assoc_buck_count,
                           std::vector<uint32_t> & mgmt_assoc_buck_count) const;
    void prune_tree_pre_alloc(bucket * buck_ptr);

    void del_subtree(bucket * ptr);

    /* check_static_hit
     * check whether a traffic block hits a bucket and associated rules or not
     */
    void check_static_hit(const b_rule & traffic_block, bucket * buck_ptr,
                          std::pair<std::set<size_t>,
                          std::set<size_t> > & cached_rules,
                          size_t & buck_count);
public:
    // dynamic related
    void merge_bucket(bucket * buck_ptr);
    void rec_occupancy(bucket * buck_ptr, std::list <bucket*> & hit_buckets);

    void repart_bucket();

    void print_bucket(std::ofstream &, bucket *, bool); // const

    // void merge_bucket_CPLX_test(bucket*);
    // void repart_bucket_CPLX_test(int);
    // void regi_occupancy(bucket*, std::deque <bucket*> &); // deprecated Apr. 24

public:
    // test use
    void search_test(const string &) ;
    void static_traf_test(const string &);

    void evolving_traf_test_dyn(const std::vector<b_rule> & prev,
                                const std::vector<b_rule> & after,
                                std::ofstream & rec_file,
                                double threshold,
                                pair<size_t, size_t> & last_overhead,
                                size_t & adj_time);

    void evolving_traf_test_stat(const std::vector<b_rule> & prev,
                                 const std::vector<b_rule> & after,
                                 std::ofstream & rec_file);
    void print_tree(const string &, bool = false); // const

};

#endif

