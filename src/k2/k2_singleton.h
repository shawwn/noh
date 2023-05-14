// (C)2005 S2 Games
// k2_singleton.h
//
// Set of macros for easy singleton implementation
//=============================================================================
#ifndef __K2_SINGLETON_H__
#define __K2_SINGLETON_H__

//=============================================================================
// SINGLETON_DEF
//=============================================================================
#define SINGLETON_DEF(name) \
private: \
    static name*    s_pInstance; \
    static bool     s_bRequested, s_bReleased; \
    name(); \
    name(const name&); \
    name& operator=(const name&); \
\
public: \
    static inline name* GetInstance() \
    { \
        assert(!s_bReleased); \
        \
        if (s_pInstance == nullptr) \
        { \
            assert(!s_bRequested); \
            s_bRequested = true; \
            s_pInstance = K2_NEW(ctx_Singleton,  name); \
        } \
        \
        return s_pInstance; \
    } \
\
    static inline bool  IsReleased()    { return s_bReleased; } \
    static inline bool  IsAllocated()   { return s_pInstance != nullptr; } \
\
    static void     Release();
//=============================================================================

//=============================================================================
// SINGLETON_INIT
//=============================================================================
#define SINGLETON_INIT(name) \
name*   name::s_pInstance; \
bool    name::s_bRequested; \
bool    name::s_bReleased; \
\
void    name::Release() \
{ \
    assert(!s_bReleased); \
    \
    if (s_pInstance != nullptr) \
        K2_DELETE(s_pInstance); \
    \
    s_bReleased = true; \
}
//=============================================================================

#endif //__K2_SINGLETON_H__
