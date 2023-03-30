// (C)2006 S2 Games
// c_worldlight.h
//
//=============================================================================
#ifndef __C_WORLDLIGHT_H__
#define __C_WORLDLIGHT_H__

//=============================================================================
// CWorldLight
//=============================================================================
class CWorldLight
{
private:
    uint    m_uiIndex;
    CVec3f  m_v3Position;
    CVec3f  m_v3Color;
    float   m_fFalloffStart;
    float   m_fFalloffEnd;

public:
    ~CWorldLight()  {}
    CWorldLight()   {}

    CWorldLight& operator=(const CWorldLight &light)
    {
        // Clone light settings
        m_v3Position = light.m_v3Position;
        m_v3Color = light.m_v3Color;
        m_fFalloffStart = light.m_fFalloffStart;
        m_fFalloffEnd = light.m_fFalloffEnd;

        return *this;
    }

    uint    GetIndex() const                    { return m_uiIndex; }
    void    SetIndex(uint uiIndex)              { m_uiIndex = uiIndex; }

    const CVec3f&   GetPosition() const         { return m_v3Position; }
    void    SetPosition(const CVec3f &v3Pos)    { m_v3Position = v3Pos; }
    void    Translate(const CVec3f &v3Pos)      { m_v3Position += v3Pos; }

    const CVec3f&   GetColor() const            { return m_v3Color; }
    float   GetColor(EColorComponent e)         { return m_v3Color[e]; }
    float   GetRed() const                      { return m_v3Color[R]; }
    float   GetGreen() const                    { return m_v3Color[G]; }
    float   GetBlue() const                     { return m_v3Color[B]; }
    void    SetColor(const CVec3f &v3Color)     { m_v3Color = v3Color; }
    void    SetRed(float fRed)                  { m_v3Color[R] = fRed; }
    void    SetGreen(float fGreen)              { m_v3Color[G] = fGreen; }
    void    SetBlue(float fBlue)                { m_v3Color[B] = fBlue; }
    void    SetColor(float fR, float fG, float fB)  { m_v3Color.Set(fR, fG, fB); }
    void    SetColor(EColorComponent e, float f)    { m_v3Color[e] = f; }

    float   GetFalloffStart() const             { return m_fFalloffStart; }
    void    SetFalloffStart(float fStart)       { m_fFalloffStart = fStart; }
    float   GetFalloffEnd() const               { return m_fFalloffEnd; }
    void    SetFalloffEnd(float fEnd)           { m_fFalloffEnd = fEnd; }
    void    ScaleFalloff(float fScale)          { m_fFalloffStart *= fScale; m_fFalloffEnd *= fScale; }
};
//=============================================================================

#endif //__C_WORLDLIGHT_H__
