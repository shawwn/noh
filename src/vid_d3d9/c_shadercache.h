// (C)2007 S2 Games
// c_shadercache.h
//
//=============================================================================
#ifndef __C_SHADERCACHE_H__
#define __C_SHADERCACHE_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_buffer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const TCHAR* const SHADERCACHE_FILENAME(_T("#/shadercache/shadercache.xml"));
const TCHAR* const SHADERCACHE_ARCHIVE_FILENAME(_T("#/shadercache/shadercache.s2z"));

const uint	NODE_APPEND(BIT(0));
const uint	NODE_REWRITE(BIT(1));
const uint	NODE_LOADED(BIT(2));

class CShaderCacheNode
{
private:
	struct SShaderCacheFileEntry
	{
		uint		uiPos;
		uint		uiSize;
		time_t		tModTime;
		uint		uiCRC32;

		SShaderCacheFileEntry() {}
		SShaderCacheFileEntry(uint _uiPos, uint _uiSize, time_t _tModTime, uint _uiCRC32) :
			uiPos(_uiPos), uiSize(_uiSize), tModTime(_tModTime), uiCRC32(_uiCRC32) {}
	};

	CFileHandle		m_hFile;
	CBufferDynamic	m_cBuffer;
	tstring			m_sDirectory;
	string			m_sDefines;

	uint			m_uiFlags;
	
	map<wstring, SShaderCacheFileEntry>	m_mapEntries;

	void	Load();

public:
	~CShaderCacheNode();
	CShaderCacheNode()		{}

	CShaderCacheNode(const tstring &sDirectory, const string &sDefines);

	const tstring&	GetDirectory() const		{ return m_sDirectory; }
	const string&	GetDefines() const			{ return m_sDefines; }

	void	CacheShader(const wstring &sPath, const byte *pBuffer, uint uiSize, time_t tModTime, uint uiCRC32);
	bool	LoadShader(const wstring &sPath, IBuffer &cBuffer, time_t tModTime, uint &uiCRC32);

	bool	NeedsWrite()		{ return (m_uiFlags & (NODE_REWRITE | NODE_APPEND)) != 0; }

	void	SetLoaded(bool bValue)		{ if (bValue) m_uiFlags |= NODE_LOADED; else m_uiFlags &= ~NODE_LOADED; }
};
//=============================================================================

//=============================================================================
// CShaderCache
//=============================================================================
class CShaderCache
{
private:
	bool		m_bInitialized;
	tstring		m_sVersion;

	CArchive	m_cArchiveRead;
	CArchive	m_cArchiveWrite;

	uint		m_uiActiveNode;

	vector<CShaderCacheNode *>		m_vShaderCacheNodes;

	void	WriteCacheDescriptor();

public:
	~CShaderCache();
	CShaderCache();

	void	Initialize();
	void	Close();

	void			SetVersion(const tstring &sVersion)		{ m_sVersion = sVersion; }
	const tstring&	GetVersion() const						{ return m_sVersion; }
	
	uint	RegisterNode(const tstring &sDirectory, const string &sDefines);
	void	ActivateNode(const string &sDefines);

	bool	LoadShader(const tstring &sPath, IBuffer &cBuffer, uint &uiCRC32);
	void	CacheShader(const tstring &sPath, const byte *pBuffer, uint uiSize, uint uiCRC32);

	CArchive&	GetArchiveRead()					{ return m_cArchiveRead; }
	CArchive&	GetArchiveWrite()					{ return m_cArchiveWrite; }

	void	Clear();
	void	Reload();
};

extern	CShaderCache		g_ShaderCache;
//=============================================================================
#endif //__C_SHADERCACHE_H__
