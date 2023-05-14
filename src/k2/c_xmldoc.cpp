// (C)2005 S2 Games
// c_xmldoc.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include <libxml/tree.h>
#include <libxml/globals.h>

#include "c_xmldoc.h"
#include "c_buffer.h"
#include "k2_api.h"
#include "stringutils.h"
#include "c_xmlnode.h"
#include "c_xmlmanager.h"
//=============================================================================

/*====================
  CXMLDoc::CXMLDoc
  ====================*/
CXMLDoc::CXMLDoc(EXMLEncoding eEncoding) :
m_pDoc(nullptr),
m_pRoot(nullptr),
m_eEncoding(eEncoding)
{
}


/*====================
  CXMLDoc::~CXMLDoc
  ====================*/
CXMLDoc::~CXMLDoc()
{
    if (m_pDoc)
        xmlFreeDoc(static_cast<xmlDocPtr>(m_pDoc));
}


/*====================
  CXMLDoc::Clear
  ====================*/
void    CXMLDoc::Clear()
{
    if (m_pDoc)
    {
        xmlFreeDoc(static_cast<xmlDocPtr>(m_pDoc));
        m_pDoc = nullptr;
    }

    m_pRoot = nullptr;

    while (!m_nodeStack.empty())
        m_nodeStack.pop();
    m_nodeStack.push(nullptr);
}


/*====================
  CXMLDoc::NewNode
  ====================*/
bool    CXMLDoc::NewNode(const string &sName, const string &sContent)
{
    if (!m_pDoc)
    {
        m_pDoc = xmlNewDoc(BAD_CAST "1.0");
        if (m_eEncoding == XML_ENCODE_UTF8)
            xmlDocPtr(m_pDoc)->encoding = xmlStrdup(BAD_CAST "UTF-8");
        else if (m_eEncoding == XML_ENCODE_UTF16)
            xmlDocPtr(m_pDoc)->encoding = xmlStrdup(BAD_CAST "UTF-16");
    }

    if (!m_pRoot)
    {
        m_pRoot = xmlNewNode(nullptr, (xmlChar*)sName.c_str());
        if (!m_pRoot)
            return false;
        xmlDocSetRootElement(static_cast<xmlDocPtr>(m_pDoc), static_cast<xmlNodePtr>(m_pRoot));
        m_nodeStack.push(m_pRoot);
    }
    else
    {
        xmlNode *newNode = xmlNewChild(!m_nodeStack.empty() ? static_cast<xmlNodePtr>(m_nodeStack.top()) : nullptr, nullptr, (xmlChar*)sName.c_str(), (xmlChar*)sContent.c_str());
        if (!newNode)
            return false;
        m_nodeStack.push(newNode);
    }

    return true;
}


/*====================
  CXMLDoc::NewTextNode
  ====================*/
bool    CXMLDoc::NewTextNode(const string &sName, const string &sContent)
{
    if (!m_pDoc)
    {
        m_pDoc = xmlNewDoc(BAD_CAST "1.0");
        if (m_eEncoding == XML_ENCODE_UTF8)
            xmlDocPtr(m_pDoc)->encoding = xmlStrdup(BAD_CAST "UTF-8");
        else if (m_eEncoding == XML_ENCODE_UTF16)
            xmlDocPtr(m_pDoc)->encoding = xmlStrdup(BAD_CAST "UTF-16");
    }

    if (!m_pRoot)
    {
        m_pRoot = xmlNewNode(nullptr, (xmlChar*)sName.c_str());
        if (!m_pRoot)
            return false;
        xmlDocSetRootElement(static_cast<xmlDocPtr>(m_pDoc), static_cast<xmlNodePtr>(m_pRoot));
        m_nodeStack.push(m_pRoot);
    }
    else
    {
        xmlNode *newNode = xmlNewTextChild(!m_nodeStack.empty() ? static_cast<xmlNodePtr>(m_nodeStack.top()) : nullptr, nullptr, (xmlChar*)sName.c_str(), (xmlChar*)sContent.c_str());
        if (!newNode)
            return false;
        m_nodeStack.push(newNode);
    }

    return true;
}


/*====================
  CXMLDoc::AddProperty
  ====================*/
