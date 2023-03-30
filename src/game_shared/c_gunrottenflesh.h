// (C)2007 S2 Games
// c_gunrottenflesh.h
//
//=============================================================================
#ifndef __C_GUNROTTENFLESH_H__
#define __C_GUNROTTENFLESH_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gunitem.h"
//=============================================================================

//=============================================================================
// CGunLocusts
//=============================================================================
class CGunRottenFlesh : public IGunItem
{
private:
	START_ENTITY_CONFIG(IGunItem)
		DECLARE_ENTITY_CVAR(uint, AmmoPerCorpse)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(Gun, RottenFlesh);

public:
	~CGunRottenFlesh()	{}
	CGunRottenFlesh() :
	IGunItem(GetEntityConfig()),
	m_pEntityConfig(GetEntityConfig())
	{}

	void		ConsumeCorpse();
};
//=============================================================================

#endif // __C_GUNROTTENFLESH_H__

