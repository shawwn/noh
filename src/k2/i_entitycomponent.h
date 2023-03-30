// (C)2005 S2 Games
// i_entitycomponent.h
//
// This class describes all entities in the world
//=============================================================================
#ifndef __I_ENTITYCOMPONENT_H__
#define __I_ENTITYCOMPONENT_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_vec3.h"
#include "c_entitycomponentfactory.h"
//=============================================================================

class CEntity;
class CXMLDoc;
class CXMLNode;

#define DECLARE_ENT_COMPONENT_CREATOR(name) \
class CEntity##name##Creator : IEntityComponentCreator \
{ \
public: \
    CEntity##name##Creator() \
    { CEntityComponentFactory::GetInstance()->Register(#name, this); } \
    IEntityComponent*   Create(CEntity *pOwner) { return K2_NEW(global,  CEntity##name)(pOwner); } \
}

#define IMPLEMENT_ENT_COMPONENT_CREATOR(name) \
CEntity##name##Creator  g_##name##Creator

#define GET_ENT_COMPONENT(ent, name)    static_cast<CEntity##name*>(ent)->GetComponent(#name)

//=============================================================================
// IEntityComponent
//=============================================================================
class IEntityComponent
{
protected:
    CEntity     *m_pOwner;
    tstring     m_sName;
    
public:
    IEntityComponent(const tstring &sName, CEntity *pOwner) :
    m_sName(sName),
    m_pOwner(pOwner)
    {
    }

    virtual ~IEntityComponent() {}

    virtual void Frame() = 0;
    
    virtual void Save(CXMLDoc &xmlDoc) = 0;
    virtual void Load(const CXMLNode &node) = 0;
};
//=============================================================================

//=============================================================================
// IEntityComponentCreator
//=============================================================================
class IEntityComponentCreator
{
public:
    IEntityComponentCreator()           {}
    virtual ~IEntityComponentCreator()  {}

    virtual IEntityComponent*   Create(CEntity *pOwner) = 0;
};
//=============================================================================
#endif // __I_ENTITYCOMPONENT_H__
