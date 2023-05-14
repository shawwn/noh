// (C)2007 S2 Games
// c_replaymanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_replaymanager.h"
#include "i_game.h"
#include "c_teaminfo.h"
#include "c_gameinfo.h"
#include "i_heroentity.h"

#include "../k2/k2_protocol.h"
#include "../k2/c_statestring.h"
#include "../k2/c_stateblock.h"
#include "../k2/c_uicmd.h"
#include "../k2/c_entitysnapshot.h"
#include "../k2/c_transmitflags.h"
#include "../k2/c_xmldoc.h"
#include "../k2/c_uimanager.h"
#include "../k2/c_eventmanager.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_resourceinfo.h"
#include "../k2/c_texture.h"

#undef pReplayManager
#undef ReplayManager
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
EXTERN_CMD(PrintGameInfo);
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CReplayManager  *pReplayManager(CReplayManager::GetInstance());

SINGLETON_INIT(CReplayManager)

CVAR_BOOLF      (replay_isPlaying,          false,      CVAR_READONLY);
CVAR_BOOL       (replay_pause,              false);
CVAR_BOOL       (replay_autoSkipPause,      false);
//=============================================================================

/*====================
  CReplayManager::~CReplayManager
  ====================*/
CReplayManager::~CReplayManager()
{
    StopPlayback();
    StopRecording();
    
    SAFE_DELETE_SNAPSHOT(m_hLastSnapshot);
}


/*====================
  CReplayManager::CReplayManager
  ====================*/
CReplayManager::CReplayManager() : 
m_sFilename(_T("")),
m_bPlaying(false),
m_bRecording(false),
m_bFrameOpen(false),
m_iCurrentFrame(-1),
m_uiBeginTime(INVALID_TIME),
m_uiEndTime(INVALID_TIME),
m_iSpeed(0),
m_uiReplayVersion(REPLAY_VERSION),
m_uiNumBitEntityFields(0),
m_hLastSnapshot(INVALID_POOL_HANDLE),
m_uiNextUniqueID(0),
m_bFirstFrame(true)
{
}


/*====================
  CReplayManager::StartRecording
  ====================*/
void    CReplayManager::StartRecording(const tstring &sFilename)
{
    if (IsPlaying() && Host.HasServer())
        return;

    m_bFrameOpen = false;

    m_hReplayData.Open(Filename_StripExtension(sFilename) + _T(".tmp"), FILE_WRITE | FILE_BINARY | FILE_UTF16);

    if (m_hReplayData.IsOpen())
        m_bRecording = true;
    else
        return;

    m_sFilename = sFilename;

    // File type code
    m_hReplayData.WriteByte('S');
    m_hReplayData.WriteByte('2');
    m_hReplayData.WriteByte('R');
    m_hReplayData.WriteByte('2');

    // Version
    m_hReplayData.WriteInt32(REPLAY_VERSION);

    // Game Version
    tsvector vsVersion(TokenizeString(K2System.GetVersionString(), '.'));
    m_hReplayData.WriteByte(byte(AtoI(vsVersion[0])));
    m_hReplayData.WriteByte(byte(AtoI(vsVersion[1])));
    m_hReplayData.WriteByte(byte(AtoI(vsVersion[2])));
    m_hReplayData.WriteByte(byte(AtoI(vsVersion[3])));

    // Write base entity type definitions
    EntAllocatorIDMap &mapAllocators(EntityRegistry.GetAllocators());
    DynamicEntityTypeIDMap &mapDynamicAllocators(EntityRegistry.GetDynamicAllocators());

    // Write static allocators
    m_hReplayData.WriteInt32(int(mapAllocators.size()));

    for (EntAllocatorIDMap_it it(mapAllocators.begin()), itEnd(mapAllocators.end()); it != itEnd; ++it)
    {
        // TypeID
        m_hReplayData.WriteInt16(it->first);
        
        // TypeName
        const string &sTypeName(TStringToUTF8(it->second->GetName()));

        m_hReplayData.Write(sTypeName.c_str(), sTypeName.length());
        m_hReplayData.WriteByte(byte(0));

        // Version
        m_hReplayData.WriteInt32(it->second->GetVersion());

        const SEntityDesc *pDesc(it->second->GetTypeDesc());

        // NumFields
        m_hReplayData.WriteByte(byte(pDesc->pFieldTypes->size()));

        const TypeVector *pFieldTypes(it->second->GetTypeVector());
        for (TypeVector::const_iterator cit(pFieldTypes->begin()), citEnd(pFieldTypes->end()); cit != citEnd; ++cit)
        {
            // SDataField::sName
            const string &sFieldName(TStringToUTF8(cit->sName));

            m_hReplayData.Write(sFieldName.c_str(), sFieldName.length());
            m_hReplayData.WriteByte(byte(0));

            // SDataField::eDataType
            m_hReplayData.WriteByte(byte(cit->eDataType));

            // SDataField::uiBits
            m_hReplayData.WriteInt32(cit->uiBits);

            // SDataField::iParam
            m_hReplayData.WriteInt32(cit->iParam);
        }

        CBufferBit cBuffer;
        uint uiLastIndex(-2);

        it->second->GetBaseline()->WriteBuffer(cBuffer, false, uiLastIndex, false);

        m_hReplayData.WriteInt16(ushort(cBuffer.GetLength()));
        m_hReplayData.Write(cBuffer.Get(0), cBuffer.GetLength());
    }

    // Generate a map of all base allocators
    map<uint, const IBaseEntityAllocator *> mapBaseTypeAllocators;
    map<uint, uint> mapBaseTypeID;

    uint uiNextBaseTypeID(0);

    for (DynamicEntityTypeIDMap_it it(mapDynamicAllocators.begin()), itEnd(mapDynamicAllocators.end()); it != itEnd; ++it)
    {
        uint uiBaseType(it->second.GetBaseType());

        map<uint, uint>::iterator itFind(mapBaseTypeID.find(uiBaseType));
        if (itFind != mapBaseTypeID.end())
            continue;

        mapBaseTypeID[uiBaseType] = uiNextBaseTypeID;
        mapBaseTypeAllocators[uiNextBaseTypeID] = it->second.GetBaseAllocator();

        ++uiNextBaseTypeID;
    }

    // Write base dynamic allocators
    m_hReplayData.WriteInt32(int(mapBaseTypeAllocators.size()));

    for (map<uint, const IBaseEntityAllocator *>::iterator it(mapBaseTypeAllocators.begin()), itEnd(mapBaseTypeAllocators.end()); it != itEnd; ++it)
    {
        // MikeG Crash Fix
        if (!it->second)
            continue;

        // TypeID
        m_hReplayData.WriteInt16(short(it->first));
        
        // TypeName
        const string &sTypeName(TStringToUTF8(it->second->GetBaseTypeName()));

        m_hReplayData.Write(sTypeName.c_str(), sTypeName.length());
        m_hReplayData.WriteByte(byte(0));

        // Version
        m_hReplayData.WriteInt32(it->second->GetVersion());

        const SEntityDesc *pDesc(it->second->GetTypeDesc());

        // NumFields
        m_hReplayData.WriteByte(byte(pDesc->pFieldTypes->size()));

        const TypeVector *pFieldTypes(it->second->GetTypeVector());
        for (TypeVector::const_iterator cit(pFieldTypes->begin()), citEnd(pFieldTypes->end()); cit != citEnd; ++cit)
        {
            // SDataField::sName
            const string &sFieldName(TStringToUTF8(cit->sName));

            m_hReplayData.Write(sFieldName.c_str(), sFieldName.length());
            m_hReplayData.WriteByte(byte(0));

            // SDataField::eDataType
            m_hReplayData.WriteByte(byte(cit->eDataType));

            // SDataField::uiBits
            m_hReplayData.WriteInt32(cit->uiBits);

            // SDataField::iParam
            m_hReplayData.WriteInt32(cit->iParam);
        }

        CBufferBit cBuffer;
        uint uiLastIndex(-2);

        it->second->GetBaseline()->WriteBuffer(cBuffer, false, uiLastIndex, false);

        m_hReplayData.WriteInt16(ushort(cBuffer.GetLength()));
        m_hReplayData.Write(cBuffer.Get(0), cBuffer.GetLength());
    }

    // Write dynamic allocator references
    m_hReplayData.WriteInt32(int(mapDynamicAllocators.size()));

    for (DynamicEntityTypeIDMap_it it(mapDynamicAllocators.begin()), itEnd(mapDynamicAllocators.end()); it != itEnd; ++it)
    {
        uint uiBaseType(it->second.GetBaseType());

        map<uint, uint>::iterator itFind(mapBaseTypeID.find(uiBaseType));
        if (itFind == mapBaseTypeID.end())
            continue; // Should never happen

        // TypeID
        m_hReplayData.WriteInt16(short(it->first));

        // BaseTypeID
        m_hReplayData.WriteInt16(short(itFind->second));
    }

    // Write world name as a utf8 string
    const string &sWorldName(TStringToUTF8(Host.GetServerWorldName()));
    m_hReplayData.Write(sWorldName.c_str(), sWorldName.length());
    m_hReplayData.WriteByte(byte(0));

    // Write state strings
    uint uiNumStateStrings(STATE_STRING_NUM_RESERVED);
    m_hReplayData.WriteInt32(uiNumStateStrings);

    for (uint uiID(1); uiID < uiNumStateStrings; ++uiID)
    {
        string sBuffer;
        Game.GetStateString(uiID).AppendToBuffer(sBuffer);

        m_hReplayData.Write(sBuffer.c_str(), sBuffer.length());
        m_hReplayData.WriteByte(byte(0));
    }

    // Write state blocks
    uint uiNumStateBlocks(STATE_BLOCK_LAST_HERO_GROUP);
    m_hReplayData.WriteInt32(uiNumStateBlocks);

    for (uint uiID(1); uiID < uiNumStateBlocks; ++uiID)
    {
        CBufferDynamic &cBuffer(Game.GetStateBlock(uiID).GetBuffer());

        m_hReplayData.WriteInt32(cBuffer.GetLength());

        if (cBuffer.GetLength() > 0)
            m_hReplayData.Write(cBuffer.Get(), cBuffer.GetLength());
    }

    m_mapStateStrings.clear();
    m_mapStateBlocks.clear();

    m_cCurrentSnapshot = CSnapshot();
    SAFE_DELETE_SNAPSHOT(m_hLastSnapshot);
}


/*====================
  CReplayManager::StopRecording
  ====================*/
void    CReplayManager::StopRecording()
{
    if (!IsRecording())
        return;

    m_hReplayData.Close();
    m_bRecording = false;
    m_bFrameOpen = false;

    m_cCurrentSnapshot = CSnapshot();
    SAFE_DELETE_SNAPSHOT(m_hLastSnapshot);

    m_hReplayData.Open(Filename_StripExtension(m_sFilename) + _T(".tmp"), FILE_READ | FILE_BINARY | FILE_UTF16);

    if (m_hReplayData.IsOpen())
    {
        tstring sPath(m_sFilename);
        CArchive cArchive(sPath, ARCHIVE_WRITE | ARCHIVE_MAX_COMPRESS);
        if (!cArchive.IsOpen())
            return;

        uint uiSize;
        const char *pBuffer(m_hReplayData.GetBuffer(uiSize));

        cArchive.WriteFile(_T("ReplayData"), pBuffer, uiSize);

        m_hReplayData.Close();

        CGameInfo *pGameInfo(Game.GetGameInfo());
        if (pGameInfo != NULL)
        {
            tstring sHost;
            const PlayerMap &mapPlayers(Game.GetPlayerMap());
            for (PlayerMap_cit it(mapPlayers.begin()); it != mapPlayers.end(); ++it)
            {
                if (it->second->HasFlags(PLAYER_FLAG_HOST))
                {
                    if (!sHost.empty())
                        sHost += _T(",");
                    sHost += XtoA(it->second->GetAccountID());
                }
            }

            CXMLDoc xmlInfo(XML_ENCODE_UTF8);

            xmlInfo.NewNode("replayinfo");
                xmlInfo.AddProperty("version", K2System.GetVersionString());
                xmlInfo.AddProperty("gamemode", CGameInfo::GetGameModeName(pGameInfo->GetGameMode()));
                xmlInfo.AddProperty("gameoptions", CGameInfo::GetGameOptionsNamesString(pGameInfo->GetGameOptions()));
                xmlInfo.AddProperty("servername", pGameInfo->GetServerName());
                xmlInfo.AddProperty("mapname", Game.GetWorldPointer()->GetName());
                xmlInfo.AddProperty("mapfancyname", Game.GetWorldPointer()->GetFancyName());
                xmlInfo.AddProperty("teamsize", XtoA(pGameInfo->GetTeamSize()));
                xmlInfo.AddProperty("gamename", pGameInfo->GetGameName());

                if (pGameInfo->GetMatchID() == uint(-1))
                    xmlInfo.AddProperty("matchid", XtoA(0));
                else
                    xmlInfo.AddProperty("matchid", XtoA(pGameInfo->GetMatchID()));

                xmlInfo.AddProperty("host", sHost);
                
                CTeamInfo *pWinner(Game.GetTeam(Game.GetWinningTeam()));
                xmlInfo.AddProperty("winner", pWinner != NULL ? pWinner->GetName() : TSNULL);
                
                xmlInfo.AddProperty("date", pGameInfo->GetServerDate());
                xmlInfo.AddProperty("time", pGameInfo->GetServerTime());
                xmlInfo.AddProperty("matchlength", pGameInfo->GetMatchLength());

                for (PlayerMap_cit it(mapPlayers.begin()); it != mapPlayers.end(); ++it)
                {
                    CPlayer *pPlayer(it->second);
                    IHeroEntity *pHero(it->second->GetHero());

                    xmlInfo.NewNode("player");

                        xmlInfo.AddProperty("name", it->second->GetName());
                        xmlInfo.AddProperty("accountid", XtoA(it->second->GetAccountID()));
                        xmlInfo.AddProperty("team", XtoA(it->second->GetTeam()));
                        xmlInfo.AddProperty("teamindex", XtoA(it->second->GetTeamIndex()));

                        if (pHero != NULL)
                        {
                            xmlInfo.AddProperty("heroname", pHero->GetTypeName());
                            xmlInfo.AddProperty("heroicon", pHero->GetIconPath());

                            // Store the alt avatar selection
                            const tstring &sAvatar(EntityRegistry.LookupName(pPlayer->GetSelectedAvatar()));
                            xmlInfo.AddProperty("avatar", sAvatar);
                        }

                    xmlInfo.EndNode();
                }
            xmlInfo.EndNode();

            cArchive.WriteFile(_T("ReplayInfo"), xmlInfo.GetBuffer()->Get(), xmlInfo.GetBuffer()->GetLength());
        }

        cArchive.Close();

        FileManager.Delete(Filename_StripExtension(m_sFilename) + _T(".tmp"));
    }
}


/*====================
  UpdateNetworkResource
  ====================*/
