// (C)2005 S2 Games
// c_freetype.h
//
//=============================================================================
#ifndef __C_FREETYPE_H__
#define __C_FREETYPE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_resource.h"
//=============================================================================

//=============================================================================
// IFreeTypeResource
//=============================================================================
class IFreeTypeResource : public IResource
{
protected:
    static FT_Library   s_FTLibrary;
    static bool         s_bInitialized;

    IFreeTypeResource();

public:
    virtual ~IFreeTypeResource();
    IFreeTypeResource(const tstring &sPath, const tstring &sName);

    static FT_Library   GetFreetypeLib();
};
//=============================================================================

#endif //__C_FREETYPE_H__
