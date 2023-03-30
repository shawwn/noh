#include "k2_common.h"
#include "c_DebugRenderer.h"
#include "c_draw2d.h"
#include "c_camera.h"
#include "c_vid.h"

CDebugRenderer *CDebugRenderer::s_pDebugRenderer(NULL);
CDebugRenderer &g_DebugRenderer(*CDebugRenderer::GetInstance());

void CDebugRenderer::AddLine(CVec3f v3Src, CVec3f v3Dst, CVec4f v4Color)
{
	m_vecWorldLines.push_back(SLine(v3Src, v3Dst, v4Color));
}

void CDebugRenderer::AddRect(CVec3f v3TL, CVec3f v3TR, CVec3f v3BL, CVec3f v3BR, CVec4f v4Color)
{
	m_vecWorldRects.push_back(SRect(v3TL, v3TR, v3BL, v3BR, v4Color));
}

void CDebugRenderer::Frame(CCamera *pCamera)
{
	{
		LineVector::const_iterator cit(m_vecWorldLines.begin()), citEnd(m_vecWorldLines.end());
		for (; cit != citEnd; ++cit)
			Vid.AddLine(cit->v3Src, cit->v3Dst, cit->v4Color);
	}

	{
		RectVector::const_iterator cit(m_vecWorldRects.begin()), citEnd(m_vecWorldRects.end());

		for (; cit != citEnd; ++cit)
		{
			CVec2f v2TL, v2TR, v2BL, v2BR;

			pCamera->WorldToScreen(cit->v3TL, v2TL);
			pCamera->WorldToScreen(cit->v3TR, v2TR);
			pCamera->WorldToScreen(cit->v3BL, v2BL);
			pCamera->WorldToScreen(cit->v3BR, v2BR);

			Draw2D.Line(v2TL, v2TR, cit->v4Color, cit->v4Color);
			Draw2D.Line(v2TR, v2BR, cit->v4Color, cit->v4Color);
			Draw2D.Line(v2BR, v2BL, cit->v4Color, cit->v4Color);
			Draw2D.Line(v2BL, v2TL, cit->v4Color, cit->v4Color);
		}
	}
}

void CDebugRenderer::ClearLists()
{
	m_vecWorldLines.clear();
	m_vecWorldRects.clear();
}

CDebugRenderer *CDebugRenderer::GetInstance()
{
	if (!s_pDebugRenderer)
		s_pDebugRenderer = K2_NEW(ctx_Singleton,  CDebugRenderer)();

	return s_pDebugRenderer;
}
