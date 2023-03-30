// (C)2007 S2 Games
// c_lighttool.h
//
//=============================================================================
#ifndef __C_SOUNDTOOL_H__
#define __C_SOUNDTOOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_range.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_model.h"
#include "../k2/c_k2model.h"

#include "c_toolbox.h"
#include "i_tool.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorldSound;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum ESoundEditMode
{
    SOUND_CREATE = 0,
    SOUND_SELECT,
    SOUND_TRANSLATE_XY,
    SOUND_TRANSLATE_Z
};
//=============================================================================

//=============================================================================
// CSoundTraceResult
//=============================================================================
class CSoundTraceResult
{
public:
    uint            uiIndex;
    float           fFraction;
    CVec3f          v3EndPos;
    CWorldSound*    pSound;

    CSoundTraceResult() :
    uiIndex(INVALID_INDEX),
    fFraction(FAR_AWAY),
    v3EndPos(0.0f, 0.0f, 0.0f),
    pSound(NULL)
    {}

    void    Clear()
    {
        uiIndex = INVALID_INDEX;
        fFraction = FAR_AWAY;
        v3EndPos.Clear();
        pSound = NULL;
    }
};
//=============================================================================

//=============================================================================
// CSoundTool
//=============================================================================
class CSoundTool : public ITool
{
private:
    bool            m_bCloning;
    CSoundTraceResult       m_Trace;
    uiset           m_setSelection;
    uiset           m_setOldSelection;
    uiset           m_setHoverSelection;
    CVec2f          m_vStartCursorPos;
    CVec2f          m_vOldCursorPos;
    int             m_iState;
    CVec3f          m_vTranslate;
    CVec3f          m_vTrueTranslate;
    bool            m_bSnapCursor;
    CVec3f          m_v3EndPos;
    ResHandle       m_hLineMaterial;

    uint            m_uiHoverIndex;

    struct  SSoundInfo
    {
        SoundHandle hSound;
        uint        uiNextStartTime;
        SSoundInfo(SoundHandle sound = INVALID_INDEX, uint start = 0) : hSound(sound), uiNextStartTime(start) {}
    };

    typedef map<int, SSoundInfo>    SoundsMap;
    typedef SoundsMap::iterator     SoundsMap_it;

    SoundsMap   m_mapSounds;

    SoundHandle     m_hWorldAmbientSound;

    ResHandle       m_hSoundModel;
    CSkeleton*      m_pSoundModelSkeleton;

    CVec3f          GetSoundPosition(uint zIndex);

    bool            CursorSoundTrace(CSoundTraceResult &result);
    CVec3f          SelectionCenter();
    void            SnapCursor(const CVec3f &vOrigin);
    void            CloneSelection();

    void            Hovering();
    void            TranslateXY();
    void            TranslateZ();

    void            Create();

    void            StartSelect();
    void            StartTranslateXY();
    void            StartTransform(int iState);

    void            ApplySelect();
    void            ApplyTransform();

    void            DrawSound(uint zIndex);

    void            UpdateHoverSelection();
    
    void            StartSound(uint uiIndex);
    void            StopSound(uint uiIndex);
    void            SetVolume(uint uiIndex, float fVolume);
    void            UpdateSounds();

public:
    CSoundTool();
    virtual ~CSoundTool()                   {}

    void        CalcToolProperties();
    uint        CreateSound(const CVec3f &vPos = V_ZERO);

    void        PrimaryUp();
    void        PrimaryDown();
    void        SecondaryUp()       {}
    void        SecondaryDown();
    void        TertiaryUp()        {}
    void        TertiaryDown()      {}
    void        QuaternaryUp()      {}
    void        QuaternaryDown()    {}

    void        Cancel();
    void        Delete();

    void        Enter();
    void        Leave();

    void        Frame(float fFrameTime);

    bool        IsSelectionActive();
    CRectf      GetSelectionRect();
    int         IsSelected(uint zIndex);
    int         IsHoverSelected(uint zIndex);
    
    void        SelectNextSound();
    void        SelectPreviousSound();

    void        Split();

    void        Draw();
    void        Render();

    void        PlayWorldAmbientSound();
    void        StopWorldAmbientSound();
    void        SetWorldAmbientSoundVolume(float fVolume);

    // Current
    CVec3f      GetCurrentSoundPosition(uint uiSound);
    float       GetVolume(uint uiSound);
    float       GetFalloff(uint uiSound);
    int         GetLoopDelayMin(uint uiSound);
    int         GetLoopDelayMax(uint uiSound);

    void        SetSelectionSound(const tstring &sSound);
    void        SetSelectionVolume(float fVolume);
    void        SetSelectionFalloff(float fFalloff);
    void        SetSelectionLoopDelayMin(int iMin);
    void        SetSelectionLoopDelayMax(int iMin);

    const uiset&        GetSelectionSet()           { return m_setSelection; }
};
//=============================================================================
#endif //__C_SOUNDTOOL_H__
