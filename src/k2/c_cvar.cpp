// (C)2005 S2 Games
// c_cvar.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_cvar.h"
#include "c_cmd.h"
#include "c_alias.h"
#include "c_consoleregistry.h"
#include "c_filehandle.h"
#include "c_xmldoc.h"
#include "c_filemanager.h"
#include "c_statestring.h"
#include "c_cvarreference.h"

#include <zlib.h>
//=============================================================================

//=============================================================================
//=============================================================================
bool    ICvar::s_bTransmitModified(false);
bool    ICvar::s_bServerInfoModified(false);
bool    ICvar::s_bTrackModifications(true);

bool    ICvar::s_bStoreCvars(true);
//=============================================================================

/*====================
  DefaultCvar_Cmd
 ====================*/
bool    DefaultCvar_Cmd(CConsoleElement *pElem, const tsvector &vArgList)
{
    assert(pElem->GetType() == ELEMENT_CVAR);   // don't pass non-cvars into this function!
    ICvar *pCvar = static_cast<ICvar *>(pElem);

    if (vArgList.empty())
    {
        switch(pCvar->GetType())
        {
        case CT_INT:
        case CT_UINT:
        case CT_FLOAT:
        case CT_BOOL:
        case CT_VEC3:
        case CT_VEC4:
            Console << pCvar->GetName() << _T(" is ") << pCvar->GetString() << newl;
            break;
        case CT_STRING:
        case CT_OTHER:
        default:
            Console << pCvar->GetName() << _T(" is \"") << pCvar->GetString() << _T("\"") << newl;
            break;
        }
    }
    else
    {
        if (pCvar->HasFlags(CVAR_READONLY)/* || (pCvar->HasFlags(CVAR_TRANSMIT) && !g_HostServer.active)*/)
        {
            Console << pCvar->GetName() << _T(" is a read - only value") << newl;
            return false;
        }

#if 0
#ifndef _DEBUG // FIXME: Need to figure out a better way to control protected cvars
        if (pCvar->HasFlags(CONEL_DEV))
        {
            Console << pCvar->GetName() << _T(" is a development value") << newl;
            return false;
        }
#endif
#endif

        pCvar->Set(ConcatinateArgs(vArgList));

        if (ICvar::StoreCvars())
            pCvar->StoreValue();
    }

    return true;
}


/*====================
  ICvar::ICvar
  ====================*/
ICvar::ICvar(const tstring &sName, int iFlags, ECvarType eType, ConsoleElementFn_t pfnCmd) :
CConsoleElement(sName, iFlags, ELEMENT_CVAR, pfnCmd),
m_eType(eType),
m_bModified(false),
m_bInherentValue(false),
m_pParent(NULL),
m_uiChildIndex(0)
{
    CMemManager::GetInstance(); // Ensure the memory manager is initialized
    MemManager.Set(m_pXYZW, 0, sizeof(m_pXYZW));
    MemManager.Set(m_pRGBA, 0, sizeof(m_pRGBA));
    MemManager.Set(m_apReferences, 0, sizeof(m_apReferences));

    try
    {
        if (m_pfnCmd == NULL)
            m_pfnCmd = DefaultCvar_Cmd;

        if (iFlags & CVAR_UNUSED)
            return;

        // If a dynamic cvar is in the way, replace it
        if (ConsoleRegistry.Exists(sName))
        {
            CConsoleElement *pElem(ConsoleRegistry.GetElement(sName));
            if (!pElem->HasFlags(CONEL_DYNAMIC))
                EX_ERROR(_T("Console element ") + QuoteStr(sName) + _T(" was already declared static"));
            if (pElem->GetType() != ELEMENT_CVAR)
                EX_ERROR(_T("Console element ") + QuoteStr(sName) + _T(" is not a cvar"));

            m_bInherentValue = true;
            m_sInherentValue = pElem->GetString();

            if (pElem->HasFlags(CVAR_SAVECONFIG))
                AddFlags(CVAR_SAVECONFIG);

            K2_DELETE(pElem);
        }

        ConsoleRegistry.Register(sName, this);
        ConsoleRegistry.AddCvar(this);

        if (m_eType == CT_VEC3 || m_eType == CT_VEC4)
        {
            m_pXYZW[X] = CreateFloat(sName + _T(".x"), 0.0f, m_iFlags | CVAR_CHILD);
            m_pXYZW[Y] = CreateFloat(sName + _T(".y"), 0.0f, m_iFlags | CVAR_CHILD);
            m_pXYZW[Z] = CreateFloat(sName + _T(".z"), 0.0f, m_iFlags | CVAR_CHILD);
            m_pRGBA[R] = CreateFloat(sName + _T(".r"), 0.0f, m_iFlags | CVAR_CHILD);
            m_pRGBA[G] = CreateFloat(sName + _T(".g"), 0.0f, m_iFlags | CVAR_CHILD);
            m_pRGBA[B] = CreateFloat(sName + _T(".b"), 0.0f, m_iFlags | CVAR_CHILD);

            m_pXYZW[X]->SetParent(this, X);
            m_pXYZW[Y]->SetParent(this, Y);
            m_pXYZW[Z]->SetParent(this, Z);
            m_pRGBA[R]->SetParent(this, R);
            m_pRGBA[G]->SetParent(this, G);
            m_pRGBA[B]->SetParent(this, B);
        }
        if (m_eType == CT_VEC4)
        {
            m_pXYZW[W] = CreateFloat(sName + _T(".w"), 0.0f, m_iFlags | CVAR_CHILD);
            m_pRGBA[A] = CreateFloat(sName + _T(".a"), 0.0f, m_iFlags | CVAR_CHILD);

            m_pXYZW[W]->SetParent(this, W);
            m_pRGBA[A]->SetParent(this, A);
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("ICvar::ICvar() - "), NO_THROW);
    }
}


/*====================
  ICvar::~ICvar
  ====================*/
ICvar::~ICvar()
{
    for (size_t z(0); z < MAX_REFERENCES_PER_CVAR; ++z)
    {
        if (m_apReferences[z] != NULL)
            m_apReferences[z]->Invalidate();
    }

    for (int i(0); i < 4; ++i)
    {
        SAFE_DELETE(m_pXYZW[i]);
        SAFE_DELETE(m_pRGBA[i]);
    }

    ConsoleRegistry.Unregister(this);
    ConsoleRegistry.RemoveCvar(this);
}


/*====================
  ICvar::GetFlagsString
  ====================*/
tstring ICvar::GetFlagsString()
{
    tstring sFlags;

    if (m_iFlags & CONEL_DEV)
        sFlags += _T("^yD");
    else
        sFlags += _T(" ");

    if (m_iFlags & CVAR_SAVECONFIG)
        sFlags += _T("^cS");
    else
        sFlags += _T(" ");

    if (m_iFlags & CVAR_READONLY)
        sFlags += _T("^rR");
    else
        sFlags += _T(" ");

    if (m_iFlags & CONEL_DYNAMIC)
        sFlags += _T("^wA");
    else
        sFlags += _T(" ");

    if (m_iFlags & CVAR_TRANSMIT)
        sFlags += _T("^mT");
    else
        sFlags += _T(" ");

    if (m_iFlags & CVAR_SERVERINFO)
        sFlags += _T("^gI");
    else
        sFlags += _T(" ");

    if (m_iFlags & CVAR_VALUERANGE)
        sFlags += _T("^mV");
    else
        sFlags += _T(" ");

    return sFlags;
}


