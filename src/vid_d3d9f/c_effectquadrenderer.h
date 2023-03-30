// (C)2008 S2 Games
// c_effectquadrenderer.h
//
//=============================================================================
#ifndef __C_EFFECTQUADRENDERER_H__
#define __C_EFFECTQUADRENDERER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_effectrenderer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CEffectQuadRenderer
//=============================================================================
class CEffectQuadRenderer : public IEffectRenderer
{
private:

public:
	static CPool<CEffectQuadRenderer>		s_Pool;
	
	void*	operator new(size_t z); // Uses CPool of preallocated instances
	
	~CEffectQuadRenderer();
	CEffectQuadRenderer(ResHandle hMaterial, uint uiStartIndex, uint uiEndIndex, int iEffectLayer, float fDepth);

	void	Render(EMaterialPhase ePhase);
};
//=============================================================================
#endif //__C_EFFECTQUADRENDERER_H__
