// (C)2005 S2 Games
// c_freetype.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_freetype.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
FT_Library  IFreeTypeResource::s_FTLibrary(NULL);
bool        IFreeTypeResource::s_bInitialized(false);
//=============================================================================

/*====================
  IFreeTypeResource::~IFreeTypeResource
  ====================*/
IFreeTypeResource::~IFreeTypeResource()
{
}


/*====================
  IFreeTypeResource::IFreeTypeResource
  ====================*/
IFreeTypeResource::IFreeTypeResource(const tstring &sPath, const tstring &sName) :
IResource(sPath, sName)
{
}


/*====================
  IFreeTypeResource::GetFreetypeLib
  ====================*/
FT_Library  IFreeTypeResource::GetFreetypeLib()
{
    try
    {
        if (s_bInitialized)
            return s_FTLibrary;

        if (FT_Init_FreeType(&s_FTLibrary) != 0)
            throw CException(_T("Failed initializing FreeType"), E_FATAL);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CFontMap::GetFreeTypeLib() - "));
    };

    return s_FTLibrary;
}
