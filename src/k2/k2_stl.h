// (C)2005 S2 Games
// k2_stl.h
//
//=============================================================================
#ifndef __K2_STL_H__
#define __K2_STL_H__

//=============================================================================
// Headers
//=============================================================================
#include <algorithm>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <stack>
#if TKTK
#ifdef __GNUC__
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ > 2) // gcc >= 4.3
// hash_map is depreciated under gcc-3.4, use tr1 equivalents
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#else
#include <ext/hash_map>
#include <ext/hash_set>
#endif
#else
#include <hash_map>
#include <hash_set>
#endif
#else
#include <unordered_map>
#include <unordered_set>
#endif
#include <utility>
#include <sstream>
#include <limits>
#include "k2_stl_allocator.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
#include <random>
namespace K2
{
    template<class RandomAccessIterator>
    void random_shuffle(RandomAccessIterator first, RandomAccessIterator last) {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(first, last, g);
    }
}

// to turn off trapping STL allocations, see k2_stl_allocator.h

// for containers, ignore the specified allocator and use K2Allocator instead.
#define K2_STL_allocator(args)              K2Allocator< args >
#define K2_STL_vector_allocator(args)       K2VectorAllocator< args >
#define K2_STL_list_allocator(args)         K2ListAllocator< args >
#define K2_STL_deque_allocator(args)        K2DequeAllocator< args >
#define K2_STL_map_allocator(args)          K2MapAllocator< args >
#define K2_STL_set_allocator(args)          K2SetAllocator< args >
#define K2_STL_hash_map_allocator(args)     K2HashMapAllocator< args >
#define K2_STL_hash_set_allocator(args)     K2HashSetAllocator< args >

// use the specified container allocator.
//#define K2_STL_allocator(args)            _Alloc
//#define K2_STL_vector_allocator(args)     _Alloc
//#define K2_STL_list_allocator(args)       _Alloc
//#define K2_STL_deque_allocator(args)      _Alloc
//#define K2_STL_map_allocator(args)        _Alloc
//#define K2_STL_set_allocator(args)        _Alloc
//#define K2_STL_hash_map_allocator(args)   _Alloc
//#define K2_STL_hash_set_allocator(args)   _Alloc

// basic declarations.
using std::abs;
using std::min;
using std::max;
using std::insert_iterator;
using std::pair;
using std::ostringstream;
using std::numeric_limits;

#ifndef K2_USE_STL_ALLOCATOR

using std::vector;
using std::list;
using std::map;
using std::deque;
using std::set;
using std::stack;
using std::multiset;

#else

// vector
#undef K2_STL_alloc_args
#define K2_STL_alloc_args   _Ty
template<class _Ty, class _Alloc = K2VectorAllocator<K2_STL_alloc_args> >
class vector
    : public std::vector< _Ty, K2_STL_vector_allocator(K2_STL_alloc_args) >
{
    typedef std::vector< _Ty, K2_STL_vector_allocator(K2_STL_alloc_args) >      _Mybase;
public:
    typedef vector< _Ty, _Alloc>                                        _Myt;
    typedef typename std::vector< _Ty, _Alloc >::size_type              size_type;
    
    vector()
        //: _Mybase()
    {
    }

    // construct from _Count * _Ty()
    explicit vector(size_type _Count)
        : _Mybase(_Count)
    {   
    }

    // construct from _Count * _Val
    vector(size_type _Count, const _Ty& _Val)
        : _Mybase(_Count, _Val)
    {   
    }

    // construct from _Count * _Val, with allocator
    vector(size_t _Count, const _Ty& _Val, const _Alloc& _Al)
        : _Mybase(_Count, _Val, _Al)
    {   
    }

    // construct by copying _Right
    vector(const _Myt& _Right)
        : _Mybase(_Right)
    {   
    }

    // construct from [_First, _Last)
    template<class _Iter>
    vector(_Iter _First, _Iter _Last)
        : _Mybase( _First, _Last )
    {
    }

    // construct from [_First, _Last), with allocator
    template<class _Iter>
    vector(_Iter _First, _Iter _Last, const _Alloc& _Al)
        : _Mybase(_First, _Last, _Al)
    {
    }
};

