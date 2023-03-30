// (C)2008 S2 Games
// c_compressedfile.cpp
//
// Intermediate class for copying a compressed file from one archive to another
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_compressedfile.h"
//=============================================================================

/*====================
  CCompressedFile::CCompressedFile
  ====================*/
CCompressedFile::CCompressedFile() :
m_pData(NULL)
{
}


/*====================
  CCompressedFile::CCompressedFile
  ====================*/
CCompressedFile::CCompressedFile(const char *pData, uint uiCRC32, uint uiStoredSize, uint uiRawSize, int iLevel) :
m_pData(pData),
m_uiCRC32(uiCRC32),
m_uiStoredSize(uiStoredSize),
m_uiRawSize(uiRawSize),
m_iLevel(iLevel)
{
}
