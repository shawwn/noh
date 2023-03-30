// (C)2006 S2 Games
// c_entitysnapshot.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include <zlib.h>

#include "c_entitysnapshot.h"
#include "c_packet.h"
//=============================================================================
#ifdef __GNUC__
__attribute__((init_priority(1000))) EntitySnapshotPool CEntitySnapshot::s_pEntitySnapshotPool(4096);
#else
EntitySnapshotPool CEntitySnapshot::s_pEntitySnapshotPool(4096);
#endif

/*====================
  CEntitySnapshot::CEntitySnapshot
  ====================*/
CEntitySnapshot::CEntitySnapshot() :
m_uiFlags(ENTITY_SNAPSHOT_CHANGED | ENTITY_SNAPSHOT_TYPE_CHANGE | ENTITY_SNAPSHOT_CHANGING),
m_cBuffer(0),
m_uiReadIndex(0),
m_uiIndex(INVALID_INDEX),
m_unType(0),
m_uiApplyToFrame(0),
m_uiVersion(0),
m_uiPublicSequence(0),
m_uiUniqueID(INVALID_INDEX),
m_yTransmitFlagBytes(0),
m_pFieldTypes(NULL),
m_pBaseline(NULL)
{
    m_cTransmitFlags.Clear();
}

CEntitySnapshot::CEntitySnapshot(const CEntitySnapshot &B) :
m_uiFlags(B.m_uiFlags),
m_cTransmitFlags(B.m_cTransmitFlags),
m_cBuffer(B.m_cBuffer),
m_uiApplyToFrame(B.m_uiApplyToFrame),
m_uiVersion(B.m_uiVersion),
m_uiReadIndex(B.m_uiReadIndex),
m_itWriteField(B.m_itWriteField),
m_uiIndex(B.m_uiIndex),
m_unType(B.m_unType),
m_pFieldTypes(B.m_pFieldTypes),
m_uiTypeSize(B.m_uiTypeSize),
m_pBaseline(B.m_pBaseline),
m_uiUniqueID(B.m_uiUniqueID),
m_uiPublicSequence(B.m_uiPublicSequence)
{
}


/*====================
  CEntitySnapshot::operator=
  ====================*/
CEntitySnapshot&    CEntitySnapshot::operator=(const CEntitySnapshot &B)
{
    m_uiFlags = B.m_uiFlags;
    m_cTransmitFlags = B.m_cTransmitFlags;
    m_cBuffer.Write(B.m_cBuffer.GetBuffer(), CEIL_MULTIPLE<4>(B.m_cBuffer.GetBufferLength()));

    m_uiApplyToFrame = B.m_uiApplyToFrame;
    m_uiVersion = B.m_uiVersion;
    m_uiReadIndex = B.m_uiReadIndex;
    m_itWriteField = B.m_itWriteField;
    m_uiIndex = B.m_uiIndex;
    m_unType = B.m_unType;
    m_pFieldTypes = B.m_pFieldTypes;
    m_uiTypeSize = B.m_uiTypeSize;
    m_pBaseline = B.m_pBaseline;

    m_uiUniqueID = B.m_uiUniqueID;
    m_uiPublicSequence = B.m_uiPublicSequence;

    return *this;
}


/*====================
  CEntitySnapshot::Clear
  ====================*/
void    CEntitySnapshot::Clear()
{
    m_uiFlags = ENTITY_SNAPSHOT_CHANGED | ENTITY_SNAPSHOT_TYPE_CHANGE | ENTITY_SNAPSHOT_CHANGING;
    m_cTransmitFlags.SetNumFields(0);
    m_cBuffer.Clear();
    m_uiIndex = INVALID_INDEX;
    m_uiApplyToFrame = 0;
    m_uiVersion = 0;
    m_uiPublicSequence = -1;
    m_uiUniqueID = -1;
    m_uiReadIndex = 0;
    m_unType = 0;
    m_pFieldTypes = NULL;
    m_uiTypeSize = 0;
    m_pBaseline = NULL;
}


/*====================
  CEntitySnapshot::ReadHeader

  Codes for index deltas:
  header+delta bits
  1 + 0
  0,1 + 4
  0,0,1 + 8
  0,0,0 + 15
  ====================*/