/*====================
  ICvar::GetTypeName
  ====================*/
tstring ICvar::GetTypeName() const
{
    tstring sName;
    switch (m_eType)
    {
    case CT_INT:    sName = _T("^cint "); break;
    case CT_UINT:   sName = _T("^muint "); break;
    case CT_FLOAT:  sName = _T("^gfloat "); break;
    case CT_STRING: sName = _T("^rstring "); break;
    case CT_BOOL:   sName = _T("^ybool "); break;
    case CT_VEC3:   sName = _T("^bvec3 "); break;
    case CT_VEC4:   sName = _T("^bvec4 "); break;
    default:        sName = _T("^*unknown "); break;
    }

    if (m_iFlags & CVAR_VALUERANGE)
        sName += _T("range ");

    sName += _T("^*");
    sName = XtoA(sName, FMT_ALIGNLEFT, 18);
    return sName;
}


/*====================
  ICvar::Print
  ====================*/
void    ICvar::Print()
{
    Console << _T("^*[") << GetFlagsString() << _T("^*] ")
            << GetTypeName() << m_sName << _T(" = ")
            << QuoteStr(GetString()) << GetRangeString() << newl;
}


/*====================
  ICvar::Write
  ====================*/
void    ICvar::Write(CFileHandle &hFile, const tstring &sWildcard, int iFlags)
{
    if (HasFlags(CVAR_DONTSAVE|CVAR_CHILD))
        return;

    if (!sWildcard.empty())
    {
        if (CompareNum(GetName(), sWildcard, sWildcard.length()) == 0)
            hFile << _T("set ") << QuoteStr(AddEscapeChars(GetName())) << _T(" ") << QuoteStr(AddEscapeChars(GetString())) << newl;
    }
    else
    {
        if (HasAllFlags(iFlags))
        {
            if (HasFlags(CVAR_SAVECONFIG))
                hFile << _T("SetSave ") << QuoteStr(AddEscapeChars(GetName())) << _T(" ") << QuoteStr(AddEscapeChars(GetString())) << newl;
            else
                hFile << _T("Set ") << QuoteStr(AddEscapeChars(GetName())) << _T(" ") << QuoteStr(AddEscapeChars(GetString())) << newl;
        }
    }
}


/*====================
  ICvar::Write
  ====================*/
void    ICvar::Write(CXMLDoc &xmlConfig, const tstring &sWildcard, int iFlags)
{
    if (HasFlags(CVAR_DONTSAVE|CVAR_CHILD))
        return;

    if (!sWildcard.empty())
    {
        if (CompareNum(GetName(), sWildcard, sWildcard.length()) == 0)
        {
            xmlConfig.NewNode("cvar");
            xmlConfig.AddProperty("name", GetName());
            xmlConfig.AddProperty("value", GetString());
            xmlConfig.EndNode();
        }
    }
    else
    {
        if (HasAllFlags(iFlags))
        {
            xmlConfig.NewNode("cvar");

            xmlConfig.AddProperty("name", GetName());
            xmlConfig.AddProperty("value", GetString());

            if (HasFlags(CVAR_SAVECONFIG))
                xmlConfig.AddProperty("save", _T("true"));

            xmlConfig.EndNode();
        }
    }
}


/*====================
 ICvar::Write
 ====================*/
void    ICvar::Write(IBuffer &buffer, const tstring &sWildcard, int iFlags)
{
    tstring sLine;

    if (HasFlags(CVAR_DONTSAVE|CVAR_CHILD))
        return;

    if (!sWildcard.empty())
    {
        if (CompareNum(GetName(), sWildcard, sWildcard.length()) == 0)
            sLine = _T("set ") + QuoteStr(AddEscapeChars(GetName())) + _T(" ") + QuoteStr(AddEscapeChars(GetString())) + newl;
    }
    else
    {
        if (HasAllFlags(iFlags))
        {
            if (HasFlags(CVAR_SAVECONFIG))
                sLine = _T("setsave ") + QuoteStr(AddEscapeChars(GetName())) + _T(" ") + QuoteStr(AddEscapeChars(GetString())) + newl;
            else
                sLine = _T("set ") + QuoteStr(AddEscapeChars(GetName())) + _T(" ") + QuoteStr(AddEscapeChars(GetString())) + newl;
        }
    }

    buffer << sLine;
}


/*====================
  ICvar::Find
  ====================*/
ICvar   *ICvar::Find(const tstring &sName)
{
    CConsoleElement *pElem(ConsoleRegistry.GetElement(LowerString(sName)));

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
        return static_cast<ICvar*>(pElem);
    else
        return NULL;
}


/*====================
  ICvar::Create
  ====================*/
ICvar*  ICvar::Create(const tstring &sName, ECvarType eType, const tstring &sValue, int iFlags)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
    {
        ICvar *pCvar = static_cast<ICvar *>(pElem);

        pCvar->Set(sValue);
        pCvar->AddFlags(iFlags);
        return pCvar;
    }
    else if (pElem)
    {
        Console << _T("Console element ") << pElem->GetName() << _T(" already exists") << newl;
        return NULL;
    }
    else
    {
        if (eType == CT_INT)
            return K2_NEW(ctx_Console,  CCvar<int>)(sName, AtoI(sValue), iFlags | CONEL_DYNAMIC, NULL);
        else if (eType == CT_UINT)
            return K2_NEW(ctx_Console,  CCvar<uint>)(sName, AtoI(sValue), iFlags | CONEL_DYNAMIC, NULL);
        else if (eType == CT_FLOAT)
            return K2_NEW(ctx_Console,  CCvar<float>)(sName, AtoF(sValue), iFlags | CONEL_DYNAMIC, NULL);
        else if (eType == CT_STRING)
        {
            typedef CCvar<tstring,TCHAR>    CCvar_t;
            return K2_NEW(ctx_Console,  CCvar_t)(sName, sValue, iFlags | CONEL_DYNAMIC, NULL);
        }
        else if (eType == CT_BOOL)
            return K2_NEW(ctx_Console,  CCvar<bool>)(sName, AtoB(sValue), iFlags | CONEL_DYNAMIC, NULL);
        else if (eType == CT_VEC3)
        {
            typedef CCvar<CVec3f,float> CCvar_t;
            return K2_NEW(ctx_Console,  CCvar_t)(sName, AtoV3(sValue), iFlags | CONEL_DYNAMIC, NULL);
        }
        else if (eType == CT_VEC4)
        {
            typedef CCvar<CVec4f,float> CCvar_t;
            return K2_NEW(ctx_Console,  CCvar_t)(sName, AtoV4(sValue), iFlags | CONEL_DYNAMIC, NULL);
        }
        else
        {
            Console.Dev << _T("ICvar::Create: invalid type ") << static_cast<int>(eType) << newl;
            return NULL;
        }
    }
}


/*====================
  ICvar::CreateBool
  ====================*/
CCvarb* ICvar::CreateBool(const tstring &sName, bool bValue, int iFlags)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
    {
        ICvar *pCvar = static_cast<ICvar *>(pElem);
        if (pCvar->GetType() != CT_BOOL)
        {
            Console.Warn << _T("A cvar named ") << QuoteStr(sName) << _T(" already exists, but is not of type BOOL") << newl;
            return NULL;
        }

        pCvar->SetBool(bValue);
        pCvar->AddFlags(iFlags);
        return reinterpret_cast<CCvarb*>(pCvar);
    }
    else if (pElem)
    {
        Console << _T("Console element ") << pElem->GetName() << _T(" already exists") << newl;
        return NULL;
    }
    else
        return K2_NEW(ctx_Console,  CCvarb)(sName, bValue, iFlags | CONEL_DYNAMIC, NULL);
}


