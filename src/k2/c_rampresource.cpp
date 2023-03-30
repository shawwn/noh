// (C)2009 S2 Games
// c_Rampresource.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"
#include "c_rampresource.h"

#include "../k2/i_resourcelibrary.h"
#include "../k2/c_xmlmanager.h"
//=============================================================================

IResource*  AllocRampResource(const tstring &sPath);

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary    g_ResLibRamp(RES_RAMP, _T("RampResource"), CRampResource::ResTypeName(), true, AllocRampResource);

//=============================================================================
// Definitions
//=============================================================================
IResource*  AllocRampResource(const tstring &sPath)
{
    return K2_NEW(ctx_Resources,  CRampResource)(sPath);
}

/*====================
  CRampResource::Load
  ====================*/
int     CRampResource::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
    PROFILE("CRampResource::Load");

    Console.Res << _T("Loading ^970Ramp definition^*: ") << m_sPath << newl;

    // Process the XML
    if (!XMLManager.ReadBuffer(pData, uiSize, _T(""), this))
    {
        Console.Warn << _T("CCliffDefinitionResource::Load(") + m_sPath + _T(") - couldn't read XML") << newl;
        return RES_LOAD_FAILED;
    }
    return 0;
}

/*====================
  CRampResource::PostLoad
  ====================*/
void    CRampResource::PostLoad()
{

}

/*====================
  CRampResource::Reloaded
  ====================*/
void    CRampResource::Reloaded()
{
}
