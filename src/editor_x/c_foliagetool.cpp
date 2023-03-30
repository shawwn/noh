// (C)2005 S2 Games
// c_foliagetool.cpp
//
// Foliage Tool
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"

#include "c_foliagetool.h"

#include "../k2/c_brush.h"
#include "../k2/c_world.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_vid.h"
#include "../k2/c_texture.h"
#include "../k2/c_scenemanager.h"
#include "../k2/s_foliagetile.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_fontmap.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
CVAR_INT    (le_foliageMode,                FOLIAGE_TEXTURE);
CVAR_INT    (le_foliageLayer,               0);
CVAR_FLOAT  (le_foliageDensity,             16.0f);
CVAR_FLOAT  (le_foliageDensity2,            0.0f);
CVAR_FLOAT  (le_foliageSizeWidth,           64.0f);
CVAR_FLOAT  (le_foliageSizeWidth2,          64.0f);
CVAR_FLOAT  (le_foliageSizeHeight,          32.0f);
CVAR_FLOAT  (le_foliageSizeHeight2,         32.0f);
CVAR_FLOAT  (le_foliageSizeRandWidth,       0.0f);
CVAR_FLOAT  (le_foliageSizeRandWidth2,      0.0f);
CVAR_FLOAT  (le_foliageSizeRandHeight,      8.0f);
CVAR_FLOAT  (le_foliageSizeRandHeight2,     8.0f);
CVAR_FLOAT  (le_foliageScale,               1.0f);
CVAR_FLOAT  (le_foliageScale2,              0.0f);
CVAR_FLOAT  (le_foliageAuto,                1.0f);
CVAR_FLOAT  (le_foliageAuto2,               1.0f);
CVAR_FLOAT  (le_foliageBrushStrength,       50.0f);
CVAR_BOOL   (le_foliageDrawBrushInfluence,  true);
CVAR_FLOAT  (le_foliageBrushInfluenceAlpha, 1.0f);
CVAR_STRING (le_foliageTexture,             "/world/foliage/textures/greengrass.tga");
CVAR_INT    (le_foliageCrossQuads,          2);
CVAR_BOOL   (le_foliageCamera,              false);
CVAR_BOOL   (le_foliageParallel,            false);
CVAR_BOOLF  (le_foliageDrawBrushCoords,     true,   CVAR_SAVECONFIG);
CVAR_BOOLF  (le_foliageDrawInfo,            true,   CVAR_SAVECONFIG);

UI_TRIGGER  (FoliageMode);
UI_TRIGGER  (FoliageTexture);
//=============================================================================

/*====================
  CFoliageTool::CFoliageTool(
  ====================*/
CFoliageTool::CFoliageTool() :
ITool(TOOL_FOLIAGE, _T("foliage")),
m_bWorking(false),
m_bInverse(false),
m_hLineMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL)),
m_hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP))
{
    FoliageMode.Trigger(_T("Texture"));
}


/*====================
  CFoliageTool::PrimaryUp

  Left mouse button up action
  ====================*/
void    CFoliageTool::PrimaryUp()
{
    if (!m_bInverse)
        m_bWorking = false;
}


/*====================
  CFoliageTool::PrimaryDown

  Default left mouse button down action
  ====================*/
void    CFoliageTool::PrimaryDown()
{
    m_bWorking = true;
    m_bInverse = false;
    CalcToolProperties();
}


/*====================
  CFoliageTool::SecondaryUp

  Right mouse button up action
  ====================*/
void    CFoliageTool::SecondaryUp()
{
    if (m_bInverse)
        m_bWorking = false;
}


/*====================
  CFoliageTool::SecondaryDown

  Default right mouse button down action - not used in this case
  ====================*/
void    CFoliageTool::SecondaryDown()
{
    m_bWorking = true;
    m_bInverse = true;
    CalcToolProperties();
}


/*====================
  CFoliageTool::TertiaryUp

  Middle mouse button up action
  ====================*/
void    CFoliageTool::TertiaryUp()
{
}


/*====================
  CFoliageTool::TertiaryDown

  Middle mouse button down action
  ====================*/
void    CFoliageTool::TertiaryDown()
{
}


/*====================
  CFoliageTool::QuaternaryUp

  Scroll wheel up action
  ====================*/
void    CFoliageTool::QuaternaryUp()
{
}


/*====================
  CFoliageTool::QuaternaryDown

  Scroll wheel down action
  ====================*/
void    CFoliageTool::QuaternaryDown()
{
}


/*====================
  CFoliageTool::Cancel
  ====================*/
void    CFoliageTool::Cancel()
{
}


/*====================
  CFoliageTool::Delete
  ====================*/
void    CFoliageTool::Delete()
{
}


/*====================
 CFoliageTool::CalcToolProperties
 ====================*/
void     CFoliageTool::CalcToolProperties()
{
    STraceInfo trace;

    if (Editor.TraceCursor(trace, TRACE_TERRAIN))
    {
        m_iX = Editor.GetWorld().GetVertFromCoord(trace.v3EndPos.x);
        m_iY = Editor.GetWorld().GetVertFromCoord(trace.v3EndPos.y);
        m_v3EndPos = trace.v3EndPos;
    }
    else
    {
        m_iX = -1;
        m_iY = -1;
        m_v3EndPos.Clear();
    }

    if (m_bInverse)
        m_fAmount = le_foliageDensity2;
    else
        m_fAmount = le_foliageDensity;

    if (le_foliageTexture.IsModified())
    {
        m_hTexture = g_ResourceManager.Register(le_foliageTexture, RES_TEXTURE);
        FoliageTexture.Trigger(le_foliageTexture);
        le_foliageTexture.SetModified(false);
    }
}


