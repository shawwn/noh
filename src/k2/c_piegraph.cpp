// (C)2006 S2 Games
// c_piegraph.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_piegraph.h"
#include "c_widgetstyle.h"
#include "c_draw2d.h"
//=============================================================================

/*====================
  CPieGraph::~CPieGraph
  ====================*/
CPieGraph::~CPieGraph()
{
}


/*====================
  CPieGraph::CPieGraph
  ====================*/
CPieGraph::CPieGraph(CInterface* pInterface, IWidget* pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_PIEGRAPH, style),

m_fStart(style.GetPropertyFloat(_T("start"), 0.0f)),
m_fEnd(style.GetPropertyFloat(_T("end"), 360.0f)),
m_fSign(1.0f),
m_iDetail(style.GetPropertyInt(_T("detail"), 8)),
m_bSquare(style.GetPropertyBool(_T("square"), false)),
m_bWrapTexture(style.GetPropertyBool(_T("wraptexture"), false)),

m_fValue(style.GetPropertyFloat(_T("value"), 1.0f)),
m_refCvar(style.GetProperty(_T("cvar")))
{
	m_iDetail = CLAMP(m_iDetail, 2, 20);
	m_fValue = CLAMP(m_fValue, 0.0f, 1.0f);

	if (m_fStart > m_fEnd)
		m_fSign = -1.0f;
}


/*====================
  CPieGraph::DrawSquare
  ====================*/
void	CPieGraph::DrawSquare(const CVec2f &v2Origin)
{
	float fValue(CLAMP(m_refCvar.IsIgnored() ? m_fValue : m_refCvar.GetFloat(), 0.0f, 1.0f));
	if (fValue <= 0.0f)
		return;

	float fHalfWidth(m_recArea.GetWidth() / 2.0f);
	float fHalfHeight(m_recArea.GetHeight() / 2.0f);
	float fEnd(m_fStart + ((m_fEnd - m_fStart) * fValue));

	for (float fStep(m_fStart); ; )
	{
		float fStep1(fStep - fmod(fStep, 45.0f) + 45.0f * m_fSign);
		if (fmod(fStep1, 90.0f) == 0.0f)
			fStep1 += 45.0f * m_fSign;
		float fStep2(fStep1 + (fmod(fStep1, 90.0f) != 0.0f ? 90.0f : 45.0f) * m_fSign);
		if (m_fSign < 0.0f)
			fStep2 = MAX(fStep2, fEnd);
		else
			fStep2 = MIN(fStep2, fEnd);

		float fc(cos(DEG2RAD(45.0f)));
		float fs(sin(DEG2RAD(45.0f)));

		float fCenterX(v2Origin.x + fHalfWidth);
		float fCenterY(v2Origin.y + fHalfHeight);
		
		// ATI + OpenGL doesn't like funny shaped quads, eg, ((-1,-1), (1,-1), (0,-1), (0,0))
		if (fStep > fStep2)
			fStep1 = CLAMP(fStep1, fStep2, fStep);
		else
			fStep1 = CLAMP(fStep1, fStep, fStep2);

		CVec2f v1(fCenterX + fHalfWidth * CLAMP(cosf(DEG2RAD(fStep2)) / fc, -1.0f, 1.0f), fCenterY + fHalfHeight * CLAMP(sinf(DEG2RAD(fStep2)) / fs, -1.0f, 1.0f));
		CVec2f v2(fCenterX + fHalfWidth * CLAMP(cosf(DEG2RAD(fStep1)) / fc, -1.0f, 1.0f), fCenterY + fHalfHeight * CLAMP(sinf(DEG2RAD(fStep1)) / fs, -1.0f, 1.0f));
		CVec2f v3(fCenterX + fHalfWidth * CLAMP(cosf(DEG2RAD(fStep)) / fc, -1.0f, 1.0f), fCenterY + fHalfHeight * CLAMP(sinf(DEG2RAD(fStep)) / fs, -1.0f, 1.0f));
		CVec2f v4(fCenterX, fCenterY);

		CVec2f	t1((CLAMP(cosf(DEG2RAD(fStep2)) / fc, -1.0f, 1.0f) + 1.0f) / 2.0f, 1.0f - ((CLAMP(sinf(DEG2RAD(fStep2)) / fs, -1.0f, 1.0f) + 1.0f) / 2.0f));
		CVec2f	t2((CLAMP(cosf(DEG2RAD(fStep1)) / fc, -1.0f, 1.0f) + 1.0f) / 2.0f, 1.0f - ((CLAMP(sinf(DEG2RAD(fStep1)) / fs, -1.0f, 1.0f) + 1.0f) / 2.0f));
		CVec2f	t3((CLAMP(cosf(DEG2RAD(fStep)) / fc, -1.0f, 1.0f) + 1.0f) / 2.0f, 1.0f - ((CLAMP(sinf(DEG2RAD(fStep)) / fs, -1.0f, 1.0f) + 1.0f) / 2.0f));
		CVec2f	t4(0.5f, 0.5f);

		if (m_fSign > 0.0f)
			Draw2D.Quad(v1, v2, v3, v4, t1, t2, t3, t4, m_hTexture[0]);
		else
			Draw2D.Quad(v3, v2, v1, v4, t3, t2, t1, t4, m_hTexture[0]);

		fStep = fStep2;
		if ((m_fSign < 0.0f && fStep <= fEnd) ||
			(m_fSign > 0.0f && fStep >= fEnd))
			break;
	}
}


/*====================
  CPieGraph::DrawCircle
  ====================*/
