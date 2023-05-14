// (C)2007 S2 Games
// c_resultsegment.h
//
//=============================================================================
#ifndef __C_RESULTSEGMENT_H__
#define __C_RESULTSEGMENT_H__

#include "c_searchnode.h"

class CSearchNode;
//enum ESearchDirection;


class CSegmentWaypoint
{
public:
    CSearchNode *pNode;
    ESearchDirection eLastDirection;
    uint uiLength;
    int iPos;
    int iNeg;

    CVec2f  v2Path;
    float   fPos;
    float   fNeg;

    CSegmentWaypoint() : pNode(nullptr), uiLength(0), iPos(0), iNeg(0) { }
};

typedef vector<CSegmentWaypoint> SegmentWaypoints;

#endif //__C_RESULTSEGMENT_H__