static void UpdateNetworkResource(const string &sStateUTF8, const string &sValueUTF8)
{
    uint uiIndex(AtoI(UTF8ToString(sStateUTF8)));
    
    tstring sValue(UTF8ToTString(sValueUTF8));
    EResourceType eType(EResourceType(AtoI(sValue.substr(0, 3))));
    tstring sPath(sValue.substr(3));

#if 0
    // at this point, the specified path should have already been precached.
    ResHandle hRes(g_ResourceManager.LookUpPrecached(sPath));
    if (hRes == INVALID_RESOURCE)
    {
        K2_WITH_GAME_RESOURCE_SCOPE()
        {
            if (eType == RES_TEXTURE)
                hRes = g_ResourceManager.Register(K2_NEW(ctx_HostClient,  CTexture)(sPath, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
            else
                hRes = g_ResourceManager.Register(sPath, eType);
        }
    }
    NetworkResourceManager.RegisterNetworkResource(hRes, uiIndex);
#else
    // Load the resource
    ResHandle hRes(INVALID_RESOURCE);
    K2_WITH_GAME_RESOURCE_SCOPE()
    {
        if (eType == RES_TEXTURE)
            hRes = g_ResourceManager.Register(K2_NEW(ctx_HostClient,  CTexture)(sPath, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
        else
            hRes = g_ResourceManager.Register(sPath, eType);
    }
    NetworkResourceManager.RegisterNetworkResource(hRes, uiIndex);
#endif
}


/*====================
  CReplayManager::UpdateNetIndexes
  ====================*/
void    CReplayManager::UpdateNetIndexes()
{
    Game.GetStateString(STATE_STRING_RESOURCES).Set(m_vStateStrings[STATE_STRING_RESOURCES]);
    Game.GetStateString(STATE_STRING_RESOURCES).ForEachState(UpdateNetworkResource, false);
}


/*====================
  CReplayManager::StartPlayback
  ====================*/
bool    CReplayManager::StartPlayback(const tstring &sFilename, bool bGenerateKeyFrames)
{
    try
    {
        if (IsRecording())
            EX_WARN(_T("No playback allowed while recording"));
        
        if (sFilename.empty())
            return false;

        m_cArchive.Close();
        m_sWorldName.clear();
        m_mapEntDescs.clear();
        m_mapBaseDynamicEntDescs.clear();
        m_mapDynamicEntDescs.clear();
        m_iCurrentFrame = -1;
        m_mapKeyFrames.clear();
        m_mapStateStrings.clear();
        m_mapStateBlocks.clear();
        m_vStateStringStore.clear();
        m_vStateBlockStore.clear();

        IModalDialog::SetNumLoadingJobs(10);
        IModalDialog::SetDisplay(LOADING_DISPLAY_INTERFACE);
        IModalDialog::Show(_T("loading"));

        IModalDialog::SetProgress(0.0f);
        IModalDialog::Update();

        m_bFrameOpen = false;

        replay_pause = true; // Start paused

        // Open archive
        m_cArchive.Open(sFilename);

        if (!m_cArchive.IsOpen())
            EX_WARN(_T("Failed to open archive"));

        // Parse replay info
        {
            CFileHandle hReplayInfo(_T("ReplayInfo"), FILE_READ, m_cArchive);
            if (!hReplayInfo.IsOpen())
                EX_WARN(_T("Failed to open replay info"));

            XMLManager.Process(hReplayInfo, _T("replayinfo"), &m_cReplayInfo);
        }
            
        // Open replay data
        m_hReplayData.Open(_T("ReplayData"), FILE_READ | FILE_BINARY | FILE_UTF16, m_cArchive);

        if (!m_hReplayData.IsOpen())
            EX_WARN(_T("Failed to open replay data"));

        // Check for a valid header
        if (m_hReplayData.ReadByte() != 'S' ||
            m_hReplayData.ReadByte() != '2' ||
            m_hReplayData.ReadByte() != 'R' ||
            m_hReplayData.ReadByte() != '2')
            EX_WARN(_T("Invalid replay header"));

        // Check replay version
        m_uiReplayVersion = m_hReplayData.ReadInt32();

        if (m_uiReplayVersion > REPLAY_VERSION)
            EX_WARN(_T("Replay is newer than client"));

        // Game Version
        m_yGameVersion[0] = m_hReplayData.ReadByte();
        m_yGameVersion[1] = m_hReplayData.ReadByte();
        m_yGameVersion[2] = m_hReplayData.ReadByte();
        m_yGameVersion[3] = m_hReplayData.ReadByte();

        tstring sGameVersion(XtoA(m_yGameVersion[0]) + _T(".") + XtoA(m_yGameVersion[1]) + _T(".") + XtoA(m_yGameVersion[2]) + _T(".") + XtoA(m_yGameVersion[3]));

        FileManager.SetCompatVersion(sGameVersion);

        // Read static entity descriptors
        uint uiNumEntDescs(m_hReplayData.ReadInt32());

        for (uint ui(0); ui != uiNumEntDescs; ++ui)
        {
            // TypeID
            ushort unTypeID(m_hReplayData.ReadInt16());

            SReplayEntityDesc cDesc;

            // TypeName
            string sTypeName;

            char cChar(m_hReplayData.ReadByte());
            while (cChar)
            {
                sTypeName += cChar;
                cChar = m_hReplayData.ReadByte();
            }

            cDesc.sName = UTF8ToTString(sTypeName);
            cDesc.uiVersion = m_hReplayData.ReadInt32();

            uint uiNumFields(m_hReplayData.ReadByte());
            for (uint uiField(0); uiField != uiNumFields; ++uiField)
            {
                SDataField cField;

                // SDataField::sName
                string sName;

                char cChar(m_hReplayData.ReadByte());
                while (cChar)
                {
                    sName += cChar;
                    cChar = m_hReplayData.ReadByte();
                }

                cField.sName = UTF8ToTString(sName);

                // SDataField::eDataType
                cField.eDataType = EDataType(m_hReplayData.ReadByte());

                // SDataField::uiBits
                cField.uiBits = m_hReplayData.ReadInt32();

                // SDataField::iParam
                cField.iParam = m_hReplayData.ReadInt32();

                cDesc.cFieldTypes.push_back(cField);
            }

            cDesc.uiSize = CEntitySnapshot::CalcSnapshotSize(&cDesc.cFieldTypes);

            CBufferBit cBuffer;
            uint uiLastIndex(-2);

            ushort unLength(m_hReplayData.ReadInt16());

            for (ushort unBuffer(0); unBuffer != unLength; ++unBuffer)
                cBuffer.WriteByte(m_hReplayData.ReadByte());

            cDesc.cBaseline.ReadHeader(cBuffer, uiLastIndex);
            cDesc.cBaseline.ReadBody(cBuffer, cDesc.cFieldTypes, cDesc.uiSize);

            m_mapEntDescs[unTypeID] = cDesc;

            SReplayEntityDesc &cDesc2(m_mapEntDescs[unTypeID]);

            cDesc2.cCompatDesc.pFieldTypes = &cDesc2.cFieldTypes;
            cDesc2.cCompatDesc.uiSize = cDesc2.uiSize;
            cDesc2.cCompatDesc.pBaseline = &cDesc2.cBaseline;
            cDesc2.cCompatDesc.uiVersion = cDesc2.uiVersion;

            cDesc2.cBaseline.SetFieldTypes(&cDesc2.cFieldTypes, cDesc2.uiSize);
        }

        // Base dynamic entity descriptors
        uint uiNumBaseDynamicDescs(m_hReplayData.ReadInt32());

        for (uint ui(0); ui != uiNumBaseDynamicDescs; ++ui)
        {
            // Index
            uint uiIndex(m_hReplayData.ReadInt16());

            SReplayEntityDesc cDesc;

            // TypeName
            string sTypeName;

            char cChar(m_hReplayData.ReadByte());
            while (cChar)
            {
                sTypeName += cChar;
                cChar = m_hReplayData.ReadByte();
            }

            cDesc.sName = UTF8ToTString(sTypeName);
            cDesc.uiVersion = m_hReplayData.ReadInt32();

            uint uiNumFields(m_hReplayData.ReadByte());
            for (uint uiField(0); uiField != uiNumFields; ++uiField)
            {
                SDataField cField;

                // SDataField::sName
                string sName;

                char cChar(m_hReplayData.ReadByte());
                while (cChar)
                {
                    sName += cChar;
                    cChar = m_hReplayData.ReadByte();
                }

                cField.sName = UTF8ToTString(sName);

                // SDataField::eDataType
                cField.eDataType = EDataType(m_hReplayData.ReadByte());

                // SDataField::uiBits
                cField.uiBits = m_hReplayData.ReadInt32();

                // SDataField::iParam
                cField.iParam = m_hReplayData.ReadInt32();

                cDesc.cFieldTypes.push_back(cField);
            }

            cDesc.uiSize = CEntitySnapshot::CalcSnapshotSize(&cDesc.cFieldTypes);

            CBufferBit cBuffer;
            uint uiLastIndex(-2);

            ushort unLength(m_hReplayData.ReadInt16());

            for (ushort unBuffer(0); unBuffer != unLength; ++unBuffer)
                cBuffer.WriteByte(m_hReplayData.ReadByte());

            cDesc.cBaseline.ReadHeader(cBuffer, uiLastIndex);
            cDesc.cBaseline.ReadBody(cBuffer, cDesc.cFieldTypes, cDesc.uiSize);

            m_mapBaseDynamicEntDescs[uiIndex] = cDesc;
            
            SReplayEntityDesc &cDesc2(m_mapBaseDynamicEntDescs[uiIndex]);

            cDesc2.cCompatDesc.pFieldTypes = &cDesc2.cFieldTypes;
            cDesc2.cCompatDesc.uiSize = cDesc2.uiSize;
            cDesc2.cCompatDesc.pBaseline = &cDesc2.cBaseline;
            cDesc2.cCompatDesc.uiVersion = cDesc2.uiVersion;

            cDesc2.cBaseline.SetFieldTypes(&cDesc2.cFieldTypes, cDesc2.uiSize);
        }

        // Dynamic entity descriptors
        uint uiNumDynamicEntDescs(m_hReplayData.ReadInt32());

        for (uint ui(0); ui != uiNumDynamicEntDescs; ++ui)
        {
            // TypeID
            ushort unTypeID(m_hReplayData.ReadInt16());

            // BaseTypeID
            uint uiIndex(m_hReplayData.ReadInt16());

            m_mapDynamicEntDescs[unTypeID] = uiIndex;
        }

        m_bPlaying = true;
        replay_isPlaying = true;

        // Read world name
        char cChar(m_hReplayData.ReadByte());
        while (cChar)
        {
            m_sWorldName += cChar;
            cChar = m_hReplayData.ReadByte();
        }

        // Read state strings
        uint uiNumStateStrings((m_uiReplayVersion > 3) ? m_hReplayData.ReadInt32() : 5);
        
        m_vStateStrings.clear();
        m_vStateStrings.resize(uiNumStateStrings);

        for (uint uiID(1); uiID < uiNumStateStrings; ++uiID)
        {
            string &sStr(m_vStateStrings[uiID]);

            sStr.clear();

            char c(m_hReplayData.ReadByte());
            while (c)
            {
                sStr += c;
                c = m_hReplayData.ReadByte();
            }

            Game.GetStateString(uiID).Set(sStr);
        }
        m_vStateStringStore.push_back(m_vStateStrings);
        m_mapStateStrings.clear();

        // Apply state strings
        ICvar::ModifyTransmitCvars(&Game.GetStateString(STATE_STRING_CVAR_SETTINGS));
        Game.GetStateString(STATE_STRING_RESOURCES).ForEachState(UpdateNetworkResource, true);

        IModalDialog::NextLoadingJob();

        // Read state blocks
        uint uiNumStateBlocks((m_uiReplayVersion > 3) ? m_hReplayData.ReadInt32() : 3);
        for (uint uiID(1); uiID < uiNumStateBlocks; ++uiID)
        {
            CBufferDynamic &cBuffer(m_mapStateBlocks[uiID]);

            uint uiLength(m_hReplayData.ReadInt32());

            cBuffer.Reserve(uiLength);
            cBuffer.Clear();

            if (uiLength > 0)
                m_hReplayData.Read(cBuffer.Lock(uiLength), uiLength);

            Game.GetStateBlock(uiID).Set(cBuffer);
        }
        m_vStateBlockStore.push_back(m_mapStateBlocks);
        m_mapStateBlocks.clear();

        m_uiNumBitEntityFields = CEIL_MULTIPLE<32>(Game.GetStateBlock(STATE_BLOCK_BIT).GetBuffer().GetLength() / 4) / 32;

        //if (m_uiReplayVersion > 3)
        //  Game.UpdateHeroList();

        // Dynamic entity definitions (from state block)
        IBuffer &cDynamicEntities(Game.GetStateBlock(STATE_BLOCK_ENTITY_TYPES).GetBuffer());
        cDynamicEntities.Rewind();
        uint uiNumDynamicEntities(cDynamicEntities.GetLength() / 6);
        for (uint ui(0); ui < uiNumDynamicEntities; ++ui)
        {
            ushort unTypeID(cDynamicEntities.ReadShort());
            uint uiNetResIndex(cDynamicEntities.ReadInt());

            EntityRegistry.RegisterDynamicEntity(unTypeID, NetworkResourceManager.GetLocalHandle(uiNetResIndex));
        }
        Game.GetStateBlock(STATE_BLOCK_ENTITY_TYPES).Modify();

        m_cCurrentSnapshot.SetFrameNumber(uint(-1));
        m_cCurrentSnapshot.SetPrevFrameNumber(uint(-1));
        m_cCurrentSnapshot.SetTimeStamp(0);
        m_cCurrentSnapshot.FreeEntities();

        m_cCurrentSnapshot.SetNumStreams(3);

        m_cCurrentSnapshot.GetStreamBitEntityBuffer(0).resize(m_uiNumBitEntityFields, uint(-1));

        m_zStartPos = m_hReplayData.Tell();
        m_uiBeginTime = INVALID_TIME;
        m_uiEndTime = INVALID_TIME;

        if (bGenerateKeyFrames)
        {
            GenerateKeyFrames();
            //TestKeyFrames();
        }

        IModalDialog::NextLoadingJob();

        m_bFirstFrame = true;

        return true;
    }
    catch (CException &ex)
    {
        if (m_hReplayData.IsOpen())
            m_hReplayData.Close();

        m_cArchive.Close();

        ex.Process(_T("CReplayManager::StartPlayback() -"), NO_THROW);

        return false;
    }
}


/*====================
  CReplayManager::StopPlayback
  ====================*/
void    CReplayManager::StopPlayback()
{
    if (!IsPlaying())
        return;

    ICvar::SetBool(_T("cg_lockCamera"), false);
    m_bFrameOpen = false;
    m_hReplayData.Close();
    m_bPlaying = false;
    replay_isPlaying = false;
    m_uiReplayVersion = REPLAY_VERSION;
    m_cArchive.Close();
    m_sWorldName.clear();
    m_mapEntDescs.clear();
    m_mapBaseDynamicEntDescs.clear();
    m_mapDynamicEntDescs.clear();
    m_mapKeyFrames.clear();

    if (m_iSpeed != 0)
        SetPlaybackSpeed(0);
}


/*====================
  CReplayManager::IsRecording
  ====================*/
bool    CReplayManager::IsRecording()
{
    return m_hReplayData.IsOpen() && m_bRecording;
}


/*====================
  CReplayManager::IsPlaying
  ====================*/
bool    CReplayManager::IsPlaying()
{
    return m_hReplayData.IsOpen() && m_bPlaying;
}


/*====================
  CReplayManager::ReadSnapshot
  ====================*/
void    CReplayManager::ReadSnapshot(int iFrame, CBufferBit &cBuffer)
{
    CSnapshot snapshot(cBuffer);

    m_cCurrentSnapshot.SetValid(true);
    m_cCurrentSnapshot.SetFrameNumber(iFrame);
    m_cCurrentSnapshot.SetPrevFrameNumber(uint(-1));
    m_cCurrentSnapshot.SetTimeStamp(snapshot.GetTimeStamp());
    
    // Clear events
    m_cCurrentSnapshot.SetNumEvents(0);
    m_cCurrentSnapshot.GetEventBuffer().Clear();

    byte yNumEvents(snapshot.GetNumEvents());

    // Translate events
    CBufferDynamic cBufferTranslate(40);
    for (int i(0); i < yNumEvents; ++i)
    {
        CGameEvent::Translate2(snapshot.GetReceivedBuffer(), cBufferTranslate);
        m_cCurrentSnapshot.AddEventSnapshot(cBufferTranslate);
    }

    uivector &vBitEntityBuffer(m_cCurrentSnapshot.GetStreamBitEntityBuffer(0));
    vBitEntityBuffer.resize(m_uiNumBitEntityFields, uint(-1));

    CTransmitFlags<8> cTransmitFlags(m_uiNumBitEntityFields);

    cTransmitFlags.ReadTransmitFlags(snapshot.GetReceivedBuffer());

    for (uint uiField(0); uiField < m_uiNumBitEntityFields; ++uiField)
    {
        if (cTransmitFlags.IsFieldSet(uiField))
            vBitEntityBuffer[uiField] = snapshot.GetReceivedBuffer().ReadBits(32);
    }

    SnapshotVector &vBaseEntities(m_cCurrentSnapshot.GetEntities());
    SnapshotVector_it citBase(vBaseEntities.begin());

    static CEntitySnapshot entSnapshot;

    uint uiLastIndex(0);

    // Translate entities
    for (;;)
    {
        // Grab a "shell" entity snapshot from the the frame snapshot.
        // The data will be filled in once we know the type.
        entSnapshot.Clear();
        if (!snapshot.GetNextEntity(entSnapshot, uiLastIndex))
            break;

        if (!entSnapshot.GetChanged())
            EX_ERROR(_T("Invalid entity snapshot"));

        if (entSnapshot.GetIndex() == INVALID_INDEX)
        {
            // Look for the index of an implied change
            while (citBase != vBaseEntities.end())
            {
                CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->second));
                if (pBaseSnapshot->GetChanging())
                {
                    entSnapshot.SetIndex(citBase->first);
                    uiLastIndex = citBase->first;
                    break;
                }
                
                ++citBase;
            }
            if (citBase == vBaseEntities.end())
            {
                if (snapshot.GetReceivedBuffer().GetUnreadBits() >= 8)
                    EX_ERROR(XtoA(snapshot.GetReceivedBuffer().GetUnreadBits()) + _T(" unread bits"));
                
                break; // End of snapshot
            }
        }
        else
        {
            while (citBase != vBaseEntities.end() && citBase->first < entSnapshot.GetIndex())
                ++citBase;
        }

        if (citBase == vBaseEntities.end() || citBase->first > entSnapshot.GetIndex())
        {
            //
            // New entity, read from baseline
            //

            ushort unType(entSnapshot.GetType());

            // If the type is NULL, the entity is dead and should be removed
            if (unType == 0)
                continue;

            const SReplayEntityDesc* pTypeDesc(GetTypeDesc(unType));
            if (pTypeDesc == NULL)
                EX_ERROR(_T("Unknown entity type, bad snapshot"));

            entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), pTypeDesc->cFieldTypes, pTypeDesc->uiSize, &pTypeDesc->cBaseline);
            citBase = vBaseEntities.insert(citBase, SnapshotEntry(entSnapshot.GetIndex(), CEntitySnapshot::Allocate(entSnapshot)));

            ++citBase;
        }
        else if (citBase->first == entSnapshot.GetIndex())
        {
            //
            // Update existing entity
            //

            CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->second));
            ushort unType(entSnapshot.GetTypeChange() ? entSnapshot.GetType() : pBaseSnapshot->GetType());

            // If the type is NULL, the entity is dead and should be removed
            if (unType == 0)
            {
                CEntitySnapshot::DeleteByHandle(citBase->second);
                citBase = vBaseEntities.erase(citBase);
                continue;
            }

            const SReplayEntityDesc* pTypeDesc(GetTypeDesc(unType));
            if (pTypeDesc == NULL)
                EX_ERROR(_T("Unknown entity type, bad snapshot"));

            if (entSnapshot.GetTypeChange())
            {
                entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), pTypeDesc->cFieldTypes, pTypeDesc->uiSize, &pTypeDesc->cBaseline);
                *pBaseSnapshot = entSnapshot;
            }
            else
            {
                entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), pTypeDesc->cFieldTypes, pTypeDesc->uiSize);
                pBaseSnapshot->ApplyDiff(entSnapshot);
            }
            ++citBase;
        }
    }

    m_cCurrentSnapshot.SetNumStreams(3);
    m_cCurrentSnapshot.ClearStreams();
    
    SnapshotVector &SnapshotsThis(m_cCurrentSnapshot.GetEntities());

    SnapshotVector_cit citThis(SnapshotsThis.begin()), citThisEnd(SnapshotsThis.end());
    for (; citThis != citThisEnd; ++citThis)
    {
        CEntitySnapshot *pThisSnapshot(CEntitySnapshot::GetByHandle(citThis->second));

        pThisSnapshot->SetPublicSequence(1); // Something different than default

        uint uiFlags(pThisSnapshot->GetChanging() ? ENTITY_STREAM_FLAG_CHANGING : 0);
        
        m_cCurrentSnapshot.AddStreamEntity(0, citThis->first, citThis->second, uiFlags);
        m_cCurrentSnapshot.AddStreamEntity(1, citThis->first, citThis->second, uiFlags);
        m_cCurrentSnapshot.AddStreamEntity(2, citThis->first, citThis->second, uiFlags);
    }
}