/*====================
  ICvar::CreateInt
  ====================*/
CCvari* ICvar::CreateInt(const tstring &sName, int iValue, int iFlags)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
    {
        ICvar *pCvar = static_cast<ICvar *>(pElem);
        if (pCvar->GetType() != CT_INT)
        {
            Console.Warn << _T("A cvar named ") << QuoteStr(sName) << _T(" already exists, but is not of type INT") << newl;
            return NULL;
        }

        pCvar->SetInteger(iValue);
        pCvar->AddFlags(iFlags);
        return reinterpret_cast<CCvari*>(pCvar);
    }
    else if (pElem)
    {
        Console << _T("Console element ") << pElem->GetName() << _T(" already exists") << newl;
        return NULL;
    }
    else
        return K2_NEW(ctx_Console,  CCvari)(sName, iValue, iFlags | CONEL_DYNAMIC, NULL);
}


/*====================
  ICvar::CreateUInt
  ====================*/
CCvarui*    ICvar::CreateUInt(const tstring &sName, uint uiValue, int iFlags)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
    {
        ICvar *pCvar = static_cast<ICvar *>(pElem);
        if (pCvar->GetType() != CT_UINT)
        {
            Console.Warn << _T("A cvar named ") << QuoteStr(sName) << _T(" already exists, but is not of type UINT") << newl;
            return NULL;
        }

        pCvar->SetUnsignedInteger(uiValue);
        pCvar->AddFlags(iFlags);
        return reinterpret_cast<CCvarui*>(pCvar);
    }
    else if (pElem)
    {
        Console << _T("Console element ") << pElem->GetName() << _T(" already exists") << newl;
        return NULL;
    }
    else
        return K2_NEW(ctx_Console,  CCvarui)(sName, uiValue, iFlags | CONEL_DYNAMIC, NULL);
}


/*====================
  ICvar::CreateFloat
  ====================*/
CCvarf* ICvar::CreateFloat(const tstring &sName, float fValue, int iFlags)
{
    CConsoleElement *pElem(ConsoleRegistry.GetElement(sName));

    if (pElem != NULL && pElem->GetType() == ELEMENT_CVAR)
    {
        ICvar *pCvar = static_cast<ICvar *>(pElem);
        if (pCvar->GetType() != CT_FLOAT)
        {
            Console.Warn << _T("A cvar named ") << QuoteStr(sName) << _T(" already exists, but is not of type FLOAT") << newl;
            return NULL;
        }

        pCvar->SetFloat(fValue);
        pCvar->AddFlags(iFlags);
        return reinterpret_cast<CCvarf*>(pCvar);
    }
    else if (pElem)
    {
        Console << _T("Console element ") << pElem->GetName() << _T(" already exists") << newl;
        return NULL;
    }

    return K2_NEW(ctx_Console,  CCvarf)(sName, fValue, iFlags | CONEL_DYNAMIC, NULL);
}


/*====================
  ICvar::CreateString
  ====================*/
CCvars* ICvar::CreateString(const tstring &sName, const tstring &sValue, int iFlags)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
    {
        ICvar *pCvar = static_cast<ICvar *>(pElem);
        if (pCvar->GetType() != CT_STRING)
        {
            Console.Warn << _T("A cvar named ") << QuoteStr(sName) << _T(" already exists, but is not of type STRING") << newl;
            return NULL;
        }

        pCvar->Set(sValue);
        pCvar->AddFlags(iFlags);
        return reinterpret_cast<CCvars*>(pCvar);
    }
    else if (pElem)
    {
        Console << _T("Console element ") << pElem->GetName() << _T(" already exists") << newl;
        return NULL;
    }
    else
        return K2_NEW(ctx_Console,  CCvars)(sName, sValue, iFlags | CONEL_DYNAMIC, NULL);
}


/*====================
  ICvar::CreateVec3
  ====================*/
CCvarv3*    ICvar::CreateVec3(const tstring &sName, const CVec3f &v3Value, int iFlags)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
    {
        ICvar *pCvar = static_cast<ICvar *>(pElem);
        if (pCvar->GetType() != CT_VEC3)
        {
            Console.Warn << _T("A cvar named ") << QuoteStr(sName) << _T(" already exists, but is not of type VEC3") << newl;
            return NULL;
        }

        pCvar->Set(XtoA(v3Value));
        pCvar->AddFlags(iFlags);
        return reinterpret_cast<CCvarv3*>(pCvar);
    }
    else if (pElem)
    {
        Console << _T("Console element ") << pElem->GetName() << _T(" already exists") << newl;
        return NULL;
    }
    else
        return K2_NEW(ctx_Console,  CCvarv3)(sName, v3Value, iFlags | CONEL_DYNAMIC, NULL);
}


/*====================
  ICvar::Import
  ====================*/
ICvar*  ICvar::Import(const tstring &sName, ECvarType eType, const tstring &sValue)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
    {
        return static_cast<ICvar *>(pElem);
    }
    else if (pElem)
    {
        Console << _T("Console element ") << pElem->GetName() << _T(" is not a cvar") << newl;
        return NULL;
    }
    else
    {
        Console.Warn << _T("Imported cvar ") << sName << _T(" doesn't exist, dynamically creating one") << newl;
        if (eType == CT_INT)
            return K2_NEW(ctx_Console,  CCvar<int>)(sName, AtoI(sValue), CONEL_DYNAMIC, NULL);
        else if (eType == CT_UINT)
            return K2_NEW(ctx_Console,  CCvar<uint>)(sName, AtoI(sValue), CONEL_DYNAMIC, NULL);
        else if (eType == CT_FLOAT)
            return K2_NEW(ctx_Console,  CCvar<float>)(sName, AtoF(sValue), CONEL_DYNAMIC, NULL);
        else if (eType == CT_STRING)
        {
            typedef CCvar<tstring,TCHAR>    CCvar_t;
            return K2_NEW(ctx_Console,  CCvar_t)(sName, sValue, CONEL_DYNAMIC, NULL);
        }
        else if (eType == CT_BOOL)
            return K2_NEW(ctx_Console,  CCvar<bool>)(sName, AtoB(sValue), CONEL_DYNAMIC, NULL);
        else if (eType == CT_VEC3)
        {
            typedef CCvar<CVec3f,float>     CCvar_t;
            return K2_NEW(ctx_Console,  CCvar_t)(sName, AtoV3(sValue), CONEL_DYNAMIC, NULL);
        }
        else if (eType == CT_VEC4)
        {
            typedef CCvar<CVec4f,float>     CCvar_t;
            return K2_NEW(ctx_Console,  CCvar_t)(sName, AtoV4(sValue), CONEL_DYNAMIC, NULL);
        }
        else
        {
            Console.Dev << _T("ICvar::Create: invalid type ") << static_cast<int>(eType) << newl;
            return NULL;
        }
    }
}


/*====================
  ICvar::ImportBool
  ====================*/
