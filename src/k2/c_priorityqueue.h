// (C)2007 S2 Games
// c_priorityqueue.h
//
//=============================================================================
#ifndef __C_PRIORITYQUEUE_H__
#define __C_PRIORITYQUEUE_H__

//=============================================================================
// CPriorityQueue
//=============================================================================
template <class T, class _Pred>
class CPriorityQueue
{
private:
    T       *m_pHeap;
    uint    m_uiHeapSize;
    uint    m_uiMaxHeapSize;

public:
    ~CPriorityQueue()
    {
        if (m_pHeap != NULL)
            K2_DELETE_ARRAY(m_pHeap);
    }

    CPriorityQueue() : m_pHeap(NULL), m_uiHeapSize(0) {}

    bool    Empty()         { return m_uiHeapSize == 0; }
    void    Init(uint uiMaxElements);
    void    Update(T _Element);
    void    FullUpdate(T _Element);
    void    Push(T _Insert);
    void    PushEx(T _Insert); // Requires the heap to be rebuilt after usage
    T       Pop();
    T&      Peek();
    void    Rebuild();
    void    Reset();
    uint    Size()          { return m_uiHeapSize; }

    T*      GetHead()       { return m_pHeap; }
    T*      GetTail()       { return &m_pHeap[m_uiHeapSize]; }
};
//=============================================================================

//=============================================================================
// Template Functions
//=============================================================================

/*====================
  CPriorityQueue<>::Init
  ====================*/
template <class T, class _Pred>
void    CPriorityQueue<T, _Pred>::Init(uint uiMaxElements)
{
    if (m_pHeap == NULL)
    {
        m_pHeap = K2_NEW_ARRAY(ctx_Nav, T, uiMaxElements);
        m_uiMaxHeapSize = uiMaxElements;
    }
}


/*====================
  CPriorityQueue<>::Update

  Can only update an element that reduced in value
  ====================*/
template <class T, class _Pred>
void    CPriorityQueue<T, _Pred>::Update(T _Element)
{
    for (uint i(0); i < m_uiHeapSize; ++i)
    {
        if (m_pHeap[i] == _Element)
        {
            std::push_heap(m_pHeap, &m_pHeap[i + 1], _Pred());
            return;
        }
    }
}


/*====================
  CPriorityQueue<>::FullUpdate
  ====================*/
template <class T, class _Pred>
void    CPriorityQueue<T, _Pred>::FullUpdate(T _Element)
{
    std::make_heap(m_pHeap, &m_pHeap[m_uiHeapSize], _Pred());
}


/*====================
  CPriorityQueue<>::Push
  ====================*/
template <class T, class _Pred>
void    CPriorityQueue<T, _Pred>::Push(T _Insert)
{
    assert(m_uiHeapSize != m_uiMaxHeapSize);

    m_pHeap[m_uiHeapSize] = _Insert;
    std::push_heap(m_pHeap, &m_pHeap[++m_uiHeapSize], _Pred());
}


/*====================
  CPriorityQueue<>::PushEx
  ====================*/
template <class T, class _Pred>
void    CPriorityQueue<T, _Pred>::PushEx(T _Insert)
{
    m_pHeap[m_uiHeapSize++] = _Insert;
}


/*====================
  CPriorityQueue<>::Pop
  ====================*/
template <class T, class _Pred>
T   CPriorityQueue<T, _Pred>::Pop()
{
    T _Ret(m_pHeap[0]);

    std::pop_heap(m_pHeap, &m_pHeap[m_uiHeapSize--], _Pred());

    return _Ret;
}


/*====================
  CPriorityQueue<>::Peek
  ====================*/
template <class T, class _Pred>
T&  CPriorityQueue<T, _Pred>::Peek()
{
    return m_pHeap[0];
}


/*====================
  CPriorityQueue<>::Rebuild
  ====================*/
template <class T, class _Pred>
void    CPriorityQueue<T, _Pred>::Rebuild()
{
    std::make_heap(m_pHeap, &m_pHeap[m_uiHeapSize], _Pred());
}


/*====================
  CPriorityQueue<>::Reset
  ====================*/
template <class T, class _Pred>
void    CPriorityQueue<T, _Pred>::Reset()
{
    m_uiHeapSize = 0;
}
//=============================================================================

#endif //__C_PRIORITYQUEUE_H__
