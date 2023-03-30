// (C)2005 S2 Games
// c_materialparameter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_materialparameter.h"
//=============================================================================

/*====================
  IMaterialParameter::GetFloat
  ====================*/
float		IMaterialParameter::GetFloat(float fTime) const
{
	switch(this->GetType())
	{
	case MPT_FLOAT:
		return static_cast<const CMaterialParameter<float> *>(this)->m_Value + static_cast<const CMaterialParameter<float> *>(this)->m_ValueSpeed * fTime;
	case MPT_INT:
		return float(static_cast<const CMaterialParameter<int> *>(this)->m_Value) + float(static_cast<const CMaterialParameter<int> *>(this)->m_ValueSpeed * fTime);
	case MPT_BOOL:
		return static_cast<const CMaterialParameter<bool> *>(this)->m_Value ? 1.0f : 0.0f;
	case MPT_VEC2:
		return static_cast<const CMaterialParameter<CVec2f> *>(this)->m_Value.x + static_cast<const CMaterialParameter<CVec2f> *>(this)->m_ValueSpeed.x * fTime;
	case MPT_VEC3:
		return static_cast<const CMaterialParameter<CVec3f> *>(this)->m_Value.x + static_cast<const CMaterialParameter<CVec3f> *>(this)->m_ValueSpeed.x * fTime;
	case MPT_VEC4:
		return static_cast<const CMaterialParameter<CVec4f> *>(this)->m_Value.x + static_cast<const CMaterialParameter<CVec4f> *>(this)->m_ValueSpeed.x * fTime;
	default:
		return AtoF(GetString());
	}
}


/*====================
  IMaterialParameter::GetInt
  ====================*/
int		IMaterialParameter::GetInt(float fTime) const
{
	switch(this->GetType())
	{
	case MPT_FLOAT:
		return INT_FLOOR(static_cast<const CMaterialParameter<float> *>(this)->m_Value + static_cast<const CMaterialParameter<float> *>(this)->m_Value * fTime);
	case MPT_INT:
		return static_cast<const CMaterialParameter<int> *>(this)->m_Value + INT_FLOOR(static_cast<const CMaterialParameter<int> *>(this)->m_Value * fTime);
	case MPT_BOOL:
		return static_cast<const CMaterialParameter<bool> *>(this)->m_Value ? 1 : 0;
	case MPT_VEC2:
		return INT_FLOOR(static_cast<const CMaterialParameter<CVec2f> *>(this)->m_Value.x + static_cast<const CMaterialParameter<CVec2f> *>(this)->m_Value.x * fTime);
	case MPT_VEC3:
		return INT_FLOOR(static_cast<const CMaterialParameter<CVec3f> *>(this)->m_Value.x + static_cast<const CMaterialParameter<CVec3f> *>(this)->m_Value.x * fTime);
	case MPT_VEC4:
		return INT_FLOOR(static_cast<const CMaterialParameter<CVec4f> *>(this)->m_Value.x + static_cast<const CMaterialParameter<CVec4f> *>(this)->m_Value.x * fTime);
	default:
		return AtoI(GetString());
	}
}


/*====================
  IMaterialParameter::GetBool
  ====================*/
bool	IMaterialParameter::GetBool(float fTime) const
{
	switch(this->GetType())
	{
	case MPT_FLOAT:
		return static_cast<const CMaterialParameter<float> *>(this)->m_Value != 0.0f;
	case MPT_INT:
		return static_cast<const CMaterialParameter<int> *>(this)->m_Value != 0;
	case MPT_BOOL:
		return static_cast<const CMaterialParameter<bool> *>(this)->m_Value;
	case MPT_VEC2:
		return static_cast<const CMaterialParameter<CVec2f> *>(this)->m_Value.x != 0.0f;
	case MPT_VEC3:
		return static_cast<const CMaterialParameter<CVec3f> *>(this)->m_Value.x != 0.0f;
	case MPT_VEC4:
		return static_cast<const CMaterialParameter<CVec4f> *>(this)->m_Value.x != 0.0f;
	default:
		return AtoB(GetString());
	}
}


/*====================
  IMaterialParameter::GetVec2
  ====================*/
