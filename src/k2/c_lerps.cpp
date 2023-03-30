// (C)2010 S2 Games
// c_Lerps.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_lerps.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

CVAR_UINTF      (cg_healthLerpMode,                 2,          CVAR_SAVECONFIG);
CVAR_UINTF      (cg_healthLerpMode2MaxMultipler,    4,          CVAR_SAVECONFIG);
CVAR_FLOATF     (cg_healthLerpMode2Multipler,       0.333f,     CVAR_SAVECONFIG);

/*====================
  CLerpFloat::CLerpFloat
  ====================*/
CLerpFloat::CLerpFloat(const tstring &sValOut, float fTargetAmount, uint uiTargetTime, uint uiType, int iStyle) :
m_fTargetAmount(fTargetAmount),
m_uiLerpType(uiType),
m_uiLerpStyle(iStyle + 1),
m_sValOutName(sValOut),
m_uiTargetTime(Host.GetTime() + uiTargetTime),
m_uiStartTime(Host.GetTime()),
m_sValStart(m_sValOutName + _T("StartAmount"))
{
    tstring sTriggerName = m_sValOutName;
    sTriggerName += _T("Trigger");
    pUITrigger = UITriggerRegistry.GetUITrigger(sTriggerName);
    if (pUITrigger == NULL)
    {
        pUITrigger = K2_NEW(MemManager.GetHeap(HEAP_CLIENT_GAME),  CUITrigger)(sTriggerName);
    }
                
    if(ICvar::GetFloat(m_sValOutName)==0)
    {
        ICvar::CreateFloat(m_sValOutName, 0);
        ICvar::CreateFloat(m_sValStart, 0);
    }
    m_fValOut = ICvar::GetFloat(m_sValOutName);
    m_fStartAmount = ICvar::GetFloat(m_sValOutName);
    m_bDone = false;

}


/*====================
  CLerpFloat::Reset
  ====================*/
void CLerpFloat::Reset(float fNewTargetAmount, uint uiNewTargetTime, uint uiNewType, int iNewStyle)
{
    m_fTargetAmount = fNewTargetAmount;
    m_uiTargetTime = uiNewTargetTime + Host.GetTime();
    m_uiStartTime = Host.GetTime();
    m_uiLerpType = uiNewType;
    m_uiLerpStyle = iNewStyle + 1;
    m_fValOut = ICvar::GetFloat(m_sValOutName);
    m_fStartAmount = m_fValOut;
    ICvar::SetFloat(m_sValStart, m_fStartAmount);
    m_bDone = false;
}


/*====================
  CLerpFloat::Update
  ====================*/
void CLerpFloat::Update()
{
    if(m_bDone==true)
        return;

    if(Host.GetTime()>m_uiTargetTime)
    {
        m_fValOut = m_fTargetAmount;
        ICvar::SetFloat(m_sValOutName, m_fValOut);
        ICvar::SetFloat(m_sValStart, m_fValOut);
        if (pUITrigger)
                pUITrigger->Trigger(XtoA(m_fTargetAmount));
        m_bDone = true;
        return;
    }
    else
    {
        float fTimeLength = (m_uiTargetTime - m_uiStartTime);
        float fCurrentTime = (Host.GetTime() - m_uiStartTime);
        float fDistance = m_fTargetAmount - m_fStartAmount;
        m_fValOut  = (fDistance * (fCurrentTime / fTimeLength)) + m_fStartAmount;
        
        if((m_fTargetAmount < m_fStartAmount && m_uiLerpStyle == LERPSTYLE_PLUS_ONLY) || (m_fTargetAmount > m_fStartAmount && m_uiLerpStyle == LERPSTYLE_MINUS_ONLY))
        {
            m_fValOut = m_fTargetAmount;
        }

        ICvar::SetFloat(m_sValOutName, m_fValOut);
        if (pUITrigger)
            pUITrigger->Trigger(XtoA(m_fValOut));
    }
}


/*====================
  CSimpleLerp::CSimpleLerp
  ====================*/
