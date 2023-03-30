// (C)2008 S2 Games
// c_texturearchive.h
//
//=============================================================================
#ifndef __C_TEXTUREARCHIVE_H__
#define __C_TEXTUREARCHIVE_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_buffer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CTextureArchiveNode
{
private:
	CFileHandle		m_hFile;
	CBufferDynamic	m_cBuffer;
	tstring			m_sDirectory;
	string			m_sDefines;

	void	Load();

public:
	~CTextureArchiveNode();
	CTextureArchiveNode()		{}

	CTextureArchiveNode(const tstring &sDirectory, const string &sDefines);

	const tstring&	GetDirectory() const		{ return m_sDirectory; }
	const string&	GetDefines() const			{ return m_sDefines; }

	void	WriteTexture(const tstring &sPath, IDirect3DBaseTexture9 *pTexture, CArchive &cArchive, bool bOverwrite);
	bool	LoadTexture(const tstring &sPath, CArchive &cArchive, CFileHandle &hTexture);
	bool	TextureExists(const tstring &sPath, CArchive &cArchive);
};
//=============================================================================

//=============================================================================
// CTextureArchive
//=============================================================================
class CTextureArchive
{
private:
	bool		m_bInitialized;
	tstring		m_sArchivePath;
	tstring		m_sVersion;

	CArchive	m_cArchiveRead;
	CArchive	m_cArchiveWrite;

	uint		m_uiActiveNode;

	vector<CTextureArchiveNode *>		m_vTextureArchiveNodes;

	void	WriteDescriptor();
	bool	OpenWriteArchive();
	
	CTextureArchive();

public:
	~CTextureArchive();
	CTextureArchive(const tstring &sMod);
	
	void	Initialize();
	void	Close();

	void			SetVersion(const tstring &sVersion)		{ m_sVersion = sVersion; }
	const tstring&	GetVersion() const						{ return m_sVersion; }
	
	uint	RegisterNode(const tstring &sDirectory, const string &sDefines);
	void	ActivateNode(const string &sDefines);
	void	WriteNode(const string &sDefines);

	bool	LoadTexture(const tstring &sPath, CFileHandle &hTexture);
	void	GetTextureList(tsvector &vFileList);
	void	WriteTexture(const tstring &sPath, IDirect3DBaseTexture9 *pTexture, bool bOverwrite);
	bool	TextureExists(const tstring &sPath);

	void	Clear();
	void	Reload();

	bool	IsValid()			{ return m_cArchiveRead.IsOpen(); }
};
//=============================================================================

#endif //__C_TEXTUREARCHIVE_H__
