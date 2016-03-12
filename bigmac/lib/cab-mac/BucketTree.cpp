#include "BucketTree.h"

// typedef vector<uint32_t>::iterator Iter_id;
// typedef vector<bucket*>::iterator Iter_son;

namespace fs = boost::filesystem;
namespace io = boost::iostreams;

using std::set;
using std::list;
using std::ifstream;
using std::ofstream;
using std::make_pair;

// ---------- bucket_tree ------------
bucket_tree::bucket_tree() {
    root = NULL;
    thres_soft = 0;
    tree_depth = 0;
}

bucket_tree::bucket_tree(pipe_line & p_line, uint32_t thr,
                         bool test_bed, size_t pa_no ) {
    thres_hard = thr;
    thres_soft = thr*2;
    p_line_ptr = & p_line;

    root = new bucket(); // full address space

    for (uint32_t i = 0; i < p_line.fwd_table.size(); i++)
        root->assoc_fwd_rules.push_back(i);
    for (uint32_t i = 0; i < p_line.mgmt_table.size(); i++)
        root->assoc_mgmt_rules.push_back(i);

    gen_candi_split(test_bed);

    split_node_static(root);
    pa_rule_no = pa_no;
    tree_depth = 0;
}

bucket_tree::~bucket_tree() {
    del_subtree(root);
}

pair<bucket *, pair<size_t, size_t> > bucket_tree::search_bucket(const addr_5tup& packet,
        bucket * buck_ptr) const {

    if (!buck_ptr->son_list.empty()) {
        size_t idx = 0;

        for (int i = 3; i >= 0; --i) {
            if (buck_ptr->cut_arr[i] != 0) {
                idx = (idx << buck_ptr->cut_arr[i]);
                size_t offset = (packet.addrs[i] - buck_ptr->addrs[i].pref);

                offset = offset/((~(buck_ptr->addrs[i].mask) >>
                                  buck_ptr->cut_arr[i]) + 1);
                idx += offset;
            }
        }

        assert (idx < buck_ptr->son_list.size());

        return search_bucket(packet, buck_ptr->son_list[idx]);

    } else { // found matched bucket
        size_t matched_fwd = -1;
        size_t matched_mgmt = -1;

        buck_ptr->hit = true;

        for (auto iter = buck_ptr->assoc_fwd_rules.begin();
                iter != buck_ptr->assoc_fwd_rules.end(); ++iter) {
            p_line_ptr->fwd_table[*iter].is_cached = true;
            if (p_line_ptr->fwd_table[*iter].packet_hit(packet)) {
                p_line_ptr->fwd_table[*iter].hit = true;
                matched_fwd = *iter;
                break;
            }
        }

        for (auto iter = buck_ptr->assoc_mgmt_rules.begin();
                iter != buck_ptr->assoc_mgmt_rules.end(); ++iter) {
            p_line_ptr->mgmt_table[*iter].is_cached = true;
            if (p_line_ptr->mgmt_table[*iter].packet_hit(packet)) {
                p_line_ptr->mgmt_table[*iter].hit = true;
                matched_mgmt = *iter;
                break;
            }
        }

        return make_pair(buck_ptr, make_pair(matched_fwd, matched_mgmt));
    }
}

bucket * bucket_tree::search_bucket_linear(const addr_5tup& packet, bucket * buck) const {
    if (!buck->son_list.empty()) {
        for (auto iter = buck->son_list.begin(); iter != buck->son_list.end(); ++iter)
            if ((*iter)->packet_hit(packet))
                return search_bucket_linear(packet, *iter);
        return NULL;
    } else {
        return buck;
    }
}