/*====================
  CReplayManager::StartFrame
  ====================*/
void    CReplayManager::StartFrame(int iFrame)
{
    if (IsPlaying() && replay_pause && !m_bFirstFrame)
    {
        m_bFrameOpen = true;
        m_cCurrentSnapshot.SetFrameNumber(iFrame);
        m_cCurrentSnapshot.ClearEvents();
    }
    else if (IsPlaying())
    {
        m_bFirstFrame = false;

        uint uiInitialGameTime(m_cCurrentSnapshot.GetTimeStamp());

        do
        {
            if (m_hReplayData.IsEOF() || uint(m_iCurrentFrame) == m_uiNumFrames)
            {
                m_bFrameOpen = true;
                m_cCurrentSnapshot.SetFrameNumber(iFrame);
                m_cCurrentSnapshot.ClearEvents();
                return;
            }

            uint uiLength(m_hReplayData.ReadInt32());

            if (uiLength == 0)
                return;

            CBufferBit cBuffer(uiLength);
            cBuffer.SetLength(uiLength);
            m_hReplayData.Read((char *)cBuffer.Get(), uiLength);

            ReadSnapshot(iFrame, cBuffer);

            uint uiNumClients;

            // Read reliable game data
            uiNumClients = m_hReplayData.ReadInt32();
            for (uint ui(0); ui < uiNumClients; ++ui)
            {
                uint uiIndex(m_hReplayData.ReadInt32());
                uint uiLength(m_hReplayData.ReadInt32());

                if (uiLength > 0)
                {
                    CBufferStatic cBuffer(uiLength);

                    for (uint uiRead(0); uiRead < uiLength; ++uiRead)
                        cBuffer << m_hReplayData.ReadByte();

                    m_mapGameDataReliable[uiIndex].Append(cBuffer.Get(), cBuffer.GetLength());
                }
            }

            // Read game data
            uiNumClients = m_hReplayData.ReadInt32();
            for (uint ui(0); ui < uiNumClients; ++ui)
            {
                uint uiIndex(m_hReplayData.ReadInt32());
                uint uiLength(m_hReplayData.ReadInt32());

                if (uiLength > 0)
                {
                    CBufferStatic cBuffer(uiLength);

                    for (uint uiRead(0); uiRead < uiLength; ++uiRead)
                        cBuffer << m_hReplayData.ReadByte();

                    m_mapGameData[uiIndex].Append(cBuffer.Get(), cBuffer.GetLength());
                }
            }

            // Read state strings
            uint uiNumStateString(m_hReplayData.ReadInt32());
            for (uint ui(0); ui < uiNumStateString; ++ui)
            {
                uint uiID(m_hReplayData.ReadInt32());
                string sStr;

                char c(m_hReplayData.ReadByte());
                while (c)
                {
                    sStr += c;
                    c = m_hReplayData.ReadByte();
                }

                m_mapStateStrings[uiID] = sStr;
            }

            // Read state blocks
            if (m_uiReplayVersion > 3)
            {
                uint uiNumStateBlocks(m_hReplayData.ReadInt32());
                for (uint ui(0); ui < uiNumStateBlocks; ++ui)
                {
                    uint uiID(m_hReplayData.ReadInt32());

                    CBufferDynamic &buffer(m_mapStateBlocks[uiID]);

                    uint uiLength(m_hReplayData.ReadInt32());

                    buffer.Clear();

                    if (uiLength == 0)
                        continue;

                    buffer.Reserve(uiLength);
                    m_hReplayData.Read(buffer.Lock(uiLength), uiLength);
                }
            }

            ++m_iCurrentFrame;

            m_bFrameOpen = true;

            if (replay_autoSkipPause && m_cCurrentSnapshot.GetTimeStamp() == uiInitialGameTime)
            {
                EndFrame(INVALID_POOL_HANDLE);
                continue;
            }
            else
            {
                break;
            }

        } while (true);
    }
    else if (IsRecording())
    {
        m_iCurrentFrame = iFrame;

        m_bFrameOpen = true;
    }
}


/*====================
  CReplayManager::EndFrame
  ====================*/
void    CReplayManager::EndFrame(PoolHandle hSnapshot)
{
    GAME_PROFILE(_T("CReplayManager::EndFrame"));

    if (!m_bFrameOpen)
        return;

    // Write game snapshot
    if (IsRecording())
    {
        const CSnapshot &cSnapshot(*CSnapshot::GetByHandle(hSnapshot));

        CBufferBit cBufferFrameData;

        if (m_hLastSnapshot != INVALID_POOL_HANDLE)
        {
            const CSnapshot &cLastSnapshot(*CSnapshot::GetByHandle(m_hLastSnapshot));
            cSnapshot.WriteDiff(cBufferFrameData, cLastSnapshot, 0, 0, 0);
        }
        else
        {
            cSnapshot.WriteBuffer(cBufferFrameData, 0, 0);
        }

        m_hReplayData.WriteInt32(cBufferFrameData.GetLength());
        m_hReplayData.Write(cBufferFrameData.Get(), cBufferFrameData.GetLength());

        // Write reliable game data
        uint uiNumClientsReliable(0);
        for (MapClientGameData::iterator it(m_mapGameDataReliable.begin()); it != m_mapGameDataReliable.end(); ++it)
        {
            if (it->second.GetLength() > 0)
                ++uiNumClientsReliable;
        }

        m_hReplayData.WriteInt32(uiNumClientsReliable);
        for (MapClientGameData::iterator it(m_mapGameDataReliable.begin()); it != m_mapGameDataReliable.end(); ++it)
        {
            if (it->second.GetLength() == 0)
                continue;

            m_hReplayData.WriteInt32(it->first);
            m_hReplayData.WriteInt32(it->second.GetLength());
            m_hReplayData.Write(it->second.Get(), it->second.GetLength());

            it->second.Clear();
        }

        // Write game data
        uint uiNumClients(0);
        for (MapClientGameData::iterator it(m_mapGameData.begin()); it != m_mapGameData.end(); ++it)
        {
            if (it->second.GetLength() > 0)
                ++uiNumClients;
        }

        m_hReplayData.WriteInt32(uiNumClients);
        for (MapClientGameData::iterator it(m_mapGameData.begin()); it != m_mapGameData.end(); ++it)
        {
            if (it->second.GetLength() == 0)
                continue;

            m_hReplayData.WriteInt32(it->first);
            m_hReplayData.WriteInt32(it->second.GetLength());
            m_hReplayData.Write(it->second.Get(), it->second.GetLength());

            it->second.Clear();
        }

        // Write state strings
        m_hReplayData.WriteInt32(uint(m_mapStateStrings.size()));
        for (MapStateString::iterator it(m_mapStateStrings.begin()); it != m_mapStateStrings.end(); ++it)
        {
            m_hReplayData.WriteInt32(it->first);

            const string &sStateString(it->second);
            m_hReplayData.Write(sStateString.c_str(), sStateString.length());
            m_hReplayData.WriteByte(byte(0));
        }
        m_mapStateStrings.clear();

        // Write state blocks
        m_hReplayData.WriteInt32(uint(m_mapStateBlocks.size()));
        for (MapStateBlock::iterator itBlock(m_mapStateBlocks.begin()), itEnd(m_mapStateBlocks.end()); itBlock != itEnd; ++itBlock)
        {
            m_hReplayData.WriteInt32(itBlock->first);
            m_hReplayData.WriteInt32(itBlock->second.GetLength());
            if (itBlock->second.GetLength() > 0)
                m_hReplayData.Write(itBlock->second.Get(), itBlock->second.GetLength());
        }
        m_mapStateBlocks.clear();

        SAFE_DELETE_SNAPSHOT(m_hLastSnapshot);
        m_hLastSnapshot = hSnapshot;
        CSnapshot::AddRefToHandle(hSnapshot);
    }
    else if (IsPlaying())
    {
        // Clear old data
        for (MapClientGameData::iterator it(m_mapGameDataReliable.begin()); it != m_mapGameDataReliable.end(); ++it)
            it->second.Clear();

        for (MapClientGameData::iterator it(m_mapGameData.begin()); it != m_mapGameData.end(); ++it)
            it->second.Clear();

        m_mapStateStrings.clear();
        m_mapStateBlocks.clear();
    }

    m_bFrameOpen = false;
}


/*====================
  CReplayManager::WriteStateString

  TODO: Better memory allocations here
  ====================*/
void    CReplayManager::WriteStateString(uint uiID, const CStateString &ss)
{
    if (IsPlaying())
        return;

    string sBuffer;
    ss.AppendToBuffer(sBuffer);

    m_mapStateStrings[uiID] = sBuffer;
}


/*====================
  CReplayManager::WriteStateBlock
  ====================*/
void    CReplayManager::WriteStateBlock(uint uiID, const IBuffer &buffer)
{
    if (IsPlaying())
        return;

    m_mapStateBlocks[uiID].Write(buffer.Get(), buffer.GetLength());
}


/*====================
  CReplayManager::WriteGameData
  ====================*/
