// (C)2006 S2 Games
// i_spellitem.h
//
//=============================================================================
#ifndef __I_SPELLITEM_H__
#define __I_SPELLITEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_inventoryitem.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CGameEvent;
class CSpellPolymorph;
//=============================================================================

//=============================================================================
// ISpellItem
//=============================================================================
class ISpellItem : public IInventoryItem
{
protected:
    START_ENTITY_CONFIG(IInventoryItem)
        DECLARE_ENTITY_CVAR(uint, CastTime)
        DECLARE_ENTITY_CVAR(uint, ImpactTime)
        DECLARE_ENTITY_CVAR(tstring, AnimName)
        DECLARE_ENTITY_CVAR(int, AnimChannel)
        DECLARE_ENTITY_CVAR(float, TargetRadius)
        DECLARE_ENTITY_CVAR(tstring, TargetState)
        DECLARE_ENTITY_CVAR(uint, TargetStateDuration)
        DECLARE_ENTITY_CVAR(tstring, CursorPath)
        DECLARE_ENTITY_CVAR(bool, Snapcast)
        DECLARE_ENTITY_CVAR(bool, RadiusCast)
        DECLARE_ENTITY_CVAR(float, Range)
        DECLARE_ENTITY_CVAR(float, MinRange)
        DECLARE_ENTITY_CVAR(float, SnapcastBreakAngle)
        DECLARE_ENTITY_CVAR(CVec3f, SnapcastSelectColor)
        DECLARE_ENTITY_CVAR(tstring, TargetMaterialPath)
        DECLARE_ENTITY_CVAR(tstring, CastEffectPath)
        DECLARE_ENTITY_CVAR(tstring, ImpactEffectPath)
        DECLARE_ENTITY_CVAR(bool, Freeze)
        DECLARE_ENTITY_CVAR(bool, TargetStatusLiving)
        DECLARE_ENTITY_CVAR(bool, TargetStatusDead)
        DECLARE_ENTITY_CVAR(bool, TargetStatusCorpse)
        DECLARE_ENTITY_CVAR(bool, TargetTeamAlly)
        DECLARE_ENTITY_CVAR(bool, TargetTeamEnemy)
        DECLARE_ENTITY_CVAR(bool, TargetTypePlayer)
        DECLARE_ENTITY_CVAR(bool, TargetTypeVehicle)
        DECLARE_ENTITY_CVAR(bool, TargetTypeBuilding)
        DECLARE_ENTITY_CVAR(bool, TargetTypeGadget)
        DECLARE_ENTITY_CVAR(bool, TargetTypeHellbourne)
        DECLARE_ENTITY_CVAR(bool, TargetTypePet)
        DECLARE_ENTITY_CVAR(bool, TargetTypeNPC)
        DECLARE_ENTITY_CVAR(bool, TargetTypeSiege)
        DECLARE_ENTITY_CVAR(float, CastExperience)
        DECLARE_ENTITY_CVAR(tstring, GadgetName)
        DECLARE_ENTITY_CVAR(int, MaxDeployable)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    uint            m_uiTargetIndex;
    CVec3f          m_v3TargetPosition;
    float           m_fManaSpent;

    bool    CreateGadget(const CVec3f &v3Target);

public:
    virtual ~ISpellItem()   {}
    ISpellItem(CEntityConfig *pConfig);

    bool                    IsSpell() const                 { return true; }

    bool                    IsSnapcast() const              { return m_pEntityConfig->GetSnapcast(); }
    ENTITY_CVAR_ACCESSOR(float, Range, 1000.0f)
    ENTITY_CVAR_ACCESSOR(float, MinRange, 0.0f)
    ENTITY_CVAR_ACCESSOR(float, SnapcastBreakAngle, 0.0f)
    ENTITY_CVAR_ACCESSOR(CVec3f, SnapcastSelectColor, WHITE.xyz())
    ENTITY_CVAR_ACCESSOR(tstring, TargetMaterialPath, _T(""))
    ENTITY_CVAR_ACCESSOR(float, TargetRadius, 0.0f)
    ENTITY_CVAR_ACCESSOR(tstring, GadgetName, _T(""))
    ENTITY_CVAR_ACCESSOR(tstring, ImpactEffectPath, _T(""))

    GAME_SHARED_API CVec3f  GetTargetLocation() const;

    virtual bool            TryImpact();
    virtual bool            ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget = true);
    virtual bool            ImpactPosition(const CVec3f &v3target, CGameEvent &evImpact);

    virtual bool            ActivatePrimary(int iButtonStatus);
    virtual bool            ActivateSecondary(int iButtonStatus);
    virtual bool            Cancel(int iButtonStatus);
    virtual bool            IsSpellToggle() const               { return false; }

    GAME_SHARED_API const CSpellPolymorph*      GetAsSpellPolymorph() const;
    GAME_SHARED_API CSpellPolymorph*            GetAsSpellPolymorph();

    GAME_SHARED_API virtual bool    IsValidTarget(IGameEntity *pEntity, bool bImpact);

    static void             ClientPrecache(CEntityConfig *pConfig);
    static void             ServerPrecache(CEntityConfig *pConfig);

    virtual const tstring&  GetTypeName()
    {
        static tstring sSnapcast(_T("Snap-cast spell"));
        static tstring sAreaCast(_T("Area cast spell"));
        static tstring sDeploy(_T("Gadget deployment"));
        if (IsSnapcast())
            return sSnapcast;
        if (!m_pEntityConfig->GetGadgetName().empty())
            return sDeploy;
        return sAreaCast;
    }
};
//=============================================================================

#endif //__I_SPELLITEM_H__
