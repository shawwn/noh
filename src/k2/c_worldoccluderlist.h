// (C)2006 S2 Games
// c_worldoccluderlist.h
//
//=============================================================================
#ifndef __C_WORLDOCCLUDERLIST_H__
#define __C_WORLDOCCLUDERLIST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class COccluder;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef map<uint, COccluder *>      OccluderMap;
typedef OccluderMap::iterator       OccluderMap_it;
//=============================================================================

//=============================================================================
// CWorldOccluderList
//=============================================================================
class CWorldOccluderList : public IWorldComponent
{
private:
    OccluderMap     m_mapOccluders;

public:
    ~CWorldOccluderList();
    CWorldOccluderList(EWorldComponent eComponent);

    bool    Load(CArchive &archive, const CWorld *pWorld);
    bool    Generate(const CWorld *pWorld);
    void    Release();
    bool    Serialize(IBuffer *pBuffer);

    K2_API uint             AllocateNewOccluder(uint uiIndex = INVALID_INDEX);
    K2_API COccluder*       GetOccluder(uint uiIndex, bool bThrow = NO_THROW);
    K2_API OccluderMap&     GetOccluderMap()                    { return m_mapOccluders; }
    K2_API void             DeleteOccluder(uint uiIndex);
};
//=============================================================================
#endif //__C_WORLDOCCLUDERLIST_H__
