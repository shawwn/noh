// (C)2006 S2 Games
// c_snapshot.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_snapshot.h"

#include "c_packet.h"
#include "c_transmitflags.h"
//=============================================================================
#ifdef __GNUC__
__attribute__((init_priority(1001))) SnapshotPool CSnapshot::s_pSnapshotPool(64);
#else
SnapshotPool CSnapshot::s_pSnapshotPool(64);
#endif

/*====================
  CSnapshot::~CSnapshot
  ====================*/
CSnapshot::~CSnapshot()
{
    FreeEntities();
}


/*====================
  CSnapshot::CSnapshot
  ====================*/
CSnapshot::CSnapshot() :
m_bValid(false),
m_uiFrameNumber(-1),
m_uiPrevFrameNumber(-1),
m_uiTimeStamp(0),
m_yStateSequence(0),
m_yNumEvents(0)
{
    m_vBitEntityBuffer.resize(1);
}

CSnapshot::CSnapshot(uint uiFrameNumber, uint uiTime) :
m_bValid(true),
m_uiFrameNumber(uiFrameNumber),
m_uiPrevFrameNumber(-1),
m_uiTimeStamp(uiTime),
m_yStateSequence(0),
m_yNumEvents(0)
{
    m_vBitEntityBuffer.resize(1);
}

CSnapshot::CSnapshot(CBufferBit &cBuffer) :
m_bValid(true),
m_uiFrameNumber(-1),
m_uiPrevFrameNumber(-1),
m_uiTimeStamp(0),
m_yStateSequence(0),
m_yNumEvents(0)
{
    m_vBitEntityBuffer.resize(1);
    ReadBuffer(cBuffer);
}


/*====================
  CSnapshot::CSnapshot
  ====================*/
CSnapshot::CSnapshot(const CSnapshot &B) :
m_bValid(B.m_bValid),
m_uiFrameNumber(B.m_uiFrameNumber),
m_uiPrevFrameNumber(B.m_uiPrevFrameNumber),
m_uiTimeStamp(B.m_uiTimeStamp),
m_yStateSequence(B.m_yStateSequence),
m_yNumEvents(B.m_yNumEvents),
m_bufferEvents(B.m_bufferEvents),
m_cBufferReceived(B.m_cBufferReceived)
{
    FreeEntities();

    uint uiNumStreams(uint(B.m_vStreams.size()));
    m_vStreams.resize(uiNumStreams);

    for (SnapshotVector_cit cit(B.m_vEntities.begin()), citEnd(B.m_vEntities.end()); cit != citEnd; ++cit)
    {
        SnapshotEntry newPair(cit->first, CEntitySnapshot::Allocate(cit->second));

        m_vEntities.push_back(newPair);

        for (uint uiStream(0); uiStream < uiNumStreams; ++uiStream)
        {
            for (SnapshotStreamVector_cit citSV(B.m_vStreams[uiStream].begin()), citSVEnd(B.m_vStreams[uiStream].end()); citSV != citSVEnd; ++citSV)
            {
                if (citSV->hPoolHandle == cit->second)
                {
                    m_vStreams[uiStream].push_back(SEntityStreamSnapshot(newPair.first, newPair.second, citSV->uiFlags));
                    break;
                }
            }
        }
    }

    for (BitEntStream_cit it(B.m_vBitEntityBuffer.begin()); it != B.m_vBitEntityBuffer.end(); ++it)
    {
        m_vBitEntityBuffer.push_back(*it);
    }
}


/*====================
  CSnapshot::operator=
  ====================*/
