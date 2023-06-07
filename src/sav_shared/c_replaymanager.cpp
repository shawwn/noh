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

#include "../k2/k2_protocol.h"
#include "../k2/c_statestring.h"
#include "../k2/c_uicmd.h"

#undef pReplayManager
#undef ReplayManager
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CReplayManager  *pReplayManager(CReplayManager::GetInstance());

SINGLETON_INIT(CReplayManager)

CVAR_BOOLF      (replay_isPlaying,          false,      CVAR_READONLY);
CVAR_BOOL       (replay_pause,              false);
//=============================================================================

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
m_iSpeed(0)
{
}


/*====================
  CReplayManager::StartRecording
  ====================*/
void    CReplayManager::StartRecording(const tstring &sFilename)
{
    if (IsPlaying() && Host.HasServer())
        return;

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
    m_hReplayData.WriteByte('0');

    // Version
    m_hReplayData.WriteInt32(REPLAY_VERSION);

    // Write world name as a wide string
    const wstring &sWorldName(TStringToWString(Host.GetServerWorldName()));
    m_hReplayData.Write(sWorldName.c_str(), sWorldName.length() * sizeof(wstring::value_type));
    m_hReplayData.WriteInt16(short(0));

    // Write state strings
    for (uint uiID(1); uiID != 4; ++uiID)
    {
        tstring sBuffer;
        Game.GetStateString(uiID).AppendToBuffer(sBuffer);

        const wstring &sStateString(TStringToWString(sBuffer));
        m_hReplayData.Write(sStateString.c_str(), sStateString.length() * sizeof(wstring::value_type));
        m_hReplayData.WriteInt16(short(0));
    }

    m_mapStateStrings.clear();
    m_cLastSnapshot = CSnapshot();
    m_cCurrentSnapshot = CSnapshot();
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

    m_hReplayData.Open(Filename_StripExtension(m_sFilename) + _T(".tmp"), FILE_READ | FILE_BINARY | FILE_UTF16);

    if (m_hReplayData.IsOpen())
    {
        tstring sPath(m_sFilename);
        CArchive hArchive(sPath, ARCHIVE_WRITE | ARCHIVE_MAX_COMPRESS);
        if (!hArchive.IsOpen())
            return;

        uint uiSize;
        const char *pBuffer(m_hReplayData.GetBuffer(uiSize));

        hArchive.WriteFile(_T("ReplayData"), pBuffer, uiSize);

        m_hReplayData.Close();
        hArchive.Close();

        FileManager.Delete(Filename_StripExtension(m_sFilename) + _T(".tmp"));
    }
}


/*====================
  CReplayManager::StartPlayback
  ====================*/
