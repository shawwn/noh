//=============================================================================
// Definitions
//=============================================================================
#ifndef __C_HOSTINTERFACE_H__
#define __C_HOSTINTERFACE_H__

class CHostInterface;
class IWidget;
class CInterface;

struct SGameInterfaceLib
{
    tstring sName;
    int     iMajorVersion;
    int     iMinorVersion;

    bool    (*Initialize)(CHostInterface *pHostInterface);
    //void  (*Frame)(); ?
    IWidget*    (*AllocateWidgetExt)(const tstring &sName, CInterface *pInterface, IWidget *pParent, const CWidgetStyle &style);
    void    (*Shutdown)();

    SGameInterfaceLib() :
    iMajorVersion(0),
    iMinorVersion(0),
    Initialize(NULL),
    //Frame(NULL),
    Shutdown(NULL)
    {
    }
};

typedef void    (FnInitInterfaceLib)(SGameInterfaceLib&);
typedef vector<IWidget *> WidgetVector;
typedef map<string, IWidget *> ChannelToWidgetMap;

//=============================================================================

//=============================================================================
// CHostServer
//=============================================================================
class CHostInterface
{
private:
  void*         m_pGameInterface;
  SGameInterfaceLib m_InterfaceLib;

public:
    K2_API ~CHostInterface();
    K2_API CHostInterface();

    K2_API bool Initialize();
    K2_API IWidget* AllocateWidgetExt(const tstring &sName, CInterface *pInterface, IWidget *pParent, const CWidgetStyle &style);
};

#endif //__C_HOSTINTERFACE_H__