/*====================
  CFoliageTool::AddFoliageDensity
  ====================*/
void    CFoliageTool::AddFoliageDensity(SFoliageVertexEntry *pRegion, const CRecti &recArea, const CBrush &brush, float fDensity, float fStrength)
{
    int iBrushSize(brush.GetBrushSize());

    int iRegionIndex(0);
    for (int y(0); y < recArea.GetHeight(); ++y)
    {
        for (int x(0); x < recArea.GetWidth(); ++x)
        {
            float fDelta = brush[BRUSH_INDEX(x, y)] * fStrength;

            if (pRegion[iRegionIndex].fDensity > fDensity)
            {
                pRegion[iRegionIndex].fDensity -= fDelta;

                if(pRegion[iRegionIndex].fDensity < fDensity)
                    pRegion[iRegionIndex].fDensity = fDensity;
            }
            else if (pRegion[iRegionIndex].fDensity < fDensity)
            {
                pRegion[iRegionIndex].fDensity += fDelta;

                if(pRegion[iRegionIndex].fDensity > fDensity)
                    pRegion[iRegionIndex].fDensity = fDensity;
            }
            ++iRegionIndex;
        }
    }
}


/*====================
  CFoliageTool::AddFoliageSize
  ====================*/
void    CFoliageTool::AddFoliageSize(SFoliageVertexEntry *pRegion, const CRecti &recArea,
                          const CBrush &brush, float fWidth, float fHeight, float fRandWidth, float fRandHeight, float fStrength)
{
    int iBrushSize(brush.GetBrushSize());

    int iRegionIndex(0);
    for (int y(0); y < recArea.GetHeight(); ++y)
    {
        for (int x(0); x < recArea.GetWidth(); ++x)
        {
            float fDelta = brush[BRUSH_INDEX(x, y)] * fStrength;

            // Width
            if (pRegion[iRegionIndex].v3Size.x > fWidth)
            {
                pRegion[iRegionIndex].v3Size.x -= fDelta;

                if(pRegion[iRegionIndex].v3Size.x < fWidth)
                    pRegion[iRegionIndex].v3Size.x = fWidth;
            }
            else if (pRegion[iRegionIndex].v3Size.x < fWidth)
            {
                pRegion[iRegionIndex].v3Size.x += fDelta;

                if(pRegion[iRegionIndex].v3Size.x > fWidth)
                    pRegion[iRegionIndex].v3Size.x = fWidth;
            }

            // Height
            if (pRegion[iRegionIndex].v3Size.y > fHeight)
            {
                pRegion[iRegionIndex].v3Size.y -= fDelta;

                if(pRegion[iRegionIndex].v3Size.y < fHeight)
                    pRegion[iRegionIndex].v3Size.y = fHeight;
            }
            else if (pRegion[iRegionIndex].v3Size.y < fHeight)
            {
                pRegion[iRegionIndex].v3Size.y += fDelta;

                if(pRegion[iRegionIndex].v3Size.y > fHeight)
                    pRegion[iRegionIndex].v3Size.y = fHeight;
            }

            // Width Jitter
            if (pRegion[iRegionIndex].v3Variance.x > fRandWidth)
            {
                pRegion[iRegionIndex].v3Variance.x -= fDelta;

                if(pRegion[iRegionIndex].v3Variance.x < fRandWidth)
                    pRegion[iRegionIndex].v3Variance.x = fRandWidth;
            }
            else if (pRegion[iRegionIndex].v3Variance.x < fRandWidth)
            {
                pRegion[iRegionIndex].v3Variance.x += fDelta;

                if(pRegion[iRegionIndex].v3Variance.x > fRandWidth)
                    pRegion[iRegionIndex].v3Variance.x = fRandWidth;
            }

            // Height Jitter
            if (pRegion[iRegionIndex].v3Variance.y > fRandHeight)
            {
                pRegion[iRegionIndex].v3Variance.y -= fDelta;

                if(pRegion[iRegionIndex].v3Variance.y < fRandHeight)
                    pRegion[iRegionIndex].v3Variance.y = fRandHeight;
            }
            else if (pRegion[iRegionIndex].v3Variance.y < fRandHeight)
            {
                pRegion[iRegionIndex].v3Variance.y += fDelta;

                if(pRegion[iRegionIndex].v3Variance.y > fRandHeight)
                    pRegion[iRegionIndex].v3Variance.y = fRandHeight;
            }
            ++iRegionIndex;
        }
    }
}


/*====================
  CFoliageTool::AddFoliageScale
  ====================*/
void    CFoliageTool::AddFoliageScale(SFoliageVertexEntry *pRegion, const CRecti &recArea, const CBrush &brush, float fFoliageScale, float fStrength)
{
    int iBrushSize(brush.GetBrushSize());

    int iRegionIndex(0);
    for (int y(0); y < recArea.GetHeight(); ++y)
    {
        for (int x(0); x < recArea.GetWidth(); ++x)
        {
            float fDelta(brush[BRUSH_INDEX(x, y)] * fStrength);

            if (pRegion[iRegionIndex].v3Size.z > fFoliageScale)
            {
                pRegion[iRegionIndex].v3Size.z -= fDelta;

                if(pRegion[iRegionIndex].v3Size.z < fFoliageScale)
                    pRegion[iRegionIndex].v3Size.z = fFoliageScale;
            }
            else if (pRegion[iRegionIndex].v3Size.z < fFoliageScale)
            {
                pRegion[iRegionIndex].v3Size.z += fDelta;

                if(pRegion[iRegionIndex].v3Size.z > fFoliageScale)
                    pRegion[iRegionIndex].v3Size.z = fFoliageScale;
            }
            ++iRegionIndex;
        }
    }
}


