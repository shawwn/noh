// (C)2005 S2 Games
// c_uicmdregistry.h
//
//=============================================================================
#ifndef __C_UICMDREGISTRY_H__
#define __C_UICMDREGISTRY_H__

//=============================================================================
// Declarations
//=============================================================================
class CUICmd;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef map<tstring, CUICmd*>           UICmdMap;
//=============================================================================

//=============================================================================
// CUICmdRegistry
//=============================================================================
class CUICmdRegistry
{
private:
    static CUICmdRegistry   *s_pInstance;
    static bool             s_bRequested, s_bReleased;

    CUICmdRegistry() {}
    CUICmdRegistry(CUICmdRegistry&);
    CUICmdRegistry& operator=(CUICmdRegistry&);

    UICmdMap        m_mapUICmds;

public:
    static CUICmdRegistry*  GetInstance();
    static void             Release();
    static bool             IsReleased()    { return s_bReleased; }

    void                    Register(CUICmd *pUICmd);
    void                    Unregister(const tstring &sName);

    K2_API inline CUICmd*   GetUICmd(const tstring &sUICmd);

    const UICmdMap&         GetUICmdMap()   { return m_mapUICmds; }

    K2_API inline bool      Exists(const tstring &sUICmd);
};


/*====================
  CUICmdRegistry::Exists
  ====================*/
bool    CUICmdRegistry::Exists(const tstring &sUICmd)
{
    UICmdMap::iterator find = m_mapUICmds.find(LowerString(sUICmd));

    if (find == m_mapUICmds.end())
        return false;

    return true;
}


/*====================
  CUICmdRegistry::GetUICmd
  ====================*/
CUICmd  *CUICmdRegistry::GetUICmd(const tstring &sUICmd)
{
    UICmdMap::iterator find = m_mapUICmds.find(LowerString(sUICmd));

    if (find == m_mapUICmds.end())
        return NULL;
    else
        return find->second;
}

extern K2_API CUICmdRegistry *pUICmdRegistry;
//=============================================================================
#endif //__C_UICMDREGISTRY_H__
