// (C)2010 S2 Games
// c_rampresource.h
//
//=============================================================================
#ifndef __C_RAMPRESOURCE_H__
#define __C_RAMPRESOURCE_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/i_resource.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct rRampPiece {
	tstring sTopPath;
	int iTopRotationVertex;
	float	fTopDefRot;
	tstring sBotPath;
	int iBotRotationVertex;
	float	fBotDefRot;
};


//=============================================================================
// CRampResource
//=============================================================================
 class CRampResource : public IResource
{
protected:
	tstring						m_sRampName;
	rRampPiece					m_rRampType1;
	rRampPiece					m_rRampType2;
	float						m_fDefaultRotation;


public:
	K2_API ~CRampResource()	{}
	K2_API CRampResource(const tstring &sPath = TSNULL) :
	IResource(sPath, TSNULL),
	m_fDefaultRotation(0.0f),
	m_sRampName(_T("undefined"))
	{}

	K2_API	virtual uint			GetResType() const			{ return RES_RAMP; }
	K2_API	virtual const tstring&	GetResTypeName() const		{ return ResTypeName(); }
	K2_API	static const tstring&	ResTypeName()				{ static tstring sTypeName(_T("{ramp}")); return sTypeName; }

	int		Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
	void	Free()											{}
	void	PostLoad();
	void	Reloaded();

	K2_API	void	SetRampName(tstring name)				{ m_sRampName = name; } 
	K2_API  void	SetRampType1(rRampPiece rRampType1)		{ m_rRampType1 = rRampType1; }
	K2_API	void	SetRampType2(rRampPiece rRampType2)		{ m_rRampType2 = rRampType2; }

	K2_API	const tstring		GetRampName() const					{ return m_sRampName; }
	K2_API	const rRampPiece	GetRampType1() const				{ return m_rRampType1; }
	K2_API	const rRampPiece	GetRampType2() const				{ return m_rRampType2; }
	
};
 //=============================================================================

#endif //__C_RAMPRESOURCE_H__