/*====================
  CFoliageTool::FoliageSmooth
  ====================*/
void    CFoliageTool::FoliageSmooth(SFoliageVertexEntry *pRegion, const CRecti &recArea, const CBrush &brush, float fStrength)
{
    #define REGION_INDEX(x, y) ((x) + ((y) * recArea.GetWidth()))

    int iBrushSize(brush.GetBrushSize());
    SFoliageVertexEntry *pRegionClean(new SFoliageVertexEntry[recArea.GetArea()]);

    MemManager.Copy(pRegionClean, pRegion, sizeof(SFoliageVertexEntry) * recArea.GetArea());

    for (int y(1); y < recArea.GetHeight() - 1; ++y)
    {
        for (int x(1); x < recArea.GetWidth() - 1; ++x)
        {
            float fLerp(brush[BRUSH_INDEX(x, y)] * fStrength);

            fLerp = CLAMP(fLerp, 0.0f, 1.0f);
            if (!fLerp)
                continue;

            CVec3f v3AverageSize =  pRegionClean[REGION_INDEX(x - 1, y - 1)].v3Size +
                                    pRegionClean[REGION_INDEX(x, y - 1)].v3Size +
                                    pRegionClean[REGION_INDEX(x + 1, y - 1)].v3Size +
                                    pRegionClean[REGION_INDEX(x - 1, y)].v3Size +
                                    pRegionClean[REGION_INDEX(x, y)].v3Size +
                                    pRegionClean[REGION_INDEX(x + 1, y)].v3Size +
                                    pRegionClean[REGION_INDEX(x - 1, y + 1)].v3Size +
                                    pRegionClean[REGION_INDEX(x, y + 1)].v3Size +
                                    pRegionClean[REGION_INDEX(x + 1, y + 1)].v3Size;

            v3AverageSize /= 9.0f;

            pRegion[REGION_INDEX(x, y)].v3Size = LERP(fLerp, pRegion[REGION_INDEX(x, y)].v3Size, v3AverageSize);

            CVec3f v3AverageVariance =  pRegionClean[REGION_INDEX(x - 1, y - 1)].v3Variance +
                                        pRegionClean[REGION_INDEX(x, y - 1)].v3Variance +
                                        pRegionClean[REGION_INDEX(x + 1, y - 1)].v3Variance +
                                        pRegionClean[REGION_INDEX(x - 1, y)].v3Variance +
                                        pRegionClean[REGION_INDEX(x, y)].v3Variance +
                                        pRegionClean[REGION_INDEX(x + 1, y)].v3Variance +
                                        pRegionClean[REGION_INDEX(x - 1, y + 1)].v3Variance +
                                        pRegionClean[REGION_INDEX(x, y + 1)].v3Variance +
                                        pRegionClean[REGION_INDEX(x + 1, y + 1)].v3Variance;

            v3AverageVariance /= 9.0f;

            pRegion[REGION_INDEX(x, y)].v3Variance = LERP(fLerp, pRegion[REGION_INDEX(x, y)].v3Variance, v3AverageVariance);

            float fDensity =    pRegionClean[REGION_INDEX(x - 1, y - 1)].fDensity +
                                pRegionClean[REGION_INDEX(x, y - 1)].fDensity +
                                pRegionClean[REGION_INDEX(x + 1, y - 1)].fDensity +
                                pRegionClean[REGION_INDEX(x - 1, y)].fDensity +
                                pRegionClean[REGION_INDEX(x, y)].fDensity +
                                pRegionClean[REGION_INDEX(x + 1, y)].fDensity +
                                pRegionClean[REGION_INDEX(x - 1, y + 1)].fDensity +
                                pRegionClean[REGION_INDEX(x, y + 1)].fDensity +
                                pRegionClean[REGION_INDEX(x + 1, y + 1)].fDensity;

            fDensity /= 9.0f;

            pRegion[REGION_INDEX(x, y)].fDensity = LERP(fLerp, pRegion[REGION_INDEX(x, y)].fDensity, fDensity);
        }
    }

    delete[] pRegionClean;

    #undef REGION_INDEX
}


/*====================
  CFoliageTool::FoliageAuto
  ====================*/
