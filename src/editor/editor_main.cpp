// (C)2005 S2 Games
// editor_main.cpp
//
// World Editor
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"
#include "c_editor.h"
#include "c_toolbox.h"
#include "i_tool.h"

#include "../k2/c_hostclient.h"
#include "../k2/c_vid.h"
#include "../k2/c_input.h"
#include "../k2/c_action.h"
#include "../k2/c_inputstate.h"
#include "../k2/c_actionregistry.h"
#include "../k2/c_buffer.h"
#include "../k2/c_brush.h"
#include "../k2/c_bitmap.h"
#include "../k2/c_function.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_world.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_clientgamelib.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/i_listwidget.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
UI_TRIGGER(BrushImage);

CEditor Editor;

EXTERN_CVAR_FLOAT(le_camFov);

CVAR_FLOAT  (le_camPitch,       -56.0f);
CVAR_FLOAT  (le_camDistance,    1650.0f);
//=============================================================================

/*--------------------
  Center
  --------------------*/
CMD(Center)
{
    CVec3f v3LookAt;
    if (Editor.GetLookAtPoint(v3LookAt))
    {
        CVec3f v3Angles(le_camPitch, 0.0f, 0.0f);

        Editor.SetCameraAngles(v3Angles);
        Editor.SetCameraPosition(v3LookAt - M_GetForwardVecFromAngles(v3Angles) * le_camDistance);
    }
    
    return true;
}

UI_VOID_CMD(Center, 0)
{
    cmdCenter();
}


/*--------------------
  MinimapLeftClick
  --------------------*/
CMD(MinimapLeftClick)
{
    if (vArgList.size() < 2)
        return false;

    CVec3f v3OldPosition(Editor.GetCamera().GetOrigin());

    float fOldHeight(v3OldPosition.z - Editor.GetWorld().GetTerrainHeight(v3OldPosition.x, v3OldPosition.y));

    CVec3f v3NewPosition;
    v3NewPosition.x = AtoF(vArgList[0]) * Editor.GetWorld().GetWorldWidth();
    v3NewPosition.y = AtoF(vArgList[1]) * Editor.GetWorld().GetWorldHeight();
    v3NewPosition.z = Editor.GetWorld().GetTerrainHeight(v3NewPosition.x, v3NewPosition.y) + fOldHeight;

    Editor.SetCameraPosition(CVec3f(v3NewPosition));

    return true;
}


/*--------------------
  NewWorld
  --------------------*/
CMD(NewWorld)
{
    if (vArgList.size() < 3)
    {
        Console << _T("syntax: NewWorld <worldname> <size> <scale> and optional <texel density> <texture scale>") << newl
                << _T("Valid sizes are 1 - ") << MAX_WORLD_SIZE << newl;
        return false;
    }

    if (vArgList[0].find(_T("/")) != tstring::npos)
    {
        Console << _T("No slashes allowed in world names") << newl;
        return false;
    }

    bool bRetValue(0);

    if (vArgList.size() == 4)
        bRetValue = Editor.GetWorld().New(vArgList[0], AtoI(vArgList[1]), AtoF(vArgList[2]), AtoI(vArgList[3]));
    else if (vArgList.size() >= 5)
        bRetValue = Editor.GetWorld().New(vArgList[0], AtoI(vArgList[1]), AtoF(vArgList[2]), AtoI(vArgList[3]), AtoF(vArgList[4]));
    else
        bRetValue = Editor.GetWorld().New(vArgList[0], AtoI(vArgList[1]), AtoF(vArgList[2]));

    if (bRetValue)
        Editor.SetNullTiles();

    return bRetValue;
}

UI_CMD(NewWorld, 3)
{
    if (vArgList[0]->Evaluate().find(_T("/")) != tstring::npos)
        return _T("No slashes allowed in world names");

    if (AtoI(vArgList[1]->Evaluate()) < 2 || AtoI(vArgList[1]->Evaluate()) > MAX_WORLD_SIZE)
        return _T("Invalid size: Valid sizes are 1 - ") + XtoA(MAX_WORLD_SIZE);

    bool bResult(false);

    if (vArgList.size() == 4)
        bResult = Editor.GetWorld().New(vArgList[0]->Evaluate(), AtoI(vArgList[1]->Evaluate()), AtoF(vArgList[2]->Evaluate()), AtoI(vArgList[3]->Evaluate()));
    else if (vArgList.size() >= 5)
        bResult = Editor.GetWorld().New(vArgList[0]->Evaluate(), AtoI(vArgList[1]->Evaluate()), AtoF(vArgList[2]->Evaluate()), AtoI(vArgList[3]->Evaluate()), AtoF(vArgList[4]->Evaluate()));
    else
        bResult = Editor.GetWorld().New(vArgList[0]->Evaluate(), AtoI(vArgList[1]->Evaluate()), AtoF(vArgList[2]->Evaluate()));

    if (bResult)
        Editor.SetNullTiles();

    if (bResult)
        return _T("");
    else
        return _T("Error creating world!");
}


/*--------------------
  Brush
  --------------------*/
CMD(Brush)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: brush <id>") << newl;
        return false;
    }

    CBrush::SelectBrush(AtoI(vArgList[0]));
    BrushImage.Trigger(CBrush::GetCurrentBrush()->GetFilename());
    return true;
}


/*--------------------
  SetBrush
  --------------------*/
UI_VOID_CMD(SetBrush, 1)
{
    CBrush::SelectBrush(AtoI(vArgList[0]->Evaluate()));
    BrushImage.Trigger(CBrush::GetCurrentBrush()->GetFilename());
}


/*--------------------
  cmdRayTrace
  --------------------*/
CMD(RayTrace)
{
    // Determine size of image to produce
    int iWidth(160);
    int iHeight(120);
    if (vArgList.size() >= 2)
    {
        iWidth = AtoI(vArgList[0]);
        iHeight = AtoI(vArgList[1]);
    }
    else if (!vArgList.empty())
    {
        iWidth = AtoI(vArgList[0]);
        iHeight = INT_ROUND(iWidth / Vid.GetAspect());
    }
    iWidth = CLAMP(iWidth, 10, 32768);
    iHeight = CLAMP(iHeight, 10, 32768);

    Console << _T("Tracing scene at a resolution of ") << iWidth << _T("x") << iHeight << newl;

    float fRatioX = Vid.GetScreenW() / static_cast<float>(iWidth);
    float fRatioY = Vid.GetScreenH() / static_cast<float>(iHeight);

    CBitmap bmp;
    bmp.Alloc(iWidth, iHeight, BITMAP_RGBA);

    uint uiMsec(K2System.Milliseconds());

    CVec4f  colors[] =
    {
        CVec4f(1.0f, 0.5f, 0.5f, 1.0f),
        CVec4f(0.0f, 1.0f, 0.5f, 1.0f),
        CVec4f(0.5f, 0.5f, 1.0f, 1.0f),
        CVec4f(0.5f, 1.0f, 1.0f, 1.0f),
        CVec4f(1.0f, 0.5f, 1.0f, 1.0f),
        CVec4f(1.0f, 1.0f, 0.5f, 1.0f)
    };

    CWorld *pWorld(&Editor.GetWorld());
    CCamera *pCamera(&Editor.GetCamera());
    bool bTraceBox(vArgList.size() > 2);
    float fBoxSize(bTraceBox ? AtoF(vArgList[2]) : 0.0f);
    float fTraceDist(vArgList.size() > 3 ? AtoF(vArgList[3]) : FAR_AWAY);
    CBBoxf bbBounds(CVec3f(-fBoxSize, -fBoxSize, -fBoxSize), CVec3f(fBoxSize, fBoxSize, fBoxSize));
    int iIgnoreSurface(0);

    if (fBoxSize == 0.0f)
        iIgnoreSurface |= SURF_HULL;

    for (int y(0); y < iHeight; ++y)
    {
        for (int x(0); x < iWidth; ++x)
        {
            // Set up the trace
            CVec3f v3Dir(pCamera->ConstructRay(x * fRatioX, y * fRatioY));
            CVec3f v3End(M_PointOnLine(pCamera->GetOrigin(), v3Dir, fTraceDist));
            STraceInfo trace;

            // Perform the trace
            if (bTraceBox)
                pWorld->TraceBox(trace, pCamera->GetOrigin(), v3End, bbBounds, iIgnoreSurface);
            else
                pWorld->TraceLine(trace, pCamera->GetOrigin(), v3End, iIgnoreSurface);

            // No hit
            if (trace.fFraction >= 1.0f)
            {
                bmp.SetPixel4b(x, y, 255, 0, 0, 255);
                //--x;
                continue;
            }

            float fDot(DotProduct(-v3Dir, trace.plPlane.v3Normal));
            if (fDot < 0.0f)
            {
                bmp.SetPixel4b(x, y, 255, 0, 255, 255);
            }
            else if (trace.uiSurfaceIndex != INVALID_INDEX)
            {
                float fLight(CLAMP(fDot, 0.0f, 1.0f));
                CVec4f  v4Color(colors[trace.uiSurfaceIndex % 6]);
                bmp.SetPixel4b(x, y, byte(v4Color[R] * fLight * 255), byte(v4Color[G] * fLight * 255), byte(v4Color[B] * fLight * 255), 255);
            }
            else if (trace.uiEntityIndex != INVALID_INDEX)
            {
                float fLight(CLAMP(fDot, 0.0f, 1.0f));
                CVec4f  v4Color(colors[trace.uiEntityIndex % 6]);
                bmp.SetPixel4b(x, y, byte(v4Color[R] * fLight * 255), byte(v4Color[G] * fLight * 255), byte(v4Color[B] * fLight * 255), 255);
            }
            else
            {
                byte yLight(static_cast<byte>(MAX(fDot, 0.0f) * 255));
                bmp.SetPixel4b(x, y, yLight, yLight, (trace.uiSurfFlags & SURF_TERRAIN) ? yLight : 0, 255);
            }
        }
    }

    Console << _T("Raytrace took ") << XtoA(MsToSec(K2System.Milliseconds() - uiMsec), 0, 0, 3) << _T(" seconds") << newl;

    tstring sFilename(FileManager.GetNextFileIncrement(4, _T("~/raytrace"), _T("png")));
    bmp.WritePNG(sFilename);
    bmp.Free();
    return true;
}


