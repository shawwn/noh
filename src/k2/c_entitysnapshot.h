// (C)2006 S2 Games
// c_entitysnapshot.h
//
//=============================================================================
#ifndef __C_ENTITYSNAPSHOT_H__
#define __C_ENTITYSNAPSHOT_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_buffer.h"
#include "c_networkresourcemanager.h"
#include "c_recyclepool.h"
#include "c_transmitflags.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CPacket;
class CEntitySnapshot;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//#define K2_VALIDATE_ENTITY_SNAPSHOT

const uint REPLAY_VERSION(4);

// Don't change the order of this ever! (only add at the end)
enum EDataType
{
    TYPE_CHAR = 0,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_V2F,
    TYPE_V3F,
    TYPE_GAMEINDEX,
    TYPE_RESHANDLE,
    TYPE_ANGLE8,
    TYPE_ANGLE16,
    TYPE_ROUND16,
    TYPE_FLOOR16,
    TYPE_CEIL16,
    TYPE_BYTEPERCENT,
    TYPE_ROUNDPOS3D,
    TYPE_DELTAPOS1D,
    TYPE_DELTAPOS2D,
    TYPE_DELTAPOS3D
};

struct SDataField
{
    tstring         sName;
    EDataType       eDataType;
    uint            uiBits;
    int             iParam;

    SDataField() {}
    SDataField(const tstring _sName, EDataType _eDataType, uint _uiBits, int _iParam) : sName(_sName), eDataType(_eDataType), uiBits(_uiBits), iParam(_iParam) {}
};

const uint ENTITY_SNAPSHOT_CHANGED      (BIT(0));
const uint ENTITY_SNAPSHOT_TYPE_CHANGE  (BIT(1));
const uint ENTITY_SNAPSHOT_CHANGING     (BIT(2));

typedef vector<SDataField>      TypeVector;
typedef CRecyclePool<CEntitySnapshot> EntitySnapshotPool;
//=============================================================================

//=============================================================================
// CEntitySnapshot
//=============================================================================
class CEntitySnapshot
{
private:
    // *** Members must be included in the copy constructor AND operator= for snapshots to function ***
    uint                    m_uiFlags;

    CTransmitFlags<8>       m_cTransmitFlags;

    CBufferDynamic          m_cBuffer;
    
    mutable uint            m_uiReadIndex;
    
    TypeVector::const_iterator  m_itWriteField;

    uint                    m_uiIndex;
    ushort                  m_unType;

    uint                    m_uiApplyToFrame;
    uint                    m_uiVersion;

    uint                    m_uiPublicSequence;
    uint                    m_uiUniqueID;

    byte                    m_yTransmitFlagBytes;

    const TypeVector        *m_pFieldTypes;
    uint                    m_uiTypeSize;

    const CEntitySnapshot   *m_pBaseline;

    void        ReadTransmitFlags(const CBufferBit &cBuffer)        { m_cTransmitFlags.ReadTransmitFlags(cBuffer); }
    void        WriteTransmitFlags(CBufferBit &cBuffer) const       { m_cTransmitFlags.WriteTransmitFlags(cBuffer); }

    K2_API static EntitySnapshotPool s_pEntitySnapshotPool;

public:
    K2_API ~CEntitySnapshot()   {}
    K2_API CEntitySnapshot();
    K2_API CEntitySnapshot(const CEntitySnapshot &B);

    bool                    GetChanged() const              { return (m_uiFlags & ENTITY_SNAPSHOT_CHANGED) != 0; }
    void                    SetIndex(uint uiIndex)          { m_uiIndex = uiIndex; }
    void                    SetType(ushort unType)          { m_unType = unType; }
    K2_API void             SetFieldTypes(const TypeVector *pFieldTypes, uint uiSize);
    K2_API static uint      CalcSnapshotSize(const TypeVector *pFieldTypes);
    void                    SetBaseline(const CEntitySnapshot *pBaseline)   { m_pBaseline = pBaseline; }
    void                    SetTypeChange(bool bTypeChange) { if (bTypeChange) m_uiFlags |= ENTITY_SNAPSHOT_TYPE_CHANGE; else m_uiFlags &= ~ENTITY_SNAPSHOT_TYPE_CHANGE; }
    void                    SetChanged(bool bChanged)       { if (bChanged) m_uiFlags |= ENTITY_SNAPSHOT_CHANGED; else m_uiFlags &= ~ENTITY_SNAPSHOT_CHANGED; }
    uint                    GetIndex() const                { return m_uiIndex; }
    ushort                  GetType() const                 { return m_unType; }
    const CEntitySnapshot*  GetBaseline() const             { return m_pBaseline; }
    bool                    GetTypeChange() const           { return (m_uiFlags & ENTITY_SNAPSHOT_TYPE_CHANGE) != 0; }
    const TypeVector*       GetFieldTypes() const           { return m_pFieldTypes; }

