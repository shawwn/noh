// (C)2005 S2 Games
// c_anim.h
//
//=============================================================================
#ifndef __C_ANIM_H__
#define __C_ANIM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_resourcewatcher2.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SAnimEvent
{
    int             iFrame;
    tstring         sCommand;

    SAnimEvent() {}
    SAnimEvent(int _iFrame, const tstring& _sCommand) : iFrame(_iFrame), sCommand(_sCommand) {}
};


class CK2Model;
class CSkeleton;
struct SBoneMotion;
//=============================================================================

//=============================================================================
// CAnim
//=============================================================================
class CAnim : public IResourceWatcher
{
private:
    uint                m_uiIndex;
    tstring             m_sName;

    // Frame triggered events, like footstep sounds and particles
    vector<SAnimEvent>  m_vFrameEvents;
    tsvector            m_vStartEvents;
    tsvector            m_vEndEvents;

    ResHandle           m_hClip;            // clip this animation references

    SBoneMotion**       m_ppMotions;        // maps model bones to clip animation data

    int                 m_iStartFrame;      // start frame
    int                 m_iNumFrames;
    int                 m_iLoopbackFrame;
    bool                m_bLoop;
    float               m_fFps;
    int                 m_iMSperFrame;
    int                 m_iNumLoopFrames;
    int                 m_uiBlendTime;
    bool                m_bLock;
    int                 m_iMinStartFrame;
    int                 m_iMaxStartFrame;
    CK2Model*           m_pModel;

    // Defaults for proper rebuilding during a clip reload
    int                 m_iStartFrameDef;
    int                 m_iNumFramesDef;
    int                 m_iLoopbackFrameDef;
    int                 m_iMinStartFrameDef;
    int                 m_iMaxStartFrameDef;

public:
    K2_API ~CAnim();
    K2_API CAnim
    (
        CK2Model *pModel,
        uint uiIndex,
        const tstring &sName,
        const tstring &sClip,
        int iStartFrame,
        int iNumFrames,
        int iLoopbackFrame,
        bool bLoop,
        float fFps,
        int iNumLoopFrames,
        int iBlendTime,
        bool bLock,
        int iMinStartFrame,
        int iMaxStartFrame,
        uint uiIgnoreFlags
    );

    ResHandle                   GetClip() const                 { return m_hClip; }
    SBoneMotion**               GetMotions() const              { return m_ppMotions; }
    float                       GetFps() const                  { return m_fFps; }
    int                         GetStartFrame() const           { return m_iStartFrame; }
    int                         GetLoopbackFrame() const        { return m_iLoopbackFrame; }
    int                         GetNumFrames() const            { return m_iNumFrames; }
    bool                        IsLooping() const               { return m_bLoop; }
    int                         GetNumLoopFrames() const        { return m_iNumLoopFrames; }
    int                         GetBlendTime() const            { return m_uiBlendTime; }
    bool                        GetLock() const                 { return m_bLock; }
    int                         GetMSperFrame() const           { return m_iMSperFrame; }
    int                         GetMinStartFrame() const        { return m_iMinStartFrame; }
    int                         GetMaxStartFrame() const        { return m_iMaxStartFrame; }

    bool                        ComputeAnimFrame(int iAnimTime, int &iLoFrame, int &iHiFrame, float &fLerpAmt, uint uiForceLength);

    void                        Rebuild(ResHandle hResource);

    void                        SetName(const tstring &sName)   { m_sName = sName; }
    const tstring&              GetName() const                 { return m_sName; }

    int                         GetLength() const               { return m_iNumFrames * m_iMSperFrame; }
    uint                        GetIndex() const                { return m_uiIndex; }

    K2_API void                 AddFrameEvent(int iFrame, const tstring &sCommand);
    K2_API void                 AddStartEvent(const tstring &sCommand);
    K2_API void                 AddEndEvent(const tstring &sCommand);

    const tsvector&             GetStartEvents()                { return m_vStartEvents; }
    const tsvector&             GetEndEvents()                  { return m_vEndEvents; }

    void                        CheckEvents(CSkeleton *pSkeleton, int iTime0, int iTime1, int iChannel);
};
//=============================================================================
#endif //__C_ANIM_H__
