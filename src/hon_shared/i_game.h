// (C)2006 S2 Games
// i_game.h
//
//=============================================================================
#ifndef __I_GAME_H__
#define __I_GAME_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_world.h"

#include "c_eventdirectory.h"
#include "i_entitydirectory.h"
#include "i_gameentity.h"
#include "c_gameinfo.h"
#include "c_gamemechanics.h"
#include "c_gamemechanicsresource.h"
#include "c_scriptdirectory.h"
#include "game_shared_cvars.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CTeamInfo;
class CClientSnapshot;
class CCamera;
class CStateString;
class CStateBlock;

GAME_SHARED_API EXTERN_CVAR_INT(g_dayStartTime);
GAME_SHARED_API EXTERN_CVAR_INT(g_dayLength);
GAME_SHARED_API EXTERN_CVAR_INT(g_dayTransitionTime);

GAME_SHARED_API EXTERN_CVAR_UINT(g_voteRemakeTimeLimit);
GAME_SHARED_API EXTERN_CVAR_UINT(g_voteAllowConcedeTime);
GAME_SHARED_API EXTERN_CVAR_UINT(g_voteCooldownTime);
GAME_SHARED_API EXTERN_CVAR_UINT(g_voteKickCooldownTime);
GAME_SHARED_API EXTERN_CVAR_UINT(g_voteConcedeUnanimousTime);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define Game (*IGame::GetCurrentGamePointer())

// CASUAL_SETTING
#define CASUAL_SETTING(type, name, cvar) \
type    Get##name() const \
{ \
    if (m_pGameInfo != NULL && m_pGameInfo->HasGameOptions(GAME_OPTION_CASUAL)) \
        return cvar##_Casual; \
    else \
        return cvar; \
}

// CASUAL_ARRAY_SETTING
#define CASUAL_ARRAY_SETTING(type, name, cvar) \
type    Get##name(uint uiIndex) const \
{ \
    if (m_pGameInfo != NULL && m_pGameInfo->HasGameOptions(GAME_OPTION_CASUAL)) \
        return cvar##_Casual[uiIndex]; \
    else \
        return cvar[uiIndex]; \
} \
uint    Get##name##Size() const \
{ \
    if (m_pGameInfo != NULL && m_pGameInfo->HasGameOptions(GAME_OPTION_CASUAL)) \
        return cvar##_Casual.GetSize(); \
    else \
        return cvar.GetSize(); \
}
//=============================================================================