//=============================================================================
// Input States
//=============================================================================
INPUT_STATE_BOOL(TurnLeft);
INPUT_STATE_BOOL(TurnRight);
INPUT_STATE_BOOL(PrimaryButton);
INPUT_STATE_BOOL(SecondaryButton);
INPUT_STATE_BOOL(ModifierButton);
INPUT_STATE_BOOL(ModifierShift);
INPUT_STATE_BOOL(ModifierCtrl);
INPUT_STATE_BOOL(ModifierAlt);
//=============================================================================


//=============================================================================
// Actions
//=============================================================================

/*--------------------
  CameraPitch
  --------------------*/
ACTION_AXIS(CameraPitch)
{
    Editor.AdjustCameraPitch(fDelta);
}


/*--------------------
  CameraYaw
  --------------------*/
ACTION_AXIS(CameraYaw)
{
    Editor.AdjustCameraYaw(fDelta);
}


/*--------------------
  MoveIn
  --------------------*/
ACTION_IMPULSE(MoveIn)
{
    Editor.ShiftCamera(1.0f);
}


/*--------------------
  MoveOut
  --------------------*/
ACTION_IMPULSE(MoveOut)
{
    Editor.ShiftCamera(-1.0f);
}


/*--------------------
  ToolPrimary
  --------------------*/
ACTION_BUTTON(ToolPrimary)
{
    try
    {
        ITool *pTool(ToolBox.GetCurrentTool());
        if (pTool == NULL)
            return;

        if (fValue)
            pTool->PrimaryDown();
        else
            pTool->PrimaryUp();
    }
    catch (CException &ex)
    {
        ex.Process(_T("ACTION(ToolPrimary) - "), NO_THROW);
    }
}


/*--------------------
  ToolSecondary
  --------------------*/
ACTION_BUTTON(ToolSecondary)
{
    try
    {
        if (Editor.IsRulerActive())
        {
            Editor.RulerPointCreate();
            return;
        }

        ITool *pTool(ToolBox.GetCurrentTool());
        if (pTool == NULL)
            return;

        if (fValue)
            pTool->SecondaryDown();
        else
            pTool->SecondaryUp();
    }
    catch (CException &ex)
    {
        ex.Process(_T("ACTION(ToolSecondary) - "), NO_THROW);
    }
}


/*--------------------
  ToolTertiary
  --------------------*/
ACTION_BUTTON(ToolTertiary)
{
    try
    {
        ITool *pTool(ToolBox.GetCurrentTool());
        if (pTool == NULL)
            return;

        if (fValue)
            pTool->TertiaryDown();
        else
            pTool->TertiaryUp();
    }
    catch (CException &ex)
    {
        ex.Process(_T("ACTION(ToolTertiary) - "), NO_THROW);
    }
}


/*--------------------
  ToolQuaternary
  --------------------*/
ACTION_BUTTON(ToolQuaternary)
{
    try
    {
        ITool *pTool(ToolBox.GetCurrentTool());
        if (pTool == NULL)
            return;

        if (fValue)
            pTool->QuaternaryDown();
        else
            pTool->QuaternaryUp();
    }
    catch (CException &ex)
    {
        ex.Process(_T("ACTION(ToolQuaternary) - "), NO_THROW);
    }
}


/*--------------------
  Cancel
  --------------------*/
ACTION_IMPULSE(Cancel)
{
    ITool *pTool(ToolBox.GetCurrentTool());
    if (pTool != NULL)
        pTool->Cancel();
}


/*--------------------
  Delete
  --------------------*/
ACTION_IMPULSE(Delete)
{
    ITool *pTool(ToolBox.GetCurrentTool());
    if (pTool != NULL)
        pTool->Delete();
}


/*--------------------
  ToolModifier1
  --------------------*/
ACTION_BUTTON(ToolModifier1)
{
    ITool *pTool(ToolBox.GetCurrentTool());
    if (pTool != NULL)
    {
        if (fValue)
            pTool->SetModifier1(true);
        else
            pTool->SetModifier1(false);
    }
}


/*--------------------
  ToolModifier2
  --------------------*/
ACTION_BUTTON(ToolModifier2)
{
    ITool *pTool(ToolBox.GetCurrentTool());
    if (pTool != NULL)
    {
        if (fValue)
            pTool->SetModifier2(true);
        else
            pTool->SetModifier2(false);
    }
}


/*--------------------
  ToolModifier3
  --------------------*/
ACTION_BUTTON(ToolModifier3)
{
    ITool *pTool(ToolBox.GetCurrentTool());
    if (pTool != NULL)
    {
        if (fValue)
            pTool->SetModifier3(true);
        else
            pTool->SetModifier3(false);
    }
}


/*--------------------
  PathSetStart
  --------------------*/
ACTION_IMPULSE(PathSetStart)
{
    Editor.PathSetStart();
}


/*--------------------
  PathSetEnd
  --------------------*/
ACTION_IMPULSE(PathSetEnd)
{
    Editor.PathSetEnd();
}


/*--------------------
  PathClear
  --------------------*/
ACTION_IMPULSE(PathClear)
{
    Editor.PathClear();
}


/*--------------------
  Ruler
  --------------------*/
ACTION_BUTTON_EX(Ruler, ACTION_NOREPEAT)
{
    if (fValue)
    {
        if (fDelta > 0.0f)
            Editor.RulerStart();
    }
    else
    {
        Editor.RulerEnd();
    }
}


/*====================
  CL_Init
  ====================*/
bool    CL_Init(CHostClient *pHostClient)
{
    return Editor.Init(pHostClient);
}


/*====================
  CL_Frame
  ====================*/
void    CL_Frame()
{
    Editor.Frame();
}


/*====================
  CL_Shutdown
  ====================*/
void    CL_Shutdown()
{
    Console << _T("Closing editor...") << newl;
}


/*====================
  InitLibrary
  ====================*/
#ifdef __GNUC__
extern "C" void __attribute__ ((visibility("default"))) InitLibrary(CClientGameLib &GameLib)
#else
void    InitLibrary(CClientGameLib &GameLib)
#endif
{
    GameLib.SetName(_T("Heroes of Newerth - Level Editor"));
    GameLib.SetTypeName(_T("honeditor"));
    GameLib.SetMajorVersion(1);
    GameLib.SetMinorVersion(0);
    GameLib.AssignInitFn(CL_Init);
    GameLib.AssignFrameFn(CL_Frame);
    GameLib.AssignShutdownFn(CL_Shutdown);
}


/*--------------------
  SetTextureScale
  --------------------*/
CMD(SetTextureScale)
{
    if (vArgList.empty())
        return false;

    Editor.SetTextureScale(AtoF(vArgList[0]));
    return true;
}


/*--------------------
  SetFancyName
  --------------------*/
CMD(SetFancyName)
{
    if (vArgList.empty())
        return false;

    Editor.SetFancyName(vArgList[0]);
    return true;
}


/*--------------------
  Load
  --------------------*/
CMD(Load)
{
    if (vArgList.empty())
    {
        Console << _T("Please specify a world name") << newl;
        return false;
    }

    return Editor.LoadWorld(vArgList[0]);
}

UI_VOID_CMD(Load, 1)
{
    Editor.LoadWorld(vArgList[0]->Evaluate());
}


/*--------------------
  Save
  --------------------*/
