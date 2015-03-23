#include "BucketTree.h"

void bucket_tree::gen_candi_cuts(int sum){
    if (sum == 1){ // [0] [1] [2] [3]
        candi_cuts.clear();
        candi_cuts.push_back(vector<int>(1,0));
        candi_cuts.push_back(vector<int>(1,1));
        candi_cuts.push_back(vector<int>(1,2));
        candi_cuts.push_back(vector<int>(1,3));
    }
    gen_candi_cuts(sum-1);

    vector<vector<int> > new_candi_cuts;
    for(auto iter = candi_cuts.begin(); iter != candi_cuts.end(); ++iter){
        for (int i = 0; i < 4; ++i){ // [..,0] [..,1] [..,2] [..,3]
            vector<int> cur = *iter;
            cur.push_back(i);
            new_candi_cuts.push_back(cur);
        }
    }
    candi_cuts = new_candi_cuts;
}

void bucket_tree::make_tree(bucket * ptr, int level) {
    if (level == 0)
        return;

    pair<double, int> opt_cost = std::make_pair(ptr->rela_size(), ptr->rela_size());
    vector<int> opt_cut;

    for (auto iter_cut = candi_cuts.begin(); iter_cut != candi_cuts.end(); ++iter_cut){
        // test availability
        if (ptr->splittable(*iter_cut)){
            auto cost = ptr->split(*iter_cut, rList);
            
            if ((cost.first < opt_cost.first) || 
                (cost.first == opt_cost.first && 
                 cost.second < opt_cost.second)){
                opt_cut = *iter_cut;
                opt_cost = cost;
            }
        }
    }

    if (opt_cut.empty())
        ptr->cleanSon();
    else{
        ptr->split(opt_cut, rList);
        for (auto iter = ptr->sonList.begin(); iter != ptr->sonList.end(); ++iter)
            make_tree(*iter, level-1); // DFS
    }
}

// ---------- bucket_tree ------------
bucket_tree::bucket_tree() {
}

bucket_tree::bucket_tree(rule_list * rL, int level, int cut_no ) {
    rList = rL;
    root = new bucket(rList, true); // full address space
    gen_candi_cuts(cut_no);
    make_tree(root, level);
}

bucket_tree::~bucket_tree() {
    delNode(root);
}

pair<bucket *, int> bucket_tree::search_bucket(const addr_5tup& packet, bucket * buck) const {
    if (!buck->sonList.empty()) {
        return search_bucket(packet, buck->gotoSon(packet));
    } else {
        return std::make_pair(buck, buck->search_rela(packet, rList));
    }
}


void bucket_tree::delNode(bucket * ptr) {
    ptr->delSubTree();
}

/*
 * debug
 */

void bucket_tree::print_bucket(ofstream & in, bucket * bk, bool detail) { // const
    if (bk->sonList.empty()) {
        in << bk->get_str(detail) << endl;
    } else {
        for (auto iter = bk->sonList.begin(); iter != bk->sonList.end(); iter++)
            print_bucket(in, *iter, detail);
    }
    return;
}


void bucket_tree::print_tree(const string & filename, bool det) { // const
    ofstream out(filename);
    print_bucket(out, root, det);
    out.close();
}

