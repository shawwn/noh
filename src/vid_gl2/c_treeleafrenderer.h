// (C)2006 S2 Games
// c_treeleafrenderer.h
//
//=============================================================================
#ifndef __C_TREELEAFRENDERER_H__
#define __C_TREELEAFRENDERER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_renderer.h"
#include "../k2/c_treemodel.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CTreeModelDef;
class CSceneEntity;
//=============================================================================

//=============================================================================
// CTreeLeafRenderer
//=============================================================================
class CTreeLeafRenderer : public IRenderer
{
private:
	const CTreeModelDef	*m_pTreeDef;
	const CSceneEntity	&m_cEntity;
	CVec3f				m_v3Dir;
	
	// Computed in Setup
	SLODData			m_avLeafLODs[2];

public:
	static CPool<CTreeLeafRenderer>		s_Pool;
	
	void*	operator new(size_t z, const char *szContext = NULL, const char *szType = NULL, const char *szFile = NULL, short nLine = 0); // Uses CPool of preallocated instances
	void	operator delete(void *p) { }
	void	operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) { }
	
	~CTreeLeafRenderer();
	CTreeLeafRenderer(const CSceneEntity &cEntity, const CTreeModelDef *pTreeDef,
		const D3DXMATRIXA16 &mWorldViewProj,
		const D3DXMATRIXA16 &mWorld,
		const D3DXMATRIXA16 &mWorldInverseRotate);

	void	Setup(EMaterialPhase ePhase);
	void	Render(EMaterialPhase ePhase);
};
//=============================================================================
#endif //__C_TREELEAFRENDERER_H__
