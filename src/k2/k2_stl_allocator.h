// (C)2010 S2 Games
// k2_stl_allocator.h
//
//=============================================================================
#ifndef __K2_STL_ALLOCATOR_H__
#define __K2_STL_ALLOCATOR_H__

//=============================================================================
// Headers
//=============================================================================
#include <memory>
#include "c_memmanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
// trap STL allocations.
#define K2_USE_STL_ALLOCATOR
//=============================================================================

//=============================================================================
// K2Allocator
//=============================================================================
template<class T>
class K2Allocator : public std::allocator<T>
{
public:
	typedef typename std::allocator<T>::size_type	size_type;
	typedef typename std::allocator<T>::pointer		pointer;
	
	template<class _Other>
	struct rebind
	{
		typedef K2Allocator<_Other> other;
	};

	K2Allocator()
	{
		// ensure that the memory manager is initialized.
		CMemManager::GetInstance();
	}

	template<class _Other>
	K2Allocator(const K2Allocator<_Other>&) {}
	K2Allocator(const K2Allocator &al) : std::allocator<T>(al) {}

	~K2Allocator() {}

	template<class _Other>
	std::allocator<T>& operator=(const std::allocator<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}

	pointer	allocate(size_type _Count)				{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL"); }
	pointer allocate(size_type _Count, const void*)	{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL"); }
	void	deallocate(pointer _Ptr, size_type)		{ MemManager.Deallocate(_Ptr, "ctx_STL"); }
};
//=============================================================================

//=============================================================================
// K2StringAllocator
//=============================================================================
template<class T>
class K2StringAllocator : public std::allocator<T>
{
public:
	typedef typename std::allocator<T>::size_type	size_type;
	typedef typename std::allocator<T>::pointer		pointer;
	
	template<class _Other>
	struct rebind
	{
		typedef K2StringAllocator<_Other> other;
	};

	K2StringAllocator()
	{
		// ensure that the memory manager is initialized.
		CMemManager::GetInstance();
	}

	template<class _Other>
	K2StringAllocator(const K2StringAllocator<_Other>&) {}
	K2StringAllocator(const K2StringAllocator &al) : std::allocator<T>(al) {}

	~K2StringAllocator() {}

	template<class _Other>
	std::allocator<T>& operator=(const std::allocator<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}

	pointer	allocate(size_type _Count)				{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_string"); }
	pointer allocate(size_type _Count, const void*)	{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_string"); }
	void	deallocate(pointer _Ptr, size_type)		{ MemManager.Deallocate(_Ptr, "ctx_STL_string"); }
};
//=============================================================================

//=============================================================================
// K2VectorAllocator
//=============================================================================
template<class T>
class K2VectorAllocator : public std::allocator<T>
{
public:
	typedef typename std::allocator<T>::size_type	size_type;
	typedef typename std::allocator<T>::pointer		pointer;
	
	template<class _Other>
	struct rebind
	{
		typedef K2VectorAllocator<_Other> other;
	};

	K2VectorAllocator()
	{
		// ensure that the memory manager is initialized.
		CMemManager::GetInstance();
	}

	template<class _Other>
	K2VectorAllocator(const K2VectorAllocator<_Other>&) {}
	K2VectorAllocator(const K2VectorAllocator &al) : std::allocator<T>(al) {}

	~K2VectorAllocator() {}

	template<class _Other>
	std::allocator<T>& operator=(const std::allocator<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}

	pointer	allocate(size_type _Count)				{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_vector"); }
	pointer allocate(size_type _Count, const void*)	{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_vector"); }
	void	deallocate(pointer _Ptr, size_type)		{ MemManager.Deallocate(_Ptr, "ctx_STL_vector"); }
};
//=============================================================================

//=============================================================================
// K2ListAllocator
//=============================================================================
template<class T>
class K2ListAllocator : public std::allocator<T>
{
public:
	typedef typename std::allocator<T>::size_type	size_type;
	typedef typename std::allocator<T>::pointer		pointer;
	
	template<class _Other>
	struct rebind
	{
		typedef K2ListAllocator<_Other> other;
	};

	K2ListAllocator()
	{
		// ensure that the memory manager is initialized.
		CMemManager::GetInstance();
	}

	template<class _Other>
	K2ListAllocator(const K2ListAllocator<_Other>&) {}
	K2ListAllocator(const K2ListAllocator &al) : std::allocator<T>(al) {}

	~K2ListAllocator() {}

	template<class _Other>
	std::allocator<T>& operator=(const std::allocator<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}

	pointer	allocate(size_type _Count)				{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_list"); }
	pointer allocate(size_type _Count, const void*)	{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_list"); }
	void	deallocate(pointer _Ptr, size_type)		{ MemManager.Deallocate(_Ptr, "ctx_STL_list"); }
};
//=============================================================================

//=============================================================================
// K2DequeAllocator
//=============================================================================
template<class T>
class K2DequeAllocator : public std::allocator<T>
{
public:
	typedef typename std::allocator<T>::size_type	size_type;
	typedef typename std::allocator<T>::pointer		pointer;
	
	template<class _Other>
	struct rebind
	{
		typedef K2DequeAllocator<_Other> other;
	};

	K2DequeAllocator()
	{
		// ensure that the memory manager is initialized.
		CMemManager::GetInstance();
	}

	template<class _Other>
	K2DequeAllocator(const K2DequeAllocator<_Other>&) {}
	K2DequeAllocator(const K2DequeAllocator &al) : std::allocator<T>(al) {}

	~K2DequeAllocator() {}

	template<class _Other>
	std::allocator<T>& operator=(const std::allocator<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}

	pointer	allocate(size_type _Count)				{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_deque"); }
	pointer allocate(size_type _Count, const void*)	{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_deque"); }
	void	deallocate(pointer _Ptr, size_type)		{ MemManager.Deallocate(_Ptr, "ctx_STL_deque"); }
};
//=============================================================================

//=============================================================================
// K2MapAllocator
//=============================================================================
template<class T>
class K2MapAllocator : public std::allocator<T>
{
public:
	typedef typename std::allocator<T>::size_type	size_type;
	typedef typename std::allocator<T>::pointer		pointer;
	
	template<class _Other>
	struct rebind
	{
		typedef K2MapAllocator<_Other> other;
	};

	K2MapAllocator()
	{
		// ensure that the memory manager is initialized.
		CMemManager::GetInstance();
	}

	template<class _Other>
	K2MapAllocator(const K2MapAllocator<_Other>&) {}
	K2MapAllocator(const K2MapAllocator &al) : std::allocator<T>(al) {}

	~K2MapAllocator() {}

	template<class _Other>
	std::allocator<T>& operator=(const std::allocator<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}

	pointer	allocate(size_type _Count)				{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_map"); }
	pointer allocate(size_type _Count, const void*)	{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_map"); }
	void	deallocate(pointer _Ptr, size_type)		{ MemManager.Deallocate(_Ptr, "ctx_STL_map"); }
};
//=============================================================================

//=============================================================================
// K2SetAllocator
//=============================================================================
template<class T>
class K2SetAllocator : public std::allocator<T>
{
public:
	typedef typename std::allocator<T>::size_type	size_type;
	typedef typename std::allocator<T>::pointer		pointer;
	
	template<class _Other>
	struct rebind
	{
		typedef K2SetAllocator<_Other> other;
	};

	K2SetAllocator()
	{
		// ensure that the memory manager is initialized.
		CMemManager::GetInstance();
	}

	template<class _Other>
	K2SetAllocator(const K2SetAllocator<_Other>&) {}
	K2SetAllocator(const K2SetAllocator &al) : std::allocator<T>(al) {}

	~K2SetAllocator() {}

	template<class _Other>
	std::allocator<T>& operator=(const std::allocator<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}

	pointer	allocate(size_type _Count)				{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_set"); }
	pointer allocate(size_type _Count, const void*)	{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_set"); }
	void	deallocate(pointer _Ptr, size_type)		{ MemManager.Deallocate(_Ptr, "ctx_STL_set"); }
};
//=============================================================================

//=============================================================================
// K2HashMapAllocator
//=============================================================================
template<class T>
class K2HashMapAllocator : public std::allocator<T>
{
public:
	typedef typename std::allocator<T>::size_type	size_type;
	typedef typename std::allocator<T>::pointer		pointer;
	
	template<class _Other>
	struct rebind
	{
		typedef K2HashMapAllocator<_Other> other;
	};

	K2HashMapAllocator()
	{
		// ensure that the memory manager is initialized.
		CMemManager::GetInstance();
	}

	template<class _Other>
	K2HashMapAllocator(const K2HashMapAllocator<_Other>&) {}
	K2HashMapAllocator(const K2HashMapAllocator &al) : std::allocator<T>(al) {}

	~K2HashMapAllocator() {}

	template<class _Other>
	std::allocator<T>& operator=(const std::allocator<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}

	pointer	allocate(size_type _Count)				{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_hash_map"); }
	pointer allocate(size_type _Count, const void*)	{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_hash_map"); }
	void	deallocate(pointer _Ptr, size_type)		{ MemManager.Deallocate(_Ptr, "ctx_STL_hash_map"); }
};
//=============================================================================

//=============================================================================
// K2HashSetAllocator
//=============================================================================
template<class T>
class K2HashSetAllocator : public std::allocator<T>
{
public:
	typedef typename std::allocator<T>::size_type	size_type;
	typedef typename std::allocator<T>::pointer		pointer;
	
	template<class _Other>
	struct rebind
	{
		typedef K2HashSetAllocator<_Other> other;
	};

	K2HashSetAllocator()
	{
		// ensure that the memory manager is initialized.
		CMemManager::GetInstance();
	}

	template<class _Other>
	K2HashSetAllocator(const K2HashSetAllocator<_Other>&) {}
	K2HashSetAllocator(const K2HashSetAllocator &al) : std::allocator<T>(al) {}

	~K2HashSetAllocator() {}

	template<class _Other>
	std::allocator<T>& operator=(const std::allocator<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}

	pointer	allocate(size_type _Count)				{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_hash_set"); }
	pointer allocate(size_type _Count, const void*)	{ return (pointer)MemManager.Allocate(_Count * sizeof(T), "ctx_STL_hash_set"); }
	void	deallocate(pointer _Ptr, size_type)		{ MemManager.Deallocate(_Ptr, "ctx_STL_hash_set"); }
};
//=============================================================================

#endif //__K2_STL_ALLOCATOR_H__

