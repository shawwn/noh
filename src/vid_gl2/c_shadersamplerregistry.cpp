// (C)2005 S2 Games
// c_shadersamplerregistry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_shadersamplerregistry.h"
#include "c_shadersampler.h"

#include "../k2/stringutils.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CShaderSamplerRegistry  *CShaderSamplerRegistry::s_pInstance;
bool            CShaderSamplerRegistry::s_bRequested;
bool            CShaderSamplerRegistry::s_bReleased;

CShaderSamplerRegistry  *pShaderSamplerRegistry = CShaderSamplerRegistry::GetInstance();
//=============================================================================


/*====================
  CShaderSamplerRegistry::GetInstance
  ====================*/
CShaderSamplerRegistry* CShaderSamplerRegistry::GetInstance()
{
    assert(!s_bReleased);

    if (s_pInstance == nullptr)
    {
        assert(!s_bRequested);
        s_bRequested = true;
        s_pInstance = K2_NEW(ctx_GL2,    CShaderSamplerRegistry);
    }

    return s_pInstance;
}


/*====================
  CShaderSamplerRegistry::Release
  ====================*/
void    CShaderSamplerRegistry::Release()
{
    assert(!s_bReleased);

    if (s_pInstance != nullptr)
        K2_DELETE(s_pInstance);

    s_bReleased = true;
}


/*====================
  CShaderSamplerRegistry::Register
  ====================*/
void    CShaderSamplerRegistry::Register(CShaderSampler *pShaderSampler)
{
    // Make sure there is no name collision
    ShaderSamplerMap::iterator findit = m_mapShaderSamplers.find(pShaderSampler->GetName());
    if (findit != m_mapShaderSamplers.end())
    {
        Console.Err << _T("A shader variable named ") << QuoteStr(pShaderSampler->GetName())
                    << _T(" already exists.") << newl;
        return;
    }

    m_mapShaderSamplers[pShaderSampler->GetName()] = pShaderSampler;
}


/*====================
  CShaderSamplerRegistry::Unregister
  ====================*/
void    CShaderSamplerRegistry::Unregister(const tstring &sName)
{
    ShaderSamplerMap::iterator findit = m_mapShaderSamplers.find(sName);
    if (findit != m_mapShaderSamplers.end())
    {
        //Console.Dev << _T("Shader Variable ") << sName << _T(" has been unregistered.") << newl;
        m_mapShaderSamplers.erase(findit);
    }
}
