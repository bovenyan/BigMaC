#ifndef UTILITY_H
#define UTILITY_H

#include <algorithm>
#include <vector>
#include <string>

using std::string;
using std::vector;
using std::copy;
using std::pair;
using std::make_pair;

#define MAX_MATCH_BYTE 4

pair<unsigned, unsigned> approxPortRangeToPrefix(pair<unsigned,
        unsigned> range);

class LongUINT{
private:
    unsigned vals[MAX_MATCH_BYTE];
public:
    LongUINT(){
       for (int i = 0; i < MAX_MATCH_BYTE; ++i)
          vals[i] = 0; 
    }

    LongUINT(unsigned val1, unsigned val2, 
            unsigned val3, unsigned val4){
        vals[2] = val1;
        vals[1] = val2;
        vals[0] = ((val3 & 0xFFFF) << 16) + (val4 & 0xFFFF); 
    }


    bool operator==(const LongUINT & rhs) const{
        for (int i = 0; i < MAX_MATCH_BYTE; ++i){
            if (rhs.vals[i] != vals[i])
                return false;
        }
        return true;
    }

    friend LongUINT operator&(const LongUINT & lhs, const LongUINT & rhs){
        LongUINT res;
        for (int i = 0; i < MAX_MATCH_BYTE; ++i)
            res.vals[i] = (lhs.vals[i] & rhs.vals[i]);
        return res;
    }

    LongUINT operator=(const LongUINT & rhs) {
        if (this != &rhs)
            copy(rhs.vals, rhs.vals + MAX_MATCH_BYTE, this->vals);
        return *this;
    }
};


#endif 
