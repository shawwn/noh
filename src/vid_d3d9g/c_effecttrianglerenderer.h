// (C)2006 S2 Games
// c_effecttrianglerenderer.h
//
//=============================================================================
#ifndef __C_EFFECTTRIANGLERENDERER_H__
#define __C_EFFECTTRIANGLERENDERER_H__

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
// CEffectTriangleRenderer
//=============================================================================
class CEffectTriangleRenderer : public IEffectRenderer
{
private:

public:
	static CPool<CEffectTriangleRenderer>		s_Pool;
	
	void*	operator new(size_t z); // Uses CPool of preallocated instances
	
	~CEffectTriangleRenderer();
	CEffectTriangleRenderer(ResHandle hMaterial, uint uiStartIndex, uint uiEndIndex, int iEffectLayer, float fDepth);

	void	Render(EMaterialPhase ePhase);
};
//=============================================================================
#endif //__C_EFFECTTRIANGLERENDERER_H__
