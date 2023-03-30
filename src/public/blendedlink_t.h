#ifndef __BLENDEDLINK_T__
#define __BLENDEDLINK_T__

struct SBlendedLink
{
    uint            *indexes;       //num_weights links will be allocated

    float           *weights;       //num_weights will be allocated

    int             num_weights;    //number of weights this vertex is affected by (from 1 to CModel::numBones, or any maximum that we decide to set)
                                    //if num_weights is 1, use the union member "index" to get the bone attachment

    //SBlendedLink() : indexes(NULL), weights(NULL), num_weights(0) {}
};

#endif // __BLENDEDLINK_T__
