// (C)2005 S2 Games
// c_heapfactory.h
//
//=============================================================================
#ifndef __C_HEAPFACTORY_H__
#define __C_HEAPFACTORY_H__

//=============================================================================
// CHeapFactory
//=============================================================================
class CHeapFactory
{
    DECLARE_SINGLETON(CHeapFactory)

private:

public:
    CHeap*  GetHeap(const char *szName);
};
//=============================================================================

// Singleton access
extern SHARED_API CHeapFactory *g_pHeapFactory;

#ifdef SHARED_EXPORTS
#define HeapFactory (*CHeapFactory::GetInstance())
#else
#define HeapFactory (*g_pHeapFactory)
#endif

#endif //__C_HEAPFACTORY_H__
