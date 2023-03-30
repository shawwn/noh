// (C)2009 S2 Games
// c_treedefinitionresource.h
//
//=============================================================================
#ifndef __C_TREEDEFINITIONRESOURCE_H__
#define __C_TREEDEFINITIONRESOURCE_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/i_resource.h"
//============================================================================

//=============================================================================
// CTreeDefinitionResource
//=============================================================================
class CTreeDefinitionResource : public IResource
{
protected:
	float				m_fTreeScaleMin;
	float				m_fTreeScaleMax;
	tstring				m_sTreeModelPath;
	tstring				m_sTreeName;
	tstring				m_sTreeDefPath;

	CTreeDefinitionResource();

public:
	~CTreeDefinitionResource()	{}
	CTreeDefinitionResource(const tstring &sPath) :
	IResource(sPath, TSNULL),
	m_sTreeModelPath(_T("undefined")),
	m_sTreeDefPath(sPath),
	m_sTreeName(_T("undefined")),
	m_fTreeScaleMin(2.4f),
	m_fTreeScaleMax(2.6f)
	{}
	
	int		Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
	void	Free()											{}
	void	PostLoad();
	void	Reloaded();

	void	SetTreeScaleMin(float tScale)	  { m_fTreeScaleMin = tScale; }
	void	SetTreeScaleMax(float tScale)	  { m_fTreeScaleMax = tScale; }
	void	SetTreeModelPath(tstring path)	  { m_sTreeModelPath = path; }
	void	SetTreeName(tstring name)		  { m_sTreeName = name; }
	void	SetTreeDefPath(tstring dpath)	  { m_sTreeDefPath = dpath; } 

	float		GetTreeScaleMin()			{ return m_fTreeScaleMin; }
	float		GetTreeScaleMax()			{ return m_fTreeScaleMax; }
	tstring		GetTreeModelPath()			{ return m_sTreeModelPath; }
	tstring		GetTreeName()				{ return m_sTreeName; }
	tstring		GetTreeDefPath()			{ return m_sTreeDefPath; }



};
//=============================================================================

#endif //__C_TREEDEFINITIONRESOURCE_H__