void    CReplayManager::WriteGameData(uint iClient, const IBuffer &buffer, bool bReliable)
{
    if (!IsRecording())
        return;

    if (bReliable)
    {
        if (m_mapGameDataReliable[iClient].GetLength() > 0)
            m_mapGameDataReliable[iClient] << NETCMD_SERVER_GAME_DATA;
        
        m_mapGameDataReliable[iClient] << buffer;
    }
    else
    {
        if (m_mapGameData[iClient].GetLength() > 0)
            m_mapGameData[iClient] << NETCMD_SERVER_GAME_DATA;
        
        m_mapGameData[iClient] << buffer;
    }
}


/*====================
  CReplayManager::GetWorldName
  ====================*/
tstring     CReplayManager::GetWorldName()
{
    return UTF8ToTString(m_sWorldName);
}


/*====================
  CReplayManager::GetSnapshot
  ====================*/
void    CReplayManager::GetSnapshot(CSnapshot &snapshot)
{
    if (!m_bFrameOpen)
        return;

    snapshot = m_cCurrentSnapshot;
}


/*====================
  CReplayManager::GetGameData
  ====================*/
void    CReplayManager::GetGameData(uint uiClient, IBuffer &buffer)
{
    if (!m_bFrameOpen)
        return;

    MapClientGameData::iterator itFind(m_mapGameData.find(uiClient));
    if (itFind != m_mapGameData.end())
        buffer = itFind->second;
}


/*====================
  CReplayManager::GetGameDataReliable
  ====================*/
void    CReplayManager::GetGameDataReliable(uint uiClient, IBuffer &buffer)
{
    if (!m_bFrameOpen)
        return;

    MapClientGameData::iterator itFind(m_mapGameDataReliable.find(uiClient));
    if (itFind != m_mapGameDataReliable.end() && itFind->second.GetLength() <= 1300)
        buffer = itFind->second;
}


/*====================
  CReplayManager::SetPlaybackFrame
  ====================*/
void    CReplayManager::SetPlaybackFrame(int iFrame)
{
    if (m_mapKeyFrames.empty())
        return;

    iFrame = CLAMP<int>(iFrame, 0, m_uiNumFrames - 1);

    if (iFrame == 0)
    {
        m_hReplayData.Seek(int(m_zStartPos));

        m_cCurrentSnapshot.SetFrameNumber(uint(-1));
        m_cCurrentSnapshot.SetPrevFrameNumber(uint(-1));
        m_cCurrentSnapshot.SetTimeStamp(0);
        m_cCurrentSnapshot.FreeEntities();

        m_mapGameData.clear();
        m_mapGameDataReliable.clear();
        m_mapStateStrings.clear();
        m_mapStateBlocks.clear();

        m_iCurrentFrame = 0;
    }

    // Search for the closest valid keyframe
    map<uint, SReplayKeyFrame>::iterator it(m_mapKeyFrames.begin());
    for (; it != m_mapKeyFrames.end(); ++it)
    {
        if (it->first >= uint(iFrame))
            break;
    }

    if (it != m_mapKeyFrames.end())
    {
        m_hReplayData.Seek(int(it->second.zPos));

        m_cCurrentSnapshot.SetFrameNumber(uint(-1));
        m_cCurrentSnapshot.SetPrevFrameNumber(uint(-1));
        m_cCurrentSnapshot.SetTimeStamp(0);
        m_cCurrentSnapshot.FreeEntities();

        uivector &vBitEntityBuffer(m_cCurrentSnapshot.GetStreamBitEntityBuffer(0));
        vBitEntityBuffer.resize(m_uiNumBitEntityFields, uint(-1));
    
        for (uint uiField(0); uiField < m_uiNumBitEntityFields; ++uiField)
            vBitEntityBuffer[uiField] = uint(-1);

        it->second.cBuffer.Rewind();

        ReadSnapshot(it->second.uiFrame, it->second.cBuffer);

        m_mapGameData.clear();
        m_mapGameDataReliable.clear();
        m_mapStateStrings.clear();
        m_mapStateBlocks.clear();

        if (it->second.uiStateString != INVALID_INDEX)
        {
            svector &vStateStrings(m_vStateStringStore[it->second.uiStateString]);

            for (uint uiID(0); uiID < uint(vStateStrings.size()); ++uiID)
            {
                CStateString &ssCurrent(Game.GetStateString(uiID));
                CStateString ssNew;
                ssNew.Set(vStateStrings[uiID]);

                CStateString ssDiff(ssNew);
                ssCurrent.GetDifference(ssDiff);

                if (ssDiff.IsEmpty())
                    continue;

                m_mapStateStrings[uiID] = vStateStrings[uiID];
            }
        }   

        if (it->second.uiStateBlock != INVALID_INDEX)
        {
            MapStateBlock &mapStateBlocks(m_vStateBlockStore[it->second.uiStateBlock]);

            for (MapStateBlock::iterator it(mapStateBlocks.begin()); it != mapStateBlocks.end(); ++it)
            {
                CBufferDynamic &cCurrentBuffer(Game.GetStateBlock(it->first).GetBuffer());
                CBufferDynamic &cNewBuffer(it->second);

                if (cCurrentBuffer.GetLength() != cNewBuffer.GetLength() ||
                    memcmp(cCurrentBuffer.GetBuffer(), cNewBuffer.GetBuffer(), cNewBuffer.GetLength()) != 0)
                {
                    m_mapStateBlocks[it->first] = cNewBuffer;
                }
            }
        }   

        m_iCurrentFrame = it->second.uiFrame;
    }
}


/*====================
  CReplayManager::TestKeyFrames
  ====================*/
void    CReplayManager::TestKeyFrames()
{
    int iOldFrame(m_iCurrentFrame);

    map<uint, SReplayKeyFrame>::iterator it(m_mapKeyFrames.begin());
    for (; it != m_mapKeyFrames.end(); ++it)
    {
        m_hReplayData.Seek(int(it->second.zPos));

        m_cCurrentSnapshot.SetFrameNumber(uint(-1));
        m_cCurrentSnapshot.SetPrevFrameNumber(uint(-1));
        m_cCurrentSnapshot.SetTimeStamp(0);
        m_cCurrentSnapshot.FreeEntities();

        it->second.cBuffer.Rewind();

        ReadSnapshot(it->second.uiFrame, it->second.cBuffer);

        m_mapGameData.clear();
        m_mapGameDataReliable.clear();
        m_mapStateStrings.clear();
        m_mapStateBlocks.clear();

        m_iCurrentFrame = it->second.uiFrame;
    }

    SetPlaybackFrame(iOldFrame);
}


/*====================
  CReplayManager::Profile
  ====================*/
