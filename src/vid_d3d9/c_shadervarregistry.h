// (C)2005 S2 Games
// c_shadervarregistry.h
//
//=============================================================================
#ifndef __C_SHADERVARREGISTRY_H__
#define __C_SHADERVARREGISTRY_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CShaderVar;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef map<tstring, CShaderVar*>           ShaderVarMap;
//=============================================================================

//=============================================================================
// CShaderVarRegistry
//=============================================================================
class CShaderVarRegistry
{
private:
    static CShaderVarRegistry   *s_pInstance;
    static bool             s_bRequested, s_bReleased;

    CShaderVarRegistry() {}
    CShaderVarRegistry(CShaderVarRegistry&);
    CShaderVarRegistry& operator=(CShaderVarRegistry&);

    ShaderVarMap        m_mapShaderVars;

public:
    static CShaderVarRegistry*  GetInstance();
    static void             Release();
    static bool             IsReleased()    { return s_bReleased; }

    void                    Register(CShaderVar *pShaderVar);
    void                    Unregister(const tstring &sName);

    inline CShaderVar*      GetShaderVar(const tstring &sShaderVar);

    const ShaderVarMap&     GetShaderVarMap()   { return m_mapShaderVars; }

    inline bool             Exists(const tstring &sShaderVar);
};


/*====================
  CShaderVarRegistry::Exists
  ====================*/
bool    CShaderVarRegistry::Exists(const tstring &sShaderVar)
{
    ShaderVarMap::iterator find = m_mapShaderVars.find(sShaderVar);

    if (find == m_mapShaderVars.end())
        return false;

    return true;
}


/*====================
  CShaderVarRegistry::GetShaderVar
  ====================*/
CShaderVar  *CShaderVarRegistry::GetShaderVar(const tstring &sShaderVar)
{
    ShaderVarMap::iterator find = m_mapShaderVars.find(sShaderVar);

    if (find == m_mapShaderVars.end())
        return NULL;
    else
        return find->second;
}
//=============================================================================
#endif //__C_SHADERVARREGISTRY_H__
