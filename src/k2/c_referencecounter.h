// (C)2005 S2 Games
// c_referencecounter.h
//
//=============================================================================
#ifndef __C_REFERENCECOUNTER_H__
#define __C_REFERENCECOUNTER_H__

//=============================================================================
// CReferenceCounter
//=============================================================================
template <class T>
class CReferenceCounter
{
private:
    T   *m_pObject;
    int m_iCount;

public:
    CReferenceCounter();
    CReferenceCounter(T *pObject);
    CReferenceCounter(const CReferenceCounter &rc);
    ~CReferenceCounter();

    T*                  Get()               { return m_pObject; }

    bool                operator==(int n)   { return m_iCount == n; }
    CReferenceCounter&  operator=(const CReferenceCounter &rc);
    CReferenceCounter&  operator++()        { ++m_iCount; return *this; }
    CReferenceCounter&  operator--()        { --m_iCount; return *this; }
    T*                  operator->()        { return m_pObject; }
    T&                  operator*()         { return *m_pObject; }
};

template <class T>
inline
CReferenceCounter<T>::CReferenceCounter() :
m_pObject(nullptr),
m_iCount(1)
{
}

template <class T>
inline
CReferenceCounter<T>::CReferenceCounter(T *pObject) :
m_pObject(pObject),
m_iCount(1)
{
}

template <class T>
inline
CReferenceCounter<T>::CReferenceCounter(const CReferenceCounter &rc)
{
    m_pObject = rc.m_pObject;
    m_iCount = ++rc.m_iCount;
}

template <class T>
inline
CReferenceCounter<T>::~CReferenceCounter()
{
    assert(m_iCount == 0);
    if (m_pObject != nullptr)
        K2_DELETE(m_pObject);
}

template <class T>
inline
CReferenceCounter<T>&   CReferenceCounter<T>::operator=(const CReferenceCounter<T> &rc)
{
    m_pObject = rc.m_pObject;
    m_iCount = ++rc.m_iCount;
}
//=============================================================================
#endif //__C_REFERENCECOUNTER_H__
