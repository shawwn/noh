// (C)2010 S2 Games
// c_entitycamera.h
//
//=============================================================================
#ifndef __C_ENTITYCAMERA_H__
#define __C_ENTITYCAMERA_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CEntityCamera
//=============================================================================
class CEntityCamera : public IVisualEntity
{
    DECLARE_ENTITY_DESC

protected:
    DECLARE_ENT_ALLOCATOR2(Entity, Camera);

    uint            m_uiStartMoveTime;
    uint            m_uiMoveDuration;
    CVec3f          m_v3StartPos;
    CVec3f          m_v3EndPos;

public:
    ~CEntityCamera()    {}
    CEntityCamera();
    
    virtual void            Baseline();
    virtual void            GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    virtual bool            ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);
    virtual void            Copy(const IGameEntity &B);

    virtual void            ApplyWorldEntity(const CWorldEntity &ent);

    virtual void            Spawn();

    virtual bool            ServerFrameCleanup();

    GAME_SHARED_API void    StartMove(uint uiStartTime, uint uiDuration, const CVec3f &v3StartPos, const CVec3f &v3EndPos);

    bool                    IsMoving() const        { return m_uiStartMoveTime != INVALID_TIME; }
};
//=============================================================================

#endif //__C_ENTITYCAMERA_H__
