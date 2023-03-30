// (C)2005 S2 Games
// modelclip_t.h
//
//=============================================================================
#ifndef __MODELCLIP_T_H__
#define __MODELCLIP_T_H__

struct SModelClip
{
    struct media_s  *media;

    int             num_motions;        
    int             num_frames;
    
    struct boneMotion_s *motions;           //will allocate num_motions motions
};
#endif //__MODELCLIP_T_H__
