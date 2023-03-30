// (C)2006 S2 Games
// c_eventcmd.h
//
//=============================================================================
#ifndef __C_EVENTCMD_H__
#define __C_EVENTCMD_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef bool(*EventCmdFn_t)(const tsvector &vArgList, int iTimeNudge);

enum EEventCmdFlags
{
    EVENTCMD_DEV = 0x0001,
};

// Declaration macros
#ifdef K2_PROFILE // START K2_PROFILE
#define EVENT_CMD(name) \
bool    _event##name##Fn(const tsvector &vArgList, int iTimeNudge);\
bool    event##name##Fn(const tsvector &vArgList, int iTimeNudge);\
CEventCmd event##name(_T(#name), _event##name##Fn, 0);\
bool    _event##name##Fn(const tsvector &vArgList, int iTimeNudge)\
{\
    PROFILE(#name);\
    return event##name##Fn(vArgList, iTimeNudge);\
}\
bool    event##name##Fn(const tsvector &vArgList, int iTimeNudge)
#else   // NOT K2_PROFILE
#define EVENT_CMD(name) \
bool    event##name##Fn(const tsvector &vArgList, int iTimeNudge);\
CEventCmd event##name(_T(#name), event##name##Fn, 0);\
bool    event##name##Fn(const tsvector &vArgList, int iTimeNudge)
#endif  // END K2_PROFILE

#define EVENT_CMD_EX(name, flags) \
bool    event##name##Fn(const tsvector &vArgList, int iTimeNudge); \
CEventCmd  event##name(_T(#name), event##name##Fn, flags); \
bool    event##name##Fn(const tsvector &vArgList, int iTimeNudge)

#define EVENT_CMD_WRAPPER(name) \
EVENT_CMD(name) \
{\
    return cmd##name(vArgList);\
}
//=============================================================================

//=============================================================================
// CEventCmd
//=============================================================================
class CEventCmd
{
private:
    tstring         m_sName;
    EventCmdFn_t    m_pfnEventCmd;
    int             m_iFlags;

    // EventCmds should not be copied
    CEventCmd(CEventCmd&);
    CEventCmd& operator=(CEventCmd&);

public:
    K2_API CEventCmd(const tstring &sName, EventCmdFn_t pfnEventCmd, int iFlags = 0);
    K2_API ~CEventCmd();

    const tstring&  GetName()           { return m_sName; }
    int             GetFlags()          { return m_iFlags; }

    bool    Execute(const tsvector &vArgList, int iTimeNudge);
    bool    operator()(const tsvector &vArgList, int iTimeNudge);
};
//=============================================================================
#endif //__C_EVENTCMD_H__