ICvar*  ICvar::ImportBool(const tstring &sName, bool bValue)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
    {
        if (static_cast<ICvar *>(pElem)->GetType() == CT_BOOL)
            return static_cast<ICvar *>(pElem);
        else
        {
            Console.Err << _T("Cvar ") << pElem->GetName() << _T(" is not of type bool") << newl;
            return NULL;
        }
    }
    else if (pElem)
    {
        Console.Err << _T("Console element ") << pElem->GetName() << _T(" is not a cvar") << newl;
        return NULL;
    }
    else
    {
        Console.Warn << _T("Imported cvar ") << sName << _T(" doesn't exist, dynamically creating one") << newl;
        return K2_NEW(ctx_Console,  CCvar<bool>)(sName, bValue, CONEL_DYNAMIC, NULL);
    }
}


/*====================
  ICvar::ImportInt
  ====================*/
ICvar*  ICvar::ImportInt(const tstring &sName, int iValue)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
    {
        if (static_cast<ICvar*>(pElem)->GetType() == CT_INT)
            return static_cast<ICvar*>(pElem);
        else
        {
            Console.Err << _T("Cvar ") << pElem->GetName() << _T(" is not of type int") << newl;
            return NULL;
        }
    }
    else if (pElem)
    {
        Console.Err << _T("Console element ") << pElem->GetName() << _T(" is not a cvar") << newl;
        return NULL;
    }
    else
    {
        Console.Warn << _T("Imported cvar ") << sName << _T(" doesn't exist, dynamically creating one") << newl;
        return K2_NEW(ctx_Console,  CCvar<int>)(sName, iValue, CONEL_DYNAMIC, NULL);
    }
}


/*====================
  ICvar::ImportUInt
  ====================*/
ICvar*  ICvar::ImportUInt(const tstring &sName, uint uiValue)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
    {
        if (static_cast<ICvar*>(pElem)->GetType() == CT_UINT)
            return static_cast<ICvar*>(pElem);
        else
        {
            Console.Err << _T("Cvar ") << pElem->GetName() << _T(" is not of type uint") << newl;
            return NULL;
        }
    }
    else if (pElem)
    {
        Console.Err << _T("Console element ") << pElem->GetName() << _T(" is not a cvar") << newl;
        return NULL;
    }
    else
    {
        Console.Warn << _T("Imported cvar ") << sName << _T(" doesn't exist, dynamically creating one") << newl;
        return K2_NEW(ctx_Console,  CCvar<uint>)(sName, uiValue, CONEL_DYNAMIC, NULL);
    }
}


/*====================
  ICvar::ImportFloat
  ====================*/
ICvar*  ICvar::ImportFloat(const tstring &sName, float fValue)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
    {
        if (static_cast<ICvar *>(pElem)->GetType() == CT_FLOAT)
            return static_cast<ICvar *>(pElem);
        else
        {
            Console.Err << _T("Cvar ") << pElem->GetName() << _T(" is not of type float") << newl;
            return NULL;
        }
    }
    else if (pElem)
    {
        Console.Err << _T("Console element ") << pElem->GetName() << _T(" is not a cvar") << newl;
        return NULL;
    }
    else
    {
        Console.Warn << _T("Imported cvar ") << sName << _T(" doesn't exist, dynamically creating one") << newl;
        return K2_NEW(ctx_Console,  CCvar<float>)(sName, fValue, CONEL_DYNAMIC, NULL);
    }
}


/*====================
  ICvar::ImportString
  ====================*/
ICvar*  ICvar::ImportString(const tstring &sName, const tstring &sValue)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
    {
        if (static_cast<ICvar *>(pElem)->GetType() == CT_STRING)
            return static_cast<ICvar *>(pElem);
        else
        {
            Console.Err << _T("Cvar ") << pElem->GetName() << _T(" is not of type string") << newl;
            return NULL;
        }
    }
    else if (pElem)
    {
        Console.Err << _T("Console element ") << pElem->GetName() << _T(" is not a cvar") << newl;
        return NULL;
    }
    else
    {
        Console.Warn << _T("Imported cvar ") << sName << _T(" doesn't exist, dynamically creating one") << newl;
        return K2_NEW(ctx_Console,  CCvar<tstring>)(sName, sValue, CONEL_DYNAMIC, NULL);
    }
}


/*====================
  ICvar::GetString
  ====================*/
tstring ICvar::GetString(const tstring &sName)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
        return static_cast<ICvar*>(pElem)->GetString();
    else
        return _T("");
}


/*====================
  ICvar::SetString
  ====================*/
void    ICvar::SetString(const tstring &sName, const tstring &s)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem == NULL)
    {
        Console.Warn << _T("Can not set cvar ") << sWhite << sName << sNoColor << _T(", it does not exist") << newl;
        return;
    }

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
        static_cast<ICvar *>(pElem)->Set(s);
}


/*====================
  ICvar::GetCvar
  ====================*/
ICvar*  ICvar::GetCvar(const tstring &sName)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
        return static_cast<ICvar *>(pElem);
    else
        return NULL;
}


/*====================
  ICvar::GetModifiedCount
  ====================*/
int     ICvar::GetModifiedCount(const tstring &sName)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
        return static_cast<ICvar*>(pElem)->GetModifiedCount();
    else
        return 0;
}


/*====================
  ICvar::GetValue
  ====================*/
float   ICvar::GetFloat(const tstring &sName)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
        return static_cast<ICvar*>(pElem)->GetFloat();
    else
        return 0.0f;
}


/*====================
  ICvar::SetFloat
  ====================*/
void    ICvar::SetFloat(const tstring &sName, float value)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    assert(pElem != NULL);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
        static_cast<ICvar*>(pElem)->SetFloat(value);
}


/*====================
  ICvar::GetInteger
  ====================*/
int     ICvar::GetInteger(const tstring &sName)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
        return static_cast<ICvar*>(pElem)->GetInteger();
    else
        return 0;
}


/*====================
  ICvar::GetUnsignedInteger
  ====================*/
uint        ICvar::GetUnsignedInteger(const tstring &sName)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
        return static_cast<ICvar*>(pElem)->GetUnsignedInteger();
    else
        return 0;
}


/*====================
  ICvar::SetInteger
  ====================*/
void    ICvar::SetInteger(const tstring &sName, int value)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem == NULL)
    {
        Console.Warn << _T("Can not set cvar ") << sWhite << sName << sNoColor << _T(", it does not exist") << newl;
        return;
    }

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
        static_cast<ICvar*>(pElem)->SetInteger(value);
}


/*====================
  ICvar::SetUnsignedInteger
  ====================*/
void    ICvar::SetUnsignedInteger(const tstring &sName, uint value)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem == NULL)
    {
        Console.Warn << _T("Can not set cvar ") << sWhite << sName << sNoColor << _T(", it does not exist") << newl;
        return;
    }

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
        static_cast<ICvar*>(pElem)->SetUnsignedInteger(value);
}


/*====================
  ICvar::GetBool
  ====================*/
bool    ICvar::GetBool(const tstring &sName)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
        return static_cast<ICvar*>(pElem)->GetBool();
    else
        return false;
}


/*====================
  ICvar::SetBool
  ====================*/
void    ICvar::SetBool(const tstring &sName, bool value)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);
    //assert(pElem != NULL);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
        static_cast<ICvar*>(pElem)->SetBool(value);
}


/*====================
  ICvar::GetVec3
  ====================*/
CVec3f  ICvar::GetVec3(const tstring &sName)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_CVAR)
        return static_cast<ICvar*>(pElem)->GetVec3();
    else
        return CVec3f(0.0f, 0.0f, 0.0f);
}