void    CReplayManager::Profile(const tstring &sFilename, int iClient)
{
#if 0
    if (!StartPlayback(sFilename, false))
        return;

    int iFrame(0);

    int iSnapshotHeaderBytes(0);
    
    int iNumEvents(0);
    int iEventBytes(0);
    
    int iNumEntitySnapshots(0);
    int iEntitySnapshotBytes(0);
    int iEntityHeaderBytes(0);
    int iEntityTransmitFlagBytes(0);
    int iEntityFieldBytes(0);
    int iEntityDynamicChanges(0);

    map<ushort, SProfileEntitySnapshot> mapProfileEntity;
    map<uint, SProfileEntitySnapshot>   mapProfileEntityDynamic;
    
    int iReliableGameDataBytes(0);
    int iGameDataBytes(0);
    int iStateStringBytes(0);

    map<uint, uint>     mapReliableGameDataBytes;
    map<uint, uint>     mapGameDataBytes;

    CBufferBit cBuffer(0x4000);
    CSnapshot snapshot;

    map<uint, uint> mapCount;
    map<int, uint> mapCount2;
    
    while (!m_hReplayData.IsEOF())
    {
        uint uiLength(m_hReplayData.ReadInt32());
        if (uiLength == 0)
            continue;

        cBuffer.Clear();

        cBuffer.Reserve(uiLength);
        cBuffer.SetLength(uiLength);
        m_hReplayData.Read((char *)cBuffer.Get(), uiLength);

        uint uiSnapshotStartPos(cBuffer.GetReadPos());

        snapshot.ReadBuffer(cBuffer);

        iSnapshotHeaderBytes += cBuffer.GetReadPos() - uiSnapshotStartPos;

        m_cCurrentSnapshot.SetValid(true);
        m_cCurrentSnapshot.SetFrameNumber(snapshot.GetFrameNumber());
        m_cCurrentSnapshot.SetPrevFrameNumber(uint(-1));
        m_cCurrentSnapshot.SetTimeStamp(snapshot.GetTimeStamp());
        
        // Clear events
        m_cCurrentSnapshot.SetNumEvents(0);
        m_cCurrentSnapshot.GetEventBuffer().Clear();

        byte yNumEvents(snapshot.GetNumEvents());

        CBufferBit &cReceivedBuffer(snapshot.GetReceivedBuffer());

        // Translate events
        for (int i(0); i < yNumEvents; ++i)
        {
            uint uiStartPos(cReceivedBuffer.GetReadPos());

            CGameEvent::AdvanceBuffer2(cReceivedBuffer);
            
            ++iNumEvents;

            iEventBytes += cReceivedBuffer.GetReadPos() - uiStartPos;
        }

        CTransmitFlags<8> cTransmitFlags(m_uiNumBitEntityFields);

        cTransmitFlags.ReadTransmitFlags(cReceivedBuffer);

        for (uint uiField(0); uiField < m_uiNumBitEntityFields; ++uiField)
        {
            if (cTransmitFlags.IsFieldSet(uiField))
                snapshot.GetReceivedBuffer().ReadBits(32);
        }
        SnapshotVector &vBaseEntities(m_cCurrentSnapshot.GetEntities());
        SnapshotVector_it citBase(vBaseEntities.begin());

        static CEntitySnapshot entSnapshot;

        bool bError(false);

        uint uiLastIndex(0);

        // Translate entities
        while (!bError)
        {
            uint uiEntitySnapshotStartPos(cBuffer.GetReadPos());
            const SEntityDesc* pTypeDesc(NULL);

            // Grab a "shell" entity snapshot from the the frame snapshot.
            // The data will be filled in once we know the type.
            entSnapshot.Clear();
            if (!snapshot.GetNextEntity(entSnapshot, uiLastIndex))
                break;

            if (!entSnapshot.GetChanged())
            {
                Console.Err << _T("Invalid entity snapshot") << newl;
                bError = true;
                break;
            }

            uint uiEntityFieldStartPos(cReceivedBuffer.GetReadPos());

            if (entSnapshot.GetIndex() == INVALID_INDEX)
            {
                // Look for the index of an implied change
                while (citBase != vBaseEntities.end())
                {
                    CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->second));
                    if (pBaseSnapshot->GetChanging())
                    {
                        entSnapshot.SetIndex(citBase->first);
                        uiLastIndex = citBase->first;
                        break;
                    }
                    
                    ++citBase;
                }
                if (citBase == vBaseEntities.end())
                {
                    if (cReceivedBuffer.GetUnreadBits() >= 8)
                    {
                        Console.Err << cReceivedBuffer.GetUnreadBits() << _T(" unread bits") << newl;
                        bError = true;
                    }
                    
                    break; // End of snapshot
                }
            }
            else
            {
                while (citBase != vBaseEntities.end() && citBase->first < entSnapshot.GetIndex())
                    ++citBase;
            }

            if (citBase == vBaseEntities.end() || citBase->first > entSnapshot.GetIndex())
            {
                //
                // New entity, read from baseline
                //

                ushort unType(entSnapshot.GetType());

                const CDynamicEntityAllocator *pDynamicAllocator(EntityRegistry.GetDynamicAllocator(unType));

                SProfileEntitySnapshot &cEntityProfile(pDynamicAllocator != NULL ? mapProfileEntityDynamic[pDynamicAllocator->GetBaseType()] : mapProfileEntity[unType]);

                ++iNumEntitySnapshots;

                ++cEntityProfile.uiCount;

                uint uiHeaderBytes(cReceivedBuffer.GetReadPos() - uiEntitySnapshotStartPos);

                iEntityHeaderBytes += uiHeaderBytes;
                cEntityProfile.uiHeaderBytes += uiHeaderBytes;

                // If the type is NULL, the entity is dead and should be removed
                if (unType == 0)
                {
                    uint uiSnapshotBytes(cReceivedBuffer.GetReadPos() - uiEntitySnapshotStartPos);

                    iEntitySnapshotBytes += uiSnapshotBytes;
                    cEntityProfile.uiSnapshotBytes += uiSnapshotBytes;
                    continue;
                }

                pTypeDesc = EntityRegistry.GetTypeDesc(unType);
                if (pTypeDesc == NULL)
                {
                    Console.Err << _T("Unknown entity type, bad snapshot") << newl;
                    bError = true;
                    break;
                }

                cEntityProfile.vFieldChanges.resize(pTypeDesc->pFieldTypes->size(), 0);

                uint uiEntityFieldStartPos(cReceivedBuffer.GetReadPos());

                entSnapshot.ReadBody(cReceivedBuffer, *pTypeDesc->pFieldTypes, pTypeDesc->uiSize, pTypeDesc->pBaseline, false);
                citBase = vBaseEntities.insert(citBase, SnapshotEntry(entSnapshot.GetIndex(), CEntitySnapshot::Allocate(entSnapshot)));
                ++citBase;

                ++iEntityDynamicChanges;

                uint uiFieldBytes(cReceivedBuffer.GetReadPos() - uiEntityFieldStartPos - entSnapshot.GetTransmitFlagBytes());

                iEntityTransmitFlagBytes += entSnapshot.GetTransmitFlagBytes();
                cEntityProfile.uiTransmitFlagBytes += entSnapshot.GetTransmitFlagBytes();

                iEntityFieldBytes += uiFieldBytes;
                cEntityProfile.uiFieldBytes += uiFieldBytes;

                uint uiSnapshotBytes(cReceivedBuffer.GetReadPos() - uiEntitySnapshotStartPos);

                iEntitySnapshotBytes += uiSnapshotBytes;
                cEntityProfile.uiSnapshotBytes += uiSnapshotBytes;
            }
            else if (citBase->first == entSnapshot.GetIndex())
            {
                //
                // Update existing entity
                //

                CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->second));
                ushort unType(entSnapshot.GetTypeChange() ? entSnapshot.GetType() : pBaseSnapshot->GetType());

                const CDynamicEntityAllocator *pDynamicAllocator(EntityRegistry.GetDynamicAllocator(unType));

                SProfileEntitySnapshot &cEntityProfile(pDynamicAllocator != NULL ? mapProfileEntityDynamic[pDynamicAllocator->GetBaseType()] : mapProfileEntity[unType]);

                ++iNumEntitySnapshots;

                ++cEntityProfile.uiCount;

                uint uiHeaderBytes(cReceivedBuffer.GetReadPos() - uiEntitySnapshotStartPos);

                iEntityHeaderBytes += uiHeaderBytes;
                cEntityProfile.uiHeaderBytes += uiHeaderBytes;

                // If the type is NULL, the entity is dead and should be removed
                if (unType == 0)
                {
                    uint uiSnapshotBytes(cReceivedBuffer.GetReadPos() - uiEntitySnapshotStartPos);

                    iEntitySnapshotBytes += uiSnapshotBytes;
                    cEntityProfile.uiSnapshotBytes += uiSnapshotBytes;

                    CEntitySnapshot::DeleteByHandle(citBase->second);
                    citBase = vBaseEntities.erase(citBase);
                    continue;
                }

                pTypeDesc = EntityRegistry.GetTypeDesc(unType);
                if (pTypeDesc == NULL)
                {
                    Console.Err << _T("Unknown entity type, bad snapshot") << newl;
                    bError = true;
                    break;
                }

                cEntityProfile.vFieldChanges.resize(pTypeDesc->pFieldTypes->size(), 0);

                if (entSnapshot.GetTypeChange())
                {
                    entSnapshot.ReadBody(cReceivedBuffer, *pTypeDesc->pFieldTypes, pTypeDesc->uiSize, pTypeDesc->pBaseline, false);
                    *pBaseSnapshot = entSnapshot;
                }
                else
                {
                    entSnapshot.ReadBody(cReceivedBuffer, *pTypeDesc->pFieldTypes, pTypeDesc->uiSize);
                    pBaseSnapshot->ApplyDiff(entSnapshot);
                }
                ++citBase;

                uint uiFieldBytes(cReceivedBuffer.GetReadPos() - uiEntityFieldStartPos - entSnapshot.GetTransmitFlagBytes());

                iEntityTransmitFlagBytes += entSnapshot.GetTransmitFlagBytes();
                cEntityProfile.uiTransmitFlagBytes += entSnapshot.GetTransmitFlagBytes();

                iEntityFieldBytes += uiFieldBytes;
                cEntityProfile.uiFieldBytes += uiFieldBytes;

                uint uiSnapshotBytes(cReceivedBuffer.GetReadPos() - uiEntitySnapshotStartPos);

                iEntitySnapshotBytes += uiSnapshotBytes;
                cEntityProfile.uiSnapshotBytes += uiSnapshotBytes;

                const TypeVector &cTypeVector(*pTypeDesc->pFieldTypes);

                for (uint ui(0); ui < cTypeVector.size(); ++ui)
                    if (entSnapshot.IsFieldSet(ui))
                        ++cEntityProfile.vFieldChanges[ui];
            }
        }

        if (bError)
        {
            m_hReplayData.Seek(0, SEEK_ORIGIN_END);
            break;
        }

        uint uiNumClients;

        // Read reliable game data
        uiNumClients = m_hReplayData.ReadInt32();
        for (uint ui(0); ui < uiNumClients; ++ui)
        {
            uint uiIndex(m_hReplayData.ReadInt32());
            uint uiLength(m_hReplayData.ReadInt32());

            if (uiLength > 0)
            {
                m_hReplayData.Seek(uiLength, SEEK_ORIGIN_CURRENT);

                iReliableGameDataBytes += uiLength + 8;

                if (mapReliableGameDataBytes.find(uiIndex) == mapReliableGameDataBytes.end())
                    mapReliableGameDataBytes[uiIndex] = uiLength + 8;
                else
                    mapReliableGameDataBytes[uiIndex] = mapReliableGameDataBytes[uiIndex] + uiLength + 8;
            }
        }

        // Read game data
        uiNumClients = m_hReplayData.ReadInt32();
        for (uint ui(0); ui < uiNumClients; ++ui)
        {
            uint uiIndex(m_hReplayData.ReadInt32());
            uint uiLength(m_hReplayData.ReadInt32());

            if (uiLength > 0)
            {
                m_hReplayData.Seek(uiLength, SEEK_ORIGIN_CURRENT);

                iGameDataBytes += uiLength + 8;

                if (mapGameDataBytes.find(uiIndex) == mapGameDataBytes.end())
                    mapGameDataBytes[uiIndex] = uiLength + 8;
                else
                    mapGameDataBytes[uiIndex] = mapGameDataBytes[uiIndex] + uiLength + 8;
            }

        }

        // Read state strings
        uint uiNumStateString(m_hReplayData.ReadInt32());
        for (uint ui(0); ui < uiNumStateString; ++ui)
        {
            m_hReplayData.ReadInt32(); // uiID

            wchar_t wChar(m_hReplayData.ReadInt16());
            while (wChar)
                wChar = m_hReplayData.ReadInt16();
        }

        ++iFrame;
    }

    tstring sSamplesOutputFilename(iClient != -1 ? _TS("~/") + Filename_StripExtension(sFilename) + _T("-") + XtoA(iClient) + _T("-samples") + _T(".txt") : _TS("~/") + Filename_StripExtension(sFilename) + _T("-samples") + _T(".txt"));
    CFileHandle hSamples(sSamplesOutputFilename, FILE_WRITE | FILE_TEXT);

    for (map<int, uint>::iterator it(mapCount2.begin()); it != mapCount2.end(); ++it)
    {
        hSamples << it->first << _T(" ") << it->second << newl;
    }

    tstring sOutputFilename(iClient != -1 ? _T("~/") + Filename_StripExtension(sFilename) + _T("-") + XtoA(iClient) + _T(".txt") : _T("~/") + Filename_StripExtension(sFilename) + _T(".txt"));

    CFileHandle hOutput(sOutputFilename, FILE_WRITE | FILE_TEXT);

    hOutput << _T("Frames: ") << iFrame << newl;
    hOutput << _T("Total Bytes: ") << uint(m_hReplayData.Tell()) << newl;
    
    hOutput << _T("Snapshot Header Bytes: ") << iSnapshotHeaderBytes << newl;
    hOutput << _T("Events: ") << iNumEvents << newl;
    hOutput << _T("Event Bytes: ") << iEventBytes << newl;

    hOutput << _T("Reliable Game Data Bytes: ") << iReliableGameDataBytes << newl;
    hOutput << _T("Game Data Bytes: ") << iGameDataBytes << newl;
    hOutput << _T("State String Bytes: ") << iStateStringBytes << newl << newl;
    hOutput << _T("Entity Dynamic Changes: ") << iEntityDynamicChanges << newl << newl;

    hOutput << _T("Name                               Count     Bytes    Header     Flags    Fields") << newl;
    hOutput << _T("================================================================================") << newl;

    hOutput << XtoA(_TS("Entity Snapshots"), FMT_ALIGNLEFT, 31)
            << XtoA(iNumEntitySnapshots, 0, 9) << _T(" ")
            << XtoA(iEntitySnapshotBytes, 0, 9) << _T(" ")
            << XtoA(iEntityHeaderBytes, 0, 9) << _T(" ")
            << XtoA(iEntityTransmitFlagBytes, 0, 9) << _T(" ")
            << XtoA(iEntityFieldBytes, 0, 9) << newl;

    SProfileEntitySnapshot &cEntityDeleteProfile(mapProfileEntity[0]);

    hOutput << XtoA(_TS("Delete"), FMT_ALIGNLEFT, 31)
            << XtoA(cEntityDeleteProfile.uiCount, 0, 9) << _T(" ")
            << XtoA(cEntityDeleteProfile.uiSnapshotBytes, 0, 9) << _T(" ")
            << XtoA(cEntityDeleteProfile.uiHeaderBytes, 0, 9) << _T(" ")
            << XtoA(cEntityDeleteProfile.uiTransmitFlagBytes, 0, 9) << _T(" ")
            << XtoA(cEntityDeleteProfile.uiFieldBytes, 0, 9) << newl << newl;

    map<ushort, tstring> mapEntities;
    EntityRegistry.GetEntityList(mapEntities);
    for (map<ushort, tstring>::iterator it(mapEntities.begin()); it != mapEntities.end(); ++it)
    {
        ushort unType(it->first);
        if (mapProfileEntity.find(unType) == mapProfileEntity.end())
            continue;

        SProfileEntitySnapshot &cEntityProfile(mapProfileEntity[unType]);

        hOutput << XtoA(it->second.substr(0, 30), FMT_ALIGNLEFT, 31)
                << XtoA(cEntityProfile.uiCount, 0, 9) << _T(" ")
                << XtoA(cEntityProfile.uiSnapshotBytes, 0, 9) << _T(" ")
                << XtoA(cEntityProfile.uiHeaderBytes, 0, 9) << _T(" ")
                << XtoA(cEntityProfile.uiTransmitFlagBytes, 0, 9) << _T(" ")
                << XtoA(cEntityProfile.uiFieldBytes, 0, 9) << newl;
    }

    map<uint, tstring> mapBaseNames;
    map<uint, const TypeVector *> mapBaseTypeVectors;
    for (map<ushort, tstring>::iterator it(mapEntities.begin()); it != mapEntities.end(); ++it)
    {
        const CDynamicEntityAllocator *pDynamicAllocator(EntityRegistry.GetDynamicAllocator(it->first));
        if (pDynamicAllocator == NULL)
            continue;

        mapBaseNames[pDynamicAllocator->GetBaseType()] = pDynamicAllocator->GetBaseTypeName();
        mapBaseTypeVectors[pDynamicAllocator->GetBaseType()] = pDynamicAllocator->GetTypeVector();
    }

    for (map<uint, SProfileEntitySnapshot>::iterator it(mapProfileEntityDynamic.begin()); it != mapProfileEntityDynamic.end(); ++it)
    {
        SProfileEntitySnapshot &cEntityProfile(it->second);

        hOutput << XtoA(mapBaseNames[it->first].substr(0, 30), FMT_ALIGNLEFT, 31)
                << XtoA(cEntityProfile.uiCount, 0, 9) << _T(" ")
                << XtoA(cEntityProfile.uiSnapshotBytes, 0, 9) << _T(" ")
                << XtoA(cEntityProfile.uiHeaderBytes, 0, 9) << _T(" ")
                << XtoA(cEntityProfile.uiTransmitFlagBytes, 0, 9) << _T(" ")
                << XtoA(cEntityProfile.uiFieldBytes, 0, 9) << newl;
    }

    map<tstring, SProfileField> mapFields;
    for (map<ushort, tstring>::iterator it(mapEntities.begin()); it != mapEntities.end(); ++it)
    {
        ushort unType(it->first);
        if (mapProfileEntity.find(unType) == mapProfileEntity.end())
            continue;

        SProfileEntitySnapshot &cEntityProfile(mapProfileEntity[unType]);

        const TypeVector *pTypeVector(EntityRegistry.GetTypeVector(unType));

        hOutput << newl << newl;

        hOutput << XtoA(it->second.substr(0, 30), FMT_ALIGNLEFT, 31) 
                << XtoA(_T("Count"), 0, 9)
                << XtoA(_T("Bytes"), 0, 9) << newl;

        hOutput << _T("=================================================") << newl;

        int iCount(0);
        int iBytes(0);

        for (uint ui(0); ui < cEntityProfile.vFieldChanges.size(); ++ui)
        {
            int iSize(0);
            switch ((*pTypeVector)[ui].eDataType)
            {
            case TYPE_CHAR:             iSize = 1;  break;
            case TYPE_SHORT:            iSize = 2;  break;
            case TYPE_INT:              iSize = 4;  break;
            case TYPE_FLOAT:            iSize = 4;  break;
            case TYPE_V2F:              iSize = 8;  break;
            case TYPE_V3F:              iSize = 12; break;
            case TYPE_RESHANDLE:        iSize = 2;  break;
            case TYPE_GAMEINDEX:        iSize = 2;  break;
            case TYPE_ANGLE8:           iSize = 1;  break;
            case TYPE_ANGLE16:          iSize = 2;  break;
            case TYPE_ROUND16:          iSize = 2;  break;
            case TYPE_FLOOR16:          iSize = 2;  break;
            case TYPE_CEIL16:           iSize = 2;  break;
            case TYPE_ROUNDPOS3D:       iSize = 6;  break;
            }

            hOutput << XtoA((*pTypeVector)[ui].sName.substr(0, 30), FMT_ALIGNLEFT, 31)
                    << XtoA(cEntityProfile.vFieldChanges[ui], 0, 9)
                    << XtoA(cEntityProfile.vFieldChanges[ui] * iSize, 0, 9) << newl;

            iCount += cEntityProfile.vFieldChanges[ui];
            iBytes += cEntityProfile.vFieldChanges[ui] * iSize;

            SProfileField &cProfileField(mapFields[(*pTypeVector)[ui].sName]);

            cProfileField.uiCount += cEntityProfile.vFieldChanges[ui];
            cProfileField.uiBytes += cEntityProfile.vFieldChanges[ui] * iSize;
        }

        hOutput << XtoA(_TS("Total"), FMT_ALIGNLEFT, 31) 
                << XtoA(iCount, 0, 9)
                << XtoA(iBytes, 0, 9) << newl;
    }

    for (map<uint, SProfileEntitySnapshot>::iterator it(mapProfileEntityDynamic.begin()); it != mapProfileEntityDynamic.end(); ++it)
    {
        SProfileEntitySnapshot &cEntityProfile(it->second);

        const TypeVector *pTypeVector(mapBaseTypeVectors[it->first]);

        hOutput << newl << newl;

        hOutput << XtoA(mapBaseNames[it->first].substr(0, 30), FMT_ALIGNLEFT, 31) 
                << XtoA(_T("Count"), 0, 9)
                << XtoA(_T("Bytes"), 0, 9) << newl;

        hOutput << _T("=================================================") << newl;

        int iCount(0);
        int iBytes(0);

        for (uint ui(0); ui < cEntityProfile.vFieldChanges.size(); ++ui)
        {
            int iSize(0);
            switch ((*pTypeVector)[ui].eDataType)
            {
            case TYPE_CHAR:             iSize = 1;  break;
            case TYPE_SHORT:            iSize = 2;  break;
            case TYPE_INT:              iSize = 4;  break;
            case TYPE_FLOAT:            iSize = 4;  break;
            case TYPE_V2F:              iSize = 8;  break;
            case TYPE_V3F:              iSize = 12; break;
            case TYPE_RESHANDLE:        iSize = 2;  break;
            case TYPE_GAMEINDEX:        iSize = 2;  break;
            case TYPE_ANGLE8:           iSize = 1;  break;
            case TYPE_ANGLE16:          iSize = 2;  break;
            case TYPE_ROUND16:          iSize = 2;  break;
            case TYPE_FLOOR16:          iSize = 2;  break;
            case TYPE_CEIL16:           iSize = 2;  break;
            case TYPE_ROUNDPOS3D:       iSize = 6;  break;
            }

            hOutput << XtoA((*pTypeVector)[ui].sName.substr(0, 30), FMT_ALIGNLEFT, 31)
                    << XtoA(cEntityProfile.vFieldChanges[ui], 0, 9)
                    << XtoA(cEntityProfile.vFieldChanges[ui] * iSize, 0, 9) << newl;

            iCount += cEntityProfile.vFieldChanges[ui];
            iBytes += cEntityProfile.vFieldChanges[ui] * iSize;

            SProfileField &cProfileField(mapFields[(*pTypeVector)[ui].sName]);

            cProfileField.uiCount += cEntityProfile.vFieldChanges[ui];
            cProfileField.uiBytes += cEntityProfile.vFieldChanges[ui] * iSize;
        }

        hOutput << XtoA(_TS("Total"), FMT_ALIGNLEFT, 31) 
                << XtoA(iCount, 0, 9)
                << XtoA(iBytes, 0, 9) << newl;
    }

    hOutput << newl << newl;

    for (map<tstring, SProfileField>::iterator it(mapFields.begin()); it != mapFields.end(); ++it)
    {
        hOutput << XtoA(it->first.substr(0, 30), FMT_ALIGNLEFT, 31) 
                << XtoA(it->second.uiCount, 0, 9)
                << XtoA(it->second.uiBytes, 0, 9) << newl;
    }

    StopPlayback();

    Console << _T("Done") << newl;
