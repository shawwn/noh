// (C)2005 S2 Games
// c_xmlnode.h
//=============================================================================
#ifndef __C_XMLNODE_H__
#define __C_XMLNODE_H__

//=============================================================================
// Declarations
//=============================================================================
class CXMLNode;
class CXMLNodeWrite;
//=============================================================================

//=============================================================================
// CXMLNode
//=============================================================================
class CXMLNode
{
public:
    typedef tstring                                             Key;
    typedef tstring                                             Value;
    typedef pair<const Key, Value>                              Property;
    typedef map<Key, Value, std::less<Key> >                    PropertyMap;
    typedef PropertyMap::iterator                               PropertyMap_it;
    typedef PropertyMap::const_iterator                         PropertyMap_cit;

    typedef list<CXMLNode>              List;
    typedef List::iterator              List_it;
    typedef List::const_iterator        List_cit;

private:
    tstring             m_sName;
    PropertyMap         m_mapProperties;
    List                m_lChildren;
    tstring             m_sContents;
    
public:
    K2_API CXMLNode();
    K2_API ~CXMLNode();

    K2_API CXMLNode(const tstring &sPropertys);

    K2_API void Clear();

    const tstring&          GetName() const                 { return m_sName; }
    void                    SetName(const tstring &sName)   { m_sName = sName; }
    void                    SetName(const TCHAR *szName)    { m_sName = szName; }

    inline bool             HasProperty(const tstring &sName) const;

    K2_API const tstring&   GetProperty(const tstring &sName, const tstring &sDefaultValue = TSNULL) const;
    K2_API int              GetPropertyInt(const tstring &sName, int iDefaultValue = 0) const;
    K2_API float            GetPropertyFloat(const tstring &sName, float fDefaultValue = 0.0f) const;
    K2_API bool             GetPropertyBool(const tstring &sName, bool bDefaultValue = false) const;
    K2_API ICvar*           GetPropertyCvar(const tstring &sName, ICvar *pDefaultValue = nullptr) const;
    K2_API CVec2f           GetPropertyV2(const tstring &sName, const CVec2f &v2Default = V2_ZERO) const;
    K2_API CVec3f           GetPropertyV3(const tstring &sName, const CVec3f &v3Default = V_ZERO) const;
    K2_API CVec4f           GetPropertyV4(const tstring &sName, const CVec4f &v4Default = V4_ZERO) const;
    
    K2_API void             SetProperty(const tstring &sName, const tstring &sValue);

    const tstring&          GetContents() const                     { return m_sContents; }
    void                    SetContents(const tstring &sContents)   { m_sContents = sContents; }

    K2_API inline bool      IsText();

    const List&             GetChildren() const             { return m_lChildren; }
    List&                   GetChildren()                   { return m_lChildren; }
    void                    AddChild(CXMLNode &child)       { m_lChildren.push_back(child); }
    CXMLNode&               PushChild()                     { m_lChildren.push_back(CXMLNode()); return m_lChildren.back(); }
    const PropertyMap&      GetPropertyMap() const          { return m_mapProperties; }

    K2_API void             ApplySubstitutions(CXMLNode &cParams);
};

// Null node
extern K2_API CXMLNode      g_NullXMLNode;

/*====================
  CXMLNode::HasProperty
  ====================*/
inline
bool    CXMLNode::HasProperty(const tstring &sName) const
{
    return m_mapProperties.find(sName) != m_mapProperties.end();
}
//=============================================================================

//=============================================================================
// CXMLNodeWrite
//=============================================================================
class CXMLNodeWrite
{
public:
    typedef pair<const tstring, tstring>                                    TStringPair;
    typedef map<tstring, tstring, std::less<tstring> >                      PropertyMap;
    typedef list<TStringPair>                                               PropertyList;
    typedef PropertyList::iterator                                          PropertyList_it;
    typedef PropertyList::const_iterator                                    PropertyList_cit;

    typedef list<CXMLNodeWrite>                 NodeList;
    typedef NodeList::iterator                  NodeList_it;
    typedef NodeList::const_iterator            NodeList_cit;

private:

    tstring         m_sName;
    PropertyMap     m_mapProperties;
    PropertyList    m_lProperties;
    NodeList        m_lChildren;
    tstring         m_sContents;

public:
    CXMLNodeWrite();
    ~CXMLNodeWrite();

    K2_API void Clear();

    const tstring&          GetName() const                 { return m_sName; }
    void                    SetName(const tstring &sName)   { m_sName = sName; }
    void                    SetName(const TCHAR *szName)    { m_sName = szName; }
    
    K2_API void             SetProperty(const tstring &sName, const tstring &sValue);
    K2_API void             RemoveProperty(const tstring &sName);

    inline bool             HasProperty(const tstring &sName) const;

    const tstring&          GetContents() const                     { return m_sContents; }
    void                    SetContents(const tstring &sContents)   { m_sContents = sContents; }

    K2_API inline bool      IsText();

    const NodeList&     GetChildren() const             { return m_lChildren; }
    NodeList&           GetChildren()                   { return m_lChildren; }
    void                AddChild(CXMLNodeWrite &child)  { m_lChildren.push_back(child); }
    CXMLNodeWrite&      PushChild()                     { m_lChildren.push_back(CXMLNodeWrite()); return m_lChildren.back(); }
    const PropertyList& GetPropertyList() const         { return m_lProperties; }
};

/*====================
  CXMLNodeWrite::HasProperty
  ====================*/
inline
bool    CXMLNodeWrite::HasProperty(const tstring &sName) const
{
    return m_mapProperties.find(sName) != m_mapProperties.end();
}
//=============================================================================


#endif // __C_XMLNODE_H__