void    CFoliageTool::FoliageAuto(SFoliageVertexEntry *pRegion, const CRecti &recArea, const CBrush &brush, float fDensity, float fWidth, float fHeight, float fRandWidth, float fRandHeight, float fScale, float fStrength)
{
    int iBrushSize(brush.GetBrushSize());

    int iRegionIndex(0);
    for (int y(0); y < recArea.GetHeight(); ++y)
    {
        for (int x(0); x < recArea.GetWidth(); ++x)
        {
            float fDelta = brush[BRUSH_INDEX(x, y)] * fStrength;

            // Density
            if (pRegion[iRegionIndex].fDensity > fDensity)
            {
                pRegion[iRegionIndex].fDensity -= fDelta;

                if(pRegion[iRegionIndex].fDensity < fDensity)
                    pRegion[iRegionIndex].fDensity = fDensity;
            }
            else if (pRegion[iRegionIndex].fDensity < fDensity)
            {
                pRegion[iRegionIndex].fDensity += fDelta;

                if(pRegion[iRegionIndex].fDensity > fDensity)
                    pRegion[iRegionIndex].fDensity = fDensity;
            }

            // Width
            if (pRegion[iRegionIndex].v3Size.x > fWidth)
            {
                pRegion[iRegionIndex].v3Size.x -= fDelta;

                if(pRegion[iRegionIndex].v3Size.x < fWidth)
                    pRegion[iRegionIndex].v3Size.x = fWidth;
            }
            else if (pRegion[iRegionIndex].v3Size.x < fWidth)
            {
                pRegion[iRegionIndex].v3Size.x += fDelta;

                if(pRegion[iRegionIndex].v3Size.x > fWidth)
                    pRegion[iRegionIndex].v3Size.x = fWidth;
            }

            // Height
            if (pRegion[iRegionIndex].v3Size.y > fHeight)
            {
                pRegion[iRegionIndex].v3Size.y -= fDelta;

                if(pRegion[iRegionIndex].v3Size.y < fHeight)
                    pRegion[iRegionIndex].v3Size.y = fHeight;
            }
            else if (pRegion[iRegionIndex].v3Size.y < fHeight)
            {
                pRegion[iRegionIndex].v3Size.y += fDelta;

                if(pRegion[iRegionIndex].v3Size.y > fHeight)
                    pRegion[iRegionIndex].v3Size.y = fHeight;
            }

            // Width Jitter
            if (pRegion[iRegionIndex].v3Variance.x > fRandWidth)
            {
                pRegion[iRegionIndex].v3Variance.x -= fDelta;

                if(pRegion[iRegionIndex].v3Variance.x < fRandWidth)
                    pRegion[iRegionIndex].v3Variance.x = fRandWidth;
            }
            else if (pRegion[iRegionIndex].v3Variance.x < fRandWidth)
            {
                pRegion[iRegionIndex].v3Variance.x += fDelta;

                if(pRegion[iRegionIndex].v3Variance.x > fRandWidth)
                    pRegion[iRegionIndex].v3Variance.x = fRandWidth;
            }

            // Height Jitter
            if (pRegion[iRegionIndex].v3Variance.y > fRandHeight)
            {
                pRegion[iRegionIndex].v3Variance.y -= fDelta;

                if(pRegion[iRegionIndex].v3Variance.y < fRandHeight)
                    pRegion[iRegionIndex].v3Variance.y = fRandHeight;
            }
            else if (pRegion[iRegionIndex].v3Variance.y < fRandHeight)
            {
                pRegion[iRegionIndex].v3Variance.y += fDelta;

                if(pRegion[iRegionIndex].v3Variance.y > fRandHeight)
                    pRegion[iRegionIndex].v3Variance.y = fRandHeight;
            }

            // Scale
            float fScaleDelta(fDelta / 16.0f);
            if (pRegion[iRegionIndex].v3Size.z > fScale)
            {
                pRegion[iRegionIndex].v3Size.z -= fScaleDelta;

                if(pRegion[iRegionIndex].v3Size.z < fScale)
                    pRegion[iRegionIndex].v3Size.z = fScale;
            }
            else if (pRegion[iRegionIndex].v3Size.z < fScale)
            {
                pRegion[iRegionIndex].v3Size.z += fScaleDelta;

                if(pRegion[iRegionIndex].v3Size.z > fScale)
                    pRegion[iRegionIndex].v3Size.z = fScale;
            }

            ++iRegionIndex;
        }
    }
}


/*====================
  CFoliageTool::PaintFoliageVertex
  ====================*/