    void                    SetChanging(bool bChanging)     { if (bChanging) m_uiFlags |= ENTITY_SNAPSHOT_CHANGING; else m_uiFlags &= ~ENTITY_SNAPSHOT_CHANGING; }
    bool                    GetChanging() const             { return (m_uiFlags & ENTITY_SNAPSHOT_CHANGING) != 0; }
    
    inline void             WriteDiffField(const char *pBuffer, uint uiSize);
    inline void             WriteDiffField(const CBufferDynamic &cBuffer, uint uiSize);
    template<class T> void  WriteDiffField(const T &_x);
    
    template<class T> void  WriteField(const T &_x);
    template<class T> void  WriteInteger(T _x);
    inline void             WriteGameIndex(uint ui);
    inline void             WriteResHandle(ResHandle hRes);
    inline void             WriteAngle8(float f);
    inline void             WriteAngle16(float f);
    inline void             WriteRound16(float f);
    inline void             WriteFloor16(float f);
    inline void             WriteCeil16(float f);
    inline void             WriteBytePercent(float f);
    inline void             WriteRoundPos3D(const CVec3f &v3);
    inline void             WriteDeltaPos1D(float f);
    inline void             WriteDeltaPos2D(const CVec2f &v2);
    inline void             WriteDeltaPos3D(const CVec3f &v3);

    bool                    IsFieldSet(uint uiIndex) const  { return m_cTransmitFlags.IsFieldSet(uiIndex); }
    void                    SetField(uint uiIndex)          { m_cTransmitFlags.SetField(uiIndex); }
    void                    SetAllFields()                  { m_cTransmitFlags.SetAllFields(); }

    K2_API void             Clear();

    void                    SetApplyToFrame(uint uiFrameNumber) { m_uiApplyToFrame = uiFrameNumber; }
    uint                    GetApplyToFrame() const         { return m_uiApplyToFrame; }

    void                    SetVersion(uint uiVersion)      { m_uiVersion = uiVersion; }
    uint                    GetVersion() const              { return m_uiVersion; }

    void                    SetPublicSequence(uint uiPublicSequence)    { m_uiPublicSequence = uiPublicSequence; }
    uint                    GetPublicSequence() const       { return m_uiPublicSequence; }
    
    void                    SetUniqueID(uint uiUniqueID)    { m_uiUniqueID = uiUniqueID; }
    uint                    GetUniqueID() const             { return m_uiUniqueID; }

    void                    RewindRead() const              { m_uiReadIndex = 0; m_cBuffer.RewindBuffer(); }        

    inline const char*      GetNextField(uint uiSize) const;

    K2_API bool             ReadHeader(CBufferBit &cBuffer, uint &uiLastIndex);
    K2_API bool             ReadBody(CBufferBit &cBuffer, const TypeVector &vTypes, uint uiTypeSize);
    K2_API bool             ReadBody(CBufferBit &cBuffer, const TypeVector &vTypes, uint uiTypeSize, const CEntitySnapshot *pBaseline, bool bSetAllFields = true);
    template<class T> bool  ReadField(T &_val) const;
    template<class T> bool  ReadInteger(T &_val) const;
    inline bool             ReadGameIndex(uint &uiVal) const;
    inline bool             ReadResHandle(ResHandle &hVal) const;
    inline bool             ReadAngle8(float &fVal) const;
    inline bool             ReadAngle16(float &fVal) const;
    inline bool             ReadFloat16(float &fVal) const;
    inline bool             ReadBytePercent(float &fVal) const;
    inline bool             ReadRoundPos3D(CVec3f &v3Val) const;
    inline bool             ReadDeltaPos1D(float &fVal) const;
    inline bool             ReadDeltaPos2D(CVec2f &v2Val) const;
    inline bool             ReadDeltaPos3D(CVec3f &v3Val) const;
    
