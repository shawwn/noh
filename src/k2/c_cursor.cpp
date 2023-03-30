// (C)2009 S2 Games
// c_cursor.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_cursor.h"

#include "i_resourcelibrary.h"
#include "c_xmlmanager.h"
#include "c_bitmap.h"
#include "c_bitmapresource.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
IResource*  AllocCursor(const tstring &sPath)
{
    return K2_NEW(ctx_Resources,  CCursor)(sPath);
}

IResourceLibrary    g_ResLibCursor(RES_K2CURSOR, _T("Cursor"), CCursor::ResTypeName(), true, AllocCursor);
//=============================================================================

/*====================
  CCursor::Load
  ====================*/
int     CCursor::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
    PROFILE("CCursor::Load");
    
    // Dedicated servers don't need .cursor files so skip this and save some memory
    if (K2System.IsDedicatedServer() || K2System.IsServerManager())
        return false;

    Console.Res << _T("Loading ^079Cursor^*: ") << m_sPath << newl;

    if (pData == NULL)
    {
        // Process the XML
        if (!XMLManager.Process(m_sPath, _T("cursor"), this))
        {
            Console.Warn << _T("CCursor::Load(") + m_sPath + _T(") - couldn't read XML") << newl;
            return RES_LOAD_FAILED;
        }
    }
    else
    {
        // Process the XML
        if (!XMLManager.ReadBuffer(pData, uiSize, _T("cursor"), this))
        {
            Console.Warn << _T("CCursor::Load(") + m_sPath + _T(") - couldn't read XML") << newl;
            return RES_LOAD_FAILED;
        }
    }

    return 0;
}


/*====================
  CCursor::GetBitmapPointer
  ====================*/
CBitmap*    CCursor::GetBitmapPointer()
{
    CBitmapResource *pBitmapResource(g_ResourceManager.GetBitmapResource(m_hBitmap));
    if (pBitmapResource != NULL)
        return &pBitmapResource->GetBitmap();
    else
        return NULL;
}