CSnapshot& CSnapshot::operator=(const CSnapshot &B)
{
    m_bValid = B.m_bValid;
    m_uiFrameNumber = B.m_uiFrameNumber;
    m_uiPrevFrameNumber = B.m_uiPrevFrameNumber;
    m_uiTimeStamp = B.m_uiTimeStamp;
    m_yStateSequence = B.m_yStateSequence;
    m_yNumEvents = B.m_yNumEvents;
    m_bufferEvents = B.m_bufferEvents;
    m_cBufferReceived = B.m_cBufferReceived;

    FreeEntities();

    uint uiNumStreams(uint(B.m_vStreams.size()));
    m_vStreams.resize(uiNumStreams);

    for (SnapshotVector_cit cit(B.m_vEntities.begin()), citEnd(B.m_vEntities.end()); cit != citEnd; ++cit)
    {
        SnapshotEntry newPair(cit->first, CEntitySnapshot::Allocate(cit->second));

        m_vEntities.push_back(newPair);

        for (uint uiStream(0); uiStream < uiNumStreams; ++uiStream)
        {
            for (SnapshotStreamVector_cit citSV(B.m_vStreams[uiStream].begin()), citSVEnd(B.m_vStreams[uiStream].end()); citSV != citSVEnd; ++citSV)
            {
                if (citSV->hPoolHandle == cit->second)
                {
                    m_vStreams[uiStream].push_back(SEntityStreamSnapshot(newPair.first, newPair.second, citSV->uiFlags));
                    break;
                }
            }
        }
    }

    m_vBitEntityBuffer.clear();
    for (BitEntStream_cit it(B.m_vBitEntityBuffer.begin()); it != B.m_vBitEntityBuffer.end(); ++it)
    {
        m_vBitEntityBuffer.push_back(*it);
    }

    return *this;
}


/*====================
  CSnapshot::FreeEntities
  ====================*/
void    CSnapshot::FreeEntities()
{
    for(SnapshotVector_cit cit(m_vEntities.begin()), citEnd(m_vEntities.end()); cit != citEnd; ++cit)
        CEntitySnapshot::DeleteByHandle(cit->second);
    m_vEntities.clear();
    m_vStreams.clear();
}


/*====================
  CSnapshot::ClearEvents
  ====================*/
void    CSnapshot::ClearEvents()
{
    m_bufferEvents.Clear();
    m_yNumEvents = 0;
}


/*====================
  CSnapshot::GetNextEntity
  ====================*/
bool    CSnapshot::GetNextEntity(CEntitySnapshot &entity, uint &uiLastIndex)
{
    entity.Clear();
    if (m_cBufferReceived.GetUnreadBits() == 0)
        return false;

    return entity.ReadHeader(m_cBufferReceived, uiLastIndex);
}


/*====================
  CSnapshot::WriteBuffer
  ====================*/
bool    CSnapshot::WriteBuffer(CBufferBit &cBuffer, byte yStateSequence, uint uiStream) const
{
    PROFILE("CSnapshot::WriteBuffer");

    try
    {
        // Header data
        cBuffer.WriteBits(m_uiFrameNumber, 32);
        
        // Encode frame delta
        cBuffer.WriteBits(0, 2);
        cBuffer.WriteBits(uint(-1), 32);
        
        cBuffer.WriteBits(m_uiTimeStamp, 32);
        cBuffer.WriteBits(yStateSequence, 8);

        // Events
        cBuffer.WriteBits(m_yNumEvents, 8);

        const byte *pEventBuffer(reinterpret_cast<const byte*>(m_bufferEvents.Get()));
        uint uiEventLength(m_bufferEvents.GetLength());
        for (uint ui(0); ui < uiEventLength; ++ui, ++pEventBuffer)
            cBuffer.WriteBits(*pEventBuffer, 8);

        // Bit entities
        const uivector &vBitEntityBuffer(m_vBitEntityBuffer[uiStream]);
        uint uiNumFields(uint(vBitEntityBuffer.size()));

        CTransmitFlags<8>   cTransmitFlags(uiNumFields);
        
        for (uint uiField(0); uiField < uiNumFields; ++uiField)
            if (vBitEntityBuffer[uiField] != uint(-1))
                cTransmitFlags.SetField(uiField);

        cTransmitFlags.WriteTransmitFlags(cBuffer);
        
        for (uint uiField(0); uiField < uiNumFields; ++uiField)
            if (vBitEntityBuffer[uiField] != uint(-1))
                cBuffer.WriteBits(vBitEntityBuffer[uiField], 32);

        // Entities
        if (uiStream < uint(m_vStreams.size()))
        {
            const SnapshotStreamVector &SnapshotsThis(m_vStreams[uiStream]);

            SnapshotStreamVector_cit citThis(SnapshotsThis.begin()), citThisEnd(SnapshotsThis.end());

            static CEntitySnapshot entSnapshot;

            uint uiLastIndex(0);

            while (citThis != citThisEnd)
            {
                // All entities are new
                const CEntitySnapshot *pThisSnapshot(CEntitySnapshot::GetByHandle(citThis->hPoolHandle));
                const CEntitySnapshot *pBaseline(pThisSnapshot->GetBaseline());

                bool bThisChanging((citThis->uiFlags & ENTITY_STREAM_FLAG_CHANGING) != 0);

                if (pBaseline != NULL)
                {
                    entSnapshot.Clear();
                    pThisSnapshot->DiffFrom(*pBaseline, entSnapshot);
                    entSnapshot.SetTypeChange(true);
                    entSnapshot.WriteBuffer(cBuffer, true, uiLastIndex, bThisChanging);
                }
                else
                    pThisSnapshot->WriteBuffer(cBuffer, true, uiLastIndex, bThisChanging);

                ++citThis;
            }
        }

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSnapshot::WriteBuffer() - "));
        return false;
    }
}