/*====================
  ICvar::SetVec3
  ====================*/
void    ICvar::SetVec3(const tstring &sName, const CVec3f &v3)
{
    try
    {
        CConsoleElement* pElem(ConsoleRegistry.GetElement(sName));
        if (pElem == NULL)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" not found"));

        if (pElem->GetType() != ELEMENT_CVAR)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" is not a cvar"));

        ICvar *pCvar(static_cast<ICvar*>(pElem));
        if (pCvar->GetType() == CT_VEC3)
        {
            static_cast<CCvar<CVec3f, float>*>(pCvar)->SetValue(v3);
        }
        else if (pCvar->GetType() == CT_VEC4)
        {
            CCvar<CVec4f, float>*   pCvarVec4(static_cast<CCvar<CVec4f, float>*>(pCvar));
            pCvarVec4->SetValue(CVec4f(v3.x, v3.y, v3.z, pCvarVec4->GetVec4().w));
        }
        else
        {
            EX_WARN(_T("Cvar ") + QuoteStr(sName) + _T(" is not a vector type"));
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("ICvar::SetVec3() - "), NO_THROW);
    }
}


/*====================
  ICvar::SetVec3
  ====================*/
void    ICvar::SetVec3(const tstring &sName, float fX, float fY, float fZ)
{
    try
    {
        CConsoleElement* pElem(ConsoleRegistry.GetElement(sName));
        if (pElem == NULL)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" not found"));

        if (pElem->GetType() != ELEMENT_CVAR)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" is not a cvar"));

        ICvar *pCvar(static_cast<ICvar*>(pElem));
        if (pCvar->GetType() == CT_VEC3)
        {
            static_cast<CCvar<CVec3f, float>*>(pCvar)->SetValue(CVec3f(fX, fY, fZ));
        }
        else if (pCvar->GetType() == CT_VEC4)
        {
            CCvar<CVec4f, float>*   pCvarVec4(static_cast<CCvar<CVec4f, float>*>(pCvar));
            pCvarVec4->SetValue(CVec4f(fX, fY, fZ, pCvarVec4->GetVec4().w));
        }
        else
        {
            EX_WARN(_T("Cvar ") + QuoteStr(sName) + _T(" is not a vector type"));
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("ICvar::SetVec3() - "), NO_THROW);
    }
}


/*====================
  ICvar::SetVec4
  ====================*/
void    ICvar::SetVec4(const tstring &sName, const CVec4f &v4)
{
    try
    {
        CConsoleElement* pElem(ConsoleRegistry.GetElement(sName));
        if (pElem == NULL)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" not found"));

        if (pElem->GetType() != ELEMENT_CVAR)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" is not a cvar"));

        ICvar *pCvar(static_cast<ICvar*>(pElem));
        if (pCvar->GetType() == CT_VEC3)
            static_cast<CCvar<CVec3f, float>*>(pCvar)->SetValue(CVec3f(v4.x, v4.y, v4.z));
        else if (pCvar->GetType() == CT_VEC4)
            static_cast<CCvar<CVec4f, float>*>(pCvar)->SetValue(v4);
        else
            EX_WARN(_T("Cvar ") + QuoteStr(sName) + _T(" is not a vector type"));
    }
    catch (CException &ex)
    {
        ex.Process(_T("ICvar::SetVec3() - "), NO_THROW);
    }
}


/*====================
  ICvar::SetVec4
  ====================*/
void    ICvar::SetVec4(const tstring &sName, float fX, float fY, float fZ, float fW)
{
    try
    {
        CConsoleElement* pElem(ConsoleRegistry.GetElement(sName));
        if (pElem == NULL)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" not found"));

        if (pElem->GetType() != ELEMENT_CVAR)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" is not a cvar"));

        ICvar *pCvar(static_cast<ICvar*>(pElem));
        if (pCvar->GetType() == CT_VEC3)
            static_cast<CCvar<CVec3f, float>*>(pCvar)->SetValue(CVec3f(fX, fY, fZ));
        else if (pCvar->GetType() == CT_VEC4)
            static_cast<CCvar<CVec4f, float>*>(pCvar)->SetValue(CVec4f(fX, fY, fZ, fW));
        else
            EX_WARN(_T("Cvar ") + QuoteStr(sName) + _T(" is not a vector type"));
    }
    catch (CException &ex)
    {
        ex.Process(_T("ICvar::SetVec3() - "), NO_THROW);
    }
}


/*====================
  ICvar::Toggle
  ====================*/
void    ICvar::Toggle(const tstring &sName)
{
    try
    {
        CConsoleElement* pElem(ConsoleRegistry.GetElement(sName));
        if (pElem == NULL)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" not found"));

        if (pElem->GetType() != ELEMENT_CVAR)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" is not a cvar"));

        ICvar *pCvar(static_cast<ICvar*>(pElem));
        pCvar->Toggle();
    }
    catch (CException &ex)
    {
        ex.Process(_T("ICvar::Toggle() - "), NO_THROW);
    }
}


/*====================
  ICvar::Reset
  ====================*/
void    ICvar::Reset(const tstring &sName)
{
    try
    {
        CConsoleElement* pElem(ConsoleRegistry.GetElement(sName));
        if (pElem == NULL)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" not found"));

        if (pElem->GetType() != ELEMENT_CVAR)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" is not a cvar"));

        ICvar *pCvar(static_cast<ICvar*>(pElem));
        pCvar->Reset();
    }
    catch (CException &ex)
    {
        ex.Process(_T("ICvar::Reset() - "), NO_THROW);
    }
}


/*====================
  ICvar::IsModified
  ====================*/
bool    ICvar::IsModified(const tstring &sName)
{
    try
    {
        CConsoleElement* pElem(ConsoleRegistry.GetElement(sName));
        if (pElem == NULL)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" not found"));

        if (pElem->GetType() != ELEMENT_CVAR)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" is not a cvar"));

        ICvar *pCvar(static_cast<ICvar*>(pElem));
        return pCvar->IsModified();
    }
    catch (CException &ex)
    {
        ex.Process(_T("ICvar::Reset() - "), NO_THROW);
        return false;
    }
}


/*====================
  ICvar::SetModified
  ====================*/
void    ICvar::SetModified(const tstring &sName, bool b)
{
    try
    {
        CConsoleElement* pElem(ConsoleRegistry.GetElement(sName));
        if (pElem == NULL)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" not found"));

        if (pElem->GetType() != ELEMENT_CVAR)
            EX_WARN(_T("Element ") + QuoteStr(sName) + _T(" is not a cvar"));

        ICvar *pCvar(static_cast<ICvar*>(pElem));
        pCvar->SetModified(b);
    }
    catch (CException &ex)
    {
        ex.Process(_T("ICvar::Reset() - "), NO_THROW);
    }
}


/*====================
  ICvar::GetFloat
  ====================*/
float   ICvar::GetFloat() const
{
    switch(this->GetType())
    {
    case CT_INT:
        return float(static_cast<const CCvar<int> *>(this)->GetValue());
    case CT_UINT:
        return float(static_cast<const CCvar<uint>*>(this)->GetValue());
    case CT_FLOAT:
        return static_cast<const CCvar<float> *>(this)->GetValue();
    case CT_BOOL:
        return static_cast<const CCvar<bool> *>(this)->GetValue() ? 1.0f : 0.0f;
    case CT_VEC3:
    case CT_VEC4:
        return 0.0f;
    default:
        return AtoF(GetString());
    }
}


