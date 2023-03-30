// (C)2006 S2 Games
// c_effect.h
//
//=============================================================================
#ifndef __C_EFFECT_H__
#define __C_EFFECT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_resource.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CEffectThread;
class CParticleSystemDef;
class CModifierDef;
class IEmitterDef;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef hash_map<tstring, CParticleSystemDef *> ParticleSystemMap;
typedef hash_map<tstring, CModifierDef *>           ModifierMap;
typedef hash_map<tstring, IEmitterDef *>            EmitterDefMap;
//=============================================================================

//=============================================================================
// CEffect
//=============================================================================
class CEffect : public IResource
{
private:
    tstring             m_sEffectName;

    CEffectThread*      m_pEffectThread;    // Holds the default settings for
                                            // new effect thread instances

    ParticleSystemMap   m_mapParticleSystems;
    ModifierMap         m_mapModifiers;
    EmitterDefMap       m_mapEmitterDefs;

    bool                m_bDeferred;
    bool                m_bPersistent;
    bool                m_bPausable;
    bool                m_bUseEntityEffectScale;

public:
    ~CEffect()  {}
    CEffect(const tstring &sPath);

    K2_API  virtual uint            GetResType() const          { return RES_EFFECT; }
    K2_API  virtual const tstring&  GetResTypeName() const      { return ResTypeName(); }
    K2_API  static const tstring&   ResTypeName()               { static tstring sTypeName(_T("{effect}")); return sTypeName; }

    int     Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
    void    Free();

    void    Setup(const tstring &sName, bool bDeferred, bool bPersistent, bool bPausable, bool bUseEntityEffectScale);

    const tstring&      GetEffectName() const                                   { return m_sEffectName; }
    CEffectThread*      GetEffectThread()                                       { return m_pEffectThread; }

    CParticleSystemDef* GetParticleSystemDef(const tstring &sParticleSystem);
    CModifierDef*       GetModifierDef(const tstring &sModifier);
    IEmitterDef*        GetEmitterDef(const tstring &sEmitter);

    void    AddParticleSystemDef(const tstring &sName, CParticleSystemDef *pParticleSystemDef);
    void    AddModifierDef(const tstring &sName, CModifierDef *pModifierDef);
    void    AddEmitterDef(const tstring &sName, IEmitterDef *pParticleSystemDef);
    void    SetEffectThread(CEffectThread *pEffectThread);

    K2_API CEffectThread*   SpawnThread(uint uiStartTime);

    bool    GetDeferred()           { return m_bDeferred; }
    bool    GetPersistent()         { return m_bPersistent; }
    bool    GetPausable()           { return m_bPausable; }
    bool    GetUseEntityEffectScale()   { return m_bUseEntityEffectScale; }
};
//=============================================================================
#endif //__C_EFFECT_H__