bool    CReplayManager::StartPlayback(const tstring &sFilename)
{
    CArchive *pArchive(NULL);

    try
    {
        if (IsRecording())
            EX_WARN(_T("No playback allowed while recording"));
        
        CArchive *pArchive(K2_NEW(global,   CArchive)(_TS("~/") + sFilename));  

        if (!pArchive->IsOpen())
        {
            K2_DELETE(pArchive);
            
            pArchive = K2_NEW(global,   CArchive)(sFilename);
            
            if (!pArchive->IsOpen())
                EX_WARN(_T("Failed to open archive"));
        }

        m_hReplayData.Open(_T("ReplayData"), FILE_READ | FILE_BINARY | FILE_UTF16, *pArchive);

        if (!m_hReplayData.IsOpen())
            EX_WARN(_T("Failed to open replay data"));

        if (m_hReplayData.ReadByte() != 'S' ||
            m_hReplayData.ReadByte() != '2' ||
            m_hReplayData.ReadByte() != 'R' ||
            m_hReplayData.ReadByte() != '0')
            EX_WARN(_T("Invalid replay header"));

        if (m_hReplayData.ReadInt32() != REPLAY_VERSION)
            EX_WARN(_T("Version mismatch"));

        m_bPlaying = true;
        replay_isPlaying = true;

        wchar_t wChar(m_hReplayData.ReadInt16());
        while (wChar)
        {
            m_sWorldName += TCHAR(wChar);
            wChar = m_hReplayData.ReadInt16();
        }

        for (uint uiID(1); uiID != 4; ++uiID)
        {
            tstring sStr;

            wchar_t wChar(m_hReplayData.ReadInt16());
            while (wChar)
            {
                sStr += TCHAR(wChar);
                wChar = m_hReplayData.ReadInt16();
            }

            m_mapStateStrings[uiID] = sStr;
        }

        m_cCurrentSnapshot.SetFrameNumber(-1);
        m_cCurrentSnapshot.SetPrevFrameNumber(-1);
        m_cCurrentSnapshot.SetTimeStamp(0);
        m_cCurrentSnapshot.FreeEntities();

        m_zStartPos = m_hReplayData.Tell();
        m_uiBeginTime = INVALID_TIME;
        m_uiEndTime = INVALID_TIME;

        GenerateKeyFrames();

        return true;
    }
    catch (CException &ex)
    {
        if (m_hReplayData.IsOpen())
            m_hReplayData.Close();

        SAFE_DELETE(pArchive);

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

    m_hReplayData.Close();
    m_bPlaying = false;
    replay_isPlaying = false;
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
void    CReplayManager::ReadSnapshot(int iFrame, IBuffer &cBuffer)
{
    CSnapshot snapshot(cBuffer);

    m_cCurrentSnapshot.SetValid(true);
    m_cCurrentSnapshot.SetFrameNumber(iFrame);
    m_cCurrentSnapshot.SetPrevFrameNumber(-1);
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

    SnapshotVector &vBaseEntities(m_cCurrentSnapshot.GetEntities());
    SnapshotVector_it citBase(vBaseEntities.begin());

    static CEntitySnapshot entSnapshot;

    // Translate entities
    for (;;)
    {
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

            const vector<SDataField>* pTypeVector(EntityRegistry.GetTypeVector(unType));
            if (pTypeVector == NULL)
                EX_ERROR(_T("Unknown entity type, bad snapshot"));

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

            const vector<SDataField> *pTypeVector(EntityRegistry.GetTypeVector(unType));
            if (pTypeVector == NULL)
                EX_ERROR(_T("Unknown entity type, bad snapshot"));

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
}


/*====================
  CReplayManager::StartFrame
  ====================*/
void    CReplayManager::StartFrame(int iFrame)
{
    if (IsPlaying() && replay_pause)
    {
        m_bFrameOpen = true;
    }
    else if (IsPlaying())
    {
        if (m_hReplayData.IsEOF() || uint(m_iCurrentFrame) == m_uiNumFrames)
            return;

        uint uiLength(m_hReplayData.ReadInt32());

        if (uiLength == 0)
            return;

        CBufferStatic cBuffer(uiLength);
        uint uiNumClients;

        for (uint uiRead(0); uiRead < uiLength; ++uiRead)
            cBuffer << m_hReplayData.ReadByte();

        ReadSnapshot(iFrame, cBuffer);

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

        ++m_iCurrentFrame;

        m_bFrameOpen = true;
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
void    CReplayManager::EndFrame()
{
    if (!m_bFrameOpen)
        return;

    // Write game snapshot
    if (IsRecording())
    {
        CBufferDynamic bufFrameData;
        m_cCurrentSnapshot.WriteDiff(bufFrameData, m_cLastSnapshot, 0, 0, -1);

        m_hReplayData.WriteInt32(bufFrameData.GetLength());
        m_hReplayData.Write(bufFrameData.Get(), bufFrameData.GetLength());

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

        m_hReplayData.WriteInt32(uiNumClients + 1);
        for (MapClientGameData::iterator it(m_mapGameData.begin()); it != m_mapGameData.end(); ++it)
        {
            if (it->second.GetLength() == 0)
                continue;

            m_hReplayData.WriteInt32(it->first);
            m_hReplayData.WriteInt32(it->second.GetLength());
            m_hReplayData.Write(it->second.Get(), it->second.GetLength());

            it->second.Clear();
        }

        // Server profiling data
        m_hReplayData.WriteInt32(-1);
        m_hReplayData.WriteInt32(5);
        m_hReplayData.WriteByte(GAME_CMD_SERVER_STATS);
        m_hReplayData.WriteInt32(Host.GetFrameLength());

        // Write state strings
        m_hReplayData.WriteInt32(uint(m_mapStateStrings.size()));
        for (MapStateString::iterator it(m_mapStateStrings.begin()); it != m_mapStateStrings.end(); ++it)
        {
            m_hReplayData.WriteInt32(it->first);

            const wstring &sStateString(TStringToWString(it->second));
            m_hReplayData.Write(sStateString.c_str(), sStateString.length() * sizeof(wstring::value_type));
            m_hReplayData.WriteInt16(short(0));
        }
        m_mapStateStrings.clear();

        m_cLastSnapshot = m_cCurrentSnapshot;
    }
    else if (IsPlaying())
    {
        // Clear old data
        for (MapClientGameData::iterator it(m_mapGameDataReliable.begin()); it != m_mapGameDataReliable.end(); ++it)
            it->second.Clear();

        for (MapClientGameData::iterator it(m_mapGameData.begin()); it != m_mapGameData.end(); ++it)
            it->second.Clear();

        m_mapStateStrings.clear();
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

    tstring sBuffer;
    ss.AppendToBuffer(sBuffer);

    m_mapStateStrings[uiID] = sBuffer;
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
  CReplayManager::WriteSnapshot
  ====================*/
void    CReplayManager::WriteSnapshot(const CSnapshot &snapshot)
{
    if (!m_bFrameOpen)
        return;

    m_cCurrentSnapshot = snapshot;
}


/*====================
  CReplayManager::GetWorldName
  ====================*/
tstring     CReplayManager::GetWorldName()
{
    return m_sWorldName;
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
    if (itFind != m_mapGameDataReliable.end())
        buffer = itFind->second;
}


/*====================
  CReplayManager::SetPlaybackFrame
  ====================*/
void    CReplayManager::SetPlaybackFrame(int iFrame)
{
    iFrame = CLAMP<int>(iFrame, 0, m_uiNumFrames - 1);

    if (iFrame == 0)
    {
        m_hReplayData.Seek(int(m_zStartPos));

        m_cCurrentSnapshot.SetFrameNumber(-1);
        m_cCurrentSnapshot.SetPrevFrameNumber(-1);
        m_cCurrentSnapshot.SetTimeStamp(0);
        m_cCurrentSnapshot.FreeEntities();

        m_mapGameData.clear();
        m_mapGameDataReliable.clear();
        m_mapStateStrings.clear();

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

        m_cCurrentSnapshot.SetFrameNumber(-1);
        m_cCurrentSnapshot.SetPrevFrameNumber(-1);
        m_cCurrentSnapshot.SetTimeStamp(0);
        m_cCurrentSnapshot.FreeEntities();

        it->second.bufSnapshot.Rewind();

        ReadSnapshot(it->second.uiFrame, it->second.bufSnapshot);

        m_mapGameData.clear();
        m_mapGameDataReliable.clear();
        m_mapStateStrings.clear();

        m_iCurrentFrame = it->second.uiFrame;
    }
}


/*====================
  CReplayManager::Profile
  ====================*/
void    CReplayManager::Profile(const tstring &sFilename, int iClient)
{
    if (!StartPlayback(sFilename))
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

    map<ushort, SProfileEntitySnapshot> mapProfileEntity;
    
    int iReliableGameDataBytes(0);
    int iGameDataBytes(0);
    int iStateStringBytes(0);

    map<uint, uint>     mapReliableGameDataBytes;
    map<uint, uint>     mapGameDataBytes;

    CBufferStatic cBuffer(0x4000);
    CSnapshot snapshot;
    
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

        const IBuffer &buffer(snapshot.GetReceivedBuffer());

        m_cCurrentSnapshot.SetValid(true);
        m_cCurrentSnapshot.SetFrameNumber(snapshot.GetFrameNumber());
        m_cCurrentSnapshot.SetPrevFrameNumber(-1);
        m_cCurrentSnapshot.SetTimeStamp(snapshot.GetTimeStamp());
        
        // Clear events
        m_cCurrentSnapshot.SetNumEvents(0);
        m_cCurrentSnapshot.GetEventBuffer().Clear();

        byte yNumEvents(snapshot.GetNumEvents());

        // Translate events
        for (int i(0); i < yNumEvents; ++i)
        {
            uint uiStartPos(buffer.GetReadPos());

            CGameEvent::AdvanceBuffer(buffer);
            
            ++iNumEvents;

            iEventBytes += buffer.GetReadPos() - uiStartPos;
        }

        SnapshotVector &vBaseEntities(m_cCurrentSnapshot.GetEntities());
        SnapshotVector_it citBase(vBaseEntities.begin());

        static CEntitySnapshot entSnapshot;

        // Translate entities
        for (;;)
        {
            uint uiEntitySnapshotStartPos(buffer.GetReadPos());
            const vector<SDataField>* pTypeVector(NULL);

            // Grab a "shell" entity snapshot from the the frame snapshot.
            // The data will be filled in once we know the type.
            entSnapshot.Clear();
            if (!snapshot.GetNextEntity(entSnapshot, -1))
                break;

            uint uiEntityFieldStartPos(buffer.GetReadPos());

            while (citBase != vBaseEntities.end() && citBase->first < entSnapshot.GetIndex())
                ++citBase;

            if (citBase == vBaseEntities.end() || citBase->first > entSnapshot.GetIndex())
            {
                //
                // New entity, read from baseline
                //

                ushort unType(entSnapshot.GetType());

                SProfileEntitySnapshot &cEntityProfile(mapProfileEntity[unType]);

                ++iNumEntitySnapshots;

                ++cEntityProfile.uiCount;

                uint uiHeaderBytes(buffer.GetReadPos() - uiEntitySnapshotStartPos);

                iEntityHeaderBytes += uiHeaderBytes;
                cEntityProfile.uiHeaderBytes += uiHeaderBytes;

                // If the type is NULL, the entity is dead and should be removed
                if (unType == 0)
                {
                    uint uiSnapshotBytes(buffer.GetReadPos() - uiEntitySnapshotStartPos);

                    iEntitySnapshotBytes += uiSnapshotBytes;
                    cEntityProfile.uiSnapshotBytes += uiSnapshotBytes;
                    continue;
                }

                pTypeVector = EntityRegistry.GetTypeVector(unType);
                if (pTypeVector == NULL)
                    EX_ERROR(_T("Unknown entity type, bad snapshot"));

                if (pTypeVector)
                    cEntityProfile.vFieldChanges.resize(pTypeVector->size(), 0);

                uint uiEntityFieldStartPos(buffer.GetReadPos());

                entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), *pTypeVector, EntityRegistry.GetBaseline(unType));
                citBase = vBaseEntities.insert(citBase, SnapshotEntry(entSnapshot.GetIndex(), CEntitySnapshot::Allocate(entSnapshot)));
                ++citBase;

                uint uiFieldBytes(buffer.GetReadPos() - uiEntityFieldStartPos - entSnapshot.GetTransmitFlagBytes());

                iEntityTransmitFlagBytes += entSnapshot.GetTransmitFlagBytes();
                cEntityProfile.uiTransmitFlagBytes += entSnapshot.GetTransmitFlagBytes();

                iEntityFieldBytes += uiFieldBytes;
                cEntityProfile.uiFieldBytes += uiFieldBytes;

                uint uiSnapshotBytes(buffer.GetReadPos() - uiEntitySnapshotStartPos);

                iEntitySnapshotBytes += uiSnapshotBytes;
                cEntityProfile.uiSnapshotBytes += uiSnapshotBytes;

                for (uint ui(0); ui < pTypeVector->size(); ++ui)
                {
                    if (entSnapshot.IsFieldSet(ui) && ((*pTypeVector)[ui].eAccess != FIELD_PRIVATE || iClient == -1 || entSnapshot.GetPrivateClient() == iClient))
                        ++cEntityProfile.vFieldChanges[ui];
                }
            }
            else if (citBase->first == entSnapshot.GetIndex())
            {
                //
                // Update existing entity
                //

                CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->second));
                ushort unType(entSnapshot.GetTypeChange() ? entSnapshot.GetType() : pBaseSnapshot->GetType());

                SProfileEntitySnapshot &cEntityProfile(mapProfileEntity[unType]);

                ++iNumEntitySnapshots;

                ++cEntityProfile.uiCount;

                uint uiHeaderBytes(buffer.GetReadPos() - uiEntitySnapshotStartPos);

                iEntityHeaderBytes += uiHeaderBytes;
                cEntityProfile.uiHeaderBytes += uiHeaderBytes;

                // If the type is NULL, the entity is dead and should be removed
                if (unType == 0)
                {
                    uint uiSnapshotBytes(buffer.GetReadPos() - uiEntitySnapshotStartPos);

                    iEntitySnapshotBytes += uiSnapshotBytes;
                    cEntityProfile.uiSnapshotBytes += uiSnapshotBytes;

                    CEntitySnapshot::DeleteByHandle(citBase->second);
                    citBase = vBaseEntities.erase(citBase);
                    continue;
                }

                pTypeVector = EntityRegistry.GetTypeVector(unType);
                if (pTypeVector == NULL)
                    EX_ERROR(_T("Unknown entity type, bad snapshot"));

                if (pTypeVector)
                    cEntityProfile.vFieldChanges.resize(pTypeVector->size(), 0);

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

                uint uiFieldBytes(buffer.GetReadPos() - uiEntityFieldStartPos - entSnapshot.GetTransmitFlagBytes());

                iEntityTransmitFlagBytes += entSnapshot.GetTransmitFlagBytes();
                cEntityProfile.uiTransmitFlagBytes += entSnapshot.GetTransmitFlagBytes();

                iEntityFieldBytes += uiFieldBytes;
                cEntityProfile.uiFieldBytes += uiFieldBytes;

                uint uiSnapshotBytes(buffer.GetReadPos() - uiEntitySnapshotStartPos);

                iEntitySnapshotBytes += uiSnapshotBytes;
                cEntityProfile.uiSnapshotBytes += uiSnapshotBytes;

                for (uint ui(0); ui < pTypeVector->size(); ++ui)
                {
                    if (entSnapshot.IsFieldSet(ui) && ((*pTypeVector)[ui].eAccess != FIELD_PRIVATE || iClient == -1 || entSnapshot.GetPrivateClient() == iClient))
                        ++cEntityProfile.vFieldChanges[ui];
                }
            }

            
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
                CBufferStatic cBuffer(uiLength);

                for (uint uiRead(0); uiRead < uiLength; ++uiRead)
                    cBuffer << m_hReplayData.ReadByte();

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
            tstring sStr;

            wchar_t wChar(m_hReplayData.ReadInt16());
            while (wChar)
            {
                sStr += TCHAR(wChar);
                wChar = m_hReplayData.ReadInt16();
            }
        }

        ++iFrame;
    }

    tstring sOutputFilename(iClient != -1 ? _T("~/") + Filename_StripExtension(sFilename) + _T("-") + XtoA(iClient) + _T(".txt") : _T("~/") + Filename_StripExtension(sFilename) + _T(".txt"));

    CFileHandle hOutput(sOutputFilename, FILE_WRITE | FILE_TEXT);

    hOutput << _T("Frames: ") << iFrame << newl;
    hOutput << _T("Total Bytes: ") << uint(m_hReplayData.GetLength()) << newl;
    
    hOutput << _T("Snapshot Header Bytes: ") << iSnapshotHeaderBytes << newl;
    hOutput << _T("Events: ") << iNumEvents << newl;
    hOutput << _T("Event Bytes: ") << iEventBytes << newl;
    
    hOutput << _T("Reliable Game Data Bytes: ") << iReliableGameDataBytes << newl;
    hOutput << _T("Game Data Bytes: ") << iGameDataBytes << newl;
    hOutput << _T("State String Bytes: ") << iStateStringBytes << newl << newl;

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

    const EntAllocatorNameMap &mapAllocatorNames(EntityRegistry.GetAllocatorNames());
    for (EntAllocatorNameMap::const_iterator cit(mapAllocatorNames.begin()); cit != mapAllocatorNames.end(); ++cit)
    {
        const IEntityAllocator *pAllocator(cit->second);
        if (!pAllocator)
            continue;

        ushort unType(pAllocator->GetID());

        if (mapProfileEntity.find(unType) == mapProfileEntity.end())
            continue;

        SProfileEntitySnapshot &cEntityProfile(mapProfileEntity[unType]);

        hOutput << XtoA(pAllocator->GetName().substr(0, 30), FMT_ALIGNLEFT, 31)
                << XtoA(cEntityProfile.uiCount, 0, 9) << _T(" ")
                << XtoA(cEntityProfile.uiSnapshotBytes, 0, 9) << _T(" ")
                << XtoA(cEntityProfile.uiHeaderBytes, 0, 9) << _T(" ")
                << XtoA(cEntityProfile.uiTransmitFlagBytes, 0, 9) << _T(" ")
                << XtoA(cEntityProfile.uiFieldBytes, 0, 9) << newl;
    }

    map<tstring, SProfileField> mapFields;

    for (EntAllocatorNameMap::const_iterator cit(mapAllocatorNames.begin()); cit != mapAllocatorNames.end(); ++cit)
    {
        const IEntityAllocator *pAllocator(cit->second);
        if (!pAllocator)
            continue;

        ushort unType(pAllocator->GetID());

        if (mapProfileEntity.find(unType) == mapProfileEntity.end())
            continue;

        SProfileEntitySnapshot &cEntityProfile(mapProfileEntity[unType]);

        const TypeVector *pTypeVector(pAllocator->GetTypeVector());

        hOutput << newl << newl;

        hOutput << XtoA(pAllocator->GetName().substr(0, 30), FMT_ALIGNLEFT, 31) 
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
            case TYPE_CHAR:         iSize = 1;  break;
            case TYPE_SHORT:        iSize = 2;  break;
            case TYPE_INT:          iSize = 4;  break;
            case TYPE_FLOAT:        iSize = 4;  break;
            case TYPE_V2F:          iSize = 8;  break;
            case TYPE_V3F:          iSize = 12; break;
            case TYPE_RESHANDLE:    iSize = 2;  break;
            case TYPE_GAMEINDEX:    iSize = 2;  break;
            case TYPE_ANGLE16:      iSize = 2;  break;
            case TYPE_ROUND16:      iSize = 2;  break;
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
        hOutput << XtoA(_T(it->first), FMT_ALIGNLEFT, 31) 
                << XtoA(it->second.uiCount, 0, 9)
                << XtoA(it->second.uiBytes, 0, 9) << newl;
    }

    StopPlayback();

    Console << _T("Done") << newl;
}


/*====================
  CReplayManager::Parse
  ====================*/
void    CReplayManager::Parse(const tstring &sFilename)
{
    if (!StartPlayback(sFilename))
        return;

    int iFrame(0);

    //CFileHandle hKeyFrames(_T("~/") + Filename_StripExtension(sFilename) + _T(".tmp"), FILE_WRITE | FILE_BINARY | FILE_UTF16);

    CBufferStatic cBuffer(0x4000);
    CSnapshot snapshot;

    while (!m_hReplayData.IsEOF())
    {
        uint uiLength(m_hReplayData.ReadInt32());
        cBuffer.Clear();

        cBuffer.Reserve(uiLength);
        cBuffer.SetLength(uiLength);
        m_hReplayData.Read((char *)cBuffer.Get(), uiLength);

        snapshot.ReadBuffer(cBuffer);

        m_cCurrentSnapshot.SetValid(true);
        m_cCurrentSnapshot.SetFrameNumber(snapshot.GetFrameNumber());
        m_cCurrentSnapshot.SetPrevFrameNumber(-1);
        m_cCurrentSnapshot.SetTimeStamp(snapshot.GetTimeStamp());
        
        // Clear events
        m_cCurrentSnapshot.SetNumEvents(0);
        m_cCurrentSnapshot.GetEventBuffer().Clear();

        byte yNumEvents(snapshot.GetNumEvents());

        // Translate events
        for (int i(0); i < yNumEvents; ++i)
            CGameEvent::AdvanceBuffer(snapshot.GetReceivedBuffer());

        SnapshotVector &vBaseEntities(m_cCurrentSnapshot.GetEntities());
        SnapshotVector_it citBase(vBaseEntities.begin());

        static CEntitySnapshot entSnapshot;

        // Translate entities
        for (;;)
        {
            const vector<SDataField> *pTypeVector(NULL);

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
                    EX_ERROR(_T("Unknown entity type, bad snapshot"));

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
                    EX_ERROR(_T("Unknown entity type, bad snapshot"));

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

        uint uiNumClients;

        // Read reliable game data
        uiNumClients = m_hReplayData.ReadInt32();
        for (uint ui(0); ui < uiNumClients; ++ui)
        {
            m_hReplayData.ReadInt32(); // uiIndex
            uint uiLength(m_hReplayData.ReadInt32());

            if (uiLength > 0)
            {
                CBufferStatic cBuffer(uiLength);

                for (uint uiRead(0); uiRead < uiLength; ++uiRead)
                    cBuffer << m_hReplayData.ReadByte();
            }
        }

        // Read game data
        uiNumClients = m_hReplayData.ReadInt32();
        for (uint ui(0); ui < uiNumClients; ++ui)
        {
            m_hReplayData.ReadInt32(); // uiIndex
            uint uiLength(m_hReplayData.ReadInt32());

            if (uiLength > 0)
            {
                CBufferStatic cBuffer(uiLength);

                for (uint uiRead(0); uiRead < uiLength; ++uiRead)
                    cBuffer << m_hReplayData.ReadByte();
            }

        }

        // Read state strings
        uint uiNumStateString(m_hReplayData.ReadInt32());
        for (uint ui(0); ui < uiNumStateString; ++ui)
        {
            m_hReplayData.ReadInt32(); // uiID
            tstring sStr;

            wchar_t wChar(m_hReplayData.ReadInt16());
            while (wChar)
            {
                sStr += TCHAR(wChar);
                wChar = m_hReplayData.ReadInt16();
            }
        }

#if 0
        if (iFrame % 100 == 0)
        {
            CBufferDynamic bufFrameData;
            m_cCurrentSnapshot.WriteBuffer(bufFrameData, 0, 0, -1);

            hKeyFrames.WriteInt32(bufFrameData.GetLength());
            hKeyFrames.Write(bufFrameData.Get(), bufFrameData.GetLength());
        }
#endif

        ++iFrame;
    }

    StopPlayback();

    //hKeyFrames.Close();

    Console << _T("Done") << newl;
}


/*====================
  CReplayManager::GenerateKeyFrames
  ====================*/
void    CReplayManager::GenerateKeyFrames()
{
    int iFrame(0);

    CBufferStatic cBuffer(0x4000);
    CSnapshot snapshot;

    while (!m_hReplayData.IsEOF())
    {
        uint uiLength(m_hReplayData.ReadInt32());
        if (!uiLength)
            continue;

        cBuffer.Clear();

        cBuffer.Reserve(uiLength);
        cBuffer.SetLength(uiLength);
        m_hReplayData.Read((char *)cBuffer.Get(), uiLength);

        snapshot.ReadBuffer(cBuffer);

        if (m_uiBeginTime == INVALID_TIME)
            m_uiBeginTime = snapshot.GetTimeStamp();

        m_cCurrentSnapshot.SetValid(true);
        m_cCurrentSnapshot.SetFrameNumber(snapshot.GetFrameNumber());
        m_cCurrentSnapshot.SetPrevFrameNumber(-1);
        m_cCurrentSnapshot.SetTimeStamp(snapshot.GetTimeStamp());
        
        // Clear events
        m_cCurrentSnapshot.SetNumEvents(0);
        m_cCurrentSnapshot.GetEventBuffer().Clear();

        byte yNumEvents(snapshot.GetNumEvents());

        // Translate events
        for (int i(0); i < yNumEvents; ++i)
            CGameEvent::AdvanceBuffer(snapshot.GetReceivedBuffer());

        SnapshotVector &vBaseEntities(m_cCurrentSnapshot.GetEntities());
        SnapshotVector_it citBase(vBaseEntities.begin());

        static CEntitySnapshot entSnapshot;

        // Translate entities
        for (;;)
        {
            const vector<SDataField>* pTypeVector(NULL);

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
                    EX_ERROR(_T("Unknown entity type, bad snapshot"));

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
                    EX_ERROR(_T("Unknown entity type, bad snapshot"));

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

        uint uiNumClients;

        // Read reliable game data
        uiNumClients = m_hReplayData.ReadInt32();
        for (uint ui(0); ui < uiNumClients; ++ui)
        {
            m_hReplayData.ReadInt32(); // uiIndex
            uint uiLength(m_hReplayData.ReadInt32());

            if (uiLength > 0)
            {
                CBufferStatic cBuffer(uiLength);

                for (uint uiRead(0); uiRead < uiLength; ++uiRead)
                    cBuffer << m_hReplayData.ReadByte();
            }
        }

        // Read game data
        uiNumClients = m_hReplayData.ReadInt32();
        for (uint ui(0); ui < uiNumClients; ++ui)
        {
            m_hReplayData.ReadInt32(); // uiIndex
            uint uiLength(m_hReplayData.ReadInt32());

            if (uiLength > 0)
            {
                CBufferStatic cBuffer(uiLength);

                for (uint uiRead(0); uiRead < uiLength; ++uiRead)
                    cBuffer << m_hReplayData.ReadByte();
            }

        }

        // Read state strings
        uint uiNumStateString(m_hReplayData.ReadInt32());
        for (uint ui(0); ui < uiNumStateString; ++ui)
        {
            m_hReplayData.ReadInt32(); // uiID
            tstring sStr;

            wchar_t wChar(m_hReplayData.ReadInt16());
            while (wChar)
            {
                sStr += TCHAR(wChar);
                wChar = m_hReplayData.ReadInt16();
            }
        }

        if (iFrame % 100 == 0)
        {
            SReplayKeyFrame &cKeyFrame = m_mapKeyFrames[iFrame] = SReplayKeyFrame();

            cKeyFrame.uiFrame = iFrame;
            cKeyFrame.zPos = m_hReplayData.Tell();

            m_cCurrentSnapshot.WriteBuffer(cKeyFrame.bufSnapshot, 0, 0, -1);
        }

        ++iFrame;
    }

    m_uiNumFrames = iFrame;
    m_uiEndTime = m_cCurrentSnapshot.GetTimeStamp();

    m_hReplayData.Seek(int(m_zStartPos));

    m_cCurrentSnapshot.SetFrameNumber(-1);
    m_cCurrentSnapshot.SetPrevFrameNumber(-1);
    m_cCurrentSnapshot.SetTimeStamp(0);
    m_cCurrentSnapshot.FreeEntities();
}


/*====================
  CReplayManager::SetPlaybackSpeed
  ====================*/
void    CReplayManager::SetPlaybackSpeed(int iSpeed)
{
    m_iSpeed = CLAMP(iSpeed, -3, 3);

    if (m_iSpeed > 0)
        host_timeScale = 1 << m_iSpeed;
    else if (m_iSpeed < 0)
        host_timeScale = 1.0f / (1 << -m_iSpeed);
    else
        host_timeScale = 1.0f;
}


/*--------------------
  cmdReplayRestart
  --------------------*/
CMD(ReplayRestart)
{
    CReplayManager::GetInstance()->SetPlaybackFrame(0);
    return true;
}


/*--------------------
  cmdReplaySetFrame
  --------------------*/
CMD(ReplaySetFrame)
{
    if (vArgList.size() < 1)
    {
        Console << "syntax: ReplaySetFrame <frame>" << newl;
        return false;
    }

    CReplayManager::GetInstance()->SetPlaybackFrame(AtoI(vArgList[0]));
    return true;
}


/*--------------------
  ReplaySetFrame
  --------------------*/
UI_VOID_CMD(ReplaySetFrame, 1)
{
    cmdReplaySetFrame(vArgList[0]->Evaluate());
}



/*--------------------
  cmdReplayIncFrame
  --------------------*/
CMD(ReplayIncFrame)
{
    if (vArgList.size() < 1)
    {
        Console << "syntax: ReplayIncFrame <numframes>" << newl;
        return false;
    }

    CReplayManager::GetInstance()->SetPlaybackFrame(CReplayManager::GetInstance()->GetFrame() + AtoI(vArgList[0]));
    return true;
}


/*--------------------
  ReplayIncFrame
  --------------------*/
UI_VOID_CMD(ReplayIncFrame, 1)
{
    cmdReplayIncFrame(vArgList[0]->Evaluate());
}


/*--------------------
  cmdReplaySetPlaybackSpeed
  --------------------*/
CMD(ReplaySetPlaybackSpeed)
{
    if (vArgList.size() < 1)
    {
        Console << "syntax: ReplaySetPlaybackSpeed <speed>" << newl;
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
        Console << "syntax: ReplayIncPlaybackSpeed <inc>" << newl;
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
  cmdReplayProfile
  --------------------*/
CMD(ReplayProfile)
{
    if (vArgList.size() < 1)
    {
        Console << "syntax: ReplayProfile <filename>" << newl;
        return false;
    }

    CReplayManager::GetInstance()->Profile(vArgList[0], vArgList.size() > 1 ? AtoI(vArgList[1]) : -1);
    return true;
}


/*--------------------
  cmdReplayParse
  --------------------*/
CMD(ReplayParse)
{
    if (vArgList.size() < 1)
    {
        Console << "syntax: ReplayParse <filename>" << newl;
        return false;
    }

    uint uiStartTime(K2System.Milliseconds());

    CReplayManager::GetInstance()->Parse(vArgList[0]);

    Console << _T("Replay parse took ") << MsToSec(K2System.Milliseconds() - uiStartTime) << _T(" secs") << newl;
    return true;
}



