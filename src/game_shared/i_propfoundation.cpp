// (C)2006 S2 Games
// i_propfoundation.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_propfoundation.h"
#include "c_teaminfo.h"
#include "c_playercommander.h"

#include "../k2/c_worldentity.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_entitysnapshot.h"
#include "../k2/c_texture.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>* IPropFoundation::s_pvFields;
//=============================================================================

/*====================
  IPropFoundation::CEntityConfig::CEntityConfig
  ====================*/
IPropFoundation::CEntityConfig::CEntityConfig(const tstring &sName) :
IPropEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(Radius, 800.0f)
{
}


/*====================
  IPropFoundation::IPropFoundation
  ====================*/
IPropFoundation::IPropFoundation(CEntityConfig *pConfig) :
IPropEntity(pConfig),
m_pEntityConfig(pConfig),

m_uiBuildingIndex(INVALID_INDEX),
m_unBuildingType(0)
{
}


/*====================
  IPropFoundation::GetTypeVector
  ====================*/
const vector<SDataField>&   IPropFoundation::GetTypeVector()
{
    if (!s_pvFields)
    {
        s_pvFields = K2_NEW(global,   vector<SDataField>)();
        s_pvFields->clear();
        const vector<SDataField> &vBase(IPropEntity::GetTypeVector());
        s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());
        
        s_pvFields->push_back(SDataField(_T("m_uiBuildingIndex"), FIELD_PUBLIC, TYPE_GAMEINDEX));
    }

    return *s_pvFields;
}


/*====================
  IPropFoundation::Baseline
  ====================*/
void    IPropFoundation::Baseline()
{
    IGameEntity::Baseline();

    m_uiBuildingIndex = INVALID_INDEX;
}


/*====================
  IPropFoundation::GetSnapshot
  ====================*/
void    IPropFoundation::GetSnapshot(CEntitySnapshot &snapshot) const
{
    IPropEntity::GetSnapshot(snapshot);

    snapshot.AddGameIndex(m_uiBuildingIndex);
}


/*====================
  IPropFoundation::ReadSnapshot
  ====================*/
bool    IPropFoundation::ReadSnapshot(CEntitySnapshot &snapshot)
{
    try
    {
        if (!IPropEntity::ReadSnapshot(snapshot))
            return false;

        snapshot.ReadNextGameIndex(m_uiBuildingIndex);
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IPropFoundation::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  IPropFoundation::AssignToTeam
  ====================*/
void    IPropFoundation::AssignToTeam(int iTeam)
{
    CEntityTeamInfo *pTeam(Game.GetTeam(iTeam));
    if (pTeam == NULL)
    {
        Console.Warn << _T("IPropFoundation::Spawn() - Team does not exist: ") << iTeam << newl;
        return;
    }

    Console << _T("Adding Building #") << m_uiWorldIndex << _T(" as entity #") << m_uiIndex << _T(" to team ") << iTeam << newl;
    //pTeam->AddTechFoundationIndex(m_uiIndex);
}


/*====================
  IPropFoundation::Spawn
  ====================*/
void    IPropFoundation::Spawn()
{
    SetStatus(ENTITY_STATUS_DORMANT);
    m_hModel = INVALID_RESOURCE;

    if (m_iTeam == -1)
    {
        for (int i(0); i < Game.GetNumTeams(); ++i)
            AssignToTeam(i);
    }
    else
    {
        AssignToTeam(m_iTeam);
    }

    if (Game.IsClient())
    {
        if (GetMinimapIconPath().empty())
            m_hMinimapIcon = INVALID_RESOURCE;
        else
            m_hMinimapIcon = g_ResourceManager.Register(K2_NEW(global,   CTexture)(GetMinimapIconPath(), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
    }
}


/*====================
  IPropFoundation::AddToScene
  ====================*/
bool    IPropFoundation::AddToScene(const CVec4f &v4Color, int iRenderFlags)
{
    // FIXME: Depricated
    return true;
}


/*====================
  IPropFoundation::Copy
  ====================*/
void    IPropFoundation::Copy(const IGameEntity &B)
{
    IPropEntity::Copy(B);

    const IPropEntity *pProp(B.GetAsProp());
    if (pProp == NULL)
        return;
    const IPropFoundation *pFoundation(pProp->GetAsFoundation());
    if (pFoundation == NULL)
        return;

    m_uiBuildingIndex = pFoundation->m_uiBuildingIndex;
}