    void                    DiffFrom(const CEntitySnapshot &snapshot, CEntitySnapshot &result) const;
    K2_API void             CalcSequence();

    K2_API void             WriteBuffer(CBufferBit &cBuffer, bool bSendIndex, uint &uiLastIndex, bool bChanging) const;

    K2_API void             ApplyDiff(CEntitySnapshot &cDiff);
    K2_API CEntitySnapshot& operator=(const CEntitySnapshot &B);

    byte                    GetTransmitFlagBytes() const    { return m_yTransmitFlagBytes; }

    K2_API const char*      GetField(uint uiFieldIndex) const;

    static EntitySnapshotPool*  GetEntitySnapshotPool()             { return &s_pEntitySnapshotPool; }

    static inline CEntitySnapshot*  GetByHandle(PoolHandle hHandle);
    static inline void              DeleteByHandle(PoolHandle hHandle);
    static inline PoolHandle        Allocate(const CEntitySnapshot &cInitialState);
    static inline PoolHandle        Allocate(PoolHandle hHandle);
};
//=============================================================================

//=============================================================================
// Inline functions
//=============================================================================

/*====================
  CEntitySnapshot::WriteDiffField
  ====================*/
inline
void    CEntitySnapshot::WriteDiffField(const CBufferDynamic &cBuffer, uint uiSize)
{
    m_cBuffer.Append(cBuffer.Get(cBuffer.GetReadPos()), uiSize); cBuffer.Advance(uiSize);
}


/*====================
  CEntitySnapshot::WriteDiffField
  ====================*/
template <class T>
inline
void    CEntitySnapshot::WriteDiffField(const T &_x)
{
    m_cBuffer << _x;
}


/*====================
  CEntitySnapshot::WriteDiffField
  ====================*/
inline
void    CEntitySnapshot::WriteDiffField(const char *pBuffer, uint uiSize)
{
    if (pBuffer)
        m_cBuffer.Append(pBuffer, uiSize);
}


/*====================
  CEntitySnapshot::WriteField
  ====================*/
template <class T>
inline
void    CEntitySnapshot::WriteField(const T &_x)
{
    m_cBuffer << _x;
    ++m_itWriteField;
}


/*====================
  CEntitySnapshot::WriteInteger
  ====================*/
template <class T>
inline
void    CEntitySnapshot::WriteInteger(T _x)
{
    try
    {
        if (m_itWriteField >= m_pFieldTypes->end())
            EX_ERROR(_T("All fields have been written"));

        m_cBuffer << T(_x - m_itWriteField->iParam);

        ++m_itWriteField;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::WriteInteger() - "), NO_THROW);
    }
}


/*====================
  CEntitySnapshot::WriteGameIndex
  ====================*/
inline
void    CEntitySnapshot::WriteGameIndex(uint ui)
{
    m_cBuffer << ushort(ui);
    ++m_itWriteField;
}


/*====================
  CEntitySnapshot::WriteResHandle
  ====================*/
inline
void    CEntitySnapshot::WriteResHandle(ResHandle hRes)
{
    m_cBuffer << ushort(NetworkResourceManager.GetNetIndex(hRes));
    ++m_itWriteField;
}


/*====================
  CEntitySnapshot::WriteAngle8
  ====================*/
inline
void    CEntitySnapshot::WriteAngle8(float f)
{
    while (f < 0.0f)
        f += 360.0f;
    
    m_cBuffer << byte(INT_ROUND(f / 360.0f * UCHAR_MAX));
    ++m_itWriteField;
}


/*====================
  CEntitySnapshot::WriteAngle16
  ====================*/
inline
void    CEntitySnapshot::WriteAngle16(float f)
{
    while (f < 0.0f)
        f += 360.0f;
    
    m_cBuffer << ushort(INT_ROUND(f / 360.0f * USHRT_MAX));
    ++m_itWriteField;
}


/*====================
  CEntitySnapshot::WriteRound16
  ====================*/
inline
void    CEntitySnapshot::WriteRound16(float f)
{
    m_cBuffer << ushort(CLAMP(INT_ROUND(f), 0, USHRT_MAX));
    ++m_itWriteField;
}


/*====================
  CEntitySnapshot::WriteFloor16
  ====================*/