// list
#undef K2_STL_alloc_args
#define K2_STL_alloc_args   _Ty
template<class _Ty, class _Alloc = K2ListAllocator<K2_STL_alloc_args> >
class list
    : public std::list< _Ty, K2_STL_list_allocator(K2_STL_alloc_args) >
{
    typedef std::list< _Ty, K2_STL_list_allocator(K2_STL_alloc_args) >      _Mybase;
public:
    typedef list< _Ty, _Alloc>                                          _Myt;
    typedef typename std::list< _Ty, _Alloc >::size_type                size_type;
    
    list()
        //: _Mybase()
    {   // construct empty list
    }

    explicit list(const _Alloc& _Al)
        : _Mybase(_Al)
    {   // construct empty list, allocator
    }

    explicit list(size_type _Count)
        : _Mybase(_Count)
    {   // construct list from _Count * _Ty()
    }

    list(size_type _Count, const _Ty& _Val)
        : _Mybase(_Count, _Val)
    {   // construct list from _Count * _Val
    }

    list(size_type _Count, const _Ty& _Val, const _Alloc& _Al)
        : _Mybase(_Count, _Val, _Al)
    {   // construct list, allocator from _Count * _Val
    }

    list(const _Myt& _Right)
        : _Mybase(_Right)
    {   // construct list by copying _Right
    }

    template<class _Iter>
    list(_Iter _First, _Iter _Last)
        : _Mybase(_First, _Last)
    {   // construct list from [_First, _Last)
    }

    template<class _Iter>
    list(_Iter _First, _Iter _Last, const _Alloc& _Al)
        : _Mybase(_First, _Last, _Al)
    {   // construct list, allocator from [_First, _Last)
    }
};

// map
#undef K2_STL_alloc_args
#define K2_STL_alloc_args   pair< const _Kty, _Ty >
template< class _Kty, class _Ty, class _Pr = std::less< _Kty >, class _Alloc = K2MapAllocator<K2_STL_alloc_args> >
class map
    : public std::map< _Kty, _Ty, _Pr, K2_STL_map_allocator(K2_STL_alloc_args) >
{
    typedef std::map< _Kty, _Ty, _Pr, K2_STL_map_allocator(K2_STL_alloc_args) >     _Mybase;
public:
    typedef typename std::map< _Kty, _Ty, _Pr, _Alloc >::size_type              size_type;
    typedef typename std::map< _Kty, _Ty, _Pr, _Alloc >::key_compare            key_compare;

    map()
        //: _Mybase()
    {   // construct empty map from defaults
    }

    explicit map(const key_compare& _Pred)
        : _Mybase(_Pred)
    {   // construct empty map from comparator
    }

    map(const key_compare& _Pred, const _Alloc& _Al)
        : _Mybase(_Pred, _Al)
    {   // construct empty map from comparator and allocator
    }

    template<class _Iter>
    map(_Iter _First, _Iter _Last)
        : _Mybase(_First, _Last)
    {   // construct map from [_First, _Last), defaults
    }

    template<class _Iter>
    map(_Iter _First, _Iter _Last, const key_compare& _Pred)
        : _Mybase(_First, _Last, _Pred)
    {   // construct map from [_First, _Last), comparator
    }

    template<class _Iter>
    map(_Iter _First, _Iter _Last, const key_compare& _Pred, const _Alloc& _Al)
        : _Mybase(_First, _Last, _Pred, _Al)
    {   // construct map from [_First, _Last), comparator, and allocator
    }
};

