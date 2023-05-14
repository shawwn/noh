// (C)2005 S2 Games
// c_shadersamplerregistry.h
//
//=============================================================================
#ifndef __C_SHADERSAMPLERREGISTRY_H__
#define __C_SHADERSAMPLERREGISTRY_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

class CShaderSampler;

//=============================================================================
// Definitions
//=============================================================================
typedef map<tstring, CShaderSampler*>           ShaderSamplerMap;

//=============================================================================

//=============================================================================
// CShaderSamplerRegistry
//=============================================================================
class CShaderSamplerRegistry
{
private:
    static CShaderSamplerRegistry   *s_pInstance;
    static bool             s_bRequested, s_bReleased;

    CShaderSamplerRegistry() {}
    CShaderSamplerRegistry(CShaderSamplerRegistry&);
    CShaderSamplerRegistry& operator=(CShaderSamplerRegistry&);

    ShaderSamplerMap        m_mapShaderSamplers;

public:
    static CShaderSamplerRegistry*  GetInstance();
    static void             Release();
    static bool             IsReleased()    { return s_bReleased; }

    void                    Register(CShaderSampler *pShaderSampler);
    void                    Unregister(const tstring &sName);

    inline CShaderSampler*      GetShaderSampler(const tstring &sShaderSampler);

    const ShaderSamplerMap&     GetShaderSamplerMap()   { return m_mapShaderSamplers; }

    inline bool             Exists(const tstring &sShaderSampler);
};


/*====================
  CShaderSamplerRegistry::Exists
  ====================*/
bool    CShaderSamplerRegistry::Exists(const tstring &sShaderSampler)
{
    ShaderSamplerMap::iterator find = m_mapShaderSamplers.find(sShaderSampler);

    if (find == m_mapShaderSamplers.end())
        return false;

    return true;
}


/*====================
  CShaderSamplerRegistry::GetShaderSampler
  ====================*/
CShaderSampler  *CShaderSamplerRegistry::GetShaderSampler(const tstring &sShaderSampler)
{
    ShaderSamplerMap::iterator find = m_mapShaderSamplers.find(sShaderSampler);

    if (find == m_mapShaderSamplers.end())
        return nullptr;
    else
        return find->second;
}
//=============================================================================
#endif //__C_SHADERSAMPLERREGISTRY_H__
