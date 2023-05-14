// (C)2008 S2 Games
// c_serverinfo.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_serverinfo.h"

#include "../k2/c_snapshot.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR3(ServerInfo, Info_Server)

DEFINE_ENTITY_DESC(CServerInfo)
{
    s_cDesc.pFieldTypes = K2_NEW(global,    TypeVector)();
    s_cDesc.pFieldTypes->clear();
    const TypeVector &vBase(IGameEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_sName"), FIELD_PUBLIC, TYPE_STRING, 0, 0));
}
//=============================================================================

/*====================
  CServerInfo::CServerInfo
  ====================*/
CServerInfo::~CServerInfo()
{
}


/*====================
  CServerInfo::CServerInfo
  ====================*/
CServerInfo::CServerInfo() :
IGameEntity(nullptr)
{
}


/*====================
  CServerInfo::GetSnapshot
  ====================*/
void    CServerInfo::GetSnapshot(CEntitySnapshot &snapshot) const
{
    IGameEntity::GetSnapshot(snapshot);

    snapshot.WriteString(m_sName);
}


/*====================
  CServerInfo::ReadSnapshot
  ====================*/
bool    CServerInfo::ReadSnapshot(CEntitySnapshot &snapshot)
{
    try
    {
        if (!IGameEntity::ReadSnapshot(snapshot))
            EX_ERROR(_T("IGameEntity::ReadSnapshot failed"));

        snapshot.ReadString(m_sName);
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CServerInfo::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  CServerInfo::Baseline
  ====================*/
void    CServerInfo::Baseline()
{
    IGameEntity::Baseline();

    m_sName.clear();
}
