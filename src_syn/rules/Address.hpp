/*
 * FileName: Address.hpp
 * Contributer: Bo Yan (NYU)
 * Description:
 *      Structure for prefix / range address
 */

#ifndef ADDRESS_H
#define ADDRESS_H

#include "../stdafx.h"
#include <boost/functional/hash.hpp>
class range_addr;

class EpochT{
	long int sec;
	long int msec;

	public:
	inline EpochT():sec(0),msec(0){}

	inline EpochT(int isec, int imsec):sec(isec),msec(imsec){}

	inline EpochT(const std::string & str){
		std::vector<std::string> temp;
		boost::split(temp, str, boost::is_any_of("%"));
		sec = boost::lexical_cast<uint32_t> (temp[0]);
		msec = boost::lexical_cast<uint32_t> (temp[1]);
	}

	inline EpochT(const double & dtime){
		sec = int(dtime);
		msec = int((dtime-sec)*1000000);
	}

	inline EpochT(const int & itime){
		sec = int(itime);
		msec = 0;
	}

	inline EpochT(const EpochT & rhs){
		sec = rhs.sec;
		msec = rhs.msec;
	}

	inline EpochT operator+(const double & dtime) const{
		long sec = this->sec + int(dtime);
		long msec = this->msec + int((dtime-int(dtime))*1000000);
		if (msec > 1000000){
			msec -= 1000000;
			sec += 1;
		}
		EpochT res(sec, msec);
		return res;
	}

	inline EpochT operator+(const int & rhs) const{
		EpochT res(this->sec+rhs, this->msec);
		return res;
	}

	inline EpochT operator+(const EpochT & rhs) const{
		long sec = this->sec+rhs.sec;
		long msec = this->msec+rhs.sec;
		if (msec > 1000000){
			msec -= 1000000;
			sec += 1;
		}
		EpochT res(sec, msec);
		return res;
	}

	inline EpochT operator-(const EpochT & rhs) const{
		long sec = this->sec-rhs.sec;
		long msec = this->msec-rhs.sec;
		if (msec < 0){
			msec += 1000000;
			sec -= 1;
		}
		EpochT res(sec, msec);
		return res;
	}

	bool operator<(const EpochT & rhs) const{
		if (this->sec < rhs.sec){
			return true;
		}
		else if(this->sec == rhs.sec){
			if (this->msec < rhs.sec)
				return true;
		}
		return false;
	}

	double toDouble(const EpochT & offset) const{
		double res = this->sec - offset.sec;
		res += double(this->msec - offset.msec)/1000000;
		return res;
	}

};

class addr_5tup {
  public:
    uint32_t addrs[4];
    bool proto;
    double timestamp;

  public:
    inline addr_5tup();
    inline addr_5tup(const addr_5tup &);
    inline addr_5tup(const std::string &); // processing gen
    inline addr_5tup(const std::string &, const EpochT &); // processing raw
    inline addr_5tup(const std::string &, double);

    inline void copy_header(const addr_5tup &);
    inline bool operator==(const addr_5tup &) const;
    inline friend uint32_t hash_value(addr_5tup const &);

    inline std::string str_readable() const;
    inline std::string str_easy_RW() const;
};


class pref_addr {
  public:
    uint32_t pref;
    uint32_t mask;

  public:
    inline pref_addr();
    inline pref_addr(const pref_addr &);
    inline pref_addr(const std::string &);
    
    inline bool operator==(const pref_addr &) const;

    inline bool match (const pref_addr &) const;
    inline bool hit (const uint32_t &) const;
    inline uint32_t get_extreme(bool) const;
    inline uint32_t get_random() const;
    inline bool truncate(pref_addr &) const;
    inline bool truncate(range_addr &) const;

    inline void shrink_shift(int bit, int shift_times); // Feb. 4
    inline void mutate(uint32_t, uint32_t, bool);

    inline void print() const;
    inline std::string get_str() const;
};


class range_addr {
  public:
    uint32_t range[2];

  public:
    inline range_addr();
    inline range_addr(const range_addr &);
    inline range_addr(const std::string &);
    inline range_addr(const pref_addr & );
    inline range_addr(uint32_t, uint32_t);