void    CFoliageTool::PaintFoliageVertex(float fFrameTime)
{
    SFoliageVertexEntry *pRegion(NULL);

    try
    {
        CBrush *pBrush(CBrush::GetCurrentBrush());
        if (pBrush == NULL)
            EX_ERROR(_T("No brush selected"));

        if (!Editor.GetWorld().IsInBounds(m_iX, m_iY, GRID_SPACE))
            EX_WARN(_T("Out of bounds coordinate"));

        // Clip against the brush data
        CRecti  recClippedBrush;
        if (!pBrush->ClipBrush(recClippedBrush))
            return;

        // Clip the brush against the world
        recClippedBrush.Shift(m_iX - pBrush->GetBrushSize() / 2, m_iY - pBrush->GetBrushSize() / 2);
        if (!Editor.GetWorld().ClipRect(recClippedBrush, GRID_SPACE))
            return;

        // Get the region
        pRegion = new SFoliageVertexEntry[recClippedBrush.GetArea()];
        if (pRegion == NULL)
            EX_ERROR(_T("Failed to allocate region"));

        if (!Editor.GetWorld().GetRegion(WORLD_VERT_FOLIAGE_MAP, recClippedBrush, pRegion, le_foliageLayer))
            EX_ERROR(_T("Failed to retrieve region"));

        // Perform the operation
        recClippedBrush.Shift(-(m_iX - pBrush->GetBrushSize() / 2), -(m_iY - pBrush->GetBrushSize() / 2));
        float fScale(fFrameTime * le_foliageBrushStrength / 500.0f);
        switch (le_foliageMode)
        {
        case FOLIAGE_DENSITY:
            AddFoliageDensity(pRegion, recClippedBrush, *pBrush, m_bInverse ? le_foliageDensity2 : le_foliageDensity, fScale);
            break;

        case FOLIAGE_SIZE:
            AddFoliageSize(pRegion, recClippedBrush, *pBrush,
                m_bInverse ? le_foliageSizeWidth2 : le_foliageSizeWidth,
                m_bInverse ? le_foliageSizeHeight2 : le_foliageSizeHeight,
                m_bInverse ? le_foliageSizeRandWidth2 : le_foliageSizeRandWidth,
                m_bInverse ? le_foliageSizeRandHeight2 : le_foliageSizeRandHeight,
                fScale);
            break;

        case FOLIAGE_SCALE:
            AddFoliageScale(pRegion, recClippedBrush, *pBrush, m_bInverse ? le_foliageScale2 : le_foliageScale, fScale / 16.0f);
            break;

        case FOLIAGE_SMOOTH:
            FoliageSmooth(pRegion, recClippedBrush, *pBrush, fScale);
            break;

        case FOLIAGE_AUTO:
            FoliageAuto(pRegion, recClippedBrush, *pBrush,
                m_bInverse ? LERP(float(le_foliageAuto2), float(le_foliageDensity), float(le_foliageDensity2)) : LERP(float(le_foliageAuto), float(le_foliageDensity2), float(le_foliageDensity)),
                m_bInverse ? LERP(float(le_foliageAuto2), float(le_foliageSizeWidth), float(le_foliageSizeWidth2)) : LERP(float(le_foliageAuto), float(le_foliageSizeWidth2), float(le_foliageSizeWidth)),
                m_bInverse ? LERP(float(le_foliageAuto2), float(le_foliageSizeHeight), float(le_foliageSizeHeight2)) : LERP(float(le_foliageAuto), float(le_foliageSizeHeight2), float(le_foliageSizeHeight)),
                m_bInverse ? LERP(float(le_foliageAuto2), float(le_foliageSizeRandWidth), float(le_foliageSizeRandWidth2)) : LERP(float(le_foliageAuto), float(le_foliageSizeRandWidth2), float(le_foliageSizeRandWidth)),
                m_bInverse ? LERP(float(le_foliageAuto2), float(le_foliageSizeRandHeight), float(le_foliageSizeRandHeight2)) : LERP(float(le_foliageAuto), float(le_foliageSizeRandHeight2), float(le_foliageSizeRandHeight)),
                m_bInverse ? LERP(float(le_foliageAuto2), float(le_foliageScale), float(le_foliageScale2)) : LERP(float(le_foliageAuto), float(le_foliageScale2), float(le_foliageScale)),
                fScale);
            break;
        }

        // Apply the modified region
        recClippedBrush.Shift(m_iX - pBrush->GetBrushSize() / 2, m_iY - pBrush->GetBrushSize() / 2);
        if (!Editor.GetWorld().SetRegion(WORLD_VERT_FOLIAGE_MAP, recClippedBrush, pRegion, le_foliageLayer))
            EX_ERROR(_T("SetRegion failed"));

        if (le_foliageMode == FOLIAGE_DENSITY)
        {
            for (int y(recClippedBrush.top); y < recClippedBrush.bottom; ++y)
            {
                for (int x(recClippedBrush.left); x < recClippedBrush.right; ++x)
                    Vid.Notify(VID_NOTIFY_FOLIAGE_DENSITY_MODIFIED, x, y, 0, &Editor.GetWorld());
            }
        }
        else
        {
            for (int y(recClippedBrush.top); y < recClippedBrush.bottom; ++y)
            {
                for (int x(recClippedBrush.left); x < recClippedBrush.right; ++x)
                    Vid.Notify(VID_NOTIFY_FOLIAGE_SIZE_MODIFIED, x, y, 0, &Editor.GetWorld());
            }
        }

        delete[] pRegion;
    }
    catch (CException &ex)
    {
        if (pRegion != NULL)
            delete[] pRegion;

        ex.Process(_T("CFoliageTool::PaintFoliageVertex() - "), NO_THROW);
    }
}


/*====================
  CFoliageTool::ApplyFoliageTexture
  ====================*/
void    CFoliageTool::ApplyFoliageTexture(SFoliageTile *pRegion, const CRecti &recArea, const CBrush &brush, ResHandle hTexture, int iNumCrossQuads, byte yFlags)
{
    int iBrushSize(brush.GetBrushSize());

    int iRegionIndex(0);
    for (int y(0); y < recArea.GetHeight(); ++y)
    {
        for (int x(0); x < recArea.GetWidth(); ++x)
        {
            if (brush[BRUSH_INDEX(x, y)] > 0)
            {
                pRegion[iRegionIndex].iTextureRef = hTexture;
                pRegion[iRegionIndex].yNumCrossQuads = byte(iNumCrossQuads);
                pRegion[iRegionIndex].yFlags = yFlags;
            }

            ++iRegionIndex;
        }
    }
}


/*====================
  CFoliageTool::ApplyFoliageTextureStroked

  Stroked to fix the alpha edge problems
  ====================*/
void    CFoliageTool::ApplyFoliageTextureStroked(SFoliageTile *pRegion, const CRecti &recArea, const CBrush &brush, ResHandle hTexture, int iNumCrossQuads, byte yFlags)
{
    int iBrushSize(brush.GetBrushSize());

    int iRegionIndex(0);
    for (int y(0); y < recArea.GetHeight(); ++y)
    {
        for (int x(0); x < recArea.GetWidth(); ++x)
        {
            if (brush[BRUSH_INDEX(x, y)] > 0 ||
                (recArea.left + x < iBrushSize - 1 && brush[BRUSH_INDEX(x + 1, y)] > 0) ||
                (recArea.top + y < iBrushSize - 1 && brush[BRUSH_INDEX(x, y + 1)] > 0) ||
                (recArea.left + x < iBrushSize - 1 && recArea.top + y < iBrushSize - 1 && brush[BRUSH_INDEX(x + 1, y + 1)] > 0))
            {
                pRegion[iRegionIndex].iTextureRef = hTexture;
                pRegion[iRegionIndex].yNumCrossQuads = byte(iNumCrossQuads);
                pRegion[iRegionIndex].yFlags = yFlags;
            }

            ++iRegionIndex;
        }
    }
}


/*====================
  CFoliageTool::PaintFoliageTile
  ====================*/
