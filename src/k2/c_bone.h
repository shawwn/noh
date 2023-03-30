// (C)2005 S2 Games
// c_bone.h
//
//=============================================================================
#ifndef __C_BONE_H__
#define __C_BONE_H__

//=============================================================================
// Definitions
//=============================================================================
const uint INVALID_BONE(-1);
//=============================================================================

//=============================================================================
// CBone
//=============================================================================
class CBone
{
private:
    uint        m_uiIndex;
    tstring     m_sName;

    uivector    m_vChildren;

    uint        m_uiParent;

    // Force new bones to be initialized
    CBone();

public:
    ~CBone()    {}

    K2_API CBone(uint uiIndex, const tstring &sName, uint uiParentIndex);

    void            SetIndex(uint uiIndex)              { m_uiIndex = uiIndex; }
    uint            GetIndex() const                    { return m_uiIndex; }

    void            SetName(const tstring &sName)       { m_sName = sName; }
    const tstring&  GetName() const                     { return m_sName; }

    uint            GetChildIndex(uint uiIndex) const   { return m_vChildren[uiIndex]; }
    void            AddChild(uint uiChildIndex)         { m_vChildren.push_back(uiChildIndex); }
    uint            NumChildren() const                 { return uint(m_vChildren.size()); }
    const uivector& GetChildren() const                 { return m_vChildren; }

    void            SetParent(uint uiParent)            { m_uiParent = uiParent; }
    uint            GetParentIndex() const              { return m_uiParent; }

    matrix43_t      m_invBase;                  //INVERSE of the base transform for the bone (to transform verts to bone space)
};
//=============================================================================
#endif //__C_BONE_H__