bool    CXMLDoc::AddProperty(const string &sName, const string &sValue)
{
    xmlNewProp(!m_nodeStack.empty() ? static_cast<xmlNodePtr>(m_nodeStack.top()) : nullptr, (xmlChar*)sName.c_str(), (xmlChar*)sValue.c_str());
    return true;
}

/*====================
  CXMLDoc::GetProperty
  ====================*/
string  CXMLDoc::GetProperty(const string &sName)
{
    xmlChar* pChar = xmlGetProp(!m_nodeStack.empty() ? static_cast<xmlNodePtr>(m_nodeStack.top()) : nullptr, (xmlChar*)sName.c_str());

    if (pChar == nullptr)
        return SNULL;

    return string((const char*)(pChar));
}

/*====================
  CXMLDoc::GetNodeName
  ====================*/
string  CXMLDoc::GetNodeName()
{
    if (m_nodeStack.empty())
        return SNULL;
    
    return string((const char*)((static_cast<xmlNodePtr>(m_nodeStack.top())->name)));
}

/*====================
  CXMLDoc::TraverseChildren
  ====================*/
bool    CXMLDoc::TraverseChildren()
{
    if (m_nodeStack.empty())
        return false;
    
    xmlNodePtr pParent((xmlNodePtr)m_nodeStack.top());
    if (pParent == nullptr
        || (pParent->type != XML_ELEMENT_NODE
            && pParent->type != XML_ENTITY_NODE
            && pParent->type != XML_DOCUMENT_NODE
            && pParent->type != XML_HTML_DOCUMENT_NODE)
        || !pParent->children)
        return false;

    xmlNode *pChild = ((xmlNode*)(m_nodeStack.top()))->children;

    if (pChild == nullptr)
        return false;

    m_nodeStack.push(pChild);
    return true;
}

/*====================
  CXMLDoc::TraverseChildrenReverse
  ====================*/
bool    CXMLDoc::TraverseChildrenReverse()
{
    if (m_nodeStack.empty())
        return false;
    
    xmlNodePtr pParent((xmlNodePtr)m_nodeStack.top());
    if (pParent == nullptr
        || (pParent->type != XML_ELEMENT_NODE
            && pParent->type != XML_ENTITY_NODE
            && pParent->type != XML_DOCUMENT_NODE
            && pParent->type != XML_HTML_DOCUMENT_NODE)
        || !pParent->children)
        return false;

    xmlNode *pChild = ((xmlNode*)(m_nodeStack.top()))->last;

    if (pChild == nullptr)
        return false;

    m_nodeStack.push(pChild);
    return true;
}

/*====================
  CXMLDoc::TraverseNextChild
  ====================*/
bool    CXMLDoc::TraverseNextChild()
{
    if (m_nodeStack.size() < 2)
        return false;

    xmlNode *pChild = ((xmlNode*)(m_nodeStack.top()))->next;

    if (pChild == nullptr)
        return false;

    m_nodeStack.pop();
    m_nodeStack.push(pChild);
    return true;
}

/*====================
  CXMLDoc::TraversePrevChild
  ====================*/
bool    CXMLDoc::TraversePrevChild()
{
    if (m_nodeStack.size() < 2)
        return false;

    xmlNode *pChild = ((xmlNode*)(m_nodeStack.top()))->prev;

    if (pChild == nullptr)
        return false;

    m_nodeStack.pop();
    m_nodeStack.push(pChild);
    return true;
}

/*====================
  CXMLDoc::EndNode
  ====================*/
bool    CXMLDoc::EndNode()
{
    if (m_nodeStack.empty() || m_nodeStack.top() == nullptr)
        return false;

    m_nodeStack.pop();
    return true;
}

/*====================
  CXMLDoc::DeleteNode
  ====================*/
bool    CXMLDoc::DeleteNode()
{
    if (m_nodeStack.empty() || m_nodeStack.top() == nullptr)
        return false;

    xmlUnlinkNode(static_cast<xmlNodePtr>(m_nodeStack.top()));
    xmlFreeNode(static_cast<xmlNodePtr>(m_nodeStack.top()));
    m_nodeStack.pop();
    return true;
}


/*====================
  CXMLDoc::ConvertNodes
  ====================*/
