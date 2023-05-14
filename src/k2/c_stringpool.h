// (C)2010 S2 Games
// c_stringpool.h
//
//  A set of unique, immutable strings shared across the application.  For
// example, a path string like "/heroes/arachna/hero.entity" will likely never
// be modified.  Therefore, there should only be 1 instance of that string in
// memory (or as close to 1 instance as we can get).
//
//  As of now (Nov 2010), string memory usage is about ~90MB during a game.
// Anything we can do to get that figure closer to 10MB seems like a good thing;
// hence, StringPool.
//
//  Note: StringPool is thread-safe.
//=============================================================================
#ifndef __C_STRINGPOOL_H__
#define __C_STRINGPOOL_H__

#if 0
//=============================================================================
// Headers
//=============================================================================
#include "c_spinmutex.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CPooledString;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
extern K2_API class CStringPool&    g_StringPool;

// StringPool alias
#define StringPool  g_StringPool
//=============================================================================

//=============================================================================
// CStringPool
//=============================================================================
class CStringPool
{
    SINGLETON_DEF(CStringPool)

    //*********************
    // Friend Classes
    //*********************
    friend class CPooledString;

    //*********************
    // Private Declarations
    //*********************
    struct SStringEntry;
    
    static const uint   STRFLAG_SMALL_STRING    = BIT(0);

    //*********************
    // Private Definitions
    //*********************
    typedef hash_map<tstring, SStringEntry>     StringsMap;

#pragma pack(push, 1)
    // SStringEntry
    struct SStringEntry
    {
        // Reference Count
        uint                    uiRefCount;

        // Flags
        union
        {
            // if STRFLAG_SMALL_STRING is set, the string's memory is never freed (because it's <= 256 bytes)
            byte                yFlags;
            uint                uiPadding;
        };

        // String pointer
        StringsMap::iterator    itString;
    };
#pragma pack(pop)

    //*********************
    // Member Variables
    //*********************
    StringsMap      m_mapStrings;
    CSpinMutex      m_cMutex;

    //*********************
    // Internal Methods
    //*********************

    bool    ShouldErase(SStringEntry* pEntry)
    {
        size_t uiBytes(sizeof(tstring));
        uiBytes += sizeof(SStringEntry);

        const tstring &sStr(pEntry->itString->first);
        uiBytes += sizeof(tstring::value_type) * sStr.size();

        // is the string large enough to worry about erasing?
        if (uiBytes > 512)
            return true;
        else
            return false;
    }


    bool    DoEraseEntry(SStringEntry* pEntry)
    {
        assert(pEntry != nullptr);

        // if a different thread modified the refcount, then don't erase it.
        if (K2_Atomic_LoadWithAcquire(pEntry->uiRefCount) != 0)
            return false;

        // Acquire exclusive access to the shared hash map
        CLockSpinMutex cLock(m_cMutex);

        // Erase the entry
        m_mapStrings.erase(pEntry->itString);

        return true;
    }


    void    DoAddRef(SStringEntry* pEntry)
    {
        assert(pEntry != nullptr);

        // Increment refcount
        K2_Atomic_FetchAdd4(&(pEntry->uiRefCount), 1);
    }


    void    DoAcquireString(SStringEntry*& pOutEntry, const tstring &sLookup)
    {
        assert(!sLookup.empty());

        // Acquire exclusive access to the shared hash map
        CLockSpinMutex cLock(m_cMutex);

        StringsMap::iterator itFind(m_mapStrings.find(sLookup));

        // Return an existing entry?
        if (itFind != m_mapStrings.end())
        {
            // Release exclusive access to the shared hash map
            cLock.Reset();
            pOutEntry = &itFind->second;

            // Increment the reference count
            DoAddRef(pOutEntry);
            return;
        }
        else
        {
            // Add a new entry, setting its refcount to 1
            itFind = m_mapStrings.insert(make_pair(sLookup, SStringEntry())).first;
            SStringEntry* pResult(&itFind->second);
            pResult->uiRefCount = 1;
            pResult->yFlags     = 0;
            pResult->itString   = itFind;

            // Store whether the string is eventually going to be deallocated
            if (!ShouldErase(pResult))
                pResult->yFlags |= STRFLAG_SMALL_STRING;

            pOutEntry = pResult;
            return;
        }
    }


