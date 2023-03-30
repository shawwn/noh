// (C)2005 S2 Games
// c_uicmd.h
//
//=============================================================================
#ifndef __C_UICMD_H__
#define __C_UICMD_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_uiscripttoken.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IWidget;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef tstring(*UICmdFn_t)(IWidget *pThis, const ScriptTokenVector &vArgList);

enum EUICmdFlags
{
    UICMD_DEV =         0x0001
};

// Declaration macros
#define UI_CMD(name, min_args) \
tstring ui##name##Fn(IWidget *pThis, const ScriptTokenVector &vArgList); \
CUICmd  ui##name(_T(#name), ui##name##Fn, min_args, 0); \
tstring ui##name##Fn(IWidget *pThis, const ScriptTokenVector &vArgList)

#define UI_VOID_CMD(name, min_args) \
void    ui##name##VoidFn(IWidget *pThis, const ScriptTokenVector &vArgList); \
tstring ui##name##Fn(IWidget *pThis, const ScriptTokenVector &vArgList) \
{ \
    ui##name##VoidFn(pThis, vArgList); \
    return TSNULL; \
} \
CUICmd  ui##name(_T(#name), ui##name##Fn, min_args, 0); \
void    ui##name##VoidFn(IWidget *pThis, const ScriptTokenVector &vArgList) \

#define UI_CMD_EX(name, min_args, flags) \
tstring ui##name##Fn(IWidget *pThis, const ScriptTokenVector &vArgList); \
CUICmd  ui##name(_T(#name), ui##name##Fn, flags); \
tstring ui##name##Fn(IWidget *pThis, const ScriptTokenVector &vArgList)
//=============================================================================

//=============================================================================
// CUICmd
//=============================================================================
class K2_API CUICmd
{
private:
    tstring         m_sName;
    UICmdFn_t       m_pfnUICmd;
    int             m_iMinArgs;
    int             m_iFlags;

    // UICmds should not be copied
    CUICmd(CUICmd&);
    CUICmd& operator=(CUICmd&);

public:
    CUICmd(const tstring &sName, UICmdFn_t pfnUICmd, int iMinArgs, int iFlags);
    ~CUICmd();

    const tstring&  GetName()           { return m_sName; }
    int             GetFlags()          { return m_iFlags; }

    tstring Execute(IWidget *pThis, const ScriptTokenVector &vArgList);
};
//=============================================================================
#endif //__C_UICMD_H__
