// (C)2006 S2 Games
// i_effectrenderer.h
//
//=============================================================================
#ifndef __I_EFFECTRENDERER_H__
#define __I_EFFECTRENDERER_H__

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
// IEffectRenderer
//=============================================================================
class IEffectRenderer : public IRenderer
{
protected:
    ResHandle       m_hMaterial;
    uint            m_uiStartIndex;
    uint            m_uiEndIndex;

public:
    ~IEffectRenderer();
    IEffectRenderer(ResHandle hMaterial, uint uiStartIndex, uint uiEndIndex, int iEffectLayer, float fDepth);

    void    Setup(EMaterialPhase ePhase);
};
//=============================================================================
#endif //__I_EFFECTRENDERER_H__
