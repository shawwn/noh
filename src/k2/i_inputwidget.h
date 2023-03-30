// (C)2005 S2 Games
// i_inputwidget.h
//
//=============================================================================
#ifndef __I_INPUTWIDGET_H__
#define __I_INPUTWIDGET_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
#include "c_widgetstyle.h"

//=============================================================================
// Declarations
//=============================================================================
class ICvar;
//=============================================================================

//=============================================================================
// IInputWidget
//=============================================================================
class IInputWidget : public IWidget
{
protected:
    tstring m_sInputLine;
    tstring m_sHiddenLine;
    tstring m_sPasswordChar;
    size_t  m_zInputPos;
    size_t  m_zStart;
    size_t  m_zEnd;
    uint    m_iNumLines;

    IInputWidget(CInterface *pInterface, IWidget *pParent, EWidgetType widgetType, const CWidgetStyle &style) :
    IWidget(pInterface, pParent, widgetType, style),
    m_sInputLine(_T("")),
    m_sHiddenLine(_T("")),
    m_sPasswordChar(style.GetProperty(_T("passwordchar"), _T(""))),
    m_zInputPos(0),
    m_zStart(0),
    m_zEnd(0),
    m_iNumLines(1)
    {
    }

public:
    virtual ~IInputWidget() {}

    const tstring&  GetInputLine()  { return m_sInputLine; }
    size_t          GetInputPos()   { return m_zInputPos; }
    size_t          GetInputSize()  { return m_sInputLine.size(); }

    virtual tstring GetValue() const    { return m_sInputLine; }

    void    AppendToInput(TCHAR chr)
    {
        m_sHiddenLine.append(1, chr);

        if (m_sPasswordChar.empty())
            m_sInputLine.append(1, chr);
        else
            m_sInputLine.append(1, m_sPasswordChar[0]);

        ++m_zInputPos;
        ++m_zEnd;

        DoEvent(WEVENT_CHANGE);
    }

    void    InsertIntoInput(TCHAR c)
    {
        m_sHiddenLine.insert(m_zInputPos, 1, c);

        if (m_sPasswordChar.empty())
            m_sInputLine.insert(m_zInputPos, 1, c);
        else
            m_sInputLine.insert(m_zInputPos, 1, m_sPasswordChar[0]);

        ++m_zInputPos;
        ++m_zEnd;

        DoEvent(WEVENT_CHANGE);
    }
};
//=============================================================================

#endif //__I_INPUTWIDGET_H__
