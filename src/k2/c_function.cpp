// (C)2005 S2 Games
// c_function.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_function.h"
#include "c_consoleregistry.h"
#include "stringutils.h"
//=============================================================================

//=============================================================================
// Global Declarations
//=============================================================================
//=============================================================================


/*====================
  CFunction::CFunction
  ====================*/
CFunction::CFunction(const tstring &sName, FunctionFn_t pfnFunction) :
CConsoleElement(sName, 0, ELEMENT_FUNCTION, nullptr),
m_pfnFunction(pfnFunction)
{
    assert(m_pfnFunction != nullptr);
    ConsoleRegistry.Register(sName, this);
    ConsoleRegistry.AddFunction(this);
}


/*====================
  CFunction::CFunction
  ====================*/
CFunction::~CFunction()
{
    ConsoleRegistry.Unregister(this);
    ConsoleRegistry.RemoveFunction(this);
}


/*====================
  CFunction::operator()

  Calls the associated function directly
  ====================*/
tstring CFunction::operator()(tstring s0, tstring s1, tstring s2, tstring s3, tstring s4,
                         tstring s5, tstring s6, tstring s7, tstring s8, tstring s9)
{
    tsvector    vArgList(10);
    if (!s0.empty()) { vArgList.push_back(s0);
    if (!s1.empty()) { vArgList.push_back(s1);
    if (!s2.empty()) { vArgList.push_back(s2);
    if (!s3.empty()) { vArgList.push_back(s3);
    if (!s4.empty()) { vArgList.push_back(s4);
    if (!s5.empty()) { vArgList.push_back(s5);
    if (!s6.empty()) { vArgList.push_back(s6);
    if (!s7.empty()) { vArgList.push_back(s7);
    if (!s8.empty()) { vArgList.push_back(s8);
    if (!s9.empty()) { vArgList.push_back(s9);
    }}}}}}}}}}

    return m_pfnFunction(vArgList);
}


/*====================
  CFunction::operator()

  Calls the associated function even more directly
  ====================*/
tstring CFunction::operator()(const tsvector &vArgList)
{
    return m_pfnFunction(vArgList);
}


/*====================
  CFunction::IsFunction

  determines whether or not sString is a function by how it looks
  ====================*/
bool    CFunction::IsFunction(const tstring &sString)
{
    if (sString.empty())
        return false;

    if (!isalpha(sString[0]))
        return false;

    size_t  zStartPos = sString.find_first_of(_T('('));

    if (zStartPos > 0 && zStartPos != tstring::npos)
    {
        size_t zEndPos = sString.find_first_of(_T(')'), zStartPos);

        if (zEndPos == sString.length() - 1)
            return true;
    }

    return false;
}


/*====================
  CFunction::Parse

  determines whether or not sString is a function by how it looks
  ====================*/
bool    CFunction::Parse(const tstring &sString, tstring &sName, tsvector &vArgList)
{
    size_t  zEndName = sString.find_first_of(_T('('));

    if (zEndName < 1 || zEndName == tstring::npos)
        return false;

    sName = sString.substr(0, zEndName);

    size_t  zStartArgs = zEndName + 1;
    size_t  zEndArgs = sString.find_first_of(_T(')'), zStartArgs);

    if (zEndArgs == tstring::npos)
        return false;

    if (zEndArgs == zStartArgs)
        return true;

    size_t  zStartNextArg = zStartArgs;
    size_t  zEndNextArg = sString.find_first_of(_T(",)"), zStartNextArg);

    while (zEndNextArg != tstring::npos)
    {
        vArgList.push_back(sString.substr(zStartNextArg, zEndNextArg - zStartNextArg));

        zStartNextArg = zEndNextArg + 1;
        zEndNextArg = sString.find_first_of(_T(",)"), zStartNextArg);
    }

    return true;
}



/*--------------------
  cmdFunctionList

  Display a list of all the functions
  --------------------*/
CMD(FunctionList)
{
    int iNumFound(0);

    const ElementList &lFunctions = ConsoleRegistry.GetFunctionList();

    // loop through the cvar list
    for (ElementList::const_iterator it = lFunctions.begin(); it != lFunctions.end(); ++it)
    {
        if (vArgList.size() == 0 || (it->second)->GetName().find(vArgList[0]) != tstring::npos)
        {
            Console << (it->second)->GetName() << newl;
            ++iNumFound;
        }
    }

    Console << newl << iNumFound << _T(" matching functions found") << newl;
    return true;
}

