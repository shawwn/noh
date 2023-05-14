// (C)2007 S2 Games
// k2_strings.h
//
//=============================================================================
#ifndef __K2_STRINGS_H__
#define __K2_STRINGS_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_stl_allocator.h"
//=============================================================================

#ifdef K2_USE_STL_ALLOCATOR
#define K2_STRINGS_allocator        K2StringAllocator
#else
#define K2_STRINGS_allocator        std::allocator
#endif

namespace K2
{
    typedef std::basic_string<char, std::char_traits<char>, K2_STRINGS_allocator<char> >            string;
    typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, K2_STRINGS_allocator<wchar_t> >   wstring;
    typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, K2_STRINGS_allocator<TCHAR> >         tstring;
    typedef std::basic_fstream<TCHAR, std::char_traits<TCHAR> > tfstream;
}

using K2::tstring;
using K2::string;
using K2::wstring;
using std::fstream;
using K2::tfstream;

#define _TS(s) tstring(_T(s))
#define _TSL(s) tstring(_T(s), sizeof(_T(s)) / sizeof(TCHAR) - 1)

#if TKTK // disable for now
//=============================================================================
// const_string
//=============================================================================
#ifdef _MSC_VER
#pragma pack(push, _CRT_PACKING)

template<class _Elem, class _Traits, class _Ax>
class const_string : public std::basic_string<_Elem, _Traits, _Ax>
{
private:
    uint    m_uiPad;

public:
    ~const_string()
    {
        _Bx._Ptr = nullptr;
    }

    const_string(const _Elem * const _Ptr) :
    basic_string()
    {
        _Bx._Ptr = (_Elem *)_Ptr;
        _Mysize = _Traits::length(_Ptr);
        _Myres = uint(-1);
    }

    const_string(const _Elem * const _Ptr, size_type _Count) :
    basic_string()
    {
        _Bx._Ptr = (_Elem *)_Ptr;
        _Mysize = _Count;
        _Myres = uint(-1);
    }
};

#pragma pack(pop)
#elif defined(__GNUC__)
// I'll look at this later -- gnu's basic string is a complex beast (there isn't just a pointer I can assign)
#endif
//=============================================================================
#endif // TKTK: disable for now

#ifdef _MSC_VER
#if TKTK // disable for now
typedef const_string<TCHAR, std::char_traits<TCHAR>, K2_STRINGS_allocator<TCHAR> > ctstring;
typedef const_string<char, std::char_traits<char>, K2_STRINGS_allocator<char> > cstring;
typedef const_string<wchar_t, std::char_traits<wchar_t>, K2_STRINGS_allocator<wchar_t> > cwstring;
#else
typedef const std::basic_string<TCHAR, std::char_traits<TCHAR>, K2_STRINGS_allocator<TCHAR> > ctstring;
typedef const std::basic_string<char, std::char_traits<char>, K2_STRINGS_allocator<char> > cstring;
typedef const std::basic_string<wchar_t, std::char_traits<wchar_t>, K2_STRINGS_allocator<wchar_t> > cwstring;
#endif
#elif defined(__GNUC__)
typedef const std::basic_string<TCHAR, std::char_traits<TCHAR>, K2_STRINGS_allocator<TCHAR> > ctstring;
typedef const std::basic_string<char, std::char_traits<char>, K2_STRINGS_allocator<char> > cstring;
typedef const std::basic_string<wchar_t, std::char_traits<wchar_t>, K2_STRINGS_allocator<wchar_t> > cwstring;
#endif

#define _CS(s) cstring(s)
#define _CTS(s) ctstring(_T(s), sizeof(_T(s)) / sizeof(TCHAR) - 1)
#define _CWS(s) cwstring(_T(s), sizeof(L##s) / sizeof(wchar_t) - 1)

#define CONST_STRING(name, sz) ctstring name(sz, sizeof(sz) / sizeof(TCHAR) - 1)

#undef K2_STRINGS_allocator

#endif //__K2_STRINGS_H__
