// (C)2005 S2 Games
// i_baseinput.h
//
//=============================================================================
#ifndef __I_BASEINPUT_H__
#define __I_BASEINPUT_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifdef CGAME
#define ACTION_HOME ACTION_CGAME
#elif SGAME
#define ACTION_HOME ACTION_SGAME
#else
#define ACTION_HOME ACTION_CORE
#endif

enum EActionType
{
    AT_INVALID = -1,
    AT_BUTTON = 0,
    AT_AXIS,
    AT_IMPULSE
};

enum EActionFlags
{
    ACTION_CORE         = BIT(0),
    ACTION_CGAME        = BIT(1),
    ACTION_SGAME        = BIT(2),
    ACTION_NOREPEAT     = BIT(3)
};

//=============================================================================
// IBaseInput
//=============================================================================
class K2_API IBaseInput
{
protected:
    tstring         m_sName;
    int             m_iFlags;
    EActionType     m_eType;

    // Actions should not be copied
    IBaseInput(IBaseInput&);
    IBaseInput& operator=(IBaseInput&);

public:
    IBaseInput(const tstring &sName, EActionType eType, int iFlags);
    virtual ~IBaseInput();

    const tstring&  GetName()           { return m_sName; }
    int             GetFlags()          { return m_iFlags; }
    EActionType     GetType()           { return m_eType; }

    virtual void    Do(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam) = 0;
    virtual void    operator()(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam) = 0;
};
//=============================================================================

#endif //__I_BASEINPUT_H__
