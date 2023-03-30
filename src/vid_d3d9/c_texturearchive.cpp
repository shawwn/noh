// (C)2008 S2 Games
// c_texturearchive.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_texturearchive.h"

#include "d3d9_texture.h"

#include "../k2/c_xmldoc.h"
#include "../k2/c_xmlnode.h"
#include "../k2/c_xmlprocroot.h"
#include "../k2/c_xmlmanager.h"

#include <sys/utime.h>
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CTextureArchiveNode::~CTextureArchiveNode
  ====================*/
CTextureArchiveNode::~CTextureArchiveNode()
{
}


/*====================
  CTextureArchiveNode::CTextureArchiveNode
  ====================*/
CTextureArchiveNode::CTextureArchiveNode(const tstring &sDirectory, const string &sDefines) :
m_sDirectory(sDirectory),
m_sDefines(sDefines)
{
}


/*====================
  CTextureArchiveNode::Load
  ====================*/
void    CTextureArchiveNode::Load()
{
}


/*====================
  CTextureArchiveNode::WriteTexture
  ====================*/
void    CTextureArchiveNode::WriteTexture(const tstring &sPath, IDirect3DBaseTexture9 *pTexture, CArchive &cArchive, bool bOverwrite)
{
    tstring sTexturePath(Filename_StripExtension(sPath) + _T(".dds"));
    tstring sFilePath(_T("/") + m_sDirectory + sTexturePath);

    if (!bOverwrite && cArchive.ContainsFile(cArchive.GetPathToArchive() + sFilePath))
        return;

    ID3DXBuffer *pD3DBuffer(NULL);
    D3DXSaveTextureToFileInMemory(&pD3DBuffer, D3DXIFF_DDS, pTexture, NULL);

    CFileHandle hDestFile(sFilePath, FILE_WRITE | FILE_TRUNCATE | FILE_BINARY | FILE_COMPRESS, cArchive);
    if (!hDestFile.IsOpen())
    {
        Console.Err << _T("Couldn't write file ") << sFilePath << _T(" in archive ") << cArchive.GetPath() << newl;
        return;
    }

    hDestFile.Write(pD3DBuffer->GetBufferPointer(), pD3DBuffer->GetBufferSize());
    pD3DBuffer->Release();
}


/*====================
  CTextureArchiveNode::LoadTexture
  ====================*/
bool    CTextureArchiveNode::LoadTexture(const tstring &sPath, CArchive &cArchive, CFileHandle &hTexture)
{
    tstring sTexturePath(Filename_StripExtension(sPath) + _T(".dds"));
    tstring sFilePath(_T("/") + m_sDirectory + sTexturePath);

    hTexture.Open(sFilePath, FILE_READ | FILE_BINARY, cArchive);

    return hTexture.IsOpen();
}


/*====================
  CTextureArchiveNode::TextureExists
  ====================*/
bool    CTextureArchiveNode::TextureExists(const tstring &sPath, CArchive &cArchive)
{
    tstring sTexturePath(Filename_StripExtension(sPath) + _T(".dds"));
    tstring sFilePath(Filename_GetPath(cArchive.GetPath()) + _T("/") + m_sDirectory + sTexturePath);

    return cArchive.ContainsFile(sFilePath);
}


/*====================
  CTextureArchive::~CTextureArchive
  ====================*/
CTextureArchive::~CTextureArchive()
{
    Close();
}


/*====================
  CTextureArchive::CTextureArchive
  ====================*/
CTextureArchive::CTextureArchive(const tstring &sMod) :
m_sArchivePath(_T(":/") + sMod + _T("/textures.s2z")),
m_sVersion(_T("1")),

m_uiActiveNode(-1),
m_bInitialized(false)
{
    Initialize();
}


/*====================
  CTextureArchive::Initialize
  ====================*/
void    CTextureArchive::Initialize()
{
    m_cArchiveRead.Open(m_sArchivePath);

    CFileHandle hDescriptor(_T("descriptor"), FILE_READ, m_cArchiveRead);
    if (hDescriptor.IsOpen())
    {
        XMLManager.Process(hDescriptor, _T("texturearchive"), this);
    }

    m_bInitialized = true;
}


