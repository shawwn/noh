// (C)2008 S2 Games
// c_entitylanenode.h
//
//=============================================================================
#ifndef __C_ENTITYLANENODE_H__
#define __C_ENTITYLANENODE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CEntityLaneNode
//=============================================================================
class CEntityLaneNode : public IVisualEntity
{
protected:
	DECLARE_ENT_ALLOCATOR2(Entity, LaneNode);

	tstring		m_sTarget;

public:
	~CEntityLaneNode()	{}
	CEntityLaneNode()	{}

	virtual bool		IsServerEntity() const			{ return true; }

	void	ApplyWorldEntity(const CWorldEntity &ent);

	void			SetTarget(const tstring &sTarget)	{ m_sTarget = sTarget; }
	const tstring&	GetTarget() const					{ return m_sTarget; }
};
//=============================================================================

#endif //__C_ENTITYLANENODE_H__
