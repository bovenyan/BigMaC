#include "Bucket.h"

// --------- bucket ------------

using std::vector;
using std::list;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::set;


namespace logging = boost::log;
namespace src = boost::log::sources;
namespace attrs = boost::log::attributes;

typedef vector<uint32_t>::iterator Iter_id;
typedef vector<bucket*>::iterator Iter_son;


src::logger bucket::lg = src::logger();

void bucket::logger_init() {
    bucket::lg.add_attribute("Class", attrs::constant< string > ("BuckObj "));
}

bucket::bucket():hit(false), parent(NULL), max_gain(0) {}

bucket::bucket(const bucket & bk) : b_rule(bk) {
    sonList = vector<bucket*>();
    assoc_fwd_rules = vector<uint32_t>();
    assoc_mgmt_rules = vector<uint32_t>();
    hit = false;
    parent = NULL;
    max_gain = 0;
}

bucket::bucket(const string & b_str, const pipe_line * p_line) : b_rule(b_str) {
    for (size_t idx = 0; idx != p_line->fwd_table.size(); ++idx)
        if (b_rule::match_rule(p_line->fwd_table[idx]))
            assoc_fwd_rules.push_back(idx);
    for (size_t idx = 0; idx != p_line->mgmt_table.size(); ++idx)
        if (b_rule::match_rule(p_line->mgmt_table[idx]))
            assoc_fwd_rules.push_back(idx);
    hit = false;
    parent = NULL;
    max_gain = 0;
}

pair<double, size_t> bucket::split(const vector<size_t> & dim , pipe_line * p_line) {
    if (!sonList.empty())
        cleanson();

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

    sonList.reserve(total_son_no);

    size_t total_rule_no = 0;
    size_t largest_rule_no = 0;

    for (size_t i = 0; i < total_son_no; ++i) {
        bucket * son_ptr = new bucket(*this);
        son_ptr->parent = this;

        // Calculate the new mask and addr
        uint32_t id = i;
        for (size_t j = 0; j < 4; ++j) {
            son_ptr->addrs[j].mask = new_masks[j];
            size_t incre = (~(new_masks[j]) + 1);
            son_ptr->addrs[j].pref += (id % (1 << dim[j]))*incre;
            id = (id >> dim[j]);
        }

        // Calculate assoc rules
        for (Iter_id iter = assoc_fwd_rules.begin();
                iter != assoc_fwd_rules.end(); ++iter) {
            if (son_ptr->match_rule(p_line->fwd_table[*iter]))
                son_ptr->assoc_fwd_rules.push_back(*iter);
        }

        for (Iter_id iter = assoc_mgmt_rules.begin();
                iter != assoc_mgmt_rules.end(); ++iter) {
            if (son_ptr->match_rule(p_line->mgmt_table[*iter]))
                son_ptr->assoc_mgmt_rules.push_back(*iter);
        }

        total_rule_no += son_ptr->assoc_fwd_rules.size() +
                         son_ptr->assoc_mgmt_rules.size();

        largest_rule_no = std::max(largest_rule_no,
                                   son_ptr->assoc_fwd_rules.size() + son_ptr->assoc_mgmt_rules.size());

        sonList.push_back(son_ptr);
    }

    return std::make_pair(double(total_rule_no)/total_son_no, largest_rule_no);
}


