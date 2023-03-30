// (C)2008 S2 Games
// c_proptree.h
//
//=============================================================================
#ifndef __C_PROPTREE_H__
#define __C_PROPTREE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_bitentity.h"
//=============================================================================

//=============================================================================
// CPropTree
//=============================================================================
class CPropTree : public IBitEntity
{
private:
	START_ENTITY_CONFIG(IBitEntity)
		DECLARE_ENTITY_CVAR(tstring, DisplayName)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(Prop, Tree);

	uint	m_uiBlockerIndex;

public:
	~CPropTree()	{}
	CPropTree();

	virtual void	Spawn();

	virtual void	Activate();
	virtual void	Deactivate();

	virtual bool	IsVisibleOnMap(CPlayer *pLocalPlayer) const					{ return false; }
	virtual void	DrawOnMap(class CUITrigger &minimap, CPlayer *pLocalPlayer) {}

	virtual bool	IsTargetType(const CTargetScheme::STestRecord &test, const IUnitEntity *pInitiator) const;

	virtual CVec3f	GetApproachPosition(const CVec3f &v3Start, const CBBoxf &bbBounds);

	ENTITY_CVAR_ACCESSOR(const tstring&, DisplayName)
};
//=============================================================================

#endif //__C_PROPTREE_H__