inline
void    CEntitySnapshot::WriteFloor16(float f)
{
    m_cBuffer << ushort(CLAMP(INT_FLOOR(f), 0, USHRT_MAX));
    ++m_itWriteField;
}


/*====================
  CEntitySnapshot::WriteCeil16
  ====================*/
inline
void    CEntitySnapshot::WriteCeil16(float f)
{
    m_cBuffer << ushort(CLAMP(INT_CEIL(f), 0, USHRT_MAX));
    ++m_itWriteField;
}


/*====================
  CEntitySnapshot::WriteBytePercent
  ====================*/
inline
void    CEntitySnapshot::WriteBytePercent(float f)
{
    m_cBuffer << byte(INT_ROUND(CLAMP(f, 0.0f, 1.0f) * 255));
    ++m_itWriteField;
}


/*====================
  CEntitySnapshot::WriteRoundPos3D
  ====================*/
inline
void    CEntitySnapshot::WriteRoundPos3D(const CVec3f &v3)
{
    m_cBuffer.WriteShort(ushort(INT_ROUND(CLAMP(v3.x, 0.0f, float(USHRT_MAX)))));
    m_cBuffer.WriteShort(ushort(INT_ROUND(CLAMP(v3.y, 0.0f, float(USHRT_MAX)))));
    m_cBuffer.WriteShort(INT_ROUND(CLAMP(v3.z, float(SHRT_MIN), float(SHRT_MAX))));
    ++m_itWriteField;
}


/*====================
  CEntitySnapshot::WriteDeltaPos1D
  ====================*/
inline
void    CEntitySnapshot::WriteDeltaPos1D(float f)
{
    short n(INT_ROUND(CLAMP(f, float(SHRT_MIN), float(SHRT_MAX))));
    m_cBuffer.Append(&n, sizeof(short));
    ++m_itWriteField;
}


/*====================
  CEntitySnapshot::WriteDeltaPos2D
  ====================*/
inline
void    CEntitySnapshot::WriteDeltaPos2D(const CVec2f &v2)
{
    short n1(INT_ROUND(CLAMP(v2.x, float(SHRT_MIN), float(SHRT_MAX))));
    short n2(INT_ROUND(CLAMP(v2.y, float(SHRT_MIN), float(SHRT_MAX))));
    m_cBuffer.Append(&n1, sizeof(short));
    m_cBuffer.Append(&n2, sizeof(short));
    ++m_itWriteField;
}


/*====================
  CEntitySnapshot::WriteDeltaPos3D
  ====================*/
inline
void    CEntitySnapshot::WriteDeltaPos3D(const CVec3f &v3)
{
    short n1(INT_ROUND(CLAMP(v3.x, float(SHRT_MIN), float(SHRT_MAX))));
    short n2(INT_ROUND(CLAMP(v3.y, float(SHRT_MIN), float(SHRT_MAX))));
    short n3(INT_ROUND(CLAMP(v3.z, float(SHRT_MIN), float(SHRT_MAX))));
    m_cBuffer.Append(&n1, sizeof(short));
    m_cBuffer.Append(&n2, sizeof(short));
    m_cBuffer.Append(&n3, sizeof(short));
    ++m_itWriteField;
}


/*====================
  CEntitySnapshot::GetNextField
  ====================*/
inline
const char* CEntitySnapshot::GetNextField(uint uiSize) const
{
    try
    {
        if (m_uiReadIndex >= m_pFieldTypes->size())
            EX_ERROR(_T("All fields have been read"));

#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        uint uiSize2(0);
        switch ((*m_pFieldTypes)[m_uiReadIndex].eDataType)
        {
        case TYPE_BYTEPERCENT:
        case TYPE_ANGLE8:
        case TYPE_CHAR:         uiSize2 = 1;    break;
        case TYPE_GAMEINDEX:
        case TYPE_RESHANDLE:
        case TYPE_ANGLE16:
        case TYPE_ROUND16:
        case TYPE_FLOOR16:
        case TYPE_CEIL16:
        case TYPE_DELTAPOS1D:
        case TYPE_SHORT:        uiSize2 = 2;    break;
        case TYPE_INT:
        case TYPE_FLOAT:
        case TYPE_DELTAPOS2D:   uiSize2 = 4;    break;
        case TYPE_V2F:          uiSize2 = 8;    break;
        case TYPE_V3F:          uiSize2 = 12;   break;
        case TYPE_ROUNDPOS3D:
        case TYPE_DELTAPOS3D:   uiSize2 = 6; break;
        
        }

        if (uiSize != uiSize2)
            EX_ERROR(_T("Size mismatch"));
#endif
        const char *pBuffer(NULL);
        if (IsFieldSet(m_uiReadIndex))
        {
            pBuffer = m_cBuffer.Get(m_cBuffer.GetReadPos());
            m_cBuffer.Advance(uiSize);
        }

        ++m_uiReadIndex;
        return pBuffer;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::GetNextField() - "));
        return NULL;
    }
}


