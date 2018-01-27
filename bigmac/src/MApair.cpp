#include "MApair.h"

Rule::Rule (string & line):MApair() {
    vector<unsigned> items;
    int prev = 1;

    while(int idx = line.find_first_of(".:\t", prev) != string::npos) {
        items.push_back(stoul(line.substr(prev, idx-prev)));
        prev = idx + 1;
    }

    unsigned ipSrc = 0;
    unsigned ipSrcMask = 0;
    unsigned ipDst = 0;
    unsigned ipDstMask = 0;

    for(int i = 0; i < 4; ++i) {
        ipSrc <<= 4;
        ipSrc += items[i];
    }


    ipSrcMask = (~(unsigned)0);
    if (items[5] != 32) {
        ipSrcMask >>= (32 - items[5]);
        ipSrcMask <<= (32 - items[5]);
    }

    for(int i = 5; i < 9; ++i) {
        ipDst <<= 4;
        ipDst += items[i];
    }

    ipSrcMask = (~(unsigned)0);
    if (items[9] != 32) {
        ipDstMask >>= (32 - items[9]);
        ipDstMask <<= (32 - items[9]);
    }

    auto srcP = approxPortRangeToPrefix(make_pair(items[10], items[11]));
    auto dstP = approxPortRangeToPrefix(make_pair(items[12], items[13]));
    match = LongUINT(ipSrc, ipDst, srcP.first, dstP.first);
    mask = LongUINT(ipSrcMask, ipDstMask, srcP.second, dstP.second);
}

pair<unsigned, unsigned> approxPortRangeToPrefix(pair<unsigned,
        unsigned> range) {
    unsigned prefix = 0;
    unsigned mask = 0xFFFF0000;

    if (range.second == 65535 && range.first == 0)
        return make_pair(prefix, mask);

    int length = range.second - range.first + 1;
    unsigned app_len = 1;

    while (length/2 > 0) {
        app_len = app_len * 2;
        length = length/2;
    }

    unsigned mid = range.second - range.second % app_len;
    if (mid + app_len - 1 <= range.second ) {
        prefix = mid;
    } else {
        if (mid - app_len >= range.first)
            prefix = mid - app_len;
        else {
            app_len = app_len/2;
            if (mid + app_len - 1 <= range.second)
                prefix = mid;
            else
                prefix = mid - app_len/2;
        }
    }

    mask = ~(unsigned)0;
    while (app_len > 1) {
        mask = mask << 1;
        app_len = app_len/2;
    }

    mask = mask | ((~unsigned(0))<<16);
    prefix = prefix & mask;
    return make_pair(prefix, mask);
}
