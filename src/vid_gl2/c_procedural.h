// (C)2005 S2 Games
// c_procedural.h
//
//=============================================================================
#ifndef __C_PROCEDURAL_H__
#define __C_PROCEDURAL_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

class CProcedural;
enum ETextureFormat;

//=============================================================================
// Definitions
//=============================================================================
typedef CVec4f (*ProceduralFn_t)(const CProcedural *pThis, float fU, float fV);

// Declaration macros
#define PROCEDURAL(name, width, height, format) \
CVec4f  procedural##name##Fn(const CProcedural *pThis, float fU, float fV); \
CProcedural  procedural##name(_T(#name), width, height, format, 0, procedural##name##Fn); \
CVec4f  procedural##name##Fn(const CProcedural *pThis, float fU, float fV)


#define PROCEDURAL_EX(name, width, height, format, flags) \
CVec4f  procedural##name##Fn(const CProcedural *pThis, float fU, float fV); \
CProcedural  procedural##name(_T(#name), width, height, format, flags, procedural##name##Fn); \
CVec4f  procedural##name##Fn(const CProcedural *pThis, float fU, float fV)

typedef CVec4f (*ProceduralMipmapsFn_t)(const CProcedural *pThis, float fU, float fV, int iLevel);

// Declaration macros
#define PROCEDURAL_MIPMAPS(name, width, height, format) \
CVec4f  procedural##name##Fn(const CProcedural *pThis, float fU, float fV, int iLevel); \
CProcedural  procedural##name(_T(#name), width, height, format, 0, procedural##name##Fn); \
CVec4f  procedural##name##Fn(const CProcedural *pThis, float fU, float fV, int iLevel)

#define PROCEDURAL_MIPMAPS_EX(name, width, height, format, flags) \
CVec4f  procedural##name##Fn(const CProcedural *pThis, float fU, float fV, int iLevel); \
CProcedural  procedural##name(_T(#name), width, height, format, flags, procedural##name##Fn); \
CVec4f  procedural##name##Fn(const CProcedural *pThis, float fU, float fV, int iLevel)
//=============================================================================

//=============================================================================
// CProcedural
//=============================================================================
class CProcedural
{
private:
    tstring                 m_sName;
    int                     m_iWidth;
    int                     m_iHeight;
    ETextureFormat          m_eFormat;
    int                     m_iFlags;
    ProceduralFn_t          m_pfnProcedural;
    ProceduralMipmapsFn_t   m_pfnProceduralMipmaps;

    // CProcedurals should not be copied
    CProcedural(CProcedural&);
    CProcedural& operator=(CProcedural&);

public:
    ~CProcedural();
    CProcedural(const tstring &sName, int iWidth, int iHeight, ETextureFormat eFormat, int iFlags, ProceduralFn_t pfnCProceduralCmd);
    CProcedural(const tstring &sName, int iWidth, int iHeight, ETextureFormat eFormat, int iFlags, ProceduralMipmapsFn_t pfnCProceduralCmd);

    const tstring&  GetName() const     { return m_sName; }
    int             GetWidth() const    { return m_iWidth; }
    int             GetHeight() const   { return m_iHeight; }
    ETextureFormat  GetFormat() const   { return m_eFormat; }
    int             GetFlags() const    { return m_iFlags; }

    bool            IsMipmaps() const   { return m_pfnProceduralMipmaps != nullptr; }

    CVec4f      Get(float fU, float fV) const;
    CVec4f      Get(float fU, float fV, int iLevel) const;
};
//=============================================================================
#endif //__C_PROCEDURAL_H__