//=============================================================================
// IGame
//=============================================================================
#ifdef __GNUC__
class __attribute__((visibility("default"))) IGame
#else
class IGame
#endif
{
private:
    GAME_SHARED_API static IGame*   s_pGame;

    uint                            m_uiGameTime;
    uint                            m_uiTotalTime; // game time + pause time
    uint                            m_uiFrameLength;
    CWorld*                         m_pWorld;
    IEntityDirectory*               m_pEntityDirectory;

    CEventDirectory                 m_EventDirectory;
    CScriptDirectory                m_ScriptDirectory;

    int                             m_iWinningTeam;
    uint                            m_uiFinalMatchTime;

    uivector                        m_vGlobalModifiers;

    CGameInfo*                      m_pGameInfo;
    ResHandle                       m_hGameMechanics;
    CGameMechanics*                 m_pGameMechanics;
    map<uint, CTeamInfo*>           m_mapTeams;

    struct SWaterMarker
    {
        CVec3f  v3Pos;
        tstring sEffectType;
    };

    vector<SWaterMarker>            m_vWaterMarkers;

    tstring                         m_sTypeName;
    tstring                         m_sResourceCategory;

protected:
    void            SetGameTime(uint uiTime)            { m_uiGameTime = uiTime; }
    void            SetTotalTime(uint uiTime)           { m_uiTotalTime = uiTime; }
    void            SetFrameLength(uint uiTime)         { m_uiFrameLength = uiTime; }

    PlayerMap                       m_mapClients;

    bool                            m_bActiveReflection;
    vector<ushort>                  m_vAutoRecipes;

public:
    virtual ~IGame()    {}
    IGame(const tstring &sTypeName) :
    m_sResourceCategory(_T("world")),
    m_sTypeName(sTypeName),
    m_uiGameTime(0),
    m_pWorld(NULL),
    m_pEntityDirectory(NULL),

    m_iWinningTeam(TEAM_INVALID),
    m_uiFinalMatchTime(INVALID_TIME),
    m_pGameInfo(NULL),
    m_hGameMechanics(INVALID_RESOURCE),
    m_pGameMechanics(NULL),
    m_bActiveReflection(false)
    {}

    static GAME_SHARED_API void         SetCurrentGamePointer(IGame *pGame);
    static IGame*                       GetCurrentGamePointer()             { return s_pGame; }

    void                SetResourceCategory(const tstring &sName)       { m_sResourceCategory = sName; }
    const tstring&      GetResourceCategory() const                     { return m_sResourceCategory; }

    bool                Validate() const;

    const tstring&      GetTypeName() const                 { return m_sTypeName; }

    virtual bool        IsServer() const                    { return false; }
    virtual bool        IsClient() const                    { return false; }

    virtual int         GetLocalClientNum()                 { return -1; }
    virtual CPlayer*    GetLocalPlayer()                    { return NULL; }

    void                AddGlobalModifier(uint uiIndex)     { m_vGlobalModifiers.push_back(uiIndex); }
    const uivector&     GetGlobalModifiers() const          { return m_vGlobalModifiers; }
    void                ClearGlobalModifiers()              { m_vGlobalModifiers.clear(); }

    // Time
    uint                GetGameTime() const                 { return m_uiGameTime; }
    uint                GetTotalTime() const                { return m_uiTotalTime; }
    uint                GetFrameLength() const              { return m_uiFrameLength; }
    uint                GetMatchTime() const;

    float               GetTimeOfDay() const                { return ((MAX<int>(GetMatchTime(), g_dayTransitionTime / 2) + g_dayStartTime) % g_dayLength) / float(g_dayLength); }
    bool                IsNight() const                     { return (GetTimeOfDay() < 0.25f) || (GetTimeOfDay() > 0.75f); }

    // Game info
    void                SetGameInfo(CGameInfo *pGameInfo)   { m_pGameInfo = pGameInfo; }
    CGameInfo*          GetGameInfo() const                 { return m_pGameInfo; }

    void                ClearFlags()                            {if (m_pGameInfo != NULL) m_pGameInfo->ClearFlags(); }
    void                SetFlags(byte yFlags)                   { if (m_pGameInfo != NULL) m_pGameInfo->SetFlags(yFlags); }
    void                RemoveFlags(byte yFlags)                { if (m_pGameInfo != NULL) m_pGameInfo->RemoveFlags(yFlags); }
    bool                HasFlags(byte yFlags) const             { return m_pGameInfo == NULL ? false : m_pGameInfo->HasFlags(yFlags); }

    uint                GetGameMode() const                     { return m_pGameInfo == NULL ? 0 : m_pGameInfo->GetGameMode(); }
    void                SetGameMode(uint uiMode)                { if (m_pGameInfo != NULL) m_pGameInfo->SetGameMode(uiMode); }
    bool                HasGameOptions(uint uiOptions) const    { return m_pGameInfo == NULL ? false : m_pGameInfo->HasGameOptions(uiOptions); }
    void                SetGameOptions(uint uiOptions)          { if (m_pGameInfo != NULL) m_pGameInfo->SetGameOptions(uiOptions); }
    void                ClearGameOptions(uint uiOptions)        { if (m_pGameInfo != NULL) m_pGameInfo->ClearGameOptions(uiOptions); }
    uint                GetTeamSize() const                     { return m_pGameInfo == NULL ? 0 : m_pGameInfo->GetTeamSize(); }
    void                SetTeamSize(uint uiTeamSize)            { if (m_pGameInfo != NULL) m_pGameInfo->SetTeamSize(uiTeamSize); }
    uint                GetCurrentSpectatorCount() const        { return m_pGameInfo == NULL ? 0 : m_pGameInfo->GetCurrentSpectatorCount(); }
    uint                GetCurrentRefereeCount() const          { return m_pGameInfo == NULL ? 0 : m_pGameInfo->GetCurrentRefereeCount(); } 
    uint                GetMaxSpectators() const                { return m_pGameInfo == NULL ? 0 : m_pGameInfo->GetMaxSpectators(); }
    void                SetMaxSpectators(uint uiMaxSpectators)  { if (m_pGameInfo != NULL) m_pGameInfo->SetMaxSpectators(uiMaxSpectators); }
    uint                GetMaxReferees() const                  { return m_pGameInfo == NULL ? 0 : m_pGameInfo->GetMaxReferees(); }
    void                SetMaxReferees(uint uiMaxReferees)      { if (m_pGameInfo != NULL) m_pGameInfo->SetMaxReferees(uiMaxReferees); }
    uint                GetStartingGold() const                 { return m_pGameInfo == NULL ? 0 : m_pGameInfo->GetStartingGold(); }
    bool                GetAlternatePicks() const               { return m_pGameInfo == NULL ? 0 : m_pGameInfo->GetAlternatePicks(); }
    uint                GetBanCount() const                     { return m_pGameInfo == NULL ? 0 : m_pGameInfo->GetBanCount(); }
    uint                GetExtraTime() const                    { return m_pGameInfo == NULL ? 0 : m_pGameInfo->GetExtraTime(); }

    ushort              GetMinPSR() const                       { return m_pGameInfo == NULL ? 0 : m_pGameInfo->GetMinPSR(); }
    ushort              GetMaxPSR() const                       { return m_pGameInfo == NULL ? 0 : m_pGameInfo->GetMaxPSR(); }
    void                SetMinPSR(const ushort unMinPSR)        { if (m_pGameInfo != NULL) m_pGameInfo->SetMinPSR(unMinPSR); }
    void                SetMaxPSR(const ushort unMaxPSR)        { if (m_pGameInfo != NULL) m_pGameInfo->SetMaxPSR(unMaxPSR); }

    uint                GetGamePhase() const                { return m_pGameInfo == NULL ? GAME_PHASE_IDLE : m_pGameInfo->GetGamePhase(); }
    uint                GetPhaseStartTime() const           { return m_pGameInfo == NULL ? INVALID_TIME : m_pGameInfo->GetPhaseStartTime(); }
    uint                GetPhaseDuration() const            { return m_pGameInfo == NULL ? INVALID_TIME : m_pGameInfo->GetPhaseDuration(); }
    uint                GetPhaseEndTime() const             { return m_pGameInfo == NULL ? INVALID_TIME : m_pGameInfo->GetPhaseEndTime(); }
    EServerAccess       GetServerAccess() const             { return m_pGameInfo == NULL ? ACCESS_PUBLIC : m_pGameInfo->GetServerAccess(); }
    byte                GetHostFlags() const                { return m_pGameInfo == NULL ? 0 : m_pGameInfo->GetHostFlags(); }
    uint                GetRemainingPhaseTime() const       { if (m_pGameInfo == NULL) return INVALID_TIME; if (m_uiGameTime >= GetPhaseEndTime()) return 0; return GetPhaseEndTime() - GetServerTime(); }
    void                SetGamePhase(EGamePhase ePhase, uint uiLength = INVALID_TIME, uint uiStartTime = INVALID_TIME)  { if (m_pGameInfo == NULL) return; m_pGameInfo->SetGamePhase(ePhase, uiLength, uiStartTime); }
    void                SetGamePhaseEndTime(uint uiTime)    { if (m_pGameInfo == NULL) return; m_pGameInfo->SetGamePhaseEndTime(uiTime); }

    virtual void        StartMatch()                        {}
    virtual void        EndMatch(int iLosingTeam)           {}

    // Game Mechanics
    virtual ResHandle                   RegisterGameMechanics(ResHandle hMechanics)     { m_hGameMechanics = hMechanics; FetchGameMechanics(); return m_hGameMechanics; }
    GAME_SHARED_API virtual ResHandle   RegisterGameMechanics(const tstring &sPath);
    GAME_SHARED_API bool                FetchGameMechanics();
    CGameMechanics*                     GetGameMechanics() const                        { return m_pGameMechanics; }

    uint                LookupAttackType(const tstring &sName) const        { return m_pGameMechanics ? m_pGameMechanics->LookupAttackType(sName) : INVALID_ATTACK_TYPE; }
    const CAttackType*  GetAttackType(uint uiAttackType) const              { return m_pGameMechanics ? m_pGameMechanics->GetAttackType(uiAttackType) : NULL; }
    const tstring&      GetAttackTypeDisplayName(uint uiAttackType) const   { return m_pGameMechanics ? m_pGameMechanics->GetAttackTypeDisplayName(uiAttackType) : TSNULL; }
    
    uint                LookupCombatType(const tstring &sName) const        { return m_pGameMechanics ? m_pGameMechanics->LookupCombatType(sName) : INVALID_COMBAT_TYPE; }
    float               GetAttackMultiplier(uint uiTypeA, uint uiTypeB)     { return m_pGameMechanics ? m_pGameMechanics->GetAttackMultiplier(uiTypeA, uiTypeB) : 1.0f; }
    float               GetSpellMultiplier(uint uiTypeA, uint uiTypeB)      { return m_pGameMechanics ? m_pGameMechanics->GetSpellMultiplier(uiTypeA, uiTypeB) : 1.0f; }
    int                 GetAggroPriority(uint uiTypeA, uint uiTypeB)        { return m_pGameMechanics ? m_pGameMechanics->GetAggroPriority(uiTypeA, uiTypeB) : 0; }
    int                 GetTargetPriority(uint uiTypeA, uint uiTypeB)       { return m_pGameMechanics ? m_pGameMechanics->GetTargetPriority(uiTypeA, uiTypeB) : 0; }
    int                 GetAttackPriority(uint uiTypeA, uint uiTypeB)       { return m_pGameMechanics ? m_pGameMechanics->GetAttackPriority(uiTypeA, uiTypeB) : 0; }
    int                 GetProximityPriority(uint uiTypeA, uint uiTypeB)    { return m_pGameMechanics ? m_pGameMechanics->GetProximityPriority(uiTypeA, uiTypeB) : 0; }

    uint                LookupStealthType(const tstring &sName) const       { return m_pGameMechanics ? m_pGameMechanics->LookupStealthType(sName) : 0; }
    uint                LookupRevealType(const tstring &sName) const        { return m_pGameMechanics ? m_pGameMechanics->LookupRevealType(sName) : 0; }
    bool                IsRevealed(uint uiStealth, uint uiReveal) const     { return (uiStealth & ~uiReveal) == 0; }
    tstring             GetEffectTypeString(uint uiEffectType) const        { return m_pGameMechanics ? m_pGameMechanics->GetEffectTypeString(uiEffectType) : TSNULL; }

    uint                LookupEffectType(const tstring &sName) const        { return m_pGameMechanics ? m_pGameMechanics->LookupEffectType(sName) : 0; }
    uint                LookupImmunityType(const tstring &sName) const      { return m_pGameMechanics ? m_pGameMechanics->LookupImmunityType(sName) : 0; }
    bool                IsImmune(uint uiEffect, uint uiImmunity) const      { return (uiEffect & uiImmunity) != 0; }
    bool                IsDebuff(uint uiEffect) const                       { return m_pGameMechanics ? m_pGameMechanics->IsDebuff(uiEffect) : false; }
    bool                IsBuff(uint uiEffect) const                         { return m_pGameMechanics ? m_pGameMechanics->IsBuff(uiEffect) : false; }
    bool                IsAssist(uint uiEffect) const                       { return m_pGameMechanics ? m_pGameMechanics->IsAssist(uiEffect) : false; }

    uint                LookupCooldownType(const tstring &sName) const      { return EntityRegistry.RegisterCooldownType(sName); }

    uint                LookupArmorType(const tstring &sName) const                     { return m_pGameMechanics ? m_pGameMechanics->LookupArmorType(sName) : INVALID_ARMOR_TYPE; }
    bool                GetIsArmorEffective(uint uiArmorType, uint uiEffectType) const  { return m_pGameMechanics ? m_pGameMechanics->IsArmorEffective(uiArmorType, uiEffectType) : false; }
    float               GetArmorDamageAdjustment(uint uiArmorType, float fArmor) const  { return m_pGameMechanics ? m_pGameMechanics->GetArmorDamageAdjustment(uiArmorType, fArmor) : 0.0f; }

    uint                    LookupTargetScheme(const tstring &sName) const  { return m_pGameMechanics ? m_pGameMechanics->LookupTargetScheme(sName) : INVALID_TARGET_SCHEME; }
    const CTargetScheme*    GetTargetScheme(uint uiIndex) const             { return m_pGameMechanics ? m_pGameMechanics->GetTargetScheme(uiIndex) : NULL; }
    const tstring&          GetTargetSchemeDisplayName(uint uiIndex) const  { return m_pGameMechanics ? m_pGameMechanics->GetTargetSchemeDisplayName(uiIndex) : TSNULL; }

    byte            LookupPopup(const tstring &sName) const { return m_pGameMechanics ? m_pGameMechanics->LookupPopup(sName) : INVALID_POPUP; }
    const CPopup*   GetPopup(byte yType)                    { return m_pGameMechanics ? m_pGameMechanics->GetPopup(yType) : NULL; }
    const CPopup*   GetPopup(EPopup eType)                  { return m_pGameMechanics ? m_pGameMechanics->GetPopup(eType) : NULL; }
    virtual void    SendPopup(byte yType, IUnitEntity *pSource, IUnitEntity *pTarget = NULL, ushort unValue = 0)    {}
    virtual void    SendPopup(EPopup eType, IUnitEntity *pSource, IUnitEntity *pTarget = NULL, ushort unValue = 0)  {}

    byte            LookupPing(const tstring &sName) const  { return m_pGameMechanics ? m_pGameMechanics->LookupPing(sName) : INVALID_PING; }
    const CPing*    GetPing(byte yType)                     { return m_pGameMechanics ? m_pGameMechanics->GetPing(yType) : NULL; }
    const CPing*    GetPing(EPing eType)                    { return m_pGameMechanics ? m_pGameMechanics->GetPing(eType) : NULL; }
    virtual void    SendPing(byte yType, IUnitEntity *pSource, IUnitEntity *pTarget = NULL, const CVec2f &v2Pos = V2_ZERO)      {}
    virtual void    SendPing(EPing eType, IUnitEntity *pSource, IUnitEntity *pTarget = NULL, const CVec2f &v2Pos = V2_ZERO)     {}

    // World
    void            SetWorldPointer(CWorld *pWorld)                         { m_pWorld = pWorld; }
    CWorld*         GetWorldPointer() const                                 { return m_pWorld; }
    bool            IsWorldLoaded() const                                   { return m_pWorld->IsLoaded(); }
    bool            IsWorldPreloaded(const tstring &sWorldName = TSNULL) const  { return m_pWorld->IsPreloaded(sWorldName); }
    float           GetWorldWidth() const                                   { return m_pWorld->GetWorldWidth(); }
    float           GetWorldHeight() const                                  { return m_pWorld->GetWorldHeight(); }
    uint            AllocateNewWorldEntity(uint uiIndex = INVALID_INDEX)    { return m_pWorld->AllocateNewEntity(uiIndex); }
    virtual void    DeleteWorldEntity(uint uiIndex)                         { m_pWorld->DeleteEntity(uiIndex); }
    bool            WorldEntityExists(uint uiIndex)                         { return m_pWorld->EntityExists(uiIndex); }
    WorldEntList&   GetWorldEntityList()                                    { return m_pWorld->GetEntityList(); }
    WorldLightsMap& GetWorldLightsMap()                                     { return m_pWorld->GetLightsMap(); }
    WorldSoundsMap& GetWorldSoundsMap()                                     { return m_pWorld->GetSoundsMap(); }
    tsmapts&            GetWorldScriptMap()                                     { return m_pWorld->GetScriptMap(); }
    CWorldEntity*   GetWorldEntity(uint uiIndex, bool bThrow = NO_THROW)    { if (m_pWorld == NULL) return NULL; return m_pWorld->GetEntity(uiIndex, bThrow); }
    CWorldLight*    GetWorldLight(uint uiIndex, bool bThrow = NO_THROW)     { if (m_pWorld == NULL) return NULL; return m_pWorld->GetLight(uiIndex, bThrow); }
    CWorldSound*    GetWorldSound(uint uiIndex, bool bThrow = NO_THROW)     { if (m_pWorld == NULL) return NULL; return m_pWorld->GetSound(uiIndex, bThrow); }
    void            LinkEntity(uint uiIndex, uint uiLinkFlags, uint uiSurfFlags) const  { m_pWorld->LinkEntity(uiIndex, uiLinkFlags, uiSurfFlags); }
    void            UnlinkEntity(uint uiIndex)                              { m_pWorld->UnlinkEntity(uiIndex); }
    float           SampleGround(float fX, float fY)                        { return m_pWorld->SampleGround(fX, fY); }
    CVec3f          GetTerrainNormal(float fX, float fY)                    { return m_pWorld->GetTerrainNormal(fX, fY); }
    float           GetTerrainHeight(float fX, float fY)                    { return m_pWorld->GetTerrainHeight(fX, fY); }
    float           GetTerrainHeight(const CVec2f &v2Pos)                   { return m_pWorld->GetTerrainHeight(v2Pos.x, v2Pos.y); }
    CVec3f          GetTerrainPosition(const CVec2f &v2Pos)                 { return CVec3f(v2Pos.x, v2Pos.y, m_pWorld->GetTerrainHeight(v2Pos.x, v2Pos.y)); }
    float           CalcMinTerrainHeight(const CRecti &recArea)             { return m_pWorld->CalcMinHeight(recArea); }
    float           CalcMaxTerrainHeight(const CRecti &recArea)             { return m_pWorld->CalcMaxHeight(recArea); }
    bool            CalcBlocked(const CRecti &recArea)                      { return m_pWorld->CalcBlocked(recArea); }
    float           GetGroundLevel()                                        { return m_pWorld->GetGroundLevel(); }
    float           GetCameraHeight(float fX, float fY)                     { return m_pWorld->GetCameraHeight(fX, fY); }
    CVec3f          GetCameraPosition(const CVec2f &v2Pos)                  { return CVec3f(v2Pos.x, v2Pos.y, m_pWorld->GetCameraHeight(v2Pos.x, v2Pos.y)); }
    inline bool     IsInBounds(float fX, float fY)                          { return m_pWorld->IsInGameBounds(fX, fY); }
    const CRectf&   GetBounds() const                                       { return m_pWorld->GetGameBounds(); }
    const CRectf&   GetCameraBounds() const                                 { return m_pWorld->GetCameraBounds(); }
    inline bool     IsInBounds(int iX, int iY, ESpaceType eSpace) const     { return m_pWorld->IsInBounds(iX, iY, eSpace); }

    void            ResetWorld()                                            { m_pWorld->RestoreWorldEntityList(); m_pWorld->ClearWorldTree(); }

    void            ClearWaterMarkers()                                     { m_vWaterMarkers.clear(); }
    void            AddWaterMarker(const CVec3f &v3Pos, const tstring &sEffectType)
    {
        SWaterMarker sMarker;
        sMarker.v3Pos = v3Pos;
        sMarker.sEffectType = sEffectType;
        m_vWaterMarkers.push_back(sMarker);
    }

    GAME_SHARED_API float   GetWaterLevel(const CVec3f &v3Pos) const;

    GAME_SHARED_API bool    TraceLine(STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, int iIgnoreSurface = 0, uint uiIgnoreEntity = -1);
    GAME_SHARED_API bool    TraceBox(STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, int iIgnoreSurface = 0, uint uiIgnoreEntity = -1);

    void    GetEntitiesInRegion(uivector &vResult, const CBBoxf &bbRegion, uint uiIgnoreSurface)
                { m_pWorld->GetEntitiesInRegion(vResult, bbRegion, uiIgnoreSurface); }
    void    GetEntitiesInRegion(uivector &vResult, const CVec3f &v3Min, const CVec3f &v3Max, uint uiIgnoreSurface)
                { GetEntitiesInRegion(vResult, CBBoxf(v3Min, v3Max), uiIgnoreSurface); }
    void    GetEntitiesInRegion(uivector &vResult, const CRectf &recArea, uint uiIgnoreSurface)
                { GetEntitiesInRegion(vResult, CBBoxf(CVec3f(recArea.lt(), -FAR_AWAY), CVec3f(recArea.rb(), FAR_AWAY)), uiIgnoreSurface); }
    void    GetEntitiesInRegion(uivector &vResult, const CVec2f &v2Min, const CVec2f &v2Max, uint uiIgnoreSurface)
                { GetEntitiesInRegion(vResult, CBBoxf(CVec3f(v2Min, -FAR_AWAY), CVec3f(v2Max, FAR_AWAY)), uiIgnoreSurface); }

    void    GetEntitiesInRadius(uivector &vResult, const CSphere &cRadius, uint uiIgnoreSurface)
                { m_pWorld->GetEntitiesInRadius(vResult, cRadius, uiIgnoreSurface); }
    void    GetEntitiesInRadius(uivector &vResult, const CVec2f &v2Center, float fRadius, uint uiIgnoreSurface)
                { m_pWorld->GetEntitiesInRadius(vResult, v2Center, fRadius, uiIgnoreSurface); }

    void    GetEntitiesInSurface(uivector &vResult, const CConvexPolyhedron &cSurface, uint uiIgnoreSurface)
                { m_pWorld->GetEntitiesInSurface(vResult, cSurface, uiIgnoreSurface); }

    void    GetEntityHandlesInRegion(WorldEntVector &vResult, const CBBoxf &bbRegion, uint uiIgnoreSurface)
                { m_pWorld->GetEntityHandlesInRegion(vResult, bbRegion, uiIgnoreSurface); }

    // Teams
    GAME_SHARED_API void            SetTeam(uint uiTeamID, CTeamInfo *pTeam);
    CTeamInfo*                      GetTeam(uint uiTeamID) const                { map<uint, CTeamInfo*>::const_iterator itFind(m_mapTeams.find(uiTeamID)); if (itFind == m_mapTeams.end()) return NULL; return itFind->second; }
    void                            ClearTeams()                                { m_mapTeams.clear(); }
    const map<uint, CTeamInfo*>&    GetTeams() const                            { return m_mapTeams; }
    void                            RemoveTeam(uint uiTeamID)                   { map<uint, CTeamInfo*>::iterator itFind(m_mapTeams.find(uiTeamID)); if (itFind != m_mapTeams.end()) m_mapTeams.erase(itFind); }

    void    SetWinningTeam(int iTeam)   { m_iWinningTeam = iTeam; }
    int     GetWinningTeam() const      { return m_iWinningTeam; }

    void    SetFinalMatchTime(uint uiTime)  { m_uiFinalMatchTime = uiTime; }
    uint    GetFinalMatchTime() const       { return m_uiFinalMatchTime; }

    virtual ushort  GetVision(float fX, float fY) const                         { return 0; }

    // Entities
    void                    SetEntityDirectory(IEntityDirectory* pDirectory)                        { m_pEntityDirectory = pDirectory; }
    virtual IGameEntity*    AllocateEntity(const tstring &sName, uint uiMinIndex = INVALID_INDEX)   { return NULL; }
    virtual IGameEntity*    AllocateEntity(ushort unType, uint uiMinIndex = INVALID_INDEX)          { return NULL; }

    virtual IGameEntity*    AllocateDynamicEntity(const tstring &sName, uint uiMinIndex, uint uiBaseType)   { return NULL; }
    virtual IGameEntity*    AllocateDynamicEntity(ushort unTypeID, uint uiMinIndex, uint uiBaseType)        { return NULL; }
    template <class T>
    T*  AllocateDynamicEntity(const tstring &sName, uint uiMinIndex = INVALID_INDEX)
    {
        return static_cast<T*>(AllocateDynamicEntity(sName, uiMinIndex, T::GetBaseType()));
    }
    template <class T>
    T*  AllocateDynamicEntity(ushort unTypeID, uint uiMinIndex = INVALID_INDEX)
    {
        return static_cast<T*>(AllocateDynamicEntity(unTypeID, uiMinIndex, T::GetBaseType()));
    }

    virtual void            DeleteEntity(IGameEntity *pEntity)                                      {}
    virtual void            DeleteEntity(uint uiIndex)                                              {}

    IGameEntity*            GetEntity(uint uiIndex)                             { return m_pEntityDirectory->GetEntity(uiIndex); }
    CGameStats*             GetStatsEntity(uint uiIndex)                        { IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsStats()); }
    IVisualEntity*          GetVisualEntity(uint uiIndex)                       { IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsVisual()); }
    IBuildingEntity*        GetBuildingEntity(uint uiIndex)                     { IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsBuilding()); }
    IPropEntity*            GetPropEntity(uint uiIndex)                         { IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsProp()); }
    IUnitEntity*            GetUnitEntity(uint uiIndex)                         { IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsUnit()); }
    IHeroEntity*            GetHeroEntity(uint uiIndex)                         { IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsHero()); }
    IGadgetEntity*          GetGadgetEntity(uint uiIndex)                       { IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsGadget()); }
    IProjectile*            GetProjectile(uint uiIndex)                         { IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsProjectile()); }
    IAffector*              GetAreaAffector(uint uiIndex)                       { IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsAffector()); }
    IEntityState*           GetEntityState(uint uiIndex)                        { IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsState()); }
    IEntityItem*            GetEntityItem(uint uiIndex)                         { IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsItem()); }
    IBitEntity*             GetBitEntity(uint uiIndex)                          { IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsBit()); }
    IVisualEntity*          GetEntityFromWorldIndex(uint uiIndex)               { CWorldEntity *pWorldEnt(GetWorldEntity(uiIndex)); if (pWorldEnt == NULL) return NULL; return GetVisualEntity(pWorldEnt->GetGameIndex()); }
    CPlayer*                GetPlayerFromClientNumber(int iClientNumber)        { PlayerMap_it itClient(m_mapClients.find(iClientNumber)); if (itClient == m_mapClients.end()) return NULL; return itClient->second; }
    IGameEntity*            GetEntityFromUniqueID(uint uiUniqueID)              { return m_pEntityDirectory->GetEntityFromUniqueID(uiUniqueID); }
    IUnitEntity*            GetUnitFromUniqueID(uint uiUniqueID)                { IGameEntity *pEntity(m_pEntityDirectory->GetEntityFromUniqueID(uiUniqueID)); return pEntity == NULL ? NULL : pEntity->GetAsUnit(); }
    uint                    GetGameIndexFromUniqueID(uint uiUniqueID)           { return m_pEntityDirectory->GetGameIndexFromUniqueID(uiUniqueID); }
    uint                    GetGameIndexFromWorldIndex(uint uiIndex)            { CWorldEntity *pWorldEnt(GetWorldEntity(uiIndex)); if (pWorldEnt == NULL) return INVALID_INDEX; return pWorldEnt->GetGameIndex(); }
    IGameEntity*            GetFirstEntity()                                    { return m_pEntityDirectory->GetFirstEntity(); }
    IGameEntity*            GetNextEntity(IGameEntity *pEntity)                 { return m_pEntityDirectory->GetNextEntity(pEntity); }
    IVisualEntity*          GetEntityFromName(const tstring &sName)             { return m_pEntityDirectory->GetEntityFromName(sName); }
    IVisualEntity*          GetNextEntityFromName(IVisualEntity *pEntity)       { return m_pEntityDirectory->GetNextEntityFromName(pEntity); }
    CPlayer*                GetPlayer(int iClientNumber)                        { PlayerMap_it itClient(m_mapClients.find(iClientNumber)); if (itClient == m_mapClients.end()) return NULL; return itClient->second; }
    void                    GetEntities(uivector &vResult, ushort unType)       { m_pEntityDirectory->GetEntities(vResult, unType); }
    const UnitList&         GetUnitList()                                       { return m_pEntityDirectory->GetUnitList(); }
    void                    ActivateBitEntity(uint uiIndex)                     { m_pEntityDirectory->ActivateBitEntity(uiIndex); }
    void                    DeactivateBitEntity(uint uiIndex)                   { m_pEntityDirectory->DeactivateBitEntity(uiIndex); }
    void                    UpdateDefinitions(ushort unType)                    { if (m_pEntityDirectory != NULL) m_pEntityDirectory->UpdateDefinitions(unType); }

    const PlayerMap&        GetPlayerMap() const                                { return m_mapClients; }

    template <class T> T*   GetEntityAs(uint uiIndex)
    {
        IGameEntity *pEntity(GetEntity(uiIndex));
        if (pEntity != NULL && pEntity->GetType() == T::GetEntityType())
            return static_cast<T*>(pEntity);
        else
            return NULL;
    }

    template <class T> T*   GetEntityFromNameAs(const tstring &sName)
    {
        IGameEntity *pEntity(reinterpret_cast<IGameEntity*>(GetEntityFromName(sName))); // TKTK 2023: Tricky forward declaration situation here; remove reinterpret_cast eventually.
        if (pEntity != NULL && pEntity->GetType() == T::GetEntityType())
            return static_cast<T*>(pEntity);
        else
            return NULL;
    }

    template <class T> T*   GetEntityFromUniqueIDAs(uint uiUniqueID)
    {
        IGameEntity *pEntity(GetEntityFromUniqueID(uiUniqueID));
        if (pEntity != NULL && pEntity->GetType() == T::GetEntityType())
            return static_cast<T*>(pEntity);
        else
            return NULL;
    }

    virtual const tstring&  GetEntityString(const tstring &sKey) const          { return sKey; }
    virtual const tstring&  GetEntityString(uint uiIndex) const                 { return TSNULL; }
    virtual uint            GetEntityStringIndex(const tstring &sKey) const     { return INVALID_INDEX; }
    
    // Resources
    virtual ResHandle       RegisterModel(const tstring &sPath) = 0;
    virtual ResHandle       RegisterEffect(const tstring &sPath) = 0;
    virtual ResHandle       RegisterIcon(const tstring &sPath) = 0;
    GAME_SHARED_API virtual ResHandle       RegisterSample(const tstring &sPath);
    GAME_SHARED_API virtual ResHandle       RegisterMaterial(const tstring &sPath);

    GAME_SHARED_API void    Precache(ResHandle hDefinition, EPrecacheScheme eScheme, const tstring &sModifier);
    virtual void            Precache(const tstring &sName, EPrecacheScheme eScheme, const tstring &sModifier) = 0;
    virtual void            Precache(ushort unType, EPrecacheScheme eScheme, const tstring &sModifier) = 0;
    virtual void            GetPrecacheList(const tstring &sName, EPrecacheScheme eScheme, const tstring &sModifier, HeroPrecacheList &deqPrecache) {}

    // Events
    void    ClearEventList()                        { m_EventDirectory.Clear(); }
    uint    AddEvent(const CGameEvent &ev)          { return m_EventDirectory.AddEvent(ev); }
    void    DeleteEvent(uint uiEvent)               { return m_EventDirectory.DeleteEvent(uiEvent); }
    void    GetEventSnapshot(CSnapshot &snapshot)   { m_EventDirectory.GetSnapshot(snapshot); }
    void    EventsFrame()                           { m_EventDirectory.Frame(); }
    void    SynchNewEvents()                        { m_EventDirectory.SynchNewEvents(); }
    void    DeleteRelatedEvents(uint uiIndex)       { m_EventDirectory.DeleteRelatedEvents(uiIndex); }

    // Client
    virtual CClientSnapshot*    GetCurrentSnapshot()                                            { return NULL; }

    virtual bool        IsEntitySelected(uint uiIndex)  { return false; }
    virtual bool        IsEntityHoverSelected(uint uiIndex) { return false; }
    virtual uint        GetActiveControlEntity() { return INVALID_INDEX; }

    virtual void        AddCameraEffectAngleOffset(const CVec3f &v3Angles)      {}
    virtual void        AddCameraEffectOffset(const CVec3f &v3Position)         {}
    virtual CCamera*    GetCamera() const                                       { return NULL; }
    virtual void        AddOverlay(const CVec4f &v4Color)                       {}
    virtual void        AddOverlay(const CVec4f &v4Color, ResHandle hMaterial)  {}

    virtual const tstring&  GetPropType(const tstring &sPath) const             { return TSNULL; }

    virtual void    SendMessage(const tstring &sMsg, int iClientNum)                                    {}
    virtual void    SendGameData(int iClient, const IBuffer &buffer, bool bReliable)                    {}
    virtual void    SendGameData(int iClient, const IBuffer &buffer, bool bReliable, uint uiDelay)      {}
    virtual void    SendReliablePacket(int iClient, const IBuffer &buffer)                              {}
    virtual void    BroadcastGameData(const IBuffer &buffer, bool bReliable, int iExcludeClient = -1, uint uiDelay = 0) {}
    virtual void    BroadcastGameDataToTeam(int iTeam, const IBuffer &buffer, bool bReliable, int iExcludeClient = -1)  {}

    virtual bool    HasMegaCreeps(uint uiTeam)                      { return false; }

    virtual CStateString&   GetStateString(uint uiID) = 0;
    virtual CStateBlock&    GetStateBlock(uint uiID) = 0;

    virtual void            UpdateHeroList()    {}

    virtual uint            GetServerFrame() = 0;
    virtual uint            GetServerTime() const = 0;
    virtual uint            GetPrevServerTime() = 0;
    virtual uint            GetServerFrameLength() = 0;

    // Pathfinding
    virtual PoolHandle      FindPath(const CVec2f &v2Src, float fEntityWidth, uint uiNavigationFlags, const CVec2f &v2Goal, float fGoalRange, vector<PoolHandle> *pBlockers = NULL) const       { return INVALID_POOL_HANDLE; }
    virtual CPath*          AccessPath(PoolHandle hPath) const                                                                          { return NULL; }
    virtual PoolHandle      ClonePath(PoolHandle hPath) const                                                                           { return INVALID_POOL_HANDLE; }
    virtual void            FreePath(PoolHandle hPath) const                                                                            {}
    virtual PoolHandle      BlockPath(uint uiFlags, const CVec2f &v2Position, float fWidth, float fHeight)                              { return INVALID_POOL_HANDLE; }
    virtual void            BlockPath(vector<PoolHandle> &vecBlockers, uint uiFlags, const CConvexPolyhedron &cSurf, float fStepHeight) {}
    virtual void            ClearPath(PoolHandle hBlockerID)                                                                            {}

    virtual int             GetConnectedClientCount(int iTeam = -1)     { return 0; }

    GAME_SHARED_API bool    IsValidTarget(uint uiTargetScheme, uint uiEffectType, const IUnitEntity *pInitiator, const IUnitEntity *pTarget, bool bIgnoreInvulnerable = false);
    GAME_SHARED_API bool    CanTargetTrait(uint uiTargetScheme, ETargetTrait eTrait);
    GAME_SHARED_API uint    GetCooldownEndTime(uint uiTime, uint uiDuration) const;

    GAME_SHARED_API void    SpawnCliff(CWorldEntity *pWorldEnt);
    GAME_SHARED_API void    SpawnWater(CWorldEntity *pWorldEnt);
    GAME_SHARED_API void    SpawnStaticProp(CWorldEntity *pWorldEnt);

    void                    AddOccludeRegion(const CVec3f &v3Pos, float fRadius) const      { m_pWorld->AddOccludeRegion(v3Pos, fRadius); }
    void                    RemoveOccludeRegion(const CVec3f &v3Pos, float fRadius) const   { m_pWorld->RemoveOccludeRegion(v3Pos, fRadius); }

    void                SetActiveReflection(bool bActiveReflection) { m_bActiveReflection = bActiveReflection; }
    bool                GetActiveReflection() const                 { return m_bActiveReflection; }
    const vector<ushort>&   GetAutoRecipeList() const               { return m_vAutoRecipes; }

    virtual void            UnitRespawned(uint uiIndex)                                                                 {}
    virtual void            UnitKilled(uint uiIndex)                                                                    {}
    virtual void            UpdateUnitVisibility(IUnitEntity *pUnit)                                                    {}

    // Game log
    virtual void    LogPlayer(EGameLogEvent eEvent, CPlayer *pPlayer)                                                                   {}
    virtual void    LogKill(IUnitEntity *pTarget, IUnitEntity *pAttacker, IGameEntity *pInflictor = NULL, ivector *pAssists = NULL)     {}
    virtual void    LogAssist(IUnitEntity *pTarget, IUnitEntity *pAttacker, IGameEntity *pInflictor, CPlayer *pPlayer)                  {}
    virtual void    LogDamage(IUnitEntity *pTarget, int iPlayer, ushort unAttackerType, ushort unInflictorType, float fDamage)          {}
    virtual void    LogDeny(IUnitEntity *pTarget, IUnitEntity *pAttacker, IGameEntity *pInflictor, float fExperience, ushort unGold)    {}
    virtual void    LogExperience(EGameLogEvent eEvent, IUnitEntity *pTarget, IUnitEntity *pSource, float fExperience)                  {}
    virtual void    LogGold(EGameLogEvent eEvent, CPlayer *pPlayer, IUnitEntity *pSource, ushort unGold)                                {}
    virtual void    LogHero(EGameLogEvent eEvent, IHeroEntity *pUnit, const tstring &sParamA = TSNULL)                                  {}
    virtual void    LogAward(EGameLogEvent eEvent, IUnitEntity *pAttacker, IUnitEntity *pTarget, ushort unGold = 0)                     {}
    virtual void    LogItem(EGameLogEvent eEvent, IEntityItem *pItem, IUnitEntity *pTarget = NULL)                                      {}
    virtual void    LogAbility(EGameLogEvent eEvent, IEntityAbility *pAbility, IUnitEntity *pTarget = NULL)                             {}

    virtual bool    UsePlayerColors()                                                                                                   { return true; }
    virtual bool    UseHeroIndicators()                                                                                                 { return true; }

    GAME_SHARED_API bool    CanLeave(uint uiTeam) const;
    GAME_SHARED_API bool    IsGameOver() const;

    GAME_SHARED_API void    SwapItem(int iClientNum, IEntityItem *pItem0, IEntityItem *pItem1);

    // Client requests
    virtual void    RequestChangeTeam(int iClientID, uint uiTeamID, uint uiSlot = uint(-1))                     {}
    virtual void    ChangeTeam(int iClientID, uint uiTeamID, uint uiSlot = uint(-1))                            {}
    virtual bool    PurchaseItem(int iClientNum, uint uiUnitIndex, ushort unShop, int iSlot)                    { return false; }
    virtual void    SellItem(int iClientNum, uint uiUnitIndex, int iSlot)                                       {}
    virtual void    MoveItem(int iClientNum, uint uiUnitIndex, int iSlot0, int iSlot1)                          {}
    virtual bool    BuyBackHero(int iClientNum, uint uiUnitIndex)                                               { return false; }
    virtual void    SetSelection(int iClientNum, const uiset &setSelection)                                     {}

    CScriptThread*  SpawnThread(const tstring &sName, uint uiTime)      { return m_ScriptDirectory.SpawnThread(sName, uiTime); }
    void            ScriptFrame()                                       { return m_ScriptDirectory.Frame(); }

    virtual bool    CanAccessAltAvatar(const tstring &sHero, const tstring &sAltAvatar)                         { return false; }

    CASUAL_SETTING(uint, HeroRespawnTimePerLevel, hero_respawnTimePerLevel);
    CASUAL_SETTING(uint, HeroGoldLossPerLevel, hero_goldLossPerLevel);
    CASUAL_SETTING(uint, HeroBuyBackCost, hero_buyBackCost);
    CASUAL_SETTING(uint, HeroBuyBackCostPerLevel, hero_buyBackCostPerLevel);
    CASUAL_SETTING(float, HeroBuyBackCostScale, hero_buyBackCostScale);

    CASUAL_SETTING(uint, HeroGoldBounty, hero_goldBounty);
    CASUAL_SETTING(uint, HeroGoldBountyPerLevel, hero_goldBountyPerLevel);
    CASUAL_SETTING(uint, HeroGoldBountyPerStreak, hero_goldBountyPerStreak);
    CASUAL_SETTING(uint, HeroGoldBountyMinStreak, hero_goldBountyMinStreak);
    CASUAL_SETTING(uint, HeroGoldBountyMaxStreak, hero_goldBountyMaxStreak);
    CASUAL_SETTING(uint, HeroGoldBountyFirstBlood, hero_goldBountyFirstBlood);
    CASUAL_SETTING(uint, HeroGoldBountyRadiusBase, hero_goldBountyRadiusBase);
    CASUAL_SETTING(uint, HeroGoldBountyRadiusPerLevel, hero_goldBountyRadiusPerLevel);
    CASUAL_SETTING(float, HeroGoldBountyAssistPercent, hero_goldBountyAssistPercent);
    CASUAL_SETTING(float, HeroExpUnsharedBountyPerLevel, hero_expUnsharedBountyPerLevel);
    CASUAL_SETTING(uint, HeroMaxLevel, hero_maxLevel);
    CASUAL_ARRAY_SETTING(float, HeroExperienceTable, hero_experienceTable);

    CASUAL_SETTING(float, ExperienceRange, g_experienceRange);
    CASUAL_SETTING(bool, TowerHeroArmorReduction, g_towerHeroArmorReduction);
};
//=============================================================================


/*====================
  IGame::Validate
  ====================*/
inline
bool    IGame::Validate() const
{
    try
    {
        if (m_pWorld == NULL)
            EX_ERROR(_T("Invalid world pointer"));

        if (m_pEntityDirectory == NULL)
            EX_ERROR(_T("Invalid entity directory pointer"));

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IGame::Validate() - "));
        return false;
    }
}


/*====================
  IGame::GetMatchTime
  ====================*/
inline
uint    IGame::GetMatchTime() const
{
    if (GetGameTime() == INVALID_TIME || GetGamePhase() != GAME_PHASE_ACTIVE)
        return 0;
    else
        return MAX<int>(int(GetGameTime()) - int(GetPhaseStartTime()), 0);
}

#endif //__I_GAME_H__