CVec2f	IMaterialParameter::GetVec2(float fTime) const
{
	switch(this->GetType())
	{
	case MPT_FLOAT:
		return CVec2f
		(
			static_cast<const CMaterialParameter<float> *>(this)->m_Value + static_cast<const CMaterialParameter<float> *>(this)->m_ValueSpeed * fTime,
			static_cast<const CMaterialParameter<float> *>(this)->m_Value + static_cast<const CMaterialParameter<float> *>(this)->m_ValueSpeed * fTime
		);
	case MPT_INT:
		return CVec2f
		(
			float(static_cast<const CMaterialParameter<int> *>(this)->m_Value),
			float(static_cast<const CMaterialParameter<int> *>(this)->m_Value)
		);
	case MPT_BOOL:
		return CVec2f
		(
			static_cast<const CMaterialParameter<bool> *>(this)->m_Value ? 1.0f : 0.0f,
			static_cast<const CMaterialParameter<bool> *>(this)->m_Value ? 1.0f : 0.0f
		);
	case MPT_VEC2:
		return static_cast<const CMaterialParameter<CVec2f> *>(this)->m_Value + static_cast<const CMaterialParameter<CVec2f> *>(this)->m_ValueSpeed * fTime;
	case MPT_VEC3:
		return static_cast<const CMaterialParameter<CVec3f> *>(this)->m_Value.xy() + static_cast<const CMaterialParameter<CVec3f> *>(this)->m_Value.xy() * fTime;
	case MPT_VEC4:
		return static_cast<const CMaterialParameter<CVec4f> *>(this)->m_Value.xy() + static_cast<const CMaterialParameter<CVec4f> *>(this)->m_Value.xy() * fTime;
	default:
		return AtoV2(GetString());
	}
}


/*====================
  IMaterialParameter::GetVec3
  ====================*/
CVec3f	IMaterialParameter::GetVec3(float fTime) const
{
	switch(this->GetType())
	{
	case MPT_FLOAT:
		return CVec3f
		(
			static_cast<const CMaterialParameter<float> *>(this)->m_Value,
			static_cast<const CMaterialParameter<float> *>(this)->m_Value,
			static_cast<const CMaterialParameter<float> *>(this)->m_Value
		);
	case MPT_INT:
		return CVec3f
		(
			float(static_cast<const CMaterialParameter<int> *>(this)->m_Value),
			float(static_cast<const CMaterialParameter<int> *>(this)->m_Value),
			float(static_cast<const CMaterialParameter<int> *>(this)->m_Value)
		);
	case MPT_BOOL:
		return CVec3f
		(
			static_cast<const CMaterialParameter<bool> *>(this)->m_Value ? 1.0f : 0.0f,
			static_cast<const CMaterialParameter<bool> *>(this)->m_Value ? 1.0f : 0.0f,
			static_cast<const CMaterialParameter<bool> *>(this)->m_Value ? 1.0f : 0.0f
		);
	case MPT_VEC2:
		return CVec3f
		(
			static_cast<const CMaterialParameter<CVec2f> *>(this)->m_Value.x,
			static_cast<const CMaterialParameter<CVec2f> *>(this)->m_Value.y,
			0.0f
		);
	case MPT_VEC3:
		return static_cast<const CMaterialParameter<CVec3f> *>(this)->m_Value;
	case MPT_VEC4:
		return static_cast<const CMaterialParameter<CVec4f> *>(this)->m_Value.xyz();
	default:
		return AtoV3(GetString());
	}
}


/*====================
  IMaterialParameter::GetVec4
  ====================*/
CVec4f	IMaterialParameter::GetVec4(float fTime) const
{
	switch(this->GetType())
	{
	case MPT_FLOAT:
		return CVec4f
		(
			static_cast<const CMaterialParameter<float> *>(this)->m_Value,
			static_cast<const CMaterialParameter<float> *>(this)->m_Value,
			static_cast<const CMaterialParameter<float> *>(this)->m_Value,
			static_cast<const CMaterialParameter<float> *>(this)->m_Value
		);
	case MPT_INT:
		return CVec4f
		(
			float(static_cast<const CMaterialParameter<int> *>(this)->m_Value),
			float(static_cast<const CMaterialParameter<int> *>(this)->m_Value),
			float(static_cast<const CMaterialParameter<int> *>(this)->m_Value),
			float(static_cast<const CMaterialParameter<int> *>(this)->m_Value)
		);
	case MPT_BOOL:
		return CVec4f
		(
			static_cast<const CMaterialParameter<bool> *>(this)->m_Value ? 1.0f : 0.0f,
			static_cast<const CMaterialParameter<bool> *>(this)->m_Value ? 1.0f : 0.0f,
			static_cast<const CMaterialParameter<bool> *>(this)->m_Value ? 1.0f : 0.0f,
			static_cast<const CMaterialParameter<bool> *>(this)->m_Value ? 1.0f : 0.0f
		);
	case MPT_VEC2:
		return CVec4f
		(
			static_cast<const CMaterialParameter<CVec2f> *>(this)->m_Value.x,
			static_cast<const CMaterialParameter<CVec2f> *>(this)->m_Value.y,
			0.0f,
			0.0f
		);
	case MPT_VEC3:
		return CVec4f
		(
			static_cast<const CMaterialParameter<CVec3f> *>(this)->m_Value.x,
			static_cast<const CMaterialParameter<CVec3f> *>(this)->m_Value.y,
			static_cast<const CMaterialParameter<CVec3f> *>(this)->m_Value.z,
			0.0f
		);
	case MPT_VEC4:
		return static_cast<const CMaterialParameter<CVec4f> *>(this)->m_Value;
	default:
		return AtoV4(GetString());
	}
}

