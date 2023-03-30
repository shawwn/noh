// (C)2005 S2 Games
// c_shaderpreprocessor.h
//
//=============================================================================
#ifndef __C_SHADERPREPROCESSOR_H__
#define __C_SHADERPREPROCESSOR_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef map<string, string> DefinitionMap;
//=============================================================================

//=============================================================================
// CShaderPreprocessor
//=============================================================================
class CShaderPreprocessor
{
private:
    DefinitionMap   m_mapDefinitions;

public:
    ~CShaderPreprocessor();
    CShaderPreprocessor();

    void    Define(const string &sName, const string &sDefinition = "");
    void    Undefine(const string &sName);

    string  GetDefinition(const string &sName);

    void    UndefineAll();

    D3DXMACRO*  AllocMacroArray();

    string  GetDefinitionHeaderString();
    string  GetDefinitionString();
};

extern  CShaderPreprocessor g_ShaderPreprocessor;
//=============================================================================
#endif //__C_SHADERPREPROCESSOR_H__
