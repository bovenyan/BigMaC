#ifndef BUCKET
#define BUCKET

#include "stdafx.h"
#include "Address.hpp"
#include "Rule.hpp"
#include "PipeLine.h"
#include <set>

class bucket: public b_rule {
private:
    static boost::log::sources::logger lg;
    static void logger_init();

public:
    std::vector<bucket*> son_list; 		// List of son nodes
    std::vector<size_t> assoc_fwd_rules;	// IDs of associated fwding rules in the bucket
    std::vector<size_t> assoc_mgmt_rules;	// IDs of associated mgmt rules in the bucket
    uint32_t cut_arr[4];	// how does this node is cut.  e.g. [2,3,0,0] means 2 cuts on dim 0, 3 cuts on dim 1
    bool hit;
    bucket * parent;
    size_t max_gain;
    size_t repart_level;

public:
    bucket();
    bucket(const bucket & bk);
    bucket(const string & bk_str, const pipe_line * pLine);

    inline int get_assoc_no(){ return assoc_fwd_rules.size() + assoc_mgmt_rules.size(); }

    /* static split
     * dimension: split on certain fields
     * pLine: for calculate the associated rules
     */
    std::pair<double, size_t> split(const std::vector<size_t> & dimension,
                                    pipe_line * p_line);

    /* dyn split
     * dimension: split on certain fields
     * p_line_ptr: pipeline info to calculate assoc rules
     */
    int reSplit(const std::vector<size_t> & dimension,
                pipe_line * p_line_ptr, bool apply_or_not = false);

    // std::vector<size_t> unq_comp(pipe_line * p_line_ptr);

    void delete_subtree();

    /* clear hit flag of subtree
     * clear the hit flag of nested buckets recursively
     */
    void clear_subtree_hit();

    string get_str() const;
};

#endif

