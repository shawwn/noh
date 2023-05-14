// (C)2007 S2 Games
// i_particlesystem.h
//
//=============================================================================
#ifndef __I_EFFECTINSTANCE_H__
#define __I_EFFECTINSTANCE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_emitter.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CEffectThread;
class CSkeleton;
//=============================================================================

//=============================================================================
// IEffectInstance
//=============================================================================
class IEffectInstance
{
protected:
    CEffectThread   *m_pEffectThread;

public:
    virtual ~IEffectInstance();
    IEffectInstance();

    IEffectInstance(CEffectThread *pEffectThread);

    virtual bool    IsParticleSystem() const        { return false; }
    virtual bool    IsModifier() const              { return false; }

    virtual bool    Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace = nullptr) = 0;
    virtual bool    IsDead() = 0;

    bool                    GetActive() const;

    virtual void            Expire(uint uiMilliseconds) = 0;
    bool                    GetExpire() const;

    CEffect*                GetEffect();
    
    CEffectThread*          GetEffectThread() const { return m_pEffectThread; }
};
//=============================================================================
#endif  //__I_EFFECTINSTANCE_H__