void    CFoliageTool::PaintFoliageTile(float fFrameTime)
{
    SFoliageTile *pRegion(NULL);

    try
    {
        CBrush *pBrush(CBrush::GetCurrentBrush());
        if (pBrush == NULL)
            EX_ERROR(_T("No brush selected"));

        if (!Editor.GetWorld().IsInBounds(m_iX, m_iY, TILE_SPACE))
            EX_WARN(_T("Out of bounds coordinate"));

        // Clip against the brush data
        CRecti  recClippedBrush;
        if (!pBrush->ClipBrush(recClippedBrush))
            return;

        if (le_foliageMode == FOLIAGE_AUTO) // Expand brush by one pixel in the negative direction to fix alpha edge problems
        {
            if (recClippedBrush.left > 0)
            {
                recClippedBrush.ShiftX(-1);
                recClippedBrush.StretchX(2);
            }


            if (recClippedBrush.top > 0)
            {
                recClippedBrush.ShiftY(-1);
                recClippedBrush.StretchY(2);
            }
        }

        // Clip the brush against the world
        recClippedBrush.Shift(m_iX - pBrush->GetBrushSize() / 2, m_iY - pBrush->GetBrushSize() / 2);
        if (!Editor.GetWorld().ClipRect(recClippedBrush, TILE_SPACE))
            return;

        // Get the region
        pRegion = new SFoliageTile[recClippedBrush.GetArea()];
        if (pRegion == NULL)
            EX_ERROR(_T("Failed to allocate region"));

        if (!Editor.GetWorld().GetRegion(WORLD_TILE_FOLIAGE_MAP, recClippedBrush, pRegion, le_foliageLayer))
            EX_ERROR(_T("Failed to retrieve region"));

        // Perform the operation
        recClippedBrush.Shift(-(m_iX - pBrush->GetBrushSize() / 2), -(m_iY - pBrush->GetBrushSize() / 2));

        byte yFlags(0);

        if (le_foliageParallel)
            yFlags |= F_PARALLEL;
        else if (le_foliageCamera)
            yFlags |= F_CAMERA;

        if (le_foliageMode == FOLIAGE_AUTO)
            ApplyFoliageTextureStroked(pRegion, recClippedBrush, *pBrush, m_hTexture, le_foliageCrossQuads, yFlags);
        else
            ApplyFoliageTexture(pRegion, recClippedBrush, *pBrush, m_hTexture, le_foliageCrossQuads, yFlags);

        // Apply the modified region
        recClippedBrush.Shift(m_iX - pBrush->GetBrushSize() / 2, m_iY - pBrush->GetBrushSize() / 2);
        if (!Editor.GetWorld().SetRegion(WORLD_TILE_FOLIAGE_MAP, recClippedBrush, pRegion, le_foliageLayer))
            EX_ERROR(_T("SetRegion failed"));

        for (int y(recClippedBrush.top); y < recClippedBrush.bottom; ++y)
        {
            for (int x(recClippedBrush.left); x < recClippedBrush.right; ++x)
                Vid.Notify(VID_NOTIFY_FOLIAGE_TEXTURE_MODIFIED, x, y, 0, &Editor.GetWorld());
        }

        delete[] pRegion;
    }
    catch (CException &ex)
    {
        if (pRegion != NULL)
            delete[] pRegion;

        ex.Process(_T("CFoliageTool::PaintFoliageTile() - "), NO_THROW);
    }
}


/*====================
  CFoliageTool::Frame
 ====================*/
void    CFoliageTool::Frame(float fFrameTime)
{
    CalcToolProperties();

    if (m_bWorking && m_iX != -1 && m_iY != -1)
    {
        switch (le_foliageMode)
        {
        case FOLIAGE_TEXTURE:
            PaintFoliageTile(fFrameTime);
            break;

        case FOLIAGE_DENSITY:
        case FOLIAGE_SIZE:
        case FOLIAGE_SCALE:
        case FOLIAGE_SMOOTH:
            PaintFoliageVertex(fFrameTime);
            break;

        case FOLIAGE_AUTO:
            PaintFoliageTile(fFrameTime);
            PaintFoliageVertex(fFrameTime);
            break;
        }
    }
}


/*====================
  DrawInfoString
  ====================*/
static void     DrawInfoString(const tstring &sString, int &iLine, CFontMap *pFontMap, ResHandle hFont)
{
    float fWidth(pFontMap->GetStringWidth(sString));
    Draw2D.SetColor(0.0f, 0.0f, 0.0f);
    Draw2D.String(Draw2D.GetScreenW() - fWidth - 3.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() * (iLine + 1) - 1.0f, fWidth, pFontMap->GetMaxHeight(), sString, hFont);
    Draw2D.SetColor(1.0f, 1.0f, 1.0f);
    Draw2D.String(Draw2D.GetScreenW() - fWidth - 4.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() * (iLine + 1) - 2.0f, fWidth, pFontMap->GetMaxHeight(), sString, hFont);
    ++iLine;
}


/*====================
  CFoliageTool::Draw
  ====================*/
