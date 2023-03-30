// (C)2005 S2 Games
// c_xmlmanager.h
//=============================================================================
#ifndef __C_XMLMANAGER_H__
#define __C_XMLMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_xmlnode.h"
#include "i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CFileHandle;

typedef map<tstring, CXMLNode>          XMLTemplateMap;
typedef XMLTemplateMap::iterator        XMLTemplateMap_it;

extern K2_API class CXMLManager XMLManager;
//=============================================================================

//=============================================================================
// CXMLManager
//=============================================================================
class CXMLManager
{
private:
    XMLTemplateMap      m_mapTemplates;

    void    ConvertNodes(void *node, CXMLNode &cNode);

    void    Preprocess(CXMLNode &cNode);
    void    PreprocessRecurse(CXMLNode &cNode);

public:
    ~CXMLManager();
    CXMLManager();

    K2_API bool ReadBuffer(const char *pBuffer, int iSize, const tstring &sRoot, void *pObject = NULL);

    K2_API bool Process(const tstring &sFilename, const tstring &sRoot, void *pObject = NULL, uint uiFileFlags = 0);
    K2_API bool Process(CFileHandle &hFile, const tstring &sRoot, void *pObject = NULL);

    K2_API void PrintVersion() const;
};
//=============================================================================

#endif // __C_XMLMANAGER_H__
