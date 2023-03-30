// (C)2008 S2 Games
// c_entitydefinitionresource.h
//
//=============================================================================
#ifndef __C_ENTITYDEFINITIONRESOURCE_H__
#define __C_ENTITYDEFINITIONRESOURCE_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/i_resource.h"
#include "i_entitydefinition.h"
//=============================================================================

//=============================================================================
// CEntityDefinitionResource
//=============================================================================
class CEntityDefinitionResource : public IResource
{
protected:
	IEntityDefinition*	m_pDefinition;
	ushort				m_unTypeID;

	CEntityDefinitionResource();

public:
	~CEntityDefinitionResource()	{}
	CEntityDefinitionResource(const tstring &sPath) :
	IResource(sPath, TSNULL),
	m_pDefinition(NULL),
	m_unTypeID(INVALID_ENT_TYPE)
	{}

	GAME_SHARED_API	virtual uint			GetResType() const			{ return RES_ENTITY_DEF; }
	GAME_SHARED_API	virtual const tstring&	GetResTypeName() const		{ return ResTypeName(); }
	GAME_SHARED_API	static const tstring&	ResTypeName()				{ static tstring sTypeName(_T("{entity}")); return sTypeName; }

	ushort		GetTypeID() const										{ return m_unTypeID; }

	template <class T>
	T*	GetDefinition() const
	{
		if (m_pDefinition == NULL)
			return NULL;
		return static_cast<T*>(m_pDefinition);
	}
	
	template <class T>
	T*	GetDefinition(ushort unModifierBits) const
	{
		if (m_pDefinition == NULL)
			return NULL;

		IEntityDefinition *pModifiedDef(m_pDefinition->GetModifiedDefinition(unModifierBits));
		if (pModifiedDef != NULL)
			return static_cast<T*>(pModifiedDef);

		return static_cast<T*>(m_pDefinition);
	}
	
	void	SetDefinition(IEntityDefinition *pDefinition)	{ m_pDefinition = pDefinition; }		
	int		Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
	void	Free();
	void	PostLoad();

	void	Precache(EPrecacheScheme eScheme, const tstring &sModifier)
	{
		if (m_pDefinition != NULL)
			m_pDefinition->Precache(eScheme, sModifier);
	}

	void	PostProcess()
	{
		if (m_pDefinition != NULL)
			m_pDefinition->PostProcess();
	}

	void	Reloaded();
};
//=============================================================================

#endif //__C_ENTITYDEFINITIONRESOURCE_H__