    void    DoReleaseString(SStringEntry* pEntry)
    {
        assert(pEntry != nullptr);

        // Decrement refcount
        __int32 iResult = K2_Atomic_FetchAdd4(&(pEntry->uiRefCount), -1);
        if (iResult == 1)
        {
            // Deallocate the string, if necessary.
            if ((pEntry->yFlags & STRFLAG_SMALL_STRING) == 0)
            {
                DoEraseEntry(pEntry);
            }
        }
    }

public:
    ~CStringPool();
};
//=============================================================================

//=============================================================================
// CPooledString
//=============================================================================
class CPooledString
{
public:
    // String character
    typedef tstring::value_type         value_type;
    typedef size_t                      size_type;
    typedef CPooledString               uniquestr_t;

    // Invalid string position
    static const size_t                 npos;

private:
    // String shared data
    typedef CStringPool::SStringEntry   StrShared;

    StrShared*          m_pShared;


    /*====================
      CPooledString::Reset
      ====================*/
    void    Reset()
    {
        // release the existing string.
        assert(m_pShared != nullptr);
        StringPool.DoReleaseString(m_pShared);
        m_pShared = nullptr;
    }


public:
    /*====================
      CPooledString::~CPooledString
      ====================*/
    ~CPooledString()
    {
        if (m_pShared != nullptr)
            Reset();
    }


    /*====================
      CPooledString::CPooledString
      ====================*/
    CPooledString()
        : m_pShared(nullptr)
    {
    }


    /*====================
      CPooledString::CPooledString
      ====================*/
    CPooledString(const value_type* pLookup)
        : m_pShared(nullptr)
    {
        if (pLookup != nullptr && *pLookup)
        {
            StringPool.DoAcquireString(m_pShared, pLookup);
            assert(m_pShared != nullptr);
        }
    }


    /*====================
      CPooledString::CPooledString
      ====================*/
    CPooledString(const tstring &sLookup)
        : m_pShared(nullptr)
    {
        if (!sLookup.empty())
        {
            StringPool.DoAcquireString(m_pShared, sLookup);
            assert(m_pShared != nullptr);
        }
    }


    /*====================
      CPooledString::CPooledString
      ====================*/
    CPooledString(const CPooledString& sExisting)
        : m_pShared(nullptr)
    {
        if (sExisting.m_pShared != nullptr)
        {
            StringPool.DoAddRef(sExisting.m_pShared);
            m_pShared = sExisting.m_pShared;
        }
    }


    /*====================
      CPooledString::operator =
      ====================*/
    CPooledString&  operator = (const CPooledString& rhs)
    {
        Assign(rhs.m_pShared);
        return *this;
    }


    /*====================
      CPooledString::Assign
      ====================*/
    void    Assign(StrShared* pShared)
    {
        if (m_pShared == pShared)
            return;

        if (pShared == nullptr)
        {
            if (m_pShared != nullptr)
                Reset();
        }
        else
        {
            StringPool.DoAddRef(pShared);
            if (m_pShared != nullptr)
                StringPool.DoReleaseString(m_pShared);
            m_pShared = pShared;
        }
    }

    const tstring&  tstr() const
    {
        if (m_pShared == nullptr)
            return TSNULL;
        else
            return m_pShared->itString->first;
    }

    operator        const tstring&() const
    {
        return tstr();
    }

    friend bool     operator ==(const uniquestr_t& sLeft, const uniquestr_t& sRight)
    {
        return (sLeft.m_pShared == sRight.m_pShared);
    }

    friend bool     operator ==(const uniquestr_t& sLeft, const tstring &sRight)
    {
        return sLeft.tstr() == sRight;
    }

    friend bool     operator ==(const tstring &sLeft, const uniquestr_t& sRight)
    {
        return sLeft == sRight.tstr();
    }

    friend bool     operator ==(const uniquestr_t& sLeft, const value_type* sRight)
    {
        return (sLeft.tstr().compare(sRight) == 0);
    }

    friend bool     operator ==(const value_type* sLeft, const uniquestr_t& sRight)
    {
        return (sRight.tstr().compare(sLeft) == 0);
    }

    friend bool     operator !=(const uniquestr_t& sLeft, const uniquestr_t& sRight)
    {
        return (sLeft.m_pShared != sRight.m_pShared);
    }

    friend bool     operator !=(const uniquestr_t& sLeft, const tstring &sRight)
    {
        return sLeft.tstr() != sRight;
    }

    friend bool     operator !=(const tstring &sLeft, const uniquestr_t& sRight)
    {
        return sLeft != sRight.tstr();
    }