#endif
}


/*====================
  CReplayManager::Parse
  ====================*/
void    CReplayManager::Parse(const tstring &sFilename)
{
#if 1
    if (!StartPlayback(sFilename, false))
        return;

    int iFrame(0);

    //CFileHandle hKeyFrames(_T("~/") + Filename_StripExtension(sFilename) + _T(".tmp"), FILE_WRITE | FILE_BINARY | FILE_UTF16);

    CBufferBit cBuffer(0x4000);
    CSnapshot snapshot;

    while (!m_hReplayData.IsEOF())
    {
        uint uiLength(m_hReplayData.ReadInt32());
        cBuffer.Clear();

        cBuffer.Reserve(uiLength);
        cBuffer.SetLength(uiLength);

        if (uiLength == 0)
        {
            assert(m_hReplayData.IsEOF());
            // TODO: Why isn't m_hReplayData.IsEOF() true before now?
            break;
        }

        m_hReplayData.Read((char *)cBuffer.Get(), uiLength);

        ReadSnapshot(iFrame, cBuffer);

        uint uiNumClients;

        // Read reliable game data
        uiNumClients = m_hReplayData.ReadInt32();
        for (uint ui(0); ui < uiNumClients; ++ui)
        {
            m_hReplayData.ReadInt32(); // uiIndex
            uint uiLength(m_hReplayData.ReadInt32());

            if (uiLength > 0)
                m_hReplayData.Seek(uiLength, SEEK_ORIGIN_CURRENT);
        }

        // Read game data
        uiNumClients = m_hReplayData.ReadInt32();
        for (uint ui(0); ui < uiNumClients; ++ui)
        {
            m_hReplayData.ReadInt32(); // uiIndex
            uint uiLength(m_hReplayData.ReadInt32());

            if (uiLength > 0)
                m_hReplayData.Seek(uiLength, SEEK_ORIGIN_CURRENT);
        }

        // Read state strings
        uint uiNumStateStrings(m_hReplayData.ReadInt32());

        if (uiNumStateStrings > 0)
        {
            if (m_vStateStringStore.size() > 0)
            {
                // Copy last element
                m_vStateStringStore.push_back(svector());
                m_vStateStringStore.back() = m_vStateStringStore[m_vStateStringStore.size() - 2];

            }
            else
            {
                // Copy default
                m_vStateStringStore.push_back(svector());
            }

            for (uint ui(0); ui < uiNumStateStrings; ++ui)
            {
                uint uiID(m_hReplayData.ReadInt32()); // uiID

                string &sStr(m_vStateStringStore.back()[uiID]);

                sStr.clear();

                char cChar(m_hReplayData.ReadByte());
                while (cChar)
                {
                    cChar = m_hReplayData.ReadByte();
                    sStr += cChar;
                }
            }
        }

        // Read state blocks
        if (m_uiReplayVersion > 3)
        {
            uint uiNumStateBlocks(m_hReplayData.ReadInt32());

            if (uiNumStateBlocks > 0)
            {
                if (m_vStateBlockStore.size() > 0)
                {
                    // Copy last element
                    m_vStateBlockStore.push_back(MapStateBlock());
                    m_vStateBlockStore.back() = m_vStateBlockStore[m_vStateBlockStore.size() - 2];

                }
                else
                {
                    // Copy default
                    m_vStateBlockStore.push_back(MapStateBlock());
                }

                for (uint ui(0); ui < uiNumStateBlocks; ++ui)
                {
                    uint uiID(m_hReplayData.ReadInt32());

                    CBufferDynamic &buffer(m_vStateBlockStore.back()[uiID]);

                    uint uiLength(m_hReplayData.ReadInt32());

                    buffer.Clear();

                    if (uiLength == 0)
                        continue;

                    buffer.Reserve(uiLength);
                    m_hReplayData.Read(buffer.Lock(uiLength), uiLength);
                }
            }
        }

        float fProgress(float(m_hReplayData.Tell()) / m_hReplayData.GetLength());
        if (iFrame % 100 == 0)
        {
//            IModalDialog::SetProgress(fProgress);
//            IModalDialog::Update();

            Console << _T("frame: ") << iFrame << SPACE;
            Console << _T("progress: ") << fProgress << SPACE;
            Console << newl;

            cmdPrintGameInfo();
        }

        ++iFrame;
    }

    StopPlayback();

    Console << _T("Done") << newl;
#endif
}


/*====================
  CReplayManager::GenerateKeyFrames
  ====================*/
void    CReplayManager::GenerateKeyFrames()
{
    int iFrame(0);

    CBufferBit cBuffer(0x4000);
    CSnapshot snapshot;
    
    while (!m_hReplayData.IsEOF())
    {
        uint uiLength(m_hReplayData.ReadInt32());
        if (!uiLength)
            continue;

        cBuffer.Clear();

        cBuffer.Reserve(uiLength);
        m_hReplayData.Read(cBuffer.Lock(uiLength), uiLength);

        snapshot.ReadBuffer(cBuffer);

        if (m_uiBeginTime == INVALID_TIME)
            m_uiBeginTime = snapshot.GetTimeStamp();

        m_cCurrentSnapshot.SetValid(true);
        m_cCurrentSnapshot.SetFrameNumber(snapshot.GetFrameNumber());
        m_cCurrentSnapshot.SetPrevFrameNumber(uint(-1));
        m_cCurrentSnapshot.SetTimeStamp(snapshot.GetTimeStamp());
        
        // Clear events
        m_cCurrentSnapshot.SetNumEvents(0);
        m_cCurrentSnapshot.GetEventBuffer().Clear();

        byte yNumEvents(snapshot.GetNumEvents());

        // Translate events
        for (int i(0); i < yNumEvents; ++i)
            CGameEvent::AdvanceBuffer2(snapshot.GetReceivedBuffer());

        // Read bit entities
        // Baseline active, might change later
        uivector &vBitEntityBuffer(m_cCurrentSnapshot.GetStreamBitEntityBuffer(0));
        vBitEntityBuffer.resize(m_uiNumBitEntityFields, uint(-1));

        CTransmitFlags<8> cTransmitFlags(m_uiNumBitEntityFields);

        cTransmitFlags.ReadTransmitFlags(snapshot.GetReceivedBuffer());

        for (uint uiField(0); uiField < m_uiNumBitEntityFields; ++uiField)
        {
            if (cTransmitFlags.IsFieldSet(uiField))
                vBitEntityBuffer[uiField] = snapshot.GetReceivedBuffer().ReadBits(32);
        }

        SnapshotVector &vBaseEntities(m_cCurrentSnapshot.GetEntities());
        SnapshotVector_it citBase(vBaseEntities.begin());

        static CEntitySnapshot entSnapshot;

        uint uiLastIndex(0);

        bool bError(false);

        // Translate entities
        while (!bError)
        {
            const SReplayEntityDesc* pTypeDesc(NULL);

            // Grab a "shell" entity snapshot from the the frame snapshot.
            // The data will be filled in once we know the type.
            entSnapshot.Clear();
            if (!snapshot.GetNextEntity(entSnapshot, uiLastIndex))
                break;

            if (!entSnapshot.GetChanged())
            {
                Console.Err << _T("Invalid entity snapshot") << newl;
                bError = true;
                break;
            }

            if (entSnapshot.GetIndex() == INVALID_INDEX)
            {
                // Look for the index of an implied change
                while (citBase != vBaseEntities.end())
                {
                    CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->second));
                    if (pBaseSnapshot->GetChanging())
                    {
                        entSnapshot.SetIndex(citBase->first);
                        uiLastIndex = citBase->first;
                        break;
                    }
                    
                    ++citBase;
                }
                if (citBase == vBaseEntities.end())
                {
                    if (snapshot.GetReceivedBuffer().GetUnreadBits() >= 8)
                    {
                        Console.Err << snapshot.GetReceivedBuffer().GetUnreadBits() << _T(" unread bits") << newl;
                        bError = true;
                    }
                    
                    break; // End of snapshot
                }
            }
            else
            {
                while (citBase != vBaseEntities.end() && citBase->first < entSnapshot.GetIndex())
                    ++citBase;
            }

            if (citBase == vBaseEntities.end() || citBase->first > entSnapshot.GetIndex())
            {
                //
                // New entity, read from baseline
                //

                ushort unType(entSnapshot.GetType());

                // If the type is NULL, the entity is dead and should be removed
                if (unType == 0)
                    continue;

                pTypeDesc = GetTypeDesc(unType);
                if (pTypeDesc == NULL)
                {
                    Console.Err << _T("Unknown entity type, bad snapshot") << newl;
                    bError = true;
                    break;
                }

                //Console << iFrame << _T(": Create ") << entSnapshot.GetIndex() << _T(" ") << entSnapshot.GetType() << newl;

                entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), pTypeDesc->cFieldTypes, pTypeDesc->uiSize, &pTypeDesc->cBaseline);
                citBase = vBaseEntities.insert(citBase, SnapshotEntry(entSnapshot.GetIndex(), CEntitySnapshot::Allocate(entSnapshot)));
                ++citBase;
            }
            else if (citBase->first == entSnapshot.GetIndex())
            {
                //
                // Update existing entity
                //

                CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->second));
                ushort unType(entSnapshot.GetTypeChange() ? entSnapshot.GetType() : pBaseSnapshot->GetType());

                // If the type is NULL, the entity is dead and should be removed
                if (unType == 0)
                {
                    CEntitySnapshot::DeleteByHandle(citBase->second);
                    citBase = vBaseEntities.erase(citBase);
                    continue;
                }

                pTypeDesc = GetTypeDesc(unType);
                if (pTypeDesc == NULL)
                {
                    Console.Err << _T("Unknown entity type, bad snapshot") << newl;
                    bError = true;
                    break;
                }

                if (entSnapshot.GetTypeChange())
                {
                    entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), pTypeDesc->cFieldTypes, pTypeDesc->uiSize, &pTypeDesc->cBaseline);
                    *pBaseSnapshot = entSnapshot;
                }
                else
                {
                    entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), pTypeDesc->cFieldTypes, pTypeDesc->uiSize);
                    pBaseSnapshot->ApplyDiff(entSnapshot);
                }
                ++citBase;
            }
        }

        if (bError)
        {
            m_hReplayData.Seek(0, SEEK_ORIGIN_END);
            break;
        }

        m_cCurrentSnapshot.SetNumStreams(3);
        m_cCurrentSnapshot.ClearStreams();
        
        SnapshotVector &SnapshotsThis(m_cCurrentSnapshot.GetEntities());

        SnapshotVector_cit citThis(SnapshotsThis.begin()), citThisEnd(SnapshotsThis.end());
        for (; citThis != citThisEnd; ++citThis)
        {
            CEntitySnapshot *pThisSnapshot(CEntitySnapshot::GetByHandle(citThis->second));

            pThisSnapshot->SetPublicSequence(1); // Something different than default

            uint uiFlags(pThisSnapshot->GetChanging() ? ENTITY_STREAM_FLAG_CHANGING : 0);
            
            m_cCurrentSnapshot.AddStreamEntity(0, citThis->first, citThis->second, uiFlags);
            m_cCurrentSnapshot.AddStreamEntity(1, citThis->first, citThis->second, uiFlags);
            m_cCurrentSnapshot.AddStreamEntity(2, citThis->first, citThis->second, uiFlags);
        }

        uint uiNumClients;

        // Read reliable game data
        uiNumClients = m_hReplayData.ReadInt32();
        for (uint ui(0); ui < uiNumClients; ++ui)
        {
            m_hReplayData.ReadInt32(); // uiIndex
            uint uiLength(m_hReplayData.ReadInt32());

            if (uiLength > 0)
                m_hReplayData.Seek(uiLength, SEEK_ORIGIN_CURRENT);
        }

        // Read game data
        uiNumClients = m_hReplayData.ReadInt32();
        for (uint ui(0); ui < uiNumClients; ++ui)
        {
            m_hReplayData.ReadInt32(); // uiIndex
            uint uiLength(m_hReplayData.ReadInt32());

            if (uiLength > 0)
                m_hReplayData.Seek(uiLength, SEEK_ORIGIN_CURRENT);
        }

        // Read state strings
        uint uiNumStateStrings(m_hReplayData.ReadInt32());

        if (uiNumStateStrings > 0)
        {
            if (m_vStateStringStore.size() > 0)
            {
                // Copy last element
                m_vStateStringStore.push_back(svector());
                m_vStateStringStore.back() = m_vStateStringStore[m_vStateStringStore.size() - 2];
                
            }
            else
            {
                // Copy default
                m_vStateStringStore.push_back(svector());
            }

            for (uint ui(0); ui < uiNumStateStrings; ++ui)
            {
                uint uiID(m_hReplayData.ReadInt32()); // uiID

                string &sStr(m_vStateStringStore.back()[uiID]);

                sStr.clear();

                char cChar(m_hReplayData.ReadByte());
                while (cChar)
                {
                    cChar = m_hReplayData.ReadByte();
                    sStr += cChar;
                }
            }
        }

        // Read state blocks
        if (m_uiReplayVersion > 3)
        {
            uint uiNumStateBlocks(m_hReplayData.ReadInt32());

            if (uiNumStateBlocks > 0)
            {
                if (m_vStateBlockStore.size() > 0)
                {
                    // Copy last element
                    m_vStateBlockStore.push_back(MapStateBlock());
                    m_vStateBlockStore.back() = m_vStateBlockStore[m_vStateBlockStore.size() - 2];
                    
                }
                else
                {
                    // Copy default
                    m_vStateBlockStore.push_back(MapStateBlock());
                }

                for (uint ui(0); ui < uiNumStateBlocks; ++ui)
                {
                    uint uiID(m_hReplayData.ReadInt32());

                    CBufferDynamic &buffer(m_vStateBlockStore.back()[uiID]);

                    uint uiLength(m_hReplayData.ReadInt32());

                    buffer.Clear();

                    if (uiLength == 0)
                        continue;

                    buffer.Reserve(uiLength);
                    m_hReplayData.Read(buffer.Lock(uiLength), uiLength);
                }
            }
        }

        if (iFrame % 100 == 0)
        {
            SReplayKeyFrame &cKeyFrame = m_mapKeyFrames[iFrame] = SReplayKeyFrame();

            cKeyFrame.uiFrame = iFrame;
            cKeyFrame.zPos = m_hReplayData.Tell();

            if (m_vStateStringStore.size() > 0)
                cKeyFrame.uiStateString = uint(m_vStateStringStore.size() - 1);
            else
                cKeyFrame.uiStateString = INVALID_INDEX;

            if (m_vStateBlockStore.size() > 0)
                cKeyFrame.uiStateBlock = uint(m_vStateBlockStore.size() - 1);
            else
                cKeyFrame.uiStateBlock = INVALID_INDEX;

            m_cCurrentSnapshot.WriteBuffer(cKeyFrame.cBuffer, 0, 0);

            IModalDialog::SetProgress(float(m_hReplayData.Tell()) / m_hReplayData.GetLength());
            IModalDialog::Update();
        }

        ++iFrame;
    }

    m_uiNumFrames = iFrame - 1;
    m_uiEndTime = m_cCurrentSnapshot.GetTimeStamp();

    m_hReplayData.Seek(int(m_zStartPos));

    m_cCurrentSnapshot.SetFrameNumber(uint(-1));
    m_cCurrentSnapshot.SetPrevFrameNumber(uint(-1));
    m_cCurrentSnapshot.SetTimeStamp(0);
    m_cCurrentSnapshot.FreeEntities();
}


