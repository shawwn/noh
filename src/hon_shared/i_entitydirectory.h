// (C)2006 S2 Games
// i_entitydirectory.h
//
//=============================================================================
#ifndef __I_ENTITYDIRECTORY_H__
#define __I_ENTITYDIRECTORY_H__

//=============================================================================
// Declarations
//=============================================================================
class IGameEntity;
class IVisualEntity;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef map<uint, IGameEntity*>		EntMap;
typedef EntMap::iterator			EntMap_it;

typedef list<IGameEntity*>			EntList;
typedef EntList::iterator			EntList_it;

typedef list<IUnitEntity *>			UnitList;
typedef UnitList::iterator			UnitList_it;
typedef UnitList::const_iterator	UnitList_cit;
//=============================================================================

//=============================================================================
// IEntityDirectory
//=============================================================================
class GAME_SHARED_API IEntityDirectory
{
protected:

public:
	virtual ~IEntityDirectory();
	IEntityDirectory();

	void					Initialize();
	virtual void			Clear() = 0;

	virtual IGameEntity*	GetEntity(uint uiIndex) = 0;
	virtual IGameEntity*	GetEntityFromUniqueID(uint uiUniqueID)								{ return NULL; }
	virtual uint			GetGameIndexFromUniqueID(uint uiUniqueID)							{ return INVALID_INDEX; }
	virtual IGameEntity*	GetFirstEntity() = 0;
	virtual IGameEntity*	GetNextEntity(IGameEntity *pEntity) = 0;
	virtual IGameEntity*	Allocate(ushort unType, uint uiMinIndex = INVALID_INDEX)			{ return NULL; }
	virtual IGameEntity*	Allocate(const tstring &sName, uint uiMinIndex = INVALID_INDEX)		{ return NULL; }
	virtual IVisualEntity*	GetEntityFromName(const tstring &sName)								{ return NULL; }
	virtual IVisualEntity*	GetNextEntityFromName(IVisualEntity *pEntity)						{ return NULL; }
	virtual const UnitList&	GetUnitList() = 0;
	virtual void			GetEntities(uivector &vResult, ushort unType) = 0;
	virtual void			ActivateBitEntity(uint uiIndex)										{}
	virtual void			DeactivateBitEntity(uint uiIndex)									{}
	virtual void			UpdateDefinitions(ushort unType)									{}
};
//=============================================================================

#endif //__I_ENTITYDIRECTORY_H__