/*====================
  CSnapshot::WriteDiff

  Codes for frame number deltas:
  header+delta bits
  1 + 0
  0,1 + 4
  0,0 + 32 (raw previous frame)
  ====================*/
bool    CSnapshot::WriteDiff(CBufferBit &cBuffer, const CSnapshot &base, byte yStateSequence, uint uiStream, uint uiBaseStream) const
{
    GAME_PROFILE(_T("CSnapshot::WriteDiff"));

    try
    {
        // Header data
        cBuffer.WriteBits(m_uiFrameNumber, 32);

        uint uiFrameDelta(m_uiFrameNumber - base.GetFrameNumber());
        
        // Encode frame delta
        if (uiFrameDelta == 1)
            cBuffer.WriteBit(1); // 1
        else if (uiFrameDelta < 1 + 16)
        {
            cBuffer.WriteBits(2, 2); // 0,1
            cBuffer.WriteBits(uiFrameDelta - 1, 4);
        }
        else
        {
            cBuffer.WriteBits(0, 2); // 0,0
            cBuffer.WriteBits(base.GetFrameNumber(), 32);
        }
        
        cBuffer.WriteBits(m_uiTimeStamp, 32);
        cBuffer.WriteBits(yStateSequence, 8);

        // Events
        cBuffer.WriteBits(m_yNumEvents, 8);

        const byte *pEventBuffer(reinterpret_cast<const byte*>(m_bufferEvents.Get()));
        uint uiEventLength(m_bufferEvents.GetLength());
        for (uint ui(0); ui < uiEventLength; ++ui, ++pEventBuffer)
            cBuffer.WriteBits(*pEventBuffer, 8);

        // Bit entities
        const uivector &vBitEntityBuffer(m_vBitEntityBuffer[uiStream]);
        uint uiNumFields(uint(vBitEntityBuffer.size()));
        uint uiBaseNumFields(uint(base.m_vBitEntityBuffer[uiStream].size()));

        CTransmitFlags<8>   cTransmitFlags(uiNumFields);
        
        for (uint uiField(0); uiField < uiNumFields; ++uiField)
            if (uiField >= uiBaseNumFields || vBitEntityBuffer[uiField] != base.m_vBitEntityBuffer[uiStream][uiField])
                cTransmitFlags.SetField(uiField);

        cTransmitFlags.WriteTransmitFlags(cBuffer);
        
        for (uint uiField(0); uiField < uiNumFields; ++uiField)
            if (uiField >= uiBaseNumFields || vBitEntityBuffer[uiField] != base.m_vBitEntityBuffer[uiStream][uiField])
                cBuffer.WriteBits(vBitEntityBuffer[uiField], 32);

        if (uiStream < uint(m_vStreams.size()))
        {
            if (uiBaseStream < uint(base.m_vStreams.size()))
            {
                const SnapshotStreamVector &SnapshotsThis(m_vStreams[uiStream]);
                const SnapshotStreamVector &SnapshotsBase(base.m_vStreams[uiBaseStream]);

                SnapshotStreamVector_cit citThis(SnapshotsThis.begin()), citThisEnd(SnapshotsThis.end());
                SnapshotStreamVector_cit citBase(SnapshotsBase.begin()), citBaseEnd(SnapshotsBase.end());

                static CEntitySnapshot entSnapshot;

                uint uiLastIndex(0);

                while (citThis != citThisEnd)
                {
                    if (citBase == citBaseEnd)
                    {
                        // No entities left in base, so this must be new
                        const CEntitySnapshot *pThisSnapshot(CEntitySnapshot::GetByHandle(citThis->hPoolHandle));
                        const CEntitySnapshot *pBaseline(pThisSnapshot->GetBaseline());

                        bool bThisChanging((citThis->uiFlags & ENTITY_STREAM_FLAG_CHANGING) != 0);

                        if (pBaseline)
                        {
                            entSnapshot.Clear();
                            pThisSnapshot->DiffFrom(*pBaseline, entSnapshot);
                            entSnapshot.SetTypeChange(true);
                            entSnapshot.WriteBuffer(cBuffer, true, uiLastIndex, bThisChanging);
                        }
                        else
                            pThisSnapshot->WriteBuffer(cBuffer, true, uiLastIndex, bThisChanging);

                        ++citThis;
                    }
                    else if (citThis->uiGameIndex > citBase->uiGameIndex)
                    {
                        // This entity no longer exists, so send a delete message (0 type)
                        entSnapshot.Clear();
                        entSnapshot.SetType(0);
                        entSnapshot.SetIndex(citBase->uiGameIndex);
                        entSnapshot.WriteBuffer(cBuffer, true, uiLastIndex, true);

                        ++citBase;
                    }
                    else if (citThis->uiGameIndex == citBase->uiGameIndex)
                    {
                        const CEntitySnapshot *pThisSnapshot(CEntitySnapshot::GetByHandle(citThis->hPoolHandle));
                        const CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->hPoolHandle));

                        bool bThisChanging((citThis->uiFlags & ENTITY_STREAM_FLAG_CHANGING) != 0);
                        bool bBaseChanging((citBase->uiFlags & ENTITY_STREAM_FLAG_CHANGING) != 0);

                        // Entity exists in both snapshots, send with diffing
                        entSnapshot.Clear();
                        pThisSnapshot->DiffFrom(*pBaseSnapshot, entSnapshot);

                        if (bThisChanging != bBaseChanging)
                            entSnapshot.WriteBuffer(cBuffer, true, uiLastIndex, bThisChanging);
                        else if (entSnapshot.GetChanged() || bThisChanging)
                            entSnapshot.WriteBuffer(cBuffer, !bBaseChanging, uiLastIndex, bThisChanging);

                        ++citBase;
                        ++citThis;
                    }
                    else if (citThis->uiGameIndex < citBase->uiGameIndex)
                    {
                        // This is a new entity, diff against baseline if we have one
                        const CEntitySnapshot *pThisSnapshot(CEntitySnapshot::GetByHandle(citThis->hPoolHandle));
                        const CEntitySnapshot *pBaseline(pThisSnapshot->GetBaseline());

                        bool bThisChanging((citThis->uiFlags & ENTITY_STREAM_FLAG_CHANGING) != 0);

                        if (pBaseline)
                        {
                            entSnapshot.Clear();
                            pThisSnapshot->DiffFrom(*pBaseline, entSnapshot);
                            entSnapshot.SetTypeChange(true);
                            entSnapshot.WriteBuffer(cBuffer, true, uiLastIndex, bThisChanging);
                        }
                        else
                            pThisSnapshot->WriteBuffer(cBuffer, true, uiLastIndex, bThisChanging);

                        ++citThis;
                    }
                }

                while (citBase != citBaseEnd)
                {
                    // All remaining entities no longer exist, so delete them
                    entSnapshot.Clear();
                    entSnapshot.SetType(0);
                    entSnapshot.SetIndex(citBase->uiGameIndex);
                    entSnapshot.WriteBuffer(cBuffer, true, uiLastIndex, true);

                    ++citBase;
                }
            }
            else
            {
                const SnapshotStreamVector &SnapshotsThis(m_vStreams[uiStream]);

                SnapshotStreamVector_cit citThis(SnapshotsThis.begin()), citThisEnd(SnapshotsThis.end());

                static CEntitySnapshot entSnapshot;

                uint uiLastIndex(0);

                while (citThis != citThisEnd)
                {
                    // All entities are new
                    const CEntitySnapshot *pThisSnapshot(CEntitySnapshot::GetByHandle(citThis->hPoolHandle));
                    const CEntitySnapshot *pBaseline(pThisSnapshot->GetBaseline());

                    bool bThisChanging((citThis->uiFlags & ENTITY_STREAM_FLAG_CHANGING) != 0);

                    if (pBaseline != NULL)
                    {
                        entSnapshot.Clear();
                        pThisSnapshot->DiffFrom(*pBaseline, entSnapshot);
                        entSnapshot.SetTypeChange(true);
                        entSnapshot.WriteBuffer(cBuffer, true, uiLastIndex, bThisChanging);
                    }
                    else
                        pThisSnapshot->WriteBuffer(cBuffer, true, uiLastIndex, bThisChanging);

                    ++citThis;
                }
            }
        }

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSnapshot::WriteDiff() - "));
        return false;
    }
}