// deque
#undef K2_STL_alloc_args
#define K2_STL_alloc_args   _Ty
template< class _Ty, class _Alloc = K2DequeAllocator<K2_STL_alloc_args>  >
class deque
    : public std::deque< _Ty, K2_STL_deque_allocator(K2_STL_alloc_args) >
{
    typedef std::deque< _Ty, K2_STL_deque_allocator(K2_STL_alloc_args) >        _Mybase;
public:
    typedef deque< _Ty, _Alloc>                                         _Myt;
    typedef typename std::deque< _Ty, _Alloc >::size_type               size_type;

    deque()
        //: _Mybase()
    {   // construct empty deque
    }

    explicit deque(const _Alloc& _Al)
        : _Mybase(_Al)
    {   // construct empty deque with allocator
    }

    explicit deque(size_type _Count)
        : _Mybase(_Count)
    {   // construct from _Count * _Ty()
    }

    deque(size_type _Count, const _Ty& _Val)
        : _Mybase(_Count, _Val)
    {   // construct from _Count * _Val
    }

    deque(size_type _Count, const _Ty& _Val, const _Alloc& _Al)
        : _Mybase(_Count, _Val, _Al)
    {   // construct from _Count * _Val with allocator
    }

    deque(const _Myt& _Right)
        : _Mybase(_Right)
    {   // construct by copying _Right
    }

    template<class _It>
    deque(_It _First, _It _Last)
        : _Mybase(_First, _Last)
    {   // construct from [_First, _Last)
    }

    template<class _It>
    deque(_It _First, _It _Last, const _Alloc& _Al)
        : _Mybase(_First, _Last, _Al)
    {   // construct from [_First, _Last) with allocator
    }
};

// set (ordered red-black tree of key values, unique keys)
#undef K2_STL_alloc_args
#define K2_STL_alloc_args   _Kty
template< class _Kty, class _Pr = std::less<_Kty>, class _Alloc = K2SetAllocator<K2_STL_alloc_args> >
class set
    : public std::set< _Kty, _Pr, K2_STL_set_allocator(K2_STL_alloc_args) >
{   
    typedef std::set< _Kty, _Pr, K2_STL_set_allocator(K2_STL_alloc_args) >      _Mybase;
public:
    typedef typename std::set< _Kty, _Pr, _Alloc >::key_compare key_compare;
    
    set()
        //: _Mybase()
    {   // construct empty set from defaults
    }

    explicit set(const key_compare& _Pred)
        : _Mybase(_Pred)
    {   // construct empty set from comparator
    }

    set(const key_compare& _Pred, const _Alloc& _Al)
        : _Mybase(_Pred, _Al)
    {   // construct empty set from comparator and allocator
    }

    template<class _Iter>
    set(_Iter _First, _Iter _Last)
        : _Mybase(_First, _Last)
    {   // construct set from [_First, _Last), defaults
    }

    template<class _Iter>
    set(_Iter _First, _Iter _Last, const key_compare& _Pred)
        : _Mybase(_First, _Last, _Pred)
    {   // construct set from [_First, _Last), comparator
    }

    template<class _Iter>
    set(_Iter _First, _Iter _Last, const key_compare& _Pred, const _Alloc& _Al)
        : _Mybase(_First, _Last, _Pred, _Al)
    {   // construct set from [_First, _Last), defaults, and allocator
    }
};

