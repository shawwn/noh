// (C)2006 S2 Games
// c_projectilenpcshot.h
//
//=============================================================================
#ifndef __C_PROJECTILENPCSHOT_H__
#define __C_PROJECTILENPCSHOT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_projectile.h"
//=============================================================================

//=============================================================================
// CProjectileNpcShot
//=============================================================================
class CProjectileNpcShot : public IProjectile
{
private:
    static vector<SDataField>   *s_pvFields;

    DECLARE_ENT_ALLOCATOR2(Projectile, NpcShot);

protected:
    EffectVector    m_vSourceEffects;
    EffectVector    m_vTargetEffects;
    float           m_fEffectRadius;
    ResHandle       m_hDeathEffect;

public:
    ~CProjectileNpcShot()   {}
    CProjectileNpcShot();

    // Network
    GAME_SHARED_API virtual void    Baseline();
    GAME_SHARED_API virtual void    GetSnapshot(CEntitySnapshot &snapshot) const;
    GAME_SHARED_API virtual bool    ReadSnapshot(CEntitySnapshot &snapshot);

    static const vector<SDataField>&    GetTypeVector();

    void    SetEffectRadius(float fEffectRadius)    { m_fEffectRadius = fEffectRadius; }
    void    SetDeathEffect(ResHandle hDeathEffect)  { m_hDeathEffect = hDeathEffect; }

    void    Killed();

    void    AddSourceEffect(const CNpcAbilityEffect &cEffect)
    {
        m_vSourceEffects.push_back(cEffect);
    }

    void    AddTargetEffect(const CNpcAbilityEffect &cEffect)
    {
        m_vTargetEffects.push_back(cEffect);
    }
};
//=============================================================================

#endif //__C_PROJECTILENPCSHOT_H__