/*====================
  CTextureArchive::Close
  ====================*/
void    CTextureArchive::Close()
{
    if (!m_bInitialized)
        return;

    m_cArchiveRead.Close();
    m_cArchiveWrite.Close();

    for (vector<CTextureArchiveNode *>::iterator it(m_vTextureArchiveNodes.begin()); it != m_vTextureArchiveNodes.end(); ++it)
        K2_DELETE(*it);

    m_vTextureArchiveNodes.clear();
}


/*====================
  CTextureArchive::RegisterNode
  ====================*/
uint    CTextureArchive::RegisterNode(const tstring &sDirectory, const string &sDefines)
{
    m_vTextureArchiveNodes.push_back(K2_NEW(ctx_D3D9,   CTextureArchiveNode)(sDirectory, sDefines));

    return uint(m_vTextureArchiveNodes.size() - 1);
}


/*====================
  CTextureArchive::ActivateNode
  ====================*/
void    CTextureArchive::ActivateNode(const string &sDefines)
{
    for (vector<CTextureArchiveNode *>::iterator it(m_vTextureArchiveNodes.begin()); it != m_vTextureArchiveNodes.end(); ++it)
    {
        if ((*it)->GetDefines() == sDefines)
        {
            m_uiActiveNode = it - m_vTextureArchiveNodes.begin();
            return;
        }
    }

    m_uiActiveNode = -1;
}


/*====================
  CTextureArchive::WriteNode
  ====================*/
void    CTextureArchive::WriteNode(const string &sDefines)
{
    for (vector<CTextureArchiveNode *>::iterator it(m_vTextureArchiveNodes.begin()); it != m_vTextureArchiveNodes.end(); ++it)
    {
        if ((*it)->GetDefines() == sDefines)
        {
            m_uiActiveNode = it - m_vTextureArchiveNodes.begin();
            return;
        }
    }

    m_uiActiveNode = -1;

    uint uiNextNode(0);
    for (;;)
    {
        vector<CTextureArchiveNode *>::iterator it(m_vTextureArchiveNodes.begin());

        for (; it != m_vTextureArchiveNodes.end(); ++it)
        {
            if (AtoI((*it)->GetDirectory()) == uiNextNode)
            {
                ++uiNextNode;
                break;
            }
        }

        if (it == m_vTextureArchiveNodes.end())
            break;
    }

    const tstring &sDirectory(XtoA(uiNextNode, FMT_PADZERO, 8));

    m_uiActiveNode = RegisterNode(sDirectory, sDefines);

    WriteDescriptor();
}


/*====================
  CTextureArchive::LoadTexture
  ====================*/
bool    CTextureArchive::LoadTexture(const tstring &sPath, CFileHandle &hTexture)
{
    if (!m_bInitialized || m_uiActiveNode == -1)
        return false;

    return m_vTextureArchiveNodes[m_uiActiveNode]->LoadTexture(sPath, m_cArchiveRead, hTexture);
}


/*====================
  CTextureArchive::GetTextureList
  ====================*/
void    CTextureArchive::GetTextureList(tsvector &vFileList)
{
    m_cArchiveRead.GetFileList(vFileList);
}


/*====================
  CTextureArchive::WriteTexture
  ====================*/
void    CTextureArchive::WriteTexture(const tstring &sPath, IDirect3DBaseTexture9 *pTexture, bool bOverwrite)
{
    if (!m_bInitialized)
        return;

    // Create a new texture cache node if we don't have one set
    if (m_uiActiveNode == -1)
    {
        uint uiNextNode(0);
        for (;;)
        {
            vector<CTextureArchiveNode *>::iterator it(m_vTextureArchiveNodes.begin());

            for (; it != m_vTextureArchiveNodes.end(); ++it)
            {
                if (AtoI((*it)->GetDirectory()) == uiNextNode)
                {
                    ++uiNextNode;
                    break;
                }
            }

            if (it == m_vTextureArchiveNodes.end())
                break;
        }

        const tstring &sDirectory(XtoA(uiNextNode, FMT_PADZERO, 8));

        m_uiActiveNode = RegisterNode(sDirectory, D3D_GetTextureDefinitionString());

        WriteDescriptor();
    }

    if (!OpenWriteArchive())
        return;

    if (!sPath.empty() && sPath[0] == _T('$'))
        m_vTextureArchiveNodes[m_uiActiveNode]->WriteTexture(_T("/") + sPath.substr(1), pTexture, m_cArchiveWrite, bOverwrite);
    else
        m_vTextureArchiveNodes[m_uiActiveNode]->WriteTexture(sPath, pTexture, m_cArchiveWrite, bOverwrite);
}


