// (C)2006 S2 Games
// c_boxrenderer.h
//
//=============================================================================
#ifndef __C_BOXRENDERER_H__
#define __C_BOXRENDERER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_debugrenderer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CBoxRenderer
//=============================================================================
class CBoxRenderer : public IDebugRenderer
{
private:
	int		m_iNumBoxes;

public:
	static CPool<CBoxRenderer>		s_Pool;
	
	void*	operator new(size_t z, const char *szContext = NULL, const char *szType = NULL, const char *szFile = NULL, short nLine = 0); // Uses CPool of preallocated instances
	void	operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) { }
	
	~CBoxRenderer();
	CBoxRenderer(int iNumBoxes);

	void	Render(EMaterialPhase ePhase);
};
//=============================================================================
#endif //__C_BOXRENDERER_H__