void    CXMLDoc::ConvertNodes(void *node, CXMLNodeWrite &c_node)
{
    xmlNode *xmlnode(static_cast<xmlNode *>(node));

    if (xmlnode->type == XML_ELEMENT_NODE)
    {
        // Set element name
        c_node.SetName(UTF8ToTString((const char*)xmlnode->name));

        // Set element properties
        xmlAttrPtr prop = xmlnode->properties;

        while (prop != nullptr)
        {
            xmlChar *ret(xmlNodeListGetString(xmlnode->doc, prop->children, 1));

            if (ret != nullptr)
            {
                c_node.SetProperty(UTF8ToTString((const char*)prop->name), UTF8ToTString((const char*)ret));

                xmlFree(ret);
            }
            prop = prop->next;
        }

        if (xmlNodeIsText(xmlnode->children))
        {
            xmlChar *szText(xmlNodeGetContent(xmlnode->children));
            tstring sContents(UTF8ToTString((const char*)szText));
//          StripNewline(sContents);

            // Get contents of container if the contents aren't indentation characters
            if (!IsAllWhiteSpace(sContents))
                c_node.SetContents(sContents);
        }
    }

    for (xmlNode *cur_node(xmlnode->children); cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            CXMLNodeWrite &new_node(c_node.PushChild());

            ConvertNodes(cur_node, new_node);
        }
    }
}


/*====================
  CXMLDoc::WriteBuffer
  ====================*/
int     CXMLDoc::WriteBuffer(void *context, const char *buffer, int len)
{
    IBuffer *buf = (IBuffer*)context;

    buf->Append(buffer, len);
    return len;
}


/*====================
  CXMLDoc::GetBuffer
  ====================*/
IBuffer*    CXMLDoc::GetBuffer()
{
    m_buffer.Clear();

    xmlOutputBufferPtr output = xmlOutputBufferCreateIO(CXMLDoc::WriteBuffer, nullptr, &m_buffer, nullptr);
    if (m_eEncoding == XML_ENCODE_UTF8)
        xmlSaveFormatFileTo(output, static_cast<xmlDocPtr>(m_pDoc), "UTF-8", 1);
    else if (m_eEncoding == XML_ENCODE_UTF16)
        xmlSaveFormatFileTo(output, static_cast<xmlDocPtr>(m_pDoc), "UTF-16", 1);

    return &m_buffer;
}


/*====================
  CXMLDoc::WriteNode
  ====================*/
void    CXMLDoc::WriteNode(CFileHandle &hFile, const CXMLNodeWrite &node, int iIndent)
{
    const CXMLNodeWrite::NodeList &lChildren(node.GetChildren());

    // Indent
    for (int i(0); i < iIndent; ++i) hFile << "\t";

    hFile << "<" << node.GetName();

    const CXMLNodeWrite::PropertyList &lProperties(node.GetPropertyList());

    for (CXMLNodeWrite::PropertyList_cit it(lProperties.begin()), itEnd(lProperties.end()); it != itEnd; ++it)
        hFile << " " << it->first << "=" << QuoteStr(it->second);

    if (lChildren.empty() && node.GetContents().empty())
    {
        hFile << " />" << newl;
    }
    else
    {
        tstring sContents(node.GetContents());
        hFile << ">" << (sContents.empty() ? newl : sContents);

        for (CXMLNodeWrite::NodeList_cit itChild(lChildren.begin()), itEnd(lChildren.end()); itChild != itEnd; ++itChild)
            WriteNode(hFile, *itChild, iIndent + 1);

        // Indent
        if (sContents.empty() || lChildren.size() > 0)
            for (int i(0); i < iIndent; ++i) hFile << "\t";

        hFile << "</" << node.GetName() << ">" << newl;
    }
}


/*====================
  CXMLDoc::WriteNodeLong
  ====================*/
