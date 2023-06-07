// (C)2005 S2 Games
// c_serverentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_server_common.h"

#include "c_serverentity.h"

#include "../shared/c_worldentity.h"
#include "../shared/c_buffer.h"
//=============================================================================

/*====================
  CServerEntity::~CServerEntity
  ====================*/
CServerEntity::~CServerEntity()
{
}


/*====================
  CServerEntity::CServerEntity
  ====================*/
CServerEntity::CServerEntity(EGameEntType eType) :
IGameEntity(eType)
{
}

CServerEntity::CServerEntity(EGameEntType eType, const CWorldEntity *worldEnt) :
IGameEntity(eType)
{
    m_uiWorldIndex = worldEnt->GetIndex();

    m_v3Position = worldEnt->GetOrigin();
    m_v3Angles = worldEnt->GetAngles();
    m_fScale = worldEnt->GetScale();

    m_hModel = worldEnt->GetModelHandle();
}


/*====================
  CServerEntity::GetUpdateData
  ====================*/
void    CServerEntity::GetUpdateData(CBufferDynamic &buffer) const
{
    buffer << m_v3Position << m_v3Angles << m_fScale << m_hModel;
}
