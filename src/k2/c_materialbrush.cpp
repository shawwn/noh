// (C)2005 S2 Games
// c_materialbrush.cpp
//
// Material brush loading, attributes, etc.
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_materialbrush.h"
#include "c_bitmap.h"
#include "c_console.h"
#include "c_xmlmanager.h"
//=============================================================================

/*====================
  CMaterialBrush::CMaterialBrush

  Loads a brush from the bitmap stored in m_sFilename
  ====================*/
CMaterialBrush::CMaterialBrush() :
m_sFilename(_T("")),
m_iScale(1),
m_iCurrentTile(0)
{
	for (int i = 0; i < 16; ++i)
		m_hTiles[i] = INVALID_RESOURCE;
}


/*====================
  CMaterialBrush::CMaterialBrush

  Loads a brush from the bitmap stored in sFilename
  ====================*/
CMaterialBrush::CMaterialBrush(const tstring &sFilename) :
m_sFilename(_T("")),
m_iScale(1),
m_iCurrentTile(0)
{
	for (int i = 0; i < 16; ++i)
		m_hTiles[i] = INVALID_RESOURCE;

	Load(sFilename);
}


/*====================
  CMaterialBrush::Load

  Loads a brush from material xml file
  ====================*/
bool	CMaterialBrush::Load(const tstring &sFilename)
{
	return XMLManager.Process(sFilename, _T("brushmat"), this);
}


/*====================
  CMaterialBrush::GetTileIndex
  ====================*/
int		CMaterialBrush::GetTileIndex(ResHandle hTexture)
{
	for (int i = 0; i < 16; ++i)
	{
		if (hTexture == m_hTiles[i])
			return i;
	}

	return 0;
}
