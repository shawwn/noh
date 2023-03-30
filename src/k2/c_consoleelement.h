// (C)2005 S2 Games
// c_consoleelement.h
//
//=============================================================================
#ifndef __C_CONSOLEELEMENT_H__
#define __C_CONSOLEELEMENT_H__

//=============================================================================
// Definitions
//=============================================================================
typedef bool (*ConsoleElementFn_t)(class CConsoleElement *pElem, const tsvector &vArgList);

#ifdef CGAME
#define CONEL_HOME CONEL_CGAME
#elif SGAME
#define CONEL_HOME CONEL_SGAME
#else
#define CONEL_HOME CONEL_CORE
#endif

enum EConsoleElementFlag
{
    CONEL_DEV           = BIT(0),
    CONEL_EXEC          = BIT(1),
    CONEL_DYNAMIC       = BIT(2),   // a dynamically allocated console element
    CONEL_CORE          = BIT(3),   // defined in core
    CONEL_CGAME         = BIT(4),   // defined in cgame.dll
    CONEL_SGAME         = BIT(5),   // defined in sgame.dll

    // Command specific
    CMD_DISABLED        = BIT(6),

    // Cvar specific
    CVAR_READONLY       = BIT(7),
    CVAR_SAVECONFIG     = BIT(8),
    CVAR_DONTSAVE       = BIT(9),   //don't save a var out when saving vars that match the name
    CVAR_VALUERANGE     = BIT(10),  //use the lorange and hirange fields of the cvar struct to constrain the variable
    CVAR_TRANSMIT       = BIT(11),  //the server will transmit this variable to the client when it is changed or on a connect
    CVAR_SERVERINFO     = BIT(12),  //the server will change its STATE_STRING_SERVER_INFO string whenenver this cvar changes
    CVAR_WORLDCONFIG    = BIT(13),
    CVAR_GAMECONFIG     = BIT(14),  // Used for tweaking gameplay on the fly, should be hidden on release
    CVAR_UNUSED         = BIT(15),  // Don't even bother registering it (for tunable values that aren't relevant to a particular object)
    CVAR_CHILD          = BIT(16),

    // Alias specific
    ALIAS_SAVECONFIG    = BIT(18),
};

enum EElementType
{
    ELEMENT_OTHER = 0,
    ELEMENT_CMD,
    ELEMENT_CVAR,
    ELEMENT_ALIAS,
    ELEMENT_FUNCTION,
    ELEMENT_CMDPRECACHE,
    ELEMENT_GAMEBIND
};
//=============================================================================

//=============================================================================
// CConsoleElement
//=============================================================================
class K2_API CConsoleElement
{
protected:
    tstring         m_sName;
    int             m_iFlags;
    EElementType    m_eType;

    ConsoleElementFn_t  m_pfnCmd;
    ConsoleElementFn_t  m_pfnPrecacheCmd;

public:
    CConsoleElement(const tstring &sName, int iFlags, EElementType eType, ConsoleElementFn_t pfnCmd);
    virtual ~CConsoleElement();

    const tstring&  GetName() const     { return m_sName; }
    EElementType    GetType() const     { return m_eType; }

    virtual void            Execute(const tsvector &vArgList)   { if (m_pfnCmd == NULL) return; m_pfnCmd(this, vArgList); }
    virtual tstring         Evaluate(const tsvector &vArgList)  { if (m_pfnCmd == NULL) return _T(""); m_pfnCmd(this, vArgList); return _T(""); }
    virtual bool            Precache(const tsvector &vArgList)  { if (m_pfnPrecacheCmd) return m_pfnPrecacheCmd(this, vArgList); else return false; }

    virtual tstring         GetString() const   = 0;
    virtual void            Set(const tstring &s)   {}

    void    AddPrecacheCommand(ConsoleElementFn_t pfnCmd)       { m_pfnPrecacheCmd = pfnCmd; }

    int     GetFlags() const            { return m_iFlags; }
    void    SetFlags(int iFlags)        { m_iFlags = iFlags; }

    bool    HasFlags(int iFlags) const  { return (m_iFlags & iFlags) != 0; }
    bool    HasAllFlags(int iFlags) const   { return (m_iFlags & iFlags) == iFlags; }
    void    AddFlags(int iFlags)        { m_iFlags |= iFlags; }
    void    RemoveFlags(int iFlags)     { m_iFlags &= ~iFlags; }
};
//=============================================================================
#endif
