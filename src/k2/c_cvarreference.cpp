// (C)2006 S2 Games
// c_cvarreference.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_cvarreference.h"
#include "c_cvar.h"
//=============================================================================

/*====================
  CCvarReference::~CCvarReference
  ====================*/
CCvarReference::~CCvarReference()
{
    if (m_pCvar != nullptr)
        m_pCvar->RemoveReference(this);
}


/*====================
  CCvarReference::CCvarReference
  ====================*/
CCvarReference::CCvarReference() :
m_pCvar(nullptr),
m_bIgnore(true)
{
}

CCvarReference::CCvarReference(ICvar *pCvar) :
m_pCvar(pCvar),
m_bIgnore(false)
{
    if (m_pCvar != nullptr)
        m_pCvar->AddReference(this);
}

CCvarReference::CCvarReference(const tstring &sName) :
m_pCvar(nullptr),
m_bIgnore(sName.empty())
{
    if (m_bIgnore)
        return;

    m_pCvar = ICvar::Find(sName);
    if (m_pCvar != nullptr)
        m_pCvar->AddReference(this);
}

CCvarReference::CCvarReference(const CCvarReference &A) :
m_pCvar(A.m_pCvar),
m_bIgnore(A.m_bIgnore)
{
    if (m_pCvar != nullptr)
        m_pCvar->AddReference(this);
}


/*====================
  CCvarReference::operator=
  ====================*/
CCvarReference& CCvarReference::operator=(const CCvarReference &A)
{
    if (m_pCvar != nullptr)
        m_pCvar->RemoveReference(this);
    m_pCvar = A.m_pCvar;
    if (m_pCvar != nullptr)
        m_pCvar->AddReference(this);

    return *this;
}


/*====================
  CCvarReference::Assign
  ====================*/
void    CCvarReference::Assign(ICvar *pCvar)
{
    if (m_pCvar != nullptr)
        m_pCvar->RemoveReference(this);
    m_pCvar = pCvar;
    if (m_pCvar != nullptr)
    {
        m_pCvar->AddReference(this);
        m_bIgnore = false;
    }
}

void    CCvarReference::Assign(const tstring &sName)
{
    if (m_pCvar != nullptr)
        m_pCvar->RemoveReference(this);
    m_pCvar = ICvar::Find(sName);
    if (m_pCvar != nullptr)
    {
        m_pCvar->AddReference(this);
        m_bIgnore = false;
    }
}


/*====================
  CCvarReference::Invalidate
  ====================*/
void    CCvarReference::Invalidate()
{
    //Console.Warn << _T("Referenced cvar has become invalid: ") << m_pCvar->GetName() << newl;
    m_pCvar = nullptr;
}
