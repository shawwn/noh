// (C)2005 S2 Games
// i_xmlprocessor.h
//=============================================================================
#ifndef __I_XMLPROCESSOR_H__
#define __I_XMLPROCESSOR_H__

//=============================================================================
// Headers
//=============================================================================
#include <utility>

#include "c_xmlmanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef map<tstring, class IXMLProcessor*>  XMLProcessorMap;
typedef XMLProcessorMap::iterator           XMLProcessorMap_it;

// _BEGIN_XML_PROCESSOR_DECLARATION
#define _BEGIN_XML_PROCESSOR_DECLARATION(name) \
{ \
private: \
    CXMLProcessor_##name(); \
    CXMLProcessor_##name(const CXMLProcessor_##name&); \
    CXMLProcessor_##name&   operator=(const CXMLProcessor_##name&); \
\
    static XMLProcessorMap* s_pChildren; \
\
    static IXMLProcessor*   GetProcessor(const tstring &sElementName); \
\
public: \
    ~CXMLProcessor_##name() \
    { \
        if (s_pChildren == nullptr) \
            return; \
\
        for (XMLProcessorMap_it it(s_pChildren->begin()); it != s_pChildren->end(); ++it) \
            it->second->RemoveParent(s_pChildren); \
    } \
\
    CXMLProcessor_##name(const tstring &sElementName); \
\
    static void RegisterProcessor(IXMLProcessor *pProcessor); \
\
    void    RemoveChild(const tstring &sName) \
    { \
        if (s_pChildren == nullptr) \
            return; \
\
        XMLProcessorMap_it itFind(s_pChildren->find(sName)); \
        if (itFind != s_pChildren->end()) \
            s_pChildren->erase(itFind); \
    } \
\
    void    ProcessChildren(const CXMLNode &node, void *pObject) \
    { \
        const CXMLNode::List &lChildren(node.GetChildren()); \
        for (CXMLNode::List_cit cit(lChildren.begin()), citEnd(lChildren.end()); cit != citEnd; ++cit) \
        { \
            IXMLProcessor *pProcessor(GetProcessor(cit->GetName())); \
            if (pProcessor == nullptr) \
                Console.Warn << _T("Unknown element ") << QuoteStr(cit->GetName()) << _T(" in ") << QuoteStr(m_sElementName) << newl; \
            else \
                pProcessor->Process(*cit, pObject, this); \
        } \
    } \
\
    bool    Process(const CXMLNode &node, void *pVoid, IXMLProcessor *pParent);

// BEGIN_XML_PROCESSOR_DECLARATION
#define BEGIN_XML_PROCESSOR_DECLARATION(name) \
class CXMLProcessor_##name : public IXMLProcessor \
_BEGIN_XML_PROCESSOR_DECLARATION(name)

// BEGIN_XML_PROCESSOR_DECLARATION_EXPORT
#define BEGIN_XML_PROCESSOR_DECLARATION_EXPORT(name) \
class K2_API CXMLProcessor_##name : public IXMLProcessor \
_BEGIN_XML_PROCESSOR_DECLARATION(name)

// DECLARE_XML_SUBPROCESSOR
#define DECLARE_XML_SUBPROCESSOR(type) \
    bool    Process(const CXMLNode &node, type *pObject);

// END_XML_PROCESSOR_DECLARATION
#define END_XML_PROCESSOR_DECLARATION \
};

// DECLARE_XML_PROCESSOR
#define DECLARE_XML_PROCESSOR(name) \
BEGIN_XML_PROCESSOR_DECLARATION(name) \
END_XML_PROCESSOR_DECLARATION

// DECLARE_XML_PROCESSOR_EXPORT
#define DECLARE_XML_PROCESSOR_EXPORT(name) \
BEGIN_XML_PROCESSOR_DECLARATION_EXPORT(name) \
END_XML_PROCESSOR_DECLARATION