/*====================
  CEntitySnapshot::ReadField
  ====================*/
template <class T>
inline
bool    CEntitySnapshot::ReadField(T &_val) const
{
    try
    {
        if (m_uiReadIndex >= m_pFieldTypes->size())
            EX_ERROR(_T("All fields have been read"));
        
#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        int iSize(0);
        switch ((*m_pFieldTypes)[m_uiReadIndex].eDataType)
        {
        case TYPE_BYTEPERCENT:
        case TYPE_ANGLE8:
        case TYPE_CHAR:         iSize = 1;  break;
        case TYPE_GAMEINDEX:
        case TYPE_RESHANDLE:
        case TYPE_ANGLE16:
        case TYPE_ROUND16:
        case TYPE_FLOOR16:
        case TYPE_CEIL16:
        case TYPE_DELTAPOS1D:
        case TYPE_SHORT:        iSize = 2;  break;
        case TYPE_INT:
        case TYPE_FLOAT:
        case TYPE_DELTAPOS2D:   iSize = 4;  break;
        case TYPE_V2F:          iSize = 8;  break;
        case TYPE_V3F:          iSize = 12; break;
        case TYPE_ROUNDPOS3D:
        case TYPE_DELTAPOS3D:   iSize = 6; break;
        }
        
        if (sizeof(T) != iSize)
            EX_ERROR(_T("Size mismatch"));
#endif

        if (IsFieldSet(m_uiReadIndex++))
        {
            m_cBuffer >> _val;
            return true;
        }
        else
            return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::ReadField() - "));
        return false;
    }
}


/*====================
  CEntitySnapshot::ReadInteger
  ====================*/
