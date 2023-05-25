// (C)2005 S2 Games
// i_resource.h
//
//=============================================================================
#ifndef __I_RESOURCE_H__
#define __I_RESOURCE_H__

//=============================================================================
// Declarations
//=============================================================================
class IResourceWatcher;
//=============================================================================

//=============================================================================
// IResource
//=============================================================================
#if defined(linux) || defined(__APPLE__)
class __attribute__ ((visibility("default"))) IResource
#else
class IResource
#endif
{
protected:
    tstring         m_sPath;
    tstring         m_sLocalizedPath;
    tstring         m_sName;
    int             m_iFlags = 0;
    const char*     m_pData = nullptr;
    uint            m_uiSize = 0;
    ResHandle       m_hHandle = INVALID_RESOURCE;
    uint            m_uiNetIndex = INVALID_INDEX;
    uint            m_uiIgnoreFlags = 0;

    set<ResHandle>  m_setDependents;

    IResource(const IResource&);

public:
    IResource() = delete;
    K2_API virtual ~IResource();
    K2_API IResource(const tstring &sPath, tstring sName);

    K2_API virtual uint             GetResType() const=0;
    K2_API virtual const tstring&   GetResTypeName() const=0;

    const tstring&      GetPath() const             { return m_sPath; }
    const tstring&      GetLocalizedPath() const    { if (m_sLocalizedPath.empty()) return m_sPath; return m_sLocalizedPath; }
    const tstring&      GetName() const             { return m_sName; }
    const char*         GetData() const             { return m_pData; }
    uint                GetSize() const             { return m_uiSize; }
    ResHandle           GetHandle() const           { return m_hHandle; }
    uint                GetNetIndex() const         { return m_uiNetIndex; }
    uint                GetIgnoreFlags() const      { return m_uiIgnoreFlags; }

    void        SetLocalizedPath(const tstring &sPath)      { m_sLocalizedPath = sPath; }
    void        SetName(const tstring &sName)               { m_sName = sName; }
    void        SetHandle(ResHandle hHandle)                { m_hHandle = hHandle; }
    void        SetNetIndex(uint uiNetIndex)                { m_uiNetIndex = uiNetIndex; }
    void        SetIgnoreFlags(uint uiIgnoreFlags)          { m_uiIgnoreFlags = uiIgnoreFlags; }

    void        SetFlags(int iFlags)            { m_iFlags = iFlags; }
    void        AddFlags(int iFlags)            { m_iFlags |= iFlags; }
    void        ClearFlags(int iFlags)          { m_iFlags &= ~iFlags; }
    void        ClearAllFlags()                 { m_iFlags = 0; }
    bool        HasFlags(int iFlags) const      { return (m_iFlags & iFlags) != 0; }

    void                    AddDependent(ResHandle hRes)    { m_setDependents.insert(hRes); }
    void                    ClearDependents()               { m_setDependents.clear(); }
    const set<ResHandle>&   GetDependents() const           { return m_setDependents; }

    K2_API bool     IsVirtualResource() const;
    K2_API bool     MatchesWildcard(const tstring &sWild,
        bool bMatchAgainstPath = true,
        bool bMatchAgainstName = true,
        bool bMatchAgainstType = true) const;

    virtual int     Load(uint uiIgnoreFlags, const char *pData, uint uiSize) = 0;
    virtual void    PostLoad()  {}
    virtual bool    LoadNull()  { return false; }
    virtual void    Free() = 0;
    virtual void    Reloaded()  {}
};
//=============================================================================
#endif //__I_RESOURCE_H__