// multiset
#undef K2_STL_alloc_args
#define K2_STL_alloc_args   _Kty
template< class _Kty, class _Pr = std::less<_Kty>, class _Alloc = K2SetAllocator<K2_STL_alloc_args> >
class multiset
    : public std::multiset< _Kty, _Pr, K2_STL_set_allocator(K2_STL_alloc_args) >
{   
    typedef std::multiset< _Kty, _Pr, K2_STL_set_allocator(K2_STL_alloc_args) >     _Mybase;
public:
    typedef typename std::multiset< _Kty, _Pr, _Alloc >::key_compare key_compare;

    multiset()
        //: _Mybase()
    {   // construct empty set from defaults
    }

    explicit multiset(const key_compare& _Pred)
        : _Mybase(_Pred)
    {   // construct empty set from comparator
    }

    multiset(const key_compare& _Pred, const _Alloc& _Al)
        : _Mybase(_Pred, _Al)
    {   // construct empty set from comparator and allocator
    }

    template<class _Iter>
    multiset(_Iter _First, _Iter _Last)
        : _Mybase(_First, _Last)
    {   // construct set from [_First, _Last)
    }

    template<class _Iter>
    multiset(_Iter _First, _Iter _Last, const key_compare& _Pred)
        : _Mybase(_First, _Last, _Pred)
    {   // construct set from [_First, _Last), comparator
    }

    template<class _Iter>
    multiset(_Iter _First, _Iter _Last, const key_compare& _Pred, const _Alloc& _Al)
        : _Mybase(_First, _Last, _Pred, _Al)
    {   // construct set from [_First, _Last), comparator, and allocator
    }
};

// stack (LIFO queue implemented with a container)
template< class _Ty, class _Container = deque< _Ty > >
class stack
    : public std::stack< _Ty, _Container >
{   
    typedef std::stack< _Ty, _Container >       _Mybase;
public:

    stack()
        //: _Mybase()
    {   // construct with empty container
    }

    explicit stack(const _Container& _Cont)
        : _Mybase(_Cont)
    {   // construct by copying specified container
    }
};

#endif // #ifndef K2_USE_STL_ALLOCATOR

#ifdef __GNUC__
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ > 2) || __clang__ // gcc >= 4.3
using std::unordered_map;
#define hash_map unordered_map
using std::unordered_set;
#define hash_set unordered_set
#ifdef K2_USE_STL_ALLOCATOR
namespace std
{
namespace tr1
{
    template <> struct hash<K2::string>
    {
        size_t operator()(const K2::string &s) const
        {
            return std::tr1::_Fnv_hash<sizeof(size_t)>::hash(s.data(), s.length());
        }
    };
    template <> struct hash<K2::wstring>
    {
        size_t operator()(const K2::wstring &s) const
        {
            return std::tr1::_Fnv_hash<sizeof(size_t)>::hash(reinterpret_cast<const char *>(s.data()), s.length() * sizeof(wchar_t));
        }
    };
}
}
#endif // K2_USE_STL_ALLOCATOR
#else
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_set;
namespace __gnu_cxx
{
#ifdef __x86_64__
    // 64 bit
    const size_t FNV_OFFSET_BASIS(14695981039346656037UL);
    const size_t FNV_PRIME(1099511628211UL);
#else
    // 32 bit
    const size_t FNV_OFFSET_BASIS(2166136261UL);
    const size_t FNV_PRIME(16777619UL);
#endif
    // FNV-1a hash
    inline size_t fnv1a_hash(const char *p, size_t size)
    {
        size_t ret(FNV_OFFSET_BASIS);
        while (size-- > 0)
        {
            ret ^= *p++;
            ret *= FNV_PRIME;
        }
        return ret;
    }
    template <> struct hash<string>
    {
        size_t operator()(const string& s) const
        {
            return fnv1a_hash(s.c_str(), s.length());
        }
    };
    template <> struct hash<wstring>
    {
        size_t operator()(const wstring& s) const
        {
            return fnv1a_hash(reinterpret_cast<const char *>(s.c_str()), s.length() * sizeof(wchar_t));
        }
    };

    template <typename _T>
    struct hash<_T*>
    {
        size_t operator()(_T* p) const
        {
            return reinterpret_cast<size_t>(p);
        }
    };

}
#endif
#else

