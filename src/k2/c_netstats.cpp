// (C)2007 S2 Games
// c_netstats.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_netstats.h"
#include "c_draw2d.h"
#include "c_input.h"
#include "c_fontmap.h"
#include "c_material.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CNetStats NetStats; // the global singleton

const TCHAR *g_aszNetStatModeNames[] =
{
    _T("Totals"),
    _T("Details")
};

const TCHAR *g_aszNetStatSocketNames[] =
{
    _T("Incoming"),
    _T("Outgoing")
};

const TCHAR *g_aszNetStatSampleTypeNames[] =
{
    _T("Snapshot"),
    _T("Gamedata")
};
//=============================================================================


/*====================
  CNetStats::CNetStats
  ====================*/
CNetStats::CNetStats() :
m_bActive(false),
m_bDraw(false),
m_bRecord(false),
m_eMode(NS_TOTALS)
{
    MemManager.Set(m_auiFrameTimeStamp, 0, sizeof(m_auiFrameTimeStamp));
    MemManager.Set(m_aiSocketBytes, 0, sizeof(m_aiSocketBytes));
    MemManager.Set(m_aiSampleTypeBytes, 0, sizeof(m_aiSampleTypeBytes));

    for (int iSocket(0); iSocket < NUM_NETSOCKET_TYPES; ++iSocket)
        m_iFrame[iSocket] = -1;
}


/*====================
  CNetStats::~CNetStats
  ====================*/
CNetStats::~CNetStats()
{
    m_bRecord = false;
    m_bActive = false;
}


/*====================
  CNetStats::RecordBytes
  ====================*/
void    CNetStats::RecordBytes(int iBytes, ENetStatSampleType eSampleType, ENetStatSocketType eSocketType)
{
    if (m_iFrame[eSocketType] < 0)
        return;

    m_aiSampleTypeBytes[m_iFrame[eSocketType]][eSocketType][eSampleType] += iBytes;
}


/*====================
  CNetStats::RecordPacket
  ====================*/
void    CNetStats::RecordPacket(ENetStatSocketType eSocketType, int iBytes)
{
    ++m_iFrame[eSocketType];
    if (m_iFrame[eSocketType] >= MAX_NETSTAT_FRAMES)
        m_iFrame[eSocketType] = 0;

    m_auiFrameTimeStamp[m_iFrame[eSocketType]][eSocketType] = Host.GetSystemTime();
    m_aiSocketBytes[m_iFrame[eSocketType]][eSocketType] = iBytes;
    MemManager.Set(m_aiSampleTypeBytes[m_iFrame[eSocketType]][eSocketType], 0, sizeof(m_aiSampleTypeBytes[m_iFrame[eSocketType]][eSocketType]));
}


/*====================
  CNetStats::Frame
  ====================*/
void    CNetStats::Frame()
{
    if (!m_bActive)
        return;

    // Steal all input while the profiler window is active
    while (!Input.IsEmpty())
    {
        const SIEvent &event = Input.Pop();
        switch(event.eType)
        {
        case INPUT_AXIS:
            break;

        case INPUT_BUTTON:
            // Filter out key releases
            if (event.cAbs.fValue == 0)
                break;

            switch(event.uID.btn)
            {
            case BUTTON_F1:
                m_bActive = false;
                break;
            case BUTTON_ESC:
                m_bDraw = m_bActive = false;
                break;
            default:
                break;
            }
            break;

        case INPUT_CHARACTER:
            if (event.uID.chr >= 32)
            {
                TCHAR key = event.uID.chr;

                if ((key >= '0' && key <= '9') || (key >= 'a' && key <= 'f'))
                {
                    int i = key >= '0' && key <= '9' ? key - '0' : key - 'a' + 10;

                    if (i >= 1 && i <= NUM_NETSTAT_MODES)
                        m_eMode = ENetStatMode(i - 1);
                }
            }
            break;

        default:
            break;
        }
    }
}


/*====================
  CNetStats::Draw
  ====================*/