CMD(Save)
{
    try
    {
        Editor.GetWorld().Save(vArgList.size() > 0 ? vArgList[0] : _T(""));
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSave() - "), NO_THROW);
        return false;
    }
}

UI_VOID_CMD(Save, 0)
{
    cmdSave(vArgList.size() > 0 ? vArgList[0]->Evaluate() : _T(""));
}



/*--------------------
  UpdateMinimapTexture
  --------------------*/
UI_VOID_CMD(UpdateMinimapTexture, 0)
{
    Editor.UpdateMinimapTexture();
}


/*--------------------
  ExportMinimap
  --------------------*/
CMD(ExportMinimap)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: ExportMinimap <filename>") << newl;
        return false;
    }

    CBitmap cMinimap(Editor.GetWorld().GetTileWidth(), Editor.GetWorld().GetTileHeight(), BITMAP_RGB);

    Editor.RenderMinimapBitmap(cMinimap);

    cMinimap.Flip();

    cMinimap.WritePNG(_TS("~/") + vArgList[0]);

    return true;
}


/*--------------------
  SetGlobalScript
  --------------------*/
UI_VOID_CMD(SetGlobalScript, 2)
{
    if (vArgList.size() < 2)
    {
        Console << _T("Syntax: SetGlobalScript <name> <script>") << newl;
        return;
    }

    Editor.GetWorld().AddScript(vArgList[0]->Evaluate(), NormalizeLineBreaks(vArgList[1]->Evaluate(), _T("\n")));
    Editor.UpdateScripts();
}


/*--------------------
  GetGlobalScript
  --------------------*/
UI_CMD(GetGlobalScript, 1)
{
    if (vArgList.size() < 1)
    {
        Console << _T("Syntax: GetGlobalScript <name>") << newl;
        return _T("");
    }

    return NormalizeLineBreaks(Editor.GetWorld().GetScript(vArgList[0]->Evaluate()));
}


/*--------------------
  IsValidScriptName
  --------------------*/
UI_CMD(IsValidScriptName, 1)
{
    if (vArgList.size() > 1)
        return XtoA(false);
    
    tstring sScript(LowerString(vArgList[0]->Evaluate()));

    if (sScript.empty() || sScript.find(_T(" ")) != tstring::npos)
        return XtoA(false);

    return XtoA(true);//XtoA(!(Editor.GetWorld().IsScriptReserved(sScript)));
}


/*--------------------
  ImportFile
  --------------------*/
UI_CMD(ImportFile, 2)
{
    bool bReturn(Editor.GetWorld().ImportFile(vArgList[0]->Evaluate(), vArgList[1]->Evaluate()));

    Editor.UpdateImportedFiles();

    return XtoA(bReturn);
}


/*--------------------
  DeleteImportedFile
  --------------------*/
UI_CMD(DeleteImportedFile, 1)
{
    bool bReturn(Editor.GetWorld().DeleteImportedFile(vArgList[0]->Evaluate()));

    Editor.UpdateImportedFiles();

    return XtoA(bReturn);
}


/*--------------------
  CalculateOcclusionMap
  --------------------*/
CMD(CalculateOcclusionMap)
{
    uint uiMsec(K2System.Milliseconds());

    Editor.GetWorld().CalculateOcclusionMap(vArgList.size() > 0 ? AtoI(vArgList[0]) : 16);

    Console << _T("OcclusionMap calculation took ") << XtoA(MsToSec(K2System.Milliseconds() - uiMsec), 0, 0, 3) << _T(" seconds") << newl;

    Console.Execute(_T("RebuildTerrain"));
    return true;
}


/*--------------------
  TraceTest
  --------------------*/
CMD(TraceTest)
{
    CWorld *pWorld(&Editor.GetWorld());
    CCamera *pCamera(&Editor.GetCamera());

    STraceInfo trace;
    pWorld->TraceLine(trace, pCamera->GetOrigin(), pCamera->GetOrigin() + CVec3f(0.0f, 0.0f, -10000.0f));

    return true;
}


/*--------------------
  SetMinPlayersPerTeam
  --------------------*/
