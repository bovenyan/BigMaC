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

// Mar 22 job save
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