void bucket_tree::pre_alloc() {
    vector<uint32_t> fwd_assoc_buck_count(p_line_ptr->fwd_table.size(), 0);
    vector<uint32_t> mgmt_assoc_buck_count(p_line_ptr->mgmt_table.size(), 0);

    cal_assoc_buckets(root, fwd_assoc_buck_count, mgmt_assoc_buck_count);

    for (uint32_t i = 0; i< pa_rule_no; i++) {
        uint32_t max_assoc_count = 0;
        bool fwd_or_mgmt = true;
        uint32_t idx;

        for (auto iter = fwd_assoc_buck_count.begin();
                iter != fwd_assoc_buck_count.end(); ++iter) {
            if (*iter > max_assoc_count) {
                max_assoc_count = *iter;
                idx = i;
            }
        }

        for (auto iter = mgmt_assoc_buck_count.begin();
                iter != mgmt_assoc_buck_count.end(); ++iter) {
            if (*iter > max_assoc_count) {
                max_assoc_count = *iter;
                idx = i;
                fwd_or_mgmt = false;
            }
        }

        if (fwd_or_mgmt) {
            fwd_assoc_buck_count[idx] = 0;
            pa_rules.first.insert(idx);
        } else {
            mgmt_assoc_buck_count[idx] = 0;
            pa_rules.second.insert(idx);
        }
    }

    prune_tree_pre_alloc(root);
}

void bucket_tree::dyn_adjust() {
    merge_bucket(root);
    // print_tree("../para_src/tree_merge.dat");
    repart_bucket();
    p_line_ptr->clear_rule_stats();
}

void bucket_tree::cal_tree_depth(bucket * ptr, int count) {
    for (auto iter = ptr->son_list.begin(); iter != ptr->son_list.end(); iter++) {
        cal_tree_depth(*iter, count+1);
    }
    if (count > tree_depth)
        tree_depth = count;
}

void bucket_tree::gen_candi_split(bool test_bed, size_t cut_no) {
    if (test_bed) {
        vector<size_t> base(4,0);
        for (size_t i = 0; i <= cut_no; ++i) {
            base[0] = i;
            base[1] = cut_no - i;
            candi_split.push_back(base);
        }
    } else {
        if (cut_no == 0) {
            vector<size_t> base(4,0);
            candi_split.push_back(base);
        } else {
            gen_candi_split(test_bed, cut_no-1);
            vector< vector<size_t> > new_candi_split;
            if (cut_no > 1)
                new_candi_split = candi_split;

            for (auto iter = candi_split.begin(); iter != candi_split.end(); ++iter) {
                for (size_t i = 0; i < 4; ++i) {
                    vector<size_t> base = *iter;
                    ++base[i];
                    new_candi_split.push_back(base);
                }
            }
            candi_split = new_candi_split;
        }
    }
}

void bucket_tree::split_node_static(bucket * ptr) {
    size_t cost = ptr->assoc_fwd_rules.size() + ptr->assoc_mgmt_rules.size();
    if (cost < thres_soft)
        return;

    pair<double, size_t> opt_cost = std::make_pair(double(cost), cost);
    vector<size_t> opt_cut;

    for (auto iter = candi_split.begin(); iter != candi_split.end(); ++iter) {
        auto cost = ptr->split(*iter, p_line_ptr);

        if (cost.first < 0)
            continue;

        if (cost.first < opt_cost.first ||
                ((cost.first == opt_cost.first) &&
                 (cost.second < opt_cost.second))) {
            opt_cut = *iter;
            opt_cost = cost;
        }
    }

    if (opt_cut.empty()) {
        ptr->delete_subtree();
        return;
    } else {
        ptr->split(opt_cut, p_line_ptr);
        for (size_t i = 0; i < 4; ++i)
            ptr->cut_arr[i] = opt_cut[i];

        for (auto iter = ptr->son_list.begin(); iter != ptr->son_list.end(); ++iter)
            split_node_static(*iter);
    }
}


void bucket_tree::cal_assoc_buckets(bucket * bk, vector<uint32_t> & fwd_assoc_buck_count,
                                    vector<uint32_t> & mgmt_assoc_buck_count) const {

    if (bk->son_list.empty()) {
        for (auto iter = bk->assoc_fwd_rules.begin();
                iter != bk->assoc_fwd_rules.end(); iter++) {
            ++fwd_assoc_buck_count[*iter];
        }
        for (auto iter = bk->assoc_mgmt_rules.begin();
                iter != bk->assoc_mgmt_rules.end(); ++iter) {
            ++mgmt_assoc_buck_count[*iter];
        }
    } else {
        for (auto iter_s = bk->son_list.begin(); iter_s != bk->son_list.end(); iter_s ++) {
            cal_assoc_buckets(*iter_s, fwd_assoc_buck_count, mgmt_assoc_buck_count);
        }
    }

    return;
}

