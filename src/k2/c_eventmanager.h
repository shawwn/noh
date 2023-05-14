// (C)2007 S2 Games
// c_eventmanager.h
//
//=============================================================================
#ifndef __C_EVENTMANAGER_H__
#define __C_EVENTMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_vid.h"
#include "c_input.h"
#include "c_uitrigger.h"
#include "c_uimanager.h"
#include "c_draw2d.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum ELoadingStatus
{
    LOADING_STATUS_NONE,
    LOADING_STATUS_IN_PROGRESS,
    LOADING_STATUS_FINISHED,
};

enum ELoadingDisplay
{
    LOADING_DISPLAY_NONE,
    LOADING_DISPLAY_BLACK,
    LOADING_DISPLAY_LOGO,
    LOADING_DISPLAY_INTERFACE
};
//=============================================================================

//=============================================================================
// IModalDialog
//=============================================================================
class K2_API IModalDialog
{
protected:
    static ELoadingDisplay  s_eDisplay;
    static CUITrigger       s_triggerTitle;
    static CUITrigger       s_triggerProgress;
    static CUITrigger       s_triggerLoadingImage;
    static CUITrigger       s_triggerVisible;
    static uint             s_uiLastUpdate;
    static uint             s_uiNumLoadingJobs;
    static uint             s_uiLoadingJob;
    static float            s_fProgress;
    static tstring          s_sLoadingInterface;

public:
    static bool IsActive()  { return s_eDisplay == LOADING_DISPLAY_INTERFACE; }

    static void SetTitle(const tstring &sTitle)
    {
        s_triggerTitle.Trigger(sTitle, true);
    }

    static void SetProgress(float fProgress)
    {
        assert(fProgress >= s_fProgress || fProgress == 0.0f);

        s_fProgress = fProgress;

        float fPercent(LERP(fProgress, float(s_uiLoadingJob) / float(s_uiNumLoadingJobs), float(s_uiLoadingJob + 1) / float(s_uiNumLoadingJobs)));
        s_triggerProgress.Trigger(XtoA(fPercent), true);
    }

    static void SetDisplay(ELoadingDisplay eDisplay)
    {
        s_eDisplay = eDisplay;
    }

    static void SetNumLoadingJobs(uint uiNumLoadingJobs)
    {
        s_uiNumLoadingJobs = uiNumLoadingJobs;
        s_uiLoadingJob = 0;
        s_uiLastUpdate = INVALID_TIME;
        s_fProgress = 0.0f;
        SetProgress(0.0f);
    }

    static void NextLoadingJob()
    {
//        assert(s_uiLoadingJob != s_uiNumLoadingJobs - 1); // TKTK this trips for a dedicated server in automatic start mode

        ++s_uiLoadingJob;
        s_uiLoadingJob = MIN(s_uiLoadingJob, s_uiNumLoadingJobs - 1);
        s_fProgress = 0.0f;
        SetProgress(0.0f);
    }

    static void SetLoadingImage(const tstring &sPath)
    {
        s_triggerLoadingImage.Trigger(sPath);
    }

    static tstring  GetTitle()
    {
        return s_triggerTitle.GetLastParam();
    }

    static void Show(const tstring &sLoadingInterface)
    {
        s_sLoadingInterface = sLoadingInterface;
        s_eDisplay = LOADING_DISPLAY_INTERFACE;
        if (!s_sLoadingInterface.empty())
            UIManager.AddOverlayInterface(s_sLoadingInterface);
        s_triggerLoadingImage.Trigger(TSNULL);

        float fPercent(LERP(s_fProgress, float(s_uiLoadingJob) / float(s_uiNumLoadingJobs), float(s_uiLoadingJob + 1) / float(s_uiNumLoadingJobs)));
        s_triggerProgress.Trigger(XtoA(fPercent), true);

        s_triggerVisible.Trigger(XtoA(true));
    }

    static void Hide()
    {
        s_eDisplay = LOADING_DISPLAY_BLACK;
        if (!s_sLoadingInterface.empty())
            UIManager.RemoveOverlayInterface(s_sLoadingInterface);
            s_uiLastUpdate = INVALID_TIME;
        s_sLoadingInterface.clear();

        s_triggerVisible.Trigger(XtoA(false));
    }