void    CNetStats::Draw()
{
    if (!m_bDraw)
        return;

    PROFILE("CNetStats::Draw");

    ResHandle hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP));
    CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
    if (pFontMap == NULL)
        return;

    float fLines = 4.0f;

    switch (m_eMode)
    {
    case NS_TOTALS:
        fLines += NUM_NETSOCKET_TYPES + 1;
        break;
    case NS_DETAILS:
        fLines += NUM_NETSOCKET_TYPES * float(NUM_NETSAMPLE_TYPES + 2);
        break;
    case NUM_NETSTAT_MODES:
        K2_UNREACHABLE();
        break;
    }

    const float FONT_WIDTH = pFontMap->GetFixedAdvance();
    const float FONT_HEIGHT = pFontMap->GetMaxHeight();
    const int   PANEL_WIDTH = 46;
    const float PANEL_HEIGHT = fLines;
    const float START_X = INT_FLOOR(Draw2D.GetScreenW() - FONT_WIDTH * PANEL_WIDTH - FONT_WIDTH);
    const float START_Y = INT_FLOOR((Draw2D.GetScreenH() - FONT_HEIGHT * PANEL_HEIGHT) / 2.0f);

    // Calculate data per sec
    uint uiSystemTime(Host.GetSystemTime());
    int aiSocketPacketsPerSec[NUM_NETSOCKET_TYPES];
    int aiSocketBytesPerSec[NUM_NETSOCKET_TYPES];
    int aiSampleTypeBytesPerSec[NUM_NETSOCKET_TYPES][NUM_NETSAMPLE_TYPES];

    MemManager.Set(aiSocketPacketsPerSec, 0, sizeof(aiSocketPacketsPerSec));
    MemManager.Set(aiSocketBytesPerSec, 0, sizeof(aiSocketBytesPerSec));
    MemManager.Set(aiSampleTypeBytesPerSec, 0, sizeof(aiSampleTypeBytesPerSec));

    for (int iSocket(0); iSocket < NUM_NETSOCKET_TYPES; ++iSocket)
    {
        for (int iPos(0); iPos < MAX_NETSTAT_FRAMES; ++iPos)
        {
            if (m_auiFrameTimeStamp[iPos][iSocket] > uiSystemTime - 1000)
            {
                aiSocketPacketsPerSec[iSocket] += 1;
                aiSocketBytesPerSec[iSocket] += m_aiSocketBytes[iPos][iSocket];

                for (int iSampleType(0); iSampleType < NUM_NETSAMPLE_TYPES; ++iSampleType)
                {
                    aiSampleTypeBytesPerSec[iSocket][iSampleType] += m_aiSampleTypeBytes[iPos][iSocket][iSampleType];
                }
            }
        }
    }

    float fDrawY = START_Y;
    tstring sStr;

    Draw2D.SetColor(0.2f, 0.2f, 0.2f, 0.5f);
    Draw2D.Rect(START_X - 2, START_Y - 2, FONT_WIDTH * PANEL_WIDTH + 4, FONT_HEIGHT * PANEL_HEIGHT + 4);

    if (m_bActive)
    {
        Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        Draw2D.RectOutline(START_X - 2, START_Y - 2, FONT_WIDTH * PANEL_WIDTH + 4, FONT_HEIGHT * fLines + 4, 1);
    }

    Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    float fDrawX(START_X);

    for (int i(0); i < NUM_NETSTAT_MODES; ++i)
    {
        if (m_eMode == i)
            Draw2D.SetColor(0.0f, 1.0f, 0.0f, 1.0f);

        sStr = XtoA(i + 1) + _T(". ") + g_aszNetStatModeNames[i];
        Draw2D.String(fDrawX, fDrawY, sStr, hFont);
        fDrawX += (sStr.length() + 6) * FONT_WIDTH;

        if (m_eMode == i)
            Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    }

    fDrawY += FONT_HEIGHT * 2;

    Draw2D.String(START_X, fDrawY, _T("Type           Bytes     Packets/s Bytes/s  "), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, _T("============================================"), hFont);
    fDrawY += FONT_HEIGHT;

    switch (m_eMode)
    {
    case NS_TOTALS:
        {
            for (int i(0); i < NUM_NETSOCKET_TYPES; ++i)
            {
                sStr = XtoA(g_aszNetStatSocketNames[i], FMT_ALIGNLEFT, 14) + SPACE
                    + XtoA(m_aiSocketBytes[m_iFrame[i]][i], FMT_ALIGNLEFT, 9) + SPACE
                    + XtoA(aiSocketPacketsPerSec[i], FMT_ALIGNLEFT, 9) + SPACE
                    + XtoA(aiSocketBytesPerSec[i], FMT_ALIGNLEFT, 9);

                Draw2D.String(START_X, fDrawY, sStr, hFont);
                fDrawY += FONT_HEIGHT;
            }
        } break;
    case NS_DETAILS:
        {
            for (int i(0); i < NUM_NETSOCKET_TYPES; ++i)
            {
                sStr = g_aszNetStatSocketNames[i];
                Draw2D.String(START_X, fDrawY, sStr, hFont);
                fDrawY += FONT_HEIGHT;

                for (int j(0); j < NUM_NETSAMPLE_TYPES; ++j)
                {
                    sStr = XtoA(g_aszNetStatSampleTypeNames[j], FMT_ALIGNLEFT, 14) + SPACE
                        + XtoA(m_aiSampleTypeBytes[m_iFrame[j]][i][j], FMT_ALIGNLEFT, 9) + SPACE
                        + XtoA(_T(""), FMT_ALIGNLEFT, 9) + SPACE
                        + XtoA(aiSampleTypeBytesPerSec[i][j], FMT_ALIGNLEFT, 9);

                    Draw2D.String(START_X, fDrawY, sStr, hFont);
                    fDrawY += FONT_HEIGHT;
                }

                fDrawY += FONT_HEIGHT;
            }
        } break;
    case NUM_NETSTAT_MODES:
        K2_UNREACHABLE();
        break;
    }

    Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}


/*--------------------
  cmdNetStatsStart
  --------------------*/
CMD(NetStatsStart)
{
    NetStats.SetDraw(true);
    NetStats.SetActive(true);

    return true;
}


/*--------------------
  cmdNetStatsDraw
  --------------------*/
CMD(NetStatsDraw)
{
    NetStats.SetDraw(true);

    return true;
}