/*====================
  CSnapshot::ReadBuffer
  ====================*/
void    CSnapshot::ReadBuffer(CBufferBit &cBuffer)
{
    try
    {
        m_bufferEvents.Clear();
        m_cBufferReceived.Clear();

        FreeEntities();

        m_uiFrameNumber = cBuffer.ReadBits(32);

        // Read frame delta
        uint uiFrameDelta(cBuffer.ReadBit());
        if (uiFrameDelta != 0)
            m_uiPrevFrameNumber = m_uiFrameNumber - 1;
        else
        {
            uiFrameDelta = cBuffer.ReadBit();
            if (uiFrameDelta != 0)
                m_uiPrevFrameNumber = m_uiFrameNumber - (cBuffer.ReadBits(4) + 1);
            else
                m_uiPrevFrameNumber = cBuffer.ReadBits(32);
        }

        m_uiTimeStamp = cBuffer.ReadBits(32);
        m_yStateSequence = cBuffer.ReadBits(8);

        // Events
        m_yNumEvents = cBuffer.ReadBits(8);

        if (cBuffer.GetFaults() != 0)
            EX_ERROR(_T("Buffer is too short to be a frame snapshot"));

        // Store a copy of the buffer for the game to interpret
        m_cBufferReceived = cBuffer;
        m_cBufferReceived.Seek(cBuffer.GetReadPos());
        m_bValid = true;
    }
    catch (CException &ex)
    {
        m_bValid = false;
        ex.Process(_T("CSnapshot::ReadBuffer() - "), NO_THROW);
    }
}