/*====================
  CTextureArchive::TextureExists
  ====================*/
bool    CTextureArchive::TextureExists(const tstring &sPath)
{
    if (!m_bInitialized || m_uiActiveNode == -1)
        return false;

    return m_vTextureArchiveNodes[m_uiActiveNode]->TextureExists(sPath, m_cArchiveRead);
}


/*====================
  CTextureArchive::WriteDescriptor
  ====================*/
void    CTextureArchive::WriteDescriptor()
{
    if (!OpenWriteArchive())
        return;

    CXMLDoc xmlDescriptor(XML_ENCODE_UTF8);

    xmlDescriptor.NewNode("texturearchive");

        xmlDescriptor.AddProperty("version", m_sVersion);

        for (vector<CTextureArchiveNode*>::iterator it(m_vTextureArchiveNodes.begin()); it != m_vTextureArchiveNodes.end(); ++it)
        {
            xmlDescriptor.NewNode("node");
                xmlDescriptor.AddProperty("directory", (*it)->GetDirectory());
                xmlDescriptor.AddProperty("defines", StringToTString((*it)->GetDefines()));
            xmlDescriptor.EndNode();
        }

    xmlDescriptor.EndNode();

    m_cArchiveWrite.WriteFile(_T("descriptor"), xmlDescriptor.GetBuffer()->Get(), xmlDescriptor.GetBuffer()->GetLength());
}


/*====================
  CTextureArchive::OpenWriteArchive
  ====================*/
bool    CTextureArchive::OpenWriteArchive()
{
    if (m_cArchiveRead.IsOpen())
        m_cArchiveRead.Close();

    if (!m_cArchiveWrite.IsOpen())
        m_cArchiveWrite.Open(m_sArchivePath, ARCHIVE_WRITE | ARCHIVE_APPEND | ARCHIVE_MAX_COMPRESS);

    // Try again without append if we failed
    if (!m_cArchiveWrite.IsOpen())
    {
        m_cArchiveWrite.Open(m_sArchivePath, ARCHIVE_WRITE | ARCHIVE_TRUNCATE | ARCHIVE_MAX_COMPRESS);

        if (!m_cArchiveWrite.IsOpen())
        {
            Console.Err << _T("Could not open ") << m_sArchivePath << _T(" for writing") << newl;

            return false;
        }
    }

    return true;
}


/*====================
  CTextureArchive::Clear
  ====================*/
void    CTextureArchive::Clear()
{
    Close();

    FileManager.Delete(m_sArchivePath);

    m_bInitialized = true;
}


/*====================
  CTextureArchive::Reload
  ====================*/
void    CTextureArchive::Reload()
{
    Close();
    Initialize();
}


namespace XMLTextureArchive
{
    // <texturearchive>
    DECLARE_XML_PROCESSOR(texturearchive)
    BEGIN_XML_REGISTRATION(texturearchive)
        REGISTER_XML_PROCESSOR(root)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(texturearchive, CTextureArchive)
        const tstring &sVersion(node.GetProperty(_T("version")));
        if (sVersion != pObject->GetVersion())
        {
            pObject->Clear();
            return true;
        }
    END_XML_PROCESSOR(pObject)


    // <node>
    DECLARE_XML_PROCESSOR(node)
    BEGIN_XML_REGISTRATION(node)
        REGISTER_XML_PROCESSOR(texturearchive)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(node, CTextureArchive)
        pObject->RegisterNode(node.GetProperty(_T("directory")), TStringToString(node.GetProperty(_T("defines"))));
    END_XML_PROCESSOR_NO_CHILDREN
};
