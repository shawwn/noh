// (C)2005 S2 Games
// c_fontface.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_fontface.h"
#include "c_fontmap.h"
#include "i_resourcelibrary.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary    g_ResLibFontFace(RES_FONTFACE, _T("Font Faces"), CFontFace::ResTypeName(), true, AllocFontFace);
//=============================================================================

/*====================
  AllocFontFace
  ====================*/
IResource*  AllocFontFace(const tstring &sPath)
{
    return K2_NEW(ctx_Resources,  CFontFace)(sPath);
}


/*====================
  CFontFace::CFontFace
  ====================*/
CFontFace::CFontFace(const tstring &sPath) :
IFreeTypeResource(sPath, TSNULL),
m_FTFace(nullptr)
{
}


/*====================
  CFontFace::Load
  ====================*/
int     CFontFace::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
    PROFILE("CFontFace::Load");

    try
    {
        // Dedicated servers don't need font files so skip this and save some memory
        if (K2System.IsDedicatedServer() || K2System.IsServerManager())
            return false;

        if (!m_sPath.empty())
            Console.Res << "Loading FontFace " << SingleQuoteStr(m_sPath) << newl;
        else if (!m_sName.empty())
            Console.Res << "Loading FontFace " << SingleQuoteStr(m_sName) << newl;
        else
            Console.Res << "Loading Unknown FontFace" << newl;

        if (pData != nullptr)
        {
            SAFE_DELETE_ARRAY(m_pData);

            m_pData = K2_NEW_ARRAY(ctx_Resources, char, uiSize);
            MemManager.Copy((char *)m_pData, pData, uiSize);

            m_uiSize = uiSize;
        }

        // Read the face data
        FT_Error iErrCode(FT_New_Memory_Face(GetFreetypeLib(), (const FT_Byte*)m_pData, (FT_Long)m_uiSize, 0, &m_FTFace));
        if (iErrCode == FT_Err_Unknown_File_Format)
            EX_ERROR(_T("Invalid file format"));
        if (iErrCode != 0)
            EX_ERROR(_T("FT_New_Memory_Face() failed"));

        tstring sStyle;
        StrToTString(sStyle, m_FTFace->style_name);
        tstring sFamily;
        StrToTString(sFamily, m_FTFace->family_name);
        Console.Res << _T("family: ") << sFamily << _T(", style: ") << sStyle << _T(", glyphs: ") << m_FTFace->num_glyphs << newl;
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CFontFace::Load(") + m_sName + _TS(") - "), NO_THROW);
        return RES_LOAD_FAILED;
    }

    return 0;
}


/*====================
  CFontFace::Free
  ====================*/
void    CFontFace::Free()
{
    FT_Done_Face(m_FTFace);
}
