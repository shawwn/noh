// (C)2005 S2 Games
// c_toolbox.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "c_toolbox.h"
#include "i_tool.h"

#include "../k2/c_uicmd.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CToolBox)
//=============================================================================

/*====================
  CToolBox::~CToolBox
  ====================*/
CToolBox::~CToolBox()
{
    for (ToolNameMap::iterator it(m_mapTools.begin()); it != m_mapTools.end(); ++it)
    {
        if (it->second != NULL)
            delete it->second;
    }
}


/*====================
  CToolBox::CToolBox
  ====================*/
CToolBox::CToolBox() :
m_pCurrentTool(NULL)
{
}


/*====================
  CToolBox::Register
 ====================*/
bool    CToolBox::Register(ITool *pTool)
{
    try
    {
        if (pTool == NULL)
            EX_ERROR(_T("Tried to register a NULL pointer"));

        ToolNameMap::iterator itName(m_mapTools.find(pTool->GetName()));
        if (itName != m_mapTools.end())
            EX_WARN(_T("Tried to add duplicate tool: ") + pTool->GetName());

        ToolIDMap::iterator itID(m_mapToolIDs.find(pTool->GetID()));
        if (itID != m_mapToolIDs.end())
            EX_WARN(_T("Tried to add duplicate tool ID: ") + XtoA(pTool->GetID()));

        m_mapTools[pTool->GetName()] = pTool;
        m_mapToolIDs[pTool->GetID()] = pTool;
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CToolBox::Register() - "), NO_THROW);
        return false;
    }
}


/*====================
  CToolBox::SelectTool
 ====================*/
ITool*  CToolBox::SelectTool(const tstring &sName)
{
    if (m_pCurrentTool)
        m_pCurrentTool->Leave();
    
    if (sName == _T("none"))
    {
        m_pCurrentTool = NULL;
        return m_pCurrentTool;
    }

    ToolNameMap::iterator findit(m_mapTools.find(sName));

    if (findit == m_mapTools.end())
        return NULL;

    m_pCurrentTool = findit->second;
    m_pCurrentTool->Enter();
    return m_pCurrentTool;
}

ITool*  CToolBox::SelectTool(EToolID eTool)
{
    if (m_pCurrentTool)
        m_pCurrentTool->Leave();

    ToolIDMap::iterator findit(m_mapToolIDs.find(eTool));
    if (findit == m_mapToolIDs.end())
        m_pCurrentTool = NULL;
    else
        m_pCurrentTool = findit->second;

    if (m_pCurrentTool)
        m_pCurrentTool->Enter();
    return m_pCurrentTool;
}


/*====================
  CToolBox::GetTool
  ====================*/
ITool*  CToolBox::GetTool(const tstring &sName, bool bThrow)
{
    try
    {
        ToolNameMap::iterator findit(m_mapTools.find(sName));
        if (findit == m_mapTools.end())
            EX_WARN(_T("Couldn't find tool named ") + QuoteStr(sName));

        return findit->second;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CToolBox::GetTool() - "), !bThrow);
        return NULL;
    }
}

ITool*  CToolBox::GetTool(EToolID eTool, bool bThrow)
{
    try
    {
        ToolIDMap::iterator findit(m_mapToolIDs.find(eTool));
        if (findit == m_mapToolIDs.end())
            EX_WARN(_T("Couldn't find tool #") + XtoA(eTool));

        return findit->second;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CToolBox::GetTool() - "), !bThrow);
        return NULL;
    }
}


/*====================
  CToolBox::IsToolSelected
 ====================*/
bool    CToolBox::IsToolSelected(EToolID eTool)
{
    if (m_pCurrentTool == NULL)
        return TOOL_INVALID;

    return m_pCurrentTool->GetID() == eTool;
}

/*--------------------
  cmdSelectTool

  select an editing tool
  --------------------*/
UI_VOID_CMD(SelectTool, 1)
{
    try
    {
        if (vArgList.size() == 0)
            EX_MESSAGE(_T("syntax: SelectTool <tool>"));

        ITool *pTool(ToolBox.SelectTool(vArgList[0]->Evaluate()));
        if (pTool == NULL)
            EX_WARN(_T("Invalid tool selected"));
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSelectTool() - "), NO_THROW);
        return;
    }
}