void bucket_tree::prune_tree_pre_alloc (bucket * buck_ptr) {
    // delete the pre-allocated rules
    for (auto iter = buck_ptr->assoc_fwd_rules.begin();
            iter != buck_ptr->assoc_fwd_rules.end(); ) {
        if (pa_rules.first.find(*iter) != pa_rules.first.end())
            iter = buck_ptr->assoc_fwd_rules.erase(iter);
        else
            ++iter;
    }

    for (auto iter = buck_ptr->assoc_mgmt_rules.begin();
            iter != buck_ptr->assoc_mgmt_rules.end();) {
        if (pa_rules.second.find(*iter) != pa_rules.second.end())
            iter = buck_ptr->assoc_fwd_rules.erase(iter);
        else
            ++iter;
    }

    // subtree rooted at buck_ptr can be pruned
    if (buck_ptr->get_assoc_no() < thres_hard) {
        for (auto iter_s = buck_ptr->son_list.begin();
                iter_s != buck_ptr->son_list.end(); iter_s++) {
            del_subtree(*iter_s);
        }
        buck_ptr->son_list.clear();
        return;
    }

    // go deeper
    for (auto iter_s = buck_ptr->son_list.begin();
            iter_s != buck_ptr->son_list.end(); ++iter_s) {
        prune_tree_pre_alloc(*iter_s);
    }
    return;
}

void bucket_tree::del_subtree(bucket * ptr) {
    for (auto iter = ptr->son_list.begin(); iter!= ptr->son_list.end(); iter++) {
        del_subtree(*iter);
    }
    delete ptr;
}

void bucket_tree::check_static_hit(const b_rule & traf_block, bucket* buck_ptr,
                                   pair<set<size_t>, set<size_t> > & cached_rules,
                                   size_t & buck_count) {
    if (buck_ptr->son_list.empty()) { // bucket
        bool this_buck_hit = false;
        // a bucket is hit only when at least one rule is hit
        for (auto iter = buck_ptr->assoc_fwd_rules.begin();
                iter != buck_ptr->assoc_fwd_rules.end() && !this_buck_hit; ++iter) {
            if (traf_block.match_rule(p_line_ptr->fwd_table[*iter])) {
                this_buck_hit = true;
            }
        }

        for (auto iter = buck_ptr->assoc_mgmt_rules.begin();
                iter != buck_ptr->assoc_mgmt_rules.end() && !this_buck_hit; ++iter) {
            if (traf_block.match_rule(p_line_ptr->mgmt_table[*iter])) {
                this_buck_hit = true;
            }
        }

        if (this_buck_hit) { // this bucket is hit
            for (auto iter = buck_ptr->assoc_fwd_rules.begin();
                    iter != buck_ptr->assoc_fwd_rules.end(); ++iter) {
                cached_rules.first.insert(*iter);

                if (traf_block.match_rule(p_line_ptr->fwd_table[*iter])) {
                    p_line_ptr->fwd_table[*iter].hit = true;
                }
            }

            ++buck_count;
            buck_ptr->hit = true; // only matching at least one rule is considered a bucket hit
        }
    } else { // recursively find rest
        for (auto iter = buck_ptr->son_list.begin();
                iter != buck_ptr->son_list.end(); ++iter) {

            if ((*iter)->overlap(traf_block))
                check_static_hit(traf_block, *iter, cached_rules, buck_count);
        }
    }
}


