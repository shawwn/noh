// (C)2007 S2 Games
// c_bytemanager.h
//
//=============================================================================
#ifndef __C_BYTEMANAGER_H__
#define __C_BYTEMANAGER_H__

#define LOW_BIT 0x01
#define HIGH_BIT 0x80
#define FULL_BYTE 0xFF
#define BITS_PER_BYTE 8

typedef unsigned int uint;
typedef unsigned char byte;

class CByteManager
{
private:
    static byte LongestConsecutive1s(byte yInput);
    inline static uint GateEdgeRight(byte yEdge);
    inline static uint GateEdgeLeft(byte yEdge);
    inline static byte GateDownRes(byte yInput);
    inline static byte OnesCount(byte yByte);

public:
    static byte m_yLongestConsecutive[256]; // returns a mask of the longest contiguous set of 1s
    static byte m_yDownres[256]; // returns a 4-bit (0-3) representation of the 8-bit input, merging two bits into one by if (bit0 && bit1 then 1)
    static byte m_yLeft[256]; // number of 1s on the left edge of the byte
    static byte m_yRight[256]; // number of 1s on the right edge of the byte
    static byte m_yOnesCount[256];

    static void Init();
    inline static uint EdgeFromRight(byte yInput) { return m_yRight[yInput]; }
    inline static uint EdgeFromLeft(byte yInput) { return m_yLeft[yInput]; }
    inline static byte LongestConsecutive(byte yInput) { return m_yLongestConsecutive[yInput]; }
    inline static byte DownRes(byte yInput) { return m_yDownres[yInput]; }
};

inline uint CByteManager::GateEdgeRight(byte yEdge)
{
    uint uiRet(0);

    if (yEdge == FULL_BYTE)
        return 8;

    while (yEdge & LOW_BIT)
    {
        yEdge >>= 1;
        ++uiRet;
    }

    return uiRet;
}


inline uint CByteManager::GateEdgeLeft(byte yEdge)
{
    uint uiRet(0);

    if (yEdge == FULL_BYTE)
        return 8;

    while (yEdge & HIGH_BIT)
    {
        yEdge <<= 1;
        ++uiRet;
    }

    return uiRet;
}


inline  byte CByteManager::GateDownRes(byte yInput)
{
    byte yRet(0);
    byte yInverse(~yInput);

    yRet |= (0xc0 & yInverse)?0:(1<<3);
    yRet |= (0x30 & yInverse)?0:(1<<2);
    yRet |= (0x0c & yInverse)?0:(1<<1);
    yRet |= (0x03 & yInverse)?0:(1);

    return yRet;
}


inline byte CByteManager::OnesCount(byte yByte)
{
    byte yRet(0);

    while (yByte > 0)
    {
        if (yByte & LOW_BIT)
            ++yRet;

        yByte >>= 1;
    }

    return yRet;
}


#endif // __C_BYTEMANAGER_H__