/*====================
  CSnapshot::CalcSequence
  ====================*/
void    CSnapshot::CalcSequence(const CSnapshot &base)
{
    GAME_PROFILE(_T("CSnapshot::CalcSequence"));

    // Entities
    SnapshotVector &SnapshotsThis(m_vEntities);

    SnapshotVector_cit citThis(SnapshotsThis.begin()), citThisEnd(SnapshotsThis.end());

    // Calculate sequences
    while (citThis != citThisEnd)
    {
        CEntitySnapshot *pThisSnapshot(CEntitySnapshot::GetByHandle(citThis->second));
        pThisSnapshot->CalcSequence();

        ++citThis;
    }

    // Calculate stream flags
    uint uiNumStreams(uint(m_vStreams.size()));
    for (uint ui(0); ui < uiNumStreams; ++ui)
    {
        if (ui < uint(base.m_vStreams.size()))
        {
            SnapshotStreamVector &SnapshotsThis(m_vStreams[ui]);
            const SnapshotStreamVector &SnapshotsBase(base.m_vStreams[ui]);

            SnapshotStreamVector_it itThis(SnapshotsThis.begin()), itThisEnd(SnapshotsThis.end());
            SnapshotStreamVector_cit citBase(SnapshotsBase.begin()), citBaseEnd(SnapshotsBase.end());

            while (itThis != itThisEnd)
            {
                if (citBase == citBaseEnd)
                {
                    // No entities in base, so this must be new
                    itThis->uiFlags |= ENTITY_STREAM_FLAG_CHANGING;
                    ++itThis;
                }
                else if (itThis->uiGameIndex > citBase->uiGameIndex)
                {
                    // This entity no longer exists
                    ++citBase;
                }
                else if (itThis->uiGameIndex == citBase->uiGameIndex)
                {
                    CEntitySnapshot *pThisSnapshot(CEntitySnapshot::GetByHandle(itThis->hPoolHandle));
                    const CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->hPoolHandle));

                    if (pThisSnapshot->GetPublicSequence() != pBaseSnapshot->GetPublicSequence() ||
                        pThisSnapshot->GetUniqueID() != pBaseSnapshot->GetUniqueID())
                        itThis->uiFlags |= ENTITY_STREAM_FLAG_CHANGING;
                    else
                        itThis->uiFlags &= ~ENTITY_STREAM_FLAG_CHANGING;

                    ++citBase;
                    ++itThis;
                }
                else if (itThis->uiGameIndex < citBase->uiGameIndex)
                {
                    // This is a new entity
                    itThis->uiFlags |= ENTITY_STREAM_FLAG_CHANGING;
                    ++itThis;
                }
            }
        }
        else
        {
            SnapshotStreamVector &SnapshotsThis(m_vStreams[ui]);

            SnapshotStreamVector_it itThis(SnapshotsThis.begin()), itThisEnd(SnapshotsThis.end());

            while (itThis != itThisEnd)
            {
                // No entities in base, so this must be new
                itThis->uiFlags |= ENTITY_STREAM_FLAG_CHANGING;
                ++itThis;
            }
        }
    }
}

