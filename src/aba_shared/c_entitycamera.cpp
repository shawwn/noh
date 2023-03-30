// (C)2010 S2 Games
// c_entitycamera.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entitycamera.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Entity, Camera)

DEFINE_ENTITY_DESC(CEntityCamera, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(g_heapTypeVector,    TypeVector)();
    s_cDesc.pFieldTypes->clear();
    const TypeVector &vBase(IVisualEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());
}
//=============================================================================

/*====================
  CEntityCamera::CEntityCamera
  ====================*/
CEntityCamera::CEntityCamera() :
m_uiStartMoveTime(INVALID_TIME),
m_uiMoveDuration(0),
m_v3StartPos(V_ZERO),
m_v3EndPos(V_ZERO)
{
}



/*====================
  CEntityCamera::Baseline
  ====================*/
void    CEntityCamera::Baseline()
{
    IVisualEntity::Baseline();
}


/*====================
  CEntityCamera::GetSnapshot
  ====================*/
void    CEntityCamera::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    IVisualEntity::GetSnapshot(snapshot, uiFlags);

}


/*====================
  CEntityCamera::ReadSnapshot
  ====================*/
bool    CEntityCamera::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    if (!IVisualEntity::ReadSnapshot(snapshot, 1))
        return false;

    return true;
}


/*====================
  CEntityCamera::Copy
  ====================*/
void    CEntityCamera::Copy(const IGameEntity &B)
{
    IVisualEntity::Copy(B);

    const CEntityCamera *pB(static_cast<const CEntityCamera *>(&B));

    if (!pB)    
        return;

    //const CEntityCamera &C(*pB);
}


/*====================
  CEntityCamera::ApplyWorldEntity
  ====================*/
void    CEntityCamera::ApplyWorldEntity(const CWorldEntity &ent)
{
    IVisualEntity::ApplyWorldEntity(ent);
}


/*====================
  CEntityCamera::Spawn
  ====================*/
void    CEntityCamera::Spawn()
{
}


/*====================
  CEntityCamera::ServerFrameCleanup
  ====================*/
bool    CEntityCamera::ServerFrameCleanup()
{
    if (m_uiStartMoveTime != INVALID_TIME)
    {
        if (Game.GetGameTime() >= m_uiStartMoveTime + m_uiMoveDuration)
        {
            m_v3Position = m_v3EndPos;
            m_uiStartMoveTime = INVALID_TIME;
            m_uiMoveDuration = 0;
        }
        else
        {
            m_v3Position = LERP(float(Game.GetGameTime() - m_uiStartMoveTime) / m_uiMoveDuration, m_v3StartPos, m_v3EndPos);
        }
    }

    return true;
}


/*====================
  CEntityCamera::StartMove
  ====================*/
void    CEntityCamera::StartMove(uint uiStartTime, uint uiDuration, const CVec3f &v3StartPos, const CVec3f &v3EndPos)
{
    m_uiStartMoveTime = uiStartTime;
    m_uiMoveDuration = uiDuration;
    m_v3StartPos = v3StartPos;
    m_v3EndPos = v3EndPos;
}
