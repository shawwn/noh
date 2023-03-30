// (C)2010 S2 Games
// c_checksumtable.h
//
//=============================================================================
#ifndef __C_CHECKSUMTABLE_H__
#define __C_CHECKSUMTABLE_H__

//=============================================================================
// Definitions
//=============================================================================
#define CHECKSUM_SIZE		16 // bytes
//=============================================================================

//=============================================================================
// CChecksumTable
//=============================================================================
class K2_API CChecksumTable
{
private:
	//=============================================================================
	// SChecksum
	//=============================================================================
	struct SChecksum
	{
		uint			index;
		byte			val[CHECKSUM_SIZE];

		SChecksum();
		SChecksum(uint uiIndex, const byte* pChecksum);

		bool			Compare(const byte* pChecksum);

		bool			operator <(const SChecksum &cChecksum) const;
	};
	//=============================================================================

	typedef hash_map<tstring, SChecksum>		ChecksumMap;

	ChecksumMap		m_mapChecksums;
	ChecksumMap		m_mapLoadedChecksums;
public:
	CChecksumTable();
	~CChecksumTable();

	void			Clear();
	void			Add(uint uiIndex, const tstring &sFilePath, const byte *pChecksum);
	bool			GetChecksum(byte *pOutChecksum, const tstring &sFilePath);
	bool			Compare(const tstring &sFilePath, const byte *pChecksum);

	bool			Serialize(byte *&pOut, uint &uiOutSize, const tsvector &vFileList);
	bool			Load(const byte *pBuf, uint uiBufSize);

	bool			HashChecksums(byte *pOutChecksum);

	static bool		ComputeChecksum(byte *pOutChecksum, const byte *pMem, uint uiMemSize);
	static void		ChecksumToString(tstring &sOutResult, byte *pChecksum);
};
//=============================================================================
#endif //__C_CHECKSUMTABLE_H__

