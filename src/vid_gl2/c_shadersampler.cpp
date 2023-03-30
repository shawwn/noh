// (C)2005 S2 Games
// c_shadersampler.cpp
//
// Self registering shader sampler controllers
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_shadersampler.h"
#include "c_shadersamplerregistry.h"

#include "../k2/c_cmd.h"
//=============================================================================

/*====================
  CShaderSampler::CShaderSampler
  ====================*/
CShaderSampler::CShaderSampler(const tstring &sName, ShaderSamplerFn_t pfnShaderSamplerCmd) :
m_sName(sName),
m_pfnShaderSampler(pfnShaderSamplerCmd)
{
    if (m_pfnShaderSampler == NULL)
        K2System.Error(_T("Tried to register a ShaderSampler with a NULL function."));

    CShaderSamplerRegistry::GetInstance()->Register(this);
}


/*====================
  CShaderSampler::~CShaderSampler
  ====================*/
CShaderSampler::~CShaderSampler()
{
    // If the registry is still valid, unregister the uicmd
    // This is important for any actions declared in a client dll that
    // is being unloaded
    if (!CShaderSamplerRegistry::IsReleased())
        CShaderSamplerRegistry::GetInstance()->Unregister(m_sName);
}


/*====================
  CShaderSampler::Get
  ====================*/
bool    CShaderSampler::Get(int iStageIndex)
{
    return m_pfnShaderSampler(this, iStageIndex);
}


/*--------------------
  cmdShaderSamplerList

  prints a list of all shader variables
  --------------------*/
CMD(ShaderSamplerList)
{
    int iNumFound(0);

    const ShaderSamplerMap &lVars = CShaderSamplerRegistry::GetInstance()->GetShaderSamplerMap();

    // Print shader variables
    for (ShaderSamplerMap::const_iterator it(lVars.begin()); it != lVars.end(); ++it)
    {
        if (vArgList.size() == 0 || it->second->GetName().find(vArgList[0]) != string::npos)
        {
            Console << it->second->GetName() << newl;
            ++iNumFound;
        }
    }

    Console << newl << iNumFound << _T(" matching ShaderSamplers found") << newl;

    return true;
}