// dynamic related
void bucket_tree::merge_bucket(bucket * ptr) { // merge using back order search
    if (!ptr->son_list.empty()) {
        for (auto iter = ptr->son_list.begin(); iter!= ptr->son_list.end(); ++iter) {
            merge_bucket(*iter);
        }
    } else
        return;

    bool at_least_one_hit = false;

    // merge if gain is large
    for (auto iter = ptr->son_list.begin(); iter != ptr->son_list.end(); ++iter) {
        if ((*iter)->hit)
            at_least_one_hit = true;
        else {
            for (auto iter_r = (*iter)->assoc_fwd_rules.begin();
                    iter_r != (*iter)->assoc_fwd_rules.end();
                    ++iter_r) {
                if (!p_line_ptr->fwd_table[*iter_r].is_cached)
                    return;
            }

            for (auto iter_r = (*iter)->assoc_mgmt_rules.begin();
                    iter_r != (*iter)->assoc_mgmt_rules.end();
                    ++iter_r) {
                if (!p_line_ptr->mgmt_table[*iter_r].is_cached)
                    return;
            }
            //if (!(*iter)->related_rules.empty())
            //    return;
        }
    }

    // don't merge if non hit
    if (!at_least_one_hit)
        return;

    for (auto iter = ptr->son_list.begin(); iter != ptr->son_list.end(); ++iter)
        delete *iter;

    ptr->son_list.clear();
    ptr->hit = true;
}

void bucket_tree::rec_occupancy(bucket * ptr, list<bucket *> & hit_buckets) {
    if (ptr->son_list.empty() && ptr->hit) {
        ptr->hit = false; // clear the hit flag
        ptr->repart_level = 0;
        hit_buckets.push_back(ptr);

        for (auto iter = ptr->assoc_fwd_rules.begin();
                iter != ptr->assoc_fwd_rules.end(); ++iter) {
            p_line_ptr->fwd_table[*iter].inc_occupancy();
        }
        for (auto iter = ptr->assoc_mgmt_rules.begin();
                iter != ptr->assoc_mgmt_rules.end(); ++iter) {
            p_line_ptr->mgmt_table[*iter].inc_occupancy();
        }
    }

    for (auto iter = ptr->son_list.begin(); iter != ptr->son_list.end(); ++iter)
        rec_occupancy(*iter, hit_buckets);
}

/*
void bucket_tree::merge_bucket_CPLX_test(bucket * ptr) { // merge using back order search
    if (!ptr->son_list.empty()) {
        for (auto iter = ptr->son_list.begin(); iter!= ptr->son_list.end(); ++iter) {
            merge_bucket_CPLX_test(*iter);
        }
    } else
        return;
    //
    if (ptr->related_rules.size() >= thres_soft*2)
        return;

    bool at_least_one_hit = false;

    for (auto iter = ptr->son_list.begin(); iter != ptr->son_list.end(); ++iter) {  // don't merge if all empty
        if ((*iter)->hit)
            at_least_one_hit = true;
        else {
            if (!(*iter)->related_rules.empty())
                return;
        }
    }

    if (!at_least_one_hit)
        return;

    for (auto iter = ptr->son_list.begin(); iter != ptr->son_list.end(); ++iter) // remove the sons.
        delete *iter;
    ptr->son_list.clear();
    ptr->hit = true;
}

void bucket_tree::regi_occupancy(bucket * ptr, deque <bucket *>  & hitBucks) {
    if (ptr->son_list.empty() && ptr->hit) {
        ptr->hit = false;  // clear the hit flag
        hitBucks.push_back(ptr);
        for (auto iter = ptr->related_rules.begin(); iter != ptr->related_rules.end(); ++iter) {
            ++rList->occupancy[*iter];
        }
    }
    for (auto iter = ptr->son_list.begin(); iter != ptr->son_list.end(); ++iter)
        regi_occupancy(*iter, hitBucks);
}*/


