// (C)2008 S2 Games
// c_compressedfile.h
//
// Intermediate class for copying a compressed file from one archive to another
//=============================================================================
#ifndef __C_COMPRESSEDFILE_H__
#define __C_COMPRESSEDFILE_H__

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CCompressedFile
//=============================================================================
class CCompressedFile
{
private:
	const char *m_pData;
	
	uint	m_uiCRC32;
	uint	m_uiStoredSize;
	uint	m_uiRawSize;
	int		m_iLevel;

public:
	~CCompressedFile()	{}
	CCompressedFile();
	CCompressedFile(const char *pData, uint uiCRC32, uint uiStoredSize, uint uiRawSize, int iLevel);

	const char*		GetData() const			{ return m_pData; }
	uint			GetCRC32() const		{ return m_uiCRC32; }
	uint			GetStoredSize() const	{ return m_uiStoredSize; }
	uint			GetRawSize() const		{ return m_uiRawSize; }
	int				GetLevel() const		{ return m_iLevel; }
};
//=============================================================================

#endif //__C_COMPRESSEDFILE_H__