UI_VOID_CMD(SetMinPlayersPerTeam, 1)
{
    Editor.GetWorld().SetMinPlayersPerTeam(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  GetMinPlayersPerTeam
  --------------------*/
UI_CMD(GetMinPlayersPerTeam, 0)
{
    return XtoA(Editor.GetWorld().GetMinPlayersPerTeam());
}


/*--------------------
  SetMaxPlayers
  --------------------*/
UI_VOID_CMD(SetMaxPlayers, 1)
{
    Editor.GetWorld().SetMaxPlayers(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  GetMaxPlayers
  --------------------*/
UI_CMD(GetMaxPlayers, 0)
{
    return XtoA(Editor.GetWorld().GetMaxPlayers());
}


/*--------------------
  PathSetStart
  --------------------*/
CMD(PathSetStart)
{
    Editor.PathSetStart();
    return true;
}


/*--------------------
  PathSetEnd
  --------------------*/
CMD(PathSetEnd)
{
    Editor.PathSetEnd();
    return true;
}


/*--------------------
  PathClear
  --------------------*/
CMD(PathClear)
{
    Editor.PathClear();
    return true;
}


/*--------------------
  PathSpam
  --------------------*/
CMD(PathSpam)
{
    Editor.PathSpam();
    return true;
}


/*--------------------
  Path
  --------------------*/
CMD(Path)
{
    if (vArgList.size() < 4)
        return false;

    CVec2f v2Start(AtoF(vArgList[0]), AtoF(vArgList[1]));
    CVec2f v2End(AtoF(vArgList[2]), AtoF(vArgList[3]));

    Editor.Path(v2Start, v2End);

    return true;
}


/*--------------------
  FixTrees
  --------------------*/
CMD(FixTrees)
{
    CWorld &cWorld(Editor.GetWorld());
    WorldEntList &vEntities(cWorld.GetEntityList());
    WorldEntList_cit citEnd(vEntities.end());

    // Snap to 2 tile grid
    for (WorldEntList_cit cit(vEntities.begin()); cit != citEnd; ++cit)
    {
        CWorldEntity *pWorldEnt(cWorld.GetEntityByHandle(*cit));
        if (pWorldEnt == NULL)
            continue;

        if (pWorldEnt->GetType() != _T("Prop_Tree"))
            continue;

        uint uiEntIndex(pWorldEnt->GetIndex());

        cWorld.UnlinkEntity(uiEntIndex);

        CVec3f v3Position(pWorldEnt->GetPosition());

        v3Position.x = ROUND(v3Position.x / 64.0f) * 64.0f;
        v3Position.y = ROUND(v3Position.y / 64.0f) * 64.0f;
        v3Position.z = Editor.GetWorld().GetTerrainHeight(v3Position.x, v3Position.y);

        pWorldEnt->SetPosition(v3Position);
        pWorldEnt->SetWorldBounds(CBBoxf(-50.0f, 50.0f) + v3Position);
        
        cWorld.LinkEntity(pWorldEnt->GetIndex(), LINK_SURFACE | LINK_MODEL, SURF_PROP);

        map<uint, CWorldEntityEx>::iterator findit(g_WorldEntData.find(uiEntIndex));

        if (findit != g_WorldEntData.end())
        {
            vector<PoolHandle> &vPathBlockers(findit->second.GetPathBlockers());

            vector<PoolHandle>::const_iterator citEnd(vPathBlockers.end());
            for (vector<PoolHandle>::const_iterator cit(vPathBlockers.begin()); cit != citEnd; ++cit)
                cWorld.ClearPath(*cit);

            const vector<CConvexPolyhedron> &cWorldSurfs(pWorldEnt->GetWorldSurfsRef());
            for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
                cWorld.BlockPath(vPathBlockers, NAVIGATION_UNIT, *cit, 24.0f);

            // Hack for tree blockers
            vPathBlockers.push_back(cWorld.BlockPath(NAVIGATION_UNIT, pWorldEnt->GetPosition().xy() - CVec2f(50.0f), 100.0f, 100.0f));
        }
    }

    uiset setRelease;

    // Delete overlaps
    for (WorldEntList_cit citA(vEntities.begin()); citA != citEnd; ++citA)
    {
        CWorldEntity *pWorldEntA(cWorld.GetEntityByHandle(*citA));
        if (pWorldEntA == NULL)
            continue;

        if (pWorldEntA->GetType() != _T("Prop_Tree"))
            continue;

        WorldEntList_cit citB(citA);
        ++citB;

        for (; citB != citEnd; ++citB)
        {
            CWorldEntity *pWorldEntB(cWorld.GetEntityByHandle(*citB));
            if (pWorldEntB == NULL)
                continue;

            if (pWorldEntB->GetType() != _T("Prop_Tree"))
                continue;

            if (IntersectBounds(pWorldEntA->GetWorldBounds(), pWorldEntB->GetWorldBounds()))
                setRelease.insert(pWorldEntB->GetIndex());
        }
    }

    uiset_it itEnd(setRelease.end());
    for (uiset_it it(setRelease.begin()); it != itEnd; ++it)
    {
        cWorld.UnlinkEntity(*it);
        cWorld.DeleteEntity(*it);

        map<uint, CWorldEntityEx>::iterator findit(g_WorldEntData.find(*it));
        if (findit != g_WorldEntData.end())
        {
            vector<PoolHandle> &vPathBlockers(findit->second.GetPathBlockers());

            vector<PoolHandle>::const_iterator citEnd(vPathBlockers.end());
            for (vector<PoolHandle>::const_iterator cit(vPathBlockers.begin()); cit != citEnd; ++cit)
                cWorld.ClearPath(*cit);

            vPathBlockers.clear();

            g_WorldEntData.erase(findit);
        }
    }

    return true;
}


/*--------------------
  CloneTrees
  --------------------*/
CMD(CloneTrees)
{
    CWorld &cWorld(Editor.GetWorld());
    WorldEntList vEntities(cWorld.GetEntityList()); // copy
    WorldEntList_cit citEnd(vEntities.end());

    ResHandle hBadTree(g_ResourceManager.Register(_T("/environment/trees/bt_brown/model.mdf"), RES_MODEL));

    //
    // Delete old bad trees
    //

    uiset setRelease;

    for (WorldEntList_cit cit(vEntities.begin()); cit != citEnd; ++cit)
    {
        CWorldEntity *pWorldEnt(cWorld.GetEntityByHandle(*cit));
        if (pWorldEnt == NULL)
            continue;

        if (pWorldEnt->GetType() != _T("Prop_Tree") || pWorldEnt->GetModelHandle() != hBadTree)
            continue;

        setRelease.insert(pWorldEnt->GetIndex());
    }

    uiset_it itEnd(setRelease.end());
    for (uiset_it it(setRelease.begin()); it != itEnd; ++it)
    {
        cWorld.UnlinkEntity(*it);
        cWorld.DeleteEntity(*it);

        map<uint, CWorldEntityEx>::iterator findit(g_WorldEntData.find(*it));
        if (findit != g_WorldEntData.end())
        {
            vector<PoolHandle> &vPathBlockers(findit->second.GetPathBlockers());

            vector<PoolHandle>::const_iterator citEnd(vPathBlockers.end());
            for (vector<PoolHandle>::const_iterator cit(vPathBlockers.begin()); cit != citEnd; ++cit)
                cWorld.ClearPath(*cit);

            vPathBlockers.clear();

            g_WorldEntData.erase(findit);
        }
    }

    vEntities = cWorld.GetEntityList(); // new copy
    citEnd = vEntities.end();

    //
    // Clone good trees across x and y axis
    //

    for (WorldEntList_cit cit(vEntities.begin()); cit != citEnd; ++cit)
    {
        CWorldEntity *pWorldEnt(cWorld.GetEntityByHandle(*cit));
        if (pWorldEnt == NULL)
            continue;

        if (pWorldEnt->GetType() != _T("Prop_Tree"))
            continue;
        
        uint uiNewEntity(Editor.GetWorld().AllocateNewEntity());
        CWorldEntity *pNewEntity(Editor.GetWorld().GetEntity(uiNewEntity, true));

        // Get pointer again in case the pool reallocated
        CWorldEntity *pOldEntity(cWorld.GetEntityByHandle(*cit));

        pNewEntity->Clone(*pOldEntity);
        
        pNewEntity->SetIndex(uiNewEntity);

        pNewEntity->SetPosition(CVec3f(cWorld.GetWorldWidth() - pOldEntity->GetPosition().x, cWorld.GetWorldWidth() - pOldEntity->GetPosition().y, pOldEntity->GetPosition().z)); 
        pNewEntity->SetModelHandle(hBadTree);

        Editor.GetWorld().LinkEntity(uiNewEntity, LINK_SURFACE|LINK_MODEL, SURF_PROP);

        map<uint, CWorldEntityEx>::iterator findit(g_WorldEntData.find(uiNewEntity));

        if (findit == g_WorldEntData.end())
        {
            g_WorldEntData.insert(pair<uint, CWorldEntityEx>(uiNewEntity, CWorldEntityEx()));
            findit = g_WorldEntData.find(uiNewEntity);
        }

        if (findit != g_WorldEntData.end())
        {
            vector<PoolHandle> &vPathBlockers(findit->second.GetPathBlockers());

            vector<PoolHandle>::const_iterator citEnd(vPathBlockers.end());
            for (vector<PoolHandle>::const_iterator cit(vPathBlockers.begin()); cit != citEnd; ++cit)
                Editor.GetWorld().ClearPath(*cit);

            vPathBlockers.clear();
        
            const vector<CConvexPolyhedron> &cWorldSurfs(pNewEntity->GetWorldSurfsRef());
            for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
                Editor.GetWorld().BlockPath(vPathBlockers, NAVIGATION_UNIT, *cit, 24.0f);

            // Hack for tree blockers
            if (pNewEntity->GetType() == _T("Prop_Tree"))
                vPathBlockers.push_back(Editor.GetWorld().BlockPath(NAVIGATION_UNIT, pNewEntity->GetPosition().xy() - CVec2f(50.0f), 100.0f, 100.0f));
        }
    }

    return true;
}


/*--------------------
  RotateTrees
  --------------------*/
CMD(RotateTrees)
{
    CWorld &cWorld(Editor.GetWorld());
    WorldEntList &vEntities(cWorld.GetEntityList());
    WorldEntList_cit citEnd(vEntities.end());

    // Snap to 2 tile grid
    for (WorldEntList_cit cit(vEntities.begin()); cit != citEnd; ++cit)
    {
        CWorldEntity *pWorldEnt(cWorld.GetEntityByHandle(*cit));
        if (pWorldEnt == NULL)
            continue;

        if (pWorldEnt->GetType() != _T("Prop_Tree"))
            continue;

        uint uiEntIndex(pWorldEnt->GetIndex());

        cWorld.UnlinkEntity(uiEntIndex);

        CVec3f v3Angles(pWorldEnt->GetAngles());
        v3Angles[YAW] = M_Randnum(0.0f, 360.0f);

        pWorldEnt->SetAngles(v3Angles);
        
        cWorld.LinkEntity(pWorldEnt->GetIndex(), LINK_SURFACE | LINK_MODEL, SURF_PROP);

        map<uint, CWorldEntityEx>::iterator findit(g_WorldEntData.find(uiEntIndex));

        if (findit != g_WorldEntData.end())
        {
            vector<PoolHandle> &vPathBlockers(findit->second.GetPathBlockers());

            vector<PoolHandle>::const_iterator citEnd(vPathBlockers.end());
            for (vector<PoolHandle>::const_iterator cit(vPathBlockers.begin()); cit != citEnd; ++cit)
                cWorld.ClearPath(*cit);

            const vector<CConvexPolyhedron> &cWorldSurfs(pWorldEnt->GetWorldSurfsRef());
            for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
                cWorld.BlockPath(vPathBlockers, NAVIGATION_UNIT, *cit, 24.0f);

            // Hack for tree blockers
            vPathBlockers.push_back(cWorld.BlockPath(NAVIGATION_UNIT, pWorldEnt->GetPosition().xy() - CVec2f(50.0f), 100.0f, 100.0f));
        }
    }

    return true;
}


/*--------------------
  ScaleTrees
  --------------------*/
CMD(ScaleTrees)
{
    CWorld &cWorld(Editor.GetWorld());
    WorldEntList &vEntities(cWorld.GetEntityList());
    WorldEntList_cit citEnd(vEntities.end());

    ResHandle hGoodTree(g_ResourceManager.Register(_T("/world/trees/t_2/model.mdf"), RES_MODEL));
    ResHandle hBadTree(g_ResourceManager.Register(_T("/environment/trees/bt_brown/model.mdf"), RES_MODEL));

    // Snap to 2 tile grid
    for (WorldEntList_cit cit(vEntities.begin()); cit != citEnd; ++cit)
    {
        CWorldEntity *pWorldEnt(cWorld.GetEntityByHandle(*cit));
        if (pWorldEnt == NULL)
            continue;

        if (pWorldEnt->GetType() != _T("Prop_Tree"))
            continue;

        uint uiEntIndex(pWorldEnt->GetIndex());

        cWorld.UnlinkEntity(uiEntIndex);

        float fScale(pWorldEnt->GetScale());

        if (pWorldEnt->GetModelHandle() == hGoodTree)
            fScale = M_Randnum(1.8f, 2.0f);
        else if (pWorldEnt->GetModelHandle() == hBadTree)
            fScale = M_Randnum(1.6f, 1.8f);

        pWorldEnt->SetScale(fScale);
        
        cWorld.LinkEntity(pWorldEnt->GetIndex(), LINK_SURFACE | LINK_MODEL, SURF_PROP);

        map<uint, CWorldEntityEx>::iterator findit(g_WorldEntData.find(uiEntIndex));

        if (findit != g_WorldEntData.end())
        {
            vector<PoolHandle> &vPathBlockers(findit->second.GetPathBlockers());

            vector<PoolHandle>::const_iterator citEnd(vPathBlockers.end());
            for (vector<PoolHandle>::const_iterator cit(vPathBlockers.begin()); cit != citEnd; ++cit)
                cWorld.ClearPath(*cit);

            const vector<CConvexPolyhedron> &cWorldSurfs(pWorldEnt->GetWorldSurfsRef());
            for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
                cWorld.BlockPath(vPathBlockers, NAVIGATION_UNIT, *cit, 24.0f);

            // Hack for tree blockers
            vPathBlockers.push_back(cWorld.BlockPath(NAVIGATION_UNIT, pWorldEnt->GetPosition().xy() - CVec2f(50.0f), 100.0f, 100.0f));
        }
    }

    return true;
}


#if 1
#pragma pack(1)
struct SDoodad
{
    uint uiType;
    uint uiVariation;
    float fX;
    float fY;
    float fZ;
    float fRotation; // Radians
    float fScaleX;
    float fScaleY;
    float fScaleZ;
    char szUnknown[14];
}; // 50 bytes
#pragma pack()

#define DOOTYPE(a, b, c, d) (a + (b << 8) + (c << 16) + (d << 24))

/*--------------------
  ImportDoo
  --------------------*/
CMD(ImportDoo)
{
    if (vArgList.size() < 1)
        return false;

    CFileHandle hFile(vArgList[0], FILE_READ);

    uint dwSig(hFile.ReadInt32());
    
    uint ui0(hFile.ReadInt32());
    uint ui1(hFile.ReadInt32());
    
    uint uiNumDoodads(hFile.ReadInt32());

    vector<SDoodad> vDoodads(uiNumDoodads);

    uint uiCount(0);
    while (!hFile.IsEOF() && uiCount < uiNumDoodads)
    {
        hFile.Read((char *)&vDoodads[uiCount], sizeof(SDoodad));
        ++uiCount;
    }

    ResHandle hGoodTree(g_ResourceManager.Register(_T("/world/trees/t_2/model.mdf"), RES_MODEL));
    ResHandle hBadTree(g_ResourceManager.Register(_T("/environment/trees/bt_brown/model.mdf"), RES_MODEL));

    ///*    Removed the "//" if you need any of the below variables
    ui1 = 0;
    ui0 = 0;
    dwSig = 0;
    hBadTree = 0;
    //*/

    for (vector<SDoodad>::iterator it(vDoodads.begin()), itEnd(vDoodads.end()); it != itEnd; ++it)
    {
        if (it->uiType == DOOTYPE('A', 'T', 't', 'r') ||
            it->uiType == DOOTYPE('A', 'T', 't', 'c') ||
            it->uiType == DOOTYPE('N', 'T', 't', 'w') ||
            it->uiType == DOOTYPE('N', 'T', 't', 'c'))
        {
            uint uiNewEntity(Editor.GetWorld().AllocateNewEntity());
            CWorldEntity *pNewEntity(Editor.GetWorld().GetEntity(uiNewEntity, true));

            pNewEntity->SetPosition(it->fX + 8192.0f, it->fY + 8192.0f, it->fZ - 896.0f);
            pNewEntity->SetAngles(CVec3f(0.0f, 0.0f, M_Randnum(0.0f, 360.0f)));
            pNewEntity->SetScale(M_Randnum(1.8f, 2.0f));

            pNewEntity->SetModelHandle(hGoodTree);
            Editor.GetWorld().LinkEntity(uiNewEntity, LINK_SURFACE | LINK_MODEL, SURF_PROP);
        }
    }

    return true;
}

#pragma pack(1)
struct SVertex
{
    ushort unHeightOffset;
    ushort un0;
    byte y0;
    byte y1;
    byte y2;
}; // 7 bytes
#pragma pack()

/*--------------------
  ImportW3E
  --------------------*/
CMD(ImportW3E)
{
    if (vArgList.size() < 1)
        return false;

    CFileHandle hFile(vArgList[0], FILE_READ);

    uint dwSig(hFile.ReadInt32());

    uint ui0(hFile.ReadInt32());
    byte y0(hFile.ReadByte());
    
    uint ui1(hFile.ReadInt32());

    uint uiNumTableSize1(hFile.ReadInt32());
    for (uint ui(0); ui < uiNumTableSize1; ++ui)
        hFile.ReadInt32();

    uint uiNumTableSize2(hFile.ReadInt32());
    for (uint ui(0); ui < uiNumTableSize2; ++ui)
        hFile.ReadInt32();

    uint uiGridWidth(hFile.ReadInt32());
    uint uiGridHeight(hFile.ReadInt32());
    uint uiNumVertexes(uiGridWidth * uiGridHeight);

    float fOffsetX(hFile.ReadFloat());
    float fOffsetY(hFile.ReadFloat());

    ///*    Removed the "//" if you need any of the below variables
    ui1 = 0;
    ui0 = 0;
    dwSig = 0;
    fOffsetY = 0.0f;
    fOffsetX = 0.0f;
    y0 = 0;
    //*/

    vector<SVertex> vVertexes(uiNumVertexes);

    uint uiCount(0);
    while (!hFile.IsEOF() && uiCount < uiNumVertexes)
    {
        hFile.Read((char *)&vVertexes[uiCount], sizeof(SVertex));
        ++uiCount;
    }

    CWorld &cWorld(Editor.GetWorld());

    float *pW3E0(K2_NEW_ARRAY(ctx_Editor, float, uiGridWidth*uiGridHeight));

    for (uint iY(0); iY < uiGridHeight; ++iY)
    {
        for (uint iX(0); iX < uiGridWidth; ++iX)
        {
            int iIndex(iY * uiGridWidth + iX);
            //pW3E0[iIndex] = (vVertexes[iIndex].unHeightOffset - 8192 + (vVertexes[iIndex].y2 & 0xf) * 512) / 4.0f - 1152.0f;
            pW3E0[iIndex] = ((vVertexes[iIndex].y2 & 0xf) * 512) / 4.0f - 1152.0f;
        }
    }

    float *pW3E(K2_NEW_ARRAY(ctx_Editor, float, uiGridWidth*uiGridHeight));

    for (uint iY(0); iY < uiGridHeight; ++iY)
    {
        for (uint iX(0); iX < uiGridWidth; ++iX)
        {
            int iIndex(iY * uiGridWidth + iX);

            float fTotal(0.0f);
            pW3E[iIndex] = 0.0f;

            if (iX != 0 && iY != 0 && iX != uiGridWidth - 1 && iY != uiGridHeight - 1 &&
                vVertexes[iY * uiGridWidth + iX].y0 & BIT(4) &&
                vVertexes[(iY - 1) * uiGridWidth + (iX - 1)].y0 & BIT(4) &&
                vVertexes[(iY + 1) * uiGridWidth + (iX + 1)].y0 & BIT(4) &&
                (vVertexes[(iY - 1) * uiGridWidth + (iX - 1)].y2 & 0xf) != (vVertexes[(iY + 1) * uiGridWidth + (iX + 1)].y2 & 0xf))
            {
                pW3E[iIndex] += MAX(pW3E0[(iY - 1) * uiGridWidth + (iX - 1)], pW3E0[iY * uiGridWidth + iX]);
                pW3E[iIndex] += MAX(pW3E0[(iY + 1) * uiGridWidth + (iX + 1)], pW3E0[iY * uiGridWidth + iX]);
                fTotal += 2.0f;
            }
            else if (iX != 0 && iY != 0 && iX != uiGridWidth - 1 && iY != uiGridHeight - 1 &&
                vVertexes[iY * uiGridWidth + iX].y0 & BIT(4) &&
                vVertexes[(iY + 1) * uiGridWidth + (iX - 1)].y0 & BIT(4) &&
                vVertexes[(iY - 1) * uiGridWidth + (iX + 1)].y0 & BIT(4) &&
                (vVertexes[(iY + 1) * uiGridWidth + (iX - 1)].y2 & 0xf) != (vVertexes[(iY - 1) * uiGridWidth + (iX + 1)].y2 & 0xf))
            {
                pW3E[iIndex] += MAX(pW3E0[(iY + 1) * uiGridWidth + (iX - 1)], pW3E0[iY * uiGridWidth + iX]);
                pW3E[iIndex] += MAX(pW3E0[(iY - 1) * uiGridWidth + (iX + 1)], pW3E0[iY * uiGridWidth + iX]);
                fTotal += 2.0f;
            }
            else if (iX != 0 && iX != uiGridWidth - 1 &&
                vVertexes[iY * uiGridWidth + iX].y0 & BIT(4) &&
                vVertexes[iY * uiGridWidth + (iX - 1)].y0 & BIT(4) &&
                vVertexes[iY * uiGridWidth + (iX + 1)].y0 & BIT(4) &&
                (vVertexes[iY * uiGridWidth + (iX - 1)].y2 & 0xf) != (vVertexes[iY * uiGridWidth + (iX + 1)].y2 & 0xf))
            {
                pW3E[iIndex] += MAX(pW3E0[iY * uiGridWidth + (iX - 1)], pW3E0[iY * uiGridWidth + iX]);
                pW3E[iIndex] += MAX(pW3E0[iY * uiGridWidth + (iX + 1)], pW3E0[iY * uiGridWidth + iX]);
                fTotal += 2.0f;
            }
            else if (iY != 0 && iY != uiGridHeight - 1 &&
                vVertexes[iY * uiGridWidth + iX].y0 & BIT(4) &&
                vVertexes[(iY - 1) * uiGridWidth + iX].y0 & BIT(4) &&
                vVertexes[(iY + 1) * uiGridWidth + iX].y0 & BIT(4) &&
                (vVertexes[(iY - 1) * uiGridWidth + iX].y2 & 0xf) != (vVertexes[(iY + 1) * uiGridWidth + iX].y2 & 0xf))
            {
                pW3E[iIndex] += MAX(pW3E0[(iY - 1) * uiGridWidth + iX], pW3E0[iY * uiGridWidth + iX]);
                pW3E[iIndex] += MAX(pW3E0[(iY + 1) * uiGridWidth + iX], pW3E0[iY * uiGridWidth + iX]);
                fTotal += 2.0f;
            }
            
            
            if (fTotal == 0.0f)
                pW3E[iIndex] = pW3E0[iIndex];
            else
                pW3E[iIndex] /= fTotal;
        }
    }

    CRecti recWorld(0, 0, cWorld.GetGridWidth(), cWorld.GetGridHeight());
    float *pK2(K2_NEW_ARRAY(ctx_Editor, float, recWorld.GetArea()));
    
    float fTileScaleX(float(uiGridWidth - 1) / cWorld.GetTileWidth());
    float fTileScaleY(float(uiGridHeight - 1) / cWorld.GetTileHeight());

    for (int iY(0); iY < cWorld.GetGridHeight(); ++iY)
    {
        for (int iX(0); iX < cWorld.GetGridWidth(); ++iX)
        {
            int iTileX(INT_FLOOR(iX * fTileScaleX));
            int iTileY(INT_FLOOR(iY * fTileScaleY));

            float fFracX(FRAC(iX * fTileScaleX));
            float fFracY(FRAC(iY * fTileScaleY));
            
            if ((FLOAT_EQUALS(fFracX, 0.0f) && FLOAT_EQUALS(fFracY, 0.0f) && iTileX != 0 && iTileY != 0 && (
                    vVertexes[iTileY * uiGridWidth + iTileX].y0 & BIT(4))
                ) ||
                (FLOAT_EQUALS(fFracX, 0.0f) && iTileX != 0 && iTileY != (int)uiGridHeight - 1 && (
                    vVertexes[iTileY * uiGridWidth + iTileX].y0 & BIT(4) &&
                    vVertexes[(iTileY + 1) * uiGridWidth + iTileX].y0 & BIT(4))
                ) ||
                (FLOAT_EQUALS(fFracY, 0.0f) && iTileY != 0 && iTileX != (int)uiGridWidth - 1 && (
                    vVertexes[iTileY * uiGridWidth + iTileX].y0 & BIT(4) &&
                    vVertexes[iTileY * uiGridWidth + iTileX + 1].y0 & BIT(4))
                ) || 
                (iTileX != (int)uiGridWidth - 1 && iTileY != (int)uiGridHeight - 1 && (
                    vVertexes[iTileY * (int)uiGridWidth + iTileX].y0 & BIT(4) &&
                    vVertexes[iTileY * (int)uiGridWidth + iTileX + 1].y0 & BIT(4) &&
                    vVertexes[(iTileY + 1) * (int)uiGridWidth + iTileX].y0 & BIT(4) &&
                    vVertexes[(iTileY + 1) * (int)uiGridWidth + iTileX + 1].y0 & BIT(4)))
                )
            {
                //
                // Ramp
                //

                int iIndex(iY * cWorld.GetGridWidth() + iX);

                const int RAMP_FILTER_WIDTH(1);

                pK2[iIndex] = 0.0f;

                for (float fY = -(RAMP_FILTER_WIDTH - 1.0f) / 2.0f; fY <= (RAMP_FILTER_WIDTH - 1.0f) / 2.0f; fY += 1.0f)
                {
                    for (float fX = -(RAMP_FILTER_WIDTH - 1.0f) / 2.0f; fX <= (RAMP_FILTER_WIDTH - 1.0f) / 2.0f; fX += 1.0f)
                    {
                        int iFilterTileX(INT_FLOOR((iX + fX) * fTileScaleX));
                        int iFilterTileY(INT_FLOOR((iY + fY) * fTileScaleY));

                        float fLerps[2] =
                        {
                            FRAC((iX + fX) * fTileScaleX),
                            FRAC((iY + fY) * fTileScaleY)
                        };

                        float afHeight[4] =
                        {
                            pW3E[iFilterTileY * uiGridWidth + iFilterTileX],
                            pW3E[iFilterTileY * uiGridWidth + iFilterTileX + 1],
                            pW3E[(iFilterTileY + 1) * uiGridWidth + iFilterTileX],
                            pW3E[(iFilterTileY + 1) * uiGridWidth + iFilterTileX + 1]
                        };

                        pK2[iIndex] += PCF(fLerps, afHeight);
                    }
                }

                pK2[iIndex] /= (RAMP_FILTER_WIDTH * RAMP_FILTER_WIDTH);
                
            }
            else if (iTileX != (int)uiGridWidth - 1 && iTileY != (int)uiGridHeight - 1)
            {
                int iLayer(vVertexes[iTileY * uiGridWidth + iTileX].y2 & 0xf);

                if ((vVertexes[iTileY * uiGridWidth + iTileX + 1].y2 & 0xf) == iLayer && 
                    (vVertexes[(iTileY + 1) * uiGridWidth + iTileX].y2 & 0xf) == iLayer &&
                    (vVertexes[(iTileY + 1) * uiGridWidth + iTileX + 1].y2 & 0xf) == iLayer)
                {
                    //
                    // Continous layer
                    //

                    int iIndex(iY * cWorld.GetGridWidth() + iX);

                    float fLerps[2] =
                    {
                        FRAC(iX * fTileScaleX),
                        FRAC(iY * fTileScaleY)
                    };

                    float afHeight[4] =
                    {
                        pW3E[iTileY * uiGridWidth + iTileX],
                        pW3E[iTileY * uiGridWidth + iTileX + 1],
                        pW3E[(iTileY + 1) * uiGridWidth + iTileX],
                        pW3E[(iTileY + 1) * uiGridWidth + iTileX + 1]
                    };
                    
                    pK2[iIndex] = PCF(fLerps, afHeight);
                }
                else
                {
                    //
                    // Cliff
                    //

                    int iIndex(iY * cWorld.GetGridWidth() + iX);

                    const int CLIFF_FILTER_WIDTH(1);

                    pK2[iIndex] = 0.0f;

                    for (float fY = -(CLIFF_FILTER_WIDTH - 1.0f) / 2.0f; fY <= (CLIFF_FILTER_WIDTH - 1.0f) / 2.0f; fY += 1.0f)
                    {
                        for (float fX = -(CLIFF_FILTER_WIDTH - 1.0f) / 2.0f; fX <= (CLIFF_FILTER_WIDTH - 1.0f) / 2.0f; fX += 1.0f)
                        {
                            int iFilterTileX(INT_FLOOR((iX + fX) * fTileScaleX));
                            int iFilterTileY(INT_FLOOR((iY + fY) * fTileScaleY));

                            float fLerps[2] =
                            {
                                FRAC((iX + fX) * fTileScaleX),
                                FRAC((iY + fY) * fTileScaleY)
                            };

                            float afHeight[4] =
                            {
                                pW3E[iFilterTileY * uiGridWidth + iFilterTileX],
                                pW3E[iFilterTileY * uiGridWidth + iFilterTileX + 1],
                                pW3E[(iFilterTileY + 1) * uiGridWidth + iFilterTileX],
                                pW3E[(iFilterTileY + 1) * uiGridWidth + iFilterTileX + 1]
                            };

                            if (FLOAT_EQUALS(fLerps[0], 0.5f) && FLOAT_EQUALS(fLerps[1], 0.5f))
                            {                       
                                pK2[iIndex] += MAX(afHeight[0], MAX(afHeight[1], MAX(afHeight[2], afHeight[3])));
                            }
                            else if (FLOAT_EQUALS(fLerps[0], 0.5f))
                            {
                                if (fLerps[1] < 0.5f)
                                    pK2[iIndex] += MAX(afHeight[0], afHeight[1]);
                                else
                                    pK2[iIndex] += MAX(afHeight[2], afHeight[3]);
                            }
                            else if (FLOAT_EQUALS(fLerps[1], 0.5f))
                            {
                                if (fLerps[0] < 0.5f)
                                    pK2[iIndex] += MAX(afHeight[0], afHeight[2]);
                                else
                                    pK2[iIndex] += MAX(afHeight[1], afHeight[3]);
                            }
                            else
                            {
                                fLerps[0] = ROUND(fLerps[0]);
                                fLerps[1] = ROUND(fLerps[1]);

                                pK2[iIndex] += PCF(fLerps, afHeight);
                            }
                        }
                    }

                    pK2[iIndex] /= (CLIFF_FILTER_WIDTH * CLIFF_FILTER_WIDTH);
                }
            }
            else
            {
                int iIndex(iY * cWorld.GetGridWidth() + iX);

                pK2[iIndex] = pW3E[iTileY * uiGridWidth + iTileX];
            }
        }
    }

    float *pK2Smooth(K2_NEW_ARRAY(ctx_Editor, float, recWorld.GetArea()));

    for (int iY(0); iY < cWorld.GetGridHeight(); ++iY)
    {
        for (int iX(0); iX < cWorld.GetGridWidth(); ++iX)
        {
            int iIndex(iY * cWorld.GetGridWidth() + iX);

            const int FILTER_WIDTH(2);

            pK2Smooth[iIndex] = 0.0f;

            for (float fY = -(FILTER_WIDTH - 1.0f) / 2.0f; fY <= (FILTER_WIDTH - 1.0f) / 2.0f; fY += 1.0f)
            {
                for (float fX = -(FILTER_WIDTH - 1.0f) / 2.0f; fX <= (FILTER_WIDTH - 1.0f) / 2.0f; fX += 1.0f)
                {
                    int iFilterTileX(CLAMP<int>(INT_FLOOR(iX + fX), 0, cWorld.GetGridWidth() - 1));
                    int iFilterTileY(CLAMP<int>(INT_FLOOR(iY + fY), 0, cWorld.GetGridHeight() - 1));

                    float fLerps[2] =
                    {
                        FRAC(iX + fX),
                        FRAC(iY + fY)
                    };

                    float afHeight[4] =
                    {
                        pK2[iFilterTileY * cWorld.GetGridWidth() + iFilterTileX],
                        pK2[iFilterTileY * cWorld.GetGridWidth() + iFilterTileX + 1],
                        pK2[(iFilterTileY + 1) * cWorld.GetGridWidth() + iFilterTileX],
                        pK2[(iFilterTileY + 1) * cWorld.GetGridWidth() + iFilterTileX + 1]
                    };

                    pK2Smooth[iIndex] += PCF(fLerps, afHeight);
                }
            }

            pK2Smooth[iIndex] /= (FILTER_WIDTH * FILTER_WIDTH);
        }
    }

    Editor.GetWorld().SetRegion(WORLD_VERT_HEIGHT_MAP, recWorld, pK2);

    Vid.Notify(VID_NOTIFY_TERRAIN_VERTEX_MODIFIED, 0, 0, 1, &Editor.GetWorld());

    K2_DELETE_ARRAY(pW3E0);
    K2_DELETE_ARRAY(pW3E);
    K2_DELETE_ARRAY(pK2);
    K2_DELETE_ARRAY(pK2Smooth);

    return true;
}


/*--------------------
  ImportW3EBlockers
  --------------------*/
CMD(ImportW3EBlockers)
{
    if (vArgList.size() < 1)
        return false;

    CFileHandle hFile(vArgList[0], FILE_READ);

    uint dwSig(hFile.ReadInt32());

    uint ui0(hFile.ReadInt32());
    byte y0(hFile.ReadByte());
    
    uint ui1(hFile.ReadInt32());

    uint uiNumTableSize1(hFile.ReadInt32());
    for (uint ui(0); ui < uiNumTableSize1; ++ui)
        hFile.ReadInt32();

    uint uiNumTableSize2(hFile.ReadInt32());
    for (uint ui(0); ui < uiNumTableSize2; ++ui)
        hFile.ReadInt32();

    uint uiGridWidth(hFile.ReadInt32());
    uint uiGridHeight(hFile.ReadInt32());
    uint uiNumVertexes(uiGridWidth * uiGridHeight);

    float fOffsetX(hFile.ReadFloat());
    float fOffsetY(hFile.ReadFloat());

    ///*    Removed the "//" if you need any of the below variables
    ui1 = 0;
    ui0 = 0;
    dwSig = 0;
    fOffsetY = 0.0f;
    fOffsetX = 0.0f;
    y0 = 0;
    //*/

    vector<SVertex> vVertexes(uiNumVertexes);

    uint uiCount(0);
    while (!hFile.IsEOF() && uiCount < uiNumVertexes)
    {
        hFile.Read((char *)&vVertexes[uiCount], sizeof(SVertex));
        ++uiCount;
    }

    CWorld &cWorld(Editor.GetWorld());

    CRecti recWorld(0, 0, cWorld.GetGridWidth(), cWorld.GetGridHeight());
    byte *pK2(K2_NEW_ARRAY(ctx_Editor, byte, recWorld.GetArea()));
    
    float fTileScaleX(float(uiGridWidth - 1) / cWorld.GetTileWidth());
    float fTileScaleY(float(uiGridHeight - 1) / cWorld.GetTileHeight());

    for (int iY(0); iY < cWorld.GetGridHeight(); ++iY)
    {
        for (int iX(0); iX < cWorld.GetGridWidth(); ++iX)
        {
            int iTileX(INT_FLOOR(iX * fTileScaleX));
            int iTileY(INT_FLOOR(iY * fTileScaleY));

            float fFracX(FRAC(iX * fTileScaleX));
            float fFracY(FRAC(iY * fTileScaleY));
            
            if ((FLOAT_EQUALS(fFracX, 0.0f) && FLOAT_EQUALS(fFracY, 0.0f) && iTileX != 0 && iTileY != 0 && (
                    vVertexes[iTileY * (int)uiGridWidth + iTileX].y0 & BIT(4))
                ) ||
                (FLOAT_EQUALS(fFracX, 0.0f) && iTileX != 0 && iTileY != (int)uiGridHeight - 1 && (
                    vVertexes[iTileY * (int)uiGridWidth + iTileX].y0 & BIT(4) &&
                    vVertexes[(iTileY + 1) * (int)uiGridWidth + iTileX].y0 & BIT(4))
                ) ||
                (FLOAT_EQUALS(fFracY, 0.0f) && iTileY != 0 && iTileX != (int)uiGridWidth - 1 && (
                    vVertexes[iTileY * (int)uiGridWidth + iTileX].y0 & BIT(4) &&
                    vVertexes[iTileY * (int)uiGridWidth + iTileX + 1].y0 & BIT(4))
                ) || 
                (iTileX != (int)uiGridWidth - 1 && iTileY != (int)uiGridHeight - 1 && (
                    vVertexes[iTileY * (int)uiGridWidth + iTileX].y0 & BIT(4) &&
                    vVertexes[iTileY * (int)uiGridWidth + iTileX + 1].y0 & BIT(4) &&
                    vVertexes[(iTileY + 1) * (int)uiGridWidth + iTileX].y0 & BIT(4) &&
                    vVertexes[(iTileY + 1) * (int)uiGridWidth + iTileX + 1].y0 & BIT(4)))
                )
            {
                //
                // Ramp
                //

                pK2[iY * cWorld.GetGridWidth() + iX] = 0;
            }
            else if (iTileX != (int)uiGridWidth - 1 && iTileY != (int)uiGridHeight - 1 && iTileX != 0 && iTileY != 0)
            {
                int iLayer(vVertexes[iTileY * uiGridWidth + iTileX].y2 & 0xf);

                if (FLOAT_EQUALS(fFracX, 0.0f) && FLOAT_EQUALS(fFracY, 0.0f) &&
                    (vVertexes[(iTileY - 1) * uiGridWidth + iTileX - 1].y2 & 0xf) == iLayer && 
                    (vVertexes[(iTileY - 1) * uiGridWidth + iTileX].y2 & 0xf) == iLayer &&
                    (vVertexes[(iTileY - 1) * uiGridWidth + iTileX + 1].y2 & 0xf) == iLayer &&
                    (vVertexes[iTileY * uiGridWidth + iTileX - 1].y2 & 0xf) == iLayer && 
                    (vVertexes[iTileY * uiGridWidth + iTileX + 1].y2 & 0xf) == iLayer &&
                    (vVertexes[(iTileY + 1) * uiGridWidth + iTileX - 1].y2 & 0xf) == iLayer &&
                    (vVertexes[(iTileY + 1) * uiGridWidth + iTileX].y2 & 0xf) == iLayer && 
                    (vVertexes[(iTileY + 1) * uiGridWidth + iTileX + 1].y2 & 0xf) == iLayer)
                {
                    //
                    // Continous layer
                    //

                    pK2[iY * cWorld.GetGridWidth() + iX] = 0;
                }
                else if (FLOAT_EQUALS(fFracX, 0.0f) && !FLOAT_EQUALS(fFracY, 0.0f) &&
                    (vVertexes[iTileY * uiGridWidth + iTileX - 1].y2 & 0xf) == iLayer && 
                    (vVertexes[iTileY * uiGridWidth + iTileX + 1].y2 & 0xf) == iLayer &&
                    (vVertexes[(iTileY + 1) * uiGridWidth + iTileX - 1].y2 & 0xf) == iLayer &&
                    (vVertexes[(iTileY + 1) * uiGridWidth + iTileX].y2 & 0xf) == iLayer && 
                    (vVertexes[(iTileY + 1) * uiGridWidth + iTileX + 1].y2 & 0xf) == iLayer)
                {
                    //
                    // Continous layer
                    //

                    pK2[iY * cWorld.GetGridWidth() + iX] = 0;
                }
                else if (!FLOAT_EQUALS(fFracX, 0.0f) && FLOAT_EQUALS(fFracY, 0.0f) &&
                    (vVertexes[(iTileY - 1) * uiGridWidth + iTileX].y2 & 0xf) == iLayer &&
                    (vVertexes[(iTileY - 1) * uiGridWidth + iTileX + 1].y2 & 0xf) == iLayer &&
                    (vVertexes[iTileY * uiGridWidth + iTileX + 1].y2 & 0xf) == iLayer &&
                    (vVertexes[(iTileY + 1) * uiGridWidth + iTileX].y2 & 0xf) == iLayer && 
                    (vVertexes[(iTileY + 1) * uiGridWidth + iTileX + 1].y2 & 0xf) == iLayer)
                {
                    //
                    // Continous layer
                    //

                    pK2[iY * cWorld.GetGridWidth() + iX] = 0;
                }
                else if (!FLOAT_EQUALS(fFracX, 0.0f) && !FLOAT_EQUALS(fFracY, 0.0f) &&
                    (vVertexes[iTileY * uiGridWidth + iTileX + 1].y2 & 0xf) == iLayer &&
                    (vVertexes[(iTileY + 1) * uiGridWidth + iTileX].y2 & 0xf) == iLayer && 
                    (vVertexes[(iTileY + 1) * uiGridWidth + iTileX + 1].y2 & 0xf) == iLayer)
                {
                    //
                    // Continous layer
                    //

                    pK2[iY * cWorld.GetGridWidth() + iX] = 0;
                }
                else
                {
                    //
                    // Cliff
                    //

                    pK2[iY * cWorld.GetGridWidth() + iX] = 1;
                }
            }
            else
            {
                pK2[iY * cWorld.GetGridWidth() + iX] = 0;
            }
        }
    }
    
    Editor.GetWorld().SetRegion(WORLD_VERT_BLOCKER_MAP, recWorld, pK2);

    K2_DELETE_ARRAY(pK2);

    return true;
}
#endif


/*--------------------
  SnapCliffs
  --------------------*/
CMD(SnapCliffs)
{
    CWorld &cWorld(Editor.GetWorld());
    WorldEntList &vEntities(cWorld.GetEntityList());
    WorldEntList_cit citEnd(vEntities.end());

    // Snap to 2 tile grid
    for (WorldEntList_cit cit(vEntities.begin()); cit != citEnd; ++cit)
    {
        CWorldEntity *pWorldEnt(cWorld.GetEntityByHandle(*cit));
        if (pWorldEnt == NULL)
            continue;

        if (pWorldEnt->GetType() != _T("Prop_Cliff") && pWorldEnt->GetType() != _T("Prop_Cliff2"))
            continue;

        uint uiEntIndex(pWorldEnt->GetIndex());

        cWorld.UnlinkEntity(uiEntIndex);

        CVec3f v3Position(pWorldEnt->GetPosition());
        v3Position.x = ROUND(v3Position.x / 32.0f) * 32.0f;
        v3Position.y = ROUND(v3Position.y / 32.0f) * 32.0f;
        v3Position.z = ROUND(v3Position.z / 32.0f) * 32.0f;

        pWorldEnt->SetPosition(v3Position);
        
        cWorld.LinkEntity(pWorldEnt->GetIndex(), LINK_SURFACE | LINK_MODEL, SURF_PROP);
    }

    return true;
}


/*--------------------
  CompressIndexes
  --------------------*/
CMD(CompressIndexes)
{
    CWorld &cWorld(Editor.GetWorld());
    WorldEntList &vEntities(cWorld.GetEntityList());
    WorldEntList vOldEntities(vEntities);

    vEntities.clear();
    for (WorldEntList_cit cit(vOldEntities.begin()), citEnd(vOldEntities.end()); cit != citEnd; ++cit)
    {
        if (*cit == INVALID_POOL_HANDLE)
            continue;

        vEntities.push_back(*cit);

        CWorldEntity *pWorldEnt(cWorld.GetEntityByHandle(vEntities.back()));
        if (pWorldEnt == NULL)
            continue; // !!!

        pWorldEnt->SetIndex(uint(vEntities.size() - 1));
    }

    return true;
}


/*--------------------
  AddEntityTypes
  --------------------*/
UI_VOID_CMD(AddEntityTypes, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));
    if (pList == NULL)
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    map<ushort, tstring> mapEntities;
    EntityRegistry.GetEntityList(mapEntities);
    tsvector vEntities;
    for (map<ushort, tstring>::iterator it(mapEntities.begin()); it != mapEntities.end(); ++it)
    {
        if (it->first <= Entity_Tangible)
            continue;

        const CDynamicEntityAllocator *pAllocator(EntityRegistry.GetDynamicAllocator(it->first));
        if (pAllocator != NULL && GET_ENTITY_BASE_TYPE1(pAllocator->GetBaseType()) != ENTITY_BASE_TYPE1_UNIT &&
            GET_ENTITY_BASE_TYPE1(pAllocator->GetBaseType()) != ENTITY_BASE_TYPE1_AFFECTOR)
            continue;

        vEntities.push_back(it->second);
    }

    tstring sTemplate(vArgList[0]->Evaluate());

    sort(vEntities.begin(), vEntities.end());
    for (tsvector_it it(vEntities.begin()); it != vEntities.end(); ++it)
    {
        mapParams[_T("label")] = *it;

        pList->CreateNewListItemFromTemplate(sTemplate, *it, mapParams);
    }
}


/*--------------------
  SetModifiers
  --------------------*/
UI_VOID_CMD(SetModifiers, 1)
{
    Editor.GetWorld().SetModifiers(vArgList[0]->Evaluate());
}


/*--------------------
  GetModifiers
  --------------------*/
UI_CMD(GetModifiers, 0)
{
    return Editor.GetWorld().GetModifiers();
}


/*--------------------
  SetFancyName
  --------------------*/
UI_VOID_CMD(SetFancyName, 1)
{
    Editor.GetWorld().SetFancyName(vArgList[0]->Evaluate());
}


/*--------------------
  GetFancyName
  --------------------*/
UI_CMD(GetFancyName, 0)
{
    return Editor.GetWorld().GetFancyName();
}


/*--------------------
  GetMapSize
  --------------------*/
UI_CMD(GetMapSize, 0)
{
    return XtoA(Editor.GetWorld().GetSize());
}


/*--------------------
  GetMapScale
  --------------------*/
UI_CMD(GetMapScale, 0)
{
    return XtoA(Editor.GetWorld().GetScale());
}


/*--------------------
  SetTextureScale
  --------------------*/
UI_VOID_CMD(SetTextureScale, 1)
{
    Editor.GetWorld().SetTextureScale(AtoF(vArgList[0]->Evaluate()));
}


/*--------------------
  GetTextureScale
  --------------------*/
UI_CMD(GetTextureScale, 0)
{
    return XtoA(Editor.GetWorld().GetTextureScale());
}


/*--------------------
  RebuildTerrain
  --------------------*/
UI_VOID_CMD(RebuildTerrain, 0)
{
    Console.Execute(_T("RebuildTerrain"));
}









