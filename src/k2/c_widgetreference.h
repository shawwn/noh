// (C)2007 S2 Games
// c_widgetreference.h
//
//=============================================================================
#ifndef __C_WIDGETREFERENCE_H__
#define __C_WIDGETREFERENCE_H__

//=============================================================================
// Declarations
//=============================================================================
class IWidget;
//=============================================================================

//=============================================================================
// CWidgetReference
//=============================================================================
class CWidgetReference
{
private:
    IWidget*    m_pTarget;
    IWidget*    m_pOwner;

    void    Invalidate();

    CWidgetReference();

    friend class IWidget;

public:
    ~CWidgetReference();
    CWidgetReference(IWidget *pOwner);
    CWidgetReference(const CWidgetReference &B);

    bool        IsValid() const                                     { return m_pTarget != nullptr; }
    IWidget*    GetTarget() const                                   { return m_pTarget; }

    CWidgetReference&   operator=(IWidget *pWidget);
    CWidgetReference&   operator=(const CWidgetReference &B);
    bool                operator==(const CWidgetReference &B) const { return m_pTarget == B.m_pTarget; }
    bool                operator<(const CWidgetReference &B) const  { return m_pTarget < B.m_pTarget; }
    IWidget*            operator->()                                { return m_pTarget; }
};
//=============================================================================

#endif //__C_WIDGETREFERENCE_H__