bool    CEntitySnapshot::ReadHeader(CBufferBit &cBuffer, uint &uiLastIndex)
{
    try
    {
        Clear();

        if (cBuffer.ReadBit())
        {
            uint uiDelta(cBuffer.ReadBit());
            if (uiDelta == 0)
            {
                uiDelta = cBuffer.ReadBit();
                if (uiDelta != 0)
                    uiDelta = cBuffer.ReadBits(4) + 1;
                else
                {
                    uiDelta = cBuffer.ReadBit();
                    if (uiDelta != 0)
                        uiDelta = cBuffer.ReadBits(8) + 1 + 16;
                    else
                        uiDelta = cBuffer.ReadBits(15) + 1 + 16 + 256;
                }
            }

            m_uiIndex = uiLastIndex + uiDelta;
            uiLastIndex = m_uiIndex;

            bool bTypeChange(cBuffer.ReadBit() != 0);

            SetTypeChange(bTypeChange);

            if (bTypeChange)
                m_unType = cBuffer.ReadBits(16);

            SetChanging(cBuffer.ReadBit() != 0);
        }
        else
        {
            SetChanging(true);
            SetTypeChange(false);
        }

        if (cBuffer.GetFaults() != 0)
            EX_ERROR(_T("Truncated entity in buffer"));

        return true;
    }
    catch (CException &ex)
    {
        SetChanged(false);
        ex.Process(_T("CEntitySnapshot::CEntitySnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  CEntitySnapshot::ReadBody
  ====================*/
bool    CEntitySnapshot::ReadBody(CBufferBit &cBuffer, const TypeVector &vTypes, uint uiTypeSize)
{
    PROFILE("CEntitySnapshot::ReadBody");

    try
    {
        SetFieldTypes(&vTypes, uiTypeSize);

        ReadTransmitFlags(cBuffer);

        // Read the entity data
        for (TypeVector::const_iterator it(m_pFieldTypes->begin()), itEnd(m_pFieldTypes->end()); it != itEnd; ++it)
        {
            if (cBuffer.GetFaults() != 0)
                EX_ERROR(_T("Truncated entity"));

            if (!IsFieldSet(uint(it - m_pFieldTypes->begin())))
                continue;

            uint uiSize(0);

            switch (it->eDataType)
            {
            case TYPE_CHAR:
            case TYPE_BYTEPERCENT:
            case TYPE_ANGLE8:
                uiSize = 1;
                break;

            case TYPE_GAMEINDEX:
            case TYPE_RESHANDLE:
            case TYPE_ANGLE16:
            case TYPE_ROUND16:
            case TYPE_FLOOR16:
            case TYPE_CEIL16:
            case TYPE_SHORT:
                uiSize = 2;
                break;

            case TYPE_INT:
            case TYPE_FLOAT:
                uiSize = 4;
                break;

            case TYPE_V2F:
                uiSize = 8;
                break;

            case TYPE_V3F:
                uiSize = 12;
                break;

            case TYPE_ROUNDPOS3D:
                uiSize = 6;
                break;

            case TYPE_DELTAPOS1D:
                {
                    // code:
                    // 1 + 5
                    // 0,1 + 6
                    // 0,0 + 15 

                    uint uiCode(cBuffer.ReadBit());
                    if (uiCode != 0)
                        uiCode = cBuffer.ReadBits(5);
                    else
                    {
                        uiCode = cBuffer.ReadBit();
                        if (uiCode != 0)
                            uiCode = cBuffer.ReadBits(6) + 32;
                        else
                            uiCode = cBuffer.ReadBits(15);
                    }

                    // Map 0,-1,1,-2,2 to -2,-1,0,1,2
                    short p(uiCode & 1 ? short(-int(uiCode + 1) >> 1) : short(uiCode >> 1));

                    m_cBuffer.Append(&p, sizeof(short));
                }
                break;

            case TYPE_DELTAPOS2D:
                {
                    // code:
                    // 1 + 5
                    // 0,1 + 6
                    // 0,0 + 15

                    for (int i(0); i < 2; ++i)
                    {
                        uint uiCode(cBuffer.ReadBit());
                        if (uiCode != 0)
                            uiCode = cBuffer.ReadBits(5);
                        else
                        {
                            uiCode = cBuffer.ReadBit();
                            if (uiCode != 0)
                                uiCode = cBuffer.ReadBits(6) + 32;
                            else
                                uiCode = cBuffer.ReadBits(15);
                        }

                        // Map 0,-1,1,-2,2 to -2,-1,0,1,2
                        short p(uiCode & 1 ? short(-int(uiCode + 1) >> 1) : short(uiCode >> 1));

                        m_cBuffer.Append(&p, sizeof(short));
                    }
                }
                break;

            case TYPE_DELTAPOS3D:
                {
                    // code:
                    // 1 + 5
                    // 0,1 + 6
                    // 0,0 + 15

                    for (int i(0); i < 3; ++i)
                    {
                        uint uiCode(cBuffer.ReadBit());
                        if (uiCode != 0)
                            uiCode = cBuffer.ReadBits(5);
                        else
                        {
                            uiCode = cBuffer.ReadBit();
                            if (uiCode != 0)
                                uiCode = cBuffer.ReadBits(6) + 32;
                            else
                                uiCode = cBuffer.ReadBits(15);
                        }

                        // Map 0,-1,1,-2,2 to -2,-1,0,1,2
                        short p(uiCode & 1 ? short(-int(uiCode + 1) >> 1) : short(uiCode >> 1));

                        m_cBuffer.Append(&p, sizeof(short));
                    }
                }
                break;
            }

            if (uiSize)
            {
                uint uiBits(it->uiBits);

                if (uiBits == 0)
                    uiBits = uiSize * 8;

                while (uiBits > 8)
                {
                    m_cBuffer.WriteByte(cBuffer.ReadBits(8));

                    uiBits -= 8;
                    --uiSize;
                }

                m_cBuffer.WriteByte(cBuffer.ReadBits(uiBits));
                --uiSize;

                if (uiSize > 0)
                    m_cBuffer.WriteBytes(0, uiSize);
            }
        }

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::ReadBody() - "), NO_THROW);
        return false;
    }
}


/*====================
  CEntitySnapshot::ReadBody
  ====================*/
bool    CEntitySnapshot::ReadBody(CBufferBit &cBuffer, const TypeVector &vTypes, uint uiTypeSize, const CEntitySnapshot *pBaseline, bool bSetAllFields)
{
    try
    {
        SetFieldTypes(&vTypes, uiTypeSize);
        m_pBaseline = pBaseline;

        ReadTransmitFlags(cBuffer);
        
        pBaseline->RewindRead();

        // Read the entity data
        for (TypeVector::const_iterator it(m_pFieldTypes->begin()), itEnd(m_pFieldTypes->end()); it != itEnd; ++it)
        {
            if (cBuffer.GetFaults() != 0)
                EX_ERROR(_T("Truncated entity"));

            uint uiFieldIndex(uint(it - m_pFieldTypes->begin()));
            uint uiSize(0);

            switch (it->eDataType)
            {
            case TYPE_CHAR:
            case TYPE_BYTEPERCENT:
            case TYPE_ANGLE8:
                uiSize = 1;
                break;

            case TYPE_GAMEINDEX:
            case TYPE_RESHANDLE:
            case TYPE_ANGLE16:
            case TYPE_ROUND16:
            case TYPE_FLOOR16:
            case TYPE_CEIL16:
            case TYPE_SHORT:
                uiSize = 2;
                break;

            case TYPE_INT:
            case TYPE_FLOAT:
                uiSize = 4;
                break;

            case TYPE_V2F:
                uiSize = 8;
                break;

            case TYPE_V3F:
                uiSize = 12;
                break;

            case TYPE_ROUNDPOS3D:
                uiSize = 6;
                break;

            case TYPE_DELTAPOS1D:
                {
                    const char *pBuffer(pBaseline->GetNextField(2));

                    if (!IsFieldSet(uiFieldIndex))
                        WriteDiffField(pBuffer, 2);
                    else
                    {
                        // code:
                        // 1 + 5
                        // 0,1 + 6
                        // 0,0 + 15 

                        uint uiCode(cBuffer.ReadBit());
                        if (uiCode != 0)
                            uiCode = cBuffer.ReadBits(5);
                        else
                        {
                            uiCode = cBuffer.ReadBit();
                            if (uiCode != 0)
                                uiCode = cBuffer.ReadBits(6) + 32;
                            else
                                uiCode = cBuffer.ReadBits(15);
                        }

                        // Map 0,-1,1,-2,2 to -2,-1,0,1,2
                        short p(uiCode & 1 ? short(-int(uiCode + 1) >> 1) : short(uiCode >> 1));

                        m_cBuffer.Append(&p, sizeof(short));
                    }
                }
                break;

            case TYPE_DELTAPOS2D:
                {
                    const char *pBuffer(pBaseline->GetNextField(4));

                    if (!IsFieldSet(uiFieldIndex))
                        WriteDiffField(pBuffer, 4);
                    else
                    {
                        for (int i(0); i < 2; ++i)
                        {
                            // code:
                            // 1 + 5
                            // 0,1 + 6
                            // 0,0 + 15 

                            uint uiCode(cBuffer.ReadBit());
                            if (uiCode != 0)
                                uiCode = cBuffer.ReadBits(5);
                            else
                            {
                                uiCode = cBuffer.ReadBit();
                                if (uiCode != 0)
                                    uiCode = cBuffer.ReadBits(6) + 32;
                                else
                                    uiCode = cBuffer.ReadBits(15);
                            }

                            // Map 0,-1,1,-2,2 to -2,-1,0,1,2
                            short p(uiCode & 1 ? short(-int(uiCode + 1) >> 1) : short(uiCode >> 1));

                            m_cBuffer.Append(&p, sizeof(short));
                        }
                    }
                }
                break;

            case TYPE_DELTAPOS3D:
                {
                    const char *pBuffer(pBaseline->GetNextField(6));

                    if (!IsFieldSet(uiFieldIndex))
                        WriteDiffField(pBuffer, 6);
                    else
                    {
                        for (int i(0); i < 3; ++i)
                        {
                            // code:
                            // 1 + 5
                            // 0,1 + 6
                            // 0,0 + 15 

                            uint uiCode(cBuffer.ReadBit());
                            if (uiCode != 0)
                                uiCode = cBuffer.ReadBits(5);
                            else
                            {
                                uiCode = cBuffer.ReadBit();
                                if (uiCode != 0)
                                    uiCode = cBuffer.ReadBits(6) + 32;
                                else
                                    uiCode = cBuffer.ReadBits(15);
                            }

                            // Map 0,-1,1,-2,2 to -2,-1,0,1,2
                            short p(uiCode & 1 ? short(-int(uiCode + 1) >> 1) : short(uiCode >> 1));

                            m_cBuffer.Append(&p, sizeof(short));
                        }
                    }
                }
                break;

            default:
                EX_ERROR(_T("Invalid entity field data type"));
                break;
            }

            if (uiSize)
            {
                const char *pBuffer(pBaseline->GetNextField(uiSize));

                if (!IsFieldSet(uiFieldIndex))
                    WriteDiffField(pBuffer, uiSize);
                else
                {
                    uint uiBits(it->uiBits);

                    if (uiBits == 0)
                        uiBits = uiSize * 8;

                    while (uiBits > 8)
                    {
                        m_cBuffer.WriteByte(cBuffer.ReadBits(8));

                        uiBits -= 8;
                        --uiSize;
                    }

                    m_cBuffer.WriteByte(cBuffer.ReadBits(uiBits));
                    --uiSize;

                    if (uiSize > 0)
                        m_cBuffer.WriteBytes(0, uiSize);
                }
            }
        }

        if (bSetAllFields)
            SetAllFields();

        return true;
    }
    catch (CException &ex)
    {
        SetChanged(false);
        ex.Process(_T("CEntitySnapshot::ReadBody() - "), NO_THROW);
        return false;
    }
}


/*====================
  CEntitySnapshot::DiffFrom
  ====================*/
void    CEntitySnapshot::DiffFrom(const CEntitySnapshot &base, CEntitySnapshot &result) const
{
    PROFILE("CEntitySnapshot::DiffFrom");

    try
    {
#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        if (m_uiIndex != base.m_uiIndex && base.m_uiIndex != INVALID_INDEX)
            EX_ERROR(_T("Entity diff index mismatch"));
#endif

        result.m_uiIndex = m_uiIndex;
        result.m_unType = m_unType;
        result.SetFieldTypes(m_pFieldTypes, m_uiTypeSize);

        const CEntitySnapshot *pBase(NULL);

        if (base.m_unType != m_unType || m_uiUniqueID != base.m_uiUniqueID)
        {
            pBase = m_pBaseline;
            result.SetTypeChange(true);
        }
        else
        {
            pBase = &base;
            result.SetTypeChange(false);
        }
        
        if (!pBase)
        {
            result.SetChanged(true);
            result.m_cBuffer = m_cBuffer;
            result.SetAllFields();
        }
        else
        {
            if (!result.GetTypeChange() && m_uiPublicSequence == pBase->m_uiPublicSequence)
            {
                result.SetChanged(false);
                return;
            }

            result.SetChanged(result.GetTypeChange());

            m_cBuffer.Rewind();
            pBase->RewindRead();

            const CBufferDynamic &cBaseBuffer(pBase->m_cBuffer);

            for (TypeVector::const_iterator it(m_pFieldTypes->begin()), itEnd(m_pFieldTypes->end()); it != itEnd; ++it)
            {
                const uint ui(uint(it - m_pFieldTypes->begin()));
                const SDataField &cField(*it);

                uint uiSize(0);

                switch (cField.eDataType)
                {
                case TYPE_CHAR:
                case TYPE_BYTEPERCENT:
                case TYPE_ANGLE8:
                    uiSize = 1;
                    break;

                case TYPE_GAMEINDEX:
                case TYPE_RESHANDLE:
                case TYPE_ANGLE16:
                case TYPE_ROUND16:
                case TYPE_FLOOR16:
                case TYPE_CEIL16:
                case TYPE_SHORT:
                    uiSize = 2;
                    break;

                case TYPE_INT:
                case TYPE_FLOAT:
                    uiSize = 4;
                    break;

                case TYPE_V2F:
                    uiSize = 8;
                    break;

                case TYPE_V3F:
                    uiSize = 12;
                    break;

                case TYPE_ROUNDPOS3D:
                    uiSize = 6;
                    break;

                case TYPE_DELTAPOS1D:
                    {
                        if (m_cBuffer.CompareBuffer(cBaseBuffer, 2) == 0)
                        {
                            cBaseBuffer.Advance(2);
                            m_cBuffer.Advance(2);
                            continue;
                        }

                        short p1, p2;
                        cBaseBuffer.Read(&p1, sizeof(short));
                        m_cBuffer.Read(&p2, sizeof(short));

                        p2 -= p1;

                        result.WriteDiffField((const char *)&p2, sizeof(p2));
                        result.SetField(ui);
                        result.SetChanged(true);
                    }
                    break;

                case TYPE_DELTAPOS2D:
                    {
                        if (m_cBuffer.CompareBuffer(cBaseBuffer, 4) == 0)
                        {
                            cBaseBuffer.Advance(4);
                            m_cBuffer.Advance(4);
                            continue;
                        }

                        CVec2<short> p1;
                        cBaseBuffer.Read(&p1.x, sizeof(short));
                        cBaseBuffer.Read(&p1.y, sizeof(short));

                        CVec2<short> p2;
                        m_cBuffer.Read(&p2.x, sizeof(short));
                        m_cBuffer.Read(&p2.y, sizeof(short));

                        p2 -= p1;

                        result.WriteDiffField((const char *)&p2, sizeof(p2));
                        result.SetField(ui);
                        result.SetChanged(true);
                    }
                    break;

                case TYPE_DELTAPOS3D:
                    {
                        if (m_cBuffer.CompareBuffer(cBaseBuffer, 6) == 0)
                        {
                            cBaseBuffer.Advance(6);
                            m_cBuffer.Advance(6);
                            continue;
                        }

                        CVec3<short> p1;
                        cBaseBuffer.Read(&p1.x, sizeof(short));
                        cBaseBuffer.Read(&p1.y, sizeof(short));
                        cBaseBuffer.Read(&p1.z, sizeof(short));

                        CVec3<short> p2;
                        m_cBuffer.Read(&p2.x, sizeof(short));
                        m_cBuffer.Read(&p2.y, sizeof(short));
                        m_cBuffer.Read(&p2.z, sizeof(short));

                        p2 -= p1;

                        result.WriteDiffField((const char *)&p2, sizeof(p2));
                        result.SetField(ui);
                        result.SetChanged(true);
                    }
                    break;
                }

                if (uiSize)
                {
                    if (m_cBuffer.CompareBuffer(cBaseBuffer, uiSize) == 0)
                    {
                        cBaseBuffer.Advance(uiSize);
                        m_cBuffer.Advance(uiSize);
                        continue;
                    }

                    cBaseBuffer.Advance(uiSize);

                    result.WriteDiffField(m_cBuffer, uiSize);
                    result.SetField(ui);
                    result.SetChanged(true);
                }
            }
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::DiffFrom() - "), NO_THROW);
    }
}


#if 0
/*====================
  CEntitySnapshot::CalcSequence
  ====================*/
void    CEntitySnapshot::CalcSequence(const CEntitySnapshot &base)
{
    PROFILE("CEntitySnapshot::CalcSequence");

    try
    {
#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        if (m_uiIndex != base.m_uiIndex && base.m_uiIndex != INVALID_INDEX)
            EX_ERROR(_T("Entity diff index mismatch"));
#endif

        if (base.m_uiUniqueID != m_uiUniqueID)
        {
            m_uiPublicSequence = base.m_uiPublicSequence + 1;
            return;
        }

        m_uiPublicSequence = base.m_uiPublicSequence;

        m_cBuffer.RewindBuffer();
        base.RewindRead();

        const CBufferDynamic &cBaseBuffer(base.m_cBuffer);

        if (m_cBuffer.FindFirstDiff(cBaseBuffer) != m_cBuffer.GetBufferLength())
        {
            m_uiPublicSequence = base.m_uiPublicSequence + 1;
            return;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::CalcSequence() - "), NO_THROW);
    }
}
#endif


/*====================
  CEntitySnapshot::CalcSequence
  ====================*/
void    CEntitySnapshot::CalcSequence()
{
    PROFILE("CEntitySnapshot::CalcSequence");

    try
    {
#ifdef K2_VALIDATE_ENTITY_SNAPSHOT
        if (m_uiIndex != base.m_uiIndex && base.m_uiIndex != INVALID_INDEX)
            EX_ERROR(_T("Entity diff index mismatch"));
#endif

        m_uiPublicSequence = crc32(0, (const Bytef*)m_cBuffer.GetBuffer(), m_cBuffer.GetBufferLength());
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntitySnapshot::CalcSequence() - "), NO_THROW);
    }
}


/*====================
  CEntitySnapshot::WriteBuffer

  Indexes are only sent when an entity starts and stops changing.
  This is tracked between successive frames.

  Codes for index deltas:
  header+delta bits
  1 + 0
  0,1 + 4
  0,0,1 + 8
  0,0,0 + 15
  ====================*/
void    CEntitySnapshot::WriteBuffer(CBufferBit &cBuffer, bool bSendIndex, uint &uiLastIndex, bool bChanging) const
{
    try
    {
        if (bSendIndex || GetTypeChange())
        {
            cBuffer.WriteBit(1);

            uint uiDelta(m_uiIndex - uiLastIndex);
            uiLastIndex = m_uiIndex;

            // Encode delta
            if (uiDelta == 1)
                cBuffer.WriteBit(1); // 1
            else if (uiDelta < 1 + 16)
            {
                cBuffer.WriteBits(2, 2); // 0,1
                cBuffer.WriteBits(uiDelta - 1, 4);
            }
            else if (uiDelta < 1 + 16 + 256)
            {
                cBuffer.WriteBits(4, 3); // 0,0,1
                cBuffer.WriteBits(uiDelta - 1 - 16, 8);
            }
            else
            {
                cBuffer.WriteBits(0, 3); // 0,0,0
                cBuffer.WriteBits(uiDelta - 1 - 16 - 256, 15);
            }

            // Next bit indicates type change
            if (GetTypeChange())
            {
                cBuffer.WriteBit(1);
                cBuffer.WriteBits(m_unType, 16);
            }
            else
            {
                cBuffer.WriteBit(0);
            }

            cBuffer.WriteBit(bChanging ? 1 : 0); // Changing flag
        }
        else
        {
            uiLastIndex = m_uiIndex;

            cBuffer.WriteBit(0);
        }

        if (m_unType == 0)
            return;

        WriteTransmitFlags(cBuffer);

        m_cBuffer.Rewind();

        // Write the fields
        for (TypeVector::const_iterator it(m_pFieldTypes->begin()), itEnd(m_pFieldTypes->end()); it != itEnd; ++it)
        {
            if (cBuffer.GetFaults() != 0)
                EX_ERROR(_T("Truncated entity"));

            if (!IsFieldSet(uint(it - m_pFieldTypes->begin())))
                continue;

            uint uiSize(0);

            switch (it->eDataType)
            {
            case TYPE_CHAR:
            case TYPE_BYTEPERCENT:
            case TYPE_ANGLE8:
                uiSize = 1;
                break;

            case TYPE_GAMEINDEX:
            case TYPE_RESHANDLE:
            case TYPE_ANGLE16:
            case TYPE_ROUND16:
            case TYPE_FLOOR16:
            case TYPE_CEIL16:
            case TYPE_SHORT:
                uiSize = 2;
                break;

            case TYPE_INT:
            case TYPE_FLOAT:
                uiSize = 4;
                break;

            case TYPE_V2F:
                uiSize = 8;
                break;

            case TYPE_V3F:
                uiSize = 12;
                break;

            case TYPE_ROUNDPOS3D:
                uiSize = 6;
                break;

            case TYPE_DELTAPOS1D:
                {
                    // code:
                    // 1 + 5
                    // 0,1 + 6
                    // 0,0 + 15 

                    short p;
                    m_cBuffer.Read(&p, sizeof(short));

                    // Map -2,-1,0,1,2 to 0,-1,1,-2,2
                    uint uiCode(p >= 0 ? uint(p) << 1 : (uint(-p) << 1) - 1);

                    if (uiCode < 32)
                    {
                        cBuffer.WriteBit(1); // 1
                        cBuffer.WriteBits(uiCode, 5);
                    }
                    else if (uiCode < 32 + 64)
                    {
                        cBuffer.WriteBits(2, 2); // 0,1
                        cBuffer.WriteBits(uiCode - 32, 6);
                    }
                    else
                    {
                        cBuffer.WriteBits(0, 2);
                        cBuffer.WriteBits(uiCode, 15);
                    }
                }
                break;

            case TYPE_DELTAPOS2D:
                {
                    for (int i(0); i < 2; ++i)
                    {
                        // code:
                        // 1 + 5
                        // 0,1 + 6
                        // 0,0 + 15

                        short p;
                        m_cBuffer.Read(&p, sizeof(short));

                        // Map -2,-1,0,1,2 to 0,-1,1,-2,2
                        uint uiCode(p >= 0 ? uint(p) << 1 : (uint(-p) << 1) - 1);

                        if (uiCode < 32)
                        {
                            cBuffer.WriteBit(1); // 1
                            cBuffer.WriteBits(uiCode, 5);
                        }
                        else if (uiCode < 32 + 64)
                        {
                            cBuffer.WriteBits(2, 2); // 0,1
                            cBuffer.WriteBits(uiCode - 32, 6);
                        }
                        else
                        {
                            cBuffer.WriteBits(0, 2);
                            cBuffer.WriteBits(uiCode, 15);
                        }
                    }
                }
                break;

            case TYPE_DELTAPOS3D:
                {
                    for (int i(0); i < 3; ++i)
                    {
                        // code:
                        // 1 + 5
                        // 0,1 + 6
                        // 0,0 + 15

                        short p;
                        m_cBuffer.Read(&p, sizeof(short));

                        // Map -2,-1,0,1,2 to 0,-1,1,-2,2
                        uint uiCode(p >= 0 ? uint(p) << 1 : (uint(-p) << 1) - 1);

                        if (uiCode < 32)
                        {
                            cBuffer.WriteBit(1); // 1
                            cBuffer.WriteBits(uiCode, 5);
                        }
                        else if (uiCode < 32 + 64)
                        {
                            cBuffer.WriteBits(2, 2); // 0,1
                            cBuffer.WriteBits(uiCode - 32, 6);
                        }
                        else
                        {
                            cBuffer.WriteBits(0, 2);
                            cBuffer.WriteBits(uiCode, 15);
                        }
                    }
                }
                break;
            }

            if (uiSize)
            {
                uint uiBits(it->uiBits);

                if (uiBits == 0)
                    uiBits = uiSize * 8;

                while (uiBits > 8)
                {
                    cBuffer.WriteBits(m_cBuffer.ReadByte(), 8);
                    uiBits -= 8;
                    --uiSize;
                }

                cBuffer.WriteBits(m_cBuffer.ReadByte(), uiBits);
                --uiSize;

                if (uiSize > 0)
                    m_cBuffer.Advance(uiSize);
            }
        }
    }
    catch (CException &ex)
    {
        cBuffer.Clear();
        ex.Process(_T("CEntitySnapshot::WriteBuffer() - "), NO_THROW);
    }
}


/*====================
  CEntitySnapshot::SetFieldTypes
  ====================*/
void    CEntitySnapshot::SetFieldTypes(const TypeVector *pFieldTypes, uint uiSize)
{
    m_pFieldTypes = pFieldTypes;
    m_uiTypeSize = uiSize;
    m_cTransmitFlags.SetNumFields(uint(m_pFieldTypes->size()));
    m_itWriteField = m_pFieldTypes->begin();

    uint uiSize2(CEIL_MULTIPLE<4>(uiSize));

    if (m_cBuffer.GetCapacity() < uiSize2)
        m_cBuffer.Reallocate(uiSize2);
}


/*====================
  CEntitySnapshot::CalcSnapshotSize
  ====================*/
uint    CEntitySnapshot::CalcSnapshotSize(const TypeVector *pFieldTypes)
{
    uint uiSize(0);

    for (uint ui(0); ui < pFieldTypes->size(); ++ui)
    {
        switch ((*pFieldTypes)[ui].eDataType)
        {
        case TYPE_CHAR:             uiSize += 1;    break;
        case TYPE_SHORT:            uiSize += 2;    break;
        case TYPE_INT:              uiSize += 4;    break;
        case TYPE_FLOAT:            uiSize += 4;    break;
        case TYPE_V2F:              uiSize += 8;    break;
        case TYPE_V3F:              uiSize += 12;   break;
        case TYPE_GAMEINDEX:        uiSize += 2;    break;
        case TYPE_RESHANDLE:        uiSize += 2;    break;
        case TYPE_ANGLE8:           uiSize += 1;    break;
        case TYPE_ANGLE16:          uiSize += 2;    break;
        case TYPE_ROUND16:          uiSize += 2;    break;
        case TYPE_FLOOR16:          uiSize += 2;    break;
        case TYPE_CEIL16:           uiSize += 2;    break;
        case TYPE_BYTEPERCENT:      uiSize += 1;    break;
        case TYPE_ROUNDPOS3D:       uiSize += 6;    break;
        case TYPE_DELTAPOS1D:       uiSize += 2;    break;
        case TYPE_DELTAPOS2D:       uiSize += 4;    break;
        case TYPE_DELTAPOS3D:       uiSize += 6;    break;
        }
    }

    return uiSize;
}


/*====================
  CEntitySnapshot::ApplyDiff
  ====================*/
void    CEntitySnapshot::ApplyDiff(CEntitySnapshot &cDiff)
{
    PROFILE("CEntitySnapshot::ApplyDiff");

    try
    {
        if (m_uiIndex != INVALID_INDEX && cDiff.m_unType == m_unType && cDiff.m_pFieldTypes != m_pFieldTypes)
            EX_ERROR(_T("Entity field type mismatch"));

        RewindRead();
        cDiff.RewindRead();

        SetChanging(cDiff.GetChanging());
        
        // Read the entity data
        int index(0);
        for (TypeVector::const_iterator it(m_pFieldTypes->begin()), itEnd(m_pFieldTypes->end()); it != itEnd; ++it, ++index)
        {
            uint uiSize(0);

            switch (it->eDataType)
            {
            case TYPE_CHAR:
            case TYPE_BYTEPERCENT:
            case TYPE_ANGLE8:
                uiSize = 1;
                break;

            case TYPE_GAMEINDEX:
            case TYPE_RESHANDLE:
            case TYPE_ANGLE16:
            case TYPE_ROUND16:
            case TYPE_FLOOR16:
            case TYPE_CEIL16:
            case TYPE_SHORT:
                uiSize = 2;
                break;

            case TYPE_INT:
            case TYPE_FLOAT:
                uiSize = 4;
                break;

            case TYPE_V2F:
                uiSize = sizeof(CVec2f);
                break;

            case TYPE_V3F:
                uiSize = sizeof(CVec3f);
                break;

            case TYPE_ROUNDPOS3D:
                uiSize = 6;
                break;

            case TYPE_DELTAPOS1D:
                {
                    if (cDiff.IsFieldSet(index))
                    {
                        short p1, p2;
                        cDiff.m_cBuffer.Read(&p1, sizeof(short));

                        uint uiReadPos(m_cBuffer.GetReadPos());
                        m_cBuffer.Read(&p2, sizeof(short));

                        p2 += p1;

                        m_cBuffer.Seek(uiReadPos);
                        m_cBuffer.Overwrite(&p2, 2);
                    }
                    else
                    {
                        m_cBuffer.Advance(2);
                    }
                }
                break;

            case TYPE_DELTAPOS2D:
                {
                    if (cDiff.IsFieldSet(index))
                    {
                        for (int i(0); i < 2; ++i)
                        {
                            short p1, p2;
                            cDiff.m_cBuffer.Read(&p1, sizeof(short));

                            uint uiReadPos(m_cBuffer.GetReadPos());
                            m_cBuffer.Read(&p2, sizeof(short));

                            p2 += p1;

                            m_cBuffer.Seek(uiReadPos);
                            m_cBuffer.Overwrite(&p2, 2);
                        }
                    }
                    else
                    {
                        m_cBuffer.Advance(4);
                    }
                }
                break;

            case TYPE_DELTAPOS3D:
                {
                    if (cDiff.IsFieldSet(index))
                    {
                        for (int i(0); i < 3; ++i)
                        {
                            short p1, p2;
                            cDiff.m_cBuffer.Read(&p1, sizeof(short));

                            uint uiReadPos(m_cBuffer.GetReadPos());
                            m_cBuffer.Read(&p2, sizeof(short));

                            p2 += p1;

                            m_cBuffer.Seek(uiReadPos);
                            m_cBuffer.Overwrite(&p2, 2);
                        }
                    }
                    else
                    {
                        m_cBuffer.Advance(6);
                    }
                }
                break;
            }

            if (uiSize)
            {
                if (cDiff.IsFieldSet(index))
                {
                    m_cBuffer.Overwrite(cDiff.m_cBuffer.Get(cDiff.m_cBuffer.GetReadPos()), uiSize);
                    cDiff.m_cBuffer.Advance(uiSize);
                }
                else
                {
                    m_cBuffer.Advance(uiSize);
                }
            }
        }
    }
    catch (CException &ex)
    {
        SetChanged(false);
        ex.Process(_T("CEntitySnapshot::ApplyDiff() - "), NO_THROW);
    }
}


/*====================
  CEntitySnapshot::GetField

  SLOW - Don't actually use this, it's just for debugging
  ====================*/
const char* CEntitySnapshot::GetField(uint uiFieldIndex) const
{
    m_cBuffer.Rewind();
    for (uint ui(0); ui < m_pFieldTypes->size() && ui <= uiFieldIndex; ++ui)
    {
        const SDataField &cField((*m_pFieldTypes)[ui]);

        if (!IsFieldSet(ui))
            continue;

        if (ui == uiFieldIndex)
            return m_cBuffer.Get(m_cBuffer.GetReadPos());

        uint uiSize(0);

        switch (cField.eDataType)
        {
        case TYPE_CHAR:
        case TYPE_BYTEPERCENT:
        case TYPE_ANGLE8:
            uiSize = 1;
            break;

        case TYPE_GAMEINDEX:
        case TYPE_RESHANDLE:
        case TYPE_ANGLE16:
        case TYPE_ROUND16:
        case TYPE_FLOOR16:
        case TYPE_CEIL16:
        case TYPE_DELTAPOS1D:
        case TYPE_SHORT:
            uiSize = 2;
            break;

        case TYPE_DELTAPOS2D:
        case TYPE_INT:
        case TYPE_FLOAT:
            uiSize = 4;
            break;

        case TYPE_V2F:
            uiSize = 8;
            break;

        case TYPE_V3F:
            uiSize = 12;
            break;

        case TYPE_ROUNDPOS3D:
        case TYPE_DELTAPOS3D:
            uiSize = 6;
            break;
        }

        if (uiSize)
        {
            m_cBuffer.Advance(uiSize);
        }
    }

    return NULL;
}
