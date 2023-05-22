// (C)2005 S2 Games
// c_cvar.h
//
//=============================================================================
#ifndef __C_CVAR_H__
#define __C_CVAR_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"

#include "c_console.h"
#include "xtoa.h"
#include "c_consoleelement.h"
#include "stringutils.h"
#include "c_heap.h"
#include "c_memmanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CCmd;
class CFileHandle;
class CXMLDoc;
class IBuffer;
class CCvarReference;
template<class T, class T2> class CCvar;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define CVAR(t, name, def)                      CCvar<t> name(_T(#name), def)
#define CVARR(t, name, def, fl, lo, hi)         CCvar<t> name(_T(#name), def, fl, lo, hi)
#define CVAREX(t, name, def, flags, lo, hi, fn) CCvar<t> name(_T(#name), def, flags, lo, hi, fn)
#define EXTERN_CVAR(t, name)                    extern CCvar<t> name
#define IMPORT_CVAR(t, name, def)               static_cast<ICvar *>(ICvar::Import(t, _T(name), def))

#define CVAR_INT(name, def)                     CCvar<int> name(_T(#name), def)
#define CVAR_INTF(name, def, fl)                CCvar<int> name(_T(#name), def, fl)
#define CVAR_INTR(name, def, fl, lo, hi)        CCvar<int> name(_T(#name), def, fl, lo, hi)
#define CVAR_INTEX(name, def, fl, lo, hi, fn)   CCvar<int> name(_T(#name), def, fl, lo, hi, fn)
#define EXTERN_CVAR_INT(name)                   extern CCvar<int> name
#define IMPORT_CVAR_INT(name, def)              static_cast<CCvar<int> *>(ICvar::ImportInt(_T(name), def))

#define CVAR_UINT(name, def)                    CCvar<uint> name(_T(#name), def)
#define CVAR_UINTF(name, def, fl)               CCvar<uint> name(_T(#name), def, fl)
#define CVAR_UINTR(name, def, fl, lo, hi)       CCvar<uint> name(_T(#name), def, fl, lo, hi)
#define CVAR_UINTEX(name, def, fl, lo, hi, fn)  CCvar<uint> name(_T(#name), def, fl, lo, hi, fn)
#define EXTERN_CVAR_UINT(name)                  extern CCvar<uint> name
#define IMPORT_CVAR_UINT(name, def)             static_cast<CCvar<uint> *>(ICvar::ImportInt(_T(name), def))

#define CVAR_FLOAT(name, def)                   CCvar<float> name(_T(#name), def)
#define CVAR_FLOATF(name, def, fl)              CCvar<float> name(_T(#name), def, fl)
#define CVAR_FLOATR(name, def, fl, lo, hi)      CCvar<float> name(_T(#name), def, fl, lo, hi)
#define CVAR_FLOATEX(name, def, fl, lo, hi, fn) CCvar<float> name(_T(#name), def, fl, lo, hi, fn)
#define EXTERN_CVAR_FLOAT(name)                 extern CCvar<float> name
#define IMPORT_CVAR_FLOAT(name, def)            static_cast<CCvar<float> *>(ICvar::ImportFloat(_T(name), def))

#define CVAR_STRING(name, def)                  CCvar<tstring> name(_T(#name), _T(def))
#define CVAR_STRINGF(name, def, fl)             CCvar<tstring> name(_T(#name), _T(def), fl)
#define CVAR_STRINGEX(name, def, fl, fn)        CCvar<tstring> name(_T(#name), _T(def), fl, fn)
#define EXTERN_CVAR_STRING(name)                extern CCvar<tstring> name
#define IMPORT_CVAR_STRING(name, def)           static_cast<CCvar<tstring> *>(ICvar::ImportString(_T(name), def))

#define CVAR_BOOL(name, def)                    CCvar<bool> name(_T(#name), def)
#define CVAR_BOOLF(name, def, fl)               CCvar<bool> name(_T(#name), def, fl)
#define CVAR_BOOLEX(name, def, fl, fn)          CCvar<bool> name(_T(#name), def, fl, fn)
#define EXTERN_CVAR_BOOL(name)                  extern CCvar<bool> name
#define IMPORT_CVAR_BOOL(name, def)             static_cast<CCvar<bool> *>(ICvar::ImportBool(_T(name), def))

#define CVAR_VEC3(name, def)                    CCvar<CVec3f, float> name(_T(#name), def)
#define CVAR_VEC3F(name, def, fl)               CCvar<CVec3f, float> name(_T(#name), def, fl)
#define CVAR_VEC3EX(name, def, fl, fn)          CCvar<CVec3f, float> name(_T(#name), def, fl, fn)
#define EXTERN_CVAR_VEC3(name)                  extern CCvar<CVec3f, float> name
#define IMPORT_CVAR_VEC3(name, def)             static_cast<CCvar<CVec3f, float> *>(ICvar::Import(CT_VEC3, _T(name), def))

#define CVAR_VEC4(name, def)                    CCvar<CVec4f, float> name(_T(#name), def)
#define CVAR_VEC4F(name, def, fl)               CCvar<CVec4f, float> name(_T(#name), def, fl)
#define CVAR_VEC4EX(name, def, fl, fn)          CCvar<CVec4f, float> name(_T(#name), def, fl, fn)
#define EXTERN_CVAR_VEC4(name)                  extern CCvar<CVec4f, float> name
#define IMPORT_CVAR_VEC4(name, def)             static_cast<CCvar<CVec4f, float> *>(ICvar::Import(CT_VEC4, _T(name), def))

#define ARRAY_CVAR_UINT(name, def)                      CArrayCvar<uint> name(_T(#name), def)
#define ARRAY_CVAR_UINTF(name, def, fl)                 CArrayCvar<uint> name(_T(#name), def, fl)
#define ARRAY_CVAR_UINTR(name, def, fl, lo, hi)         CArrayCvar<uint> name(_T(#name), def, fl, lo, hi)
#define ARRAY_CVAR_UINTEX(name, def, fl, lo, hi, fn)    CArrayCvar<uint> name(_T(#name), def, fl, lo, hi, fn)
#define EXTERN_ARRAY_CVAR_UINT(name)                    extern CArrayCvar<uint> name

#define ARRAY_CVAR_FLOAT(name, def)                     CArrayCvar<float> name(_T(#name), def)
#define ARRAY_CVAR_FLOATF(name, def, fl)                CArrayCvar<float> name(_T(#name), def, fl)
#define ARRAY_CVAR_FLOATR(name, def, fl, lo, hi)        CArrayCvar<float> name(_T(#name), def, fl, lo, hi)
#define ARRAY_CVAR_FLOATEX(name, def, fl, lo, hi, fn)   CArrayCvar<float> name(_T(#name), def, fl, lo, hi, fn)
#define EXTERN_ARRAY_CVAR_FLOAT(name)                   extern CArrayCvar<float> name

#define ARRAY_CVAR_STRING(name, def)                    CArrayCvar<tstring> name(_T(#name), _T(def))
#define ARRAY_CVAR_STRINGF(name, def, fl)               CArrayCvar<tstring> name(_T(#name), _T(def), fl)
#define ARRAY_CVAR_STRINGEX(name, def, fl, fn)          CArrayCvar<tstring> name(_T(#name), _T(def), fl, fn)
#define EXTERN_ARRAY_CVAR_STRING(name)                  extern CArrayCvar<tstring> name

#define ARRAY_CVAR_VEC4(name, def)                      CArrayCvar<CVec4f> name(_T(#name), _T(def))
#define ARRAY_CVAR_VEC4F(name, def, fl)                 CArrayCvar<CVec4f> name(_T(#name), _T(def), fl)
#define ARRAY_CVAR_VEC4EX(name, def, fl, fn)            CArrayCvar<CVec4f> name(_T(#name), _T(def), fl, fn)
#define EXTERN_ARRAY_CVAR_VEC$(name)                    extern CArrayCvar<CVec4f> name

const size_t MAX_REFERENCES_PER_CVAR(32);

enum ECvarType
{
    CT_OTHER = 0,
    CT_INT,
    CT_UINT,
    CT_FLOAT,
    CT_STRING,
    CT_BOOL,
    CT_VEC3,
    CT_VEC4,

    CT_INVALID
};

K2_API bool DefaultCvar_Cmd(CConsoleElement *pElem, const tsvector &vArgList);

typedef CCvar<float, float>     CCvarf;
typedef CCvar<int, int>         CCvari;
typedef CCvar<uint, uint>       CCvarui;
typedef CCvar<bool, bool>       CCvarb;
typedef CCvar<tstring, tstring> CCvars;
typedef CCvar<CVec3f, float>    CCvarv3;
//=============================================================================


//=============================================================================
// ICvar
//=============================================================================
class K2_API ICvar : public CConsoleElement
{
protected:
    ECvarType   m_eType;
    bool        m_bModified;
    int         m_iModifiedCount;
    bool        m_bInherentValue;
    tstring     m_sInherentValue;

    ICvar*      m_pParent;
    uint        m_uiChildIndex;
    ICvar*      m_pXYZW[4];
    ICvar*      m_pRGBA[4];

    CCvarReference* m_apReferences[MAX_REFERENCES_PER_CVAR];

    static bool s_bTransmitModified;
    static bool s_bServerInfoModified;
    static bool s_bTrackModifications;

    static bool s_bStoreCvars;

public:
    ICvar(const tstring &sName, int iFlags, ECvarType eType, ConsoleElementFn_t pfnCmd);
    virtual ~ICvar();

    void                    SetParent(ICvar* pParent, uint uiIndex) { m_pParent = pParent; m_uiChildIndex = uiIndex; }

    virtual tstring         GetString() const = 0;
    virtual void            Set(const tstring &s) = 0;
    virtual void            SetIndex(uint uiIndex, const tstring &s) = 0;
    virtual void            Inc(const tstring &s) = 0;
    virtual void            Toggle() = 0;
    virtual void            Reset() = 0;
    virtual bool            IsDefault() const = 0;

    virtual void            StoreValue() = 0;
    virtual void            RestoreStored() = 0;

    ECvarType   GetType() const         { return m_eType; }

    tstring                 GetFlagsString();
    tstring                 GetTypeName() const;
    virtual tstring         GetRangeString() const = 0;
    void                    Print();
    void                    Write(CFileHandle &hFile, const tstring &sWildcard, int iFlags);
    void                    Write(CXMLDoc &xmlConfig, const tstring &sWildcard, int iFlags);
    void                    Write(IBuffer &buffer, const tstring &sWildcard, int iFlags);
    static bool             WriteConfigFile(CFileHandle &hFile, const tsvector &wildcards, int iFlags);
    static bool             WriteConfigFile(CXMLDoc &xmlConfig, const tsvector &wildcards, int iFlags);
    static bool             WriteConfigFile(IBuffer &buffer, const tsvector &wildcards, int iFlags, bool bWriteBinds);

    static void             ResetVars(int iFlags);

    static ICvar*           Find(const tstring &sName);

    static void             SetTrackModifications(bool b)   { s_bTrackModifications = b; }

    bool                    IsModified()            { return m_bModified; }
    void                    SetModified(bool b)     { m_bModified = b; }
    int                     GetModifiedCount()      { return m_iModifiedCount; }
    void                    Modified()
    {
        if (!s_bTrackModifications)
            return;

        m_bModified = true;
        ++m_iModifiedCount;
        if (m_iFlags & CVAR_TRANSMIT)
            SetTransmitModified(true);
        if (m_iFlags & CVAR_SERVERINFO)
            SetServerInfoModified(true);
    }

    static ICvar*           Create(const tstring &sName, ECvarType eType, const tstring &sValue, int iFlags = 0);
    static CCvarb*          CreateBool(const tstring &sName, bool bValue, int iFlags = 0);
    static CCvari*          CreateInt(const tstring &sName, int iValue, int iFlags = 0);
    static CCvarui*         CreateUInt(const tstring &sName, uint uiValue, int iFlags = 0);
    static CCvarf*          CreateFloat(const tstring &sName, float fValue, int iFlags = 0);
    static CCvars*          CreateString(const tstring &sName, const tstring &sValue, int iFlags = 0);
    static CCvarv3*         CreateVec3(const tstring &sName, const CVec3f &v3Value, int iFlags = 0);

    static ICvar*           Import(const tstring &sName, ECvarType eType, const tstring &sValue);
    static ICvar*           ImportBool(const tstring &sName, bool bValue);
    static ICvar*           ImportInt(const tstring &sName, int iValue);
    static ICvar*           ImportUInt(const tstring &sName, uint uiValue);
    static ICvar*           ImportFloat(const tstring &sName, float fValue);
    static ICvar*           ImportString(const tstring &sName, const tstring &sValue);

    static void             SetTransmitCvars(const class CStateString *pStateString);
    static void             ModifyTransmitCvars(const class CStateString *pStateString);
    static void             GetTransmitCvars(class CStateString &ss);
    static void             UnprotectTransmitCvars();
    static void             ProtectTransmitCvars();
    static void             GetServerInfo(class CStateString &ss);

    static bool             IsTransmitModified()    { return s_bTransmitModified; }
    static bool             IsServerInfoModified()  { return s_bServerInfoModified; }

    static void             SetTransmitModified(bool bModified)     { s_bTransmitModified = bModified; }
    static void             SetServerInfoModified(bool bModified)   { s_bServerInfoModified = bModified; }

    // Untyped access to cvars (usage of these should be kept to a minimum)
    static ICvar*           GetCvar(const tstring &sName);
    static int              GetModifiedCount(const tstring &sName);
    static tstring          GetString(const tstring &sName);
    static float            GetFloat(const tstring &sName);
    static int              GetInteger(const tstring &sName);
    static uint             GetUnsignedInteger(const tstring &sName);
    static bool             GetBool(const tstring &sName);
    static CVec3f           GetVec3(const tstring &sName);

    static void             SetString(const tstring &sName, const tstring &s);
    static void             SetFloat(const tstring &sName, float value);
    static void             SetInteger(const tstring &sName, int value);
    static void             SetUnsignedInteger(const tstring &sName, uint value);
    static void             SetBool(const tstring &sName, bool value);
    static void             SetVec3(const tstring &sName, const CVec3f &v3);
    static void             SetVec3(const tstring &sName, float fX, float fY, float fZ);
    static void             SetVec4(const tstring &sName, const CVec4f &v4);
    static void             SetVec4(const tstring &sName, float fX, float fY, float fZ, float fW);

    static void             Toggle(const tstring &sName);
    static void             Reset(const tstring &sName);
    static bool             IsModified(const tstring &sName);
    static void             SetModified(const tstring &sName, bool b);

    static bool             StoreCvars()                        { return s_bStoreCvars; }
    static void             StoreCvars(bool bValue)             { s_bStoreCvars = bValue; }

    float                   GetFloat() const;
    int                     GetInteger() const;
    uint                    GetUnsignedInteger() const;
    bool                    GetBool() const;
    CVec3f                  GetVec3() const;
    CVec4f                  GetVec4() const;

    void                    SetFloat(float value);
    void                    SetInteger(int value);
    void                    SetUnsignedInteger(uint value);
    void                    SetBool(bool value);

    CCvarReference*         GetReference();
    void                    AddReference(CCvarReference *pRef);
    void                    RemoveReference(CCvarReference *pRef);
};
//=============================================================================

//=============================================================================
// CCvar<T>
//=============================================================================
template <class T, class T2 = T>
class CCvar : public ICvar
{
private:
    T   m_Default;
    T   m_Value;
    T   m_Stored;
    T   m_RangeMin, m_RangeMax;

    // Prevent copies
    CCvar();
    CCvar(const CCvar<T> &);

public:
    inline CCvar(const tstring &sName, T _Default, int iFlags = 0, ConsoleElementFn_t pfnCmd = nullptr);
    inline CCvar(const tstring &sName, T _Default, int iFlags, T _Min, T _Max, ConsoleElementFn_t pfnCmd = nullptr);

    ~CCvar() {}

    void            Reset()                                     { if (m_Value == m_Default) return; m_Value = m_Default; Modified(); }
    bool            IsDefault() const                           { return m_Value == m_Default; }

    T               GetStored() const                           { return m_Stored; }
    void            StoreValue()                                { m_Stored = m_Value; }
    void            RestoreStored()                             { m_Value = m_Stored; }

    tstring         GetString() const                           { return XtoA(m_Value); }
    T               GetValue() const                            { return m_Value; }
    T               GetDefault() const                          { return m_Default; }
    void            SetValue(T _t)                              { m_Value = _t; Modified(); }
    void            SetValueIndex(uint ui, T2 _t)               { m_Value[ui] = _t; Modified(); }
    inline void     Set(const tstring &s);
    void            SetIndex(uint uiIndex, const tstring &s)    {}
    void            UpdateChildren()                            {}
    inline void     Inc(const tstring &s)                       {}
    inline void     Toggle()                                    {}

    tstring         GetRangeString() const;

    bool            operator==(const CCvar<T> &_t) const        { return m_Value == _t.m_Value; }
    bool            operator!=(const CCvar<T> &_t) const        { return m_Value != _t.m_Value; }
    bool            operator>(const CCvar<T> &_t) const         { return m_Value > _t.m_Value; }
    bool            operator<(const CCvar<T> &_t) const         { return m_Value < _t.m_Value; }
    bool            operator>=(const CCvar<T> &_t) const        { return m_Value >= _t.m_Value; }
    bool            operator<=(const CCvar<T> &_t) const        { return m_Value <= _t.m_Value; }

    T               operator=(const T &_t)                      { Modified(); return m_Value = _t; }
    T               operator+=(const T &_t)                     { Modified(); return m_Value += _t; }
    T               operator-=(const T &_t)                     { Modified(); return m_Value -= _t; }
    T               operator*=(const T &_t)                     { Modified(); return m_Value *= _t; }
    T               operator/=(const T &_t)                     { Modified(); return m_Value /= _t; }
    T               operator%=(const T &_t)                     { Modified(); return m_Value %= _t; }
    T               operator<<=(const T &_t)                    { Modified(); return m_Value <<= _t; }
    T               operator>>=(const T &_t)                    { Modified(); return m_Value >>= _t; }
    T               operator|=(const T &_t)                     { Modified(); return m_Value |= _t; }
    T               operator&=(const T &_t)                     { Modified(); return m_Value &= _t; }
    T               operator^=(const T &_t)                     { Modified(); return m_Value ^= _t; }

    T               operator++()                                { Modified(); ++m_Value; return m_Value; }
    T               operator--()                                { Modified(); --m_Value; return m_Value; }
    T               operator++(int)                             { Modified(); T val(m_Value); ++m_Value; return val; }
    T               operator--(int)                             { Modified(); T val(m_Value); --m_Value; return val; }

    T2&             operator[](int n)                           { return m_Value[n]; }

                    operator T() const                          { return m_Value; }
};
//=============================================================================


//=============================================================================
// CCvar<tstring>
//=============================================================================
template<>
class CCvar<tstring> : public ICvar
{
private:
    tstring     m_Default;
    tstring     m_Stored;
    tstring     m_Value;

    // Prevent copies
    CCvar();
    CCvar(const CCvar<tstring> &);

public:
    inline CCvar(const tstring &sName, const tstring &_Default, int iFlags = 0, ConsoleElementFn_t pfnCmd = nullptr);

    ~CCvar() {}

    void            Reset()                                 { if (m_Value == m_Default) return; m_Value = m_Default; Modified(); }
    bool            IsDefault() const                       { return m_Value == m_Default; }

    const tstring&  GetStored() const                       { return m_Stored; }
    void            StoreValue()                            { m_Stored = m_Value; }
    void            RestoreStored()                         { m_Value = m_Stored; }

    tstring         GetString() const                       { return m_Value; }
    const tstring&  GetValue() const                        { return m_Value; }
    const tstring&  GetDefault() const                      { return m_Default; }
    void            SetValue(const tstring & _t)            { m_Value = _t; Modified(); }
    inline void     Set(const tstring &s);
    void            SetIndex(uint uiIndex, const tstring &s)    {}
    inline void     Inc(const tstring &s);
    inline void     Toggle();
    bool            empty() const                           { return m_Value.empty(); }
    const TCHAR*    c_str() const                           { return m_Value.c_str(); }

    tstring         GetRangeString() const                  { return _T(""); }

    bool            operator==(const CCvar<tstring> &_t) const  { return m_Value == _t.m_Value; }
    bool            operator!=(const CCvar<tstring> &_t) const  { return m_Value != _t.m_Value; }
    bool            operator>(const CCvar<tstring> &_t) const   { return m_Value > _t.m_Value; }
    bool            operator<(const CCvar<tstring> &_t) const   { return m_Value < _t.m_Value; }
    bool            operator>=(const CCvar<tstring> &_t) const  { return m_Value >= _t.m_Value; }
    bool            operator<=(const CCvar<tstring> &_t) const  { return m_Value <= _t.m_Value; }

    const tstring&  operator=(const tstring &_t)            { Modified(); return m_Value = _t; }
    const tstring&  operator+=(const tstring &_t)           { Modified(); return m_Value += _t; }

    TCHAR           operator[](int n)                       { return m_Value[n]; }

                    operator tstring() const                { return m_Value; }
};
//=============================================================================


//=============================================================================
// CArrayCvar<T>
//=============================================================================
template <class T>
class CArrayCvar : public ICvar
{
private:
    tstring     m_sDefault;
    vector<T>   m_vDefault;
    tstring     m_sValue;
    vector<T>   m_vValues;
    vector<T>   m_vStored;
    T           m_RangeMin, m_RangeMax;

    // Prevent copies
    CArrayCvar();
    CArrayCvar(const CArrayCvar<T> &);

    void    UpdateValueString()
    {
        m_sValue.clear();
        for (uint ui(0); ui < m_vValues.size(); ++ui)
        {
            m_sValue += XtoA(m_vValues[ui]);
            if (ui < m_vValues.size() - 1)
                m_sValue += _T(",");
        }
    }

public:
    ~CArrayCvar() {}

    CArrayCvar(const tstring &sName, const tstring &sDefault, int iFlags = 0, ConsoleElementFn_t pfnCmd = nullptr) :
    ICvar(sName, iFlags | CONEL_HOME, CT_OTHER, pfnCmd),
    m_sDefault(sDefault),
    m_sValue(sDefault)
    {
        tsvector vTokens(TokenizeString(sDefault, _T(',')));
        m_vValues.resize(vTokens.size());
        m_vDefault.resize(vTokens.size());
        m_vStored.resize(vTokens.size());
        for (uint ui(0); ui < vTokens.size(); ++ui)
        {
            T _value;
            AtoX(vTokens[ui], _value);
            m_vValues[ui] = _value;
            m_vDefault[ui] = _value;
            m_vStored[ui] = _value;
        }
    }

    CArrayCvar(const tstring &sName, const tstring &sDefault, int iFlags, T _Min, T _Max, ConsoleElementFn_t pfnCmd = nullptr) :
    ICvar(sName, iFlags | CONEL_HOME | CVAR_VALUERANGE, CT_OTHER, pfnCmd),
    m_RangeMin(_Min),
    m_RangeMax(_Max)
    {
        tsvector vTokens(TokenizeString(sDefault, _T(',')));
        m_vValues.resize(vTokens.size());
        m_vDefault.resize(vTokens.size());
        m_vStored.resize(vTokens.size());
        for (uint ui(0); ui < vTokens.size(); ++ui)
        {
            T _value;
            AtoX(vTokens[ui], _value);
            _value = CLAMP(_value, m_RangeMin, m_RangeMax);
            m_vValues[ui] = _value;
            m_vDefault[ui] = _value;
            m_vStored[ui] = _value;
        }

        UpdateValueString();
        m_sDefault = m_sValue;
    }

    void    Reset()
    {
        if (m_vValues == m_vDefault)
            return;

        m_sValue = m_sDefault;
        m_vValues = m_vDefault;
        Modified();
    }

    bool                IsDefault() const                           { return m_vValues == m_vDefault; }

    const vector<T>&    GetStored() const                           { return m_vStored; }
    void                StoreValue()                                { m_vStored = m_vValues; }
    void                RestoreStored()                             { m_vValues = m_vStored; }

    tstring             GetString() const                           { return m_sValue; }

    const tstring&      GetValue() const                            { return m_sValue; }
    T                   GetValue(uint uiIndex) const
    {
        if (uiIndex >= INT_SIZE(m_vValues.size()))
        {
            Console.Warn << _T("Index '") << uiIndex << _T("' is out of bounds for ArrayCvar: ") << m_sName << _T(" (size is: ") << INT_SIZE(m_vValues.size()) << _T(")") << newl;
            T _return;
            if (m_vValues.empty())
                AtoX(SNULL, _return);
            else
                _return = m_vValues.back();
            return _return;
        }
        return m_vValues[uiIndex];
    }

    const tstring&      GetDefault() const                          { return m_sDefault; }
    void                SetValue(uint uiIndex, T _value)
    {
        if (uiIndex >= m_vValues.size())
            m_vValues.resize(uiIndex + 1);
        m_vValues[uiIndex] = _value;
        UpdateValueString();
        Modified();
    }

    uint                GetSize() const                             { return INT_SIZE(m_vValues.size()); }
    void                Resize(uint uiSize, T _value)               { m_vValues.resize(uiSize, _value); }

    void    Set(const tstring &s)
    {
        tsvector vTokens(TokenizeString(s, _T(',')));
        m_vValues.resize(vTokens.size());
        for (uint ui(0); ui < vTokens.size(); ++ui)
        {
            T _value;
            AtoX(vTokens[ui], _value);
            if (HasFlags(CVAR_VALUERANGE))
                _value = CLAMP(_value, m_RangeMin, m_RangeMax);
            m_vValues[ui] = _value;
        }

        UpdateValueString();
        Modified();
    }

    void    SetIndex(uint uiIndex, const tstring &s)
    {
        T _value;
        AtoX(s, _value);
        if (HasFlags(CVAR_VALUERANGE))
            _value = CLAMP(_value, m_RangeMin, m_RangeMax);
        m_vValues[uiIndex] = _value;
        
        UpdateValueString();
        Modified();
    }
    
    void                Inc(const tstring &s)                       {}
    void                Toggle()                                    {}

    tstring GetRangeString() const
    {
        if (m_iFlags & CVAR_VALUERANGE)
            return _T(" [") + XtoA(m_RangeMin) + _T(", ") + XtoA(m_RangeMax) + _T("] ");

        return _T("");
    }

    T&                  operator[](int n)                           { return m_vValues[n]; }
};
//=============================================================================


//=============================================================================
// CArrayCvar<tstring>
//=============================================================================
template <>
class CArrayCvar<tstring> : public ICvar
{
private:
    tstring m_sDefault;
    tsvector    m_vDefault;
    tstring m_sValue;
    tsvector    m_vValues;
    tsvector    m_vStored;

    // Prevent copies
    CArrayCvar();
    CArrayCvar(const CArrayCvar<tstring>&);

public:
    ~CArrayCvar() {}

    CArrayCvar(const tstring &sName, const tstring &sDefault, int iFlags = 0, ConsoleElementFn_t pfnCmd = nullptr) :
    ICvar(sName, iFlags | CONEL_HOME, CT_OTHER, pfnCmd),
    m_sDefault(sDefault),
    m_sValue(sDefault)
    {
        tsvector vTokens(TokenizeString(sDefault, _T(',')));
        m_vValues.resize(vTokens.size());
        m_vDefault.resize(vTokens.size());
        m_vStored.resize(vTokens.size());
        for (uint ui(0); ui < vTokens.size(); ++ui)
        {
            tstring sValue(vTokens[ui]);
            m_vValues[ui] = sValue;
            m_vDefault[ui] = sValue;
            m_vStored[ui] = sValue;
        }
    }

    void    Reset()
    {
        if (m_vValues == m_vDefault)
            return;

        m_sValue = m_sDefault;
        m_vValues = m_vDefault;
        Modified();
    }

    bool                IsDefault() const                           { return m_vValues == m_vDefault; }

    const tsvector&     GetStored() const                           { return m_vStored; }
    void                StoreValue()                                { m_vStored = m_vValues; }
    void                RestoreStored()                             { m_vValues = m_vStored; }

    tstring             GetString() const                           { return m_sValue; }

    const tstring&      GetValue() const                            { return m_sValue; }
    const tstring&      GetValue(uint uiIndex) const
    {
        if (uiIndex < INT_SIZE(m_vValues.size()))
            return m_vValues[uiIndex];

        Console.Warn << _T("Index '") << uiIndex << _T("' is out of bounds for ArrayCvar: ") << m_sName << _T(" (size is: ") << INT_SIZE(m_vValues.size()) << _T(")") << newl;
        if (m_vValues.empty())
            return TSNULL;
        else
            return m_vValues.back();
    }

    const tstring&      GetDefault() const                              { return m_sDefault; }
    
    void    SetValue(uint uiIndex, const tstring &sValue)
    {
        if (uiIndex >= m_vValues.size())
            m_vValues.resize(uiIndex + 1);
        m_vValues[uiIndex] = sValue;
        m_sValue = ConcatinateArgs(m_vValues.begin(), m_vValues.end(), _T(","));
        Modified();
    }

    uint                GetSize() const                                 { return INT_SIZE(m_vValues.size()); }

    void                Set(const tstring &s)                           { m_sValue = s; m_vValues = TokenizeString(s, _T(',')); }
    void                SetIndex(uint uiIndex, const tstring &sValue)   { SetValue(uiIndex, sValue); }
    
    void                Inc(const tstring &s)                           {}
    void                Toggle()                                        {}

    tstring             GetRangeString() const                          { return TSNULL; }

    const tstring&      operator[](int n)                               { return m_vValues[n]; }
};
//=============================================================================

/*====================
  CCvar<T>::CCvar
  ====================*/
template <class T, class T2>
inline CCvar<T, T2>::CCvar(const tstring &sName, T _Default, int iFlags, ConsoleElementFn_t pfnCmd) :
ICvar(sName, iFlags | CONEL_HOME, CT_OTHER, pfnCmd),
m_Default(_Default),
m_Value(_Default),
m_Stored(_Default)
{
    if (m_bInherentValue)
    {
        tsvector vArgList2; vArgList2.push_back(m_sInherentValue);

        DefaultCvar_Cmd(this, vArgList2);
    }
}


/*====================
  CCvar<T>::CCvar
  ====================*/
template <class T, class T2>
inline CCvar<T, T2>::CCvar(const tstring &sName, T _Default, int iFlags, T _Min, T _Max, ConsoleElementFn_t pfnCmd) :
ICvar(sName, iFlags | CONEL_HOME | CVAR_VALUERANGE, CT_OTHER, pfnCmd),
m_Default(_Default),
m_Value(_Default),
m_Stored(_Default),
m_RangeMin(_Min),
m_RangeMax(_Max)
{
    if (m_bInherentValue)
    {
        tsvector vArgList2; vArgList2.push_back(m_sInherentValue);

        DefaultCvar_Cmd(this, vArgList2);
    }
}


/*====================
  CCvar<int>::CCvar
  ====================*/
template<>
inline CCvar<int>::CCvar(const tstring &sName, int _Default, int iFlags, ConsoleElementFn_t pfnCmd) :
ICvar(sName, iFlags | CONEL_HOME, CT_INT, pfnCmd),
m_Default(_Default),
m_Value(_Default),
m_Stored(_Default)
{
    if (m_bInherentValue)
    {
        tsvector vArgList2; vArgList2.push_back(m_sInherentValue);
        DefaultCvar_Cmd(this, vArgList2);
    }
}

template<>
inline CCvar<int>::CCvar(const tstring &sName, int _Default, int iFlags, int _Min, int _Max, ConsoleElementFn_t pfnCmd) :
ICvar(sName, iFlags | CONEL_HOME | CVAR_VALUERANGE, CT_INT, pfnCmd),
m_Default(_Default),
m_Value(_Default),
m_Stored(_Default),
m_RangeMin(_Min),
m_RangeMax(_Max)
{
    if (m_bInherentValue)
    {
        tsvector vArgList2; vArgList2.push_back(m_sInherentValue);
        DefaultCvar_Cmd(this, vArgList2);
    }
}


/*====================
  CCvar<uint>::CCvar
  ====================*/
template<>
inline CCvar<uint>::CCvar(const tstring &sName, uint _Default, int iFlags, ConsoleElementFn_t pfnCmd) :
ICvar(sName, iFlags | CONEL_HOME, CT_UINT, pfnCmd),
m_Default(_Default),
m_Value(_Default),
m_Stored(_Default)
{
    if (m_bInherentValue)
    {
        tsvector vArgList2; vArgList2.push_back(m_sInherentValue);
        DefaultCvar_Cmd(this, vArgList2);
    }
}

template<>
inline CCvar<uint>::CCvar(const tstring &sName, uint _Default, int iFlags, uint _Min, uint _Max, ConsoleElementFn_t pfnCmd) :
ICvar(sName, iFlags | CONEL_HOME | CVAR_VALUERANGE, CT_UINT, pfnCmd),
m_Default(_Default),
m_Value(_Default),
m_Stored(_Default),
m_RangeMin(_Min),
m_RangeMax(_Max)
{
    if (m_bInherentValue)
    {
        tsvector vArgList2; vArgList2.push_back(m_sInherentValue);
        DefaultCvar_Cmd(this, vArgList2);
    }
}


/*====================
  CCvar<float>::CCvar
  ====================*/
template<>
inline CCvar<float>::CCvar(const tstring &sName, float _Default, int iFlags, ConsoleElementFn_t pfnCmd) :
ICvar(sName, iFlags | CONEL_HOME, CT_FLOAT, pfnCmd),
m_Default(_Default),
m_Value(_Default),
m_Stored(_Default)
{
    if (m_bInherentValue)
    {
        tsvector vArgList2; vArgList2.push_back(m_sInherentValue);

        DefaultCvar_Cmd(this, vArgList2);
    }
}


/*====================
  CCvar<float>::CCvar
  ====================*/
template<>
inline CCvar<float>::CCvar(const tstring &sName, float _Default, int iFlags, float _Min, float _Max, ConsoleElementFn_t pfnCmd) :
ICvar(sName, iFlags | CONEL_HOME | CVAR_VALUERANGE, CT_FLOAT, pfnCmd),
m_Default(_Default),
m_Value(_Default),
m_Stored(_Default),
m_RangeMin(_Min),
m_RangeMax(_Max)
{
    if (m_bInherentValue)
    {
        tsvector vArgList; vArgList.push_back(m_sInherentValue);

        DefaultCvar_Cmd(this, vArgList);
    }
}


/*====================
  CCvar<tstring>::CCvar
  ====================*/
inline CCvar<tstring>::CCvar(const tstring &sName, const tstring &_Default, int iFlags, ConsoleElementFn_t pfnCmd) :
ICvar(sName, iFlags | CONEL_HOME, CT_STRING, pfnCmd),
m_Default(_Default),
m_Stored(_Default),
m_Value(_Default)
{
    if (m_bInherentValue)
    {
        tsvector vArgList; vArgList.push_back(m_sInherentValue);

        DefaultCvar_Cmd(this, vArgList);
    }
}


/*====================
  CCvar<bool>::CCvar
  ====================*/
template<>
inline CCvar<bool>::CCvar(const tstring &sName, bool _Default, int iFlags, ConsoleElementFn_t pfnCmd) :
ICvar(sName, iFlags | CONEL_HOME, CT_BOOL, pfnCmd),
m_Default(_Default),
m_Value(_Default),
m_Stored(_Default)
{
    if (m_bInherentValue)
    {
        tsvector vArgList2;
        vArgList2.push_back(m_sInherentValue);

        DefaultCvar_Cmd(this, vArgList2);
    }
}


/*====================
  CCvar<CVec3f, float>::UpdateChildren
  ====================*/
template<>
inline void CCvar<CVec3f, float>::UpdateChildren()
{
    for (uint ui(X); ui <= Z; ++ui)
    {
        if (m_pXYZW[ui] == nullptr || m_pRGBA[ui] == nullptr)
        {
            Console.Err << _T("Failed to allocate a subscript cvar for ") << QuoteStr(m_sName) << newl;
            break;
        }

        m_pXYZW[ui]->SetFloat(m_Value[ui]);
        m_pRGBA[ui]->SetFloat(m_Value[ui]);
    }
}


/*====================
  CCvar<CVec4f, float>::UpdateChildren
  ====================*/
template<>
inline void CCvar<CVec4f, float>::UpdateChildren()
{
    for (uint ui(X); ui <= W; ++ui)
    {
        if (m_pXYZW[ui] == nullptr || m_pRGBA[ui] == nullptr)
        {
            Console.Err << _T("Failed to allocate a subscript cvar for ") << QuoteStr(m_sName) << newl;
            break;
        }

        m_pXYZW[ui]->SetFloat(m_Value[ui]);
        m_pRGBA[ui]->SetFloat(m_Value[ui]);
    }
}


/*====================
  CCvar<CVec3f, float>::CCvar
  ====================*/
template<>
inline CCvar<CVec3f, float>::CCvar(const tstring &sName, CVec3f _Default, int iFlags, ConsoleElementFn_t pfnCmd) :
ICvar(sName, iFlags | CONEL_HOME, CT_VEC3, pfnCmd),
m_Default(_Default),
m_Value(_Default),
m_Stored(_Default)
{
    if (m_bInherentValue)
    {
        tsvector vArgList2; vArgList2.push_back(m_sInherentValue);
        DefaultCvar_Cmd(this, vArgList2);
    }

    UpdateChildren();
}


/*====================
  CCvar<CVec4f, float>::CCvar
  ====================*/
template<>
inline CCvar<CVec4f, float>::CCvar(const tstring &sName, CVec4f _Default, int iFlags, ConsoleElementFn_t pfnCmd) :
ICvar(sName, iFlags | CONEL_HOME, CT_VEC4, pfnCmd),
m_Default(_Default),
m_Value(_Default),
m_Stored(_Default)
{
    if (m_bInherentValue)
    {
        tsvector vArgList2; vArgList2.push_back(m_sInherentValue);
        DefaultCvar_Cmd(this, vArgList2);
    }

    UpdateChildren();
}


/*====================
  CCvar<T, T2>::GetRangeString
  ====================*/
template<class T, class T2>
inline tstring      CCvar<T, T2>::GetRangeString() const
{
    if (m_iFlags & CVAR_VALUERANGE)
        return _T(" [") + XtoA(m_RangeMin) + _T(", ") + XtoA(m_RangeMax) + _T("] ");

    return _T("");
}


/*====================
  CCvar<T>::Set
  ====================*/
template<class T, class T2>
inline void CCvar<T, T2>::Set(const tstring &s)
{
    T Value;

    AtoX(s, Value);

    if (Value != m_Value)
    {
        m_Value = Value;
        Modified();
    }
}


/*====================
  CCvar<int>::Set

  Set function that checks for valid ranges.
  ====================*/
template<>
inline void CCvar<int>::Set(const tstring &s)
{
    int Value;

    AtoX(s, Value);

    if ((m_iFlags & CVAR_VALUERANGE) && (Value < m_RangeMin || Value > m_RangeMax))
        Console.Warn << m_sName << _T(": ")
            << s << _T(" isn't within the valid range [") << m_RangeMin << _T(", ") << m_RangeMax << _T("]") << newl;
    else if (m_Value != Value)
    {
        m_Value = Value;
        Modified();
    }
}


/*====================
  CCvar<float>::Set

  Set function that checks for valid ranges.
  ====================*/
template<>
inline void CCvar<float>::Set(const tstring &s)
{
    float fValue(AtoF(s));
    if ((m_iFlags & CVAR_VALUERANGE) && (fValue < m_RangeMin || fValue > m_RangeMax))
    {
        Console.Warn << m_sName << _T(": ") << s << _T(" isn't within the valid range [") << m_RangeMin << _T(", ") << m_RangeMax << _T("]") << newl;
        return;
    }

    if (fValue != m_Value)
    {
        m_Value = fValue;
        Modified();
    }

    if (m_pParent != nullptr)
    {
        if (m_pParent->GetType() == CT_VEC3)
            static_cast<CCvar<CVec3f, float>*>(m_pParent)->SetValueIndex(m_uiChildIndex, m_Value);
        else if (m_pParent->GetType() == CT_VEC4)
            static_cast<CCvar<CVec4f, float>*>(m_pParent)->SetValueIndex(m_uiChildIndex, m_Value);
    }
}


/*====================
  CCvar<CVec3f, float>::Set
  ====================*/
template<>
inline void CCvar<CVec3f, float>::Set(const tstring &s)
{
    CVec3f Vec;

    AtoX(s, Vec);

    if (Vec != m_Value)
    {
        m_Value = Vec;
        Modified();
        UpdateChildren();
    }
}


/*====================
  CCvar<CVec4f, float>::Set
  ====================*/
template<>
inline void CCvar<CVec4f, float>::Set(const tstring &s)
{
    CVec4f Vec;

    AtoX(s, Vec);

    if (Vec != m_Value)
    {
        m_Value = Vec;
        Modified();
        UpdateChildren();
    }
}


/*====================
  CCvar<tstring>::Set
  ====================*/
inline void CCvar<tstring>::Set(const tstring &s)
{
    tstring String;

    String = s;

    if (s != m_Value)
    {
        m_Value = s;
        Modified();
    }
}


/*====================
  CCvar<CVec3f, float>::SetIndex
  ====================*/
template <>
inline void CCvar<CVec3f, float>::SetIndex(uint uiIndex, const tstring &s)
{
    if (uiIndex > Z)
    {
        Console.Warn << _T("Index ") << uiIndex << _T(" is out of range for cvar ") << QuoteStr(m_sName) << newl;
        return;
    }

    m_Value[uiIndex] = AtoF(s);
    Modified();
}


/*====================
  CCvar<CVec4f, float>::SetIndex
  ====================*/
template <>
inline void CCvar<CVec4f, float>::SetIndex(uint uiIndex, const tstring &s)
{
    if (uiIndex > W)
    {
        Console.Warn << _T("Index ") << uiIndex << _T(" is out of range for cvar ") << QuoteStr(m_sName) << newl;
        return;
    }

    float fValue;

    fValue = AtoF(s);

    if (fValue != m_Value[uiIndex])
    {
        m_Value[uiIndex] = AtoF(s);
        Modified();
    }
}


/*====================
  CCvar<>::Inc
  ====================*/
template <> inline void CCvar<int>::Inc(const tstring &s)   
{
    int Value;

    Value = AtoI(s);

    if (Value)
    {
        m_Value += AtoI(s);
        Modified();
    }
}
template <> inline void CCvar<float>::Inc(const tstring &s)
{
    float fValue;

    fValue = AtoF(s);

    if (fValue)
    {
        m_Value += AtoF(s);
        Modified();
    }
}


/*====================
  CCvar<tstring>::Inc
  ====================*/
inline void CCvar<tstring>::Inc(const tstring &s)
{
    tstring sValue;

    sValue = XtoA(AtoF(m_Value) + AtoF(s));
    Console.Perf << "Inc called on a string cvar" << newl;

    if (sValue != m_Value)
    {
        m_Value = sValue;
        Modified();
    }
}


/*====================
  CCvar<>::Toggle
  ====================*/
template <> inline void CCvar<int>::Toggle()    { m_Value = m_Value ? 0 : 1; Modified(); }
template <> inline void CCvar<float>::Toggle()  { m_Value = m_Value ? 0.0f : 1.0f; Modified(); }
template <> inline void CCvar<bool>::Toggle()   { m_Value = m_Value ? false : true; Modified(); }


/*====================
  CCvar<tstring>::Toggle
  ====================*/
inline void CCvar<tstring>::Toggle()
{
    // Do nothing by default...
    Console.Perf << "Toggle called on a string cvar" << newl;
}
//=============================================================================

/*====================
  operator+
  ====================*/
template <class T>
inline
tstring operator+(const tstring &s, const CCvar<T> &var)
{
    return s + var.GetString();
}

inline
tstring operator+(const CCvar<tstring> &var, const tstring &s)
{
    return var.GetString() + s;
}


/*====================
  operator==
  ====================*/
inline
bool    operator==(const tstring &s, const CCvar<tstring> &var)
{
    return s == tstring(var);
}

inline
bool    operator==(const CCvar<tstring> &var, const tstring &str)
{
    return var.GetString() == str;
}


/*====================
  operator!=
  ====================*/
template <class T>
inline
bool    operator!=(const tstring &s, const CCvar<T> &var)
{
    return s != tstring(var);
}

inline
bool    operator!=(const CCvar<tstring> &var, const tstring &str)
{
    return var.GetString() != str;
}


/*====================
  operator>
  ====================*/
template <class T>
inline
bool    operator>(const tstring &s, const CCvar<T> &var)
{
    return s > tstring(var);
}

inline
bool    operator>(const CCvar<tstring> &var, const tstring &str)
{
    return var.GetString() > str;
}


/*====================
  operator<
  ====================*/
template <class T>
inline
bool    operator<(const tstring &s, const CCvar<T> &var)
{
    return s < tstring(var);
}

inline
bool    operator<(const CCvar<tstring> &var, const tstring &str)
{
    return var.GetString() < str;
}


/*====================
  operator>=
  ====================*/
template <class T>
inline
bool    operator>=(const tstring &s, const CCvar<T> &var)
{
    return s != tstring(var);
}

inline
bool    operator>=(const CCvar<tstring> &var, const tstring &str)
{
    return var.GetString() >= str;
}


/*====================
  operator<=
  ====================*/
template <class T>
inline
bool    operator<=(const tstring &s, const CCvar<T> &var)
{
    return s != tstring(var);
}

inline
bool    operator<=(const CCvar<tstring> &var, const tstring &str)
{
    return var.GetString() <= str;
}
//=============================================================================
#endif //__C_CVAR_H__
