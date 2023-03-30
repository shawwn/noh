// (C)2009 S2 Games
// c_gamemechanicsource.h
//
//=============================================================================
#ifndef __C_GAMEMECHANICSRESOURCE_H__
#define __C_GAMEMECHANICSRESOURCE_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/i_resource.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CGameMechanics;
//=============================================================================

//=============================================================================
// CGameMechanicsResource
//=============================================================================
class CGameMechanicsResource : public IResource
{
protected:
	CGameMechanics*	m_pGameMechanics;

	CGameMechanicsResource();

public:
	~CGameMechanicsResource()	{}
	CGameMechanicsResource(const tstring &sPath) :
	IResource(sPath, TSNULL),
	m_pGameMechanics(NULL)
	{}

	void			SetMechanics(CGameMechanics *pMechanics)	{ m_pGameMechanics = pMechanics; }
	CGameMechanics*	GetMechanics() const						{ return m_pGameMechanics; }

	int		Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
	void	Free()		{}
	void	PostLoad();

	void	Precache()	{}
};
//=============================================================================

#endif //__C_GAMEMECHANICSRESOURCE_H__