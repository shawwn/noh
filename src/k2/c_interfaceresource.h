// (C)2005 S2 Games
// c_interfaceresource.h
//
//=============================================================================
#ifndef __C_INTERFACERESOURCE_H__
#define __C_INTERFACERESOURCE_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/i_resource.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CInterface;
class CWidgetStyle;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CInterfaceResource
//=============================================================================
class CInterfaceResource : public IResource
{
private:
    CInterface* m_pInterface;

public:
    K2_API ~CInterfaceResource()    {}
    K2_API CInterfaceResource(const tstring &sPath);

    K2_API  virtual uint            GetResType() const          { return RES_INTERFACE; }
    K2_API  virtual const tstring&  GetResTypeName() const      { return ResTypeName(); }
    K2_API  static const tstring&   ResTypeName()               { static tstring sTypeName(_T("{interface}")); return sTypeName; }

    CInterface*     GetInterface()  { return m_pInterface; }

    bool    Allocate(const CWidgetStyle& style);

    int     Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
    void    Free();

    void    Reloaded();
};
//=============================================================================

#endif //__C_INTERFACERESOURCE_H__