    static void Update()
    {
        if (s_uiLastUpdate != INVALID_TIME && K2System.Milliseconds() - s_uiLastUpdate < 50)
            return;

        K2System.HandleOSMessages();
        Vid.BeginFrame();
        Input.Frame();
        UIManager.ProcessInput();
        UIManager.Frame(s_uiLastUpdate != INVALID_TIME ? K2System.Milliseconds() - s_uiLastUpdate : 0);
        Vid.EndFrame();
        s_uiLastUpdate = K2System.Milliseconds();
    }
};
//=============================================================================

//=============================================================================
// CLoadJob
//=============================================================================
template<class C>
class CLoadJob : public IModalDialog
{
public:
    class IFunctions
    {
    protected:
        C*              m_pList;
        IModalDialog*   m_pJob;

    public:
        IFunctions() : m_pList(nullptr), m_pJob(nullptr)  {}
        virtual ~IFunctions() {}

        void    SetJob(IModalDialog *pJob)  { m_pJob = pJob; }
        void    SetList(C *pList)           { m_pList = pList; }

        void    SetTitle(const tstring &sTitle) const               { if (m_pJob != nullptr) m_pJob->SetTitle(sTitle); }
        void    SetProgress(float fProgress) const                  { if (m_pJob != nullptr) m_pJob->SetProgress(fProgress); }

        virtual float   PreFrame(typename C::iterator&, float f) const  { return 0.0f; }
        virtual float   Frame(typename C::iterator&, float f) const     { return 0.0f; }
        virtual float   PostFrame(typename C::iterator&, float f) const { return 0.0f; }
    };

private:
    C&              m_List;
    IFunctions&     m_Functions;
    float           m_fProgress;
    ELoadingDisplay m_eDisplay;

    CLoadJob();

public:
    ~CLoadJob() { s_eDisplay = LOADING_DISPLAY_NONE; }
    CLoadJob(C &_List, IFunctions *_Functions, ELoadingDisplay eDisplay) :
    m_List(_List),
    m_Functions(*_Functions),
    m_fProgress(0.0f),
    m_eDisplay(eDisplay)
    {
        m_Functions.SetJob(this);
        m_Functions.SetList(&m_List);
        s_eDisplay = m_eDisplay;
    }

    void    Execute(float fSize)
    {
        int fCount(0);
        uint uiLastUpdateMS(0);
        for (typename C::iterator it(m_List.begin()); it != m_List.end(); )
        {
            s_eDisplay = m_eDisplay;

            if (fSize == 0)
                m_fProgress = 1.0f;
            else
                m_fProgress = fCount / fSize;

            // Limit loading screen rendering to 20 FPS
            uint uiMS(K2System.Milliseconds());

            if (uiMS - uiLastUpdateMS > 50)
            {
                K2System.HandleOSMessages();
                
                fCount += m_Functions.PreFrame(it, m_fProgress);
                Vid.BeginFrame();
                
                if (m_eDisplay != LOADING_DISPLAY_NONE)
                {
                    Vid.SetColor(BLACK);
                    Vid.Clear();
                }

                if (m_eDisplay == LOADING_DISPLAY_LOGO)
                {
                    ResHandle hLogo(g_ResourceManager.LookUpName(_T("logo"), RES_TEXTURE));
                    if (hLogo != INVALID_RESOURCE)
                    {
                        Draw2D.SetColor(WHITE);
                        Draw2D.Rect((Draw2D.GetScreenW() - 512.0f) * 0.5f, (Draw2D.GetScreenH() - 512.0f) * 0.5f, 512.0f, 512.0f, 0.0f, 0.0f, 1.0f, 1.0f, hLogo);
                    }
                }
                
                fCount += m_Functions.Frame(it, m_fProgress);
                Input.Frame();

                if (m_eDisplay == LOADING_DISPLAY_INTERFACE)
                {
                    UIManager.ProcessInput();
                    UIManager.Frame(Host.GetFrameLength());
                }
                
                Vid.EndFrame();

                fCount += m_Functions.PostFrame(it, m_fProgress);

                uiLastUpdateMS = uiMS;
            }
            else
            {
                fCount += m_Functions.PreFrame(it, m_fProgress);
                fCount += m_Functions.Frame(it, m_fProgress);
                fCount += m_Functions.PostFrame(it, m_fProgress);
            }
        }

        SetProgress(0.0f);
    }
};
//=============================================================================

#endif //__C_EVENTMANAGER_H__
