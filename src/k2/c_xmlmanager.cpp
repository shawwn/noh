// (C)2005 S2 Games
// c_xmlmanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/globals.h>
#include <libxml/xmlversion.h>

#include "c_xmlmanager.h"
#include "c_xmlprocroot.h"
#include "c_filehandle.h"
#include "c_xmldoc.h"
#include "c_alias.h"
#include "c_action.h"
#include "c_actionregistry.h"
#include "stringutils.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
K2_API wchar_t*     SingleToWide(wchar_t *out, const xmlChar *in, size_t len);

inline
char*   strncpy(char *out, const xmlChar *in, size_t len)
{
    if (STRNCPY_S(out, len, (const char*)in, len))
        return out;
    else
        return nullptr;
}

CXMLManager XMLManager;
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
EXTERN_XML_PROCESSOR(root)
//=============================================================================

/*====================
  CXMLManager::~CXMLManager
  ====================*/
CXMLManager::~CXMLManager()
{
    xmlCleanupParser();
}


/*====================
  CXMLManager::CXMLManager
  ====================*/
CXMLManager::CXMLManager()
{   
    xmlInitParser();
}


/*====================
  CXMLManager::ConvertNodes
  ====================*/
void    CXMLManager::ConvertNodes(void *node, CXMLNode &cNode)
{
    xmlNode *xmlnode(static_cast<xmlNode *>(node));

    if (xmlnode->type == XML_ELEMENT_NODE)
    {
        // Set element name
        cNode.SetName(UTF8ToTString((char*)xmlnode->name));

        // Set element properties
        xmlAttrPtr prop = xmlnode->properties;

        while (prop != nullptr)
        {
#if 0
            xmlChar *ret = xmlNodeListGetString(xmlnode->doc, prop->children, 1);

            if (ret != nullptr)
            {
#ifdef _UNICODE
                tstring sPropertyName;
                StrToTString(sPropertyName, (const char*)prop->name);
                tstring sValue;
                StrToTString(sValue, (const char*)ret);

                cNode.SetProperty(sPropertyName, sValue);
#else
                cNode.SetProperty((const char*)prop->name, (const char*)ret);
#endif

                xmlFree(ret);
            }
#else
            cNode.SetProperty(UTF8ToTString((const char*)prop->name), UTF8ToTString((const char*)prop->children->content));
#endif
            prop = prop->next;
        }

        if (xmlNodeIsText(xmlnode->children))
        {
            xmlChar *szText(xmlNodeGetContent(xmlnode->children));

            // Skip if this node is empy
            xmlChar *sz(szText);
            while (*sz)
            {
                if (*sz != '\n' &&
                    *sz != '\r' &&
                    *sz != '\t' &&
                    *sz != ' ')
                    break;

                ++sz;
            }

            if (*sz)
            {
                tstring sContents(UTF8ToTString((const char*)szText));
                StripNewline(sContents);

                // Get contents of container if the contents aren't indentation characters
                if (!IsAllWhiteSpace(sContents))
                    cNode.SetContents(sContents);
            }

            xmlFree(szText);
        }
    }

    for (xmlNode *cur_node(xmlnode->children); cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            CXMLNode &new_node(cNode.PushChild());

            ConvertNodes(cur_node, new_node);
        }
    }
}


/*====================
  CXMLManager::PreprocessRecurse
  ====================*/
void    CXMLManager::PreprocessRecurse(CXMLNode &cNode)
{
    CXMLNode::List &lChildren(cNode.GetChildren());
    for (CXMLNode::List_it it(lChildren.begin()), itEnd(lChildren.end()); it != itEnd; )
    {
        CXMLNode &cChild(*it);

        if (cChild.GetName() == _T("template"))
        {
            m_mapTemplates[cChild.GetProperty(_T("name"))] = cChild;
            
            it = lChildren.erase(it);
            itEnd = lChildren.end();
        }
        else if (cChild.GetName() == _T("instance"))
        {
            XMLTemplateMap_it itFind(m_mapTemplates.find(cChild.GetProperty(_T("name"))));

            CXMLNode::List_it itInstance(it);

            if (itFind != m_mapTemplates.end())
            {
                CXMLNode::List &lTemplateChildren(itFind->second.GetChildren());
                for (CXMLNode::List_it itT(lTemplateChildren.begin()), itTEnd(lTemplateChildren.end()); itT != itTEnd; ++itT)
                {
                    CXMLNode cNewNode(*itT);
                    cNewNode.ApplySubstitutions(*itInstance);

                    it = lChildren.insert(it, cNewNode);
                    ++it;
                }

                itEnd = lChildren.end();
            }

            it = lChildren.erase(itInstance);
        }
        else
        {
            PreprocessRecurse(cChild);
            ++it;
        }
    }
}