void    CXMLDoc::WriteNodeLong(CFileHandle &hFile, const CXMLNodeWrite &node, int iIndent)
{
    const CXMLNodeWrite::NodeList &lChildren(node.GetChildren());

    // Indent
    for (int i(0); i < iIndent; ++i) hFile << "\t";

    hFile << "<" << node.GetName();

    const CXMLNodeWrite::PropertyList &lProperties(node.GetPropertyList());

    if (lProperties.size() > 0)
    {
        hFile << newl;

        for (CXMLNodeWrite::PropertyList_cit it(lProperties.begin()); it != lProperties.end(); ++it)
        {
            // Indent
            for (int i(0); i < iIndent; ++i) hFile << "\t";
            hFile << "\t";

            hFile << it->first << "=" << QuoteStr(it->second) << newl;
        }

        // Indent
        for (int i(0); i < iIndent; ++i) hFile << "\t";

        if (lChildren.empty() && node.GetContents().empty())
        {
            hFile << "/>" << newl;
        }
        else
        {
            hFile << ">" << (node.GetContents().empty() ? newl : node.GetContents());

            for (CXMLNodeWrite::NodeList_cit itChild(lChildren.begin()); itChild != lChildren.end(); ++itChild)
                WriteNodeLong(hFile, *itChild, iIndent + 1);

            // Indent
            for (int i(0); i < iIndent; ++i) hFile << "\t";

            hFile << "</" << node.GetName() << ">" << newl;
        }
    }
    else
    {
        if (lChildren.empty() && node.GetContents().empty())
        {
            hFile << " />" << newl;
        }
        else
        {
            hFile << ">" << (node.GetContents().empty() ? newl : node.GetContents());

            for (CXMLNodeWrite::NodeList_cit itChild(lChildren.begin()); itChild != lChildren.end(); ++itChild)
                WriteNodeLong(hFile, *itChild, iIndent + 1);

            // Indent
            for (int i(0); i < iIndent; ++i) hFile << "\t";

            hFile << "</" << node.GetName() << ">" << newl;
        }
    }
}


/*====================
  CXMLDoc::WriteFile
  ====================*/
void    CXMLDoc::WriteFile(const tstring &sPath, bool bLong)
{
    int iMode(FILE_WRITE | FILE_TEXT);
    if (m_eEncoding == XML_ENCODE_UTF8)
        iMode |= FILE_UTF8;
    else if (m_eEncoding == XML_ENCODE_UTF16)
        iMode |= FILE_UTF16;

    CFileHandle hFile(sPath, iMode);
    if (!hFile.IsOpen())
        return;

    if (m_eEncoding == XML_ENCODE_UTF8)
        hFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << newl;
    else if (m_eEncoding == XML_ENCODE_UTF16)
        hFile << "<?xml version=\"1.0\" encoding=\"UTF-16\"?>" << newl;
    
    CXMLNodeWrite nodes;
    ConvertNodes(xmlDocGetRootElement(static_cast<xmlDocPtr>(m_pDoc)), nodes);

    if (bLong)
        WriteNodeLong(hFile, nodes, 0);
    else
        WriteNode(hFile, nodes, 0);
}



/*====================
  CXMLDoc::ReadBuffer
  ====================*/
bool    CXMLDoc::ReadBuffer(const char *pBuffer, int iSize)
{
    PROFILE("CXMLManager::ReadBuffer");

    try
    {
        if (m_pDoc)
        {
            xmlFreeDoc(static_cast<xmlDocPtr>(m_pDoc));
            m_pDoc = nullptr;
        }

        if (pBuffer == nullptr)
            EX_WARN(_T("nullptr buffer"));

        // Parse XML File
        m_pDoc = xmlParseMemory(pBuffer, iSize);

        if (m_pDoc == nullptr)
        {
            xmlError *pError(xmlGetLastError());
            if (pError == nullptr)
                EX_ERROR(_T("Unknown error in xmlParseMemory"));

            EX_ERROR(UTF8ToTString(pError->message));
        }

        m_pRoot = xmlDocGetRootElement(static_cast<xmlDocPtr>(m_pDoc));

        if (m_pRoot == nullptr)
        {
            xmlError *pError(xmlGetLastError());
            if (pError == nullptr)
                EX_ERROR(_T("Unknown error in xmlDocGetRootElement"));

            EX_ERROR(UTF8ToTString(pError->message));
        }

        m_nodeStack.push(m_pRoot);

        return true;
    }
    catch (CException &ex)
    {
        if (m_pDoc != nullptr)
        {
            xmlFreeDoc(static_cast<xmlDocPtr>(m_pDoc));
            m_pDoc = nullptr;
        }

        ex.Process(_T("CXMLDoc::ReadBuffer() - "), NO_THROW);
        return false;
    }
}

