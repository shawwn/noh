// (C)2009 S2 Games
// c_Cliffdefinitionresource.h
//
//=============================================================================
#ifndef __C_CLIFFDEFINITIONRESOURCE_H__
#define __C_CLIFFDEFINITIONRESOURCE_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/i_resource.h"
#include "../k2/c_rampresource.h"
//============================================================================

//=============================================================================
// CVariation
//=============================================================================
class CVariation
{
private:
	tstring				m_sPath;
	float				m_fDefaultRotation;
	int					m_iRotationVertex;

public:
	~CVariation()	{}
	CVariation() {};

	tstring		GetPiecePath()						{ return m_sPath; }
	float		GetDefaultRotation()				{ return m_fDefaultRotation; }
	int			GetRotationVertex()					{ return m_iRotationVertex; }

	void		SetPiecePath(tstring sPath)			{ m_sPath = sPath; }
	void		SetDefaultRotation(float fRotation)	{ m_fDefaultRotation = fRotation; }
	void		SetRotationVertex(int iRotationVertex) { m_iRotationVertex = iRotationVertex; }
};
//=============================================================================

//=============================================================================
// CCliffPiece
//=============================================================================
class CCliffPiece
{
private:
	tstring							m_sType;
	std::vector<CVariation>			m_vVariations;

public:
	~CCliffPiece()	{}
	CCliffPiece() {}

	tstring		GetPieceType()						{ return m_sType; }
	void		SetPieceType(tstring sType)			{ m_sType = sType; }

	K2_API std::vector<CVariation>*		GetVariations()					{ return &m_vVariations; }
	K2_API CVariation*					GetVariation(int iVariation);
};
//=============================================================================

//=============================================================================
// CCliffDefinitionResource
//=============================================================================
 class CCliffDefinitionResource : public IResource
{
protected:
	tstring						m_sCliffName;
	tstring						m_sCliffDefPath;
	std::vector<CCliffPiece>	m_vCliffPieces;
	CRampResource				m_Ramp;

public:
	K2_API ~CCliffDefinitionResource()	{}
	K2_API CCliffDefinitionResource(const tstring &sPath) :
	IResource(sPath, TSNULL),
	m_sCliffDefPath(sPath),
	m_sCliffName(_T("undefined"))
	{}

	K2_API	virtual uint			GetResType() const			{ return RES_CLIFFDEF; }
	K2_API	virtual const tstring&	GetResTypeName() const		{ return ResTypeName(); }
	K2_API	static const tstring&	ResTypeName()				{ static tstring sTypeName(_T("{cliff}")); return sTypeName; }

	int		Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
	void	Free()											{}
	void	PostLoad();
	void	Reloaded();


	K2_API	void			SetCliffName(tstring name)		{ m_sCliffName = name; }
	K2_API	void			SetCliffDefPath(tstring dpath)	{ m_sCliffDefPath = dpath; } 

	K2_API	CRampResource*	GetRamp()						{ return &m_Ramp; }
	K2_API	CCliffPiece		GetInnerCorner();
	K2_API	CCliffPiece		GetOuterCorner();
	K2_API	CCliffPiece		GetFront();
	K2_API	CCliffPiece		GetWedge();
	K2_API	CCliffPiece		GetInnerCorner256();
	K2_API	CCliffPiece		GetOuterCorner256();
	K2_API	CCliffPiece		GetFront256();
	K2_API	CCliffPiece		GetWedge256();
	K2_API	CCliffPiece		GetOuterCornerTransition1();
	K2_API	CCliffPiece		GetOuterCornerTransition2();
	K2_API	CCliffPiece		GetInnerCornerTransition1();
	K2_API	CCliffPiece		GetInnerCornerTransition2();
	K2_API	CCliffPiece		GetFrontTransition1();
	K2_API	CCliffPiece		GetFrontTransition2();
	K2_API	CCliffPiece		GetInnerCornerSimple();
	K2_API	CCliffPiece		GetWedgeTransition();
	K2_API	CCliffPiece		GetWedgeShift();

	K2_API	tstring		GetCliffName()				{ return m_sCliffName; }
	K2_API	tstring		GetCliffDefPath()			{ return m_sCliffDefPath; }
	
	K2_API	std::vector<CCliffPiece>* GetCliffPieces() { return &m_vCliffPieces; }
};
 //=============================================================================

#endif //__C_CLIFFDEFINITIONRESOURCE_H__
