#include "BucketTree.h"

typedef vector<uint32_t>::iterator Iter_id;
typedef vector<bucket*>::iterator Iter_son;


using std::set;
using std::deque;
using std::ifstream;
using std::ofstream;


void bucket_tree::splitNode_fix(bucket * ptr) {
    double cost = ptr->related_rules.size();

    pair<double, size_t> opt_cost = std::make_pair(ptr->related_rules.size(), ptr->related_rules.size());
    vector<size_t> opt_cut;

    
    auto cost = ptr->split(*iter, rList);
    
    if (cost.first < 0)
        continue;

        if (cost.first < opt_cost.first || ((cost.first == opt_cost.first) && (cost.second < opt_cost.second))) {
            opt_cut = *iter;
            opt_cost = cost;
        }
    }

    if (opt_cut.empty()) {
        ptr->cleanson();
        return;
    } else {
        ptr->split(opt_cut, rList);
        for (size_t i = 0; i < 4; ++i)
            ptr->cutArr[i] = opt_cut[i];

        for (auto iter = ptr->sonList.begin(); iter != ptr->sonList.end(); ++iter)
            splitNode_fix(*iter);
    }
}


// ---------- bucket_tree ------------
bucket_tree::bucket_tree() {
    root = NULL;
    thres_soft = 0;
    tree_depth = 0;
}

bucket_tree::bucket_tree(rule_list & rL, uint32_t thr, bool test_bed, size_t pa_no ) {
    thres_hard = thr;
    thres_soft = thr*2;
    rList = &rL;
    root = new bucket(); // full address space
    for (uint32_t i = 0; i < rL.list.size(); i++)
        root->related_rules.insert(root->related_rules.end(), i);

    gen_candi_split(test_bed);
    splitNode_fix(root);

    pa_rule_no = pa_no;
    tree_depth = 0;
}

bucket_tree::~bucket_tree() {
    delNode(root);
}

pair<bucket *, int> bucket_tree::search_bucket(const addr_5tup& packet, bucket * buck) const {
    if (!buck->sonList.empty()) {
        size_t idx = 0;
        for (int i = 3; i >= 0; --i) {
            if (buck->cutArr[i] != 0) {
                idx = (idx << buck->cutArr[i]);
                size_t offset = (packet.addrs[i] - buck->addrs[i].pref);
                offset = offset/((~(buck->addrs[i].mask) >> buck->cutArr[i]) + 1);
                idx += offset;
            }
        }
        assert (idx < buck->sonList.size());
        return search_bucket(packet, buck->sonList[idx]);
    } else {
        buck->hit = true;
        int rule_id = -1;

        for (auto iter = buck->related_rules.begin(); iter != buck->related_rules.end(); ++iter) {
            if (rList->list[*iter].packet_hit(packet)) {
                rList->list[*iter].hit = true;
                rule_id = *iter;
                break;
            }
        }
        return std::make_pair(buck, rule_id);
    }
}

bucket * bucket_tree::search_bucket_seri(const addr_5tup& packet, bucket * buck) const {
    if (buck->sonList.size() != 0) {
        for (auto iter = buck->sonList.begin(); iter != buck->sonList.end(); ++iter)
            if ((*iter)->packet_hit(packet))
                return search_bucket_seri(packet, *iter);
        return NULL;
    } else {
        return buck;
    }
}


void bucket_tree::delNode(bucket * ptr) {
    for (Iter_son iter = ptr->sonList.begin(); iter!= ptr->sonList.end(); iter++) {
        delNode(*iter);
    }
    delete ptr;
}

/*
 * debug
 */

void bucket_tree::print_bucket(ofstream & in, bucket * bk, bool detail) { // const
    if (bk->sonList.empty()) {
        in << bk->get_str() << endl;
        if (detail) {
            in << "re: ";
            for (Iter_id iter = bk->related_rules.begin(); iter != bk->related_rules.end(); iter++) {
                in << *iter << " ";
            }
            in <<endl;
        }

    } else {
        for (Iter_son iter = bk->sonList.begin(); iter != bk->sonList.end(); iter++)
            print_bucket(in, *iter, detail);
    }
    return;
}


void bucket_tree::print_tree(const string & filename, bool det) { // const
    ofstream out(filename);
    print_bucket(out, root, det);
    out.close();
}