    inline bool operator<(const range_addr &) const;
    inline bool operator==(const range_addr &) const;
    inline friend uint32_t hash_value(range_addr const & ra);

    inline range_addr intersect(const range_addr &) const;
    inline bool truncate(range_addr &) const;
    inline void shrink_shift(int times, int off_times); // Feb. 4

    inline bool overlap (const range_addr &) const;
    inline bool match (const pref_addr &) const;
    inline bool hit (const uint32_t &) const;
    inline bool join (const range_addr &) const;
    // inline void getTighter(const uint32_t &, const range_addr &);  // Mar 14
    inline friend std::vector<range_addr> minus_rav(std::vector<range_addr> &, std::vector<range_addr> &);

    inline uint32_t get_extreme(bool) const;
    inline uint32_t get_random() const;

    inline void print() const;
    inline std::string get_str() const;
};


// ---------------------- Addr_5tup ---------------------
using std::vector;
using std::string;
using std::stringstream;
using std::cout;
using std::endl;

inline addr_5tup::addr_5tup() {
    for (uint32_t i = 0; i < 4; i++)
        addrs[i] = 0;
    proto = true;
    timestamp = 0;
}

inline addr_5tup::addr_5tup(const addr_5tup & ad) {
    for (uint32_t i = 0; i < 4; i++)
        addrs[i] = ad.addrs[i];
    proto = ad.proto;
    timestamp = ad.timestamp;
}

inline addr_5tup::addr_5tup(const string & str){
    vector<string> temp;
    boost::split(temp, str, boost::is_any_of("%"));
    proto = true;
    timestamp = boost::lexical_cast<double>(temp[0]);
    addrs[0] = boost::lexical_cast<uint32_t>(temp[1]);
    addrs[1] = boost::lexical_cast<uint32_t>(temp[2]);
    addrs[2] = boost::lexical_cast<uint32_t>(temp[3]);
    addrs[3] = boost::lexical_cast<uint32_t>(temp[4]);
}

inline addr_5tup::addr_5tup(const string & str, const EpochT & offset){
    vector<string> temp;
    boost::split(temp, str, boost::is_any_of("%"));
    proto = true;
    EpochT ts_ep(boost::lexical_cast<uint32_t>(temp[0]), boost::lexical_cast<uint32_t>(temp[1]));
    timestamp = ts_ep.toDouble(offset);
    addrs[0] = boost::lexical_cast<uint32_t>(temp[2]);
    addrs[1] = boost::lexical_cast<uint32_t>(temp[3]);
    addrs[2] = boost::lexical_cast<uint32_t>(temp[4]);
    addrs[3] = boost::lexical_cast<uint32_t>(temp[5]);
}

/*
inline addr_5tup::addr_5tup(const string & str, bool readable) {
    vector<string> temp;
    boost::split(temp, str, boost::is_any_of("%"));
    // ts
    timestamp = boost::lexical_cast<double>(temp[0]);
    proto = true;
    // ip
    if (readable) {
        vector<string> temp1;
        boost::split(temp1, temp[1], boost::is_any_of("."));
        addrs[0] = 0;
        for(uint32_t i=0; i<4; i++) {
            addrs[0] = (addrs[0]<<8) + boost::lexical_cast<uint32_t>(temp1[i]);
        }
        boost::split(temp1, temp[2], boost::is_any_of("."));
        addrs[1] = 0;
        for(uint32_t i=0; i<4; i++) {
            addrs[1] = (addrs[1]<<8) + boost::lexical_cast<uint32_t>(temp1[i]);
        }
    } else {
        addrs[0] = boost::lexical_cast<uint32_t>(temp[1]);
        addrs[1] = boost::lexical_cast<uint32_t>(temp[2]);
    }
    // port
    addrs[2] = boost::lexical_cast<uint32_t>(temp[3]);
    addrs[3] = boost::lexical_cast<uint32_t>(temp[4]);
    // proto neglect
}
*/


