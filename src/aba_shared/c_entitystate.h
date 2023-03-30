// (C)2007 S2 Games
// c_entitystate.h
//
//=============================================================================
#ifndef __C_ENTITYSTATE_H__
#define __C_ENTITYSTATE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef EntityStateScript	vector<class CEntityStateInstruction>;
//=============================================================================

//=============================================================================
// CEntityStateInstruction
//=============================================================================
class CEntityStateInstruction
{
private:
	tstring		m_sName;

public:
	~CEntityStateInstruction()	{}
	CEntityStateInstruction()	{}
};
//=============================================================================

//=============================================================================
// CEntityStateDefinition
//=============================================================================
class CEntityStateDefinition
{
private:
	tstring				m_sName;

	EntityStateScript	m_scriptStart;
	EntityStateScript	m_scriptFrame;
	EntityStateScript	m_scriptEnd;

public:
	~CEntityStateDefinition()	{}
	CEntityStateDefinition()	{}
};
//=============================================================================

//=============================================================================
// CEntityState
//=============================================================================
class CEntityState : public IGameEntity
{
private:
	uint	m_uiTarget;
	uint	m_uiOwner;
	uint	m_uiCreationTime;
	uint	m_uiDuration;

	CEntityStateDefinition*	m_pDefinition;

public:
	~CEntityState()	{}
	CEntityState()	{}

	void	Spawn()	{}
	bool	ServerFrame()	{ return true; }
};
//=============================================================================

#endif //__C_ENTITYSTATE_H__
