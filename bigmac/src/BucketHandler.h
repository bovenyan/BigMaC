#ifndef BUCKET_HANDLER
#define BUCKET_HANDLER

#include "MApair.h"

class BucketHandler{
private:
    Bucket * root;
public:
    void GenerateBucket();

    ~BucketHandler();
};

#endif