inline addr_5tup::addr_5tup(const string & str, double ts) {
    vector<string> temp;
    boost::split(temp, str, boost::is_any_of("\t"));
    timestamp = ts;
    proto = true;
    for (uint32_t i = 0; i < 4; i++) {
        addrs[i] = boost::lexical_cast<uint32_t>(temp[i]);
    }
}

inline void addr_5tup::copy_header(const addr_5tup & ad) {
    for (uint32_t i = 0; i < 4; i++)
        addrs[i] = ad.addrs[i];
    proto = ad.proto;
}

inline bool addr_5tup::operator==(const addr_5tup & rhs) const {
    for (uint32_t i = 0; i < 4; i++) {
        if (addrs[i] != rhs.addrs[i])
            return false;
    }
    return (proto == rhs.proto);
}

inline uint32_t hash_value(addr_5tup const & packet) {
    size_t seed = 0;
    boost::hash_combine(seed, packet.addrs[0]);
    boost::hash_combine(seed, packet.addrs[1]);
    boost::hash_combine(seed, packet.addrs[2]);
    boost::hash_combine(seed, packet.addrs[3]);
    return seed;
}

inline string addr_5tup::str_readable() const {
    stringstream ss;
    ss.precision(15);
    ss<<timestamp<<"%";
    for (uint32_t i = 0; i < 2; i++) {
        for (uint32_t j = 0; j < 4; j++) {
            ss << ((addrs[i] >> (24-8*j)) & ((1<<8)-1));
            if (j!=3)
                ss<<".";
        }
        ss<<"%";
    }
    for (uint32_t i = 2; i < 4; i++)
        ss<<addrs[i]<<"%";

    if (proto)
        ss<<"6";
    else
        ss<<"13";
    return ss.str();
}

inline string addr_5tup::str_easy_RW() const {
    stringstream ss;
    ss.precision(15);
    ss<<timestamp<<"%";
    for (uint32_t i = 0; i < 4; i++) {
        ss<<addrs[i]<<"%";
    }
    if (proto)
        ss<<"1";
    else
        ss<<"0";
    return ss.str();
}



/* Constructor with string input
 * parse the string in form of
 * 127.0.0.1 \t 10.0.0.1 \t 1023 \t 24 \t tcp
 * to 5tup
*/
/*
addr_5tup::addr_5tup(string addr_str){

}*/



// ---------------------- pref_addr ---------------------

inline pref_addr::pref_addr() {
    pref = 0;
    mask = 0;
}

inline pref_addr::pref_addr(const string & prefstr) {
    vector<string> temp1;
    boost::split(temp1, prefstr, boost::is_any_of("/"));

    uint32_t maskInt = boost::lexical_cast<uint32_t>(temp1[1]);
    mask = 0;
    if (maskInt != 0)
        mask = ((~uint32_t(0)) << (32-maskInt));

    vector<string> temp2;
    boost::split(temp2, temp1[0], boost::is_any_of("."));

    pref = 0;
    for(uint32_t i=0; i<4; i++) {
        pref = (pref<<8) + boost::lexical_cast<uint32_t>(temp2[i]);
    }
    pref=(pref & mask);
}

inline pref_addr::pref_addr(const pref_addr & pa) {
    pref = pa.pref;
    mask = pa.mask;
}

inline bool pref_addr::operator==(const pref_addr & rhs) const{
	if (pref != rhs.pref)
		return false;
	if (mask != rhs.mask)
		return false;
	return true;
}

inline bool pref_addr::hit(const uint32_t & ad) const {
    return (pref == (ad & mask));
}

inline bool pref_addr::match(const pref_addr & ad) const {
    uint32_t mask_short;
    if (mask > ad.mask)
        mask_short = ad.mask;
    else
        mask_short = mask;

    return ((pref & mask_short) == (ad.pref & mask_short));
}

inline uint32_t pref_addr::get_extreme(bool hi) const {
    if (hi)
        return (pref+(~mask));
    else
        return pref;
}

inline uint32_t pref_addr::get_random() const {
    if (!(~mask+1))
        return pref;
    return (pref + rand()%(~mask+1));
}

