// (C)2007 S2 Games
// c_asCasting.h
//
//=============================================================================
#ifndef __C_ASCASTING_H__
#define __C_ASCASTING_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_ActionState.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IEntityTool;

const uint ASR_CHANNELING		(BIT(5));
//=============================================================================

//=============================================================================
// CASCasting
//=============================================================================
class CASCasting : public IActionState
{
private:
	uint			m_uiBeginToolUID;
	uint			m_uiBeginTargetIndex;
	CVec3f			m_v3BeginTargetPosition;
	CVec3f			m_v3BeginTargetDelta;
	bool			m_bBeginSecondary;
	int				m_iBeginIssuedClientNumber;

	uint			m_uiToolUID;
	uint			m_uiTargetIndex;
	CVec3f			m_v3TargetPosition;
	CVec3f			m_v3TargetDelta;
	bool			m_bSecondary;
	int				m_iIssuedClientNumber;

	uint			m_uiInitTime;
	
	void			Reset()
	{
		ClearAllFlags();
		m_uiInitTime = 0;
	}

	IEntityTool*	GetBeginTool() const
	{
		IGameEntity *pEntity(Game.GetEntityFromUniqueID(m_uiBeginToolUID));
		return pEntity ? pEntity->GetAsTool() : NULL;
	}

	IEntityTool*	GetTool() const
	{
		IGameEntity *pEntity(Game.GetEntityFromUniqueID(m_uiToolUID));
		return pEntity ? pEntity->GetAsTool() : NULL;
	}

public:
	CASCasting(CBrain &cParent);

	void			CopyFrom(const CASCasting *pActionState);

	// Core Operations
	virtual bool	BeginState();
	virtual bool	ContinueStateMovement();
	virtual bool	ContinueStateAction();
	virtual bool	ContinueStateCleanup() { return true; }
	virtual bool	EndState(uint uiPriority);

	// Begin Parameters
	void			SetBeginToolUID(uint uiUID)						{ m_uiBeginToolUID = uiUID; }
	void			SetBeginTargetIndex(uint uiIndex)				{ m_uiBeginTargetIndex = uiIndex; }
	void			SetBeginTargetPosition(const CVec3f &v3Pos)		{ m_v3BeginTargetPosition = v3Pos; }
	void			SetBeginTargetDelta(const CVec3f &v3Delta)		{ m_v3BeginTargetDelta = v3Delta; }
	void			SetBeginSecondary(bool bSecondary)				{ m_bBeginSecondary = bSecondary; }
	void			SetBeginIssuedClientNumber(int iClientNumber)	{ m_iBeginIssuedClientNumber = iClientNumber; }

	// Actual Paramers
	void			SetToolUID(uint uiUID)							{ m_uiToolUID = uiUID; }
	void			SetTargetIndex(uint uiIndex)					{ m_uiTargetIndex = uiIndex; }
	void			SetTargetPosition(const CVec3f &v3Pos)			{ m_v3TargetPosition = v3Pos; }
	void			SetTargetDelta(const CVec3f &v3Delta)			{ m_v3TargetDelta = v3Delta; }
	void			SetSecondary(bool bSecondary)					{ m_bSecondary = bSecondary; }
	void			SetIssuedClientNumber(int iClientNumber)		{ m_iIssuedClientNumber = iClientNumber; }
};
//=============================================================================

#endif //__C_ASCASTING_H__