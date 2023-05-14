// (C)2005 S2 Games
// c_xmlproc_fontface.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlproc_resourcelist.h"
#include "c_xmlproc_interface.h"
#include "c_fontface.h"
#include "c_fontmap.h"
#include "i_resourcelibrary.h"
#include "c_resourcemanager.h"
//=============================================================================

// <fontface>
DECLARE_XML_PROCESSOR(fontface)
BEGIN_XML_REGISTRATION(fontface)
    REGISTER_XML_PROCESSOR(interface)
    REGISTER_XML_PROCESSOR_EX(XMLResourceList, resourcelist)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(fontface, void)
    // Get properties
    const tstring &sPath(node.GetProperty(_T("file")));

    // Load the face
    ResHandle hFace(g_ResourceManager.Register(sPath, RES_FONTFACE));
END_XML_PROCESSOR(g_ResourceManager.GetFontFace(hFace))


// <fontmap>
DECLARE_XML_PROCESSOR(fontmap)
BEGIN_XML_REGISTRATION(fontmap)
    REGISTER_XML_PROCESSOR(interface)
    REGISTER_XML_PROCESSOR_EX(XMLResourceList, resourcelist)
    REGISTER_XML_PROCESSOR(fontface)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(fontmap, void)
    try
    {
        // Get propterties
        const tstring &sPath(node.GetProperty(_T("file")));
        const tstring &sName(node.GetProperty(_T("name")));
        const tstring &sStyle(node.GetProperty(_T("style")));
        int iSize(node.GetPropertyInt(_T("size")));
        bool bDynamicResize(node.GetPropertyBool(_T("dynamic_fontsize")));
        int iBaseResolution(node.GetPropertyInt(_T("baseresolution"), 1024));
        const tstring &sAxis(node.GetProperty(_T("axis")));

        int iStyle(0);
        if (sStyle == _T("italic"))
            iStyle |= FONT_STYLE_ITALIC;
        if (sStyle == _T("bold"))
            iStyle |= FONT_STYLE_BOLD;

        uint uiCharacters(0);
        tsvector vCharacterSets(TokenizeString(node.GetProperty(_T("language")), _T(',')));
        if (vCharacterSets.empty())
            uiCharacters = BIT(FONT_RANGE_LATIN);
        for (tsvector_it it(vCharacterSets.begin()); it != vCharacterSets.end(); ++it)
        {
            if (TStringCompare(*it, _T("latin")) == 0)
                uiCharacters |= BIT(FONT_RANGE_LATIN);
            else if (TStringCompare(*it, _T("greek")) == 0)
                uiCharacters |= BIT(FONT_RANGE_GREEK);
            else if (TStringCompare(*it, _T("cyrillic")) == 0)
                uiCharacters |= BIT(FONT_RANGE_CYRILLIC);
            else if (TStringCompare(*it, _T("armenian")) == 0)
                uiCharacters |= BIT(FONT_RANGE_ARMENIAN);
            else if (TStringCompare(*it, _T("hebrew")) == 0)
                uiCharacters |= BIT(FONT_RANGE_HEBREW);
            else if (TStringCompare(*it, _T("arabic")) == 0)
                uiCharacters |= BIT(FONT_RANGE_ARABIC);
            else if (TStringCompare(*it, _T("devangari")) == 0)
                uiCharacters |= BIT(FONT_RANGE_DEVANGARI);
            else if (TStringCompare(*it, _T("bengali")) == 0)
                uiCharacters |= BIT(FONT_RANGE_BENGALI);
            else if (TStringCompare(*it, _T("gurmukhi")) == 0)
                uiCharacters |= BIT(FONT_RANGE_GURMUKHI);
            else if (TStringCompare(*it, _T("gujarati")) == 0)
                uiCharacters |= BIT(FONT_RANGE_GUJARATI);
            else if (TStringCompare(*it, _T("thai")) == 0)
                uiCharacters |= BIT(FONT_RANGE_THAI);
            else if (TStringCompare(*it, _T("hangul_jamo")) == 0)
                uiCharacters |= BIT(FONT_RANGE_HANGUL_JAMO);
            else if (TStringCompare(*it, _T("tagalog")) == 0)
                uiCharacters |= BIT(FONT_RANGE_TAGALOG);
            else if (TStringCompare(*it, _T("mongolian")) == 0)
                uiCharacters |= BIT(FONT_RANGE_MONGOLIAN);
            else if (TStringCompare(*it, _T("latin_addition")) == 0)
                uiCharacters |= BIT(FONT_RANGE_LATIN_ADDITION);
            else if (TStringCompare(*it, _T("hiragana")) == 0)
                uiCharacters |= BIT(FONT_RANGE_HIRAGANA);
            else if (TStringCompare(*it, _T("katakana")) == 0)
                uiCharacters |= BIT(FONT_RANGE_KATAKANA);
            else if (TStringCompare(*it, _T("cjk")) == 0)
                uiCharacters |= BIT(FONT_RANGE_CJK);
            else if (TStringCompare(*it, _T("hangul_syllable")) == 0)
                uiCharacters |= BIT(FONT_RANGE_HANGUL_SYLLABLE);
        }

        // Get the face
        ResHandle hParentFace(INVALID_RESOURCE);
        CFontFace *pFontFace(static_cast<CFontFace*>(pVoid));
        if (pFontFace != nullptr)
            hParentFace = pFontFace->GetHandle();

        if (pFontFace == nullptr || hParentFace == INVALID_RESOURCE)
        {
            if (sPath.empty())
                EX_ERROR(_T("fontmap does not reference any face"));

            hParentFace = g_ResourceManager.Register(sPath, RES_FONTFACE);
            pFontFace = g_ResourceManager.GetFontFace(hParentFace);
            if (pFontFace == nullptr || hParentFace == INVALID_RESOURCE)
                EX_ERROR(_T("No entry for current fontface handle"));
        }

        // Generate and register the fontmap
        IResource *pFontMap(K2_NEW(ctx_Resources,  CFontMap)(sName, iSize, iStyle, uiCharacters, hParentFace, bDynamicResize, iBaseResolution, sAxis));
        ResHandle hNewFontMap(g_ResourceManager.Register(pFontMap, RES_FONTMAP));

        // Register this map as a dependant of the face
        pFontFace->AddDependent(hNewFontMap);
    }
    catch (CException &ex)
    {
        ex.Process(_T("<fontmap> - "), NO_THROW);
        return false;
    }
END_XML_PROCESSOR_NO_CHILDREN