inline bool pref_addr::truncate(pref_addr & rule) const {
    if (rule.mask < mask) { // trunc
        if (rule.pref == (pref & rule.mask)) {
            rule.mask = mask;
            rule.pref = pref;
            return true;
        } else
            return false;
    } else {
        if ((rule.pref & mask) == pref)
            return true;
        else
            return false;
    }
}

inline bool pref_addr::truncate(range_addr & rule) const {
    if (rule.range[1] < pref || rule.range[0] > pref+(~mask))
        return false;

    if (rule.range[0] < pref)
        rule.range[0] = pref;
    if (rule.range[1] > pref+(~mask))
        rule.range[1] = pref+(~mask);

    return true;
}

inline void pref_addr::shrink_shift(int bit, int shift_times){
    for (int i = 0; i < bit; ++i)
        mask |= (mask >> 1);
    pref = (pref & mask) + (~mask + 1) * shift_times;
}

inline void pref_addr::mutate(uint32_t s_shrink, uint32_t s_expand, bool port) {
    if (rand()%2 > 0) { // expand
	if (s_expand == 0)
		s_expand = 1;
    	uint32_t mdig = rand() % (s_expand+1);
        for (uint32_t i = 0; i < mdig; ++i) {
            if ((mask == 0 && !port) || (mask == ((~0) << 16) && port))
                break;
            mask = mask << mdig;
        }
	pref = pref & mask;
    } else { // shrink
	if (s_shrink == 0)
		s_shrink = 1;
    	uint32_t mdig = rand() % (s_shrink+1);
        for (uint32_t i = 0; i < mdig; ++i) {
            if (~mask == 0)
                break;
	    uint32_t new_mask = (mask >> 1) + (1 << 31);
	    pref += (rand()%2 * (new_mask - mask));
            mask = new_mask;
        }
        pref = pref & mask;
    }
}

inline void pref_addr::print() const {
    cout<<get_str()<<endl;
}

inline string pref_addr::get_str() const {
    stringstream ss;
    for (uint32_t i = 0; i<4; i++) {
        ss<<((pref>>(24-(i*8))&((1<<8)-1)));
        if (i != 3)
            ss<<".";
    }
    ss<<"/";

    uint32_t m = 0;
    uint32_t mask_cp = mask;

    if ((~mask_cp) == 0) {
        ss<<32;
        return ss.str();
    }
    for (uint32_t i=0; mask_cp; i++) {
        m++;
        mask_cp = (mask_cp << 1);
    }
    ss<<m;
    return ss.str();
}


/* ---------------------- range_addr ---------------------
 * brief:
 * range address: two icons are range start and termin
 */

/* constructors:
 *
 * options:
 * 	()			default
 * 	(const range_addr &)	copy
 * 	(const string &)	generate from a string  "1:1024"
 * 	(const pref_addr &)	transform out of a prefix_addr
 * 	(uint32_t, uint32_t)	explicitly initialize with range value
 */
inline range_addr::range_addr() {
    range[0] = 0;
    range[1] = 0;
}

inline range_addr::range_addr(const range_addr & ra) {
    range[0] = ra.range[0];
    range[1] = ra.range[1];
}

inline range_addr::range_addr(const string & rangestr) {
    vector<string> temp1;
    boost::split(temp1, rangestr, boost::is_any_of(":"));
    boost::trim(temp1[0]);
    boost::trim(temp1[1]);
    range[0] = boost::lexical_cast<uint32_t> (temp1[0]);
    range[1] = boost::lexical_cast<uint32_t> (temp1[1]);
}

inline range_addr::range_addr(const pref_addr & rule) {
    range[0] = rule.pref;
    range[1] = rule.pref + (~rule.mask);
}

inline range_addr::range_addr(uint32_t i, uint32_t j) {
    range[0] = i;
    range[1] = j;
}

/* operator functions
 *
 * for hash_bashed and comparison based use
 */
inline bool range_addr::operator<(const range_addr & ra) const {
    return range[0]< ra.range[0];
}

inline bool range_addr::operator==(const range_addr & ra) const {
    return ( range[0] == ra.range[0] && range[1] == ra.range[1]);
}

inline uint32_t hash_value(range_addr const & ra) {
    size_t seed = 0;
    boost::hash_combine(seed, ra.range[0]);
    boost::hash_combine(seed, ra.range[1]);
    return seed;
}

