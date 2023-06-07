// (C)2006 S2 Games
// i_light.h
//
//=============================================================================
#ifndef __I_LIGHT_H__
#define __I_LIGHT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CWorldLight;
//=============================================================================

//=============================================================================
// ILight
//=============================================================================
class ILight : public IVisualEntity
{
private:
    static vector<SDataField>   *s_pvFields;
    
    START_ENTITY_CONFIG(IVisualEntity)
    END_ENTITY_CONFIG
    
    CVec3f  m_v3Color;
    float   m_fFalloffStart;
    float   m_fFalloffEnd;

public:
    ~ILight()   {}
    ILight();

    bool                IsLight() const             { return true; }

    // Network
    virtual void    Baseline();
    virtual void    GetSnapshot(CEntitySnapshot &snapshot) const;
    virtual bool    ReadSnapshot(CEntitySnapshot &snapshot);

    GAME_SHARED_API static const vector<SDataField>&    GetTypeVector();

    GAME_SHARED_API void        Spawn();

    virtual bool    ServerFrame()                   { return true; }

    virtual bool    AddToScene(const CVec4f &v4Color, int iFlags);

    void            Copy(const IGameEntity &B);

    virtual bool    AIShouldTarget()                                                { return false; }
    virtual bool    IsVisibleOnMinimap(IPlayerEntity *pLocalPlayer, bool bLargeMap) { return false; }
};
//=============================================================================

#endif //__I_LIGHT_H__
