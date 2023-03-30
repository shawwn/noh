// (C)2006 S2 Games
// c_terrainrenderer.h
//
//=============================================================================
#ifndef __C_TERRAINRENDERER_H__
#define __C_TERRAINRENDERER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_renderer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CTerrainRenderer
//=============================================================================
class CTerrainRenderer : public IRenderer
{
private:
	void	RenderChunk(EMaterialPhase ePhase, int iChunkX, int iChunkY);
	void	RenderChunkNormals(EMaterialPhase ePhase, int iChunkX, int iChunkY);
	void	RenderChunkGrid(EMaterialPhase ePhase, int iChunkX, int iChunkY);
	void	RenderChunkCliffs(EMaterialPhase ePhase, int iChunkX, int iChunkY);

	bool	m_bFirstChunk;
	int		m_iAlphaStageIndex;
	int		m_iDiffuse0StageIndex;
	int		m_iNormalmap0StageIndex;
	int		m_iNormalmap01StageIndex;
	int		m_iDiffuse1StageIndex;
	int		m_iNormalmap1StageIndex;
	int		m_iNormalmap11StageIndex;

public:
	static CPool<CTerrainRenderer>		s_Pool;
	
	void*	operator new(size_t z, const char *szContext = NULL, const char *szType = NULL, const char *szFile = NULL, short nLine = 0); // Uses CPool of preallocated instances
	void	operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) { }

	~CTerrainRenderer();
	CTerrainRenderer();

	void	Setup(EMaterialPhase ePhase);
	void	Render(EMaterialPhase ePhase);

};
//=============================================================================
#endif //__C_TERRAINRENDERER_H__
