// (C)2008 S2 Games
// c_entitylanenode.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "c_entitylanenode.h"

#include "../k2/c_texture.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Entity, LaneNode)
//=============================================================================

/*====================
  CEntityLaneNode::ApplyWorldEntity
  ====================*/
void    CEntityLaneNode::ApplyWorldEntity(const CWorldEntity &ent)
{
    IVisualEntity::ApplyWorldEntity(ent);

    m_sTarget = ent.GetProperty(_T("target0"), TSNULL);
}
