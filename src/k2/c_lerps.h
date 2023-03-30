// (C)2010 S2 Games
// c_Lerps.h
//
//=============================================================================
#ifndef __C_LERPS_H__
#define __C_LERPS_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_uitrigger.h"
#include "c_uitriggerregistry.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum ELERPTYPE
{
	LERPTYPE_LINEAR,
	LERPTYPE_HEAVYSTART,
	LERPTYPE_HEAVYEND,
	LERPTYPE_HEAVYMID,
	LERPTYPE_HEAVYSTARTEND,
	NUM_LERPTYPES
};

enum ELerpStyle
{
	LERPSTYLE_NONE,
	LERPSTYLE_MINUS_ONLY,
	LERPSTYLE_PLUS_ONLY,

	NUM_LERPSTYLES
};
//=============================================================================

//=============================================================================
// CLerpFloat
//=============================================================================
class CLerpFloat
{

private:

	CUITrigger* pUITrigger;
	float		m_fTargetAmount;
	float		m_fStartAmount;
	uint		m_uiStartTime;
	uint		m_uiTargetTime;
	uint		m_uiLerpType;
	uint		m_uiLerpStyle;
	bool		m_bDone;
	tstring		m_sValOutName;
	tstring		m_sValStart;

public:
	
	~CLerpFloat() {}
	CLerpFloat(const tstring &sValOut, float fTargetAmount, uint uiTargetTime, uint uiType, int iStyle);

	float		m_fValOut;

	bool		IsDone()			{ return m_bDone; }
	void 		Update();
	void 		Reset(float fNewTargetAmount, uint uiNewTargetTime, uint uiNewType, int iNewStyle);

};
//=============================================================================


//=============================================================================
// CSimpleLerp
//=============================================================================
class K2_API CSimpleLerp
{
private:
	float		m_fTargetAmount;
	float		m_fStartAmount;
	uint		m_uiStartTime;
	uint		m_uiTargetTime;
	ELerpStyle	m_eLerpStyle;
	uint		m_uiDelayedStart;
	bool		m_bDone;
	float		m_fValOut;

public:
	~CSimpleLerp() {}
	CSimpleLerp();
	CSimpleLerp(float fTargetAmount, uint uiTargetTime, ELerpStyle eStyle, uint uiDelayedStart);

	float		GetValue() const		{ return m_fValOut; }
	void		SetValue(float fValOut)	{ m_fValOut = fValOut; }
	float		GetTarget() const		{ return m_fTargetAmount; }
	bool		IsDone() const 			{ return m_bDone; }
	void 		Update();
	void 		Reset(float fNewTargetAmount, uint uiNewTargetTime, ELerpStyle eNewStyle, uint uiDelayedStart);
};
//=============================================================================

#endif // __C_LERPS_H__