void bucket_tree::repart_bucket() {
    list<bucket *> proc_line;
    rec_occupancy(root, proc_line);

    size_t suc_counter = 0;
    auto proc_iter = proc_line.begin();

    while (!proc_line.empty()) {
        while(true) {
            if (suc_counter == proc_line.size())
                return;

            if (proc_iter == proc_line.end())   // cycle
                proc_iter = proc_line.begin();

            bool found = false;

            for (auto rule_iter = (*proc_iter)->assoc_fwd_rules.begin();
                    !found && rule_iter != (*proc_iter)->assoc_fwd_rules.end();
                    ++rule_iter) {
                if (p_line_ptr->fwd_table[*rule_iter].get_occupancy() == 1) {
                    found = true;
                }
            }

            for (auto rule_iter = (*proc_iter)->assoc_mgmt_rules.begin();
                    !found && rule_iter != (*proc_iter)->assoc_mgmt_rules.end();
                    ++rule_iter) {
                if (p_line_ptr->mgmt_table[*rule_iter].get_occupancy() == 1) {
                    found = true;
                }
            }

            if (found)
                break;
            else {
                ++proc_iter;
                ++suc_counter; // suc_counter;
            }

        }

        bucket* to_proc_bucket = *proc_iter;

        vector<size_t> opt_cut;
        int opt_gain = -1; // totally greedy: no gain don't partition

        for (auto iter = candi_split.begin(); iter != candi_split.end(); ++iter) {
            int gain = to_proc_bucket->reSplit(*iter, p_line_ptr);
            if (gain > opt_gain) {
                opt_gain = gain;
                opt_cut = *iter;
            }
        }

        if (opt_cut.empty()) {
            to_proc_bucket->delete_subtree();
            ++proc_iter; // keep the bucket
            ++suc_counter;
        } else {
            //BOOST_LOG(bTree_log) << "success";
            proc_iter = proc_line.erase(proc_iter); // delete the bucket
            suc_counter = 0;
            to_proc_bucket->reSplit(opt_cut, p_line_ptr, true);

            for (size_t i = 0; i < 4; ++i)
                to_proc_bucket->cut_arr[i] = opt_cut[i];

            for (auto iter = to_proc_bucket->son_list.begin(); // push son
                    iter != to_proc_bucket->son_list.end(); // immediate proc
                    ++iter) {
                bool son_hit = false;

                for(auto r_iter = (*iter)->assoc_fwd_rules.begin();
                        !son_hit && r_iter != (*iter)->assoc_fwd_rules.end(); ++r_iter) {
                    if (p_line_ptr->fwd_table[*r_iter].hit)
                        son_hit = true;
                }

                for(auto r_iter = (*iter)->assoc_mgmt_rules.begin();
                        !son_hit && r_iter != (*iter)->assoc_mgmt_rules.end(); ++r_iter) {
                    if (p_line_ptr->mgmt_table[*r_iter].hit)
                        son_hit = true;
                }

                if (son_hit) {
                    proc_iter = proc_line.insert(proc_iter, *iter);
                }
            }
        }
    }
}

void bucket_tree::print_bucket(ofstream & in, bucket * bk, bool detail) { // const
    if (bk->son_list.empty()) {
        in << bk->get_str() << endl;
        if (detail) {
            in << "re (fwd): ";
            for (auto iter = bk->assoc_fwd_rules.begin();
                    iter != bk->assoc_fwd_rules.end();
                    iter++) {
                in << *iter << " ";
            }
            in << endl;

            in << "re (mgmt): ";
            for (auto iter = bk->assoc_mgmt_rules.begin();
                    iter != bk->assoc_mgmt_rules.end();
                    ++iter) {
                in << *iter << " ";
            }
            in << endl;
        }

    } else {
        for (auto iter = bk->son_list.begin(); iter != bk->son_list.end(); iter++)
            print_bucket(in, *iter, detail);
    }
    return;
}