template <class T>
inline
bool    CEntitySnapshot::ReadInteger(T &_val) const
{
    try
    {
        if (m_uiReadIndex >= m_pFieldTypes->size())
            EX_ERROR(_T("All fields have been read"));
        
#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        int iSize(0);
        switch ((*m_pFieldTypes)[m_uiReadIndex].eDataType)
        {
        case TYPE_CHAR:     iSize = 1;  break;
        case TYPE_SHORT:    iSize = 2;  break;
        case TYPE_INT:      iSize = 4;  break;
        }
        
        if (sizeof(T) != iSize)
            EX_ERROR(_T("Size mismatch"));
#endif

        if (IsFieldSet(m_uiReadIndex))
        {
            m_cBuffer >> _val;
            _val += (*m_pFieldTypes)[m_uiReadIndex].iParam;
            ++m_uiReadIndex;
            return true;
        }
        else
        {
            ++m_uiReadIndex;
            return false;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::ReadField() - "));
        return false;
    }
}


/*====================
  CEntitySnapshot::ReadGameIndex
  ====================*/
inline
bool    CEntitySnapshot::ReadGameIndex(uint &uiVal) const
{
    try
    {
        if (m_uiReadIndex >= m_pFieldTypes->size())
            EX_ERROR(_T("All fields have been read"));

#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        if ((*m_pFieldTypes)[m_uiReadIndex].eDataType != TYPE_GAMEINDEX)
            EX_ERROR(_T("Type mismatch"));
#endif

        if (IsFieldSet(m_uiReadIndex++))
        {
            ushort unVal;
            m_cBuffer >> unVal;
            
            if (unVal == ushort(-1))
                uiVal = INVALID_INDEX;
            else
                uiVal = unVal;

            return true;
        }
        else
            return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::ReadGameIndex() - "));
        return false;
    }
}


/*====================
  CEntitySnapshot::ReadResHandle
  ====================*/
inline
bool    CEntitySnapshot::ReadResHandle(ResHandle &hVal) const
{
    try
    {
        if (m_uiReadIndex >= m_pFieldTypes->size())
            EX_ERROR(_T("All fields have been read"));

#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        if ((*m_pFieldTypes)[m_uiReadIndex].eDataType != TYPE_RESHANDLE)
            EX_ERROR(_T("Type mismatch"));
#endif

        if (IsFieldSet(m_uiReadIndex++))
        {
            ushort unVal;
            m_cBuffer >> unVal;

            hVal = NetworkResourceManager.GetLocalHandle(unVal == ushort(-1) ? INVALID_INDEX : unVal);

            return true;
        }
        else
            return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::ReadResHandle() - "));
        return false;
    }
}


/*====================
  CEntitySnapshot::ReadAngle8
  ====================*/
inline
bool    CEntitySnapshot::ReadAngle8(float &fVal) const
{
    try
    {
        if (m_uiReadIndex >= m_pFieldTypes->size())
            EX_ERROR(_T("All fields have been read"));

#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        if ((*m_pFieldTypes)[m_uiReadIndex].eDataType != TYPE_ANGLE8)
            EX_ERROR(_T("Type mismatch"));
#endif

        if (IsFieldSet(m_uiReadIndex++))
        {
            byte yVal;
            m_cBuffer >> yVal;

            fVal = float(yVal) / UCHAR_MAX * 360.0f;

            if (fVal > 180.0f)
                fVal -= 360.0f;

            return true;
        }
        else
            return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::ReadAngle8() - "));
        return false;
    }
}


/*====================
  CEntitySnapshot::ReadAngle16
  ====================*/
inline
bool    CEntitySnapshot::ReadAngle16(float &fVal) const
{
    try
    {
        if (m_uiReadIndex >= m_pFieldTypes->size())
            EX_ERROR(_T("All fields have been read"));

#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        if ((*m_pFieldTypes)[m_uiReadIndex].eDataType != TYPE_ANGLE16)
            EX_ERROR(_T("Type mismatch"));
#endif

        if (IsFieldSet(m_uiReadIndex++))
        {
            ushort unVal;
            m_cBuffer >> unVal;

            fVal = float(unVal) / USHRT_MAX * 360.0f;

            if (fVal > 180.0f)
                fVal -= 360.0f;
            
            return true;
        }
        else
            return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::ReadAngle16() - "));
        return false;
    }
}


/*====================
  CEntitySnapshot::ReadFloat16
  ====================*/
inline
bool    CEntitySnapshot::ReadFloat16(float &fVal) const
{
    try
    {
        if (m_uiReadIndex >= m_pFieldTypes->size())
            EX_ERROR(_T("All fields have been read"));

#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        if ((*m_pFieldTypes)[m_uiReadIndex].eDataType != TYPE_ROUND16 &&
            (*m_pFieldTypes)[m_uiReadIndex].eDataType != TYPE_FLOOR16 &&
            (*m_pFieldTypes)[m_uiReadIndex].eDataType != TYPE_CEIL16)
            EX_ERROR(_T("Type mismatch"));
#endif

        if (IsFieldSet(m_uiReadIndex++))
        {
            ushort unVal;
            m_cBuffer >> unVal;

            fVal = unVal;

            return true;
        }
        else
            return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::ReadFloat16() - "));
        return false;
    }
}


/*====================
  CEntitySnapshot::ReadBytePercent
  ====================*/
inline
bool    CEntitySnapshot::ReadBytePercent(float &fVal) const
{
    try
    {
        if (m_uiReadIndex >= m_pFieldTypes->size())
            EX_ERROR(_T("All fields have been read"));

#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        if ((*m_pFieldTypes)[m_uiReadIndex].eDataType != TYPE_BYTEPERCENT)
            EX_ERROR(_T("Type mismatch"));
#endif

        if (IsFieldSet(m_uiReadIndex++))
        {
            byte yVal;
            m_cBuffer >> yVal;

            fVal = yVal / 255.0f;

            return true;
        }
        else
            return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::ReadBytePercent() - "));
        return false;
    }
}


/*====================
  CEntitySnapshot::ReadRoundPos3D
  ====================*/