/* member function
 */

inline range_addr range_addr::intersect(const range_addr & ra) const { // return the join of two range addr  sym
    uint32_t lhs = range[0] > ra.range[0] ? range[0] : ra.range[0];
    uint32_t rhs = range[1] < ra.range[1] ? range[1] : ra.range[1];
    return range_addr(lhs, rhs);
}

inline bool range_addr::truncate(range_addr & ra) const { // truncate a rule using current rule  sym
    if (ra.range[0] > range[1] || ra.range[1] < range[0])
        return false;
    if (ra.range[0] < range[0])
        ra.range[0] = range[0];
    if (ra.range[1] > range[1])
        ra.range[1] = range[1];
    return true;
}

inline void range_addr::shrink_shift(int times, int off_times){
    uint32_t offset = (range[0] + range[1])/2;
    uint32_t len = (range[1] - range[0] + 1) / times;
    range[0] = offset - len/2;
    range[1] = offset + len/2;
    offset = off_times * len;
    range[0] += offset;
    range[1] += offset;
}

inline bool range_addr::overlap(const range_addr & ad) const { // whether two range_addr overlap  sym
    return (!(range[1] < ad.range[0]) || (range[0] > ad.range[1]));
}

inline bool range_addr::match(const pref_addr & ad) const { // whether a range matchs a prefix  sym
    return (! ((range[1] & ad.mask) < ad.pref || (range[0] & ad.mask) > ad.pref));
}

inline bool range_addr::hit(const uint32_t & ad) const { // whether a packet hit
    return (range[0] <= ad && range[1] >= ad);
}

inline bool range_addr::join(const range_addr & ad){
    if (range[0] < ad.range[0])
        range[0] = ad.range[0];
    if (range[1] > ad.range[1])
        range[1] = ad.range[1];
}
/*
inline void range_addr::getTighter(const uint32_t & hit, const range_addr & ra) { // get the micro address
    if (ra.range[0] <= hit && ra.range[0] > range[0]) {
        range[0] = ra.range[0];
    }

    if (ra.range[1] < hit && ra.range[1] > range[0]) {
        range[0] = ra.range[1]+1;
    }

    if (ra.range[0] > hit && ra.range[0] < range[1]) {
        range[1] = ra.range[0] - 1;
    }

    if (ra.range[1] >= hit && ra.range[1] < range[1]) {
        range[1] = ra.range[1];
    }
}*/

inline vector<range_addr> minus_rav(vector<range_addr> & lhs, vector<range_addr> & rhs) { // minus the upper rules
    vector <range_addr> res;
    sort(lhs.begin(), lhs.end());
    sort(rhs.begin(), rhs.end());
    vector<range_addr>::const_iterator iter_l = lhs.begin();
    vector<range_addr>::const_iterator iter_r = rhs.begin();
    while (iter_l != lhs.end()) {
        uint32_t lb = iter_l->range[0];
        while (iter_r != rhs.end()) {
            if (iter_r->range[1] < iter_l->range[0]) {
                iter_r++;
                continue;
            }
            if (iter_r->range[0] > iter_l->range[1]) {
                break;
            }
            range_addr minus_item = iter_l->intersect(*iter_r);
            if (lb < minus_item.range[0])
                res.insert(res.end(), range_addr(lb, minus_item.range[0]-1));
            lb = minus_item.range[1]+1;
            iter_r++;
        }
        if (lb <= iter_l->range[1])
            res.insert(res.end(), range_addr(lb, iter_l->range[1]));
        iter_l++;
    }
    return res;
}

inline uint32_t range_addr::get_extreme(bool hi) const { // get the higher or lower range of the addr
    if (hi)
        return range[1];
    else
        return range[0];
}

inline uint32_t range_addr::get_random() const { // get a random point picked
    return (range[0] + rand()%(range[1]-range[0] + 1));
}



/* print function
 */
inline void range_addr::print() const {
    cout<< get_str() <<endl;
}

inline string range_addr::get_str() const {
    stringstream ss;
    ss<<range[0] << ":" << range[1];
    return ss.str();
}

#endif