/*
void bucket_tree::repart_bucket_CPLX_test(int level) {
    // deque<bucket *> proc_line;  // Apr.25 updated
    list<bucket *> proc_line;
    rec_occupancy(root, proc_line);

    size_t suc_counter = 0;
    auto proc_iter = proc_line.begin();

    while (!proc_line.empty()) {
        while(true) {
            if (suc_counter == proc_line.size())
                return;

            if (proc_iter == proc_line.end())   // cycle
                proc_iter = proc_line.begin();

            bool found = false;
            for (auto rule_iter = (*proc_iter)->related_rules.begin();
                    rule_iter != (*proc_iter)->related_rules.end();
                    ++rule_iter) {
                if (rList->occupancy[*rule_iter] == 1) {
                    found = true;
                    break;
                }
            }

            if (found)
                break;
            else {
                ++proc_iter;
                ++suc_counter; // suc_counter;
            }

        }

        bucket* to_proc_bucket = *proc_iter;

        // Junan: check depth to limit maximum split
        if ( (to_proc_bucket->repart_level >= level) &&
                (to_proc_bucket->related_rules.size() < thres_hard) ) {
            proc_iter = proc_line.erase(proc_iter); // delete the bucket
            suc_counter = 0;
            continue;
        }

        vector<size_t> opt_cut;
        int opt_gain = -1; // totally greedy: no gain don't partition

        for (auto iter = candi_split.begin(); iter != candi_split.end(); ++iter) {
            int gain = to_proc_bucket->reSplit(*iter, rList);
            if (gain > opt_gain) {
                opt_gain = gain;
                opt_cut = *iter;
            }
        }
        // Junan: force to cut
        size_t cut[4] = {1,1,0,0};
        for (size_t i = 0; i < 4; i++)
            opt_cut[i] = cut[i];

        if (opt_cut.empty()) {
            to_proc_bucket->cleanson();
            ++proc_iter; // keep the bucket
            ++suc_counter;
        } else {
            //BOOST_LOG(bTree_log) << "success";
            proc_iter = proc_line.erase(proc_iter); // delete the bucket
            suc_counter = 0;
            to_proc_bucket->reSplit(opt_cut, rList, true);

            for (size_t i = 0; i < 4; ++i)
                to_proc_bucket->cutArr[i] = opt_cut[i];

            for (auto iter = to_proc_bucket->son_list.begin(); // push son
                    iter != to_proc_bucket->son_list.end(); // immediate proc
                    ++iter) {
                // Junan: record repart levels to limit repartition
                (*iter)->repart_level = to_proc_bucket->repart_level + 1;

                bool son_hit = false;
                for(auto r_iter = (*iter)->related_rules.begin();
                        r_iter != (*iter)->related_rules.end(); ++r_iter) {
                    if (rList->list[*r_iter].hit) {
                        son_hit = true;
                        break;
                    }
                }
                // Junan: if son bucket contain rules then add to proc_line
                if (!(*iter)->related_rules.empty())
                    son_hit = true;

                if (son_hit) {
                    // Junan: didn't increase occupancy in reSplit(). so do it here

                    for (auto iter_id = (*iter)->related_rules.begin();
                            iter_id != (*iter)->related_rules.end(); ++iter_id) {
                        ++rList->occupancy[*iter_id];
                    }
                    proc_iter = proc_line.insert(proc_iter, *iter);
                }
            }
        }
    }
}
*/


/* TEST USE Functions
 *
 */

void bucket_tree::search_test(const string & tracefile_str) {
    io::filtering_istream in;
    in.push(io::gzip_decompressor());
    ifstream infile(tracefile_str);
    in.push(infile);

    string str;
    cout << "Start search testing ... "<< endl;
    size_t cold_packet = 0;
    size_t hot_packet = 0;

    while (getline(in, str)) {
        addr_5tup packet(str, false);
        auto result = search_bucket(packet, root);

        if (result.first->assoc_fwd_rules.size() < 10 &&
                result.first->assoc_mgmt_rules.size() < 10) {
            ++cold_packet;
        } else {
            ++hot_packet;
        }

        if (result.first != (search_bucket_linear(packet, root))) {
            BOOST_LOG(bTree_log) << "Within bucket error: packet: " << str;
            BOOST_LOG(bTree_log) << "search_buck   res : " << result.first->get_str();
            BOOST_LOG(bTree_log) << "search_buck_s res : " << result.first->get_str();
        }

        auto linear_search_rules = p_line_ptr->get_matched_rules(packet);
        if (result.second.first != linear_search_rules.first) {
            if (pa_rules.first.find(linear_search_rules.first) ==
                    pa_rules.first.end()) {
                BOOST_LOG(bTree_log) << "Search rule error: packet:" << str;
                if (result.second.first > 0)
                    BOOST_LOG(bTree_log) << "search_buck res (fwd): " <<
                                         p_line_ptr->fwd_table[result.second.first].get_str();
                else
                    BOOST_LOG(bTree_log) << "search_buck res (fwd): " << "None";

                BOOST_LOG(bTree_log) << "linear_search res (fwd): " <<
                                     p_line_ptr->fwd_table[linear_search_rules.first].get_str();
            }
        }

        if (result.second.second != linear_search_rules.second) {
            if (pa_rules.second.find(linear_search_rules.second) ==
                    pa_rules.second.end()) {
                BOOST_LOG(bTree_log) << "Search rule error: packet:" << str;
                if (result.second.second > 0)
                    BOOST_LOG(bTree_log) << "search_buck res (mgmt): " <<
                                         p_line_ptr->fwd_table[result.second.second].get_str();
                else
                    BOOST_LOG(bTree_log) << "search_buck res (mgmt): " << "None";

                BOOST_LOG(bTree_log) << "linear_search res (mgmt): " <<
                                     p_line_ptr->fwd_table[linear_search_rules.second].get_str();
            }
        }
    }

    BOOST_LOG(bTree_log) << "hot packets: "<< hot_packet;
    BOOST_LOG(bTree_log) << "cold packets: "<< cold_packet;
    cout << "Search testing finished ... " << endl;
}

