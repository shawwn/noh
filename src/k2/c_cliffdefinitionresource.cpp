// (C)2009 S2 Games
// c_Cliffdefinitionresource.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"
#include "c_cliffdefinitionresource.h"

#include "../k2/i_resourcelibrary.h"
#include "../k2/c_xmlmanager.h"
//=============================================================================

IResource*  AllocCliffDefinition(const tstring &sPath);

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary    g_ResLibCliff(RES_CLIFFDEF, _T("CliffDefinition"), CCliffDefinitionResource::ResTypeName(), true, AllocCliffDefinition);

//=============================================================================
// Definitions
//=============================================================================
IResource*  AllocCliffDefinition(const tstring &sPath)
{
    return K2_NEW(ctx_Resources,  CCliffDefinitionResource)(sPath);
}


/*====================
  CCliffDefinitionResource::GetOuterCorner
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetOuterCorner()
{
    CCliffPiece outerCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("OC"))
            outerCornerPiece = m_vCliffPieces[x];
    }

    return outerCornerPiece;
}


/*====================
  CCliffDefinitionResource::Load
  ====================*/
int     CCliffDefinitionResource::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
    PROFILE("CCliffDefinitionResource::Load");

    Console.Res << _T("Loading ^970Cliff definition^*: ") << m_sPath << newl;

    // Process the XML
    if (!XMLManager.ReadBuffer(pData, uiSize, _T(""), this))
    {
        Console.Warn << _T("CCliffDefinitionResource::Load(") + m_sPath + _T(") - couldn't read XML") << newl;
        return RES_LOAD_FAILED;
    }

    return 0;
}


/*====================
  CCliffDefinitionResource::PostLoad
  ====================*/
void    CCliffDefinitionResource::PostLoad()
{

}

/*====================
  CCliffDefinitionResource::Reloaded
  ====================*/
void    CCliffDefinitionResource::Reloaded()
{
}


/*====================
  CCliffDefinitionResource::GetInnerCorner
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetInnerCorner()
{
    CCliffPiece innerCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("IC"))
            innerCornerPiece = m_vCliffPieces[x];
    }

    return innerCornerPiece;
}


/*====================
  CCliffDefinitionResource::GetWedge
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetWedge()
{
    CCliffPiece frontCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("W"))
            frontCornerPiece = m_vCliffPieces[x];
    }
    return frontCornerPiece;
}


/*====================
  CCliffDefinitionResource::GetFront
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetFront()
{
    CCliffPiece frontCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("F"))
            frontCornerPiece = m_vCliffPieces[x];
    }
    return frontCornerPiece;
}


/*====================
  CCliffDefinitionResource::GetFront256
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetFront256()
{
    CCliffPiece frontCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("F256"))
            frontCornerPiece = m_vCliffPieces[x];
    }
    return frontCornerPiece;
}


/*====================
  CCliffDefinitionResource::GetWedge
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetWedge256()
{
    CCliffPiece frontCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("W256"))
            frontCornerPiece = m_vCliffPieces[x];
    }
    return frontCornerPiece;
}


/*====================
  CCliffDefinitionResource::GetInnerCorner256
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetInnerCorner256()
{
    CCliffPiece frontCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("IC256"))
            frontCornerPiece = m_vCliffPieces[x];
    }
    return frontCornerPiece;
}


/*====================
  CCliffDefinitionResource::GetOuterCorner256
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetOuterCorner256()
{
    CCliffPiece frontCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("OC256"))
            frontCornerPiece = m_vCliffPieces[x];
    }
    return frontCornerPiece;
}

/*====================
  CCliffDefinitionResource::GetOuterCornerTransition1
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetOuterCornerTransition1()
{
    CCliffPiece frontCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("OCT1"))
            frontCornerPiece = m_vCliffPieces[x];
    }
    return frontCornerPiece;
}

/*====================
  CCliffDefinitionResource::GetOuterCornerTransition2
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetOuterCornerTransition2()
{
    CCliffPiece frontCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("OCT2"))
            frontCornerPiece = m_vCliffPieces[x];
    }
    return frontCornerPiece;
}

/*====================
  CCliffDefinitionResource::GetInnerCornerTransition1
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetInnerCornerTransition1()
{
    CCliffPiece frontCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("ICT1"))
            frontCornerPiece = m_vCliffPieces[x];
    }
    return frontCornerPiece;
}

/*====================
  CCliffDefinitionResource::GetInnerCornerTransition2
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetInnerCornerTransition2()
{
    CCliffPiece frontCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("ICT2"))
            frontCornerPiece = m_vCliffPieces[x];
    }
    return frontCornerPiece;
}

/*====================
  CCliffDefinitionResource::GetFrontTransition1
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetFrontTransition1()
{
    CCliffPiece frontCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("FT1"))
            frontCornerPiece = m_vCliffPieces[x];
    }
    return frontCornerPiece;
}

/*====================
  CCliffDefinitionResource::GetFrontTransition2
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetFrontTransition2()
{
    CCliffPiece frontCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("FT2"))
            frontCornerPiece = m_vCliffPieces[x];
    }
    return frontCornerPiece;
}

/*====================
  CCliffDefinitionResource::GetInnerCornerSimple
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetInnerCornerSimple()
{
    CCliffPiece frontCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("ICS"))
            frontCornerPiece = m_vCliffPieces[x];
    }
    return frontCornerPiece;
}

/*====================
  CCliffDefinitionResource::WedgeTransition
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetWedgeTransition()
{
    CCliffPiece frontCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("WT"))
            frontCornerPiece = m_vCliffPieces[x];
    }
    return frontCornerPiece;
}

/*====================
  CCliffDefinitionResource::WedgeShift
  ====================*/
CCliffPiece     CCliffDefinitionResource::GetWedgeShift()
{
    CCliffPiece frontCornerPiece;
    for (unsigned int x(0); x < m_vCliffPieces.size(); x++)
    {
        if (m_vCliffPieces[x].GetPieceType() == _T("WS"))
            frontCornerPiece = m_vCliffPieces[x];
    }
    return frontCornerPiece;
}

/*====================
  CCliffPiece::GetVariation
  ====================*/
CVariation*     CCliffPiece::GetVariation(int iVariation)
{ 
    if (m_vVariations.empty())
        return nullptr;

    int iRealVariation(iVariation);
    
    if (iVariation > ((int)m_vVariations.size() - 1))
        iRealVariation = iVariation % (int)m_vVariations.size();

    return &m_vVariations[iRealVariation]; 
}
