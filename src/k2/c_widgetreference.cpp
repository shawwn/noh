// (C)2007 S2 Games
// c_widgetreference.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_widgetreference.h"
#include "i_widget.h"
//=============================================================================

/*====================
  CWidgetReference::~CWidgetReference
  ====================*/
CWidgetReference::~CWidgetReference()
{
    if (m_pTarget != nullptr)
        m_pTarget->RemoveReference(this);
}


/*====================
  CWidgetReference::CWidgetReference
  ====================*/
CWidgetReference::CWidgetReference(IWidget *pWidget) :
m_pTarget(nullptr),
m_pOwner(pWidget)
{
}

CWidgetReference::CWidgetReference(const CWidgetReference &B) :
m_pTarget(B.m_pTarget),
m_pOwner(B.m_pOwner)
{
    if (m_pTarget != nullptr)
        m_pTarget->AddReference(this);
}


/*====================
  CWidgetReference::Invalidate
  ====================*/
void    CWidgetReference::Invalidate()
{
    IWidget *pOldTarget(m_pTarget);

    if (m_pOwner != nullptr)
        m_pOwner->LostReference(m_pTarget);

    if (pOldTarget == m_pTarget)
        m_pTarget = nullptr;
}


/*====================
  CWidgetReferece::operator=
  ====================*/
CWidgetReference&   CWidgetReference::operator=(const CWidgetReference &B)
{
    if (m_pTarget != nullptr)
        m_pTarget->RemoveReference(this);
    
    m_pTarget = B.m_pTarget;
    if (m_pTarget != nullptr)
        m_pTarget->AddReference(this);

    return *this;
}

CWidgetReference&   CWidgetReference::operator=(IWidget *pWidget)
{
    if (m_pTarget != nullptr)
        m_pTarget->RemoveReference(this);
    
    m_pTarget = pWidget;
    if (m_pTarget != nullptr)
        m_pTarget->AddReference(this);

    return *this;
}