// BEGIN_XML_REGISTRATION
#define BEGIN_XML_REGISTRATION(name) \
IXMLProcessor*  CXMLProcessor_##name::GetProcessor(const tstring &sElementName) \
{ \
    if (s_pChildren == nullptr) \
        return nullptr; \
\
    XMLProcessorMap_it itFind(s_pChildren->find(sElementName)); \
    if (itFind == s_pChildren->end()) \
        return nullptr; \
\
    return itFind->second; \
} \
\
void    CXMLProcessor_##name::RegisterProcessor(IXMLProcessor *pProcessor) \
{ \
    if (s_pChildren == nullptr) \
        s_pChildren = K2_NEW(ctx_Resources,  XMLProcessorMap); \
    if (s_pChildren == nullptr) \
    { \
        Console.Err << _T("Failed to allocate a processor registry for xml processor: ") _T(#name) << newl; \
        return; \
    } \
\
    XMLProcessorMap_it itFind(s_pChildren->find(pProcessor->GetName())); \
    if (itFind != s_pChildren->end()) \
    { \
        Console.Err << _T("Duplicate XML processor, skipping registration: ") << pProcessor->GetName() << newl; \
        return; \
    } \
\
    s_pChildren->insert(pair<tstring, IXMLProcessor*>(pProcessor->GetName(), pProcessor)); \
    pProcessor->AddParent(s_pChildren); \
} \
\
CXMLProcessor_##name::CXMLProcessor_##name(const tstring &sElementName) : \
IXMLProcessor(sElementName) \
{

// REGISTER_XML_PROCESSOR
#define REGISTER_XML_PROCESSOR(parent) \
CXMLProcessor_##parent::RegisterProcessor(this);

// REGISTER_XML_PROCESSOR_EX
#define REGISTER_XML_PROCESSOR_EX(namespace, parent) \
namespace::CXMLProcessor_##parent::RegisterProcessor(this);

// END_XML_REGISTRATION
#define END_XML_REGISTRATION \
}

// _BEGIN_XML_PROCESSOR
#define _BEGIN_XML_PROCESSOR(name, object_type) \
bool    CXMLProcessor_##name::Process(const CXMLNode &node, void *pVoid, IXMLProcessor *pParent) \
{ \
    object_type* pObject(static_cast<object_type*>(pVoid)); \
    if (IsNull(pObject)) \
        return false;
 
// BEGIN_XML_PROCESSOR
#define BEGIN_XML_PROCESSOR(name, object_type) \
XMLProcessorMap*        CXMLProcessor_##name::s_pChildren; \
CXMLProcessor_##name    g_xmlproc_##name(_T(#name)); \
_BEGIN_XML_PROCESSOR(name, object_type)

// BEGIN_XML_SUBPROCESSOR
#define BEGIN_XML_SUBPROCESSOR(name, object_type) \
bool    CXMLProcessor_##name::Process(const CXMLNode &node, object_type *pObject) \
{ \
    if (pObject == nullptr) \
        return false;

// END_XML_PROCESSOR
#define END_XML_PROCESSOR(object) \
    ProcessChildren(node, object); \
    return true; \
}

// END_XML_PROCESSOR
#define END_XML_PROCESSOR_NO_CHILDREN \
    return true; \
}

template <class T>
bool    IsNull(T *pObject)      { return pObject == nullptr; }

template <> inline
bool    IsNull(void *pObject)   { return false; }

// EXTERN_XML_PROCESSOR
#define EXTERN_XML_PROCESSOR(name) extern class CXMLProcessor_##name g_xmlproc_##name;
//=============================================================================

//=============================================================================
// IXMLProcessor
//=============================================================================
class K2_API IXMLProcessor
{
private:
    IXMLProcessor();

protected:
    tstring                 m_sElementName;
    list<XMLProcessorMap*>  m_lParents;

public:
    virtual ~IXMLProcessor()
    {
        for (auto & m_lParent : m_lParents)
            m_lParent->erase(m_sElementName);
    }

    explicit IXMLProcessor(tstring sElementName) :
    m_sElementName(std::move(sElementName))
    {}

    void            AddParent(XMLProcessorMap *pParent)     { if (pParent != nullptr) m_lParents.push_back(pParent); }
    void            RemoveParent(XMLProcessorMap *pParent)  { m_lParents.remove(pParent); }
    bool            IsOrphaned() const                      { return m_lParents.empty(); }
    virtual void    RemoveChild(const tstring &sName) = 0;

    const tstring&  GetName() const                         { return m_sElementName; }

    virtual bool    Process(const CXMLNode &node, void *pVoid, IXMLProcessor *pParent) = 0;
};
//=============================================================================

#endif // __I_XMLPROCESSOR_H__
