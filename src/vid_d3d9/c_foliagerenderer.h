// (C)2006 S2 Games
// c_foliagerenderer.h
//
//=============================================================================
#ifndef __C_FOLIAGERENDERER_H__
#define __C_FOLIAGERENDERER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_renderer.h"
#include "d3d9_foliage.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int FOLIAGE_ALPHATEST		(BIT(0));
const int FOLIAGE_ALPHABLEND	(BIT(1));
const int FOLIAGE_DEPTHWRITE	(BIT(2));
//=============================================================================

//=============================================================================
// CFoliageRenderer
//=============================================================================
class CFoliageRenderer : public IRenderer
{
private:
	EFoliageRenderOrder	m_eOrder;
	int					m_iFlags;

	bool				m_bFirstChunk;

	int					m_iDiffuseStageIndex;

	void	RenderChunk(EMaterialPhase ePhase, int iChunkX, int iChunkY);
	void	RenderChunkNormals(EMaterialPhase ePhase, int iChunkX, int iChunkY);

public:
	static CPool<CFoliageRenderer>		s_Pool;
	
	void*	operator new(size_t z, const char *szContext = NULL, const char *szType = NULL, const char *szFile = NULL, short nLine = 0); // Uses CPool of preallocated instances
	void	operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) { }
	
	~CFoliageRenderer();
	CFoliageRenderer(EFoliageRenderOrder eOrder, int iFlags);

	void	Setup(EMaterialPhase ePhase);
	void	Render(EMaterialPhase ePhase);
};
//=============================================================================
#endif //__C_FOLIAGERENDERITEM_H__
