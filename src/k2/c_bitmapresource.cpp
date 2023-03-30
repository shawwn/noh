// (C)2009 S2 Games
// c_bitmapresource.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_bitmapresource.h"
#include "c_bitmap.h"

#include "../k2/i_resourcelibrary.h"
#include "../k2/c_xmlmanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
IResource*  AllocBitmapResource(const tstring &sPath);
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary    g_ResLibBitmapResource(RES_BITMAP, _T("Bitmaps"), CBitmapResource::ResTypeName(), true, AllocBitmapResource);
//=============================================================================

/*====================
  AllocBitmapResource
  ====================*/
IResource*  AllocBitmapResource(const tstring &sPath)
{
    return K2_NEW(ctx_Resources,  CBitmapResource)(sPath);
}


/*====================
  CBitmapResource::CBitmapResource
  ====================*/
CBitmapResource::CBitmapResource(const tstring &sPath) :
IResource(sPath, TSNULL)
{
}


/*====================
  CBitmapResource::Load
  ====================*/
int     CBitmapResource::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
    PROFILE("CBitmapResource::Load");

    // Dedicated servers don't need .bmp files so skip this and save some memory
    if (K2System.IsDedicatedServer() || K2System.IsServerManager())
        return false;
        
    int iResult(0);

    if (!m_sPath.empty())
        Console.Res << "Loading ^mBitmap^* " << SingleQuoteStr(m_sPath) << newl;
    else if (!m_sName.empty())
        Console.Res << "Loading ^mBitmap^* " << SingleQuoteStr(m_sName) << newl;
    else
        Console.Res << "Loading ^mUnknown Bitmap^*" << newl;

    assert(pData == NULL);

    if (!m_cBitmap.Load(m_sPath))
        iResult = RES_LOAD_FAILED;

    return iResult;
}


/*====================
  CBitmapResource::Free
  ====================*/
void    CBitmapResource::Free()
{
}