namespace K2
{
    // hash_map
    #undef K2_STL_alloc_args
    #define K2_STL_alloc_args   pair< const _Kty, _Ty >
    template<class _Kty,
        class _Ty,
        class _Tr = stdext::hash_compare< _Kty, std::less<_Kty> >,
        class _Alloc = K2HashMapAllocator<K2_STL_alloc_args> >
    class hash_map
        : public stdext::hash_map< _Kty, _Ty, _Tr, K2_STL_hash_map_allocator(K2_STL_alloc_args) >
    {
        typedef stdext::hash_map< _Kty, _Ty, _Tr, K2_STL_hash_map_allocator(K2_STL_alloc_args) >        _Mybase;
    public:
        typedef typename stdext::hash_map< _Kty, _Ty, _Tr, _Alloc >::size_type              size_type;
        typedef typename stdext::hash_map< _Kty, _Ty, _Tr, _Alloc >::key_compare            key_compare;

        hash_map()
            //: _Mybase()
        {   // construct empty map from defaults
        }

        explicit hash_map(const key_compare& _Traits)
            : _Mybase(_Traits)
        {   // construct empty map from comparator
        }

        hash_map(const key_compare& _Traits, const _Alloc& _Al)
            : _Mybase(_Traits, _Al)
        {   // construct empty map from comparator and allocator
        }

        template<class _Iter>
        hash_map(_Iter _First, _Iter _Last)
            : _Mybase(_First, _Last)
        {   // construct map from [_First, _Last), defaults
        }

        template<class _Iter>
        hash_map(_Iter _First, _Iter _Last, const key_compare& _Traits)
            : _Mybase(_First, _Last, _Traits)
        {   // construct map from [_First, _Last), comparator
        }

        template<class _Iter>
        hash_map(_Iter _First, _Iter _Last, const key_compare& _Traits, const _Alloc& _Al)
            : _Mybase(_First, _Last, _Traits, _Al)
        {   // construct map from [_First, _Last), comparator, and allocator
        }
    };

    // hash_set
    #undef K2_STL_alloc_args
    #define K2_STL_alloc_args   _Kty
    template<class _Kty,
        class _Tr = stdext::hash_compare< _Kty, std::less<_Kty> >,
        class _Alloc = K2HashSetAllocator<K2_STL_alloc_args> >
    class hash_set
        : public stdext::hash_set< _Kty, _Tr, K2_STL_hash_set_allocator(K2_STL_alloc_args) >
    {
        typedef stdext::hash_set< _Kty, _Tr, K2_STL_hash_set_allocator(K2_STL_alloc_args) >     _Mybase;
    public:
        typedef typename stdext::hash_set< _Kty, _Tr, _Alloc >::size_type               size_type;
        typedef typename stdext::hash_set< _Kty, _Tr, _Alloc >::key_compare         key_compare;

        hash_set()
            //: _Mybase()
        {   // construct empty map from defaults
        }

        explicit hash_set(const key_compare& _Traits)
            : _Mybase(_Traits)
        {   // construct empty map from comparator
        }

        hash_set(const key_compare& _Traits, const _Alloc& _Al)
            : _Mybase(_Traits, _Al)
        {   // construct empty map from comparator and allocator
        }

        template<class _Iter>
        hash_set(_Iter _First, _Iter _Last)
            : _Mybase(_First, _Last)
        {   // construct map from [_First, _Last), defaults
        }

        template<class _Iter>
        hash_set(_Iter _First, _Iter _Last, const key_compare& _Traits)
            : _Mybase(_First, _Last, _Traits)
        {   // construct map from [_First, _Last), comparator
        }

        template<class _Iter>
        hash_set(_Iter _First, _Iter _Last, const key_compare& _Traits, const _Alloc& _Al)
            : _Mybase(_First, _Last, _Traits, _Al)
        {   // construct map from [_First, _Last), comparator, and allocator
        }
    };
}

#if TKTK
// in release mode, you can get an increase in performance of up to ~3.5x in practice
// by using a hash_map or hash_set, rather than map or set.  And it is always going
// to be faster to use a hash_map than a map, and hash_set instead of set.
//
// the ONLY reason to ever use 'map' or 'set' over 'hash_map' and 'hash_set' is because
// you care that the resulting elements are in perfect sorted order --- but you almost
// never care about the ordering of the elements.

