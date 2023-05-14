// (C)2006 S2 Games
// c_particlesystem.h
//
//=============================================================================
#ifndef __C_PARTICLESYSTEM_H__
#define __C_PARTICLESYSTEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_effectinstance.h"
#include "i_emitter.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CEffectThread;
class CSkeleton;
class CCamera;
class CWorld;

typedef vector<IEmitterDef *>               EmitterDefList;
typedef vector<IEmitter *>                  EmitterList;
typedef hash_map<tstring, IEmitter *>       NamedEmitterMap;

enum    ESystemSpace
{
    WORLD_SPACE = 0,
    BONE_SPACE,
    ENTITY_SPACE
};
//=============================================================================

//=============================================================================
// CParticleSystemDef
//=============================================================================
class CParticleSystemDef
{
private:
    EmitterDefList      m_vEmitterDefs;
    ESystemSpace        m_eSpace;
    float               m_fScale;
    CVec3f              m_v3Color;

public:
    ~CParticleSystemDef();
    CParticleSystemDef
    (
        ESystemSpace eSpace,
        float fScale,
        const CVec3f &v3Color
    );

    void    AddEmitterDef(IEmitterDef *pEmitterDef);

    const EmitterDefList&   GetEmitterDefs() const  { return m_vEmitterDefs; }
    ESystemSpace            GetSpace() const        { return m_eSpace; }
    float                   GetScale() const        { return m_fScale; }
    const CVec3f&           GetColor() const        { return m_v3Color; }
};
//=============================================================================

//=============================================================================
// CParticleSystem
//=============================================================================
class CParticleSystem : public IEffectInstance
{
private:
    EmitterList     m_vEmitters;
    NamedEmitterMap m_mapNamedEmitters;
    uint            m_uiStartTime;
    uint            m_uiLastUpdateTime;
    ESystemSpace    m_eSpace;
    float           m_fScale;
    CVec3f          m_v3Color;

    CVec3f          m_v3CustomPos;
    CAxis           m_aCustomAxis;
    float           m_fCustomScale;

public:
    ~CParticleSystem();
    CParticleSystem();

    CParticleSystem(uint uiStartTime, CEffectThread *pEffectThread, const CParticleSystemDef &psSettings);

    virtual bool            IsParticleSystem() const        { return true; }

    K2_API bool             Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace = nullptr);
    K2_API bool             UpdatePaused(uint uiMilliseconds);
    K2_API bool             IsDead();

    const EmitterList&      GetEmitterList() const  { return m_vEmitters; }
    IEmitter*               GetEmitter(const tstring &sName);
    ESystemSpace            GetSpace() const        { return m_eSpace; }
    float                   GetScale() const        { return m_fScale; }
    const CVec3f&           GetColor() const        { return m_v3Color; }

    K2_API CVec3f           GetSourceBonePosition(const tstring &sBone, uint uiTime);
    K2_API void             GetSourceBoneAxisPos(const tstring &sBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos);
    K2_API matrix43_t*      GetSourceBoneTransform(const tstring &sBone);
    K2_API bool             GetSourceVisibility();
    K2_API bool             GetSourceVisibility(const tstring &sBone);
    K2_API CAxis            SourceTransformAxis(const CAxis &aAxis);
    K2_API CVec3f           SourceTransformPosition(const CVec3f &v3Pos);
    K2_API CVec3f           GetSourceMeshPosition(const tstring &sMesh);
    K2_API CVec3f           GetSourceRandomPositionOnMesh(const tstring &sMesh);
    K2_API CVec3f           GetSourceRandomPositionWithNormalOnMesh(const tstring &sMesh, CVec3f &v3Dir);
    K2_API CVec3f           GetSourceRandomPositionOnMesh(const tstring &sMesh, const CVec3f &v3Dir);
    K2_API CVec3f           GetSourceRandomPositionOnSkeleton();
    K2_API const CVec3f&    GetSourcePosition() const;
    K2_API const CAxis&     GetSourceAxis() const;
    K2_API float            GetSourceScale() const;
    K2_API CSkeleton*       GetSourceSkeleton() const;
    K2_API CModel*          GetSourceModel() const;
    
    K2_API CVec3f           GetTargetBonePosition(const tstring &sBone, uint uiTime);
    K2_API void             GetTargetBoneAxisPos(const tstring &sBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos);
    K2_API matrix43_t*      GetTargetBoneTransform(const tstring &sBone);
    K2_API bool             GetTargetVisibility();
    K2_API bool             GetTargetVisibility(const tstring &sBone);
    K2_API CAxis            TargetTransformAxis(const CAxis &aAxis);
    K2_API CVec3f           TargetTransformPosition(const CVec3f &v3Pos);
    K2_API CVec3f           GetTargetMeshPosition(const tstring &sMesh);
    K2_API CVec3f           GetTargetRandomPositionOnMesh(const tstring &sMesh);
    K2_API CVec3f           GetTargetRandomPositionWithNormalOnMesh(const tstring &sMesh, CVec3f &v3Dir);
    K2_API CVec3f           GetTargetRandomPositionOnMesh(const tstring &sMesh, const CVec3f &v3Dir);
    K2_API CVec3f           GetTargetRandomPositionOnSkeleton();
    K2_API const CVec3f&    GetTargetPosition() const;
    K2_API const CAxis&     GetTargetAxis() const;
    K2_API float            GetTargetScale() const;
    K2_API CSkeleton*       GetTargetSkeleton() const;
    K2_API CModel*          GetTargetModel() const;

    K2_API CVec3f           GetCustomBonePosition(CSkeleton *pSkeleton, const tstring &sBone, uint uiTime);
    K2_API bool             GetCustomVisibility(CSkeleton *pSkeleton, const tstring &sBone);
    K2_API void             GetCustomBoneAxisPos(CSkeleton *pSkeleton, const tstring &sBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos);

    K2_API CCamera*         GetCamera();
    K2_API CWorld*          GetWorld();

    K2_API CBBoxf           GetBounds() const;
    
    K2_API bool             GetCustomVisibility() const;

    void                    Expire(uint uiMilliseconds);

    void                    SetCustomPos(const CVec3f &v3Pos)                       { m_v3CustomPos = v3Pos; }
    const CVec3f&           GetCustomPos() const                                    { return m_v3CustomPos; }

    void                    SetCustomAxis(const CAxis &aAxis)                       { m_aCustomAxis = aAxis; }
    const CAxis&            GetCustomAxis() const                                   { return m_aCustomAxis; }

    void                    SetCustomScale(float fScale)                            { m_fCustomScale = fScale; }
    float                   GetCustomScale() const                                  { return m_fCustomScale; }
};
//=============================================================================

#endif  //__C_PARTICLESYSTEM_H__