int bucket::reSplit(const vector<size_t> & dim , pipe_line * p_line_ptr,
                    bool apply_or_not) {
    if (!sonList.empty())
        cleanson();

    uint32_t new_masks[4];
    size_t total_son_no = 1;

    for (size_t i = 0; i < 4; ++i) { // new mask
        new_masks[i] = addrs[i].mask;

        for (size_t j = 0; j < dim[i]; ++j) {
            if (~(new_masks[i]) == 0)
                return -1; // invalid

            new_masks[i] = (new_masks[i] >> 1) + (1 << 31);
            total_son_no *= 2;
        }
    }

    sonList.reserve(total_son_no);
    set<size_t> to_cache_fwd_rules;
    set<size_t> to_cache_mgmt_rules;
    int gain = 0;

    for (size_t i = 0; i < total_son_no; ++i) {
        bool to_cache = false;
        bucket * son_ptr = new bucket(*this);
        son_ptr->parent = this;

        uint32_t id = i;
        for (size_t j = 0; j < 4; ++j) { // new pref
            son_ptr->addrs[j].mask = new_masks[j];
            size_t incre = (~(new_masks[j]) + 1);
            son_ptr->addrs[j].pref += (id % (1 << dim[j]))*incre;
            id = id >> dim[j];
        }
        
        // check whether the bucket needs to be cached
        for (Iter_id iter = assoc_fwd_rules.begin();
                iter != assoc_fwd_rules.end(); ++iter) { // rela rule
            if (son_ptr->match_rule(p_line_ptr->fwd_table[*iter])) {
                son_ptr->assoc_fwd_rules.push_back(*iter);

                if (p_line_ptr->fwd_table[*iter].hit) {
                    to_cache = true;
                }
            }
        }

        for (Iter_id iter = assoc_mgmt_rules.begin();
                iter != assoc_mgmt_rules.end(); ++iter) { // rela rule
            if (son_ptr->match_rule(p_line_ptr->mgmt_table[*iter])) {
                son_ptr->assoc_mgmt_rules.push_back(*iter);

                if (p_line_ptr->mgmt_table[*iter].hit) {
                    to_cache = true;
                }
            }
        }

        // needs to be cached
        if (to_cache) {
            --gain; // cache one more bucket

            for (auto iter = son_ptr->assoc_fwd_rules.begin(); 
                    iter != son_ptr->assoc_fwd_rules.end(); ++iter) {
                to_cache_fwd_rules.insert(*iter);
                if (apply_or_not)  // apply the occupancy to the bucket
                    p_line_ptr->fwd_table[*iter].inc_occupancy();
            }

            for (auto iter = son_ptr->assoc_mgmt_rules.begin(); 
                    iter != son_ptr->assoc_mgmt_rules.end(); ++iter) {
                to_cache_mgmt_rules.insert(*iter);
                if (apply_or_not)  // apply the occupancy to the bucket
                    p_line_ptr->mgmt_table[*iter].inc_occupancy();
            }
        }
        sonList.push_back(son_ptr);
    }


    if (apply_or_not) { // remove the occupancy of old bucket
        for (auto iter = assoc_fwd_rules.begin(); iter != assoc_fwd_rules.end(); ++iter)
            p_line_ptr->fwd_table[*iter].dec_occupancy();
    } else {
        ++gain; // gain 1 by removing old buck
        for (auto iter = assoc_fwd_rules.begin(); 
                iter != assoc_fwd_rules.end(); ++iter) {
            if ((to_cache_fwd_rules.find(*iter) == to_cache_fwd_rules.end()) &&  // not cached
                    p_line_ptr->fwd_table[*iter].get_occupancy() == 1) // dominant the bucket
                ++gain;
        }
    }

    return gain;
}


vector<size_t> bucket::unq_comp(pipe_line * p_line_ptr) {
    vector<size_t> result;
    size_t sum = 0;

    for (size_t i = 0; i < 2; ++i) {
        set<size_t> comp;

        for (auto iter = related_rules.begin(); iter != related_rules.end(); ++iter) {
            size_t pref = rList->list[*iter].hostpair[i].pref;
            size_t mask = rList->list[*iter].hostpair[i].mask;
            comp.insert(pref);
            comp.insert(pref+mask);
        }

        result.push_back(comp.size()-1);
        sum += comp.size() - 1;
    }

    for (size_t i = 0; i < 2; ++i) {
        set<size_t> comp;
        for (auto iter = related_rules.begin(); iter != related_rules.end(); ++iter) {
            comp.insert(rList->list[*iter].portpair[i].range[0]);
            comp.insert(rList->list[*iter].portpair[i].range[1]);
        }
        result.push_back(comp.size()-1);
        sum += comp.size() - 1;
    }
    double avg = sum/4;

    vector<size_t> outstand;
    for (size_t i = 0; i < 4; ++i) {
        if (result[i] >= avg)
            outstand.push_back(i);
    }

    return outstand;
}

string bucket::get_str() const {
    stringstream ss;
    ss << b_rule::get_str() << "\t associated fwd" << assoc_fwd_rules.size();
    ss << "\t associated mgmt rules" << assoc_mgmt_rules.size();
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

