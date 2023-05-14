// (C)2005 S2 Games
// c_profilemanager.cpp
//
// Class based hierarchical ingame profiler
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_profilemanager.h"
#include "c_profilenode.h"
#include "c_draw2d.h"
#include "c_input.h"
#include "c_fontmap.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVAR_STRINGF(prof_font,         "system_medium",        CVAR_SAVECONFIG);
CVAR_BOOL(prof_max,             false);

bool g_bProfile; // Happy global

CProfileManager ProfileManager; // the global singleton
//=============================================================================


/*====================
  CProfileManager::CProfileManager
  ====================*/
CProfileManager::CProfileManager() :
m_bActive(false),
m_bDraw(false)
{
    m_pPrintNode = m_pRootNode = m_pCurrentNode = K2_NEW(ctx_Profile,  CProfileNode)(_T("Root"), PROFILE_ROOT, nullptr);
    g_bProfile = true;
}


/*====================
  CProfileManager::~CProfileManager
  ====================*/
CProfileManager::~CProfileManager()
{
    g_bProfile = false;
    m_bActive = false;
    K2_DELETE(m_pRootNode);
}


/*====================
  CProfileManager::StartSample
  ====================*/
bool    CProfileManager::StartSample(const TCHAR *szNode, eProfileType eType)
{
    if (!g_bProfile || !m_pCurrentNode)
        return false;

    m_pCurrentNode = m_pCurrentNode->StartSample(szNode, eType);
    return true;
}


/*====================
  CProfileManager::EndSample
  ====================*/
void    CProfileManager::EndSample()
{
    if (!g_bProfile || m_pCurrentNode == m_pRootNode)
        return;

    if (m_pCurrentNode->GetType() != m_pCurrentNode->GetParent()->GetType())
    {
        CProfileNode *pNode = m_pCurrentNode;

        m_pCurrentNode = m_pCurrentNode->EndSample();
        pNode->ResetFrame(pNode->GetType());
    }
    else
    {
        m_pCurrentNode = m_pCurrentNode->EndSample();
    }
}


/*====================
  CProfileManager::ResetFrame
  ====================*/
void    CProfileManager::ResetFrame(eProfileType eType)
{
    m_pRootNode->ResetFrame(eType);
}


/*====================
  CProfileManager::Frame

  Handles input for the profile manager
  ====================*/
void    CProfileManager::Frame()
{
    if (!m_bActive)
        return;

    // Steal all input while the profiler window is active
    while (!Input.IsEmpty())
    {
        const SIEvent &event = Input.Pop();
        switch (event.eType)
        {
        case INPUT_AXIS:
            break;

        case INPUT_BUTTON:
            // Filter out key releases
            if (event.cAbs.fValue == 0)
                break;

            switch (event.uID.btn)
            {
            case BUTTON_F1:
                m_bActive = false;
                break;
            case BUTTON_ESC:
                m_bDraw = m_bActive = false;
                break;
            case BUTTON_MISC3:
                {
                    ProfileVector vChildren(m_pPrintNode->GetChildren());
                    LONGLONG llMax(0);

                    for (ProfileVector::iterator it(vChildren.begin()); it != vChildren.end(); ++it)
                    {
                        if ((*it)->GetFrameTime() > llMax)
                        {
                            m_pPrintNode = *it;
                            llMax = (*it)->GetFrameTime();
                        }
                    }
                }
                break;
            default:
                break;
            }
            break;

        case INPUT_CHARACTER:
            if (event.uID.chr >= 32)
            {
                TCHAR key = event.uID.chr;

                SetNode(key);
            }
            break;

        default:
            break;
        }
    }
}


/*====================
  CProfileManager::Draw
  ====================*/
