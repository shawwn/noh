// (C)2005 S2 Games
// c_materialparameter.h
//
//=============================================================================
#ifndef __C_MATERIALPARAMETER_H__
#define __C_MATERIALPARAMETER_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"

#include "c_console.h"
#include "xtoa.h"
#include "c_consoleelement.h"
#include "stringutils.h"
#include "c_heap.h"
#include "c_memmanager.h"
//=============================================================================

enum EMatertialParamType
{
    MPT_OTHER = 0,
    MPT_FLOAT,
    MPT_INT,
    MPT_BOOL,
    MPT_VEC2,
    MPT_VEC3,
    MPT_VEC4
};

//=============================================================================
// IMaterialParameter
//=============================================================================
class IMaterialParameter
{
protected:
    tstring                 m_sName;
    EMatertialParamType     m_eType;

public:
    IMaterialParameter(const tstring &sName, EMatertialParamType eType) :
    m_sName(sName),
    m_eType(eType)
    {
    }

    virtual ~IMaterialParameter() {};

    virtual tstring     GetString() const = 0;

    const tstring&      GetName() const         { return m_sName; }
    EMatertialParamType GetType() const         { return m_eType; }

    K2_API float            GetFloat(float fTime) const;
    K2_API int              GetInt(float fTime) const;
    K2_API bool             GetBool(float fTime) const;
    K2_API CVec2f           GetVec2(float fTime) const;
    K2_API CVec3f           GetVec3(float fTime) const;
    K2_API CVec4f           GetVec4(float fTime) const;
};
//=============================================================================

//=============================================================================
// CMaterialParameter
//=============================================================================
template <class T>
class CMaterialParameter : public IMaterialParameter
{
private:
    // Prevent copies
    CMaterialParameter();
    CMaterialParameter(const CMaterialParameter<T> &);

public:
    T   m_Value;
    T   m_ValueSpeed;

    inline CMaterialParameter(const tstring &sName, T _Value, T _ValueSpeed);

    ~CMaterialParameter() {}

    tstring         GetString() const               { return XtoA(m_Value); }
};


/*====================
  CMaterialParameter<T>::CMaterialParameter
  ====================*/
template <class T>
inline CMaterialParameter<T>::CMaterialParameter(const tstring &sName, T _Value, T _ValueSpeed) :
IMaterialParameter(sName, MPT_OTHER),
m_Value(_Value),
m_ValueSpeed(_ValueSpeed)
{
}


/*====================
  CMaterialParameter<int>::CMaterialParameter
  ====================*/
template<>
inline CMaterialParameter<int>::CMaterialParameter(const tstring &sName, int _Value, int _ValueSpeed) :
IMaterialParameter(sName, MPT_INT),
m_Value(_Value),
m_ValueSpeed(_ValueSpeed)
{
}


/*====================
  CMaterialParameter<float>::CMaterialParameter
  ====================*/
template<>
inline CMaterialParameter<float>::CMaterialParameter(const tstring &sName, float _Value, float _ValueSpeed) :
IMaterialParameter(sName, MPT_FLOAT),
m_Value(_Value),
m_ValueSpeed(_ValueSpeed)
{
}


/*====================
  CMaterialParameter<bool>::CMaterialParameter
  ====================*/
template<>
inline CMaterialParameter<bool>::CMaterialParameter(const tstring &sName, bool _Value, bool _ValueSpeed) :
IMaterialParameter(sName, MPT_BOOL),
m_Value(_Value),
m_ValueSpeed(_ValueSpeed)
{
}


/*====================
  CMaterialParameter<CVec2f>::CMaterialParameter
  ====================*/
template<>
inline CMaterialParameter<CVec2f>::CMaterialParameter(const tstring &sName, CVec2f _Value, CVec2f _ValueSpeed) :
IMaterialParameter(sName, MPT_VEC2),
m_Value(_Value),
m_ValueSpeed(_ValueSpeed)
{
}



/*====================
  CMaterialParameter<CVec3f>::CMaterialParameter
  ====================*/
template<>
inline CMaterialParameter<CVec3f>::CMaterialParameter(const tstring &sName, CVec3f _Value, CVec3f _ValueSpeed) :
IMaterialParameter(sName, MPT_VEC3),
m_Value(_Value),
m_ValueSpeed(_ValueSpeed)
{
}


/*====================
  CMaterialParameter<CVec4f>::CMaterialParameter
  ====================*/
template<>
inline CMaterialParameter<CVec4f>::CMaterialParameter(const tstring &sName, CVec4f _Value, CVec4f _ValueSpeed) :
IMaterialParameter(sName, MPT_VEC4),
m_Value(_Value),
m_ValueSpeed(_ValueSpeed)
{
}
//=============================================================================
#endif // __C_MATERIALPARAMETER_H__