/*====================
  ICvar::GetInteger
  ====================*/
int     ICvar::GetInteger() const
{
    switch(this->GetType())
    {
    case CT_INT:
        return static_cast<const CCvar<int> *>(this)->GetValue();
    case CT_UINT:
        Console.Warn << _T("Unsigned cvar ") << GetName() << _T(" accessed as signed data") << newl;
        return static_cast<const CCvar<uint>*>(this)->GetValue();
    case CT_FLOAT:
        return INT_FLOOR(static_cast<const CCvar<float> *>(this)->GetValue());
    case CT_BOOL:
        return static_cast<const CCvar<bool> *>(this)->GetValue() ? 1 : 0;
    case CT_VEC3:
    case CT_VEC4:
        return 0;
    default:
        const tstring &sValue(GetString());
        if (sValue == _T("true"))
            return 1;
        return AtoI(sValue);
    }
}


/*====================
  ICvar::GetUnsignedInteger
  ====================*/
uint    ICvar::GetUnsignedInteger() const
{
    switch(this->GetType())
    {
    case CT_INT:
        Console.Warn << _T("Signed cvar ") << GetName() << _T(" accessed as unsigend data") << newl;
        return static_cast<const CCvar<int> *>(this)->GetValue();
    case CT_UINT:
        return static_cast<const CCvar<uint>*>(this)->GetValue();
    case CT_FLOAT:
        return INT_FLOOR(static_cast<const CCvar<float> *>(this)->GetValue());
    case CT_BOOL:
        return static_cast<const CCvar<bool> *>(this)->GetValue() ? 1 : 0;
    case CT_VEC3:
    case CT_VEC4:
        return 0;
    default:
        return AtoI(GetString());
    }
}


/*====================
  ICvar::GetBool
  ====================*/
bool    ICvar::GetBool() const
{
    switch(this->GetType())
    {
    case CT_INT:
    case CT_UINT:
        return static_cast<const CCvar<int> *>(this)->GetValue() != 0;
    case CT_FLOAT:
        return static_cast<const CCvar<float> *>(this)->GetValue() != 0.0f;
    case CT_BOOL:
        return static_cast<const CCvar<bool> *>(this)->GetValue();
    case CT_VEC3:
    case CT_VEC4:
        return false;
    default:
        return AtoB(GetString());
    }
}


/*====================
  ICvar::GetVec3
  ====================*/
CVec3f  ICvar::GetVec3() const
{
    switch(GetType())
    {
    case CT_VEC3:
        return static_cast<const CCvar<CVec3f>*>(this)->GetValue();
    case CT_VEC4:
        {
            CVec4f v4(static_cast<const CCvar<CVec4f>*>(this)->GetValue());
            return CVec3f(v4.x, v4.y, v4.z);
        }
    default:
        return V_ZERO;
    }
}


/*====================
  ICvar::GetVec4
  ====================*/
CVec4f  ICvar::GetVec4() const
{
    switch(GetType())
    {
    case CT_VEC3:
        {
            CVec3f v3(static_cast<const CCvar<CVec3f>*>(this)->GetValue());
            return CVec4f(v3.x, v3.y, v3.z, 0.0f);
        }
    case CT_VEC4:
        return static_cast<const CCvar<CVec4f>*>(this)->GetValue();
    default:
        return V4_ZERO;
    }
}


/*====================
  ICvar::SetFloat
  ====================*/
void    ICvar::SetFloat(float value)
{
    switch(this->GetType())
    {
    case CT_INT:
        static_cast<CCvar<int> *>(this)->SetValue(INT_FLOOR(value));
        break;
    case CT_UINT:
        static_cast<CCvar<uint>*>(this)->SetValue(INT_FLOOR(value));
        break;
    case CT_FLOAT:
        static_cast<CCvar<float> *>(this)->SetValue(value);
        break;
    case CT_BOOL:
        static_cast<CCvar<bool> *>(this)->SetValue(value != 0.0f);
        break;
    case CT_VEC3:
    case CT_VEC4:
        break;
    default:
        this->Set(XtoA(value));
        break;
    }

    if (m_pParent != NULL)
    {
        if (m_pParent->GetType() == CT_VEC3)
            static_cast<CCvar<CVec3f, float>*>(m_pParent)->SetValueIndex(m_uiChildIndex, value);
        else if (m_pParent->GetType() == CT_VEC4)
            static_cast<CCvar<CVec4f, float>*>(m_pParent)->SetValueIndex(m_uiChildIndex, value);
    }
}


/*====================
  ICvar::SetInteger
  ====================*/
void    ICvar::SetInteger(int value)
{
    switch(this->GetType())
    {
    case CT_INT:
        static_cast<CCvar<int> *>(this)->SetValue(value);
        break;
    case CT_UINT:
        Console.Warn << _T("Signed value: ") << value << _T(" assigned to unsignend cvar: ") << GetName() << newl;
        static_cast<CCvar<uint>*>(this)->SetValue(value);
        break;
    case CT_FLOAT:
        static_cast<CCvar<float> *>(this)->SetValue(float(value));
        break;
    case CT_BOOL:
        static_cast<CCvar<bool> *>(this)->SetValue(value != 0);
        break;
    case CT_VEC3:
    case CT_VEC4:
        break;
    default:
        this->Set(XtoA(value));
        break;
    }
}


/*====================
  ICvar::SetUnsignedInteger
  ====================*/
void    ICvar::SetUnsignedInteger(uint value)
{
    switch(this->GetType())
    {
    case CT_INT:
        Console.Warn << _T("Unsigned value: ") << value << _T(" assigned to signend cvar: ") << GetName() << newl;
        static_cast<CCvar<int> *>(this)->SetValue(value);
        break;
    case CT_UINT:
        static_cast<CCvar<uint>*>(this)->SetValue(value);
        break;
    case CT_FLOAT:
        static_cast<CCvar<float> *>(this)->SetValue(float(value));
        break;
    case CT_BOOL:
        static_cast<CCvar<bool> *>(this)->SetValue(value != 0);
        break;
    case CT_VEC3:
    case CT_VEC4:
        break;
    default:
        this->Set(XtoA(value));
        break;
    }
}


/*====================
  ICvar::SetBool
  ====================*/
void    ICvar::SetBool(bool value)
{
    switch(this->GetType())
    {
    case CT_INT:
    case CT_UINT:
        static_cast<CCvar<int> *>(this)->SetValue(value ? 1 : 0);
        break;
    case CT_FLOAT:
        static_cast<CCvar<float> *>(this)->SetValue(value ? 1.0f : 0.0f);
        break;
    case CT_BOOL:
        static_cast<CCvar<bool> *>(this)->SetValue(value);
        break;
    case CT_VEC3:
    case CT_VEC4:
        break;
    default:
        this->Set(XtoA(value, true));
        break;
    }
}


/*====================
  ICvar::GetReference
  ====================*/
CCvarReference* ICvar::GetReference()
{
    return K2_NEW(ctx_Console,  CCvarReference)(this);
}


/*====================
  ICvar::AddReference
  ====================*/