    friend bool     operator !=(const uniquestr_t& sLeft, const value_type* sRight)
    {
        return (sLeft.tstr().compare(sRight) != 0);
    }

    friend bool     operator !=(const value_type* sLeft, const uniquestr_t& sRight)
    {
        return (sRight.tstr().compare(sLeft) != 0);
    }

    bool    operator < (const tstring &sStr) const
    {
        return (tstr() < sStr);
    }

    friend tstring      operator +(const uniquestr_t& sLeft, const uniquestr_t& sRight)
    {   // return left + right
        return sLeft.tstr() + sRight.tstr();
    }

    friend tstring      operator +(const uniquestr_t& sLeft, const tstring &sRight)
    {   // return left + right
        return sLeft.tstr() + sRight;
    }

    friend tstring      operator +(const tstring &sLeft, const uniquestr_t& sRight)
    {   // return left + right
        return sLeft + sRight.tstr();
    }

    friend tstring      operator +(const value_type* sLeft, const uniquestr_t& sRight)
    {   // return left + right
        return tstring(sLeft).append(sRight.tstr());
    }

    friend tstring      operator +(const uniquestr_t& sLeft, const value_type* sRight)
    {   // return left + right
        return sLeft.tstr() + sRight;
    }

    value_type          operator [](int _Off) const
    {   // subscript
        return tstr()[_Off];
    }

    value_type          operator [](size_t _Off) const
    {   // subscript
        return tstr()[_Off];
    }

    const value_type*   c_str() const
    {   // return pointer to null-terminated nonmutable array
        return tstr().c_str();
    }

    const value_type*   data() const
    {   // return pointer to nonmutable array
        return tstr().data();
    }

    size_t              length() const
    {   // return length of sequence
        return tstr().length();
    }

    size_t              size() const
    {   // return length of sequence
        return tstr().size();
    }

    bool                empty() const
    {   // test if sequence is empty
        return tstr().empty();
    }

    size_t              find(const tstring& _Right, size_t _Off = 0) const
    {   // look for _Right beginnng at or after _Off
        return tstr().find(_Right, _Off);
    }

    size_t              find(const value_type* Ptr, size_t _Off, size_t _Count) const
    {   // look for [_Ptr, _Ptr + _Count) beginnng at or after _Off
        return tstr().find(Ptr, _Off, _Count);
    }

    size_t              find(const value_type* Ptr, size_t _Off = 0) const
    {   // look for [_Ptr, <null>) beginnng at or after _Off
        return tstr().find(Ptr, _Off);
    }

    size_t              find(value_type* _Ch, size_t _Off = 0) const
    {   // look for _Ch at or after _Off
        return tstr().find(_Ch, _Off);
    }

    size_t              find(value_type _Ch, size_type _Off = 0) const
    {   // look for _Ch at or after _Off
        return tstr().find(_Ch, _Off);
    }

    size_t              rfind(const tstring& _Right, size_t _Off = npos) const
    {   // look for _Right beginning before _Off
        return tstr().rfind(_Right, _Off);
    }

    size_t              rfind(const value_type* Ptr, size_t _Off, size_t _Count) const
    {   // look for [_Ptr, _Ptr + _Count) beginning before _Off
        return tstr().rfind(Ptr, _Off, _Count);
    }

    size_t              rfind(const value_type* Ptr, size_t _Off = npos) const
    {   // look for [_Ptr, <null>) beginning before _Off
        return tstr().rfind(Ptr, _Off);
    }

    size_t              rfind(value_type* _Ch, size_t _Off = npos) const
    {   // look for _Ch before _Off
        return tstr().rfind(_Ch, _Off);
    }

    size_t              rfind(value_type _Ch, size_type _Off = npos) const
    {   // look for _Ch before _Off
        return tstr().rfind(_Ch, _Off);
    }

    size_t              find_first_of(const tstring& _Right, size_t _Off = 0) const
    {   // look for one of _Right at or after _Off
        return tstr().find_first_of(_Right, _Off);
    }

    size_t              find_first_of(const value_type* Ptr, size_t _Off, size_t _Count) const
    {   // look for one of [_Ptr, _Ptr + _Count) at or after _Off
        return tstr().find_first_of(Ptr, _Off, _Count);
    }

    size_t              find_first_of(const value_type* Ptr, size_t _Off = 0) const
    {   // look for one of [_Ptr, <null>) at or after _Off
        return tstr().find_first_of(Ptr, _Off);
    }

