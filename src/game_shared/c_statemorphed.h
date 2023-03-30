// (C)2007 S2 Games
// c_statemorphed.h
//
//=============================================================================
#ifndef __C_STATEMORPHED_H__
#define __C_STATEMORPHED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateMorphed
//=============================================================================
class CStateMorphed : public IEntityState
{
private:
	static vector<SDataField>	*s_pvFields;

	START_ENTITY_CONFIG(IEntityState)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Morphed)

public:
	~CStateMorphed()	{}
	CStateMorphed();

	bool	IsDisguised() const		{ return true; }

	// Network
	GAME_SHARED_API virtual void	Baseline();
	GAME_SHARED_API virtual void	GetSnapshot(CEntitySnapshot &snapshot) const;
	GAME_SHARED_API virtual bool	ReadSnapshot(CEntitySnapshot &snapshot);

	static const vector<SDataField>&	GetTypeVector();
};
//=============================================================================

#endif //__C_STATEMORPHED_H__
