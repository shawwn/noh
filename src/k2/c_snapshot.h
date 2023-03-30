// (C)2006 S2 Games
// c_snapshot.h
//
//=============================================================================
#ifndef __C_SNAPSHOT_H__
#define __C_SNAPSHOT_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_entitysnapshot.h"
#include "c_referencerecyclepool.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CSnapshot;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint ENTITY_STREAM_FLAG_CHANGING(BIT(0));

struct SEntityStreamSnapshot
{
    uint        uiGameIndex;
    PoolHandle  hPoolHandle;
    uint        uiFlags;

    SEntityStreamSnapshot() : uiGameIndex(INVALID_INDEX), hPoolHandle(INVALID_POOL_HANDLE), uiFlags(0) {}
    SEntityStreamSnapshot(uint _uiGameIndex, PoolHandle _hPoolHandle, uint _uiFlags) :
    uiGameIndex(_uiGameIndex), hPoolHandle(_hPoolHandle), uiFlags(_uiFlags) {}
};

typedef vector<CSnapshot> SnapshotBuffer;

typedef pair<uint, PoolHandle> SnapshotEntry;
typedef vector<SnapshotEntry> SnapshotVector;
typedef SnapshotVector::iterator SnapshotVector_it;
typedef SnapshotVector::const_iterator SnapshotVector_cit;

typedef vector<SEntityStreamSnapshot> SnapshotStreamVector;
typedef SnapshotStreamVector::iterator SnapshotStreamVector_it;
typedef SnapshotStreamVector::const_iterator SnapshotStreamVector_cit;

typedef vector<SnapshotStreamVector> SnapshotStream;
typedef SnapshotStream::iterator SnapshotStream_it;
typedef SnapshotStream::const_iterator SnapshotStream_cit;

typedef CReferenceRecyclePool<CSnapshot> SnapshotPool;

typedef vector<uivector> BitEntStream;
typedef vector<uivector>::iterator BitEntStream_it;
typedef vector<uivector>::const_iterator BitEntStream_cit;

#define SAFE_DELETE_SNAPSHOT(h)         { if ((h) != INVALID_POOL_HANDLE) { CSnapshot::DeleteByHandle(h); (h) = INVALID_POOL_HANDLE; } }
//=============================================================================

//=============================================================================
// CSnapshot
//=============================================================================
class CSnapshot
{
private:
    bool    m_bValid;

    uint    m_uiFrameNumber;
    uint    m_uiPrevFrameNumber;
    uint    m_uiTimeStamp;
    byte    m_yStateSequence;

    byte                m_yNumEvents;
    CBufferDynamic      m_bufferEvents;

    SnapshotVector      m_vEntities;
    SnapshotStream      m_vStreams;

    CBufferBit          m_cBufferReceived;

    BitEntStream        m_vBitEntityBuffer;

    K2_API static SnapshotPool  s_pSnapshotPool;

public:
    K2_API ~CSnapshot();
    K2_API CSnapshot();
    K2_API CSnapshot(uint uiFrameNumber, uint uiTime);
    K2_API CSnapshot(CBufferBit &cBuffer);
    K2_API CSnapshot(const CSnapshot &B);

    bool            IsValid() const                                 { return m_bValid; }
    uint            GetFrameNumber() const                          { return m_uiFrameNumber; }
    uint            GetPrevFrameNumber() const                      { return m_uiPrevFrameNumber; }
    uint            GetTimeStamp() const                            { return m_uiTimeStamp; }
    byte            GetNumEvents() const                            { return m_yNumEvents; }
    byte            GetStateSequence() const                        { return m_yStateSequence; }
    SnapshotVector& GetEntities()                                   { return m_vEntities; }
    SnapshotStream& GetStreams()                                    { return m_vStreams; }
    K2_API void     FreeEntities();
    K2_API void     ClearEvents();

    K2_API CSnapshot& operator=(const CSnapshot& B);

    void            SetValid(bool bValid)                           { m_bValid = bValid; }
    void            SetFrameNumber(uint uiFrameNumber)              { m_uiFrameNumber = uiFrameNumber; }
    void            SetPrevFrameNumber(uint uiPrevFrameNumber)      { m_uiPrevFrameNumber = uiPrevFrameNumber; }
    void            SetTimeStamp(uint uiTimeStamp)                  { m_uiTimeStamp = uiTimeStamp; }
    void            SetNumEvents(byte yNumEvents)                   { m_yNumEvents = yNumEvents; }

    K2_API bool     GetNextEntity(CEntitySnapshot &snapEntity, uint &uiLastIndex);

    void            AddEventSnapshot(const IBuffer &buffer)         { m_bufferEvents << buffer; ++m_yNumEvents; }
    CBufferDynamic& GetEventBuffer()                                { return m_bufferEvents; }

    K2_API bool     WriteBuffer(CBufferBit &cBuffer, byte yStateSequence, uint uiStream) const;
    CBufferBit&     GetReceivedBuffer()                             { return m_cBufferReceived; }

    K2_API bool     WriteDiff(CBufferBit &cBuffer, const CSnapshot &base, byte yStateSequence, uint uiStream, uint uiBaseStream) const;
    K2_API void     ReadBuffer(CBufferBit &cBuffer);
    K2_API void     CalcSequence(const CSnapshot &base);
    
    K2_API PoolHandle   PushNewEntity(uint uiIndex);

    void                SetNumStreams(uint uiNumStreams)                { m_vStreams.resize(uiNumStreams); m_vBitEntityBuffer.resize(uiNumStreams); }

    K2_API uivector&    GetStreamBitEntityBuffer(uint uiStream);
    K2_API void         SetStreamBitEntityBuffer(uint uiStream, const uivector &vBitBuffer);

    void                AddStreamEntity(uint uiStream, uint uiGameIndex, PoolHandle hPoolHandle, uint uiFlags = ENTITY_STREAM_FLAG_CHANGING)
    {
        if (uiStream < uint(m_vStreams.size()))
            m_vStreams[uiStream].push_back(SEntityStreamSnapshot(uiGameIndex, hPoolHandle, uiFlags));
    }

    void            ClearStreams()
    {
        for (uint ui(0); ui < uint(m_vStreams.size()); ++ui)
            m_vStreams[ui].clear();
    }

    static SnapshotPool*    GetSnapshotPool()                       { return &s_pSnapshotPool; }

    static K2_API CSnapshot*    GetByHandle(PoolHandle hHandle);
    static K2_API void          DeleteByHandle(PoolHandle hHandle);
    static K2_API void          AddRefToHandle(PoolHandle hHandle);
    static K2_API PoolHandle    Allocate(const CSnapshot &cInitialState);
};
//=============================================================================

#endif //__C_SNAPSHOT_H__
