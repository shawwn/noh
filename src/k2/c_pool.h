// (C)2006 S2 Games
// c_pool.h
//
// Object instance pool template class
//=============================================================================
#ifndef __C_POOL_H__
#define __C_POOL_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================


//=============================================================================
// CPool<T>
//=============================================================================
template <class T>
class CPool
{
private:
    T*          m_pBuffer;
    uint        m_uiSize;
    uint        m_uiPos;

    uint        m_uiMaxSize;

    vector<T*>  m_vOldBuffers;

    CPool();
    
public:
    ~CPool()    { Reset(); MemManager.Deallocate(m_pBuffer, "c_pool"); m_pBuffer = NULL; }
    CPool(uint uiStartSize, uint uiMaxSize);
    CPool(const CPool &C);
    CPool&  operator=(const CPool &A);

    uint                        GetSize() const             { return m_uiSize; }

    T*                          Allocate();
    void                        Reset();
};


/*====================
  CPool::CPool
  ====================*/
template <class T>
inline
CPool<T>::CPool(uint uiStartSize, uint uiMaxSize) :
m_pBuffer(static_cast<T*>(MemManager.Allocate(sizeof(T) * MAX(uiStartSize, 1u), "c_pool"))),
m_uiSize(uint(MAX(uiStartSize, 1u))),
m_uiPos(0),
m_uiMaxSize(uiMaxSize)
{
}


/*====================
  CPool::CPool
  ====================*/
template <class T>
inline
CPool<T>::CPool(const CPool &C) :
m_pBuffer(static_cast<T*>(MemManager.Allocate(sizeof(T) * C.m_uiSize, "c_pool"))),
m_uiSize(C.m_uiSize),
m_uiPos(0),
m_uiMaxSize(C.m_uiMaxSize)
{
}


/*====================
  CPool::CPool
  ====================*/
template <class T>
inline
CPool<T>&   CPool<T>::operator=(const CPool &A)
{
    //Reset();
    MemManager.Deallocate(m_pBuffer, "c_pool");

    m_pBuffer = static_cast<T*>(MemManager.Allocate(sizeof(T) * A.m_uiSize, "c_pool"));
    m_uiSize = A.m_uiSize;
    m_uiPos = 0;
    m_uiMaxSize = A.m_uiMaxSize;

    return *this;
}


/*====================
  CPool::Reset
  ====================*/
template <class T>
inline
void    CPool<T>::Reset()
{
    for (typename vector<T*>::iterator it(m_vOldBuffers.begin()); it != m_vOldBuffers.end(); ++it)
        MemManager.Deallocate(*it, "c_pool");

    m_vOldBuffers.clear();

    m_uiPos = 0;
}


/*====================
  CPool::Allocate
  ====================*/
template <class T>
inline
T*  CPool<T>::Allocate()
{
    if (m_uiPos == m_uiSize)
    {
        if (m_uiSize != m_uiMaxSize)
        {
            m_uiSize = uint(MIN(m_uiSize * 2u, m_uiMaxSize));
            m_vOldBuffers.push_back(m_pBuffer);
            m_pBuffer = static_cast<T*>(MemManager.Allocate(sizeof(T) * m_uiSize, "c_pool"));
        }
        else
            EX_WARN(_T("Pool new() requested more instances than are available"));
    }

    return &m_pBuffer[m_uiPos++];
}
//=============================================================================
#endif // __C_POOL_H__