/*====================
  CReplayManager::Encode
  ====================*/
void    CReplayManager::Encode(const tstring &sInFilename, const tstring &sOutFilename)
{
#if 0
    if (!StartPlayback(sInFilename, false))
        return;

    // Start new ReplayData
    CFileHandle hOutReplayData(Filename_StripExtension(sOutFilename) + _T(".tmp"), FILE_WRITE | FILE_BINARY | FILE_UTF16);

    if (!hOutReplayData.IsOpen())
        return;

    // File type code
    hOutReplayData.WriteByte('S');
    hOutReplayData.WriteByte('2');
    hOutReplayData.WriteByte('R');
    hOutReplayData.WriteByte('2');

    // Version
    hOutReplayData.WriteInt32(REPLAY_VERSION);

    // Write world name as a wide string
    const wstring &sWorldName(TStringToWString(m_sWorldName));
    hOutReplayData.Write(sWorldName.c_str(), sWorldName.length() * sizeof(wstring::value_type));
    hOutReplayData.WriteInt16(short(0));

    // Write state strings
    for (uint uiID(1); uiID != 4; ++uiID)
    {
        tstring sBuffer;
        Game.GetStateString(uiID).AppendToBuffer(sBuffer);

        const wstring &sStateString(TStringToWString(sBuffer));
        hOutReplayData.Write(sStateString.c_str(), sStateString.length() * sizeof(wstring::value_type));
        hOutReplayData.WriteInt16(short(0));
    }

    // Write state blocks
    for (uint uiID(1); uiID != 3; ++uiID)
    {
        CBufferDynamic &cBuffer(Game.GetStateBlock(uiID).GetBuffer());

        hOutReplayData.WriteInt32(cBuffer.GetLength());
        hOutReplayData.Write(cBuffer.Get(), cBuffer.GetLength());
    }

    m_cLastSnapshot = CSnapshot();

    m_cLastSnapshot.GetBitEntityBuffer().resize(m_uiNumBitEntityFields, uint(-1));

    int iFrame(0);

    CBufferStatic cBuffer(0x4000);
    CSnapshot snapshot;

    while (!m_hReplayData.IsEOF() && iFrame <= 48000)
    {
        uint uiLength(m_hReplayData.ReadInt32());
        cBuffer.Clear();

        cBuffer.Reserve(uiLength);
        cBuffer.SetLength(uiLength);
        m_hReplayData.Read((char *)cBuffer.Get(), uiLength);

        snapshot.ReadBuffer(cBuffer);

        m_cCurrentSnapshot.SetValid(true);
        m_cCurrentSnapshot.SetFrameNumber(snapshot.GetFrameNumber());
        m_cCurrentSnapshot.SetPrevFrameNumber(uint(-1));
        m_cCurrentSnapshot.SetTimeStamp(snapshot.GetTimeStamp());
        
        // Clear events
        m_cCurrentSnapshot.SetNumEvents(0);
        m_cCurrentSnapshot.GetEventBuffer().Clear();

        byte yNumEvents(snapshot.GetNumEvents());

        // Translate events
        CBufferDynamic bufferTranslate(40);
        for (int i(0); i < yNumEvents; ++i)
        {
            CGameEvent::Translate(snapshot.GetReceivedBuffer(), bufferTranslate);
            m_cCurrentSnapshot.AddEventSnapshot(bufferTranslate);
        }

        uivector &vBitEntityBuffer(m_cCurrentSnapshot.GetBitEntityBuffer());
        vBitEntityBuffer.resize(m_uiNumBitEntityFields, uint(-1));

        CTransmitFlags<8> cTransmitFlags(m_uiNumBitEntityFields);

        cTransmitFlags.ReadTransmitFlags(snapshot.GetReceivedBuffer());

        for (uint uiField(0); uiField < m_uiNumBitEntityFields; ++uiField)
        {
            if (cTransmitFlags.IsFieldSet(uiField))
                vBitEntityBuffer[uiField] = snapshot.GetReceivedBuffer().ReadInt();
        }

        SnapshotVector &vBaseEntities(m_cCurrentSnapshot.GetEntities());
        SnapshotVector_it citBase(vBaseEntities.begin());

        static CEntitySnapshot entSnapshot;

        bool bError(false);

        // Translate entities
        while (!bError)
        {
            const TypeVector *pTypeVector(NULL);

            // Grab a "shell" entity snapshot from the the frame snapshot.
            // The data will be filled in once we know the type.
            entSnapshot.Clear();
            if (!snapshot.GetNextEntity(entSnapshot, -1))
                break;

            while (citBase != vBaseEntities.end() && citBase->first < entSnapshot.GetIndex())
                ++citBase;

            if (citBase == vBaseEntities.end() || citBase->first > entSnapshot.GetIndex())
            {
                //
                // New entity, read from baseline
                //

                ushort unType(entSnapshot.GetType());

                // If the type is NULL, the entity is dead and should be removed
                if (unType == 0)
                    continue;

                pTypeVector = EntityRegistry.GetTypeVector(unType);
                if (pTypeVector == NULL)
                {
                    Console.Err << _T("Unknown entity type, bad snapshot") << newl;
                    bError = true;
                    break;
                }

                entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), *pTypeVector, EntityRegistry.GetBaseline(unType));
                citBase = vBaseEntities.insert(citBase, SnapshotEntry(entSnapshot.GetIndex(), CEntitySnapshot::Allocate(entSnapshot)));
                ++citBase;
            }
            else if (citBase->first == entSnapshot.GetIndex())
            {
                //
                // Update existing entity
                //

                CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->second));
                ushort unType(entSnapshot.GetTypeChange() ? entSnapshot.GetType() : pBaseSnapshot->GetType());

                // If the type is NULL, the entity is dead and should be removed
                if (unType == 0)
                {
                    CEntitySnapshot::DeleteByHandle(citBase->second);
                    citBase = vBaseEntities.erase(citBase);
                    continue;
                }

                pTypeVector = EntityRegistry.GetTypeVector(unType);
                if (pTypeVector == NULL)
                {
                    Console.Err << _T("Unknown entity type, bad snapshot") << newl;
                    bError = true;
                    break;
                }

                if (entSnapshot.GetTypeChange())
                {
                    entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), *pTypeVector, EntityRegistry.GetBaseline(unType));
                    *pBaseSnapshot = entSnapshot;
                }
                else
                {
                    entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), *pTypeVector);
                    pBaseSnapshot->ApplyDiff(entSnapshot);
                }
                ++citBase;
            }
        }

        if (bError)
        {
            m_hReplayData.Seek(0, SEEK_ORIGIN_END);
            break;
        }

        uint uiNumClients;

        // Read reliable game data
        uiNumClients = m_hReplayData.ReadInt32();
        for (uint ui(0); ui < uiNumClients; ++ui)
        {
            uint uiIndex(m_hReplayData.ReadInt32());
            uint uiLength(m_hReplayData.ReadInt32());

            if (uiLength > 0)
            {
                CBufferStatic cBuffer(uiLength);

                for (uint uiRead(0); uiRead < uiLength; ++uiRead)
                    cBuffer << m_hReplayData.ReadByte();

                m_mapGameDataReliable[uiIndex].Append(cBuffer.Get(), cBuffer.GetLength());
            }
        }

        // Read game data
        uiNumClients = m_hReplayData.ReadInt32();
        for (uint ui(0); ui < uiNumClients; ++ui)
        {
            uint uiIndex(m_hReplayData.ReadInt32());
            uint uiLength(m_hReplayData.ReadInt32());

            if (uiLength > 0)
            {
                CBufferStatic cBuffer(uiLength);

                for (uint uiRead(0); uiRead < uiLength; ++uiRead)
                    cBuffer << m_hReplayData.ReadByte();

                m_mapGameData[uiIndex].Append(cBuffer.Get(), cBuffer.GetLength());
            }
        }

        // Read state strings
        uint uiNumStateString(m_hReplayData.ReadInt32());
        for (uint ui(0); ui < uiNumStateString; ++ui)
        {
            uint uiID(m_hReplayData.ReadInt32());
            tstring sStr;

            wchar_t wChar(m_hReplayData.ReadInt16());
            while (wChar)
            {
                sStr += TCHAR(wChar);
                wChar = m_hReplayData.ReadInt16();
            }

            m_mapStateStrings[uiID] = sStr;
        }

        // Write game snapshot
        {
            m_cCurrentSnapshot.CalcSequence(m_cLastSnapshot);

            CBufferBit cBufferFrameData;
            m_cCurrentSnapshot.WriteDiff(cBufferFrameData, m_cLastSnapshot, 0, 0, -1);

            hOutReplayData.WriteInt32(cBufferFrameData.GetLength());
            hOutReplayData.Write(cBufferFrameData.Get(), cBufferFrameData.GetLength());

            // Write reliable game data
            uint uiNumClientsReliable(0);
            for (MapClientGameData::iterator it(m_mapGameDataReliable.begin()); it != m_mapGameDataReliable.end(); ++it)
            {
                if (it->second.GetLength() > 0)
                    ++uiNumClientsReliable;
            }

            hOutReplayData.WriteInt32(uiNumClientsReliable);
            for (MapClientGameData::iterator it(m_mapGameDataReliable.begin()); it != m_mapGameDataReliable.end(); ++it)
            {
                if (it->second.GetLength() == 0)
                    continue;

                hOutReplayData.WriteInt32(it->first);
                hOutReplayData.WriteInt32(it->second.GetLength());
                hOutReplayData.Write(it->second.Get(), it->second.GetLength());

                it->second.Clear();
            }

            // Write game data
            uint uiNumClients(0);
            for (MapClientGameData::iterator it(m_mapGameData.begin()); it != m_mapGameData.end(); ++it)
            {
                if (it->second.GetLength() > 0)
                    ++uiNumClients;
            }

            hOutReplayData.WriteInt32(uiNumClients);
            for (MapClientGameData::iterator it(m_mapGameData.begin()); it != m_mapGameData.end(); ++it)
            {
                if (it->second.GetLength() == 0)
                    continue;

                hOutReplayData.WriteInt32(it->first);
                hOutReplayData.WriteInt32(it->second.GetLength());
                hOutReplayData.Write(it->second.Get(), it->second.GetLength());

                it->second.Clear();
            }

            // Write state strings
            hOutReplayData.WriteInt32(uint(m_mapStateStrings.size()));
            for (MapStateString::iterator it(m_mapStateStrings.begin()); it != m_mapStateStrings.end(); ++it)
            {
                hOutReplayData.WriteInt32(it->first);

                const wstring &sStateString(TStringToWString(it->second));
                hOutReplayData.Write(sStateString.c_str(), sStateString.length() * sizeof(wstring::value_type));
                hOutReplayData.WriteInt16(short(0));
            }
            m_mapStateStrings.clear();

            m_cLastSnapshot = m_cCurrentSnapshot;
        }

        ++iFrame;
    }

    StopPlayback();

    // Write new archive
    hOutReplayData.Close();

    hOutReplayData.Open(Filename_StripExtension(sOutFilename) + _T(".tmp"), FILE_READ | FILE_BINARY | FILE_UTF16);

    if (hOutReplayData.IsOpen())
    {
        tstring sPath(sOutFilename);
        CArchive hArchive(sPath, ARCHIVE_WRITE | ARCHIVE_MAX_COMPRESS);
        if (!hArchive.IsOpen())
            return;

        uint uiSize;
        const char *pBuffer(hOutReplayData.GetBuffer(uiSize));

        hArchive.WriteFile(_T("ReplayData"), pBuffer, uiSize);

        hOutReplayData.Close();
        hArchive.Close();

        //FileManager.Delete(Filename_StripExtension(sOutFilename) + _T(".tmp"));
    }

    Console << _T("Done") << newl;
#endif
}


/*====================
  CReplayManager::Encode2
  ====================*/