CSimpleLerp::CSimpleLerp() :
m_fValOut(0.0f),
m_uiDelayedStart(Host.GetTime()),
m_fStartAmount(0.0f),
m_fTargetAmount(0.0f),
m_eLerpStyle(LERPSTYLE_NONE),
m_uiTargetTime(INVALID_TIME),
m_uiStartTime(INVALID_TIME),
m_bDone(false)
{
}

CSimpleLerp::CSimpleLerp(float fTargetAmount, uint uiTargetTime, ELerpStyle eStyle, uint uiDelayedStart) :
m_fValOut(0.0f),
m_uiDelayedStart(uiDelayedStart + Host.GetTime()),
m_fStartAmount(0.0f),
m_fTargetAmount(fTargetAmount),
m_eLerpStyle(eStyle),
m_uiTargetTime(uiDelayedStart + uiTargetTime),
m_uiStartTime(uiDelayedStart),
m_bDone(false)
{
}


/*====================
  CSimpleLerp::Reset
  ====================*/
void CSimpleLerp::Reset(float fNewTargetAmount, uint uiNewTargetTime, ELerpStyle eNewStyle, uint uiDelayedStart)
{
    float fLastChange(fNewTargetAmount - m_fTargetAmount);

    if (cg_healthLerpMode == 1)
    {
        if(m_bDone)
            m_uiDelayedStart = uiDelayedStart + Host.GetTime();
        else if(m_uiDelayedStart < Host.GetTime())
            m_uiDelayedStart = Host.GetTime();
    }
    else if (cg_healthLerpMode == 2)
    {
        if(m_bDone)
        {
            m_uiDelayedStart = uiDelayedStart + Host.GetTime();
        }
        else
        {
            bool bModeChange(false);
            if (m_eLerpStyle != eNewStyle || (fLastChange > 0.0f == (eNewStyle != LERPSTYLE_MINUS_ONLY)))
                bModeChange = true;

            if (bModeChange)
                m_uiDelayedStart += (uiDelayedStart * cg_healthLerpMode2Multipler);
            else if (m_uiDelayedStart < Host.GetTime())
                m_uiDelayedStart = Host.GetTime();

            if (m_uiDelayedStart - Host.GetTime() > (uiDelayedStart * cg_healthLerpMode2MaxMultipler))
                m_uiDelayedStart = (uiDelayedStart * cg_healthLerpMode2MaxMultipler) + Host.GetTime();

        }
    }
    else
    {
        m_uiDelayedStart = uiDelayedStart + Host.GetTime();
    }

    m_fTargetAmount = fNewTargetAmount;
    m_uiTargetTime = m_uiDelayedStart + uiNewTargetTime;
    m_uiStartTime = m_uiDelayedStart;
    m_eLerpStyle = eNewStyle;
    m_fStartAmount = m_fValOut;
    m_bDone = false;
}


/*====================
  CSimpleLerp::Update
  ====================*/
void    CSimpleLerp::Update()
{
    if (m_bDone)
    {
        m_fValOut = m_fTargetAmount;
        return;
    }

    if ((m_fTargetAmount < m_fStartAmount && m_eLerpStyle == LERPSTYLE_PLUS_ONLY) ||
        (m_fTargetAmount > m_fStartAmount && m_eLerpStyle == LERPSTYLE_MINUS_ONLY))
    {
        m_bDone = true;
        m_fValOut = m_fTargetAmount;
        return;
    }

    if (Host.GetTime() < m_uiDelayedStart)
        return;

    if (Host.GetTime() > m_uiTargetTime)
    {
        m_fValOut = m_fTargetAmount;
        m_fStartAmount = m_fTargetAmount;
        m_bDone = true;
        return;
    }
    else
    {
        float fTimeLength((m_uiTargetTime - m_uiStartTime));
        float fCurrentTime((Host.GetTime() - m_uiStartTime));
        float fDistance(m_fTargetAmount - m_fStartAmount);
        m_fValOut  = (fDistance * (fCurrentTime / fTimeLength)) + m_fStartAmount;
    }
}
