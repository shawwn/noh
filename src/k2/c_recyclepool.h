// (C)2007 S2 Games
// c_recyclepool.h
//
//=============================================================================
#ifndef __C_RECYCLEPOOL_H__
#define __C_RECYCLEPOOL_H__

//=============================================================================
// Definitions
//=============================================================================
const int INVALID_POOL_OFFSET(0);
const int POOL_FAULT_OUTOFRANGE(1); // Request beyond the limit index 
const int POOL_FAULT_OVERWRITE(2); // Request for an index already in use
const int POOL_FAULT_NOTEXISTS(4); // Request for a non-existant data member
const int POOL_FAULT_FREEDNULL(8); // Freed an index not in use

#define VALID_PH(hPoolHandle) (hPoolHandle != INVALID_POOL_HANDLE)

#define STRINGIZE(s) STRINGIZE_HELPER(s) 
#define STRINGIZE_HELPER(s) #s

typedef int PoolOffset;
typedef vector<bool> AllocMap;
typedef pair<uint, PoolHandle> PoolMapEntry;
//=============================================================================

//=============================================================================
// CRecyclePool<T>
//=============================================================================
template <class T>
class CRecyclePool
{
private:
    T* m_pBuffer;
    uint m_uiSize;
    uint m_uiPos;

    AllocMap m_vAllocated;
    deque<uint> m_Released;

    mutable int m_iFaults;

    CRecyclePool();

    inline void CheckForResize()
    {
        if (m_uiPos >= m_uiSize && m_Released.size() == 0)
        {
            PROFILE("CRecyclePool::CheckForResize");

            uint uiNewSize(m_uiSize + (m_uiSize >> 1)); // Grow by 50%

            if (uiNewSize >= INVALID_POOL_HANDLE)
            {
#if defined(UNICODE) && defined(__GNUC__)
                Console.Err << StringToTString(__FUNCTION__) << _T(" Pool overflow") << newl;
#else
                Console.Err << _T(__FUNCTION__) << _T(" Pool overflow") << newl;
#endif
                m_iFaults |= POOL_FAULT_OUTOFRANGE;
                return;
            }

#if defined(UNICODE) && defined(__GNUC__)
            Console.Dev << StringToTString(__FUNCTION__) << _T(" Resize ") << uiNewSize << newl;
#else
            Console.Dev << _T(__FUNCTION__) <<_T(" Resize ") << uiNewSize << newl;
#endif

            T *pNew(K2_NEW_ARRAY(ctx_Pools, T, uiNewSize));

            // Transfer data to new buffer
            for (uint ui(0); ui < m_uiPos; ++ui)
                pNew[ui] = m_pBuffer[ui]; // Explicit call to the buffer operator=

            // Release old buffer
            K2_DELETE_ARRAY(m_pBuffer);

            // Store new buffer
            m_pBuffer = pNew;
            m_uiSize = uiNewSize;
            // Resize the alloc status vector
            m_vAllocated.resize(m_uiSize, false);
        }
    }

    inline PoolHandle GetAvailableHandle()
    {
        PoolHandle hHandle(INVALID_POOL_HANDLE);

        if (m_Released.size())
        {
            hHandle = m_Released.back();
            m_Released.pop_back();
        }
        else if (m_uiPos < m_uiSize)
        {
            hHandle = m_uiPos++;
        }

        return hHandle;
    }

public:
    inline CRecyclePool(uint size) : m_uiSize(size), m_uiPos(0), m_vAllocated(size), m_iFaults(0) { m_pBuffer = K2_NEW_ARRAY(ctx_Pools, T, m_uiSize); }
    ~CRecyclePool() { SAFE_DELETE_ARRAY(m_pBuffer); }

    // DO NOT CALL w/ A REFERENCE OBTAINED FROM Get()
    // Allocating can invalidate references into the pool
    PoolHandle New(const T &cInitialState)
    {
        CheckForResize();

        PoolHandle hHandle(GetAvailableHandle());

        if (hHandle == INVALID_POOL_HANDLE)
            return INVALID_POOL_HANDLE;

        m_pBuffer[hHandle] = cInitialState;
        m_vAllocated[hHandle] = true;

        return hHandle;
    }

    // Used to create a new member w/ a value copied from within the pool
    PoolHandle NewFromHandle(PoolHandle hCopyFrom)
    {
        CheckForResize();

        PoolHandle hCopyTo(GetAvailableHandle());
        
        if (hCopyTo == INVALID_POOL_HANDLE)
            return INVALID_POOL_HANDLE;

        m_pBuffer[hCopyTo] = m_pBuffer[hCopyFrom];
        m_vAllocated[hCopyTo] = true;

        return hCopyTo;
    }

    T*  GetReferenceByHandle(PoolHandle hHandle) const
    {
        if (hHandle == INVALID_POOL_HANDLE)
            return NULL;

#ifdef K2_FAULT_NOTEXIST
        if (!m_vAllocated[hHandle])
        {
            m_iFaults |= POOL_FAULT_NOTEXISTS;
            return NULL;
        }
        else
            return &m_pBuffer[hHandle];
#else
        return &m_pBuffer[hHandle];
#endif
    }

    PoolOffset GetHandleByReference(T *pRef)
    {
        ptrdiff_t iOffset(pRef - m_pBuffer);

#ifdef K2_FAULT_NOTEXIST
        if (iOffset < 0 || iOffset >= int(m_uiSize) || !m_vAllocated[PoolHandle(iOffset)])
        {
            m_iFaults |= POOL_FAULT_NOTEXISTS;
            return INVALID_POOL_OFFSET;
        }
        else
        {
            return PoolHandle(iOffset);
        }
#else
        return PoolHandle(iOffset);
#endif
    }

    void    Free(PoolHandle hHandle)
    {
        if (hHandle == INVALID_POOL_HANDLE)
            return;

        if (m_vAllocated[hHandle])
        {
            m_Released.push_back(hHandle);
            m_vAllocated[hHandle] = false;
        }
        else
        {
            m_iFaults |= POOL_FAULT_FREEDNULL;
        }
    }

    int     GetFaults() const
    {
        return m_iFaults;
    }

    uint    GetNumAllocated()
    {
        return uint(m_uiPos - m_Released.size());
    }

    void    Clear()
    {
        for (uint ui(0); ui < m_uiPos; ++ui)
            m_vAllocated[ui] = false;

        m_Released.clear();
        
        m_uiPos = 0;
        m_iFaults = 0;
    }
};
//=============================================================================

#endif //__C_RECYCLEPOOL_H__