    size_t              find_first_of(value_type* _Ch, size_t _Off = 0) const
    {   // look for _Ch at or after _Off
        return tstr().find_first_of(_Ch, _Off);
    }

    size_t              find_last_of(const tstring& _Right, size_t _Off = npos) const
    {   // look for one of _Right before _Off
        return tstr().find_last_of(_Right, _Off);
    }

    size_t              find_last_of(const value_type* Ptr, size_t _Off, size_t _Count) const
    {   // look for one of [_Ptr, _Ptr + _Count) before _Off
        return tstr().find_last_of(Ptr, _Off, _Count);
    }

    size_t              find_last_of(const value_type* Ptr, size_t _Off = npos) const
    {   // look for one of [_Ptr, <null>) before _Off
        return tstr().find_last_of(Ptr, _Off);
    }

    size_t              find_last_of(value_type* _Ch, size_t _Off = npos) const
    {   // look for _Ch before _Off
        return tstr().find_last_of(_Ch, _Off);
    }

    size_t              find_first_not_of(const tstring& _Right, size_t _Off = 0) const
    {   // look for none of _Right at or after _Off
        return tstr().find_last_not_of(_Right, _Off);
    }

    size_t              find_first_not_of(const value_type* Ptr, size_t _Off, size_t _Count) const
    {   // look for none of [_Ptr, _Ptr + _Count) at or after _Off
        return tstr().find_last_not_of(Ptr, _Off, _Count);
    }

    size_t              find_first_not_of(const value_type* Ptr, size_t _Off = 0) const
    {   // look for one of [_Ptr, <null>) at or after _Off
        return tstr().find_last_not_of(Ptr, _Off);
    }

    size_t              find_first_not_of(value_type* _Ch, size_t _Off = 0) const
    {   // look for non _Ch at or after _Off
        return tstr().find_last_not_of(_Ch, _Off);
    }

    size_t              find_last_not_of(const tstring& _Right, size_t _Off = npos) const
    {   // look for none of _Right before _Off
        return tstr().find_last_not_of(_Right, _Off);
    }

    size_t              find_last_not_of(const value_type* Ptr, size_t _Off, size_t _Count) const
    {   // look for none of [_Ptr, _Ptr + _Count) before _Off
        return tstr().find_last_not_of(Ptr, _Off, _Count);
    }

    size_t              find_last_not_of(const value_type* Ptr, size_t _Off = npos) const
    {   // look for none of [_Ptr, <null>) before _Off
        return tstr().find_last_not_of(Ptr, _Off);
    }

    size_t              find_last_not_of(value_type* _Ch, size_t _Off = npos) const
    {   // look for non _Ch before _Off
        return tstr().find_last_not_of(_Ch, _Off);
    }

    tstring             substr(size_t _Off = 0, size_t _Count = npos) const
    {   // return [_Off, _Off + _Count) as new string
        return tstr().substr(_Off, _Count);
    }

    int                 compare(const uniquestr_t& _Right) const
    {   // compare [0, _Mysize) with _Right
        return (*this == _Right);
    }

    int                 compare(const tstring& _Right) const
    {   // compare [0, _Mysize) with _Right
        return tstr().compare(_Right);
    }
    int                 compare(size_t _Off, size_t _N0, const tstring& _Right) const
    {   // compare [_Off, _Off + _N0) with _Right
        return tstr().compare(_Off, _N0, _Right);
    }

    int                 compare(size_t _Off, size_t _N0, const tstring& _Right, size_t _Roff, size_t _Count) const
    {   // compare [_Off, _Off + _N0) with _Right [_Roff, _Roff + _Count)
        return tstr().compare(_Off, _N0, _Right, _Roff, _Count);
    }

    int                 compare(const value_type* Ptr) const
    {   // compare [0, _Mysize) with [_Ptr, <null>)
        return tstr().compare(Ptr);
    }

    int                 compare(size_t _Off, size_t _N0, const value_type* Ptr) const
    {   // compare [_Off, _Off + _N0) with [_Ptr, <null>)
        return tstr().compare(_Off, _N0, Ptr);
    }

    int                 compare(size_t _Off, size_t _N0, const value_type* Ptr, size_t _Count) const
    {   // compare [_Off, _Off + _N0) with [_Ptr, _Ptr + _Count)
        return tstr().compare(_Off, _N0, Ptr, _Count);
    }
};
//=============================================================================
#endif

#endif //__C_STRINGPOOL_H__