void bucket_tree::static_traf_test(const string & file_str) {
    ifstream file(file_str);
    size_t counter = 0;
    pair<set<size_t>, set<size_t> > cached_rules;
    size_t buck_count = 0;

    debug = false;
    for (string str; getline(file, str); ++counter) {
        vector<string> temp;
        boost::split(temp, str, boost::is_any_of("\t"));
        size_t r_exp = boost::lexical_cast<size_t>(temp.back());
        if (r_exp > 40) {
            --counter;
            continue;
        }

        b_rule traf_blk(str);
        check_static_hit(traf_blk, root, cached_rules, buck_count);
        if (counter > 80)
            break;
    }
    cout << "Cached: " << cached_rules.first.size() << " fwding rules, ";
    cout << cached_rules.second.size() << "mgmt rules, ";
    cout << buck_count << " buckets " <<endl;

    dyn_adjust();
    print_tree("../para_src/tree_split.dat");

    buck_count = 0;
    p_line_ptr->clear_rule_stats();
    cached_rules.first.clear();
    cached_rules.second.clear();

    counter = 0;
    file.seekg(std::ios::beg);

    for (string str; getline(file, str); ++counter) {
        vector<string> temp;
        boost::split(temp, str, boost::is_any_of("\t"));
        size_t r_exp = boost::lexical_cast<size_t>(temp.back());
        if (r_exp > 40) {
            --counter;
            continue;
        }

        b_rule traf_blk(str);
        check_static_hit(traf_blk, root, cached_rules, buck_count);
        if (counter > 80)
            break;
    }


    list<bucket *> proc_line;
    rec_occupancy(root, proc_line);

    size_t unused_count = 0;
    stringstream ss;

    for (auto iter = cached_rules.first.begin(); iter != cached_rules.first.end(); ++iter) {
        if (!p_line_ptr->fwd_table[*iter].hit) {
            ++unused_count;
            ss<<*iter << "("<< p_line_ptr->fwd_table[*iter].get_occupancy()<<") ";
        }
    }

    for (auto iter = cached_rules.second.begin(); iter != cached_rules.second.end(); ++iter) {
        if (!p_line_ptr->mgmt_table[*iter].hit) {
            ++unused_count;
            ss<<*iter << "("<< p_line_ptr->mgmt_table[*iter].get_occupancy()<<") ";
        }
    }

    BOOST_LOG(bTree_log)<< "Unused rules: " << ss.str()
        << "Cached: " << cached_rules.first.size() << " fwding rules ,"
        << cached_rules.second.size() << "mgmt rules (" << unused_count << ") "
        << buck_count << " buckets " << endl;

}