void    CFoliageTool::Draw()
{
    if (le_foliageDrawBrushCoords)
    {
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFont));

        Draw2D.SetColor(0.0f, 0.0f, 0.0f);
        Draw2D.String(4.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 1.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
        Draw2D.SetColor(1.0f, 1.0f, 1.0f);
        Draw2D.String(3.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 2.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
    }

    if (le_foliageDrawInfo)
    {
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFont));

        CWorld &oWorld(Editor.GetWorld());
        const float fTileSize(float(oWorld.GetScale()));

        int iTileX(oWorld.GetTileFromCoord(m_v3EndPos.x));
        int iTileY(oWorld.GetTileFromCoord(m_v3EndPos.y));

        float fLerps[2] =
        {
            FRAC(m_v3EndPos.x / fTileSize),
            FRAC(m_v3EndPos.y / fTileSize)
        };

        int iLine(0);

        //
        // Size, Rand Size, Scale
        //

        CVec3f v3Sizes[4] =
        {
            oWorld.GetFoliageSize(iTileX, iTileY, le_foliageLayer),
            oWorld.GetFoliageSize(iTileX + 1, iTileY, le_foliageLayer),
            oWorld.GetFoliageSize(iTileX, iTileY + 1, le_foliageLayer),
            oWorld.GetFoliageSize(iTileX + 1, iTileY + 1, le_foliageLayer)
        };

        CVec3f v3Variances[4] =
        {
            oWorld.GetFoliageVariance(iTileX, iTileY, le_foliageLayer),
            oWorld.GetFoliageVariance(iTileX + 1, iTileY, le_foliageLayer),
            oWorld.GetFoliageVariance(iTileX, iTileY + 1, le_foliageLayer),
            oWorld.GetFoliageVariance(iTileX + 1, iTileY + 1, le_foliageLayer)
        };

        float fWidths[4] =
        {
            v3Sizes[0].x,
            v3Sizes[1].x,
            v3Sizes[2].x,
            v3Sizes[3].x
        };

        float fPCFWidth(PCF(fLerps, fWidths));

        float fHeights[4] =
        {
            v3Sizes[0].y,
            v3Sizes[1].y,
            v3Sizes[2].y,
            v3Sizes[3].y
        };
        float fPCFHeight(PCF(fLerps, fHeights));

        float fRandWidths[4] =
        {
            v3Variances[0].x,
            v3Variances[1].x,
            v3Variances[2].x,
            v3Variances[3].x
        };
        float fPCFRandWidth(PCF(fLerps, fRandWidths));

        float fRandHeights[4] =
        {
            v3Variances[0].y,
            v3Variances[1].y,
            v3Variances[2].y,
            v3Variances[3].y
        };
        float fPCFRandHeight(PCF(fLerps, fRandHeights));

        float fScales[4] =
        {
            v3Sizes[0].z,
            v3Sizes[1].z,
            v3Sizes[2].z,
            v3Sizes[3].z
        };
        float fPCFScale(PCF(fLerps, fScales));

        DrawInfoString(_T("Rand Size: ") + XtoA(fPCFRandWidth, 0, 0, 1) + _T(", ") + XtoA(fPCFRandHeight, 0, 0, 1), iLine, pFontMap, m_hFont);
        DrawInfoString(_T("Size: ") + XtoA(fPCFWidth, 0, 0, 1) + _T(", ") + XtoA(fPCFHeight, 0, 0, 1), iLine, pFontMap, m_hFont);
        DrawInfoString(_T("Scale: ") + XtoA(fPCFScale, 0, 0, 1), iLine, pFontMap, m_hFont);

        //
        // Density
        //

        float fDensity[4] =
        {
            oWorld.GetFoliageDensity(iTileX, iTileY, le_foliageLayer),
            oWorld.GetFoliageDensity(iTileX + 1, iTileY, le_foliageLayer),
            oWorld.GetFoliageDensity(iTileX, iTileY + 1, le_foliageLayer),
            oWorld.GetFoliageDensity(iTileX + 1, iTileY + 1, le_foliageLayer)
        };

        float fPCFDensity(PCF(fLerps, fDensity));

        DrawInfoString(_T("Density: ") + XtoA(fPCFDensity, 0, 0, 1), iLine, pFontMap, m_hFont);

        DrawInfoString(_T("CrossQuads: ") + XtoA(oWorld.GetFoliageCrossQuads(iTileX, iTileY, le_foliageLayer)), iLine, pFontMap, m_hFont);

        DrawInfoString(_T("Material: ") + Filename_StripPath(g_ResourceManager.GetPath(oWorld.GetFoliageMaterial(iTileX, iTileY, le_foliageLayer))), iLine, pFontMap, m_hFont);

        DrawInfoString(_T("Texture: ") + Filename_StripPath(g_ResourceManager.GetPath(oWorld.GetFoliageTexture(iTileX, iTileY, le_foliageLayer))), iLine, pFontMap, m_hFont);

        DrawInfoString(_T("Layer ") + XtoA(le_foliageLayer + 1), iLine, pFontMap, m_hFont);
    }
}


/*====================
  CFoliageTool::Render
  ====================*/