inline
bool    CEntitySnapshot::ReadRoundPos3D(CVec3f &v3Val) const
{
    try
    {
        if (m_uiReadIndex >= m_pFieldTypes->size())
            EX_ERROR(_T("All fields have been read"));

#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        if ((*m_pFieldTypes)[m_uiReadIndex].eDataType != TYPE_ROUNDPOS3D)
            EX_ERROR(_T("Type mismatch"));
#endif

        if (IsFieldSet(m_uiReadIndex++))
        {
            v3Val.x = ushort(m_cBuffer.ReadShort());
            v3Val.y = ushort(m_cBuffer.ReadShort());
            v3Val.z = m_cBuffer.ReadShort();

            return true;
        }
        else
            return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::ReadRoundPos3D() - "));
        return false;
    }
}


/*====================
  CEntitySnapshot::ReadDeltaPos1D
  ====================*/
inline
bool    CEntitySnapshot::ReadDeltaPos1D(float &fVal) const
{
    try
    {
        if (m_uiReadIndex >= m_pFieldTypes->size())
            EX_ERROR(_T("All fields have been read"));

#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        if ((*m_pFieldTypes)[m_uiReadIndex].eDataType != TYPE_DELTAPOS1D)
            EX_ERROR(_T("Type mismatch"));
#endif

        if (IsFieldSet(m_uiReadIndex++))
        {
            short n;
            m_cBuffer.Read(&n, sizeof(short));
            fVal = n;
            return true;
        }
        else
            return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::ReadDeltaPos1D() - "));
        return false;
    }
}


/*====================
  CEntitySnapshot::ReadDeltaPos2D
  ====================*/
inline
bool    CEntitySnapshot::ReadDeltaPos2D(CVec2f &v2Val) const
{
    try
    {
        if (m_uiReadIndex >= m_pFieldTypes->size())
            EX_ERROR(_T("All fields have been read"));

#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        if ((*m_pFieldTypes)[m_uiReadIndex].eDataType != TYPE_DELTAPOS2D)
            EX_ERROR(_T("Type mismatch"));
#endif

        if (IsFieldSet(m_uiReadIndex++))
        {
            short n1, n2;
            m_cBuffer.Read(&n1, sizeof(short));
            m_cBuffer.Read(&n2, sizeof(short));
            v2Val.x = n1;
            v2Val.y = n2;
            return true;
        }
        else
            return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::ReadDeltaPos1D() - "));
        return false;
    }
}


/*====================
  CEntitySnapshot::ReadDeltaPos3D
  ====================*/
inline
bool    CEntitySnapshot::ReadDeltaPos3D(CVec3f &v3Val) const
{
    try
    {
        if (m_uiReadIndex >= m_pFieldTypes->size())
            EX_ERROR(_T("All fields have been read"));

#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        if ((*m_pFieldTypes)[m_uiReadIndex].eDataType != TYPE_DELTAPOS3D)
            EX_ERROR(_T("Type mismatch"));
#endif

        if (IsFieldSet(m_uiReadIndex++))
        {
            short n1, n2, n3;
            m_cBuffer.Read(&n1, sizeof(short));
            m_cBuffer.Read(&n2, sizeof(short));
            m_cBuffer.Read(&n3, sizeof(short));
            v3Val.x = n1;
            v3Val.y = n2;
            v3Val.z = n3;
            return true;
        }
        else
            return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::ReadDeltaPos1D() - "));
        return false;
    }
}


/*====================
  CEntitySnapshot::GetByHandle
  ====================*/
inline
CEntitySnapshot*    CEntitySnapshot::GetByHandle(PoolHandle hHandle)
{
    return s_pEntitySnapshotPool.GetReferenceByHandle(hHandle);
}


/*====================
  CEntitySnapshot::DeleteByHandle
  ====================*/
inline
void    CEntitySnapshot::DeleteByHandle(PoolHandle hHandle)
{
    s_pEntitySnapshotPool.Free(hHandle);
}


/*====================
  CEntitySnapshot::Allocate
  ====================*/
inline
PoolHandle  CEntitySnapshot::Allocate(const CEntitySnapshot &cInitialState)
{
    return s_pEntitySnapshotPool.New(cInitialState);
}


/*====================
  CEntitySnapshot::Allocate
  ====================*/
inline
PoolHandle  CEntitySnapshot::Allocate(PoolHandle hHandle)
{
    return s_pEntitySnapshotPool.NewFromHandle(hHandle);
}

//=============================================================================

#endif //__C_ENTITYSNAPSHOT_H__
