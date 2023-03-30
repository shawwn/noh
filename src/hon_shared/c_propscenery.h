// (C)2006 S2 Games
// c_propscenery.h
//
//=============================================================================
#ifndef __C_PROPSCENERY_H__
#define __C_PROPSCENERY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_propentity.h"
#include "../k2/c_sceneentity.h"
//=============================================================================

//=============================================================================
// CPropScenery
//=============================================================================
class CPropScenery : public IPropEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Prop, Scenery);

    CSceneEntity        m_cSceneEntity;

public:
    ~CPropScenery() {}
    CPropScenery() :
    IPropEntity(GetEntityConfig())
    {}

    virtual bool        IsStatic() const                { return true; }

    virtual void        Spawn();

    virtual bool        IsVisibleOnMap(CPlayer *pLocalPlayer) const                 { return false; }
    virtual void        DrawOnMap(class CUITrigger &minimap, CPlayer *pLocalPlayer) {}

    virtual bool        AddToScene(const CVec4f &v4Color, int iFlags);
    
    virtual void        Copy(const IGameEntity &B);
};
//=============================================================================

#endif //__C_PROPSCENERY_H__
