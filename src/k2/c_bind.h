// (C)2005 S2 Games
// c_bind.h
//
//=============================================================================
#ifndef __C_BIND_H__
#define __C_BIND_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
#include "c_action.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CAction;

#ifdef CGAME
#define BIND_HOME BIND_CGAME
#elif SGAME
#define BIND_HOME BIND_SGAME
#else
#define BIND_HOME BIND_CORE
#endif

enum EBindFlags
{
	BIND_CORE			= BIT(0),
	BIND_CGAME			= BIT(1),
	BIND_SGAME			= BIT(2),

	BIND_PRIORITY		= BIT(3),
	BIND_SAVECONFIG		= BIT(4),
	BIND_NOREPEAT		= BIT(5),
	BIND_LOADCONFIG		= BIT(6)
};

const uint	BIND_CONFIG(BIND_SAVECONFIG | BIND_LOADCONFIG);
//=============================================================================

//=============================================================================
// CBind
//=============================================================================
class CBind
{
private:
	IBaseInput*		m_pAction;
	tstring			m_sParam;
	int				m_iFlags;
	tstring			m_sActionName;
	EActionType		m_eType;

public:
	CBind() :
	m_pAction(NULL),
	m_iFlags(0)
	{}

	CBind(IBaseInput *pAction, const tstring &sParam, int iFlags) :
	m_pAction(pAction),
	m_sParam(sParam),
	m_iFlags(iFlags),
	m_sActionName(pAction->GetName()),
	m_eType(pAction->GetType())
	{}

	CBind(const tstring &sAction, EActionType eType, const tstring &sParam, int iFlags) :
	m_pAction(NULL),
	m_sParam(sParam),
	m_iFlags(iFlags),
	m_sActionName(sAction),
	m_eType(eType)
	{}

	void	DoAction(float fValue, float fDelta, const CVec2f &v2Cursor)
	{
		if (m_pAction)
			m_pAction->Do(fValue, fDelta, v2Cursor, m_sParam);
	}

	IBaseInput*		GetAction() const		{ return m_pAction;	}
	const tstring&	GetParam() const		{ return m_sParam;	}
	int				GetFlags() const		{ return m_iFlags;	}
	const tstring&	GetActionName() const	{ return m_sActionName; }
	EActionType		GetActionType() const	{ return m_eType; }

	void	SetAction(IBaseInput *pAction)	{ m_pAction = pAction; }

	bool	HasFlags(int iFlags) const	{ return (m_iFlags & iFlags) != 0; }
	bool	HasAllFlags(int iFlags) const	{ return (m_iFlags & iFlags) == iFlags; }
};
//=============================================================================
#endif //__C_BIND_H__