void    CReplayManager::Encode2(const tstring &sInFilename, const tstring &sOutFilename)
{
#if 0
    if (!StartPlayback(sInFilename, false))
        return;

    // Start new ReplayData
    CFileHandle hOutReplayData(Filename_StripExtension(sOutFilename) + _T(".tmp"), FILE_WRITE | FILE_BINARY | FILE_UTF16);

    if (!hOutReplayData.IsOpen())
        return;

    // File type code
    hOutReplayData.WriteByte('S');
    hOutReplayData.WriteByte('2');
    hOutReplayData.WriteByte('R');
    hOutReplayData.WriteByte('2');

    // Version
    hOutReplayData.WriteInt32(REPLAY_VERSION);

    // Write world name as a wide string
    const wstring &sWorldName(TStringToWString(m_sWorldName));
    hOutReplayData.Write(sWorldName.c_str(), sWorldName.length() * sizeof(wstring::value_type));
    hOutReplayData.WriteInt16(short(0));

    // Write state strings
    for (uint uiID(1); uiID != 4; ++uiID)
    {
        tstring sBuffer;
        Game.GetStateString(uiID).AppendToBuffer(sBuffer);

        const wstring &sStateString(TStringToWString(sBuffer));
        hOutReplayData.Write(sStateString.c_str(), sStateString.length() * sizeof(wstring::value_type));
        hOutReplayData.WriteInt16(short(0));
    }

    // Write state blocks
    for (uint uiID(1); uiID != 3; ++uiID)
    {
        CBufferDynamic &cBuffer(Game.GetStateBlock(uiID).GetBuffer());

        hOutReplayData.WriteInt32(cBuffer.GetLength());
        hOutReplayData.Write(cBuffer.Get(), cBuffer.GetLength());
    }

    m_cLastSnapshot = CSnapshot();

    m_cLastSnapshot.GetBitEntityBuffer().resize(m_uiNumBitEntityFields, uint(-1));

    int iFrame(0);

    CBufferBit cBuffer(0x4000);
    CSnapshot snapshot;

    while (!m_hReplayData.IsEOF() && iFrame <= 48000)
    {
        uint uiLength(m_hReplayData.ReadInt32());
        cBuffer.Clear();

        cBuffer.Reserve(uiLength);
        cBuffer.SetLength(uiLength);
        m_hReplayData.Read((char *)cBuffer.Get(), uiLength);

        snapshot.ReadBuffer(cBuffer);

        m_cCurrentSnapshot.SetValid(true);
        m_cCurrentSnapshot.SetFrameNumber(snapshot.GetFrameNumber());
        m_cCurrentSnapshot.SetPrevFrameNumber(uint(-1));
        m_cCurrentSnapshot.SetTimeStamp(snapshot.GetTimeStamp());
        
        // Clear events
        m_cCurrentSnapshot.SetNumEvents(0);
        m_cCurrentSnapshot.GetEventBuffer().Clear();

        byte yNumEvents(snapshot.GetNumEvents());

        // Translate events
        CBufferDynamic bufferTranslate(40);
        for (int i(0); i < yNumEvents; ++i)
        {
            CGameEvent::Translate2(snapshot.GetReceivedBuffer(), bufferTranslate);
            m_cCurrentSnapshot.AddEventSnapshot(bufferTranslate);
        }

        uivector &vBitEntityBuffer(m_cCurrentSnapshot.GetBitEntityBuffer());
        vBitEntityBuffer.resize(m_uiNumBitEntityFields, uint(-1));

        CTransmitFlags<8> cTransmitFlags(m_uiNumBitEntityFields);

        cTransmitFlags.ReadTransmitFlags(snapshot.GetReceivedBuffer());

        for (uint uiField(0); uiField < m_uiNumBitEntityFields; ++uiField)
        {
            if (cTransmitFlags.IsFieldSet(uiField))
                vBitEntityBuffer[uiField] = snapshot.GetReceivedBuffer().ReadBits(32);
        }

        SnapshotVector &vBaseEntities(m_cCurrentSnapshot.GetEntities());
        SnapshotVector_it citBase(vBaseEntities.begin());

        static CEntitySnapshot entSnapshot;

        bool bError(false);
        uint uiLastIndex(0);

        // Translate entities
        while (!bError)
        {
            const TypeVector *pTypeVector(NULL);

            // Grab a "shell" entity snapshot from the the frame snapshot.
            // The data will be filled in once we know the type.
            entSnapshot.Clear();
            if (!snapshot.GetNextEntity(entSnapshot, uiLastIndex))
                break; // End of snapshot

            if (entSnapshot.GetIndex() == INVALID_INDEX)
            {
                // Look for the index of an implied change
                while (citBase != vBaseEntities.end())
                {
                    CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->second));
                    if (pBaseSnapshot->GetChanging())
                    {
                        entSnapshot.SetIndex(citBase->first);
                        uiLastIndex = citBase->first;
                        break;
                    }
                    
                    ++citBase;
                }
                if (citBase == vBaseEntities.end())
                    break; // End of snapshot
            }
            else
            {
                while (citBase != vBaseEntities.end() && citBase->first < entSnapshot.GetIndex())
                    ++citBase;
            }

            if (citBase == vBaseEntities.end() || citBase->first > entSnapshot.GetIndex())
            {
                //
                // New entity, read from baseline
                //

                ushort unType(entSnapshot.GetType());

                // If the type is NULL, the entity is dead and should be removed
                if (unType == 0)
                    continue;

                pTypeVector = EntityRegistry.GetTypeVector(unType);
                if (pTypeVector == NULL)
                {
                    Console.Err << _T("Unknown entity type, bad snapshot") << newl;
                    bError = true;
                    break;
                }

                entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), *pTypeVector, EntityRegistry.GetBaseline(unType));
                citBase = vBaseEntities.insert(citBase, SnapshotEntry(entSnapshot.GetIndex(), CEntitySnapshot::Allocate(entSnapshot)));
                ++citBase;
            }
            else if (citBase->first == entSnapshot.GetIndex())
            {
                //
                // Update existing entity
                //

                CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->second));
                ushort unType(entSnapshot.GetTypeChange() ? entSnapshot.GetType() : pBaseSnapshot->GetType());

                // If the type is NULL, the entity is dead and should be removed
                if (unType == 0)
                {
                    CEntitySnapshot::DeleteByHandle(citBase->second);
                    citBase = vBaseEntities.erase(citBase);
                    continue;
                }

                pTypeVector = EntityRegistry.GetTypeVector(unType);
                if (pTypeVector == NULL)
                {
                    Console.Err << _T("Unknown entity type, bad snapshot") << newl;
                    bError = true;
                    break;
                }

                if (entSnapshot.GetTypeChange())
                {
                    entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), *pTypeVector, EntityRegistry.GetBaseline(unType));
                    *pBaseSnapshot = entSnapshot;
                }
                else
                {
                    entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), *pTypeVector);
                    pBaseSnapshot->ApplyDiff(entSnapshot);
                }
                ++citBase;
            }
        }

        if (bError)
        {
            m_hReplayData.Seek(0, SEEK_ORIGIN_END);
            break;
        }

        uint uiNumClients;

        // Read reliable game data
        uiNumClients = m_hReplayData.ReadInt32();
        for (uint ui(0); ui < uiNumClients; ++ui)
        {
            uint uiIndex(m_hReplayData.ReadInt32());
            uint uiLength(m_hReplayData.ReadInt32());

            if (uiLength > 0)
            {
                CBufferStatic cBuffer(uiLength);

                for (uint uiRead(0); uiRead < uiLength; ++uiRead)
                    cBuffer << m_hReplayData.ReadByte();

                m_mapGameDataReliable[uiIndex].Append(cBuffer.Get(), cBuffer.GetLength());
            }
        }

        // Read game data
        uiNumClients = m_hReplayData.ReadInt32();
        for (uint ui(0); ui < uiNumClients; ++ui)
        {
            uint uiIndex(m_hReplayData.ReadInt32());
            uint uiLength(m_hReplayData.ReadInt32());

            if (uiLength > 0)
            {
                CBufferStatic cBuffer(uiLength);

                for (uint uiRead(0); uiRead < uiLength; ++uiRead)
                    cBuffer << m_hReplayData.ReadByte();

                m_mapGameData[uiIndex].Append(cBuffer.Get(), cBuffer.GetLength());
            }
        }

        // Read state strings
        uint uiNumStateString(m_hReplayData.ReadInt32());
        for (uint ui(0); ui < uiNumStateString; ++ui)
        {
            uint uiID(m_hReplayData.ReadInt32());
            tstring sStr;

            wchar_t wChar(m_hReplayData.ReadInt16());
            while (wChar)
            {
                sStr += TCHAR(wChar);
                wChar = m_hReplayData.ReadInt16();
            }

            m_mapStateStrings[uiID] = sStr;
        }

        // Write game snapshot
#if 0
        {
            CBufferDynamic cBufferFrameData;
            m_cCurrentSnapshot.WriteDiff(cBufferFrameData, m_cLastSnapshot, 0, 0, -1);

            hOutReplayData.WriteInt32(cBufferFrameData.GetLength());
            hOutReplayData.Write(cBufferFrameData.Get(), cBufferFrameData.GetLength());

            // Write reliable game data
            uint uiNumClientsReliable(0);
            for (MapClientGameData::iterator it(m_mapGameDataReliable.begin()); it != m_mapGameDataReliable.end(); ++it)
            {
                if (it->second.GetLength() > 0)
                    ++uiNumClientsReliable;
            }

            hOutReplayData.WriteInt32(uiNumClientsReliable);
            for (MapClientGameData::iterator it(m_mapGameDataReliable.begin()); it != m_mapGameDataReliable.end(); ++it)
            {
                if (it->second.GetLength() == 0)
                    continue;

                hOutReplayData.WriteInt32(it->first);
                hOutReplayData.WriteInt32(it->second.GetLength());
                hOutReplayData.Write(it->second.Get(), it->second.GetLength());

                it->second.Clear();
            }

            // Write game data
            uint uiNumClients(0);
            for (MapClientGameData::iterator it(m_mapGameData.begin()); it != m_mapGameData.end(); ++it)
            {
                if (it->second.GetLength() > 0)
                    ++uiNumClients;
            }

            hOutReplayData.WriteInt32(uiNumClients);
            for (MapClientGameData::iterator it(m_mapGameData.begin()); it != m_mapGameData.end(); ++it)
            {
                if (it->second.GetLength() == 0)
                    continue;

                hOutReplayData.WriteInt32(it->first);
                hOutReplayData.WriteInt32(it->second.GetLength());
                hOutReplayData.Write(it->second.Get(), it->second.GetLength());

                it->second.Clear();
            }

            // Write state strings
            hOutReplayData.WriteInt32(uint(m_mapStateStrings.size()));
            for (MapStateString::iterator it(m_mapStateStrings.begin()); it != m_mapStateStrings.end(); ++it)
            {
                hOutReplayData.WriteInt32(it->first);

                const wstring &sStateString(TStringToWString(it->second));
                hOutReplayData.Write(sStateString.c_str(), sStateString.length() * sizeof(wstring::value_type));
                hOutReplayData.WriteInt16(short(0));
            }
            m_mapStateStrings.clear();

            m_cLastSnapshot = m_cCurrentSnapshot;
        }
#endif

        ++iFrame;
    }

    StopPlayback();

    // Write new archive
    hOutReplayData.Close();

    hOutReplayData.Open(Filename_StripExtension(sOutFilename) + _T(".tmp"), FILE_READ | FILE_BINARY | FILE_UTF16);

    if (hOutReplayData.IsOpen())
    {
        tstring sPath(sOutFilename);
        CArchive hArchive(sPath, ARCHIVE_WRITE | ARCHIVE_MAX_COMPRESS);
        if (!hArchive.IsOpen())
            return;

        uint uiSize;
        const char *pBuffer(hOutReplayData.GetBuffer(uiSize));

        hArchive.WriteFile(_T("ReplayData"), pBuffer, uiSize);

        hOutReplayData.Close();
        hArchive.Close();

        //FileManager.Delete(Filename_StripExtension(sOutFilename) + _T(".tmp"));
    }

    Console << _T("Done") << newl;
#endif
}


/*====================
  CReplayManager::GetSpeedScale
  ====================*/
float   CReplayManager::GetSpeedScale() const
{
    if (m_iSpeed > 0)
        return 1 << m_iSpeed;
    else if (m_iSpeed < 0)
        return 1.0f / (1 << -m_iSpeed);
    else
        return 1.0f;
}


/*====================
  CReplayManager::SetPlaybackSpeed
  ====================*/
void    CReplayManager::SetPlaybackSpeed(int iSpeed)
{
    m_iSpeed = CLAMP(iSpeed, -3, 3);

    host_timeScale = GetSpeedScale();
}


/*====================
  CReplayManager::GetTypeDesc
  ====================*/
const SReplayEntityDesc*    CReplayManager::GetTypeDesc(ushort unType) const
{
    try
    {
        if (unType < Entity_Dynamic)
        {
            EntDescIDMap::const_iterator    citFind(m_mapEntDescs.find(unType));
            if (citFind == m_mapEntDescs.end())
                EX_ERROR(_T("No allocator found for entity type: ") + SHORT_HEX_TSTR(unType));

            return &citFind->second;
        }
        else
        {
            DynamicEntDescMap::const_iterator citFind(m_mapDynamicEntDescs.find(unType));
            if (citFind == m_mapDynamicEntDescs.end())
                EX_ERROR(_T("No entry found for entity type: ") + SHORT_HEX_TSTR(unType));

            BaseDynamicEntDescMap::const_iterator citFind2(m_mapBaseDynamicEntDescs.find(citFind->second));
            if (citFind2 == m_mapBaseDynamicEntDescs.end())
                EX_ERROR(_T("No entry found for entity base type: ") + INT_HEX_TSTR(citFind->second));

            return &citFind2->second;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityRegistry::GetTypeDesc() - "), NO_THROW);
        return NULL;
    }
}


/*====================
  CReplayManager::GetCompatTypeDesc
  ====================*/
const SEntityDesc*  CReplayManager::GetCompatTypeDesc(ushort unType) const
{
    try
    {
        if (unType < Entity_Dynamic)
        {
            EntDescIDMap::const_iterator    citFind(m_mapEntDescs.find(unType));
            if (citFind == m_mapEntDescs.end())
                EX_ERROR(_T("No allocator found for entity type: ") + SHORT_HEX_TSTR(unType));

            return &citFind->second.cCompatDesc;
        }
        else
        {
            DynamicEntDescMap::const_iterator citFind(m_mapDynamicEntDescs.find(unType));
            if (citFind == m_mapDynamicEntDescs.end())
                EX_ERROR(_T("No entry found for entity type: ") + SHORT_HEX_TSTR(unType));

            BaseDynamicEntDescMap::const_iterator citFind2(m_mapBaseDynamicEntDescs.find(citFind->second));
            if (citFind2 == m_mapBaseDynamicEntDescs.end())
                EX_ERROR(_T("No entry found for entity base type: ") + INT_HEX_TSTR(citFind->second));

            return &citFind2->second.cCompatDesc;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityRegistry::GetTypeDesc() - "), NO_THROW);
        return NULL;
    }
}


/*====================
  CReplayManager::SetPaused
  ====================*/
void    CReplayManager::SetPaused(bool bPaused)
{
    replay_pause = bPaused;
}


/*====================
  CReplayManager::IsPaused
  ====================*/
bool    CReplayManager::IsPaused() const
{
    return replay_pause;
}


/*--------------------
  cmdReplaySetPlaybackSpeed
  --------------------*/
CMD(ReplaySetPlaybackSpeed)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: ReplaySetPlaybackSpeed <speed>") << newl;
        return false;
    }

    CReplayManager::GetInstance()->SetPlaybackSpeed(AtoI(vArgList[0]));
    return true;
}


/*--------------------
  ReplaySetPlaybackSpeed
  --------------------*/
UI_VOID_CMD(ReplaySetPlaybackSpeed, 1)
{
    cmdReplaySetPlaybackSpeed(vArgList[0]->Evaluate());
}



/*--------------------
  cmdReplayIncPlaybackSpeed
  --------------------*/
CMD(ReplayIncPlaybackSpeed)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: ReplayIncPlaybackSpeed <inc>") << newl;
        return false;
    }

    CReplayManager::GetInstance()->SetPlaybackSpeed(CReplayManager::GetInstance()->GetPlaybackSpeed() + AtoI(vArgList[0]));
    return true;
}


/*--------------------
  ReplayIncPlaybackSpeed
  --------------------*/
UI_VOID_CMD(ReplayIncPlaybackSpeed, 1)
{
    cmdReplayIncPlaybackSpeed(vArgList[0]->Evaluate());
}


/*--------------------
  PrintGameInfo
  --------------------*/
CMD(PrintGameInfo)
{
    const PlayerMap &mapPlayers(Game.GetPlayerMap());
    for (PlayerMap_cit itPlayer(mapPlayers.begin()); itPlayer != mapPlayers.end(); ++itPlayer)
    {
        CPlayer *pPlayer(itPlayer->second);
        if (pPlayer == nullptr)
            continue;

        IHeroEntity *pHero(pPlayer->GetHero());
        Console << XtoA(pHero ? pHero->GetDisplayName() : _CTS("<none>") , FMT_NONE, 14) << SPACE;
        Console << _T("gold: ") << pPlayer->GetGold() << SPACE;
        if (pHero)
        {
            Console << _T("level: ") << pHero->GetLevel() << SPACE;
            Console << _T("health: ") << pHero->GetHealth() << SPACE;
            Console << _T("exp: ") << pHero->GetExperience() << SPACE;
        }
        Console << newl;
    }
    Console << newl;
    return true;
}