void    CFoliageTool::Render()
{
    if (!le_foliageDrawBrushInfluence || m_iX < 0 || m_iY < 0)
        return;

    SSceneFaceVert poly[1024];
    MemManager.Set(poly, 0, sizeof(poly));
    int p = 0;

    if (le_foliageMode != FOLIAGE_TEXTURE)
    {
        CBrush *pBrush = CBrush::GetCurrentBrush();
        float fTileSize = Editor.GetWorld().GetScale();

        if (!pBrush)
            return;

        int iX = m_iX, iY = m_iY;

        for (int y = 0; y < pBrush->GetBrushSize() - 1; ++y)
        {
            for (int x = 0; x < pBrush->GetBrushSize() - 1; ++x)
            {
                if (p >= 1024) // restart batch if we overflow
                {
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
                    MemManager.Set(poly, 0, sizeof(poly));
                    p = 0;
                }

                int i = x + y * pBrush->GetBrushSize();
                int dX = iX + x - pBrush->GetBrushSize() / 2;
                int dY = iY + y - pBrush->GetBrushSize() / 2;
                if (!Editor.GetWorld().IsInBounds(dX, dY, GRID_SPACE))
                    continue;

                // left
                if (dY < Editor.GetWorld().GetGridHeight() - 1)
                {
                    byte alpha0 = CLAMP(INT_FLOOR((*pBrush)[i] * le_foliageBrushInfluenceAlpha), 0, 255);
                    byte alpha1 = CLAMP(INT_FLOOR((*pBrush)[i + pBrush->GetBrushSize()] * le_foliageBrushInfluenceAlpha), 0, 255);

                    if (alpha0 || alpha1)
                    {
                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, dY);
                        SET_VEC4(poly[p].col, 0, 255, 0, alpha0);
                        ++p;

                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, (dY + 1));
                        SET_VEC4(poly[p].col, 0, 255, 0, alpha1);
                        ++p;
                    }
                }

                if (p >= 1024) // restart batch if we overflow
                {
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
                    MemManager.Set(poly, 0, sizeof(poly));
                    p = 0;
                }

                // top
                if (dX < Editor.GetWorld().GetGridWidth() - 1)
                {
                    byte alpha0 = INT_FLOOR((*pBrush)[i] * le_foliageBrushInfluenceAlpha);
                    byte alpha1 = INT_FLOOR((*pBrush)[i + 1] * le_foliageBrushInfluenceAlpha);

                    if (alpha0 || alpha1)
                    {
                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, dY);
                        SET_VEC4(poly[p].col, 0, 255, 0, alpha0);
                        ++p;

                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), dY);
                        SET_VEC4(poly[p].col, 0, 255, 0, alpha1);
                        ++p;
                    }
                }
            }
        }

        if (p > 0)
            SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
    }
    else
    {
        CBrush *pBrush = CBrush::GetCurrentBrush();
        float fTileSize = Editor.GetWorld().GetScale();

        if (!pBrush)
            return;

        int iX = m_iX, iY = m_iY;

        for (int y = 0; y < pBrush->GetBrushSize(); ++y)
        {
            for (int x = 0; x < pBrush->GetBrushSize(); ++x)
            {
                int i = x + y * pBrush->GetBrushSize();

                // left
                if ((x > 0 && (*pBrush)[i] && !(*pBrush)[i - 1]) || (x == 0 && (*pBrush)[i]))
                {
                    int dX = iX + x - pBrush->GetBrushSize()/2;
                    int dY = iY + y - pBrush->GetBrushSize()/2;

                    if (Editor.GetWorld().IsInBounds(dX, dY, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX, dY + 1, GRID_SPACE))
                    {
                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, dY);
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;

                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, (dY + 1));
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;
                    }
                }

                // right
                if ((x < pBrush->GetBrushSize() - 1 && (*pBrush)[i] && !(*pBrush)[i + 1]) || (x == pBrush->GetBrushSize() - 1 && (*pBrush)[i]))
                {
                    int dX = iX + x - pBrush->GetBrushSize()/2;
                    int dY = iY + y - pBrush->GetBrushSize()/2;

                    if (Editor.GetWorld().IsInBounds(dX + 1, dY, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX + 1, dY + 1, GRID_SPACE))
                    {
                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), dY);
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;

                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), (dY+ 1));
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;
                    }
                }

                // top
                if ((y > 0 && (*pBrush)[i] && !(*pBrush)[i - pBrush->GetBrushSize()]) || (y == 0 && (*pBrush)[i]))
                {
                    int dX = iX + x - pBrush->GetBrushSize()/2;
                    int dY = iY + y - pBrush->GetBrushSize()/2;

                    if (Editor.GetWorld().IsInBounds(dX, dY, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX + 1, dY, GRID_SPACE))
                    {
                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, dY);
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;

                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), dY);
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;
                    }
                }

                // bottom
                if ((y < pBrush->GetBrushSize() - 1 && (*pBrush)[i] && !(*pBrush)[i+pBrush->GetBrushSize()]) || (y == pBrush->GetBrushSize() - 1 && (*pBrush)[i]))
                {
                    int dX = iX + x - pBrush->GetBrushSize()/2;
                    int dY = iY + y - pBrush->GetBrushSize()/2;

                    if (Editor.GetWorld().IsInBounds(dX + 1, dY + 1, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX, dY + 1, GRID_SPACE))
                    {
                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), (dY+ 1));
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;

                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, (dY + 1));
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;
                    }
                }
            }
        }

        if (p > 0)
            SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
    }
}


/*--------------------
  cmdFoliageMode
  --------------------*/
UI_VOID_CMD(FoliageMode, 1)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: foliagemode texture|density|size|scale") << newl;
        return;
    }

    tstring sValue(vArgList[0]->Evaluate());

    if (sValue == _T("texture"))
    {
        le_foliageMode = FOLIAGE_TEXTURE;
        FoliageMode.Trigger(_T("Texture"));
        return;
    }
    else if (sValue == _T("density"))
    {
        le_foliageMode = FOLIAGE_DENSITY;
        FoliageMode.Trigger(_T("Density"));
        return;
    }
    else if (sValue == _T("size"))
    {
        le_foliageMode = FOLIAGE_SIZE;
        FoliageMode.Trigger(_T("Size"));
        return;
    }
    else if (sValue == _T("scale"))
    {
        le_foliageMode = FOLIAGE_SCALE;
        FoliageMode.Trigger(_T("Scale"));
        return;
    }
    else if (sValue == _T("smooth"))
    {
        le_foliageMode = FOLIAGE_SMOOTH;
        FoliageMode.Trigger(_T("Smooth"));
        return;
    }
    else if (sValue == _T("auto"))
    {
        le_foliageMode = FOLIAGE_AUTO;
        FoliageMode.Trigger(_T("Lazy"));
        return;
    }
}
