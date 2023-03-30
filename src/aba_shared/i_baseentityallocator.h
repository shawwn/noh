// (C)2009 S2 Games
// i_baseentityallocator.h
//
//=============================================================================
#ifndef __I_BASEENTITYALLOCATOR_H__
#define __I_BASEENTITYALLOCATOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_entitysnapshot.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IGameEntity;
struct SEntityDesc;

enum EPrecacheScheme
{
	PRECACHE_ALL,
	PRECACHE_SELF,
	PRECACHE_ALLY,
	PRECACHE_OTHER
};
//=============================================================================

//=============================================================================
// IBaseEntityAllocator
//=============================================================================
class IBaseEntityAllocator
{
protected:
	tstring				m_sBaseTypeName;
	uint				m_uiBaseType;
	CEntitySnapshot*	m_pBaseline;

public:
	virtual ~IBaseEntityAllocator()	{}
	IBaseEntityAllocator(const tstring &sBaseTypeName, uint uiBaseType) :
	m_sBaseTypeName(sBaseTypeName),
	m_uiBaseType(uiBaseType)
	{}

	virtual IGameEntity*		Allocate() const = 0;
	virtual const TypeVector*	GetTypeVector() const = 0;
	virtual const SEntityDesc*	GetTypeDesc() const = 0;
	virtual uint				GetVersion() const = 0;
	virtual void				ClientPrecache(EPrecacheScheme eScheme) const = 0;
	virtual void				ServerPrecache(EPrecacheScheme eScheme) const = 0;
	virtual void				PostProcess() const = 0;

	const tstring&				GetBaseTypeName() const		{ return m_sBaseTypeName; }
	uint						GetBaseType() const			{ return m_uiBaseType; }
	const CEntitySnapshot*		GetBaseline() const			{ return m_pBaseline; }
};
//=============================================================================

#endif //__I_BASEENTITYALLOCATOR_H__