void    ICvar::AddReference(CCvarReference *pRef)
{
    for (size_t z(0); z < MAX_REFERENCES_PER_CVAR; ++z)
    {
        if (m_apReferences[z] == pRef)
            return;
    }

    for (size_t z(0); z < MAX_REFERENCES_PER_CVAR; ++z)
    {
        if (m_apReferences[z] != NULL)
            continue;

        m_apReferences[z] = pRef;
        return;
    }

    Console.Err << _T("ICvar::AddReference() - Could not find a free reference slot for cvar: ") << m_sName << newl;
}


/*====================
  ICvar::RemoveReference
  ====================*/
void    ICvar::RemoveReference(CCvarReference *pRef)
{
    bool bSuccess(false);

    for (size_t z(0); z < MAX_REFERENCES_PER_CVAR; ++z)
    {
        if (m_apReferences[z] == pRef)
        {
            m_apReferences[z]->Invalidate();
            m_apReferences[z] = NULL;
            bSuccess = true;
        }
    }

    if (!bSuccess)
        Console.Warn << _T("ICvar::RemoveReference() - Unknown reference for cvar: ") << m_sName << newl;
}


/*====================
  ICvar::WriteConfigFile
  ====================*/
bool    ICvar::WriteConfigFile(CFileHandle &hFile, const tsvector &wildcards, int iFlags)
{
    if (wildcards.empty())
    {
        // write variables
        const CvarList &lCvars = ConsoleRegistry.GetCvarList();

        // loop through the cvar list
        for (CvarList_cit it(lCvars.begin()); it != lCvars.end(); ++it)
            it->second->Write(hFile, _T(""), iFlags);
    }
    else
    {
        const CvarList &lCvars = ConsoleRegistry.GetCvarList();

        for (size_t i(0); i < wildcards.size(); ++i)
        {
            // write variables
            for (CvarList_cit it(lCvars.begin()); it != lCvars.end(); ++it)
                it->second->Write(hFile, wildcards[i], iFlags);
        }
    }

    return true;
}


/*====================
  ICvar::WriteConfigFile
  ====================*/
bool    ICvar::WriteConfigFile(CXMLDoc &xmlConfig, const tsvector &wildcards, int iFlags)
{
    if (wildcards.empty())
    {
        //write variables
        const CvarList &lCvars = ConsoleRegistry.GetCvarList();

        // loop through the cvar list
        for (CvarList_cit it(lCvars.begin()); it != lCvars.end(); ++it)
            it->second->Write(xmlConfig, _T(""), iFlags);
    }
    else
    {
        const CvarList &lCvars = ConsoleRegistry.GetCvarList();

        for (size_t i(0); i < wildcards.size(); ++i)
        {
            //write variables
            for (CvarList_cit it(lCvars.begin()); it != lCvars.end(); ++it)
                it->second->Write(xmlConfig, wildcards[i], iFlags);
        }
    }

    return true;
}

/*====================
  ICvar::WriteConfigFile
  ====================*/
bool    ICvar::WriteConfigFile(IBuffer &buffer, const tsvector &wildcards, int iFlags, bool bWriteBinds)
{
    tstring sLine;

    if (wildcards.empty())
    {
        const CvarList &lCvars = ConsoleRegistry.GetCvarList();

        for (CvarList_cit it(lCvars.begin()); it != lCvars.end(); ++it)
            it->second->Write(buffer, _T(""), iFlags);
    }
    else
    {
        const CvarList &lCvars = ConsoleRegistry.GetCvarList();

        for (size_t i(0); i < wildcards.size(); ++i)
        {
            for (CvarList_cit it(lCvars.begin()); it != lCvars.end(); ++it)
                it->second->Write(buffer, wildcards[i], iFlags);
        }
    }

    return true;
}


/*====================
  SetTransmitCvar
  ====================*/
void    SetTransmitCvar(const string &sStateUTF8, const string &sValueUTF8)
{
    tstring sState(UTF8ToTString(sStateUTF8));
    tstring sValue(UTF8ToTString(sValueUTF8));

    ICvar *pCvar(ICvar::GetCvar(sState));
    if (pCvar == NULL)
        ICvar::CreateString(sState, sValue, CVAR_READONLY);
    else
        pCvar->Set(sValue);
}


/*====================
  ICvar::ModifyTransmitCvars
 ====================*/
void    ICvar::ModifyTransmitCvars(const CStateString *pStateString)
{
    PROFILE("ICvar::SetTransmitCvars");

    pStateString->ForEachState(SetTransmitCvar, false);
}


/*====================
  ICvar::SetTransmitCvars
 ====================*/
void    ICvar::SetTransmitCvars(const CStateString *pStateString)
{
    PROFILE("ICvar::SetTransmitCvars");

    const CvarList &lTransmitCvars(ConsoleRegistry.GetCvarList(CVAR_TRANSMIT));

    for (CvarList::const_iterator it = lTransmitCvars.begin(); it != lTransmitCvars.end(); ++it)
    {
        tstring sVarName(it->second->GetName());
        if (pStateString->HasState(sVarName))
        {
            it->second->AddFlags(CVAR_READONLY);
            it->second->Set(pStateString->GetString(sVarName));
        }
        else
            it->second->Reset();
    }

    pStateString->ForEachState(SetTransmitCvar, false);
}


/*====================
  ICvar::GetTransmitCvars
  ====================*/
void    ICvar::GetTransmitCvars(CStateString &ss)
{
    PROFILE("ICvar::GetTransmitCvars");

    ss.Clear();

    const CvarList &lTransmitCvars(ConsoleRegistry.GetCvarList(CVAR_TRANSMIT));
    for (CvarList_cit cit(lTransmitCvars.begin()); cit != lTransmitCvars.end(); ++cit)
    {
        if (!cit->second->IsDefault() || cit->second->IsModified())
            ss.Set(cit->second->GetName(), cit->second->GetString());
    }
}


/*====================
  ICvar::UnprotectTransmitCvars
  ====================*/
void    ICvar::UnprotectTransmitCvars()
{
    const CvarList &lTransmitCvars(ConsoleRegistry.GetCvarList(CVAR_TRANSMIT));
    for (CvarList_cit cit(lTransmitCvars.begin()); cit != lTransmitCvars.end(); ++cit)
    {
        if (cit->second->HasAllFlags(CVAR_READONLY | CVAR_TRANSMIT))
            cit->second->RemoveFlags(CVAR_READONLY);
    }
}


/*====================
  ICvar::ProtectTransmitCvars
  ====================*/
void    ICvar::ProtectTransmitCvars()
{
    const CvarList &lTransmitCvars(ConsoleRegistry.GetCvarList(CVAR_TRANSMIT));
    for (CvarList_cit cit(lTransmitCvars.begin()); cit != lTransmitCvars.end(); ++cit)
    {
        if (cit->second->HasAllFlags(CVAR_TRANSMIT))
            cit->second->AddFlags(CVAR_READONLY);
    }
}


/*====================
  ICvar::GetServerInfo
  ====================*/
void    ICvar::GetServerInfo(CStateString &ss)
{
    PROFILE("ICvar::GetServerInfo");

    const CvarList &lServerInfoCvars = ConsoleRegistry.GetCvarList(CVAR_SERVERINFO);
    for (CvarList_cit it = lServerInfoCvars.begin(); it != lServerInfoCvars.end(); ++it)
        ss.Set(it->second->GetName(), it->second->GetString());
}


/*====================
  ICvar::ResetVars
  ====================*/
void    ICvar::ResetVars(int iFlags)
{
    const CvarList &lCvars = ConsoleRegistry.GetCvarList();

    for (CvarList_cit it(lCvars.begin()); it != lCvars.end(); ++it)
    {
        if (it->second->HasAllFlags(iFlags))
            it->second->Reset();
    }
}