// minor note (do not worry about this; nevertheless, I am documenting it):
// in debug mode, the unoptimized performance of hash_map and hash_set is significantly
// slower than map / set.  This is because of the math involved with hashing elements,
// and also the unoptimized time it takes hash containers to reallocate their buckets
// (when inserting a bunch of elements into the hash_map).  Therefore, in debug mode,
// hash_map will resolve to map, and hash_set will resolve to set.
#ifdef _DEBUG
#define K2_NO_HASH_CONTAINERS
#endif
#endif

//
// again, please ALWAYS use hash_map and hash_set unless you REALLY need your elements
// to be in "perfect sorted order" (which you almost never do).
//
#ifdef K2_NO_HASH_CONTAINERS
#define hash_map    map
#define hash_set    set
#else
using K2::hash_map;
using K2::hash_set;
#endif

#endif
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================

// Common containers
typedef vector<byte>                yvector;
typedef yvector::iterator           yvector_it;
typedef yvector::const_iterator     yvector_cit;
typedef yvector::reverse_iterator   yvector_rit;

typedef vector<int>                 ivector;
typedef ivector::iterator           ivector_it;
typedef ivector::const_iterator     ivector_cit;
typedef ivector::reverse_iterator   ivector_rit;

typedef vector<uint>                uivector;
typedef uivector::iterator          uivector_it;
typedef uivector::const_iterator    uivector_cit;
typedef uivector::reverse_iterator  uivector_rit;

typedef vector<float>               fvector;
typedef fvector::iterator           fvector_it;
typedef fvector::const_iterator     fvector_cit;
typedef fvector::reverse_iterator   fvector_rit;

typedef vector<tstring>             tsvector;
typedef tsvector::iterator          tsvector_it;
typedef tsvector::const_iterator    tsvector_cit;
typedef tsvector::reverse_iterator  tsvector_rit;

typedef vector<string>              svector;
typedef svector::iterator           svector_it;
typedef svector::const_iterator     svector_cit;
typedef svector::reverse_iterator   svector_rit;

typedef vector<wstring>             wsvector;
typedef wsvector::iterator          wsvector_it;
typedef wsvector::const_iterator    wsvector_cit;
typedef wsvector::reverse_iterator  wsvector_rit;

typedef vector<size_t>              zvector;
typedef zvector::iterator           zvector_it;
typedef zvector::const_iterator     zvector_cit;
typedef zvector::reverse_iterator   zvector_rit;

typedef list<int>                   ilist;
typedef ilist::iterator             ilist_it;
typedef ilist::const_iterator       ilist_cit;
typedef ilist::reverse_iterator     ilist_rit;

typedef list<uint>                  uilist;
typedef uilist::iterator            uilist_it;
typedef uilist::const_iterator      uilist_cit;
typedef uilist::reverse_iterator    uilist_rit;

typedef list<float>                 flist;
typedef flist::iterator             flist_it;
typedef flist::const_iterator       flist_cit;
typedef flist::reverse_iterator     flist_rit;

typedef list<tstring>               slist;
typedef slist::iterator             slist_it;
typedef slist::const_iterator       slist_cit;
typedef slist::reverse_iterator     slist_rit;

typedef list<size_t>                zlist;
typedef zlist::iterator             zlist_it;
typedef zlist::const_iterator       zlist_cit;
typedef zlist::reverse_iterator     zlist_rit;

typedef set<int>                    iset;
typedef iset::iterator              iset_it;
typedef iset::const_iterator        iset_cit;
typedef iset::reverse_iterator      iset_rit;

typedef set<uint>                   uiset;
typedef uiset::iterator             uiset_it;
typedef uiset::const_iterator       uiset_cit;
typedef uiset::reverse_iterator     uiset_rit;

