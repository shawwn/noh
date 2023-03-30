// (C)2008 S2 Games
// c_stateblock.h
//
//=============================================================================
#ifndef __C_STATEBLOCK_H__
#define __C_STATEBLOCK_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_buffer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef vector<class CStateBlock>           StateBlockVector;
typedef StateBlockVector::iterator          StateBlockVector_it;
typedef StateBlockVector::const_iterator    StateBlockVector_cit;
//=============================================================================

//=============================================================================
// CStateBlock
//=============================================================================
class CStateBlock
{
private:
    CBufferDynamic  m_cBuffer;
    int             m_iModifiedCount;

protected:
    

public:
    ~CStateBlock()  {}
    K2_API CStateBlock();
    CStateBlock(const IBuffer &buffer);

    void                    Clear()                                 { m_cBuffer.Clear(); m_iModifiedCount = 0; }
    CBufferDynamic&         GetBuffer()                             { return m_cBuffer; }
    const CBufferDynamic&   GetBuffer() const                       { return m_cBuffer; }
    bool                    IsEmpty() const                         { return m_cBuffer.GetLength() == 0; }
    int                     GetModifiedCount() const                { return m_iModifiedCount; }
    K2_API void             Set(const IBuffer &buffer);
    void                    Modify(const IBuffer &buffer)           { Set(buffer); ++m_iModifiedCount; }
    void                    Modify()                                { ++m_iModifiedCount; }
    void                    AppendToBuffer(IBuffer &buffer) const   { buffer << m_cBuffer; }
    void                    GetDifference(CStateBlock &ss);

    template <class T> CStateBlock&     operator<<(T _a)            { m_cBuffer << _a; ++m_iModifiedCount; return *this; }
};
//=============================================================================

#endif //__C_STATEBLOCK_H__
