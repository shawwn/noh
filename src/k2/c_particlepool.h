// (C)2008 S2 Games
// c_particlepool.h
//
//=============================================================================
#ifndef __C_PARTICLEPOOL_H__
#define __C_PARTICLEPOOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_singleton.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CSimpleParticle;

const uint NUM_PARTICLEPOOL_BUCKETS(16);

#define ParticlePool    (*CParticlePool::GetInstance())
//=============================================================================

//=============================================================================
// CParticlePool
//=============================================================================
class CParticlePool
{
    SINGLETON_DEF(CParticlePool)

private:
    vector<byte*>       m_vFreeLists[NUM_PARTICLEPOOL_BUCKETS];

public:
    ~CParticlePool();

    CSimpleParticle*    Allocate(uint uiCount);
    void                Deallocate(CSimpleParticle *pPtr);
};
//=============================================================================

//=============================================================================
// CParticleAllocator
//=============================================================================
template<class T>
class CParticleAllocator : public std::allocator<T>
{
public:
#ifdef __GNUC__
    typedef typename std::allocator<T>::size_type   size_type;
    typedef typename std::allocator<T>::pointer     pointer;
#endif

#ifndef WIN32
    template<class _Other>
    struct rebind
    {
        typedef CParticleAllocator<_Other> other;
    };
#endif

    CParticleAllocator() {}

#ifdef __GNUC__
    CParticleAllocator(const CParticleAllocator &__a) : std::allocator<T>(__a) {}
    template<class _Other>
    CParticleAllocator(const CParticleAllocator<_Other>&) {}
    ~CParticleAllocator() {}
#else
    template<class _Other>
    allocator<T>& operator=(const allocator<_Other>&)
    {   // assign from a related allocator (do nothing)
    return (*this);
    }
#endif

    pointer allocate(size_type _Count)
    {
        return (pointer)ParticlePool.Allocate(uint(_Count));
    }

    void    deallocate(pointer _Ptr, size_type)
    {
        ParticlePool.Deallocate(_Ptr);
    }
    
    pointer allocate(size_type _Count, const void *)    { allocate(_Count); }
};
//=============================================================================

#endif  //__C_SIMPLEPARTICLE_H__