typedef set<float>                  fset;
typedef fset::iterator              fset_it;
typedef fset::const_iterator        fset_cit;
typedef fset::reverse_iterator      fset_rit;

typedef set<tstring>                sset;
typedef sset::iterator              sset_it;
typedef sset::const_iterator        sset_cit;
typedef sset::reverse_iterator      sset_rit;

typedef set<size_t>                 zset;
typedef zset::iterator              zset_it;
typedef zset::const_iterator        zset_cit;
typedef zset::reverse_iterator      zset_rit;

typedef deque<int>                  ideque;
typedef ideque::iterator            ideque_it;
typedef ideque::const_iterator      ideque_cit;
typedef ideque::reverse_iterator    ideque_rit;

typedef deque<uint>                 uideque;
typedef uideque::iterator           uideque_it;
typedef uideque::const_iterator     uideque_cit;
typedef uideque::reverse_iterator   uideque_rit;

typedef deque<float>                fdeque;
typedef fdeque::iterator            fdeque_it;
typedef fdeque::const_iterator      fdeque_cit;
typedef fdeque::reverse_iterator    fdeque_rit;

typedef deque<tstring>              sdeque;
typedef sdeque::iterator            sdeque_it;
typedef sdeque::const_iterator      sdeque_cit;
typedef sdeque::reverse_iterator    sdeque_rit;

typedef deque<size_t>               zdeque;
typedef zdeque::iterator            zdeque_it;
typedef zdeque::const_iterator      zdeque_cit;
typedef zdeque::reverse_iterator    zdeque_rit;

typedef map<string, string>         smaps;
typedef smaps::iterator             smaps_it;
typedef smaps::const_iterator       smaps_cit;
typedef smaps::reverse_iterator     smaps_rit;

typedef map<tstring, tstring>       tsmapts;
typedef tsmapts::iterator           tsmapts_it;
typedef tsmapts::const_iterator     tsmapts_cit;
typedef tsmapts::reverse_iterator   tsmapts_rit;

typedef map<int, tstring>           imaps;
typedef imaps::iterator             imaps_it;
typedef imaps::const_iterator       imaps_cit;
typedef imaps::reverse_iterator     imaps_rit;

typedef map<short, tstring>         nmaps;
typedef nmaps::iterator             nmaps_it;
typedef nmaps::const_iterator       nmaps_cit;
typedef nmaps::reverse_iterator     nmaps_rit;

typedef map<uint, tstring>          uimaps;
typedef uimaps::iterator            uimaps_it;
typedef uimaps::const_iterator      uimaps_cit;
typedef uimaps::reverse_iterator    uimaps_rit;

typedef map<ushort, tstring>        unmaps;
typedef unmaps::iterator            unmaps_it;
typedef unmaps::const_iterator      unmaps_cit;
typedef unmaps::reverse_iterator    unmaps_rit;

typedef map<int, int>               imapi;
typedef imapi::iterator             imapi_it;
typedef imapi::const_iterator       imapi_cit;
typedef imapi::reverse_iterator     imapi_rit;

typedef map<int, byte>              imapy;
typedef imapy::iterator             imapy_it;
typedef imapy::const_iterator       imapy_cit;
typedef imapy::reverse_iterator     imapyrit;

typedef hash_set<tstring>                   TStringSet;
typedef TStringSet::iterator                TStringSet_it;
typedef TStringSet::const_iterator          TStringSet_cit;

typedef set<tstring>                        TStringSortedSet;
typedef TStringSortedSet::iterator          TStringSortedSet_it;
typedef TStringSortedSet::const_iterator    TStringSortedSet_cit;
typedef TStringSortedSet::reverse_iterator  TStringSortedSet_rit;

// use this ONLY for associative containers; for sequences, use it = container.erase
#ifdef __GNUC__
#define STL_ERASE(container, it)    container.erase(it++)
#else
#define STL_ERASE(container, it)    it = container.erase(it)
#endif
//=============================================================================

#endif //__K2_STL_H__