void bucket_tree::evolving_traf_test_dyn(const vector<b_rule> & prev,
        const vector<b_rule> & after, ofstream & rec_file,
        double threshold, pair<size_t, size_t> & last_overhead,
        size_t & adj_time) {
    vector <b_rule> current = prev;
    bool to_adjust = true;
    for (size_t counter = 0; counter < prev.size(); ++counter) {
        pair<set<size_t>, set<size_t> > cached_rules;
        size_t buck_count = 0;

        if (to_adjust) { // dyn_adj
            for (auto iter = current.begin(); iter != current.end(); ++iter) {
                check_static_hit(*iter, root, cached_rules, buck_count);
            }
            dyn_adjust();
            cached_rules.first.clear();
            cached_rules.second.clear();
            buck_count = 0;
            ++adj_time;
        }

        for (auto iter = current.begin(); iter != current.end(); ++iter) {
            check_static_hit(*iter, root, cached_rules, buck_count);
        }

        size_t unused_count = 0;
        for (auto iter = cached_rules.first.begin(); 
                iter != cached_rules.first.end(); ++iter) {
            if (!p_line_ptr->fwd_table[*iter].hit) {
                ++unused_count;
            }
        }
        for (auto iter = cached_rules.second.begin(); 
                iter != cached_rules.second.end(); ++iter) {
            if (!p_line_ptr->mgmt_table[*iter].hit) {
                ++unused_count;
            }
        }

        BOOST_LOG(bTree_log) << "Dyn Cached: " << cached_rules.first.size()
            << " fwding rules , " << cached_rules.second.size() << " mgmt rules ("
            << unused_count << " unused) " << buck_count << " buckets ";

        rec_file << cached_rules.first.size() << "\t";
        rec_file << cached_rules.second.size() << "\t" << buck_count << "\t";
        rec_file << cached_rules.first.size() + cached_rules.second.size() + buck_count<<endl;

        if (to_adjust) {
            BOOST_LOG(bTree_log) << "Adjust here: " << counter;
            last_overhead.first = unused_count;
            last_overhead.second = buck_count;
            to_adjust = false;
        } else {
            if (unused_count < last_overhead.first)
                last_overhead.first = unused_count;
            else if (unused_count > threshold * last_overhead.first)
                to_adjust = true;
            if (buck_count < last_overhead.second)
                last_overhead.second = buck_count;
            else if (buck_count > threshold * last_overhead.second)
                to_adjust = true;
        }

        current[counter] = after[counter]; // evolve traffic
        root->clear_subtree_hit();
        p_line_ptr->clear_rule_stats();
    }
}

void bucket_tree::evolving_traf_test_stat(const vector<b_rule> & prev, const vector<b_rule> & after, ofstream & rec_file) {
    vector <b_rule> current = prev;
    for (size_t counter = 0; counter < prev.size(); ++counter) {
        pair<set<size_t>, set<size_t> > cached_rules;
        size_t buck_count = 0;

        for (auto iter = current.begin(); iter != current.end(); ++iter) {
            check_static_hit(*iter, root, cached_rules, buck_count);
        }

        size_t unused_count = 0;
        for (auto iter = cached_rules.first.begin();
                iter != cached_rules.first.end(); ++iter) {
            if (!p_line_ptr->fwd_table[*iter].hit) {
                ++unused_count;
            }
        }
        for (auto iter = cached_rules.second.begin();
                iter != cached_rules.second.end(); ++iter) {
            if (!p_line_ptr->mgmt_table[*iter].hit) {
                ++unused_count;
            }
        }

        rec_file << cached_rules.first.size() << "\t" << cached_rules.second.size();
        rec_file << "\t" << buck_count << "\t";
        rec_file << cached_rules.first.size() + cached_rules.second.size() + buck_count<<endl;
        BOOST_LOG(bTree_log) << "Stat Cached: " << cached_rules.first.size()
            << " fwding rules, " << cached_rules.second.size() << " mgmt rules ("
            << unused_count << " unused) " << buck_count << " buckets ";

        current[counter] = after[counter]; // evolve traffic
        root->clear_subtree_hit();
        p_line_ptr->clear_rule_stats();
    }

}

void bucket_tree::print_tree(const string & filename, bool det) { // const
    ofstream out(filename);
    print_bucket(out, root, det);
    out.close();
}






