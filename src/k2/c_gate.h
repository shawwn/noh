// (C)2007 S2 Games
// c_gate.h
//
//=============================================================================
#ifndef __C_GATE_H__
#define __C_GATE_H__

#define GATE_OPEN BIT(0)
#define GATE_CLOSE BIT(1)

class CGate
{
    float m_fPosition;
    uint m_uiFlags;

public:
    CGate(float fPosition, bool bOpen) : m_fPosition(fPosition) { if (bOpen) m_uiFlags = GATE_OPEN; else m_uiFlags = GATE_CLOSE; }

    bool operator<(CGate cComp) const { return m_fPosition < cComp.m_fPosition; }
    bool operator<(float fPosition) const { return m_fPosition < fPosition; }

    inline uint GetFlags() const { return m_uiFlags; }
    inline float GetPosition() const { return m_fPosition; }
};

#endif //__C_GATE_H__
