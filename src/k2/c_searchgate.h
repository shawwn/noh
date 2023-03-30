// (C)2007 S2 Games
// c_searchgate.h
//
//=============================================================================
#ifndef __C_SEARCHGATE_H__
#define __C_SEARCHGATE_H__
// the span of a node which is travelable by the entity

class CSearchGate
{
private:
    union
    {
        float m_fMin;
        float m_fPositiveX;
    };
    union
    {
        float m_fMax;
        float m_fPositiveY;
    };
    float m_fNegativeX;
    float m_fNegativeY;
    
public:
    CSearchGate() : m_fMin(FAR_AWAY), m_fMax(FAR_AWAY) { }
    CSearchGate(float fMin, float fMax) : m_fMin(fMin), m_fMax(fMax) { }
    CSearchGate(float fPosX, float fPosY, float fNegX, float fNegY) : m_fPositiveX(fPosX), m_fPositiveY(fPosY), m_fNegativeX(fNegX), m_fNegativeY(fNegY) { }

    void Trim(float fLength) { m_fMin += fLength; m_fMax -= fLength; }

    float Length() const { return m_fMax - m_fMin; }
    int Valid() const { return m_fMin != FAR_AWAY; }
    float Min() const { return m_fMin; }
    float Max() const { return m_fMax; }
};

#endif //__C_SEARCHGATE_H__