void	CPieGraph::DrawCircle(const CVec2f &v2Origin)
{
	float fHalfWidth(m_recArea.GetWidth() / 2.0f);
	float fHalfHeight(m_recArea.GetHeight() / 2.0f);
	float fStep((360.0f / m_iDetail) / 2.0f * m_fSign);
	float fAngle(m_fStart);
	float fValue((m_refCvar.IsIgnored()) ? m_fValue : CLAMP(m_refCvar.GetFloat(), 0.0f, 1.0f));
	float fEnd(m_fStart + ((m_fEnd - m_fStart) * fValue));

	/*
	float fRefX(v2Origin.x + m_recArea.GetWidth() * 1.1f);
	float fRefY(v2Origin.y + m_recArea.GetHeight() * 0.1f);
	float fRefWidth(m_recArea.GetWidth() / 2.0f);
	float fRefHeight(m_recArea.GetHeight() / 2.0f);
	Draw2D.Rect(fRefX, fRefY, fRefWidth, fRefHeight, m_hTexture);
	/**/

	for (int i(0); i < m_iDetail * 2; ++i)
	{	
		float fThisStep;
		if (m_fSign < 0.0f)
			fThisStep = CLAMP(fAngle + fStep, fEnd, m_fStart);
		else
			fThisStep = CLAMP(fAngle + fStep, m_fStart, fEnd);

		CVec2f	v1(fHalfWidth, fHalfHeight);
		CVec2f	v2(fHalfWidth + cos(DEG2RAD(fThisStep)) * fHalfWidth, fHalfHeight + sin(DEG2RAD(fThisStep)) * fHalfHeight);
		CVec2f	v3(fHalfWidth + cos(DEG2RAD(fAngle)) * fHalfWidth, fHalfHeight + sin(DEG2RAD(fAngle)) * fHalfHeight);
		CVec2f	v4(v1);

		CVec2f	t1, t2, t3, t4;
		if (m_bWrapTexture)
		{
			t1.Set(m_fSign * fThisStep / 360.0f, 0.0f);
			t2.Set(m_fSign * fThisStep / 360.0f, 1.0f);
			t3.Set(m_fSign * fAngle / 360.0f, 1.0f);
			t4.Set(m_fSign * fAngle / 360.0f, 0.0f);
		}
		else
		{
			t1.Set(0.5f, 0.5f);
			t2.Set((cos(DEG2RAD(fThisStep)) + 1.0f) / 2.0f, 1.0f - ((sin(DEG2RAD(fThisStep)) + 1.0f) / 2.0f));
			t3.Set((cos(DEG2RAD(fAngle)) + 1.0f) / 2.0f, 1.0f - ((sin(DEG2RAD(fAngle)) + 1.0f) / 2.0f));
			t4.Set(0.5f, 0.5f);
		}

		if (m_fSign < 0.0f)
			Draw2D.Quad(v2Origin + v4, v2Origin + v3, v2Origin + v2, v2Origin + v1, t4, t3, t2, t1, m_hTexture[0]);
		else
			Draw2D.Quad(v2Origin + v1, v2Origin + v2, v2Origin + v3, v2Origin + v4, t1, t2, t3, t4, m_hTexture[0]);

		/*
		CVec4f v4Color(i / (m_iDetail * 2.0f), i / (m_iDetail * 2.0f), i / (m_iDetail * 2.0f), 1.0f);

		Draw2D.Line(CVec2f(fRefX + fRefWidth * t1.x, fRefY + fRefHeight * t1.y), CVec2f(fRefX + fRefWidth * t2.x, fRefY + fRefHeight * t2.y), v4Color, v4Color);
		Draw2D.Line(CVec2f(fRefX + fRefWidth * t2.x, fRefY + fRefHeight * t2.y), CVec2f(fRefX + fRefWidth * t3.x, fRefY + fRefHeight * t3.y), v4Color, v4Color);
		Draw2D.Line(CVec2f(fRefX + fRefWidth * t3.x, fRefY + fRefHeight * t3.y), CVec2f(fRefX + fRefWidth * t4.x, fRefY + fRefHeight * t4.y), v4Color, v4Color);
		Draw2D.Line(CVec2f(fRefX + fRefWidth * t4.x, fRefY + fRefHeight * t4.y), CVec2f(fRefX + fRefWidth * t1.x, fRefY + fRefHeight * t1.y), v4Color, v4Color);
		/**/

		/*
		Draw2D.Line(v2Origin + v4, v2Origin + v3, v4Color, v4Color);
		Draw2D.Line(v2Origin + v3, v2Origin + v2, v4Color, v4Color);
		Draw2D.Line(v2Origin + v2, v2Origin + v1, v4Color, v4Color);
		Draw2D.Line(v2Origin + v1, v2Origin + v4, v4Color, v4Color);
		/**/

		fAngle += fStep;
		if ((m_fSign > 0.0f && fAngle >= fEnd) ||
			(m_fSign < 0.0f && fAngle <= fEnd))
			break;
	}
}


/*====================
  CPieGraph::RenderWidget
  ====================*/
void	CPieGraph::RenderWidget(const CVec2f& v2Origin, float fFade)
{
	if (HasFlags(WFLAG_NO_DRAW) || !HasFlags(WFLAG_VISIBLE))
		return;

	const CVec4f &v4Color(fFade == 1.0f ? m_v4Color : GetFadedColor(m_v4Color, fFade));

	Draw2D.SetColor(v4Color);

	if (m_bSquare)
		DrawSquare(v2Origin);
	else
		DrawCircle(v2Origin);
}