/*--------------------
  cmdSet

  Set the value of a cvar
  --------------------*/
CMD(Set)
{
    if (!vArgList.size())
    {
        Console << _T("syntax: set <variable> <value>") << newl;
        return false;
    }

    ICvar *pCvar(ICvar::Find(vArgList[0]));

    if (!pCvar)
    {
        //Console << _T("\"") << vArgList[0] << _T("\" not found") << newl;
        ICvar::CreateString(vArgList[0], vArgList.size() > 1 ? ConcatinateArgs(vArgList.begin() + 1, vArgList.end()) : _T(""));
        return false;
    }

    // Send second arg on to DefaultCvar_Cmd
    tsvector vArgList2;

    vArgList2.push_back(vArgList.size() > 1 ? ConcatinateArgs(vArgList.begin() + 1, vArgList.end()) : _T(""));

    pCvar->Execute(vArgList2);
    return true;
}


/*--------------------
  cmdSetSave

  Set the value of a cvar and flag it to be saved when a config
  file is written
  --------------------*/
CMD(SetSave)
{
    if (!vArgList.size())
    {
        Console << _T("syntax: setsave <variable> <value>") << newl;
        return false;
    }

    cmdSet(vArgList);

    ICvar* pCvar(ICvar::Find(vArgList[0]));
    if (pCvar)
        pCvar->AddFlags(CVAR_SAVECONFIG);

    return true;
}


/*--------------------
  cmdInc

  increment a cvar
  --------------------*/
CMD(Inc)
{
    if (!vArgList.size())
    {
        Console << _T("syntax: inc <variable> <value>") << newl;
        return false;
    }

    ICvar *var = ICvar::Find(vArgList[0]);
    if (!var)
    {
        Console << _T("\"") << vArgList[0] << _T("\" not found") << newl;
        return false;
    }
    else
        var->Inc(vArgList.size() > 1 ? vArgList[1] : _T("1"));

    return true;
}


/*--------------------
  cmdToggle

  toggle a cvar between 0 and 1
  --------------------*/
CMD(Toggle)
{
    if (!vArgList.size())
    {
        Console << _T("syntax: toggle <variable>") << newl;
        return false;
    }

    ICvar *var = ICvar::Find(vArgList[0]);
    if (!var)
    {
        Console << SingleQuoteStr(vArgList[0]) << _T(" not found") << newl;
        return false;
    }
    else
        var->Toggle();

    return true;
}


/*--------------------
  cmdReset

  resets a cvar to its default value
  "all" causes all cvars to be reset
  --------------------*/
CMD(Reset)
{
    if (!vArgList.size())
    {
        Console << _T("syntax: reset <variable> | all") << newl;
        return false;
    }

    if (CompareNoCase(vArgList[0], _T("all")) == 0)
    {
        const CvarList &lCvars = ConsoleRegistry.GetCvarList();

        // loop through the cvar list
        for (CvarList_cit it = lCvars.begin(); it != lCvars.end(); ++it)
            it->second->Reset();
    }
    else
    {
        ICvar *var = ICvar::Find(vArgList[0]);
        if (!var)
        {
            Console << _T("\"") << vArgList[0] << _T("\" not found") << newl;
            return false;
        }
        else
            var->Reset();
    }

    return true;
}


/*--------------------
  cmdCreateVar

  Create a new cvar
  --------------------*/
CMD(CreateVar)
{
    if (vArgList.size() < 2)
    {
        Console << _T("syntax: createvar <type> <variable> [initial value]") << newl;
        return false;
    }

    ICvar *pOldVar = ICvar::Find(vArgList[1]);
    if (pOldVar)
    {
        if (vArgList.size() > 2)
        {
            if (pOldVar->HasFlags(CVAR_READONLY))
            {
                Console << pOldVar->GetName() << _T(" is already registered and is write protected") << newl;
                return false;
            }

#ifndef _DEBUG // not right...
            if (pOldVar->HasFlags(CONEL_DEV))
            {
                Console << pOldVar->GetName() << _T(" is already registered and is a development variable") << newl;
                return false;
            }
#endif

            //just set the value
            pOldVar->Set(vArgList[2]);
        }
        return true;
    }
    else
    {
        CConsoleElement *pElem = ConsoleRegistry.GetElement(vArgList[1]);

        if (pElem)
            Console << _T("Console element ") << pElem->GetName() << _T(" already exists") << newl;

        const tstring &sType = vArgList[0];
        const tstring &sName = vArgList[1];
        ICvar *pCvar;

        if (sType == _T("int"))
            pCvar = K2_NEW(ctx_Console,  CCvar<int>)(sName, vArgList.size() > 2 ? AtoI(vArgList[2]) : 0, CONEL_DYNAMIC, NULL);
        else if (sType == _T("float"))
            pCvar = K2_NEW(ctx_Console,  CCvar<float>)(sName, vArgList.size() > 2 ? AtoF(vArgList[2]) : 0.0f, CONEL_DYNAMIC, NULL);
        else if (sType == _T("string"))
            pCvar = K2_NEW(ctx_Console,  CCvar<tstring>)(sName, vArgList.size() > 2 ? vArgList[2] : _T(""), CONEL_DYNAMIC, NULL);
        else if (sType == _T("bool"))
            pCvar = K2_NEW(ctx_Console,  CCvar<bool>)(sName, vArgList.size() > 2 ? AtoB(vArgList[2]) : false, CONEL_DYNAMIC, NULL);
        else
        {
            Console << _T("syntax error: invalid type ") << QuoteStr(sType) << newl;
            return false;
        }
    }

    return true;
}


/*--------------------
  cmdCvarList

  Lists all exisiting cvars
  --------------------*/
CMD(CvarList)
{
    int iNumFound(0);

    const CvarList &lCvars = ConsoleRegistry.GetCvarList();

    // loop through the cvar list
    for (CvarList_cit it = lCvars.begin(); it != lCvars.end(); ++it)
    {
        if (it->second->HasFlags(CVAR_CHILD))
            continue;

        if (!vArgList.empty())
        {
            tstring sName(LowerString(it->second->GetName()));
            if (sName.find(LowerString(vArgList[0])) == tstring::npos)
                continue;
        }

        it->second->Print();
        ++iNumFound;
    }

    Console << iNumFound << _T(" matching variables found") << newl;

    Console << _T("Flags:") << newl
            << sYellow  << _T("D        ") << sNoColor << _T("Development variable") << newl
            << sCyan    << _T("S        ") << sNoColor << _T("Saved to config file") << newl
            << sRed     << _T("R        ") << sNoColor << _T("Read only") << newl
            << sWhite   << _T("A        ") << sNoColor << _T("Dynamically allocated") << newl
            << sMagenta << _T("T        ") << sNoColor << _T("Server dictated setting") << newl
            << sGreen   << _T("I        ") << sNoColor << _T("Server info") << newl
            << sBlue    << _T("N        ") << sNoColor << _T("Local client network setting") << newl
            << sMagenta << _T("V        ") << sNoColor << _T("Value range") << newl;

    return true;
}


/*--------------------
  cmdCvarInfo
  --------------------*/
CMD(CvarInfo)
{
    Console << _T("Total cvars: ") << (uint)ConsoleRegistry.GetCvarList().size() << newl;
    return true;
}