/*====================
  CSnapshot::GetStreamBitEntityBuffer
  ====================*/
uivector&   CSnapshot::GetStreamBitEntityBuffer(uint uiStream)
{
    assert(uiStream < m_vBitEntityBuffer.size());
    if (uiStream < m_vBitEntityBuffer.size())
        return m_vBitEntityBuffer[uiStream];

    Console.Err << "CSnapshot::GetStreamBitEntityBuffer: Invalid uiStream '" << uiStream << "'!" << newl;
    static uivector vEmpty;
    return vEmpty;
}

/*====================
  CSnapshot::SetStreamBitEntityBuffer
  ====================*/
void        CSnapshot::SetStreamBitEntityBuffer(uint uiStream, const uivector &vBitBuffer)
{
    assert(uiStream < m_vStreams.size());
    if (uiStream < m_vStreams.size())
    {
        m_vBitEntityBuffer[uiStream] = vBitBuffer;
        return;
    }

    Console.Err << "CSnapshot::SetStreamBitEntityBuffer: Invalid uiStream '" << uiStream << "'!" << newl;
}

/*====================
  CSnapshot::PushNewEntity
  ====================*/
PoolHandle  CSnapshot::PushNewEntity(uint uiIndex)
{
    SnapshotEntry newPair(uiIndex, CEntitySnapshot::Allocate(CEntitySnapshot()));
    m_vEntities.push_back(newPair);

    return newPair.second;
}


/*====================
  CSnapshot::GetByHandle
  ====================*/
CSnapshot*  CSnapshot::GetByHandle(PoolHandle hHandle)
{
    return s_pSnapshotPool.GetReferenceByHandle(hHandle);
}


/*====================
  CSnapshot::DeleteByHandle
  ====================*/
void    CSnapshot::DeleteByHandle(PoolHandle hHandle)
{
    if (s_pSnapshotPool.GetRefCount(hHandle) == 1)
    {
        CSnapshot *pSnapshot(CSnapshot::GetByHandle(hHandle));
        if (pSnapshot != NULL)
            *pSnapshot = CSnapshot();
    }

    s_pSnapshotPool.Free(hHandle);
}


/*====================
  CSnapshot::Allocate
  ====================*/
PoolHandle  CSnapshot::Allocate(const CSnapshot &cInitialState)
{
    return s_pSnapshotPool.New(cInitialState);
}


/*====================
  CSnapshot::AllocateSnapshot
  ====================*/
void    CSnapshot::AddRefToHandle(PoolHandle hHandle)
{
    return s_pSnapshotPool.AddRef(hHandle);
}
