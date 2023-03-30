// (C)2010 S2 Games
// i_entitystatenetaccum.h
//
//=============================================================================
#ifndef __I_ENTITYSTATENETACCUM_H__
#define __I_ENTITYSTATENETACCUM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
#include "c_statenetaccumdefinition.h"
//=============================================================================

//=============================================================================
// IEntityStateNetAccum
//=============================================================================
class IEntityStateNetAccum : public IEntityState
{
	DECLARE_ENTITY_DESC

public:
	typedef CStateNetAccumDefinition TDefinition;

protected:

public:
	virtual ~IEntityStateNetAccum()	{}
	IEntityStateNetAccum();

	SUB_ENTITY_ACCESSOR(IEntityStateNetAccum, StateNetAccum)

	// Network
	GAME_SHARED_API virtual void	Baseline();
	GAME_SHARED_API virtual void	GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	GAME_SHARED_API virtual bool	ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);
};
//=============================================================================

#endif //__I_ENTITYABILITYATTRIBUTE_H__
