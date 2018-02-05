#ifndef UTILITY_H
#define UTILITY_H

#include <algorithm>
#include <vector>
#include <string>
#include <assert.h>

using std::string;
using std::vector;
using std::copy;
using std::pair;
using std::make_pair;

#define MAX_MATCH_BYTE 3 

pair<unsigned, unsigned> approxPortRangeToPrefix(pair<unsigned,
        unsigned> range);

enum FIELD {SRC = 0, DST, TSRC, TDST, FIELD_END};

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

    LongUINT(unsigned val, FIELD f) : LongUINT(){
        switch (f) {
            case SRC:
                vals[2] = val;
                break;
            case DST:
                vals[1] = val;
                break;
            case TSRC:
                vals[0] = (val << 16);
                break;
            case TDST:
                vals[0] = val;
                break;
            default:
                break;
        }  
    }

    LongUINT(const LongUINT & rhs){
        for (int i = 0; i < MAX_MATCH_BYTE; ++i)
            vals[i] = rhs.vals[i];
    }

    bool operator==(const LongUINT & rhs) const{
        for (int i = 0; i < MAX_MATCH_BYTE; ++i){
            if (rhs.vals[i] != vals[i])
                return false;
        }
        return true;
    }

    operator bool() const {
        for (int i = 0; i < MAX_MATCH_BYTE; ++i){
            if (vals[i] != 0)
                return true;
        }
        return false;
    }

    friend LongUINT operator&(const LongUINT & lhs, const LongUINT & rhs){
        LongUINT res;
        for (int i = 0; i < MAX_MATCH_BYTE; ++i)
            res.vals[i] = (lhs.vals[i] & rhs.vals[i]);
        return res;
    }

    friend LongUINT operator^(const LongUINT & lhs, const LongUINT & rhs){
        LongUINT res;
        for (int i = 0; i < MAX_MATCH_BYTE; ++i)
            res.vals[i] = (lhs.vals[i] ^ rhs.vals[i]);
        return res;
    }

    LongUINT operator=(const LongUINT & rhs) {
        if (this != &rhs)
            copy(rhs.vals, rhs.vals + MAX_MATCH_BYTE, this->vals);
        return *this;
    }

    bool divMask(FIELD f){
        switch (f){
            case SRC:
                if (~vals[2] == 0)
                    return false;
                else
                    vals[2] = 0x80000000 + (vals[2] >> 1);
                break;
            case DST:
                if (~vals[1] == 0)
                    return false;
                else
                    vals[1] = 0x80000000 + (vals[1] >> 1);
                break; 
            case TSRC:
                if ((vals[0] >> 16) == 0xffff)
                    return false;
                else
                    vals[0] = (vals[0] & 0xffff) + ((vals[0] & 0xffff0000) >> 1) + 0x80000000;
                break;
            case TDST:
                if ((vals[0] & 0xffff) == 0xffff)
                    return false;
                else
                    vals[0] = (vals[0] & 0xffff0000) + ((vals[0] & 0xffff) >> 1) + 0x8000;
                break;
            default:
                break;
        }

        return true;
    }

};


#endif 
