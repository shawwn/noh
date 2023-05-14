// (C)2006 S2 Games
// c_effect.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_effect.h"
#include "i_resourcelibrary.h"
#include "c_xmlmanager.h"
#include "c_effectthread.h"
#include "c_particlesystem.h"
#include "c_sceneentitymodifier.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_BOOLF(efx_debug,   false,  CONEL_DEV);
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
IResource*  AllocEffect(const tstring &sPath);
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary    g_ResLibEffect(RES_EFFECT, _T("Effects"), CEffect::ResTypeName(), true, AllocEffect);
//=============================================================================

/*====================
  AllocEffect
  ====================*/
IResource*  AllocEffect(const tstring &sPath)
{
    return K2_NEW(ctx_Effects,  CEffect)(sPath);
}


/*====================
  CEffect::CEffect
  ====================*/
CEffect::CEffect(const tstring &sPath) :
IResource(sPath, _T("")),
m_pEffectThread(nullptr),
m_bDeferred(false),
m_bPersistent(false),
m_bPausable(false),
m_bUseEntityEffectScale(false)
{
}


/*====================
  CEffect::Load
  ====================*/
int     CEffect::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
    PROFILE("CEffect::Load");

    assert(!(m_sPath.empty() && m_sName.empty()));
    if (m_sPath.empty() && m_sName.empty())
        return RES_LOAD_FAILED;

    tstring sStrEffect(m_sPath);
    if (m_sPath.empty())
        sStrEffect = m_sName;

    if (pData == nullptr || uiSize <= 0)
    {
        Console.Err << _T("Invalid effect '^y") << sStrEffect << _T("^*'") << newl;
        return RES_LOAD_FAILED;
    }

    try
    {
        // Dedicated servers don't need .effect files so skip this and save some memory
        if (K2System.IsDedicatedServer() || K2System.IsServerManager())
            return false;

        Console.Res << _T("Loading Effect ") << SingleQuoteStr(sStrEffect);
        if ((uiIgnoreFlags & RES_EFFECT_IGNORE_ALL))
            Console.Res << _T(" [IGNORE_ALL]");
        Console.Res << newl;

        m_uiIgnoreFlags = uiIgnoreFlags;

        if (!(uiIgnoreFlags & RES_EFFECT_IGNORE_ALL))
        {
            if (!XMLManager.ReadBuffer(pData, uiSize, _T("effect"), this))
                throw CException(_TS("CEffect::Load(") + m_sPath + _T(") - couldn't read XML"), E_WARNING);
        }
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CEffect::Load(") + sStrEffect + _TS(") - "), NO_THROW);
        return RES_LOAD_FAILED;
    }

    return 0;
}


/*====================
  CEffect::Free
  ====================*/
void    CEffect::Free()
{
    SAFE_DELETE(m_pEffectThread);

    for (ParticleSystemMap::iterator it(m_mapParticleSystems.begin()); it != m_mapParticleSystems.end(); ++it)
        K2_DELETE(it->second);

    m_mapParticleSystems.clear();

    for (ModifierMap::iterator it(m_mapModifiers.begin()); it != m_mapModifiers.end(); ++it)
        K2_DELETE(it->second);

    m_mapModifiers.clear();

    for (EmitterDefMap::iterator it(m_mapEmitterDefs.begin()); it != m_mapEmitterDefs.end(); ++it)
        K2_DELETE(it->second);

    m_mapEmitterDefs.clear();
}


/*====================
  CEffect::Setup
  ====================*/
void    CEffect::Setup(const tstring &sName, bool bDeferred, bool bPersistent, bool bPausable, bool bUseEntityEffectScale)
{
    m_sEffectName = sName;
    m_bDeferred = bDeferred;
    m_bPersistent = bPersistent;
    m_bPausable = bPausable;
    m_bUseEntityEffectScale = bUseEntityEffectScale;
}


/*====================
  CEffect::SpawnThread
  ====================*/
CEffectThread*  CEffect::SpawnThread(uint uiMilliseconds)
{
    PROFILE("CEffect::SpawnThread");

    if (efx_debug)
        Console.Dev << _T("CEffect::SpawnThread(") << SingleQuoteStr(m_sPath) << _T(", ") << uiMilliseconds << _T(")") << newl;

    if (!m_pEffectThread)
        return nullptr;
    else
        return K2_NEW(ctx_Effects,  CEffectThread)(uiMilliseconds, *m_pEffectThread);
}


/*====================
  CEffect::AddParticleSystemDef
  ====================*/
void    CEffect::AddParticleSystemDef(const tstring &sName, CParticleSystemDef *pParticleSystemDef)
{
    m_mapParticleSystems[sName] = pParticleSystemDef;
}


/*====================
  CEffect::AddModifierDef
  ====================*/
void    CEffect::AddModifierDef(const tstring &sName, CModifierDef *pModifierDef)
{
    m_mapModifiers[sName] = pModifierDef;
}


/*====================
  CEffect::AddEmitterDef
  ====================*/
void    CEffect::AddEmitterDef(const tstring &sName, IEmitterDef *pEmitterDef)
{
    m_mapEmitterDefs[sName] = pEmitterDef;
}



/*====================
  CEffect::SetEffectThread
  ====================*/
void    CEffect::SetEffectThread(CEffectThread *pEffectThread)
{
    SAFE_DELETE(m_pEffectThread);

    m_pEffectThread = pEffectThread;
}


/*====================
  CEffect::GetParticleSystemDef
  ====================*/
CParticleSystemDef* CEffect::GetParticleSystemDef(const tstring &sParticleSystem)
{
    ParticleSystemMap::iterator findit(m_mapParticleSystems.find(sParticleSystem));

    if (findit != m_mapParticleSystems.end())
        return findit->second;
    else
        return nullptr;
}


/*====================
  CEffect::GetModifierDef
  ====================*/
CModifierDef*   CEffect::GetModifierDef(const tstring &sModifier)
{
    ModifierMap::iterator findit(m_mapModifiers.find(sModifier));

    if (findit != m_mapModifiers.end())
        return findit->second;
    else
        return nullptr;
}


/*====================
  CEffect::GetEmitterDef
  ====================*/
IEmitterDef*    CEffect::GetEmitterDef(const tstring &sEmitter)
{
    EmitterDefMap::iterator findit(m_mapEmitterDefs.find(sEmitter));

    if (findit != m_mapEmitterDefs.end())
        return findit->second;
    else
        return nullptr;
}