/*====================
  CXMLManager::Preprocess
  ====================*/
void    CXMLManager::Preprocess(CXMLNode &cNode)
{
    PreprocessRecurse(cNode);
    m_mapTemplates.clear();
}


/*====================
  CXMLManager::ReadBuffer
  ====================*/
bool    CXMLManager::ReadBuffer(const char *pBuffer, int iSize, const tstring &sRoot, void *pObject)
{
    PROFILE("CXMLManager::ReadBuffer");

    xmlDoc  *pXMLDoc(nullptr);
    xmlNode *pRootElement(nullptr);

    try
    {
        if (pBuffer == nullptr)
            EX_WARN(_T("nullptr buffer"));

        // Parse XML File
        {
            PROFILE("xmlParseMemory");
            pXMLDoc = xmlParseMemory(pBuffer, iSize);
        }

        if (pXMLDoc == nullptr)
        {
            xmlError *pError(xmlGetLastError());
            if (pError == nullptr)
                EX_ERROR(_T("Unknown error in xmlParseMemory"));

            tstring sError;
            StrToTString(sError, pError->message);
            EX_ERROR(sError);
        }

        // Get the root element node
        CXMLNode rootNode;
        rootNode.SetName(_T("root"));
        CXMLNode &newNode(rootNode.PushChild());

        pRootElement = xmlDocGetRootElement(pXMLDoc);

        {
            PROFILE("ConvertNodes");
            ConvertNodes(pRootElement, newNode);
        }

        // Cleanup
        xmlFreeDoc(pXMLDoc);
        pXMLDoc = nullptr;

        CXMLNode::List &lChildren(rootNode.GetChildren());

        if (lChildren.size() == 0)
            EX_ERROR(_T("Empty document"));
        else if (lChildren.size() > 1)
            EX_ERROR(_T("Invalid xml document"));
        else if (!sRoot.empty() && lChildren.front().GetName() != sRoot)
            EX_ERROR(_T("Invalid root node ") + SingleQuoteStr(lChildren.front().GetName()) + _T(", expected ") + SingleQuoteStr(sRoot));

        // TODO: make this a parameter of the processor or this function or something...
        if (sRoot == _T("effect"))
            Preprocess(rootNode);
        
        // Step through the nodes
        g_xmlproc_root.Process(rootNode, pObject, nullptr);
        return true;
    }
    catch (CException &ex)
    {
        if (pXMLDoc != nullptr)
            xmlFreeDoc(pXMLDoc);
        ex.Process(_T("CXMLManager::ReadBuffer() - "), NO_THROW);
        return false;
    }
}


/*====================
  CXMLManager::Process
  ====================*/
bool    CXMLManager::Process(const tstring &sFilename, const tstring &sRoot, void* pObject, unsigned int uiFileFlags)
{
    try
    {
        CFileHandle hFile(sFilename, FILE_READ | FILE_BINARY | uiFileFlags);

        if (!hFile.IsOpen())
            EX_ERROR(_T("Could not open ") + QuoteStr(sFilename));

        uint uiSize(0);
        const char* pScriptBuffer(hFile.GetBuffer(uiSize));
        if (pScriptBuffer == nullptr)
            EX_ERROR(_T("Could not read ") + QuoteStr(sFilename));

        tstring sOldDir(FileManager.GetWorkingDirectory());

        FileManager.SetWorkingDirectory(Filename_GetPath(FileManager.SanitizePath(sFilename)));

        if (!ReadBuffer(pScriptBuffer, uiSize, sRoot, pObject))
        {
            FileManager.SetWorkingDirectory(sOldDir);
            EX_ERROR(_T("Error in ") + QuoteStr(sFilename));
        }

        FileManager.SetWorkingDirectory(sOldDir);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CXMLManager::Process() - "), NO_THROW);
        return false;
    }

    return true;
}

bool    CXMLManager::Process(CFileHandle &hFile, const tstring &sRoot, void *pObject)
{
    try
    {
        uint uiSize(0);
        const char *pScriptBuffer(hFile.GetBuffer(uiSize));

        if (pScriptBuffer == nullptr)
            EX_ERROR(_T("Could not read ") + QuoteStr(hFile.GetPath()));

        if (!ReadBuffer(pScriptBuffer, uiSize, sRoot, pObject))
            EX_ERROR(_T("Error in ") + QuoteStr(hFile.GetPath()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CXMLManager::Process() - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  CXMLManager::PrintVersion
  ====================*/
void    CXMLManager::PrintVersion() const
{
    Console.Dev << _T("libxml2 version: ") << LIBXML_VERSION_STRING << newl;
}
