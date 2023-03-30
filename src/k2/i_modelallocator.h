// (C)2005 S2 Games
// i_modelallocator.h
//
//=============================================================================
#ifndef __I_MODELALLOCATOR_H__
#define __I_MODELALLOCATOR_H__

//=============================================================================
// Definitions
//=============================================================================
#define DEFINE_MODEL_ALLOCATOR(classname, tagname) \
class classname##Allocator : public IModelAllocator \
{ \
private: \
public: \
    ~classname##Allocator() {} \
    classname##Allocator() : \
    IModelAllocator(_T(#tagname)) \
    {} \
 \
 IModel*    Allocate()  { PROFILE(#classname L"Allocator::Allocate"); return K2_NEW(ctx_Resources,  classname); } \
} \
g_##classname##Allocator
//=============================================================================

//=============================================================================
// IModelAllocator
//=============================================================================
class IModelAllocator
{
protected:
    tstring m_sType;

    IModelAllocator();

public:
    virtual ~IModelAllocator();
    IModelAllocator(const tstring &sType);

    virtual class IModel*   Allocate() = 0;
};
//=============================================================================

#endif //__I_MODELALLOCATOR_H__