void    CProfileManager::Draw()
{
    if (!m_bDraw)
        return;

    PROFILE("CProfileManager::Draw");

    ResHandle hProfilerFont(g_ResourceManager.LookUpName(prof_font, RES_FONTMAP));
    CFontMap *pFontMap(g_ResourceManager.GetFontMap(hProfilerFont));
    if (pFontMap == nullptr)
        return;

    const float FONT_WIDTH = pFontMap->GetFixedAdvance();
    const float FONT_HEIGHT = pFontMap->GetMaxHeight();
    const int   PANEL_WIDTH = 72;
    const float START_X = Draw2D.GetScreenW() - FONT_WIDTH * (PANEL_WIDTH + 1);
    const float START_Y = FONT_HEIGHT;
    const LONGLONG FREQUENCY = K2System.GetFrequency();

    float iDrawY = START_Y;
    tstring sStr;

    Draw2D.SetColor(0.2f, 0.2f, 0.2f, 0.5f);
    Draw2D.Rect(START_X - 2, START_Y - 2, FONT_WIDTH * PANEL_WIDTH + 4, FONT_HEIGHT * (5.0f + m_pPrintNode->GetChildren().size()) + 4.0f);

    if (m_bActive)
    {
        Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        Draw2D.RectOutline(START_X - 2, START_Y - 2, FONT_WIDTH * PANEL_WIDTH + 4, FONT_HEIGHT * (5.0f + m_pPrintNode->GetChildren().size()) + 4.0f, 1);
    }

    Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    LONGLONG llFrameTime(0);
    const CProfileNode *pNode(m_pPrintNode->GetParent());
    if (pNode)
        while (pNode->GetParent() && pNode->GetParent()->GetType() == m_pPrintNode->GetType())
            pNode = pNode->GetParent();

    if (pNode)
        llFrameTime += prof_max ? pNode->GetMaxFrameTime() : pNode->GetFrameTime();

    if (llFrameTime)
    {
        sStr = _T("  ")
            + XtoA(m_pPrintNode->GetName(), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + SPACE
            + XtoA(float(prof_max ? m_pPrintNode->GetMaxFrameTime() : m_pPrintNode->GetFrameTime()) / llFrameTime * 100.0f, 0, 6, 2) + _T("% ")
            + XtoA(float(prof_max ? m_pPrintNode->GetMaxFrameTime() : m_pPrintNode->GetFrameTime()) / FREQUENCY * 1000.0f, 0, 10, 3) + _T(" ms ")
            + XtoA(prof_max ? m_pPrintNode->GetMaxFrameCalls() : m_pPrintNode->GetFrameCalls(), 0, 10);
    }
    else
    {
        sStr = _T("  ")
            + XtoA(m_pPrintNode->GetName(), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + SPACE
            + XtoA(float(prof_max ? m_pPrintNode->GetMaxFrameTime() : m_pPrintNode->GetFrameTime()) / FREQUENCY * 1000.0f, 0, 18, 3) + _T(" ms ")
            + XtoA(prof_max ? m_pPrintNode->GetMaxFrameCalls() : m_pPrintNode->GetFrameCalls(), 0, 10);
    }

    Draw2D.String(START_X, iDrawY, sStr, hProfilerFont);
    iDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, iDrawY, _T("=============================================================="), hProfilerFont);
    iDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, iDrawY, _T("0 .."), hProfilerFont);
    iDrawY += FONT_HEIGHT;

    int i = 1;

    const ProfileVector &vChildren(m_pPrintNode->GetChildren());

    LONGLONG    llParentFrameTime = prof_max ? m_pPrintNode->GetMaxFrameTime() : m_pPrintNode->GetFrameTime();
    LONGLONG    llUnloggedTime = llParentFrameTime;

    for (ProfileVector::const_iterator it(vChildren.begin()); it != vChildren.end(); ++it)
    {
        if ((*it)->GetType() == m_pPrintNode->GetType())
        {
            float   fPercentageOfParent = (prof_max ? (*it)->GetMaxFrameTime() : (*it)->GetFrameTime()) ? llParentFrameTime ? static_cast<float>(prof_max ? (*it)->GetMaxFrameTime() : (*it)->GetFrameTime()) / static_cast<float>(llParentFrameTime) * 100.0f : 100.0f : 0.0f;

            sStr = XtoA(i, FMT_NOPREFIX, 0, 36) + SPACE
                + XtoA(tstring((*it)->GetName()).substr(0, PANEL_WIDTH - 39), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + SPACE
                + XtoA(fPercentageOfParent, 0, 6, 2) + _T("% ")
                + XtoA(float(prof_max ? (*it)->GetMaxFrameTime() : (*it)->GetFrameTime()) / FREQUENCY * 1000.0f, 0, 10, 3) + _T(" ms ")
                + XtoA(prof_max ? (*it)->GetMaxFrameCalls() : (*it)->GetFrameCalls(), 0, 10);

            Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
            Draw2D.String(START_X, iDrawY, sStr, hProfilerFont);
            iDrawY += FONT_HEIGHT;
            ++i;

            llUnloggedTime -= prof_max ? (*it)->GetMaxFrameTime() : (*it)->GetFrameTime();
        }
        else
        {
            if ((*it)->GetFrame() == m_pPrintNode->GetFrame())
            {
                float   fPercentageOfParent = (prof_max ? (*it)->GetMaxFrameTime() : (*it)->GetFrameTime()) ? llParentFrameTime ? static_cast<float>(prof_max ? (*it)->GetMaxFrameTime() : (*it)->GetFrameTime()) / static_cast<float>(llParentFrameTime) * 100.0f : 100.0f : 0.0f;

                sStr = XtoA(i, FMT_NOPREFIX, 0, 36) + SPACE
                    + XtoA(tstring((*it)->GetName()).substr(0, PANEL_WIDTH - 39), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + SPACE
                    + XtoA(fPercentageOfParent, 0, 6, 2) + _T("% ")
                    + XtoA(float(prof_max ? (*it)->GetMaxFrameTime() : (*it)->GetFrameTime()) / FREQUENCY * 1000.0f, 0, 10, 3) + _T(" ms ")
                    + XtoA(prof_max ? (*it)->GetMaxFrameCalls() : (*it)->GetFrameCalls(), 0, 10);

                Draw2D.SetColor(0.0f, 1.0f, 0.0f, 1.0f);
                Draw2D.String(START_X, iDrawY, sStr, hProfilerFont);
                iDrawY += FONT_HEIGHT;
                ++i;

                llUnloggedTime -= (*it)->GetFrameTime();
            }
            else
            {
                sStr = XtoA(i, FMT_NOPREFIX, 0, 36) + SPACE
                    + XtoA(tstring((*it)->GetName()).substr(0, PANEL_WIDTH - 39), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + _T("     --  ")
                    + XtoA(float(prof_max ? (*it)->GetMaxFrameTime() : (*it)->GetFrameTime()) / FREQUENCY * 1000.0f, 0, 10, 3) + _T(" ms ")
                    + XtoA(prof_max ? (*it)->GetMaxFrameCalls() : (*it)->GetFrameCalls(), 0, 10);

                Draw2D.SetColor(0.0f, 1.0f, 0.0f, 1.0f);
                Draw2D.String(START_X, iDrawY, sStr, hProfilerFont);
                iDrawY += FONT_HEIGHT;
                ++i;
            }
        }
    }

    float   fPercentageOfParent = llUnloggedTime ? llParentFrameTime ? static_cast<float>(llUnloggedTime) / static_cast<float>(llParentFrameTime) * 100.0f : 100.0f : 0.0f;

    sStr = _T("  ")
        + XtoA(_T("Unlogged"), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + SPACE
        + XtoA(fPercentageOfParent, 0, 6, 2) + _T("% ")
        + XtoA(float(llUnloggedTime) / FREQUENCY * 1000.0f, 0, 10, 3) + _T(" ms");

    Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    Draw2D.String(START_X, iDrawY, sStr, hProfilerFont);
    iDrawY += FONT_HEIGHT;
}


/*====================
  CProfileManager::Print
  ====================*/
void    CProfileManager::Print()
{
    tstring sStr;

    const int   PANEL_WIDTH = 72;
    const LONGLONG FREQUENCY = K2System.GetFrequency();

    LONGLONG llFrameTime(0);
    const CProfileNode *pNode(m_pPrintNode->GetParent());
    if (pNode)
        while (pNode->GetParent() && pNode->GetParent()->GetType() == m_pPrintNode->GetType())
            pNode = pNode->GetParent();

    if (pNode)
        llFrameTime += pNode->GetFrameTime();

    if (llFrameTime)
    {
        sStr = _T("  ")
            + XtoA(m_pPrintNode->GetName(), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + SPACE
            + XtoA(float(m_pPrintNode->GetFrameTime()) / llFrameTime * 100.0f, 0, 6, 2) + _T("% ")
            + XtoA(float(m_pPrintNode->GetFrameTime()) / FREQUENCY * 1000.0f, 0, 10, 3) + _T(" ms ")
            + XtoA(m_pPrintNode->GetFrameCalls(), 0, 10);
    }
    else
    {
        sStr = _T("  ")
            + XtoA(m_pPrintNode->GetName(), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + SPACE
            + XtoA(float(m_pPrintNode->GetFrameTime()) / FREQUENCY * 1000.0f, 0, 18, 3) + _T(" ms ")
            + XtoA(m_pPrintNode->GetFrameCalls(), 0, 10);
    }

    Console.Std << sStr << newl;
    Console.Std << _T("==============================================================") << newl;
    Console.Std << _T("0 ..") << newl;

    int i = 1;

    const ProfileVector &vChildren(m_pPrintNode->GetChildren());

    LONGLONG    llParentFrameTime = m_pPrintNode->GetFrameTime();
    LONGLONG    llUnloggedTime = llParentFrameTime;

    for (ProfileVector::const_iterator(it) = vChildren.begin(); it != vChildren.end(); ++it)
    {
        if ((*it)->GetType() == m_pPrintNode->GetType())
        {
            float   fPercentageOfParent = (*it)->GetFrameTime() ? llParentFrameTime ? static_cast<float>((*it)->GetFrameTime()) / static_cast<float>(llParentFrameTime) * 100.0f : 100.0f : 0.0f;

            sStr = XtoA(i, FMT_NOPREFIX, 0, 36) + SPACE
                + XtoA(tstring((*it)->GetName()).substr(0, PANEL_WIDTH - 39), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + SPACE
                + XtoA(fPercentageOfParent, 0, 6, 2) + _T("% ")
                + XtoA(float((*it)->GetFrameTime()) / FREQUENCY * 1000.0f, 0, 10, 3) + _T(" ms ")
                + XtoA((*it)->GetFrameCalls(), 0, 10);

            Console.Std << sStr << newl;
            ++i;

            llUnloggedTime -= (*it)->GetFrameTime();
        }
        else
        {
            if ((*it)->GetFrame() == m_pPrintNode->GetFrame())
            {
                float   fPercentageOfParent = (*it)->GetFrameTime() ? llParentFrameTime ? static_cast<float>((*it)->GetFrameTime()) / static_cast<float>(llParentFrameTime) * 100.0f : 100.0f : 0.0f;

                sStr = XtoA(i, FMT_NOPREFIX, 0, 36) + SPACE
                    + XtoA(tstring((*it)->GetName()).substr(0, PANEL_WIDTH - 39), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + SPACE
                    + XtoA(fPercentageOfParent, 0, 6, 2) + _T("% ")
                    + XtoA(float((*it)->GetFrameTime()) / FREQUENCY * 1000.0f, 0, 10, 3) + _T(" ms ")
                    + XtoA((*it)->GetFrameCalls(), 0, 10);

                Console.Std << sStr << newl;
                ++i;

                llUnloggedTime -= (*it)->GetFrameTime();
            }
            else
            {
                sStr = XtoA(i, FMT_NOPREFIX, 0, 36) + SPACE
                    + XtoA(tstring((*it)->GetName()).substr(0, PANEL_WIDTH - 39), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + _T("     --  ")
                    + XtoA(float((*it)->GetFrameTime()) / FREQUENCY * 1000.0f, 0, 10, 3) + _T(" ms ")
                    + XtoA((*it)->GetFrameCalls(), 0, 10);

                Console.Std << sStr << newl;
                ++i;
            }
        }
    }

    float   fPercentageOfParent = llUnloggedTime ? llParentFrameTime ? static_cast<float>(llUnloggedTime) / static_cast<float>(llParentFrameTime) * 100.0f : 100.0f : 0.0f;

    sStr = _T("  ")
        + XtoA(_T("Unlogged"), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + SPACE
        + XtoA(fPercentageOfParent, 0, 6, 2) + _T("% ")
        + XtoA(float(llUnloggedTime) / FREQUENCY * 1000.0f, 0, 10, 3) + _T(" ms");

    Console.Std << sStr << newl;
}


/*====================
  CProfileManager::PrintMax
  ====================*/
void    CProfileManager::PrintMax()
{
    tstring sStr;

    const int   PANEL_WIDTH = 72;
    const LONGLONG FREQUENCY = K2System.GetFrequency();

    LONGLONG llFrameTime(0);
    const CProfileNode *pNode(m_pPrintNode->GetParent());
    if (pNode)
        while (pNode->GetParent() && pNode->GetParent()->GetType() == m_pPrintNode->GetType())
            pNode = pNode->GetParent();

    if (pNode)
        llFrameTime += pNode->GetMaxFrameTime();

    if (llFrameTime)
    {
        sStr = _T("  ")
            + XtoA(m_pPrintNode->GetName(), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + SPACE
            + XtoA(float(m_pPrintNode->GetMaxFrameTime()) / llFrameTime * 100.0f, 0, 6, 2) + _T("% ")
            + XtoA(float(m_pPrintNode->GetMaxFrameTime()) / FREQUENCY * 1000.0f, 0, 10, 3) + _T(" ms ")
            + XtoA(m_pPrintNode->GetMaxFrameCalls(), 0, 10);
    }
    else
    {
        sStr = _T("  ")
            + XtoA(m_pPrintNode->GetName(), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + SPACE
            + XtoA(float(m_pPrintNode->GetMaxFrameTime()) / FREQUENCY * 1000.0f, 0, 18, 3) + _T(" ms ")
            + XtoA(m_pPrintNode->GetMaxFrameCalls(), 0, 10);
    }

    Console << sStr << newl;
    Console << _T("==============================================================") << newl;
    Console << _T("0 ..") << newl;

    int i = 1;

    const ProfileVector &vChildren(m_pPrintNode->GetChildren());

    LONGLONG    llParentFrameTime = m_pPrintNode->GetMaxFrameTime();
    LONGLONG    llUnloggedTime = llParentFrameTime;

    for (ProfileVector::const_iterator it(vChildren.begin()); it != vChildren.end(); ++it)
    {
        if ((*it)->GetType() == m_pPrintNode->GetType())
        {
            float   fPercentageOfParent = (*it)->GetMaxFrameTime() ? llParentFrameTime ? static_cast<float>((*it)->GetMaxFrameTime()) / static_cast<float>(llParentFrameTime) * 100.0f : 100.0f : 0.0f;

            sStr = XtoA(i, FMT_NOPREFIX, 0, 36) + SPACE
                + XtoA(tstring((*it)->GetName()).substr(0, PANEL_WIDTH - 39), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + SPACE
                + XtoA(fPercentageOfParent, 0, 6, 2) + _T("% ")
                + XtoA(float((*it)->GetMaxFrameTime()) / FREQUENCY * 1000.0f, 0, 10, 3) + _T(" ms ")
                + XtoA((*it)->GetMaxFrameCalls(), 0, 10);

            Console << sStr << newl;
            ++i;

            llUnloggedTime -= (*it)->GetMaxFrameTime();
        }
        else
        {
            if ((*it)->GetFrame() == m_pPrintNode->GetFrame())
            {
                float   fPercentageOfParent = (*it)->GetMaxFrameTime() ? llParentFrameTime ? static_cast<float>((*it)->GetMaxFrameTime()) / static_cast<float>(llParentFrameTime) * 100.0f : 100.0f : 0.0f;

                sStr = XtoA(i, FMT_NOPREFIX, 0, 36) + SPACE
                    + XtoA(tstring((*it)->GetName()).substr(0, PANEL_WIDTH - 39), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + SPACE
                    + XtoA(fPercentageOfParent, 0, 6, 2) + _T("% ")
                    + XtoA(float((*it)->GetMaxFrameTime()) / FREQUENCY * 1000.0f, 0, 10, 3) + _T(" ms ")
                    + XtoA((*it)->GetMaxFrameCalls(), 0, 10);

                Console << sStr << newl;
                ++i;

                llUnloggedTime -= (*it)->GetMaxFrameTime();
            }
            else
            {
                sStr = XtoA(i, FMT_NOPREFIX, 0, 36) + SPACE
                    + XtoA(tstring((*it)->GetName()).substr(0, PANEL_WIDTH - 39), FMT_ALIGNLEFT, PANEL_WIDTH - 38) + _T("     --  ")
                    + XtoA(float((*it)->GetMaxFrameTime()) / FREQUENCY * 1000.0f, 0, 10, 3) + _T(" ms ")
                    + XtoA((*it)->GetMaxFrameCalls(), 0, 10);

                Console << sStr << newl;
                ++i;
            }
        }
    }
}


/*====================
  CProfileManager::ResetMax
  ====================*/
void    CProfileManager::ResetMax()
{
    if (m_pRootNode)
        m_pRootNode->ResetMax();
}


/*====================
  CProfileManager::SetNode
  ====================*/
void    CProfileManager::SetNode(TCHAR ch)
{
    if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z'))
    {
        int i(ch >= '0' && ch <= '9' ? ch - '0' : ch - 'a' + 10);

        if (i == 0)
        {
            if (m_pPrintNode->GetParent())
                m_pPrintNode = m_pPrintNode->GetParent();
        }
        else
        {
            ProfileVector vChildren(m_pPrintNode->GetChildren());

            for (ProfileVector::iterator it = vChildren.begin(); it != vChildren.end(); ++it)
            {
                --i;

                if (i == 0)
                {
                    m_pPrintNode = *it;
                    break;
                }
            }
        }
    }
}


/*--------------------
  cmdProfileStart
  --------------------*/
CMD(ProfileStart)
{
    ProfileManager.SetDraw(true);
    ProfileManager.SetActive(true);

    return true;
}


/*--------------------
  cmdProfileDraw
  --------------------*/
CMD(ProfileDraw)
{
    ProfileManager.SetDraw(true);

    return true;
}


/*--------------------
  cmdProfilePrint
  --------------------*/
CMD(ProfilePrint)
{
    ProfileManager.Print();

    return true;
}


/*--------------------
  cmdProfilePrintMax
  --------------------*/
CMD(ProfilePrintMax)
{
    ProfileManager.PrintMax();

    return true;
}


/*--------------------
  cmdProfileResetMax
  --------------------*/
CMD(ProfileResetMax)
{
    ProfileManager.ResetMax();

    return true;
}


/*--------------------
  cmdProfileNode
  --------------------*/
CMD(ProfileNode)
{
    if (vArgList.empty())
        return false;

    ProfileManager.SetNode(vArgList[0][0]);

    return true;
}

