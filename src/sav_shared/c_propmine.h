// (C)2006 S2 Games
// c_propmine.h
//
//=============================================================================
#ifndef __C_PROPMINE_H__
#define __C_PROPMINE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_propfoundation.h"
//=============================================================================

//=============================================================================
// CPropMine
//=============================================================================
class CPropMine : public IPropFoundation
{
private:
    static vector<SDataField>*  s_pvFields;

    DECLARE_ENT_ALLOCATOR2(Prop, Mine);

    uint    m_uiTotalGold;
    uint    m_uiHarvestedGold;
    uint    m_uiHarvestRate;
    uint    m_uiRaidBonus;

public:
    ~CPropMine()    {}
    CPropMine() :
    IPropFoundation(GetEntityConfig()),
    m_uiTotalGold(0),
    m_uiHarvestedGold(0),
    m_uiHarvestRate(0),
    m_uiRaidBonus(0)
    {}

    bool    IsMine() const          { return true; }
    bool    IsSelectable() const    { return true; }

    GAME_SHARED_API static const vector<SDataField>&    GetTypeVector();
    virtual void                    Baseline();
    virtual void                    GetSnapshot(CEntitySnapshot &snapshot) const;
    virtual bool                    ReadSnapshot(CEntitySnapshot &snapshot);

    void                    ApplyWorldEntity(const CWorldEntity &ent);

    GAME_SHARED_API void    Spawn();
    bool                    AddToScene(const CVec4f &v4Color, int iFlags);

    uint                    HarvestGold()
    {
        uint uiAmount(MIN(m_uiHarvestRate, m_uiTotalGold - m_uiHarvestedGold));
        m_uiHarvestedGold += uiAmount;
        return uiAmount;
    }

    uint                    RaidGold()
    {
        uint uiAmount(MIN(m_uiRaidBonus, m_uiTotalGold - m_uiHarvestedGold));
        m_uiHarvestedGold += uiAmount;
        return uiAmount;
    }

    uint                    GetHarvestRate() const          { return m_uiHarvestRate; }
    uint                    GetRemainingGold() const        { return m_uiTotalGold - m_uiHarvestedGold; }
    float                   GetRemainingGoldPercent() const { return GetRemainingGold() / float(m_uiTotalGold); }
    uint                    GetTotalGold() const            { return m_uiTotalGold; }
    uint                    GetRaidBonus() const            { return m_uiRaidBonus; }
    bool                    CanSupportBuilding() const      { return GetRemainingGold() > 0; }

    CSkeleton*              AllocateSkeleton()  { return IPropEntity::AllocateSkeleton(); }

    bool                    IsVisibleOnMinimap() const          { return true; }

    virtual void            Copy(const IGameEntity &B);

    virtual void            Link();
};
//=============================================================================

#endif //__C_PROPMINE_H__
